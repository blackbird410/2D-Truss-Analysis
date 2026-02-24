/**
 * @file io_types.hpp
 * @brief File I/O types, formats, and options for truss file operations.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include <stdexcept>
#include <string>

namespace truss::infrastructure::io {

/**
 * @brief File format options for truss file I/O
 */
enum class FileFormat {
    JSON,  ///< JSON format (default)
    XML,   ///< XML format
    Auto   ///< Auto-detect from file extension
};

/**
 * @brief File I/O options for reading and writing
 */
struct FileIOOptions {
    bool prettyPrint{true};         ///< Pretty-print output (adds indentation/whitespace)
    int indentSize{2};              ///< Number of spaces for indentation
    bool includeMetadata{true};     ///< Include metadata (timestamp, version, etc.)
    bool overwriteExisting{false};  ///< Allow overwriting existing files
    bool validateOnWrite{
        false};  ///< Validate domain model before serialization (via TrussValidator)
    bool validateOnRead{
        false};  ///< Validate domain model after deserialization (via TrussValidator)
};

/**
 * @brief Exceptions for File I/O operations
 */
class FileIOException : public std::runtime_error {
public:
    explicit FileIOException(const std::string& message) : std::runtime_error(message) {}
};

class FileNotFoundException : public FileIOException {
public:
    explicit FileNotFoundException(const std::string& filepath)
        : FileIOException("File not found: " + filepath) {}
};

class FileWriteException : public FileIOException {
public:
    explicit FileWriteException(const std::string& filepath)
        : FileIOException("Failed to write file: " + filepath) {}
};

class FileReadException : public FileIOException {
public:
    explicit FileReadException(const std::string& filepath)
        : FileIOException("Failed to read file: " + filepath) {}
};

class ParseException : public FileIOException {
public:
    explicit ParseException(const std::string& message)
        : FileIOException("Parse error: " + message) {}
};

class ValidationException : public FileIOException {
public:
    explicit ValidationException(const std::string& message)
        : FileIOException("Validation error: " + message) {}
};

/**
 * @brief Get file extension for a given format
 */
inline std::string getFileExtension(FileFormat format) {
    switch (format) {
        case FileFormat::JSON:
            return ".json";
        case FileFormat::XML:
            return ".xml";
        case FileFormat::Auto:
            return "";
        default:
            return "";
    }
}

/**
 * @brief Detect file format from extension
 */
inline FileFormat detectFileFormat(const std::string& filepath) {
    if (filepath.ends_with(".json"))
        return FileFormat::JSON;
    if (filepath.ends_with(".xml"))
        return FileFormat::XML;
    return FileFormat::Auto;  // Unknown
}

/**
 * @brief Get human-readable format name
 */
inline std::string getFormatName(FileFormat format) {
    switch (format) {
        case FileFormat::JSON:
            return "JSON";
        case FileFormat::XML:
            return "XML";
        case FileFormat::Auto:
            return "Auto";
        default:
            return "Unknown";
    }
}

}  // namespace truss::infrastructure::io
