/**
 * @file MockTrussApplicationService.hpp
 * @brief Mock implementation of TrussApplicationService for testing
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * This mock service enables isolated testing of Controllers and Presenters
 * without depending on the actual Application Service implementation.
 * 
 * Architecture: Testing Infrastructure (MVP Pattern)
 * Purpose: Enable unit testing of GUI Controllers with mocked Application layer
 */

#pragma once

#include "application/TrussApplicationService.hpp"
#include <gmock/gmock.h>

namespace truss::test {

/**
 * @brief Mock TrussApplicationService for Controller testing
 * 
 * This mock allows testing Controllers in isolation by providing
 * controllable, predictable responses from the Application layer.
 */
class MockTrussApplicationService {
public:
    // Mock lifecycle operations
    MOCK_METHOD(application::Result<application::TrussHandle>, 
                createTruss, (const std::string& name), ());
    
    MOCK_METHOD(application::Result<application::TrussHandle>, 
                loadTruss, (const std::filesystem::path& filepath), ());
    
    MOCK_METHOD(application::Result<bool>, 
                saveTruss, 
                (application::TrussHandle handle, 
                 const std::filesystem::path& filepath, 
                 bool overwrite), 
                ());
    
    MOCK_METHOD(bool, clearTruss, (application::TrussHandle handle), ());
    
    MOCK_METHOD(void, clearAll, (), ());
    
    MOCK_METHOD(bool, isValidHandle, (application::TrussHandle handle), (const));
    
    // Mock editing operations with Application DTOs
    MOCK_METHOD(application::Result<core::NodeId>, 
                addNode, 
                (application::TrussHandle handle,
                 const core::Point2D& position,
                 core::SupportType supportType), 
                ());
    
    MOCK_METHOD(application::Result<core::MemberId>, 
                addMember, 
                (application::TrussHandle handle,
                 core::NodeId startNodeId,
                 core::NodeId endNodeId,
                 const application::MaterialSpec& material,
                 const application::SectionSpec& section), 
                ());
    
    MOCK_METHOD(application::Result<bool>, 
                removeNode, 
                (application::TrussHandle handle, core::NodeId nodeId), 
                ());
    
    MOCK_METHOD(application::Result<bool>, 
                removeMember, 
                (application::TrussHandle handle, core::MemberId memberId), 
                ());
    
    MOCK_METHOD(application::Result<bool>, 
                setNodeSupport, 
                (application::TrussHandle handle,
                 core::NodeId nodeId,
                 core::SupportType supportType), 
                ());
    
    MOCK_METHOD(application::Result<bool>, 
                applyNodeLoad, 
                (application::TrussHandle handle,
                 core::NodeId nodeId,
                 const core::Force2D& force), 
                ());
    
    MOCK_METHOD(application::Result<bool>, 
                clearNodeLoad, 
                (application::TrussHandle handle, core::NodeId nodeId), 
                ());
    
    // Mock query operations
    MOCK_METHOD(bool, hasUnsavedChanges, (application::TrussHandle handle), (const));
    
    MOCK_METHOD(void, markAsSaved, (application::TrussHandle handle), ());
    
    MOCK_METHOD(void, markAsModified, (application::TrussHandle handle), ());
};

}  // namespace truss::test
