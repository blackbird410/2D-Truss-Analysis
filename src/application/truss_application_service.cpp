/**
 * @file truss_application_service.cpp
 * @brief Application service for truss model management and operations.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "truss_application_service.hpp"

#include <stdexcept>

namespace truss::application {

using namespace truss::core;
using namespace truss::infrastructure::io;

TrussApplicationService::TrussApplicationService() = default;

Result<TrussHandle> TrussApplicationService::createTruss(const std::string& name) {
    try {
        auto truss = std::make_shared<Truss>(name);
        TrussHandle handle = generateHandle();
        m_trusses[handle] = truss;
        m_modifiedFlags[handle] = false;  // New truss, not modified yet
        return Result<TrussHandle>::Success(handle);
    } catch (const std::exception& e) {
        return Result<TrussHandle>::Failure(std::string("Failed to create truss: ") + e.what());
    }
}

Result<TrussHandle> TrussApplicationService::loadTruss(const std::filesystem::path& filepath) {
    try {
        // Auto-detect format from extension using FileIOFactory
        auto format = FileIOFactory::detectFormat(filepath);
        if (format == FileFormat::Auto) {
            return Result<TrussHandle>::Failure("Could not detect file format from extension: " +
                                                filepath.string());
        }
        return loadTruss(filepath, format);
    } catch (const std::exception& e) {
        return Result<TrussHandle>::Failure(std::string("Failed to load truss: ") + e.what());
    }
}

Result<TrussHandle> TrussApplicationService::loadTruss(const std::filesystem::path& filepath,
                                                       FileFormat format) {
    try {
        // Create appropriate reader
        auto reader = FileIOFactory::createReader(format);

        // Read DTO from file
        FileIOOptions options;
        options.validateOnRead = false;  // We'll validate after assembly
        auto dto = reader->read(filepath, options);

        // Assemble Domain model from DTO
        auto truss = assembly::TrussAssembler::assembleTruss(dto);

        // Validate assembled truss
        auto validationResult = m_validator.validate(*truss);
        if (!validationResult.isValid()) {
            std::string errorMsg = "Truss loaded but failed validation: " +
                                   validationResult.getSummary();

            // Append detailed error messages
            auto errorMessages = validationResult.getErrorMessages();
            if (!errorMessages.empty()) {
                errorMsg += "\nErrors:\n";
                for (const auto& msg : errorMessages) {
                    errorMsg += "  - " + msg + "\n";
                }
            }

            return Result<TrussHandle>::Failure(errorMsg);
        }

        // Store and return handle
        TrussHandle handle = generateHandle();
        m_trusses[handle] = truss;
        m_modifiedFlags[handle] = false;  // Loaded truss, not modified yet
        return Result<TrussHandle>::Success(handle);

    } catch (const FileIOException& e) {
        return Result<TrussHandle>::Failure(std::string("File I/O error: ") + e.what());
    } catch (const std::exception& e) {
        return Result<TrussHandle>::Failure(std::string("Failed to load truss: ") + e.what());
    }
}

Result<bool> TrussApplicationService::saveTruss(TrussHandle handle,
                                                const std::filesystem::path& filepath,
                                                bool overwrite) {
    try {
        // Auto-detect format from extension using FileIOFactory
        auto format = FileIOFactory::detectFormat(filepath);
        if (format == FileFormat::Auto) {
            return Result<bool>::Failure("Could not detect file format from extension: " +
                                         filepath.string());
        }
        return saveTruss(handle, filepath, format, overwrite);
    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Failed to save truss: ") + e.what());
    }
}

Result<bool> TrussApplicationService::saveTruss(TrussHandle handle,
                                                const std::filesystem::path& filepath,
                                                FileFormat format,
                                                bool overwrite) {
    try {
        // Validate handle
        if (!isValidHandle(handle)) {
            return Result<bool>::Failure("Invalid truss handle");
        }

        auto truss = m_trusses[handle];

        // Validate truss before saving
        auto validationResult = m_validator.validate(*truss);
        if (!validationResult.isValid()) {
            return Result<bool>::Failure("Cannot save invalid truss: " +
                                         validationResult.getSummary());
        }

        // Create DTO from Domain model
        auto dto = assembly::TrussAssembler::createDTO(*truss);

        // Create appropriate writer
        auto writer = FileIOFactory::createWriter(format);

        // Write to file
        FileIOOptions options;
        options.overwriteExisting = overwrite;
        options.prettyPrint = true;
        options.includeMetadata = true;

        writer->write(dto, filepath, options);
        markAsSaved(handle);  // Mark as saved after successful write
        return Result<bool>::Success(true);

    } catch (const FileIOException& e) {
        return Result<bool>::Failure(std::string("File I/O error: ") + e.what());
    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Failed to save truss: ") + e.what());
    }
}

Result<validation::ValidationResult> TrussApplicationService::validateTruss(TrussHandle handle) {
    try {
        if (!isValidHandle(handle)) {
            return Result<validation::ValidationResult>::Failure("Invalid truss handle");
        }

        auto truss = m_trusses[handle];
        auto result = m_validator.validate(*truss);
        return Result<validation::ValidationResult>::Success(std::move(result));

    } catch (const std::exception& e) {
        return Result<validation::ValidationResult>::Failure(std::string("Validation failed: ") +
                                                             e.what());
    }
}

const core::interfaces::ITrussView&
TrussApplicationService::getTrussView(TrussHandle handle) const {
    if (!isValidHandle(handle)) {
        throw std::invalid_argument("Invalid truss handle: " + std::to_string(handle));
    }
    return *m_trusses.at(handle);
}

core::Truss& TrussApplicationService::getTrussMutable(TrussHandle handle) {
    if (!isValidHandle(handle)) {
        throw std::invalid_argument("Invalid truss handle: " + std::to_string(handle));
    }
    return *m_trusses.at(handle);
}

bool TrussApplicationService::clearTruss(TrussHandle handle) {
    m_modifiedFlags.erase(handle);
    return m_trusses.erase(handle) > 0;
}

void TrussApplicationService::clearAll() {
    m_trusses.clear();
    m_modifiedFlags.clear();
}

bool TrussApplicationService::isValidHandle(TrussHandle handle) const {
    return m_trusses.find(handle) != m_trusses.end();
}

// ============================================================
// NEW GUI-FACING METHOD IMPLEMENTATIONS (Phase 3B)
// ============================================================

Result<NodeId> TrussApplicationService::addNode(TrussHandle handle,
                                                const Point2D& position,
                                                SupportType supportType) {
    try {
        if (!isValidHandle(handle)) {
            return Result<NodeId>::Failure("Invalid truss handle");
        }

        auto& truss = *m_trusses[handle];
        NodePtr node = truss.addNode(position, supportType);
        markAsModified(handle);

        return Result<NodeId>::Success(node->getId());

    } catch (const std::exception& e) {
        return Result<NodeId>::Failure(std::string("Failed to add node: ") + e.what());
    }
}

Result<MemberId> TrussApplicationService::addMember(TrussHandle handle,
                                                    NodeId startNodeId,
                                                    NodeId endNodeId,
                                                    const MaterialSpec& materialSpec,
                                                    const SectionSpec& sectionSpec) {
    try {
        if (!isValidHandle(handle)) {
            return Result<MemberId>::Failure("Invalid truss handle");
        }

        // Convert DTOs to Domain types with sensible defaults
        MaterialProperties material{
            materialSpec.youngsModulusPa,            // E
            7850.0,                                  // density (default for steel)
            materialSpec.youngsModulusPa * 0.00125,  // yield strength (estimate)
            materialSpec.youngsModulusPa * 0.002,    // ultimate strength (estimate)
            materialSpec.name                        // name
        };

        SectionProperties section{
            sectionSpec.areaM2,                              // area
            sectionSpec.areaM2 * sectionSpec.areaM2 / 12.0,  // moment of inertia (estimate)
            sectionSpec.areaM2,                              // shear area
            sectionSpec.profile                              // designation
        };

        auto& truss = *m_trusses[handle];
        MemberPtr member = truss.addMember(startNodeId, endNodeId, material, section);
        markAsModified(handle);

        return Result<MemberId>::Success(member->getId());

    } catch (const std::exception& e) {
        return Result<MemberId>::Failure(std::string("Failed to add member: ") + e.what());
    }
}

Result<bool> TrussApplicationService::removeNode(TrussHandle handle, NodeId nodeId) {
    try {
        if (!isValidHandle(handle)) {
            return Result<bool>::Failure("Invalid truss handle");
        }

        auto& truss = *m_trusses[handle];
        bool removed = truss.removeNode(nodeId);

        if (removed) {
            markAsModified(handle);
            return Result<bool>::Success(true);
        } else {
            return Result<bool>::Failure("Node not found: " + std::to_string(nodeId));
        }

    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Failed to remove node: ") + e.what());
    }
}

Result<bool> TrussApplicationService::removeMember(TrussHandle handle, MemberId memberId) {
    try {
        if (!isValidHandle(handle)) {
            return Result<bool>::Failure("Invalid truss handle");
        }

        auto& truss = *m_trusses[handle];
        bool removed = truss.removeMember(memberId);

        if (removed) {
            markAsModified(handle);
            return Result<bool>::Success(true);
        } else {
            return Result<bool>::Failure("Member not found: " + std::to_string(memberId));
        }

    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Failed to remove member: ") + e.what());
    }
}

Result<bool> TrussApplicationService::setNodeSupport(TrussHandle handle,
                                                     NodeId nodeId,
                                                     SupportType supportType) {
    try {
        if (!isValidHandle(handle)) {
            return Result<bool>::Failure("Invalid truss handle");
        }

        auto& truss = *m_trusses[handle];
        truss.setSupportType(nodeId, supportType);
        markAsModified(handle);
        return Result<bool>::Success(true);

    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Failed to set node support: ") + e.what());
    }
}

Result<bool>
TrussApplicationService::applyNodeLoad(TrussHandle handle, NodeId nodeId, const Force2D& force) {
    try {
        if (!isValidHandle(handle)) {
            return Result<bool>::Failure("Invalid truss handle");
        }

        auto& truss = *m_trusses[handle];
        truss.applyForce(nodeId, force);
        markAsModified(handle);
        return Result<bool>::Success(true);

    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Failed to apply node load: ") + e.what());
    }
}

Result<bool> TrussApplicationService::clearNodeLoad(TrussHandle handle, NodeId nodeId) {
    try {
        if (!isValidHandle(handle)) {
            return Result<bool>::Failure("Invalid truss handle");
        }

        const auto& truss = *m_trusses[handle];
        // Get the node and clear its force
        auto node = truss.getNode(nodeId);
        if (!node) {
            return Result<bool>::Failure("Node not found: " + std::to_string(nodeId));
        }

        node->setAppliedForce(Force2D{0.0, 0.0});
        markAsModified(handle);
        return Result<bool>::Success(true);

    } catch (const std::exception& e) {
        return Result<bool>::Failure(std::string("Failed to clear node load: ") + e.what());
    }
}

[[maybe_unused]] bool TrussApplicationService::hasUnsavedChanges(TrussHandle handle) const {
    if (!isValidHandle(handle)) {
        return false;
    }
    auto it = m_modifiedFlags.find(handle);
    return it != m_modifiedFlags.end() && it->second;
}

void TrussApplicationService::markAsSaved(TrussHandle handle) {
    if (isValidHandle(handle)) {
        m_modifiedFlags[handle] = false;
    }
}

void TrussApplicationService::markAsModified(TrussHandle handle) {
    m_modifiedFlags[handle] = true;
}

}  // namespace truss::application
