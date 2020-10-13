set(LEVELDB_UTILS
    arena.cc
    coding.cc
    hash.cc
    filter_policy.cc
    bloom.cc)

add_library(leveldb_utils ${LEVELDB_UTILS})