/**
 * @file itruss_service.hpp
 * @brief Abstract interface for truss application services.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This interface defines the contract for truss lifecycle and editing operations,
 * enabling dependency injection and unit testing of GUI controllers.
 *
 * Design Pattern: Dependency Inversion Principle (depend on abstractions)
 * Architecture: Application Layer Interface
 */

#pragma once

#include "../../core/interfaces/itruss_view.hpp"
#include "../../core/model/types.hpp"
#include "../../core/validation/truss_validator.hpp"  // For ValidationResult
#include "../result.hpp"
#include "../truss_edit_dtos.hpp"

#include <filesystem>
#include <string>

// Forward declarations
namespace truss::core {
class Truss;
}

namespace truss::application {

/**
 * @brief Abstract interface for truss application services
 *
 * Enables polymorphic dependency injection for controllers,
 * allowing mock implementations for unit testing.
 *
 * All methods return Result<T> for consistent error handling.
 */
class ITrussService {
public:
    virtual ~ITrussService() = default;

    // Lifecycle operations
    virtual Result<TrussHandle> createTruss(const std::string& name) = 0;
    virtual Result<TrussHandle> loadTruss(const std::filesystem::path& filepath) = 0;
    virtual Result<bool> saveTruss(TrussHandle handle,
                                   const std::filesystem::path& filepath,
                                   bool overwrite = false) = 0;
    virtual bool clearTruss(TrussHandle handle) = 0;
    virtual void clearAll() = 0;
    virtual bool isValidTrussHandle(TrussHandle handle) const = 0;

    // View access
    virtual const core::interfaces::ITrussView& getTrussView(TrussHandle handle) const = 0;
    virtual core::Truss& getTrussMutable(TrussHandle handle) = 0;

    // Validation
    virtual Result<core::validation::ValidationResult> validateTruss(TrussHandle handle) = 0;

    // Editing operations (Application DTOs)
    virtual Result<core::NodeId>
    addNode(TrussHandle handle,
            const core::Point2D& position,
            core::SupportType supportType = core::SupportType::Free) = 0;

    virtual Result<core::MemberId> addMember(TrussHandle handle,
                                             core::NodeId startNodeId,
                                             core::NodeId endNodeId,
                                             const MaterialSpec& material,
                                             const SectionSpec& section) = 0;

    virtual Result<bool> removeNode(TrussHandle handle, core::NodeId nodeId) = 0;
    virtual Result<bool> removeMember(TrussHandle handle, core::MemberId memberId) = 0;

    virtual Result<bool>
    setNodeSupport(TrussHandle handle, core::NodeId nodeId, core::SupportType supportType) = 0;

    virtual Result<bool>
    applyNodeLoad(TrussHandle handle, core::NodeId nodeId, const core::Force2D& force) = 0;

    virtual Result<bool> clearNodeLoad(TrussHandle handle, core::NodeId nodeId) = 0;
};

}  // namespace truss::application
