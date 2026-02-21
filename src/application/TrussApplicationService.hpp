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

#include "interfaces/ITrussService.hpp"
#include "Result.hpp"
#include "../core/model/Truss.hpp"
#include "../core/interfaces/ITrussView.hpp"
#include "../core/assembly/TrussAssembler.hpp"
#include "../core/validation/TrussValidator.hpp"
#include "../infrastructure/io/fileio_factory.hpp"
#include "TrussEditDTOs.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>

namespace truss::application {

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
class TrussApplicationService : public ITrussService {
public:
    TrussApplicationService();
    ~TrussApplicationService() override = default;
    
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
    Result<TrussHandle> createTruss(const std::string& name) override;
    
    /**
     * @brief Load truss from file (auto-detects format)
     * @param filepath Path to truss file (.json or .xml)
     * @return Result containing TrussHandle on success
     */
    Result<TrussHandle> loadTruss(const std::filesystem::path& filepath) override;
    
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
                           bool overwrite = false) override;
    
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
    Result<core::validation::ValidationResult> validateTruss(TrussHandle handle) override;
    
    /**
     * @brief Get read-only view of truss (via interface)
     * @param handle Truss handle
     * @return Reference to ITrussView interface
     * @throws std::invalid_argument if handle is invalid
     */
    const core::interfaces::ITrussView& getTrussView(TrussHandle handle) const override;
    
    /**
     * @brief Get mutable access to truss (for UI editing)
     * @param handle Truss handle
     * @return Reference to Truss model
     * @throws std::invalid_argument if handle is invalid
     * 
     * @note Use sparingly - prefer using view interface when possible
     */
    core::Truss& getTrussMutable(TrussHandle handle) override;
    
    /**
     * @brief Clear/delete truss model
     * @param handle Truss handle to remove
     * @return true if truss was deleted, false if handle was invalid
     */
    bool clearTruss(TrussHandle handle) override;
    
    /**
     * @brief Clear all truss models
     */
    void clearAll() override;
    
    /**
     * @brief Check if handle is valid
     * @param handle Truss handle to check
     * @return true if handle references an existing truss
     */
    bool isValidHandle(TrussHandle handle) const override;
    
    /**
     * @brief Get count of active truss models
     * @return Number of trusses currently managed
     */
    size_t getTrussCount() const { return m_trusses.size(); }
    
    // ============================================================
    // NEW GUI-FACING METHODS (Phase 3B)
    // ============================================================
    
    /**
     * @brief Add node to truss
     * @param handle Truss handle
     * @param position Node position in world coordinates
     * @param supportType Support condition (default: Free)
     * @return Result containing NodeId on success
     */
    Result<core::NodeId> addNode(TrussHandle handle,
                                  const core::Point2D& position,
                                  core::SupportType supportType = core::SupportType::Free) override;
    
    /**
     * @brief Add member connecting two nodes
     * 
     * Uses Application-layer DTOs instead of Domain types to decouple
     * Interface layer (GUI/CLI) from Domain implementation.
     * 
     * @param handle Truss handle
     * @param startNodeId Start node identifier
     * @param endNodeId End node identifier
     * @param material Material specification (DTO)
     * @param section Section specification (DTO)
     * @return Result containing MemberId on success
     */
    Result<core::MemberId> addMember(TrussHandle handle,
                                      core::NodeId startNodeId,
                                      core::NodeId endNodeId,
                                      const MaterialSpec& material,
                                      const SectionSpec& section) override;
    
    /**
     * @brief Remove node from truss
     * @param handle Truss handle
     * @param nodeId Node to remove
     * @return Result indicating success/failure
     */
    Result<bool> removeNode(TrussHandle handle, core::NodeId nodeId) override;
    
    /**
     * @brief Remove member from truss
     * @param handle Truss handle
     * @param memberId Member to remove
     * @return Result indicating success/failure
     */
    Result<bool> removeMember(TrussHandle handle, core::MemberId memberId) override;
    
    /**
     * @brief Update node support condition
     * @param handle Truss handle
     * @param nodeId Node to update
     * @param supportType New support type
     * @return Result indicating success/failure
     */
    Result<bool> setNodeSupport(TrussHandle handle,
                                 core::NodeId nodeId,
                                 core::SupportType supportType) override;
    
    /**
     * @brief Apply force to node
     * @param handle Truss handle
     * @param nodeId Node to apply load
     * @param force Force vector
     * @return Result indicating success/failure
     */
    Result<bool> applyNodeLoad(TrussHandle handle,
                                core::NodeId nodeId,
                                const core::Force2D& force) override;
    
    /**
     * @brief Remove load from node
     * @param handle Truss handle
     * @param nodeId Node to clear load
     * @return Result indicating success/failure
     */
    Result<bool> clearNodeLoad(TrussHandle handle, core::NodeId nodeId) override;
    
    /**
     * @brief Check if truss has unsaved changes
     * @param handle Truss handle
     * @return true if modified since last save
     */
    bool hasUnsavedChanges(TrussHandle handle) const;
    
    /**
     * @brief Mark truss as saved (clear modification flag)
     * @param handle Truss handle
     */
    void markAsSaved(TrussHandle handle);

private:
    std::unordered_map<TrussHandle, std::shared_ptr<core::Truss>> m_trusses;
    std::unordered_map<TrussHandle, bool> m_modifiedFlags;  // Track unsaved changes
    core::validation::TrussValidator m_validator;
    TrussHandle m_nextHandle{1};
    
    TrussHandle generateHandle() { return m_nextHandle++; }
    void markAsModified(TrussHandle handle);  // Internal helper
};

} // namespace truss::application
