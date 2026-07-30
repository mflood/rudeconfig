add_executable(test_roundtrip test_roundtrip.cpp)
target_link_libraries(test_roundtrip PRIVATE rudeconfig::rudeconfig)
add_test(NAME roundtrip COMMAND test_roundtrip)

add_executable(test_parse test_parse.cpp)
target_link_libraries(test_parse PRIVATE rudeconfig::rudeconfig)
add_test(NAME parse COMMAND test_parse)

# Each test runs in its own scratch dir so file fixtures don't collide.
set_tests_properties(roundtrip parse PROPERTIES TIMEOUT 60)
foreach(t roundtrip parse)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/run-${t}")
    set_tests_properties(${t} PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/run-${t}")
endforeach()
