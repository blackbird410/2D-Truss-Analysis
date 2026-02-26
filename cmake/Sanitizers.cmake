# Sanitizer Configuration
# Provides options for enabling various runtime sanitizers

function(enable_sanitizers target_name)
    option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(ENABLE_SANITIZER_UNDEFINED "Enable undefined behavior sanitizer" OFF)
    
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Sanitizers are only supported with GCC/Clang")
        return()
    endif()
    
    if(ENABLE_SANITIZER_ADDRESS)
        target_compile_options(${target_name} PRIVATE -fsanitize=address)
        target_link_options(${target_name} PRIVATE -fsanitize=address)
        message(STATUS "Address sanitizer enabled for ${target_name}")
    endif()
    
    if(ENABLE_SANITIZER_MEMORY)
        target_compile_options(${target_name} PRIVATE -fsanitize=memory)
        target_link_options(${target_name} PRIVATE -fsanitize=memory)
        message(STATUS "Memory sanitizer enabled for ${target_name}")
    endif()
    
    if(ENABLE_SANITIZER_THREAD)
        target_compile_options(${target_name} PRIVATE -fsanitize=thread)
        target_link_options(${target_name} PRIVATE -fsanitize=thread)
        message(STATUS "Thread sanitizer enabled for ${target_name}")
    endif()
    
    if(ENABLE_SANITIZER_UNDEFINED)
        target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
        target_link_options(${target_name} PRIVATE -fsanitize=undefined)
        message(STATUS "Undefined behavior sanitizer enabled for ${target_name}")
    endif()
endfunction()
