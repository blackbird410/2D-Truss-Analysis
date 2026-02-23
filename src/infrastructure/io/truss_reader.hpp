/**
 * @file truss_reader.hpp
 * @brief Abstract interface for reading truss structures from files
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#pragma once

#include "../../core/interfaces/truss_dto.hpp"
#include "io_types.hpp"

#include <filesystem>
#include <memory>

namespace truss::infrastructure::io {

/**
 * @brief Abstract interface for truss file readers (Strategy pattern)
 *
 * This interface defines the contract for reading truss structures from
 * various file formats. Concrete implementations handle specific formats
 * (JSON, XML, etc.).
 *
 * Design Pattern: Strategy
 * SOLID Principles:
 * - Interface Segregation: Single focused responsibility (reading)
 * - Dependency Inversion: Clients depend on abstraction, not implementation
 * - Open/Closed: New formats can be added without modifying existing code
 */
class ITrussReader {
public:
    virtual ~ITrussReader() = default;

    /**
     * @brief Read a truss structure from a file
     *
     * @param filepath Path to the file to read
     * @param options File I/O options
     * @return TrussDTO containing the loaded truss data
     * @throws FileNotFoundException if file doesn't exist
     * @throws FileReadException if file cannot be read
     * @throws ParseException if file format is invalid
     *
     * NOTE: Returns DTO to enforce DIP - Infrastructure does not create Domain objects.
     * Clients should use TrussAssembler to convert DTO → Domain object if needed.
     */
    virtual core::interfaces::TrussDTO read(const std::filesystem::path& filepath,
                                            const FileIOOptions& options = FileIOOptions{}) = 0;

    /**
     * @brief Check if this reader supports a given file format
     *
     * @param format File format to check
     * @return true if format is supported, false otherwise
     */
    virtual bool supportsFormat(FileFormat format) const = 0;

    /**
     * @brief Get the file format this reader handles
     *
     * @return File format supported by this reader
     */
    virtual FileFormat getFormat() const = 0;
};

}  // namespace truss::infrastructure::io
