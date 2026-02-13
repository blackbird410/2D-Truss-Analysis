/**
 * @file fileio_factory.hpp
 * @brief Factory for creating file I/O readers and writers
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#pragma once

#include "truss_reader.hpp"
#include "truss_writer.hpp"
#include "io_types.hpp"
#include <memory>
#include <filesystem>

namespace truss::infrastructure::io {

/**
 * @brief Factory for creating file I/O readers and writers
 * 
 * This factory creates appropriate reader/writer instances based on
 * file format, following the Factory design pattern.
 * 
 * Design Pattern: Factory
 * Benefits:
 * - Centralizes object creation logic
 * - Decouples client code from concrete implementations
 * - Supports format auto-detection from file extensions
 */
class FileIOFactory {
public:
    /**
     * @brief Create a truss reader for the specified format
     * 
     * @param format File format to read
     * @return Unique pointer to the reader instance
     * @throws std::invalid_argument if format is not supported
     */
    static std::unique_ptr<ITrussReader> createReader(FileFormat format);
    
    /**
     * @brief Create a truss reader based on file extension
     * 
     * Auto-detects format from file extension (.json, .xml)
     * 
     * @param filepath Path to file (extension used for detection)
     * @return Unique pointer to the reader instance
     * @throws std::invalid_argument if format cannot be detected
     */
    static std::unique_ptr<ITrussReader> createReader(const std::filesystem::path& filepath);
    
    /**
     * @brief Create a truss writer for the specified format
     * 
     * @param format File format to write
     * @return Unique pointer to the writer instance
     * @throws std::invalid_argument if format is not supported
     */
    static std::unique_ptr<ITrussWriter> createWriter(FileFormat format);
    
    /**
     * @brief Create a truss writer based on file extension
     * 
     * Auto-detects format from file extension (.json, .xml)
     * 
     * @param filepath Path to file (extension used for detection)
     * @return Unique pointer to the writer instance
     * @throws std::invalid_argument if format cannot be detected
     */
    static std::unique_ptr<ITrussWriter> createWriter(const std::filesystem::path& filepath);
    
    /**
     * @brief Check if a file format is supported for reading
     * 
     * @param format File format to check
     * @return true if format is supported, false otherwise
     */
    static bool isFormatSupported(FileFormat format);
    
    /**
     * @brief Detect file format from file extension
     * 
     * @param filepath Path to file
     * @return Detected file format, or FileFormat::Auto if unknown
     */
    static FileFormat detectFormat(const std::filesystem::path& filepath);
    
    /**
     * @brief Get file extension for a given format
     * 
     * @param format File format
     * @return File extension (e.g., ".json", ".xml")
     */
    static std::string getExtension(FileFormat format);
    
    /**
     * @brief Get human-readable format name
     * 
     * @param format File format
     * @return Format name (e.g., "JSON", "XML")
     */
    static std::string getFormatName(FileFormat format);
};

} // namespace truss::infrastructure::io
