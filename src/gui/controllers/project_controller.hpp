/**
 * @file project_controller.hpp
 * @brief ProjectController (truss::gui::ctrl namespace) replacing the
 *        legacy truss_controllers::ProjectController from Phase 8 onward.
 *
 * Phase 5: Full Q_OBJECT implementation with IConfirmationProvider.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#pragma once

#include "gui/interfaces/iconfirmation_provider.hpp"

#include <QObject>
#include <QString>

#include <cstddef>

namespace truss::interface {
class ITrussAnalysisFacade;
}

namespace truss::gui::ctrl {

/**
 * @brief Manages project lifecycle: new, open, save, save-as.
 *
 * Guards destructive actions (new project) against unsaved changes using
 * IConfirmationProvider, enabling headless unit tests without modal dialogs.
 * File paths are obtained via QFileDialog in production; tests inject a fake
 * path by instead calling the slot directly.
 */
class ProjectController : public QObject {
    Q_OBJECT

public:
    explicit ProjectController(truss::interface::ITrussAnalysisFacade& facade,
                               truss::gui::interfaces::IConfirmationProvider& confirmProvider,
                               QObject* parent = nullptr);

    /// @brief Update dirty state (true = model has unsaved changes).
    void setDirty(bool dirty) noexcept;

public slots:
    void onNewProjectRequested();
    void onOpenFileRequested();
    void onSaveRequested();
    void onSaveAsRequested();
    /// Update the active truss handle (called by MainWindowController).
    void onTrussHandleUpdated(std::size_t trussHandle);

signals:
    void trussCreated(std::size_t trussHandle);
    void trussLoaded(std::size_t trussHandle, const QString& filePath);
    void projectSaved(const QString& filePath);
    void operationFailed(const QString& errorMessage);

private:
    truss::interface::ITrussAnalysisFacade& m_facade;
    truss::gui::interfaces::IConfirmationProvider& m_confirm;
    std::size_t m_trussHandle{0};
    QString m_currentFilePath;
    bool m_isDirty{false};
};

}  // namespace truss::gui::ctrl
