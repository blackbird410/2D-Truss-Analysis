/**
 * @file canvas_controller.cpp
 * @brief CanvasController implementation (Phase 5).
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/canvas_controller.hpp"

#include "application/truss_edit_dtos.hpp"
#include "interface/itruss_analysis_facade.hpp"

namespace truss::gui::ctrl {

// Default material / section used for all new members until Phase 6 adds a
// material picker.  These are reasonable structural-steel defaults.
namespace {
constexpr double kDefaultYoungsModulusPa = 200e9;   // 200 GPa — structural steel
constexpr double kDefaultAreaM2          = 100e-4;  // 100 cm² = 0.01 m²
}  // namespace

CanvasController::CanvasController(truss::interface::ITrussAnalysisFacade& facade,
                                    QObject*                                 parent)
    : QObject{parent}, m_facade{facade}
{}

void CanvasController::onTrussHandleUpdated(std::size_t trussHandle)
{
    m_trussHandle = trussHandle;
}

void CanvasController::onNodeDropRequested(truss::core::Point2D  pos,
                                            truss::core::SupportType support)
{
    if (m_trussHandle == 0) {
        emit operationFailed(QStringLiteral("No active truss: create or open a project first."));
        return;
    }

    auto result = m_facade.addNode(m_trussHandle, pos, support);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to add node: " + result.errorMessage));
    }
}

void CanvasController::onMemberDrawRequested(truss::core::NodeId startId,
                                              truss::core::NodeId endId)
{
    if (m_trussHandle == 0) {
        emit operationFailed(QStringLiteral("No active truss: create or open a project first."));
        return;
    }

    using truss::application::MaterialSpec;
    using truss::application::SectionSpec;

    MaterialSpec mat;
    mat.youngsModulusPa = kDefaultYoungsModulusPa;
    mat.name            = "Steel";

    SectionSpec sec;
    sec.areaM2   = kDefaultAreaM2;
    sec.profile  = "Generic";

    auto result = m_facade.addMember(m_trussHandle, startId, endId, mat, sec);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to add member: " + result.errorMessage));
    }
}

void CanvasController::onNodeDeleteRequested(truss::core::NodeId id)
{
    if (m_trussHandle == 0) return;

    auto result = m_facade.removeNode(m_trussHandle, id);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to remove node: " + result.errorMessage));
    }
}

void CanvasController::onMemberDeleteRequested(truss::core::MemberId id)
{
    if (m_trussHandle == 0) return;

    auto result = m_facade.removeMember(m_trussHandle, id);
    if (result) {
        emit trussModified(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to remove member: " + result.errorMessage));
    }
}

}  // namespace truss::gui::ctrl
