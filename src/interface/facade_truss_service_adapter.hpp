/**
 * @file facade_truss_service_adapter.hpp
 * @brief Adapter implementing ITrussService through TrussAnalysisFacade public API
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 *
 * **Design**: Proper Adapter Pattern
 * - Implements ITrussService interface
 * - Delegates to TrussAnalysisFacade public methods (NO friend access)
 * - Maintains proper encapsulation and SOLID principles
 * - Clean separation of concerns
 */

#pragma once
#include "application/interfaces/itruss_service.hpp"
#include "interface/itruss_analysis_facade.hpp"
#include "truss/analysis/analysis_options.hpp"

namespace truss::interface {

class ITrussAnalysisFacade;

/**
 * @brief ITrussService adapter for TrussAnalysisFacade
 *
 * Implements the ITrussService interface by delegating to the
 * public methods of TrussAnalysisFacade. Maintains proper encapsulation
 * by using only public APIs (no friend declarations needed).
 *
 * Design Rationale (SOLID Compliant):
 * - Dependency Inversion: Uses public interface of Facade, not private details
 * - Encapsulation: No friend declarations breaking Facade's boundaries
 * - Single Responsibility: Adapter sole purpose is to implement ITrussService
 * - Open/Closed: Can  replace Facade with any object implementing same public API
 */
class FacadeTrussServiceAdapter final : public application::ITrussService {
public:
    explicit FacadeTrussServiceAdapter(truss::interface::ITrussAnalysisFacade& facade);

    application::Result<application::TrussHandle> createTruss(const std::string& name) override;
    application::Result<application::TrussHandle>
    loadTruss(const std::filesystem::path& filepath) override;
    application::Result<bool> saveTruss(application::TrussHandle handle,
                                        const std::filesystem::path& filepath,
                                        bool overwrite = false) override;
    bool clearTruss(application::TrussHandle handle) override;
    void clearAll() override;
    bool isValidTrussHandle(application::TrussHandle handle) const override;

    const core::interfaces::ITrussView&
    getTrussView(application::TrussHandle handle) const override;
    core::Truss& getTrussMutable(application::TrussHandle handle) override;

    application::Result<core::validation::ValidationResult>
    validateTruss(application::TrussHandle handle) override;

    application::Result<core::NodeId>
    addNode(application::TrussHandle handle,
            const core::Point2D& position,
            core::SupportType supportType = core::SupportType::Free) override;

    application::Result<core::MemberId> addMember(application::TrussHandle handle,
                                                  core::NodeId startNodeId,
                                                  core::NodeId endNodeId,
                                                  const application::MaterialSpec& material,
                                                  const application::SectionSpec& section) override;

    application::Result<bool> removeNode(application::TrussHandle handle,
                                         core::NodeId nodeId) override;
    application::Result<bool> removeMember(application::TrussHandle handle,
                                           core::MemberId memberId) override;

    application::Result<bool> setNodeSupport(application::TrussHandle handle,
                                             core::NodeId nodeId,
                                             core::SupportType supportType) override;

    application::Result<bool> applyNodeLoad(application::TrussHandle handle,
                                            core::NodeId nodeId,
                                            const core::Force2D& force) override;

    application::Result<bool> clearNodeLoad(application::TrussHandle handle,
                                            core::NodeId nodeId) override;

    application::Result<bool> updateNode(application::TrussHandle handle,
                                         core::NodeId nodeId,
                                         const application::NodeUpdateSpec& update) override;

    application::Result<bool> updateMember(application::TrussHandle handle,
                                           core::MemberId memberId,
                                           const application::MemberUpdateSpec& update) override;

private:
    truss::interface::ITrussAnalysisFacade& m_facade;
};

}  // namespace truss::interface
