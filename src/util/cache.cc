/*
 * @Author: py.wang 
 * @Date: 2020-10-14 18:27:40 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-10-20 19:50:15
 */
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "leveldb/cache.h"
#include "port/port.h"
#include "port/thread_annotations.h"
#include "util/hash.h"
#include "util/mutexlock.h"

namespace leveldb {

Cache::~Cache() {}

namespace {

// LRU caceh implementation
struct LRUHandle {
    void *value;
    void (*deleter)(const Slice&, void *value);
    LRUHandle* next_hash;
    LRUHandle* next;
    LRUHandle* prev;
    size_t charge;
    size_t key_length;
    bool in_cache;
    uint32_t refs;      // references, like smart pointer
    uint32_t hash;      // hash of key
    char key_data[1];   // beginning of key

    Slice key() const {
        assert(next != this);

        return Slice(key_data, key_length);
    }
};

class HandleTable {
public:
    HandleTable() : length_(0), elems_(0), list_(nullptr) {
        Resize();
    }

    ~HandleTable() {
        delete[] list_;
    }

    LRUHandle* Lookup(const Slice& key, uint32_t hash) {
        return *FindPointer(key, hash);
    }

    LRUHandle* Insert(LRUHandle* h) {
        LRUHandle** ptr = FindPointer(h->key(), h->hash);
        LRUHandle* old = *ptr;
        h->next_hash = (old == nullptr ? nullptr : old->next_hash);
        *ptr = h;
        if (old == nullptr) {
            ++elems_;
            if (elems_ > length_) {
                Resize();
            }
        }
        return old;
    }

    LRUHandle* Remove(const Slice& key, uint32_t hash) {
        LRUHandle** ptr = FindPointer(key, hash);
        LRUHandle* result = *ptr;
        if (result != nullptr) {
            *ptr = result->next_hash;
            --elems_;
        }
        return result;
    }

private:
    uint32_t length_;
    uint32_t elems_;
    // array of LRUHandle*
    LRUHandle** list_;

    // Return a pointer to slot that pointers to a cache entry that
    // matches key/hash. If there is no such cache entry, return a
    // pointer to the trailing slot in the corresponding linked list.
    LRUHandle** FindPointer(const Slice& key, uint32_t hash) {
        // find position by hash
        LRUHandle** ptr = &list_[hash & (length_ - 1)];
        while (*ptr != nullptr && ((*ptr)->hash != hash || key != (*ptr)->key())) {
            ptr = &(*ptr)->next_hash;
        }
        return ptr;
    }

    void Resize() {
        // start
        uint32_t new_length = 4;
        // expand
        while (new_length < elems_) {
            new_length *= 2;
        }
        LRUHandle** new_list = new LRUHandle*[new_length];
        memset(new_list, 0, sizeof(new_list[0]) * new_length);
        uint32_t count = 0;
        // reverse copy
        for (uint32_t i = 0; i < length_; ++i) {
            LRUHandle* h = list_[i];
            while (h != nullptr) {
                LRUHandle* next = h->next_hash;
                uint32_t hash = h->hash;
                // new position
                LRUHandle** ptr = &new_list[hash & (new_length - 1)];
                h->next_hash = *ptr;
                *ptr = h;
                h = next;
                ++count;
            }
        }
        assert(elems_ == count);
        delete[] list_;
        list_ = new_list;
        length_ = new_length;
    }
};

// A single shard of sharded cache
class LRUCache {
public:
    LRUCache();
    ~LRUCache();

    void SetCapacity(size_t capacity) {
        capacity_ = capacity;
    }

    Cache::Handle* Insert(const Slice& key, uint32_t hash, void* value,
                        size_t charge,
                        void (*deleter)(const Slice& key, void* value));
    Cache::Handle* Lookup(const Slice& key, uint32_t hash);
    void Release(Cache::Handle* handle);
    void Erase(const Slice& key, uint32_t hash);
    void Prune();
    size_t TotalCharge() const {
        MutexLock l(&mutex_);
        return usage_;
    }

private:
    void LRU_Remove(LRUHandle* e);
    void LRU_Append(LRUHandle* list, LRUHandle* e);
    void Ref(LRUHandle* e);
    void Unref(LRUHandle* e);
    bool FinishErase(LRUHandle* e) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

    // Initialized before use
    size_t capacity_;

    // mutex_ protects the following stats
    mutable port::Mutex mutex_;
    size_t usage_ GUARDED_BY(mutex_);

    // Dummy head of LRU list
    LRUHandle lru_ GUARDED_BY(mutex_);

    // Dummy head of in-use list
    LRUHandle in_use_ GUARDED_BY(mutex_);

