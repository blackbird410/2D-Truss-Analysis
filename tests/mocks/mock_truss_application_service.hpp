/**
 * @file mock_truss_application_service.hpp
 * @brief GoogleMock for ITrussService interface.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This mock service enables isolated testing of Controllers and Presenters
 * without depending on the actual Application Service implementation.
 *
 * Architecture: Testing Infrastructure (MVP Pattern)
 * Purpose: Enable unit testing of GUI Controllers with mocked Application layer
 *
 * GoogleMock Integration: Inherits from ITrussService interface to enable
 * polymorphic dependency injection in Controller tests.
 */

#pragma once

#include "application/interfaces/itruss_service.hpp"

#include <gmock/gmock.h>

namespace truss::test {

/**
 * @brief Mock TrussApplicationService for Controller testing
 *
 * This mock allows testing Controllers in isolation by providing
 * controllable, predictable responses from the Application layer.
 *
 * Inherits from ITrussService to ensure type compatibility with Controllers
 * that depend on the interface abstraction (Dependency Inversion Principle).
 */
class MockTrussApplicationService : public truss::application::ITrussService {
public:
    ~MockTrussApplicationService() override = default;

    // Mock lifecycle operations
    MOCK_METHOD(application::Result<application::TrussHandle>,
                createTruss,
                (const std::string& name),
                (override));

    MOCK_METHOD(application::Result<application::TrussHandle>,
                loadTruss,
                (const std::filesystem::path& filepath),
                (override));

    MOCK_METHOD(application::Result<bool>,
                saveTruss,
                (application::TrussHandle handle,
                 const std::filesystem::path& filepath,
                 bool overwrite),
                (override));

    MOCK_METHOD(bool, clearTruss, (application::TrussHandle handle), (override));

    MOCK_METHOD(void, clearAll, (), (override));

    MOCK_METHOD(bool, isValidHandle, (application::TrussHandle handle), (const, override));

    // Mock view access (required by ITrussService interface)
    MOCK_METHOD(const core::interfaces::ITrussView&,
                getTrussView,
                (application::TrussHandle handle),
                (const, override));

    MOCK_METHOD(core::Truss&, getTrussMutable, (application::TrussHandle handle), (override));

    // Mock validation (required by ITrussService interface)
    MOCK_METHOD(application::Result<core::validation::ValidationResult>,
                validateTruss,
                (application::TrussHandle handle),
                (override));

    // Mock editing operations with Application DTOs
    MOCK_METHOD(application::Result<core::NodeId>,
                addNode,
                (application::TrussHandle handle,
                 const core::Point2D& position,
                 core::SupportType supportType),
                (override));

    MOCK_METHOD(application::Result<core::MemberId>,
                addMember,
                (application::TrussHandle handle,
                 core::NodeId startNodeId,
                 core::NodeId endNodeId,
                 const application::MaterialSpec& material,
                 const application::SectionSpec& section),
                (override));

    MOCK_METHOD(application::Result<bool>,
                removeNode,
                (application::TrussHandle handle, core::NodeId nodeId),
                (override));

    MOCK_METHOD(application::Result<bool>,
                removeMember,
                (application::TrussHandle handle, core::MemberId memberId),
                (override));

    MOCK_METHOD(application::Result<bool>,
                setNodeSupport,
                (application::TrussHandle handle,
                 core::NodeId nodeId,
                 core::SupportType supportType),
                (override));

    MOCK_METHOD(application::Result<bool>,
                applyNodeLoad,
                (application::TrussHandle handle, core::NodeId nodeId, const core::Force2D& force),
                (override));

    MOCK_METHOD(application::Result<bool>,
                clearNodeLoad,
                (application::TrussHandle handle, core::NodeId nodeId),
                (override));

    // Additional helper methods (not in ITrussService but useful for testing)
    MOCK_METHOD(bool, hasUnsavedChanges, (application::TrussHandle handle), (const));
    MOCK_METHOD(void, markAsSaved, (application::TrussHandle handle), ());
    MOCK_METHOD(void, markAsModified, (application::TrussHandle handle), ());
};

}  // namespace truss::test
