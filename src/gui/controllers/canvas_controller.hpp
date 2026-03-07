/**
 * @file canvas_controller.hpp
 * @brief Controller mediating TrussCanvasWidget interactions and
 *        ITrussAnalysisFacade calls for node/member mutation.
 *
 * Phase 5: Full Q_OBJECT implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "application/truss_edit_dtos.hpp"
#include "core/model/types.hpp"

#include <QObject>
#include <QString>

#include <cstddef>

namespace truss::interface {
class ITrussAnalysisFacade;
}

namespace truss::gui::ctrl {

/**
 * @brief Translates canvas interaction signals into ITrussAnalysisFacade calls.
 *
 * CanvasController is constructed with a facade reference and a default
 * truss handle.  When the user drops a node or draws a member on the canvas
 * the corresponding signal is forwarded here; the controller calls the facade
 * and emits trussModified(handle) on success or operationFailed(message) on
 * failure.  The active handle is updated via onTrussHandleUpdated.
 *
 * New members are created with a default Steel material / 100 cm² section
 * (sensible defaults pending a full material picker in Phase 6).
 */
class CanvasController : public QObject {
    Q_OBJECT

public:
    explicit CanvasController(truss::interface::ITrussAnalysisFacade& facade,
                              QObject* parent = nullptr);

public slots:
    /// Drop a node at @p pos with given support type.
    void onNodeDropRequested(truss::core::Point2D pos, truss::core::SupportType support);
    /// Draw a member between two existing nodes.
    void onMemberDrawRequested(truss::core::NodeId startId, truss::core::NodeId endId);
    /// Delete a node and all members connected to it.
    void onNodeDeleteRequested(truss::core::NodeId id);
    /// Delete a single member.
    void onMemberDeleteRequested(truss::core::MemberId id);
    /// Update the active truss handle (called by MainWindowController on every truss change).
    void onTrussHandleUpdated(std::size_t trussHandle);

signals:
    void trussModified(std::size_t trussHandle);
    void operationFailed(const QString& message);

private:
    truss::interface::ITrussAnalysisFacade& m_facade;
    std::size_t m_trussHandle{0};
};

}  // namespace truss::gui::ctrl
