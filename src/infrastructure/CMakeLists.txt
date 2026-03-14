# src/infrastructure/CMakeLists.txt
#
# TrussInfrastructure — infrastructure layer.
# Contains export formatters, file I/O adapters, logging, and console
# output adapters.  Depends on TrussCore (for domain types used in
# serialisation and export) but carries no application or GUI code.

add_library(TrussInfrastructure STATIC
    # Export subsystem
    export/export_types.hpp
    export/exporter.hpp
    export/exporter_factory.cpp
    export/exporter_factory.hpp
    export/csv_exporter.cpp
    export/csv_exporter.hpp
    export/json_exporter.cpp
    export/json_exporter.hpp
    export/xml_exporter.cpp
    export/xml_exporter.hpp
    export/html_exporter.cpp
    export/html_exporter.hpp
    export/latex_exporter.cpp
    export/latex_exporter.hpp
    export/text_exporter.cpp
    export/text_exporter.hpp

    # Logging subsystem
    logging/logger.hpp
    logging/console_logger.cpp
    logging/console_logger.hpp
    logging/file_logger.cpp
    logging/file_logger.hpp
    logging/logger_factory.cpp
    logging/logger_factory.hpp

    # Console output adapter
    adapters/console_output_adapter.cpp
    adapters/console_output_adapter.hpp

    # File I/O subsystem
    io/io_types.hpp
    io/truss_reader.hpp
    io/truss_writer.hpp
    io/json_truss_reader.cpp
    io/json_truss_reader.hpp
    io/json_truss_writer.cpp
    io/json_truss_writer.hpp
    io/xml_truss_reader.cpp
    io/xml_truss_reader.hpp
    io/xml_truss_writer.cpp
    io/xml_truss_writer.hpp
    io/support_type_serializer.cpp
    io/support_type_serializer.hpp
    io/fileio_factory.cpp
    io/fileio_factory.hpp
)

# PUBLIC: expose infrastructure sub-directory headers to consumers.
# src/infrastructure/export etc. allow include patterns such as
# #include "exporter_factory.hpp" used in some consumer source files.
target_include_directories(TrussInfrastructure
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/export>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logging>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/io>
        $<INSTALL_INTERFACE:include>
)

# tinyxml2 is an implementation detail of the XML exporters/readers;
# consumers do not include tinyxml2 headers directly, so PRIVATE suffices.
# TrussCore (and its transitive deps nlohmann_json, Eigen3) are already
# PUBLIC on TrussCore and propagate transitively.
target_link_libraries(TrussInfrastructure
    PUBLIC
        TrussCore
    PRIVATE
        tinyxml2::tinyxml2
)

target_compile_features(TrussInfrastructure PUBLIC cxx_std_20)

set_target_properties(TrussInfrastructure PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
