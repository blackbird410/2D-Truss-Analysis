cmake_minimum_required(VERSION 3.20)
project(TrussAnalysis2D 
    VERSION 2.0.0
    DESCRIPTION "Professional 2D Truss Analysis Application"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Enable testing
enable_testing()

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts OpenGL OpenGLWidgets)
find_package(Eigen3 REQUIRED)
find_package(SQLite3 REQUIRED)

# Optional packages for enhanced functionality
find_package(Catch2 3 QUIET)
find_package(spdlog QUIET)

# Set Qt6 MOC settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create library for core functionality
add_subdirectory(src/core)
add_subdirectory(src/gui)
add_subdirectory(src/database)
add_subdirectory(src/analysis)
add_subdirectory(src/visualization)

# Main executable
add_executable(TrussAnalysis2D
    src/main.cpp
)

# Link libraries to main executable
target_link_libraries(TrussAnalysis2D PRIVATE
    TrussCore
    TrussGUI
    TrussDatabase
    TrussAnalysis
    TrussVisualization
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Eigen3::Eigen
    SQLite::SQLite3
)

# Set target properties
set_target_properties(TrussAnalysis2D PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist.in
)

# Add tests if Catch2 is available
if(Catch2_FOUND)
    add_subdirectory(tests)
endif()

# Installation rules
install(TARGETS TrussAnalysis2D
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Package configuration
set(CPACK_PACKAGE_NAME "TrussAnalysis2D")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Civil Engineering Software Solutions")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
else()
    set(CPACK_GENERATOR "DEB;RPM")
endif()

include(CPack)
