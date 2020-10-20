/*
 * @Author: py.wang 
 * @Date: 2020-10-16 19:05:50 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-10-20 21:00:59
 */
#include <vector>
#include <cassert>
#include "leveldb/cache.h"
#include "util/coding.h"
#define CATCH_CONFIG_MAIN
#include "include/catch.hpp"

namespace leveldb {

// Conversion between numeric keys/values and the types expected by Cache
static std::string EncodeKey(int k) {
    std::string result;
    PutFixed32(&result, k);
    return result;
}

static int DecodeKey(const Slice& k) {
    assert(k.size() == 4);
    return DecodeFixed32(k.data());
}

static void* EncodeValue(uintptr_t v) {
    return reinterpret_cast<void*>(v);
}

static int DecodeValue(void* v) {
    return reinterpret_cast<uintptr_t>(v);
}

class CacheTest {
public:
    static void Deleter(const Slice& key, void* v) {
        current_->deleted_keys_.push_back(DecodeKey(key));
        current_->deleted_values_.push_back(DecodeValue(v));
    }

    static constexpr int kCacheSize = 1000;
    static CacheTest* current_;
    std::vector<int> deleted_keys_;
    std::vector<int> deleted_values_;
    Cache* cache_;

    CacheTest() : cache_(NewLRUCache(kCacheSize)) {
        current_ = this;
    }

    ~CacheTest() {
        delete cache_;
    }

    int Lookup(int key) {
        Cache::Handle* handle = cache_->Lookup(EncodeKey(key));
        const int r = (handle == nullptr) ? -1 : DecodeValue(cache_->Value(handle));
        if (handle != nullptr) {
            cache_->Release(handle);
        }
        return r;
    }

    void Insert(int key, int value, int charge = 1) {
        cache_->Release(cache_->Insert(EncodeKey(key), EncodeValue(value), charge,
                                        &CacheTest::Deleter));
    }

    Cache::Handle* InsertAndReturnHandle(int key, int value, int charge = 1) {
        return cache_->Insert(EncodeKey(key), EncodeValue(value), charge,
                                &CacheTest::Deleter);
    }

