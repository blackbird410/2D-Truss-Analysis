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
    # Phase 2 - Analysis decomposition components in analysis/ subdirectory
    analysis/StiffnessAssembler.cpp
    analysis/StiffnessAssembler.hpp
    analysis/BoundaryConditionHandler.cpp
    analysis/BoundaryConditionHandler.hpp
    analysis/ILinearSolver.hpp
    analysis/DirectSolver.cpp
    analysis/DirectSolver.hpp
    analysis/IterativeSolver.cpp
    analysis/IterativeSolver.hpp
    analysis/SolverFactory.cpp
    analysis/SolverFactory.hpp
    analysis/AnalysisOrchestrator.cpp
    analysis/AnalysisOrchestrator.hpp
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
