# src/interface/CMakeLists.txt
#
# TrussInterface — public API layer (Facade + Builder patterns).
# Provides the stable, simplified API consumed by CLI commands, GUI
# controllers, and external integrations.  Depends on TrussApplication.

add_library(TrussInterface STATIC
    truss_builder.cpp
    truss_builder.hpp
    truss_analysis_facade.cpp
    truss_analysis_facade.hpp
    facade_truss_service_adapter.cpp
    facade_truss_service_adapter.hpp
    facade_analysis_service_adapter.cpp
    facade_analysis_service_adapter.hpp
)

# include/ contains the public umbrella header (truss.hpp) and the
# truss/ sub-directory with interface contracts.
target_include_directories(TrussInterface
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/truss>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(TrussInterface
    PUBLIC
        TrussApplication
)

target_compile_features(TrussInterface PUBLIC cxx_std_20)

set_target_properties(TrussInterface PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
