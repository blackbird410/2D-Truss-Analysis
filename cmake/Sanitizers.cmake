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
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
        message(STATUS "Address sanitizer enabled for ${target_name}")
    endif()
    
    if(ENABLE_SANITIZER_MEMORY)
        add_compile_options(-fsanitize=memory)
        add_link_options(-fsanitize=memory)
        message(STATUS "Memory sanitizer enabled for ${target_name}")
    endif()
    
    if(ENABLE_SANITIZER_THREAD)
        add_compile_options(-fsanitize=thread)
        add_link_options(-fsanitize=thread)
        message(STATUS "Thread sanitizer enabled for ${target_name}")
    endif()
    
    if(ENABLE_SANITIZER_UNDEFINED)
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)
        message(STATUS "Undefined behavior sanitizer enabled for ${target_name}")
    endif()
endfunction()
