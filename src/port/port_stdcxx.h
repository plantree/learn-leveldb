/*
 * @Author: py.wang 
 * @Date: 2020-09-24 19:31:54 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-28 20:56:20
 */
#ifndef STORAGE_LEVELDB_PORT_PORT_STDCXX_H
#define STORAGE_LEVELDB_PORT_PORT_STDCXX_H

// port/port_config.h availablity is automatically detected via __has_include
// in newer compilers. If LEVELDB_HAS_PORT_CONFIG_H is defined, it overrides the
// configuration detection.
#if defined(LEVELDB_HAS_PORT_CONFIG_H)
#include "port/port_config.h"
#endif  // LEVELDB_HAS_PORT_CONFIG_H

#elif defined(__has_include)

#if __has_include("port/port_config.h")
#include "port/port_config.h"
#endif  // __has_include("port/port_config.h")

#endif  // LEVELDB_HAS_PORT_CONFIG_H

#if HAVE_CRC32C
#include <crc32c/crc32c.h>
#endif  // HAVE_CRC32C
#if HAVE_SNAPPY
#include <snappy.h>
#endif  // HAVE_SNAPPY

#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

#include "port/thread_annotations.h"

namespace leveldb {

namespace port {

class CondVar;

// wraps std::mutex
class LOCKABLE Mutex {
public:
    Mutex() = default;
    ~Mutex() = default;

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void Lock() EXCLUSIVE_LOCK_FUNCTION() { mutex_.lock(); }
    void Unlock() UNLOCK_FUNCTION() { mutex_.unlock(); }
    void AssertHeld() ASSERT_EXCLUSIVE_LOCK() {}

private:
    friend class CondVar;
    std::mutex mutex_;
};

// wraps std::condition_variable
class CondVar {
public:
    explicit CondVar(Mutex* mutex) : mutex_(mutex) { assert(mutex != nullptr); }
    ~CondVar() = default;

    CondVar(const CondVar&) = delete;
    CondVar& operator=(const CondVar&) = delete;

    void Wait() {
        std::unique_lock<std::mutex> lock(mutex_->mutex_, std::adopt_lock);
        cv_.wait(lock);
        lock.release();
    }
    void Signal() { cv_.notify_one(); }
    void SignalAll() { cv_.notify_all(); }

private:
    std::condition_variable cv_;
    Mutex* const mutex_;
};

inline bool Snappy_compress(const char* input, size_t length,
                            std::string* output) {
#if HAVE_SNAPPY
    output->resize(snappy::MaxCompressedLength(length));
    size_t outlen;
    snappy::RawCompress(input, length, &(*output)[0], &outlen);
    output->resize(outlen);
    return true;
#else 
    // silence compiler warnings about unused arguments
    (void)input;
    (void)length;
    (void)output;
#endif // HAVE SNAPPY

    return false;
}

inline bool Snappy_GetUncompressedLength(const char* input, size_t length,
                                        size_t* result) {
#if HAVE_SNAPPY
    return snappy::GetUncompressedLength(input, length, result);
#else 
    // silence compiler warnings about unused arguments
    (void)input;
    (void)length;
    (void)result;
    return false;
#endif  // HAVE_SNAPPY
}

inline bool Snappy_Uncompressed(const char* input, size_t length, char* output) {
#if HAVE_SNAPPY
    return snappy::RawUncompress(input, length, output);
#else
    // silence compiler warnings about unused arguments
    (void)input;
    (void)length;
    (void)output;
    return false;
#endif  // HAVE_SNAPPY
}

inline bool GetHeapProfile(void (*func)(void*, const char*, int), void* args) {
    // silence compiler warnings
    (void)func;
    (void)args;
    return false;
}

inline uint32_t AccelerateCRC32C(uint32_t crc, const char* buf, size_t size) {
#if HAVE_CRC32C
    return ::crc32c::Extend(crc, reinterpret_cast<const uint8_t*>(buf), size);
#else 
    // silence compiler warnings
    (void)crc;
    (void)buf;
    (void)size;
    return 0;
#endif  // HAVE_CRC32C
}

}   // namespace port

}   // namespace leveldb

namespace leveldb {



}   // namespace leveldb


#if LEVELDB_HAS_PORT_CONFIG_H


#endif // STORAGE_LEVELDB_PORT_PORT_STDCXX_H