    HandleTable table_ GUARDED_BY(mutex_);
};

LRUCache::LRUCache() : capacity_(0), usage_(0) {
    // Make empty circular linked lists
    lru_.next = &lru_;
    lru_.prev = &lru_;
    in_use_.next = &in_use_;
    in_use_.prev = &in_use_;
}

LRUCache::~LRUCache() {
    assert(in_use_.next == &in_use_);   // Error if caller has an unreleased handle
    for (LRUHandle* e = lru_.next; e != &lru_; ) {
        LRUHandle* next = e->next;
        assert(e->in_cache);
        e->in_cache = false;
        assert(e->refs == 1);
        Unref(e);
        e = next;
    }
}

// add reference
void LRUCache::Ref(LRUHandle* e) {
    if (e->refs == 1 && e->in_cache) {  // If on lru_ list, move to in_use_ list
        LRU_Remove(e);
        LRU_Append(&in_use_, e);
    }
    ++e->refs;
}

// remove reference
void LRUCache::Unref(LRUHandle* e) {
    assert(e->refs > 0);
    --e->refs;
    if (e->refs == 0) {     // Deallocate
        assert(!e->in_cache);
        (*e->deleter)(e->key(), e->value);
        free(e);
    } else if (e->in_cache && e->refs == 1) {   // Move to lru_
        LRU_Remove(e);
        LRU_Append(&lru_, e);
    }
}

void LRUCache::LRU_Remove(LRUHandle* e) {
    e->next->prev = e->prev;
    e->prev->next = e->next;
}

void LRUCache::LRU_Append(LRUHandle* list, LRUHandle* e) {
    // Make "e" newest entry by inserting just before *list
    e->next = list;
    e->prev = list->prev;
    e->prev->next = e;
    e->next->prev = e;
}

Cache::Handle* LRUCache::Lookup(const Slice& key, uint32_t hash) {
    MutexLock l(&mutex_);
    LRUHandle* e = table_.Lookup(key, hash);
    if (e != nullptr) {
        // Add reference
        Ref(e);
    }
    return reinterpret_cast<Cache::Handle*>(e);
}

void LRUCache::Release(Cache::Handle* handle) {
    MutexLock l(&mutex_);
    // Reduce reference
    Unref(reinterpret_cast<LRUHandle*>(handle));
}

Cache::Handle* LRUCache::Insert(const Slice& key, uint32_t hash, void* value,
                                size_t charge,
                                void (*deleter)(const Slice& key,
                                                void* value)) {
    MutexLock l(&mutex_);

    LRUHandle* e = 
        reinterpret_cast<LRUHandle*>(malloc(sizeof(LRUHandle) - 1 + key.size()));
    e->value = value;
    e->deleter = deleter;
    e->charge = charge;
    e->key_length = key.size();
    e->hash = hash;
    e->in_cache = false;
    e->refs = 1;
    std::memcpy(e->key_data, key.data(), key.size());

    // Have location
    if (capacity_ > 0) {
        ++e->refs;
        e->in_cache = true;
        LRU_Append(&in_use_, e);
        usage_ += charge;
        FinishErase(table_.Insert(e));
    } else {
        // don't cache
        e->next = nullptr;
    }
    // Remove from lru_
    while (usage_ > capacity_ && lru_.next != &lru_) {
        LRUHandle* old = lru_.next;
        assert(old->refs == 1);
        bool erased = FinishErase(table_.Remove(old->key(), old->hash));
        if (!erased) {
            assert(erased);
        }
    }

    return reinterpret_cast<Cache::Handle*>(e);
}

// If e != nullptr, finish removing *e from the cache; it has already been
// removed from the hash table. Return wether e != nullptr
bool LRUCache::FinishErase(LRUHandle* e) {
    if (e != nullptr) {
        assert(e->in_cache);
        LRU_Remove(e);
        e->in_cache = false;
        usage_ -= e->charge;
        Unref(e);
    }
    return e != nullptr;
}

void LRUCache::Erase(const Slice& key, uint32_t hash) {
    MutexLock l(&mutex_);
    FinishErase(table_.Remove(key, hash));
}

void LRUCache::Prune() {
    MutexLock l(&mutex_);
    while (lru_.next != &lru_) {
        LRUHandle* e = lru_.next;
        assert(e->refs == 1);
        bool erased = FinishErase(table_.Remove(e->key(), e->hash));
        if (!erased) {
            assert(erased);
        }
    }
}

static const int kNumShardBits = 4;
static const int kNumShards = 1 << kNumShardBits;

class ShardedLRUCache : public Cache {
public:
    explicit ShardedLRUCache(size_t capacity) : last_id_(0) {
        const size_t per_shard = (capacity + (kNumShards - 1)) / kNumShards;
        for (int s = 0; s < kNumShards; ++s) {
            shard_[s].SetCapacity(per_shard);
        }
    }
    ~ShardedLRUCache() override {}
    Handle* Insert(const Slice& key, void* value, size_t charge,
                    void (*deleter)(const Slice& key, void* value)) override {
        const uint32_t hash = HashSlice(key);
        return shard_[Shard(hash)].Insert(key, hash, value, charge, deleter);
    }
    Handle* Lookup(const Slice& key) override {
        const uint32_t hash = HashSlice(key);
        return shard_[Shard(hash)].Lookup(key, hash);
    }
    void Release(Handle* handle) override {
        LRUHandle* h = reinterpret_cast<LRUHandle*>(handle);
        shard_[Shard(h->hash)].Release(handle);
    }
    void Erase(const Slice& key) override {
        const uint32_t hash = HashSlice(key);
        shard_[Shard(hash)].Erase(key, hash);
    }
    void* Value(Handle* handle) override {
        return reinterpret_cast<LRUHandle*>(handle)->value;
    }
    uint64_t NewId() override {
        MutexLock l(&id_mutex_);
        return ++last_id_;
    }
    void Prune() override {
        for (int s = 0; s < kNumShards; ++s) {
            shard_[s].Prune();
        }
    }
    size_t TotalCharge() const override {
        size_t total = 0;
        for (int s = 0; s < kNumShardBits; ++s) {
            total += shard_[s].TotalCharge();
        }
        return total;
    }
private:
    LRUCache shard_[kNumShards];
    port::Mutex id_mutex_;
    uint64_t last_id_;

    static inline uint32_t HashSlice(const Slice& s) {
        return Hash(s.data(), s.size(), 0);
    }

    static uint32_t Shard(uint32_t hash) {
        return hash >> (32 - kNumShardBits);
    }
};

}   // anonymous namespace

Cache* NewLRUCache(size_t capacity) {
    return new ShardedLRUCache(capacity);
}

}   // namespace leveldb
