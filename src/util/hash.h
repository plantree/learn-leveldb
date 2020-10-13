/*
 * @Author: py.wang 
 * @Date: 2020-09-24 17:10:23 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-24 17:27:40
 */

#ifndef STORAGE_LEVELDB_UTIL_HASH_H
#define STORAGE_LEVELDB_UTIL_HASH_H

#include <cstddef>
#include <cstdint>

namespace leveldb {

uint32_t Hash(const char* data, size_t n, uint32_t seed);

}   // namespace leveldb

#endif  // STORAGE_LEVELDB_UTIL_HASH_H