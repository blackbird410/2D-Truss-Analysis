/**
 * @file TrussAssembler.hpp
 * @brief Utility for assembling Domain objects from DTOs
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 * 
 * TrussAssembler converts Data Transfer Objects (DTOs) from Infrastructure
 * layer into fully-formed Domain objects. This enforces DIP by ensuring
 * Infrastructure never directly instantiates Domain objects.
 */

#pragma once

#include "../interfaces/TrussDTO.hpp"
#include "../model/Truss.hpp"
#include <memory>
#include <stdexcept>

namespace truss::core::assembly {

/**
 * @brief Assembles Domain objects from DTOs
 * 
 * This utility class provides static methods to convert DTOs (created by
 * Infrastructure layer during file reading) into Domain objects. This
 * separation ensures Infrastructure depends only on data structures,
 * not concrete Domain implementations.
 * 
 * Design Pattern: Factory/Builder pattern for Domain object construction
 * SOLID: Enforces Dependency Inversion Principle (DIP)
 * 
 * Usage:
 * @code
 * TrussDTO dto = reader->read("truss.json");
 * auto truss = TrussAssembler::assembleTruss(dto);
 * @endcode
 */
class TrussAssembler {
public:
    /**
     * @brief Assemble a Truss Domain object from TrussDTO
     * 
     * Creates a fully-formed Truss with all nodes and members properly
     * connected. Validates that all referenced node IDs exist.
     * 
     * @param dto The data transfer object containing truss data
     * @return Shared pointer to the assembled Truss
     * @throws std::invalid_argument if DTO contains invalid data
     *         (e.g., member references non-existent node)
     */
    static std::shared_ptr<Truss> assembleTruss(const interfaces::TrussDTO& dto);
    
    /**
     * @brief Create a TrussDTO from a Truss Domain object
     * 
     * Converts a Truss (via its ITrussView interface) into a DTO suitable
     * for serialization by Infrastructure layer writers.
     * 
     * @param truss The truss to convert (via view interface)
     * @return TrussDTO containing all truss data
     */
    static interfaces::TrussDTO createDTO(const interfaces::ITrussView& truss);

private:
    TrussAssembler() = delete;  // Static utility class
};

} // namespace truss::core::assembly
