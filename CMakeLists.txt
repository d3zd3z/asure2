# CMake descriptor for asure2.

cmake_minimum_required(VERSION 2.6)

project(asure)

set(ASURE_VERSION_MAJOR 2)
set(ASURE_VERSION_MINOR 0)
set(ASURE_VERSION_SUB 0)

find_package(ZLIB REQUIRED)
link_libraries(${ZLIB_LIBRARIES})
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(PkgConfig)

include_directories(boost)
include_directories(lib)

# Warn aggressively.
add_definitions(-Wall -Wextra)
add_definitions(-Werror)

# Enable optimization, even with debugging.  This can be turned off
# for faster compiles, but is necessary to get some warnings (such as
# unused vars.
# add_definitions(-O)

add_subdirectory(lib)
add_subdirectory(main)
