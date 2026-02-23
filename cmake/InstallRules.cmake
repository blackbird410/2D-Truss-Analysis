# Installation Rules Configuration
# Defines how the project should be packaged and installed

function(setup_installation)
    # Install header files
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
            DESTINATION include
            FILES_MATCHING PATTERN "*.hpp"
    )
    
    # Install libraries
    install(TARGETS TrussCore TrussApplication
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
    )
    
    # Install executables
    if(BUILD_GUI)
        install(TARGETS TrussAnalysisGUI
                RUNTIME DESTINATION bin
        )
    endif()
    
    install(TARGETS TrussAnalysisCLI
            RUNTIME DESTINATION bin
    )
    
    # Install documentation
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/docs/"
            DESTINATION share/doc/truss-analysis
            PATTERN ".md"
    )
    
    # Install configuration files
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/config/"
            DESTINATION etc/truss-analysis
    )
    
    # Install examples
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/examples/"
            DESTINATION share/truss-analysis/examples
    )
    
    # CPack configuration
    set(CPACK_PROJECT_NAME "TrussAnalysis")
    set(CPACK_PROJECT_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "2D Truss Analysis Tool")
    set(CPACK_PACKAGE_VENDOR "Civil Engineering Software")
    set(CPACK_PACKAGE_CONTACT "support@civilengsoft.com")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
    
    include(CPack)
endfunction()
