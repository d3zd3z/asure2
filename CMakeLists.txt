# CMake descriptor for asure2.

cmake_minimum_required(VERSION 2.6)

project(asure)

set(ASURE_VERSION_MAJOR 2)
set(ASURE_VERSION_MINOR 0)
set(ASURE_VERSION_SUB 0)

find_path(BLORT stdio.h)
message(${BLORT})

# This shouldn't be needed, but boost doesn't seem to always find it.
find_package(BZip2 REQUIRED)
link_libraries(${BZIP2_LIBRARIES})

find_library(RT_LIBRARY rt)
link_libraries(${RT_LIBRARY})

find_library(PTHREAD_LIBRARY pthread)
link_libraries(${PTHREAD_LIBRARY})

include(CheckIncludeFiles)
find_package(Boost 1.41 REQUIRED iostreams unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(PkgConfig)

pkg_check_modules(OPENSSL REQUIRED openssl)

pkg_check_modules(UUID REQUIRED uuid)

include_directories(lib)

# Warn aggressively.
add_definitions(-Wall -Wextra)
add_definitions(-Werror)

add_subdirectory(lib)
add_subdirectory(main)
add_subdirectory(test)
