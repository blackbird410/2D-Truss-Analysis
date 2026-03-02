/**
 * @file project_controller_v2.hpp
 * @brief New ProjectController (truss::gui::ctrl namespace) that replaces the
 *        legacy truss_controllers::ProjectController from Phase 8 onward.
 *
 * Phase 1 stub — class declaration only.
 * Full implementation in Phase 5.
 *
 * @note NAMING: This file uses the _v2 suffix temporarily to coexist with the
 *       legacy src/gui/controllers/project_controller.hpp during Phases 1–7.
 *       In Phase 8 the legacy file is deleted and this file is renamed to
 *       project_controller.hpp.
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
 * @brief Manages project lifecycle: new, open, save, save-as, close.
 *
 * Guards destructive actions (new project, close) against unsaved changes
 * using an IConfirmationProvider interface to allow headless testing without
 * modal dialogs. The default implementation delegates to QMessageBox.
 *
 * @todo Phase 5: Add Q_OBJECT macro, implement all project lifecycle slots,
 *       introduce IConfirmationProvider, wire QFileDialog calls, emit
 *       trussCreated/trussLoaded/projectSaved signals.
 */
class ProjectController : public QObject {
public:
    explicit ProjectController(QObject* parent = nullptr) : QObject(parent) {}

    // TODO Phase 5: explicit ProjectController(application::ITrussService* service,
    //                                           QObject* parent = nullptr)
    // TODO Phase 5: public slots:
    //   void onNewProjectRequested()
    //   void onOpenFileRequested()
    //   void onSaveRequested()
    //   void onSaveAsRequested()
    //   void onTrussHandleUpdated(std::size_t trussHandle)
    // TODO Phase 5: signals:
    //   void trussCreated(std::size_t trussHandle)
    //   void trussLoaded(std::size_t trussHandle, const QString& filePath)
    //   void projectSaved(const QString& filePath)
    //   void operationFailed(const QString& errorMessage)
};

}  // namespace truss::gui::ctrl