    void Erase(int key) {
        cache_->Erase(EncodeKey(key));
    }
};

CacheTest* CacheTest::current_ = nullptr;

TEST_CASE("HitAndMiss", "[cache]") {
    CacheTest t;
    REQUIRE(t.Lookup(100) == -1);

    t.Insert(100, 101);
    REQUIRE(101 == t.Lookup(100));
    REQUIRE(-1 == t.Lookup(200));
    REQUIRE(-1 == t.Lookup(300));

    t.Insert(200, 201);
    REQUIRE(101 == t.Lookup(100));
    REQUIRE(201 == t.Lookup(200));
    REQUIRE(-1 == t.Lookup(300));

    t.Insert(100, 102);
    REQUIRE(102 == t.Lookup(100));
    REQUIRE(201 == t.Lookup(200));
    REQUIRE(-1 == t.Lookup(300));
    
    REQUIRE(1 == t.deleted_keys_.size());
    REQUIRE(100 == t.deleted_keys_[0]);
    REQUIRE(101 == t.deleted_values_[0]);
}

TEST_CASE("Erase", "[cache]") {
    CacheTest t;
    t.Erase(200);
    REQUIRE(0 == t.deleted_keys_.size());

    t.Insert(100, 101);
    t.Insert(200, 201);
    t.Erase(100);
    REQUIRE(-1 == t.Lookup(100));
    REQUIRE(201 == t.Lookup(200));
    REQUIRE(1 == t.deleted_keys_.size());
    REQUIRE(100 == t.deleted_keys_[0]);
    REQUIRE(101 == t.deleted_values_[0]);

    t.Erase(100);
    REQUIRE(-1 == t.Lookup(100));
    REQUIRE(201 == t.Lookup(200));
    REQUIRE(1 == t.deleted_keys_.size());
}

TEST_CASE("EntiresArePinned", "[cache]") {
    CacheTest t;
    t.Insert(100, 101);
    Cache::Handle* h1 = t.cache_->Lookup(EncodeKey(100));
    REQUIRE(101 == DecodeValue(t.cache_->Value(h1)));

    t.Insert(100, 102);
    Cache::Handle* h2 = t.cache_->Lookup(EncodeKey(100));
    REQUIRE(102 == DecodeValue(t.cache_->Value(h2)));

    t.cache_->Release(h1);
    REQUIRE(1 == t.deleted_keys_.size());
    REQUIRE(100 == t.deleted_keys_[0]);
    REQUIRE(101 == t.deleted_values_[0]);

    t.Erase(100);
    REQUIRE(-1 == t.Lookup(100));
    REQUIRE(1 == t.deleted_keys_.size());

    t.cache_->Release(h2);
    REQUIRE(2 == t.deleted_keys_.size());
    REQUIRE(100 == t.deleted_keys_[1]);
    REQUIRE(102 == t.deleted_values_[1]);
}

TEST_CASE("EvictionPolicy", "[cache]") {
    CacheTest t;
    t.Insert(100, 101);
    t.Insert(200, 201);
    t.Insert(300, 301);
    Cache::Handle* h = t.cache_->Lookup(EncodeKey(300));

    // Frequently used entry must be kept arount,
    // as must things that are still in use
    for (int i = 0; i < t.kCacheSize + 100; ++i) {
        t.Insert(1000 + i, 2000 + i);
        REQUIRE(2000 + i == t.Lookup(1000 + i));
        REQUIRE(101 == t.Lookup(100));
    }
    REQUIRE(101 == t.Lookup(100));
    REQUIRE(-1 == t.Lookup(200));
    REQUIRE(301 == t.Lookup(300));
    t.cache_->Release(h);
}

TEST_CASE("UseExceedsCacheSize", "[cache]") {
    CacheTest t;
    std::vector<Cache::Handle*> h;
    for (int i = 0; i < t.kCacheSize + 100; ++i) {
        h.push_back(t.InsertAndReturnHandle(1000 + i, 2000 + i));
    }

    // Check that all the entries can be found in the cache
    for (int i = 0; i < h.size(); ++i) {
        REQUIRE(2000 + i == t.Lookup(1000 + i));
    }

    for (int i = 0; i < h.size(); ++i) {
        t.cache_->Release(h[i]);
    }
}

TEST_CASE("HeavyEntries", "[cache]") {
    CacheTest t;
    const int kLight = 1;
    const int kHeavy = 10;
    int added = 0;
    int index = 0;
    while (added < 2 * t.kCacheSize) {
        const int weight = (index & 1) ? kLight : kHeavy;
        t.Insert(index, 1000 + index, weight);
        added += weight;
        ++index;
    }

    int cached_weight = 0;
    for (int i = 0; i < index; ++i) {
        const int weight = (i & 1) ? kLight : kHeavy;
        int r = t.Lookup(i);
        if (r >= 0) {
            cached_weight += weight;
            REQUIRE(1000 + i == r);
        }
    }
    REQUIRE(cached_weight <= t.kCacheSize + t.kCacheSize / 10);
}

TEST_CASE("NewId", "[cache]") {
    CacheTest t;
    uint64_t a = t.cache_->NewId();
    uint64_t b = t.cache_->NewId();
    REQUIRE(a != b);
}

TEST_CASE("Prune", "[cache]") {
    CacheTest t;
    t.Insert(1, 100);
    t.Insert(2, 200);

    Cache::Handle* handle = t.cache_->Lookup(EncodeKey(1));
    REQUIRE(handle != nullptr);
    t.cache_->Prune();
    t.cache_->Release(handle);

    REQUIRE(100 == t.Lookup(1));
    REQUIRE(-1 == t.Lookup(2));
}

TEST_CASE("ZeroSizeCache", "[cache]") {
    CacheTest t;
    delete t.cache_;

    t.cache_ = NewLRUCache(0);

    t.Insert(1, 100);
    REQUIRE(-1 == t.Lookup(1));
}

}   // namespace leveldb