cmake_minimum_required(VERSION 3.16)

project(rudeconfig
    VERSION 5.1.0
    DESCRIPTION "C++ library for reading and writing configuration/.ini files"
    HOMEPAGE_URL "https://github.com/mflood/rudeconfig"
    LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

option(RUDECONFIG_BUILD_EXAMPLES "Build example programs" ON)

add_library(rudeconfig
    src/AbstractData.cpp
    src/AbstractOrganiser.cpp
    src/AbstractParser.cpp
    src/AbstractWriter.cpp
    src/Base64Encoder.cpp
    src/Comment.cpp
    src/ConfigImpl.cpp
    src/DataLine.cpp
    src/File.cpp
    src/KeyValue.cpp
    src/ParserJuly2004.cpp
    src/RealOrganiser.cpp
    src/Section.cpp
    src/WhiteSpace.cpp
    src/Writer.cpp
    src/config.cpp)

add_library(rudeconfig::rudeconfig ALIAS rudeconfig)

target_compile_features(rudeconfig PUBLIC cxx_std_17)

# Public header is consumed as <rude/config.h>; stage it under that path
# so in-tree consumers see the same layout as installed ones.
configure_file(src/config.h "${CMAKE_CURRENT_BINARY_DIR}/include/rude/config.h" COPYONLY)
target_include_directories(rudeconfig PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# Shared-library version follows the old libtool -version-info 5:1:2 lineage.
set_target_properties(rudeconfig PROPERTIES
    VERSION 3.2.2
    SOVERSION 3
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(MSVC)
    target_compile_options(rudeconfig PRIVATE /W4)
else()
    target_compile_options(rudeconfig PRIVATE -Wall -Wextra)
endif()

# ---- Install ----

install(TARGETS rudeconfig
    EXPORT rudeconfig-targets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(FILES src/config.h
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rude")

install(FILES man3/rudeconfig.3
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man3")

install(EXPORT rudeconfig-targets
    FILE rudeconfig-targets.cmake
    NAMESPACE rudeconfig::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/rudeconfig")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/rudeconfig-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rudeconfig-config.cmake"
    "include(\"\${CMAKE_CURRENT_LIST_DIR}/rudeconfig-targets.cmake\")\n")
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/rudeconfig-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/rudeconfig-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/rudeconfig")

configure_file(rudeconfig.pc.in "${CMAKE_CURRENT_BINARY_DIR}/rudeconfig.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/rudeconfig.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# ---- Tests & examples ----

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(RUDECONFIG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
