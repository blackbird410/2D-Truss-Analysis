# cmake/PrintVersion.cmake
# Prints the project version declared in the root CMakeLists.txt.
# Usage: cmake -P cmake/PrintVersion.cmake
#
# This script is intentionally standalone (no configure step required) so
# that packaging scripts can query the version without a full CMake run.

cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_LIST_DIR}/../CMakeLists.txt" _cmakelists)
string(REGEX MATCH
    "project\\([^)]*VERSION[ \t]+([0-9]+\\.[0-9]+\\.[0-9]+[^) \t]*)"
    _match "${_cmakelists}")

if(CMAKE_MATCH_1)
    message("${CMAKE_MATCH_1}")
else()
    message(FATAL_ERROR "Could not extract project version from CMakeLists.txt")
endif()
