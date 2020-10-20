/*
 * @Author: py.wang 
 * @Date: 2020-10-13 20:09:28 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-10-14 17:13:49
 */
#ifndef STORAGE_LEVELDB_UTIL_MUTEXLOCK_H
#define STORAGE_LEVELDB_UTIL_MUTEXLOCK_H

#include "port/port.h"
#include "port/thread_annotations.h"

namespace leveldb {

class SCOPED_LOCKABLE MutexLock {
public:
    explicit MutexLock(port::Mutex* mu) EXCLUSIVE_LOCK_FUNCTION(mu) : mu_(mu) {
        this->mu_->Lock();
    }

    ~MutexLock() UNLOCK_FUNCTION() {
        this->mu_->Unlock();
    }

    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;

private:
    port::Mutex* const mu_;
};

}   // namespace leveldb

#endif  // STORAGE_LEVELDB_UTIL_MUTEXLOCK_H