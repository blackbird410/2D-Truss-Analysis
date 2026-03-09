# cmake/InstallRules.cmake
#
# FHS-compliant installation layout for 2D Truss Analysis.
#
# Included by the top-level CMakeLists.txt AFTER all targets are defined and
# AFTER include(GNUInstallDirs).  Rules execute at include-time (no function
# wrapper) so they are always active once this file is processed.
#
# With CMAKE_INSTALL_PREFIX=/usr the layout is:
#   /usr/bin/truss-analysis-cli
#   /usr/bin/truss-analysis              (GUI, if BUILD_GUI)
#   /usr/share/applications/truss-analysis.desktop
#   /usr/share/icons/hicolor/scalable/apps/truss-analysis.svg
#   /usr/share/metainfo/io.github.blackbird410.TrussAnalysis.metainfo.xml
#   /usr/share/doc/2d-truss-analysis/README.md
#   /usr/share/doc/2d-truss-analysis/copyright
#   /usr/share/truss-analysis/examples/
#   /etc/truss-analysis/default.json.example
#   /etc/truss-analysis/logging.json.example

# ============================================================================
# Binaries
# TrussCore, TrussApplication, TrussInterface, TrussUtils are static libraries
# fully linked into the executables at build time.  They are NOT installed.
# ============================================================================

install(TARGETS TrussAnalysisCLI
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT cli
)

if(BUILD_GUI)
    install(TARGETS TrussAnalysisGUI
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT gui
    )

    # ------------------------------------------------------------------
    # Desktop integration (GUI only)
    # ------------------------------------------------------------------

    # XDG application entry — registers the app in GNOME, KDE, etc.
    install(FILES "${CMAKE_SOURCE_DIR}/packaging/linux/truss-analysis.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
        COMPONENT gui
    )

    # Scalable application icon in the hicolor theme
    install(FILES "${CMAKE_SOURCE_DIR}/packaging/linux/icons/truss-analysis.svg"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
        COMPONENT gui
    )

    # AppStream metainfo — enables discovery in GNOME Software / KDE Discover
    install(FILES
        "${CMAKE_SOURCE_DIR}/packaging/linux/io.github.blackbird410.TrussAnalysis.metainfo.xml"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo"
        COMPONENT gui
    )
endif()

# ============================================================================
# Documentation
# Debian Policy requires a 'copyright' file under /usr/share/doc/<pkg>/.
# CMAKE_INSTALL_DOCDIR expands to share/doc/2d-truss-analysis when
# CPACK_PACKAGE_NAME is set before include(GNUInstallDirs).
# ============================================================================

install(FILES
    "${CMAKE_SOURCE_DIR}/README.md"
    "${CMAKE_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    COMPONENT common
)

# Debian-format copyright file (mirrors LICENSE content under the required name)
if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/debian/copyright")
    install(FILES "${CMAKE_SOURCE_DIR}/packaging/debian/copyright"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT common
    )
endif()

# ============================================================================
# System-level configuration templates
# Installed as *.example so that package upgrades never overwrite user edits.
# Applications should read from $XDG_CONFIG_HOME/truss-analysis/ first,
# falling back to /etc/truss-analysis/ for system defaults.
# ============================================================================

install(FILES "${CMAKE_SOURCE_DIR}/config/default.json"
    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/truss-analysis"
    RENAME "default.json.example"
    COMPONENT common
)

install(FILES "${CMAKE_SOURCE_DIR}/config/logging.json"
    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/truss-analysis"
    RENAME "logging.json.example"
    COMPONENT common
)

# ============================================================================
# Example project files
# ============================================================================

install(DIRECTORY "${CMAKE_SOURCE_DIR}/examples/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/truss-analysis/examples"
    COMPONENT common
    FILES_MATCHING
    PATTERN "*.json"
    PATTERN "*.xml"
    PATTERN "README.md"
)
