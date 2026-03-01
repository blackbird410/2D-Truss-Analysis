/**
 * @file facade_truss_service_adapter.cpp
 * @brief ITrussService adapter implementation backed by TrussAnalysisFacade.
 * @version 3.0.0
 * @date 2026-02-27
 * @author Neil Taison Rigaud
 */

#include "facade_truss_service_adapter.hpp"
#include "truss_analysis_facade.hpp"

namespace truss::interface {

FacadeTrussServiceAdapter::FacadeTrussServiceAdapter(TrussAnalysisFacade& facade)
    : m_facade(facade) {}

application::Result<application::TrussHandle>
FacadeTrussServiceAdapter::createTruss(const std::string& name) {
    return m_facade.createTruss(name);
}

application::Result<application::TrussHandle>
FacadeTrussServiceAdapter::loadTruss(const std::filesystem::path& filepath) {
    return m_facade.loadTruss(filepath);
}

application::Result<bool> FacadeTrussServiceAdapter::saveTruss(
    application::TrussHandle handle, const std::filesystem::path& filepath, bool overwrite) {
    return m_facade.saveTruss(handle, filepath, overwrite);
}

bool FacadeTrussServiceAdapter::clearTruss(application::TrussHandle handle) {
    return m_facade.clearTruss(handle);
}

void FacadeTrussServiceAdapter::clearAll() {
    m_facade.clearAll();
}

bool FacadeTrussServiceAdapter::isValidTrussHandle(application::TrussHandle handle) const {
    return m_facade.isValidTrussHandle(handle);
}

const core::interfaces::ITrussView&
FacadeTrussServiceAdapter::getTrussView(application::TrussHandle handle) const {
    return m_facade.getTrussView(handle);
}

core::Truss& FacadeTrussServiceAdapter::getTrussMutable(application::TrussHandle handle) {
    return m_facade.getTrussMutable(handle);
}

application::Result<core::validation::ValidationResult>
FacadeTrussServiceAdapter::validateTruss(application::TrussHandle handle) {
    return m_facade.validateTruss(handle);
}

application::Result<core::NodeId> FacadeTrussServiceAdapter::addNode(
    application::TrussHandle handle, const core::Point2D& position, core::SupportType supportType) {
    return m_facade.addNode(handle, position, supportType);
}

application::Result<core::MemberId>
FacadeTrussServiceAdapter::addMember(application::TrussHandle handle,
                                     core::NodeId startNodeId,
                                     core::NodeId endNodeId,
                                     const application::MaterialSpec& material,
                                     const application::SectionSpec& section) {
    return m_facade.addMember(handle, startNodeId, endNodeId, material, section);
}

application::Result<bool> FacadeTrussServiceAdapter::removeNode(application::TrussHandle handle,
                                                                core::NodeId nodeId) {
    return m_facade.removeNode(handle, nodeId);
}

application::Result<bool> FacadeTrussServiceAdapter::removeMember(application::TrussHandle handle,
                                                                  core::MemberId memberId) {
    return m_facade.removeMember(handle, memberId);
}

application::Result<bool> FacadeTrussServiceAdapter::setNodeSupport(application::TrussHandle handle,
                                                                    core::NodeId nodeId,
                                                                    core::SupportType supportType) {
    return m_facade.setNodeSupport(handle, nodeId, supportType);
}

application::Result<bool> FacadeTrussServiceAdapter::applyNodeLoad(application::TrussHandle handle,
                                                                   core::NodeId nodeId,
                                                                   const core::Force2D& force) {
    return m_facade.applyNodeLoad(handle, nodeId, force);
}

application::Result<bool> FacadeTrussServiceAdapter::clearNodeLoad(application::TrussHandle handle,
                                                                   core::NodeId nodeId) {
    return m_facade.clearNodeLoad(handle, nodeId);
}

}  // namespace truss::interface
