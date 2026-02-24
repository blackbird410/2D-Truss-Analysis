/**
 * @file fileio_factory.cpp
 * @brief Implements the file I/O factory.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "fileio_factory.hpp"

#include "json_truss_reader.hpp"
#include "json_truss_writer.hpp"
#include "xml_truss_reader.hpp"
#include "xml_truss_writer.hpp"

#include <stdexcept>

namespace truss::infrastructure::io {

std::unique_ptr<ITrussReader> FileIOFactory::createReader(FileFormat format) {
    switch (format) {
        case FileFormat::JSON:
            return std::make_unique<JsonTrussReader>();

        case FileFormat::XML:
            return std::make_unique<XmlTrussReader>();

        case FileFormat::Auto:
            throw std::invalid_argument("Cannot create reader with Auto format. "
                                        "Use createReader(filepath) for auto-detection.");

        default:
            throw std::invalid_argument("Unsupported file format");
    }
}

std::unique_ptr<ITrussReader> FileIOFactory::createReader(const std::filesystem::path& filepath) {
    FileFormat format = detectFormat(filepath);

    if (format == FileFormat::Auto) {
        throw std::invalid_argument("Cannot detect file format from extension: " +
                                    filepath.extension().string());
    }

    return createReader(format);
}

std::unique_ptr<ITrussWriter> FileIOFactory::createWriter(FileFormat format) {
    switch (format) {
        case FileFormat::JSON:
            return std::make_unique<JsonTrussWriter>();

        case FileFormat::XML:
            return std::make_unique<XmlTrussWriter>();

        case FileFormat::Auto:
            throw std::invalid_argument("Cannot create writer with Auto format. "
                                        "Use createWriter(filepath) for auto-detection.");

        default:
            throw std::invalid_argument("Unsupported file format");
    }
}

std::unique_ptr<ITrussWriter> FileIOFactory::createWriter(const std::filesystem::path& filepath) {
    FileFormat format = detectFormat(filepath);

    if (format == FileFormat::Auto) {
        throw std::invalid_argument("Cannot detect file format from extension: " +
                                    filepath.extension().string());
    }

    return createWriter(format);
}

[[maybe_unused]] bool FileIOFactory::isFormatSupported(FileFormat format) {
    return format == FileFormat::JSON || format == FileFormat::XML;
}

FileFormat FileIOFactory::detectFormat(const std::filesystem::path& filepath) {
    // Delegate to the central format-detection utility to ensure consistent behavior
    // across the codebase (e.g., for names like "model.json.bak").
    return io::detectFileFormat(filepath.filename().string());
}

std::string FileIOFactory::getExtension(FileFormat format) {
    return io::getFileExtension(format);
}

std::string FileIOFactory::getFormatName(FileFormat format) {
    return io::getFormatName(format);
}

}  // namespace truss::infrastructure::io
