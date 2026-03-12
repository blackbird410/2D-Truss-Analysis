# cmake/CleanGcda.cmake
#
# Invoked by CTest as a FIXTURES_SETUP script (see tests/CMakeLists.txt).
# Deletes all stale .gcda coverage-counter files from the build tree before
# any test binary runs.
#
# ## Why this is necessary
#
# gcov stores per-source-file arc counters in .gcda files inside the build
# tree.  Each binary embeds a checksum of its source structure;  when a
# binary is rebuilt (even a minor source change), the checksum changes.  If
# a .gcda file from the previous build is still present, the gcov runtime
# detects the mismatch and calls gcov_error() → abort() with the message
# "corrupt arc tag".  CTest surfaces this as "Exception: SegFault".
#
# Deleting stale files before the test run forces the gcov runtime to create
# fresh .gcda files that match the current binary, avoiding the abort().
# Coverage totals are not lost: the files are regenerated on the next run.
#
# ## Invocation
#
# The script receives BUILD_DIR as a -D variable from the add_test() call:
#
#   cmake -DBUILD_DIR=<cmake_binary_dir> -P cmake/CleanGcda.cmake
#
# Usage outside CTest (manual clean):
#
#   cmake -DBUILD_DIR=$(pwd)/build -P cmake/CleanGcda.cmake

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED BUILD_DIR OR BUILD_DIR STREQUAL "")
    message(FATAL_ERROR "CleanGcda.cmake: BUILD_DIR must be specified via -DBUILD_DIR=<path>")
endif()

file(GLOB_RECURSE _gcda_files
    LIST_DIRECTORIES FALSE
    "${BUILD_DIR}/*.gcda"
)

list(LENGTH _gcda_files _count)
message(STATUS "CleanGcda: removing ${_count} stale .gcda file(s) from ${BUILD_DIR}")

if(_count GREATER 0)
    foreach(_f IN LISTS _gcda_files)
        file(REMOVE "${_f}")
    endforeach()
endif()
