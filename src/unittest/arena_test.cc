/*
 * @Author: py.wang 
 * @Date: 2020-09-10 20:11:24 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-16 18:41:32
 */
#include <vector>
#include <utility>
#include "arena.h"
#include "random.h"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("test arena", "[arena]") {
    std::vector<std::pair<size_t, char*>> allocated;
    leveldb::Arena arena;
    const int N = 100000;
    size_t bytes = 0;
    leveldb::Random rnd(301);
    for (int i = 0; i < N; ++i) {
        size_t s;
        if (i % (N / 10) == 0) {
            s = i;
        } else {
            s = rnd.OneIn(4000) 
                ? rnd.Uniform(6000)
                : (rnd.OneIn(10) ? rnd.Uniform(100) : rnd.Uniform(20));
        }
        if (s == 0) {
            // disallow size 0 allocation
            s = 1;
        }
        char *r;
        if (rnd.OneIn(10)) { 
            r = arena.AllocateAligned(s);
        } else {
            r = arena.Allocate(s);
        }

        for (size_t b = 0; b < s; ++b) {
            r[b] = i % 256;
        }
        bytes += s;
        allocated.push_back(std::make_pair(s, r));
        REQUIRE(arena.MemoryUsage() >= bytes);
        if (i > N / 10) {
            REQUIRE(arena.MemoryUsage() <= bytes * 1.10);
        }
    }
    for (size_t i = 0; i < allocated.size(); ++i) {
        size_t num_bytes = allocated[i].first;
        const char* p = allocated[i].second;
        for (size_t b = 0; b < num_bytes; ++b) {
            REQUIRE((static_cast<int>(p[b]) & 0xff) == i % 256);
        }
    }
}

