cmake_minimum_required(VERSION 3.20)
project(TrussAnalysis VERSION 2.2.0 LANGUAGES CXX)

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
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

# Find required packages
find_package(Eigen3 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt6 automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create library for core functionality
add_library(TrussCore STATIC
    src/core/Logger.cpp
    src/core/Logger.hpp
    src/core/Types.hpp
    src/core/Node.cpp
    src/core/Node.hpp
    src/core/Member.cpp
    src/core/Member.hpp
    src/core/Truss.cpp
    src/core/Truss.hpp
    src/core/AnalysisEngine.cpp
    src/core/AnalysisEngine.hpp
    src/core/ResultsExporter.cpp
    src/core/ResultsExporter.hpp
    src/core/Application.cpp
    src/core/Application.hpp
)

target_include_directories(TrussCore PUBLIC
    src/core
)

target_link_libraries(TrussCore PUBLIC
    Eigen3::Eigen
)

# Set target properties for library
set_target_properties(TrussCore PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Main application CLI executable
add_executable(TrussAnalysisCLI
    src/main_app.cpp
)

target_link_libraries(TrussAnalysisCLI PRIVATE
    TrussCore
)

# Set target properties for CLI executable
set_target_properties(TrussAnalysisCLI PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "TrussAnalysisCLI"
)

# GUI executable
add_executable(TrussAnalysisGUI
    src/gui/main.cpp
    src/gui/MainWindow.cpp
    src/gui/NodeInputWidget.cpp
    src/gui/MemberInputWidget.cpp
    src/gui/LoadInputWidget.cpp
    src/gui/DataTableWidget.cpp
    src/gui/ResultsWidget.cpp
    src/gui/PlotWidget.cpp
    src/gui/InteractiveDrawingWidget.cpp
    src/gui/ProjectFileManager.cpp
    src/gui/DeformedTrussWidget.cpp
)

target_link_libraries(TrussAnalysisGUI PRIVATE
    TrussCore
    Qt6::Core
    Qt6::Widgets
)

# Set target properties for GUI executable
set_target_properties(TrussAnalysisGUI PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "TrussAnalysisGUI"
)

# Install rules for packaging
install(TARGETS TrussAnalysisCLI TrussAnalysisGUI
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Platform-specific settings
if(APPLE)
    # macOS Bundle settings
    set_target_properties(TrussAnalysisGUI PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in
        MACOSX_BUNDLE_BUNDLE_NAME "2D Truss Analysis"
        MACOSX_BUNDLE_DISPLAY_NAME "2D Truss Analysis"
        MACOSX_BUNDLE_IDENTIFIER "com.civilengsoft.trussanalysis"
        MACOSX_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "© 2024 Civil Engineering Software Solutions"
    )
    
    # Code signing for macOS
    find_program(CODESIGN_EXECUTABLE codesign)
    if(CODESIGN_EXECUTABLE)
        # Use ad-hoc signing (-) which doesn't require a certificate but allows local execution
        add_custom_command(TARGET TrussAnalysisGUI POST_BUILD
            COMMAND ${CODESIGN_EXECUTABLE} --force --deep --sign - "$<TARGET_BUNDLE_DIR:TrussAnalysisGUI>"
            COMMENT "Code signing TrussAnalysisGUI with ad-hoc signature"
            VERBATIM
        )
        
        # Also sign the CLI executable
        add_custom_command(TARGET TrussAnalysisCLI POST_BUILD
            COMMAND ${CODESIGN_EXECUTABLE} --force --sign - "$<TARGET_FILE:TrussAnalysisCLI>"
            COMMENT "Code signing TrussAnalysisCLI with ad-hoc signature"
            VERBATIM
        )
        
        message(STATUS "Code signing enabled with ad-hoc signatures")
    else()
        message(WARNING "codesign not found - applications will not be signed")
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    message(STATUS "Configuring for Linux build")
    
    # Set RPATH for better library linking
    set_target_properties(TrussAnalysisGUI PROPERTIES
        INSTALL_RPATH "\$ORIGIN/../lib;\$ORIGIN/lib"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH ON
    )
    
    set_target_properties(TrussAnalysisCLI PROPERTIES
        INSTALL_RPATH "\$ORIGIN/../lib;\$ORIGIN/lib"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH ON
    )
    
    # Strip debug symbols in release builds for smaller binaries
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET TrussAnalysisGUI POST_BUILD
            COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:TrussAnalysisGUI>"
            COMMENT "Stripping debug symbols from TrussAnalysisGUI"
        )
        
        add_custom_command(TARGET TrussAnalysisCLI POST_BUILD
            COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:TrussAnalysisCLI>"
            COMMENT "Stripping debug symbols from TrussAnalysisCLI"
        )
    endif()
    
    message(STATUS "Linux optimizations enabled")
elseif(WIN32)
    # Windows-specific settings
    message(STATUS "Configuring for Windows build")
    
    # Set Windows-specific properties
    set_target_properties(TrussAnalysisGUI PROPERTIES
        WIN32_EXECUTABLE TRUE
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
    # Enable modern Windows features
    target_compile_definitions(TrussAnalysisGUI PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0A00  # Windows 10
    )
    
    target_compile_definitions(TrussAnalysisCLI PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0A00  # Windows 10
    )
    
    # Set up proper DLL copying for Qt
    if(Qt6_FOUND)
        # Find Qt6 installation path
        get_target_property(QT6_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(QT6_WINDEPLOYQT_EXECUTABLE ${QT6_QMAKE_EXECUTABLE} PATH)
        set(QT6_WINDEPLOYQT_EXECUTABLE "${QT6_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")
        
        if(EXISTS ${QT6_WINDEPLOYQT_EXECUTABLE})
            # Deploy Qt libraries after build
            add_custom_command(TARGET TrussAnalysisGUI POST_BUILD
                COMMAND ${QT6_WINDEPLOYQT_EXECUTABLE} --debug --compiler-runtime $<TARGET_FILE:TrussAnalysisGUI>
                COMMENT "Deploying Qt libraries for TrussAnalysisGUI")
        endif()
    endif()
    
    message(STATUS "Windows optimizations enabled")
endif()

# CPack configuration for DMG creation
if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_FORMAT "UDBZ")
    set(CPACK_DMG_VOLUME_NAME "2D Truss Analysis")
    set(CPACK_PACKAGE_NAME "2D_Truss_Analysis")
    set(CPACK_PACKAGE_VENDOR "Civil Engineering Software Solutions")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Professional 2D truss structural analysis software")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
    
    # DMG background and appearance (optional)
    # set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/assets/dmg_background.png")
    # set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_SOURCE_DIR}/assets/dmg_setup.scpt")
    
    include(CPack)
endif()
