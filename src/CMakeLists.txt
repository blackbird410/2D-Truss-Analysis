# src/CMakeLists.txt
#
# Top-level source orchestrator.
# Adds each library module in dependency order so that every target is
# defined before the targets that depend on it.
# The root CMakeLists.txt calls add_subdirectory(src); all library and
# executable targets are defined here, not in the root file.

add_subdirectory(utilities)
add_subdirectory(core)
add_subdirectory(infrastructure)
add_subdirectory(application)
add_subdirectory(interface)
add_subdirectory(cli)

if(BUILD_GUI)
    add_subdirectory(gui)
endif()
