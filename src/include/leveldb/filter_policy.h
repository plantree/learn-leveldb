/*
 * @Author: py.wang 
 * @Date: 2020-10-12 17:18:57 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-10-12 17:29:18
 */
#ifndef STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H
#define STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H

#include <string>
#include "leveldb/export.h"

namespace leveldb {

class Slice;

class LEVELDB_EXPORT FilterPolicy {
public:
    virtual ~FilterPolicy();

    // Return the name of this policy. Note that if the filter encoding
    // changes in an incompatible way, the name returned by this method
    // must be changed. Otherwise, old incompatible filters may be
    // passed to methods of this type.
    virtual const char* Name() const = 0;

    // keys[0,n-1] contains a list of keys (potentially with duplicates)
    // that are ordered according to the user supplied comparator.
    // Append a filter that summarizes  keys[0,n-1] to *dst
    //
    // Warning: do not change the initial contents of *dst. Instead
    // append the newly constructed filter to *dst
    virtual void CreateFilter(const Slice* keys, int n,
                            std::string* dst) const = 0;

    // "filter" contains the data appended by a preceding call to
    // CreateFilter() ont this class. This method must return true if
    // the key was in the list of keys passed to CreateFilter().
    // This method may return true or flase if the key was not on the
    // list, but it should aim to return false with a high probability.
    virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0;
    
};

// Return a new filter policy that uses a bloom filter with approximately
// the specified number of bits per key. A good value for bits_per_key
// is 10, which yields a filter with ~ 1% false positive rate.
LEVELDB_EXPORT const FilterPolicy* NewBloomFilterPolicy(int bits_per_key);
     
}   // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_FILTER_POLICY