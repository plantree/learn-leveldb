/*
 * @Author: py.wang 
 * @Date: 2020-09-10 17:48:08 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-24 17:27:32
 */
#ifndef STORAGE_LEVELDB_UTIL_ARENA_H
#define STORAGE_LEVELDB_UTIL_ARENA_H

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace leveldb {

class Arena {
public:
    Arena();

    // cannot be copied    
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    ~Arena();

    // return a pointer to a newly allocated memory block of "bytes" bytes.
    char* Allocate(size_t bytes);
    
    // allocate memory with the normal alignment guarantees provided by malloc
    char* AllocateAligned(size_t bytes);

    // returns an estimate of the total memory usage of data allocated by the arena
    size_t MemoryUsage() const {
        // gurantee atomicity
        return memory_usage_.load(std::memory_order_relaxed);
    }

private:
    // 回退：降级
    char* AllocateFallback(size_t bytes);
    char* AllocateNewBlock(size_t block_bytes);

    // allocation state
    char* alloc_ptr_;
    size_t alloc_bytes_remaining_;

    // array of new[] allocated memory blocks
    std::vector<char*> blocks_;

    // total memory usage of the arena
    std::atomic<size_t> memory_usage_;
};

inline char* Arena::Allocate(size_t bytes) {
    assert(bytes > 0);
    if (bytes <= alloc_bytes_remaining_) {
        char* result = alloc_ptr_;
        alloc_ptr_ += bytes;
        alloc_bytes_remaining_ -= bytes;
        return result;
    }
    return AllocateFallback(bytes);
}

};  // namepsace leveldb


#endif  // STORAGE_LEVELDB_UTIL_ARENA_H