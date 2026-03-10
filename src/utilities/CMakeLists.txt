# src/utilities/CMakeLists.txt
#
# TrussUtils — pure utility library with no project dependencies.
# Safe to use from every layer in the dependency hierarchy.

add_library(TrussUtils STATIC
    string_utils.cpp
    string_utils.hpp
    math_utils.hpp
)

# Expose src/ so consumers can use "utilities/string_utils.hpp"
target_include_directories(TrussUtils
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(TrussUtils PUBLIC cxx_std_20)

set_target_properties(TrussUtils PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
