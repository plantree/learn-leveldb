/*
 * @Author: py.wang 
 * @Date: 2020-09-10 20:12:55 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-18 20:52:23
 */
#ifndef STORAGE_LEVELDB_UTIL_RANDOM_H
#define STORAGE_LEVELDB_UTIL_RANDOM_H

#include <cstdint>

namespace leveldb {

// A very simple random number generator. Not especially good at
// generating truly random bits, but good enough for our need in 
// this package.
class Random {
public:
    explicit Random(uint32_t s) : seed_(s & 0x7fffffffu) {
        // avoid bad seeds
        if (seed_ == 0 || seed_ == 2147483647L) {
            seed_ = 1;
        }
    }

    uint32_t Next() {
        static const uint32_t M = 2147483647L;  // 2^31-1
        static const uint64_t A = 16807;        // bits 14, 8, 7, 5, 2, 1, 0
    
        uint64_t product = seed_ * A;

        seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
        if (seed_ > M) {
            seed_ -= M;
        }
        return seed_;
    }

    // return a uniformly distributed value in the range [0..n-1]
    // REQUIRE: n > 0
    uint32_t Uniform(int n) {
        return Next() % n;
    }

    bool OneIn(int n) {
        return (Next() % n) == 0; 
    }

    uint32_t Skewed(int max_log) {
        return Uniform(1 << Uniform(max_log + 1));
    }
    
private:
    uint32_t seed_;
};

};  // namespace leveldb

#endif 