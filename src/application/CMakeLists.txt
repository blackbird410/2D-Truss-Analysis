# src/application/CMakeLists.txt
#
# TrussApplication — application layer (orchestration facades).
# Provides high-level services consumed by the Interface layer and GUI/CLI.
# Depends on both TrussCore (domain) and TrussInfrastructure (I/O, export).

add_library(TrussApplication STATIC
    truss_application_service.cpp
    truss_application_service.hpp
    analysis_application_service.cpp
    analysis_application_service.hpp
    material_library_service.cpp
    material_library_service.hpp
)

# src/ is already PUBLIC on TrussCore and propagates transitively, so
# relative imports like "../core/..." work without additional declarations.
# Expose the application directory itself for consumers that include
# "application/truss_application_service.hpp".
target_include_directories(TrussApplication
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(TrussApplication
    PUBLIC
        TrussCore
        TrussInfrastructure
)

target_compile_features(TrussApplication PUBLIC cxx_std_20)

set_target_properties(TrussApplication PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
