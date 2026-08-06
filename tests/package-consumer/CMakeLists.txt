cmake_minimum_required(VERSION 3.16)

project(rudeconfig_package_consumer LANGUAGES CXX)

find_package(rudeconfig 6.1 REQUIRED CONFIG)

add_executable(cmake_consumer main.cpp)
target_link_libraries(cmake_consumer PRIVATE rudeconfig::rudeconfig)
target_compile_features(cmake_consumer PRIVATE cxx_std_17)

if(RUDECONFIG_TEST_PKGCONFIG)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(rudeconfig_pc REQUIRED IMPORTED_TARGET rudeconfig>=6.1.0)

    add_executable(pkgconfig_consumer main.cpp)
    target_link_libraries(pkgconfig_consumer PRIVATE PkgConfig::rudeconfig_pc)
    target_compile_features(pkgconfig_consumer PRIVATE cxx_std_17)
endif()
