/*
 * @Author: py.wang 
 * @Date: 2020-09-25 19:43:05 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-25 19:57:53
 */
#ifndef STORAGE_LEVELDB_PORT_PORT_CONFIG_H
#define STORAGE_LEVELDB_PORT_PORT_CONFIG_H

#if !defined(HAVE_DFATASYC)
#cmakedefine01 HAVE_FDATASYNC
#endif  // !defined(HAVE_FDATASYNC)

#if !defined(HAVE_FULLSYNC)
#cmakedefine01 HAVE_FULLSYNC
#endif  // !defined(HAVE_FULLSYNC)

#if !defined(HAVE_O_CLOEXEC)
#cmakedefine01 HAVE_O_CLOEXEC
#endif  // !defined(HAVE_O_CLOEXEC)

#if !defined(HAVE_CRC32C)
#cmakedefine01 HAVE_CRC32C
#endif  // !defined(HAVE_CRC32C)

#if !defined(HAVE_SNAPPY)
#cmakedefine01 HAVE_SNAPPY
#endif 

#endif  // STORAGE_LEVELDB_PORT_PORT_CONFIG_H