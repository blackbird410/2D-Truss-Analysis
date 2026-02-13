/**
 * @file truss_writer.hpp
 * @brief Abstract interface for writing truss structures to files
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#pragma once

#include "../../core/model/Truss.hpp"
#include "io_types.hpp"
#include <filesystem>

namespace truss::infrastructure::io {

/**
 * @brief Abstract interface for truss file writers (Strategy pattern)
 * 
 * This interface defines the contract for writing truss structures to
 * various file formats. Concrete implementations handle specific formats
 * (JSON, XML, etc.).
 * 
 * Design Pattern: Strategy
 * SOLID Principles:
 * - Interface Segregation: Single focused responsibility (writing)
 * - Dependency Inversion: Clients depend on abstraction, not implementation
 * - Open/Closed: New formats can be added without modifying existing code
 */
class ITrussWriter {
public:
    virtual ~ITrussWriter() = default;
    
    /**
     * @brief Write a truss structure to a file
     * 
     * @param truss The truss structure to write
     * @param filepath Path to the file to write
     * @param options File I/O options
     * @return true if write successful, false otherwise
     * @throws FileWriteException if file cannot be written
     * @throws ValidationException if truss structure is invalid (when validateOnWrite=true)
     */
    virtual bool write(
        const core::Truss& truss,
        const std::filesystem::path& filepath,
        const FileIOOptions& options = FileIOOptions{}
    ) = 0;
    
    /**
     * @brief Check if this writer supports a given file format
     * 
     * @param format File format to check
     * @return true if format is supported, false otherwise
     */
    virtual bool supportsFormat(FileFormat format) const = 0;
    
    /**
     * @brief Get the file format this writer handles
     * 
     * @return File format supported by this writer
     */
    virtual FileFormat getFormat() const = 0;
};

} // namespace truss::infrastructure::io
