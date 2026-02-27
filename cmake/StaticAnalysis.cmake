# Static Analysis Configuration
# Provides options and configuration for static analysis tools

function(enable_static_analysis target_name)
    option(ENABLE_CPPCHECK "Run cppcheck during build" OFF)
    option(ENABLE_CLANG_TIDY "Run clang-tidy during build" OFF)
    
    if(ENABLE_CPPCHECK)
        find_program(CPPCHECK_EXECUTABLE cppcheck)
        if(CPPCHECK_EXECUTABLE)
            set(CPPCHECK_ARGS
                --enable=all
                --suppress=missingIncludeSystem
                --error-exitcode=1
            )
            set_target_properties(${target_name} PROPERTIES
                CXX_CPPCHECK "${CPPCHECK_EXECUTABLE};${CPPCHECK_ARGS}"
            )
            message(STATUS "cppcheck enabled for ${target_name}")
        else()
            message(WARNING "cppcheck not found, skipping static analysis")
        endif()
    endif()
    
    if(ENABLE_CLANG_TIDY)
        find_program(CLANG_TIDY_EXECUTABLE clang-tidy)
        if(CLANG_TIDY_EXECUTABLE)
            set(CLANG_TIDY_ARGS
                -checks=readability-*,performance-*,bugprone-*
            )
            set_target_properties(${target_name} PROPERTIES
                CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};${CLANG_TIDY_ARGS}"
            )
            message(STATUS "clang-tidy enabled for ${target_name}")
        else()
            message(WARNING "clang-tidy not found, skipping static analysis")
        endif()
    endif()
endfunction()
