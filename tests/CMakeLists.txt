add_executable(test_roundtrip test_roundtrip.cpp)
target_link_libraries(test_roundtrip PRIVATE rudeconfig::rudeconfig)
add_test(NAME roundtrip COMMAND test_roundtrip)

add_executable(test_parse test_parse.cpp)
target_link_libraries(test_parse PRIVATE rudeconfig::rudeconfig)
add_test(NAME parse COMMAND test_parse)

add_executable(test_version test_version.cpp)
target_link_libraries(test_version PRIVATE rudeconfig::rudeconfig)
target_compile_definitions(test_version PRIVATE EXPECTED_VERSION="${PROJECT_VERSION}")
add_test(NAME version COMMAND test_version)

add_executable(test_roundtrip_fidelity test_roundtrip_fidelity.cpp)
target_link_libraries(test_roundtrip_fidelity PRIVATE rudeconfig::rudeconfig)
add_test(NAME roundtrip_fidelity COMMAND test_roundtrip_fidelity)

add_executable(test_contract test_contract.cpp)
target_link_libraries(test_contract PRIVATE rudeconfig::rudeconfig)
add_test(NAME contract COMMAND test_contract)

add_executable(test_enumeration test_enumeration.cpp)
target_link_libraries(test_enumeration PRIVATE rudeconfig::rudeconfig)
add_test(NAME enumeration COMMAND test_enumeration)

# Each test runs in its own scratch dir so file fixtures don't collide.
set_tests_properties(roundtrip parse enumeration version roundtrip_fidelity contract PROPERTIES TIMEOUT 60)
foreach(t roundtrip parse enumeration version roundtrip_fidelity contract)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/run-${t}")
    set_tests_properties(${t} PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/run-${t}")
endforeach()

# Guards the 5.1.1 pkg-config fix: rudeconfig.pc must not hardcode the
# configure-time prefix, or "cmake --install --prefix <dir>" ships a .pc
# pointing somewhere else entirely (worst case, an older system copy).
add_test(NAME pkgconfig_relocatable
    COMMAND "${CMAKE_COMMAND}"
        "-DPC_FILE=${CMAKE_BINARY_DIR}/rudeconfig.pc"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_pc_relocatable.cmake")
