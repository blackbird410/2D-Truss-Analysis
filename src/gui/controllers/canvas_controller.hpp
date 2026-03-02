/**
 * @file canvas_controller.hpp
 * @brief Controller mediating between TrussCanvasWidget interactions and
 *        ITrussService facade calls.
 *
 * Phase 1 stub — class declaration only.
 * Full implementation in Phase 5.
 *
 * @note Q_OBJECT is added in Phase 5.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QObject>

namespace truss::application { class ITrussService; }

namespace truss::gui::ctrl {

/**
 * @brief Translates canvas interaction signals into ITrussService calls.
 *
 * Receives signals from TrussCanvasWidget (nodeDropRequested,
 * memberDrawRequested, deleteRequested) and forwards them to the
 * ITrussService* it was constructed with.
 *
 * On success:  emits trussModified(TrussHandle) to MainWindowController.
 * On failure:  emits operationFailed(QString) to NotificationRail.
 *
 * @note CanvasController receives ITrussService* (not the full facade) because
 *       it only needs node/member mutation operations. The sub-interface is
 *       extracted and passed by MainWindow during construction.
 *
 * @todo Phase 5: Add Q_OBJECT macro, implement all public slots with
 *       EXPECT_CALL-compatible facade calls, emit trussModified/operationFailed.
 */
class CanvasController : public QObject {
public:
    explicit CanvasController(QObject* parent = nullptr) : QObject(parent) {}

    // TODO Phase 5: explicit CanvasController(application::ITrussService* service,
    //                                          QObject* parent = nullptr)
    // TODO Phase 5: public slots:
    //   void onNodeDropRequested(core::Point2D pos, core::SupportType support)
    //   void onMemberDrawRequested(core::NodeId startId, core::NodeId endId)
    //   void onNodeDeleteRequested(core::NodeId id)
    //   void onMemberDeleteRequested(core::MemberId id)
    // TODO Phase 5: signals:
    //   void trussModified(std::size_t trussHandle)
    //   void operationFailed(const QString& message)
};

}  // namespace truss::gui::ctrl
