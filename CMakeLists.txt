# CMake descriptor for asure2.

cmake_minimum_required(VERSION 2.6)

project(asure)

set(ASURE_VERSION_MAJOR 2)
set(ASURE_VERSION_MINOR 0)
set(ASURE_VERSION_SUB 0)

include(CheckIncludeFiles)
find_package(Boost 1.41 REQUIRED iostreams unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(PkgConfig)

pkg_check_modules(OPENSSL REQUIRED openssl)
include_directories(${OPENSSL_INCLUDE_DIRS})

pkg_check_modules(UUID REQUIRED uuid)
include_directories(${UUID_INCLUDE_DIRS})

include_directories(lib)

add_subdirectory(lib)
add_subdirectory(main)
add_subdirectory(test)
