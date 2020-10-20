set(LEVELDB_UTILS
    arena.cc
    coding.cc
    hash.cc
    filter_policy.cc
    bloom.cc
    cache.cc)

# add macro
if (WIN32)
    set(LEVELDB_PLATFORM_NAME LEVELDB_PLATFORM_WINDOWS)
    add_definitions(-D_UNICODE -DUNICODE)
else (WIN32)
    set(LEVELDB_PLATFORM_NAME LEVELDB_PLATFORM_POSIX)
endif (WIN32)

add_library(leveldb_utils ${LEVELDB_UTILS})
target_compile_definitions(leveldb_utils
    PRIVATE
        # used by port/port.h         
        ${LEVELDB_PLATFORM_NAME}=1
)