/*
 * @Author: py.wang 
 * @Date: 2020-10-12 18:39:20 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-10-13 17:00:29
 */
#include <vector>
#include "leveldb/filter_policy.h"
#include "util/coding.h"
#define CATCH_CONFIG_MAIN
#include "include/catch.hpp"

namespace leveldb {

static const int  kVerbose = 1;

static Slice Key(int i, char* buffer) {
    EncodeFixed32(buffer, i);
    return Slice(buffer, sizeof(uint32_t));
}

class BloomTest {
public:
    BloomTest() : policy_(NewBloomFilterPolicy(10)) {}
    
    ~BloomTest() {
        delete policy_;
    }

    void Reset() {
        keys_.clear();
        filter_.clear();
    }

    void Add(const Slice& s) {
        keys_.push_back(s.ToString());
    }

    void Build() {
        std::vector<Slice> key_slices;
        for (size_t i = 0; i < keys_.size(); ++i) {
            key_slices.push_back(Slice(keys_[i]));
        }
        filter_.clear();
        policy_->CreateFilter(&key_slices[0], static_cast<int>(key_slices.size()),
                            &filter_);
        keys_.clear();
        if (kVerbose >= 2) {
            DumpFilter();
        }
    }

    size_t FilterSize() const {
        return filter_.size();
    }

    void DumpFilter() {
        std::fprintf(stderr, "F(");
        // ignore probes in the last
        for (size_t i = 0; i < filter_.size() - 1; ++i) {
            const unsigned int c = static_cast<unsigned int>(filter_[i]);
            for (int j = 0; j < 8; ++j) {
                std::fprintf(stderr, "%c", (c & (1 < j)) ? '1' : '.');
            }
        }
        std::fprintf(stderr, ")\n");
    }

    bool Matches(const Slice& s) {
        if (!keys_.empty()) {
            Build();
        }
        return policy_->KeyMayMatch(s, filter_);
    }

    double FalsePositiveRate() {
        char buffer[sizeof(int)];
        int result = 0;
        for (int i = 0; i < 10000; ++i) {
            if (Matches(Key(i + 1000000000, buffer))) {
                ++result;
            }
        }
        return result / 10000.0;
    }

private:
    const FilterPolicy* policy_;
    std::string filter_;
    std::vector<std::string> keys_;
};

TEST_CASE("Empty", "[bloom]") {
    BloomTest t;
    REQUIRE(!t.Matches("hello"));
    REQUIRE(!t.Matches("world"));
}

TEST_CASE("Small", "[bloom]") {
    BloomTest t;
    t.Add("hello");
    t.Add("world");
    REQUIRE(t.Matches("hello"));
    REQUIRE(t.Matches("world"));
    REQUIRE(!t.Matches("x"));
    REQUIRE(!t.Matches("foo"));
}

static int NextLength(int length) {
    if (length < 10) {
        length += 1;
    } else if (length < 100) {
        length += 10;
    } else if (length < 1000) {
        length += 100;
    } else {
        length += 1000;
    }
    return length;
}

TEST_CASE("VaringLengths", "[bloom]") {
    BloomTest t;
    char buffer[sizeof(int)];

    // count number of filters that significantly exceed the false positive rate
    int mediocre_filters = 0,
        good_filters = 0;
    
    for (int length = 1; length <= 10000; length = NextLength(length)) {
        t.Reset();
        for (int i = 0; i < length; ++i) {
            t.Add(Key(i, buffer));
        }
        t.Build();

        REQUIRE(t.FilterSize() <= static_cast<size_t>((length * 10 / 8) + 40));

        // all added keys must match
        for (int i = 0; i < length; ++i) {
            REQUIRE(t.Matches(Key(i, buffer)));
        }

        // check false positive rate
        double rate = t.FalsePositiveRate();
        REQUIRE(rate <= 0.02);
        if (rate >= 0.0125) {
            ++mediocre_filters;
        } else {
            ++good_filters;
        }
    }
    REQUIRE(mediocre_filters <= good_filters / 5);
}




}   // namespace leveldb