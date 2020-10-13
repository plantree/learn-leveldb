/*
 * @Author: py.wang 
 * @Date: 2020-09-25 19:54:09 
 * @Last Modified by: py.wang
 * @Last Modified time: 2020-09-27 20:38:30
 */
#ifndef STORAGE_LEVELDB_PORT_THREAD_ANNOTATIONS_H
#define STORAGE_LEVELDB_PORT_THREAD_ANNOTATIONS_H

// Use Clang's thread safety analysis annotations when available

#if !defined(THREAD_ANNOTATION_ATTRIBUTE__)

#if defined(__clang__)

#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)
#endif

#endif  // !defined(THREAD_ANNOTATION_ATTRIBUTE)

#ifndef GUARDED_BY
#define GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))
#endif 

#ifndef PT_GUARDED_BY
#define PT_GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))
#endif 

#ifndef ACQUIRED_AFTER
#define ACQUIRED_AFTER(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))
#endif 

#ifndef ACQUIRED_BEFORE
#define ACQUIRED_BEFORE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acqurie_before(__VA_ARGS__))
#endif

#ifndef EXCLUSIVE_LOCKS_REQUIRED
#define EXCLUSIVE_LOCKS_REQUIRED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_locsk_required(__VA_ARGS__))
#endif

#ifndef SHARED_LOCKS_REQUIRED
#define SHARED_LOCKS_REQUIRED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(shared_locks_required(__VA_ARGS__))
#endif 

#ifndef LOCKS_EXCLUDED
#define LOCKS_EXLCUDED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))
#endif

#ifndef LOCK_RETURNED
#define LOCK_RETURNED(...)  \
    THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(__VA_ARGS__))
#endif 

#ifndef LOCKABLE
#define LOCKABLE THREAD_ANNOTATION_ATTRIBUTE__(lockable)
#endif

#ifndef SCOPED_LOCKABLE
#define SCOPED_LOCKABLE THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)
#endif 

#ifndef EXCLUSIVE_LOCK_FUNCTION
#define EXCLUSIVE_LOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock_function(__VA_ARGS__))
#endif 

#ifndef SHARED_LOCK_FUNCTION
#define SHARED_LOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(shared_lock_function(__VA_ARGS__))
#endif 

#ifndef SHARED_TRYLOCK_FUNCTION
#define SHARED_TRYLOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(shared_trylock_function(__VA_ARGS__))
#endif 

#ifndef UNLOCK_FUNCTION
#define UNLOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(unlock_function(__VA_ARGS__))
#endif 

#ifndef NO_THREAD_SAFETY_ANALYSIS
#define NO_THREAD_SAFETY_ANALYSIS \
    THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)
#endif 

#ifndef ASSERT_EXCLUSIVE_LOCK
#define ASSERT_EXCLUSIVE_LOCK(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_exclusive_lock(__VA_ARGS__))
#endif 

#ifndef ASSERT_EXCLUSIVE_LOCK
#define ASSERT_EXCLUSIVE_LOCK(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_lock(__VA_ARGS__))
#endif

#ifndef ASSERT_SHARED_LOCK
#define ASSERT_SHARED_LOCK(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_lock(__VA_ARGS__))
#endif

#endif  // STORAGE_LEVELDB_PORT_THREAD_ANNOTATIONS_H