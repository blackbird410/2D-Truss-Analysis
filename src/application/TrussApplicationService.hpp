/**
 * @file TrussApplicationService.hpp
 * @brief Application facade for truss model operations
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * This service provides a high-level API for Interface layer (GUI/CLI) to interact
 * with truss models without direct dependency on Domain implementation details.
 * 
 * Responsibilities:
 * - Load/Save truss models (coordinates I/O + Assembly)
 * - Create/Clear truss models
 * - Validate truss structures
 * - Manage truss lifecycle
 * 
 * Design Pattern: Facade (simplifies complex subsystems)
 * Architecture: Application Layer (orchestrates Domain + Infrastructure)
 */

#pragma once

#include "../core/model/Truss.hpp"
#include "../core/interfaces/ITrussView.hpp"
#include "../core/assembly/TrussAssembler.hpp"
#include "../core/validation/TrussValidator.hpp"
#include "../infrastructure/io/fileio_factory.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>

namespace truss::application {

/**
 * @brief Handle type for managing truss instances
 * 
 * Provides opaque reference to internal truss storage without
 * exposing implementation details to Interface layer.
 */
using TrussHandle = size_t;

/**
 * @brief Result type for operations that may fail
 */
template<typename T>
struct Result {
    bool success;
    T value;
    std::string errorMessage;
    
    static Result<T> Success(T val) { return {true, std::move(val), ""}; }
    static Result<T> Failure(const std::string& msg) { return {false, T{}, msg}; }
    
    operator bool() const { return success; }
};

/**
 * @brief Application service for truss model operations
 * 
 * This service acts as a facade between the Interface layer and the Domain/Infrastructure layers.
 * It coordinates operations across multiple subsystems while keeping the Interface layer
 * decoupled from implementation details.
 * 
 * Thread Safety: Not thread-safe (intended for single-threaded GUI/CLI applications)
 * 
 * Usage Example:
 * @code
 * TrussApplicationService service;
 * auto result = service.createTruss("MyBridge");
 * if (result) {
 *     TrussHandle handle = result.value;
 *     const auto& view = service.getTrussView(handle);
 *     // Use view to read truss data...
 * }
 * @endcode
 */
class TrussApplicationService {
public:
    TrussApplicationService();
    ~TrussApplicationService() = default;
    
    // Disable copy (manages unique resources)
    TrussApplicationService(const TrussApplicationService&) = delete;
    TrussApplicationService& operator=(const TrussApplicationService&) = delete;
    
    // Allow move
    TrussApplicationService(TrussApplicationService&&) noexcept = default;
    TrussApplicationService& operator=(TrussApplicationService&&) noexcept = default;
    
    /**
     * @brief Create a new empty truss model
     * @param name Truss name (for display/metadata)
     * @return Result containing TrussHandle on success
     */
    Result<TrussHandle> createTruss(const std::string& name);
    
    /**
     * @brief Load truss from file (auto-detects format)
     * @param filepath Path to truss file (.json or .xml)
     * @return Result containing TrussHandle on success
     */
    Result<TrussHandle> loadTruss(const std::filesystem::path& filepath);
    
    /**
     * @brief Load truss from file with explicit format
     * @param filepath Path to truss file
     * @param format File format (JSON or XML)
     * @return Result containing TrussHandle on success
     */
    Result<TrussHandle> loadTruss(const std::filesystem::path& filepath, 
                                   infrastructure::io::FileFormat format);
    
    /**
     * @brief Save truss to file (auto-detects format from extension)
     * @param handle Truss handle
     * @param filepath Path to save file
     * @param overwrite Allow overwriting existing file
     * @return Result indicating success/failure
     */
    Result<bool> saveTruss(TrussHandle handle, 
                           const std::filesystem::path& filepath,
                           bool overwrite = false);
    
    /**
     * @brief Save truss to file with explicit format
     * @param handle Truss handle
     * @param filepath Path to save file
     * @param format File format (JSON or XML)
     * @param overwrite Allow overwriting existing file
     * @return Result indicating success/failure
     */
    Result<bool> saveTruss(TrussHandle handle,
                           const std::filesystem::path& filepath,
                           infrastructure::io::FileFormat format,
                           bool overwrite = false);
    
    /**
     * @brief Validate truss structure
     * @param handle Truss handle
     * @return Result containing validation report
     */
    Result<core::validation::ValidationResult> validateTruss(TrussHandle handle);
    
    /**
     * @brief Get read-only view of truss (via interface)
     * @param handle Truss handle
     * @return Reference to ITrussView interface
     * @throws std::invalid_argument if handle is invalid
     */
    const core::interfaces::ITrussView& getTrussView(TrussHandle handle) const;
    
    /**
     * @brief Get mutable access to truss (for UI editing)
     * @param handle Truss handle
     * @return Reference to Truss model
     * @throws std::invalid_argument if handle is invalid
     * 
     * @note Use sparingly - prefer using view interface when possible
     */
    core::Truss& getTrussMutable(TrussHandle handle);
    
    /**
     * @brief Clear/delete truss model
     * @param handle Truss handle to remove
     * @return true if truss was deleted, false if handle was invalid
     */
    bool clearTruss(TrussHandle handle);
    
    /**
     * @brief Clear all truss models
     */
    void clearAll();
    
    /**
     * @brief Check if handle is valid
     * @param handle Truss handle to check
     * @return true if handle references an existing truss
     */
    bool isValidHandle(TrussHandle handle) const;
    
    /**
     * @brief Get count of active truss models
     * @return Number of trusses currently managed
     */
    size_t getTrussCount() const { return m_trusses.size(); }

private:
    std::unordered_map<TrussHandle, std::shared_ptr<core::Truss>> m_trusses;
    core::validation::TrussValidator m_validator;
    TrussHandle m_nextHandle{1};
    
    TrussHandle generateHandle() { return m_nextHandle++; }
};

} // namespace truss::application
