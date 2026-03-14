# src/core/CMakeLists.txt
#
# TrussCore — pure domain layer.
# Contains the domain model (nodes, members, loads, trusses), assembly
# utilities, validation services, and the analysis engine.
# No infrastructure or application code belongs here.

add_library(TrussCore STATIC
    application.hpp

    # Domain model
    model/types.hpp
    model/node.cpp
    model/node.hpp
    model/member.cpp
    model/member.hpp
    model/truss.cpp
    model/truss.hpp
    model/load.cpp
    model/load.hpp

    # Domain assembly utilities
    assembly/truss_assembler.cpp
    assembly/truss_assembler.hpp

    # Domain validation services
    validation/truss_validator.cpp
    validation/truss_validator.hpp

    # Analysis engine
    analysis/stiffness_assembler.cpp
    analysis/stiffness_assembler.hpp
    analysis/boundary_condition_handler.cpp
    analysis/boundary_condition_handler.hpp
    analysis/ilinear_solver.hpp
    analysis/direct_solver.cpp
    analysis/direct_solver.hpp
    analysis/iterative_solver.cpp
    analysis/iterative_solver.hpp
    analysis/solver_factory.cpp
    analysis/solver_factory.hpp
    analysis/analysis_orchestrator.cpp
    analysis/analysis_orchestrator.hpp
)

# PUBLIC: consumers need these paths to include core headers.
# CMAKE_CURRENT_SOURCE_DIR (src/core/) is exposed so that includes like
# "model/node.hpp" resolve from within tests and other consumers.
target_include_directories(TrussCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/truss>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(TrussCore
    PUBLIC
        TrussUtils
        Eigen3::Eigen
        nlohmann_json::nlohmann_json
)

target_compile_features(TrussCore PUBLIC cxx_std_20)

set_target_properties(TrussCore PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
