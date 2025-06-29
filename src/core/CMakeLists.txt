# Core module CMakeLists.txt

add_library(TrussCore STATIC
    Application.cpp
    Application.hpp
    Logger.cpp
    Logger.hpp
    Types.hpp
    Node.cpp
    Node.hpp
    Member.cpp
    Member.hpp
    Truss.hpp
    AnalysisEngine.cpp
    AnalysisEngine.hpp
)

target_include_directories(TrussCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(TrussCore PUBLIC
    Qt6::Core
    Eigen3::Eigen
)

# Set target properties
set_target_properties(TrussCore PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
