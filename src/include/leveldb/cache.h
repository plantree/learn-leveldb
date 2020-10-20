/*
 * @Author: py.wang 
 * @Date: 2020-10-14 17:14:36 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-10-19 19:15:06
 */

// A Cache is an interface that maps keys to values. It has
// internal synchronization and may be safely accessed concurrently from
// multiple threads.
// A builtin cache implementation with a least-recently-used eviction policy

#ifndef STORAGE_LEVELDB_INCLUDE_CACHE_H
#define STORAGE_LEVELDB_INCLUDE_CACHE_H

#include <stdint.h>
#include "leveldb/export.h"
#include "leveldb/slice.h"

namespace leveldb {

class LEVELDB_EXPORT Cache;

// Create a new cache with a fixed size capacity
LEVELDB_EXPORT Cache *NewLRUCache(size_t capacity);

class LEVELDB_EXPORT Cache {
public:
    Cache() = default;

    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    // Destroys all existing entries by calling the "deleter"
    // function that was passed to the constructor.
    virtual ~Cache();

    // Opaque handle to an entry stored in the cache
    struct Handle {};

    virtual Handle* Insert(const Slice& key, void* value, size_t charge,
                        void (*deleter)(const Slice& key, void *value)) = 0;
    virtual Handle* Lookup(const Slice& key) = 0;
    
    // Release a mapping returned by a previous Lookup()
    virtual void Release(Handle* handle) = 0;

    virtual void* Value(Handle* handle) = 0;

    virtual void Erase(const Slice& key) = 0;

    // Returns a new numeric id.
    virtual uint64_t NewId() = 0;
    
    // Remove all cache entries that are not actively in use.
    virtual void Prune() {}

    // Return an estimate of the combined charges of all elements stored
    // in the cache
    virtual size_t TotalCharge() const = 0;
    
private:
    void LRU_Remove(Handle* e);
    void LRU_Append(Handle* e);
    void Unref(Handle* e);

    struct Rep;
    Rep* rep_;
};

}   // namespace leveldb


#endif  // STORAGE_LEVELDB_INCLUDE_CACHE_H