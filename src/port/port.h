/*
 * @Author: py.wang 
 * @Date: 2020-09-24 19:28:57 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-10-20 19:22:53
 */
#ifndef STORAGE_LEVELDB_PORT_PORT_H
#define STORAGE_LEVELDB_PORT_PORT_H

#include <string.h>

// include the appropriate platform specific file below
//#include "port/port_stdcxx.h"
#if defined(LEVELDB_PLATFORM_POSIX) || defined(LEVELDB_PLATFORM_WINDOWS)
#include "port/port_stdcxx.h"
#elif defined(LEVELDB_PLATFORM_CHROMIUN)
#include "port/port_chromium.h"
#endif

#endif  // STORAGE_LEVELDB_PORT_PORT_H