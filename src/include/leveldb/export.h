/*
 * @Author: py.wang 
 * @Date: 2020-09-24 17:30:58 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-24 17:37:57
 */
#ifndef STORAGE_LEVELDB_INCLUDE_EXPORT_H
#define STORAGE_LEVELDB_INCLUDE_EXPORT_H

#if !defined(LEVELDB_EXPORT)

#if defined(LEVELDB_SHARED_LIBRARY)
#if defined(_WIN32)

#if defined(LEVELDB_COMPILE_LIBRARY)
#define LEVELDB_EXPORT __declspec(dllexport)
#else 
#define LEVELDB_EXPROT __declspec(dllimport)
#endif  // LEVELDB_COMPILE_LIBRARY

#else  // _WIN32
#if defined(LEVELDB_COMPILE_LIBRARY)
#define LEVELDB_EXPORT __attribute__((visibility("default")))
#else 
#define LEVELDB_EXPORT
#endif 

#endif  // _WIN32

#else   // LEVELDB_SHARED_LIBRARY
#define LEVELDB_EXPORT
#endif  

#endif  // !LEVELDB_EXPORT

#endif // STORAGE_LEVELDB_INCLUDE_EXPORT_H