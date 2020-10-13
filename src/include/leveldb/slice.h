/*
 * @Author: py.wang 
 * @Date: 2020-09-24 17:30:06 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-29 15:31:13
 */
// Slice is a simple structure conatining a pointer into some external
// storage and size. The user of a Slice must ensure that the slice
// is not used after the corresponding external storage has been
// deallocated.
//
// Multiple threads can invoke const methods on a Slice without
// external synchronozation, but if any of the threads may call a
// non-const method, all threads accessing the same Slice must use
// external synchronization.

#ifndef STORAGE_LEVELDB_INCLUDE_SLICE_H
#define STORAGE_LEVELDB_INCLUDE_SLICE_H

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

#include "leveldb/export.h"

namespace leveldb {

class LEVELDB_EXPORT Slice {
public:
    // create an empty slice
    Slice() : data_(""), size_(0) {}

    // create a slice that refers to d[0, n-1]
    Slice(const char* d, size_t n) : data_(d), size_(n) {}

    // create a slice that refers to the contents of "s"
    Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

    // create a slice that refers to s[0, strlen(s)-1]
    Slice(const char* s) : data_(s), size_(strlen(s)) {}

    // intentionally copyable
    Slice(const Slice&) = default;
    Slice& operator=(const Slice&) = default;
    
    // return a pointer to the beginning of the referenced data
    const char* data() const {
        return data_;
    }

    // return the length (in bytes) of the referenced data
    size_t size() const {
        return size_;
    }

    // return true if the length of the referenced data is zero
    bool empty() const {
        return size_ == 0;
    }

    // return the ith byte in the reference data
    // REQUIRES: n < size()
    char operator[](size_t n) const {
        assert(n < size());
        return data_[n];
    }

    // change this slice to refer to an empty array
    void clear() {
        data_ = "";
        size_ = 0;
    }

    // drop the first "n" bytes from this slice
    void remove_prefix(size_t n) {
        assert(n <= size());
        data_ += n;
        size_ -= n;
    }

    // return a string that contians the copy of the referenced data
    std::string ToString() const {
        return std::string(data_, size_);
    }

    int compare(const Slice& b) const;

    // return true if "x" is a prefix of "*this"
    bool starts_with(const Slice& x) const {
        return ((size_ >= x.size()) && (memcmp(data_, x.data_, x.size_) == 0));
    }

private:
    const char* data_;
    size_t size_;
};

inline bool operator==(const Slice& x, const Slice& y) {
    return ((x.size() == y.size()) &&
            (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) {
    return !(x == y);
}

inline int Slice::compare(const Slice& b) const {
    const size_t min_len = (size_ < b.size_) ? size_ : b.size();
    int r = memcmp(data_, b.data_, min_len);
    if (r == 0) {
        if (size_ < b.size_) {
            r = -1;
        } else if (size_ > b.size_) {
            r = +1;
        }
    }
    return r;
}

}   // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_SLICE_H