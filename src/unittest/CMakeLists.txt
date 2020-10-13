add_executable(arena_test arena_test.cc)
target_link_libraries(arena_test leveldb_utils)
add_test(NAME arena_test COMMAND arena_test)

add_executable(coding_test coding_test.cc)
target_link_libraries(coding_test leveldb_utils)
add_test(NAME coding_test COMMAND coding_test)

add_executable(bloom_test bloom_test.cc)
target_link_libraries(bloom_test leveldb_utils)
add_test(NAME bloom_test COMMAND bloom_test)
