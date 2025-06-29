# Core module CMakeLists.txt

add_library(TrussCore STATIC
    Application.cpp
    Application.hpp
    Logger.cpp
    Logger.hpp
)

target_include_directories(TrussCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(TrussCore PUBLIC
    Qt6::Core
)

# Set target properties
set_target_properties(TrussCore PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
