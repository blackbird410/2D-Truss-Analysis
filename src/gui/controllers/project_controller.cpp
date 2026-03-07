/**
 * @file project_controller.cpp
 * @brief ProjectController implementation (Phase 5).
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/project_controller.hpp"

#include "interface/itruss_analysis_facade.hpp"

#include <QFileDialog>

namespace truss::gui::ctrl {

ProjectController::ProjectController(truss::interface::ITrussAnalysisFacade& facade,
                                     truss::gui::interfaces::IConfirmationProvider& confirmProvider,
                                     QObject* parent)
    : QObject{parent}, m_facade{facade}, m_confirm{confirmProvider} {}

void ProjectController::setDirty(bool dirty) noexcept {
    m_isDirty = dirty;
}

void ProjectController::onTrussHandleUpdated(std::size_t trussHandle) {
    m_trussHandle = trussHandle;
}

// ---------------------------------------------------------------------------
// New project
// ---------------------------------------------------------------------------

void ProjectController::onNewProjectRequested() {
    if (m_isDirty && m_trussHandle != 0) {
        const bool confirmed = m_confirm.confirm(
            QStringLiteral("Unsaved Changes"),
            QStringLiteral("The current project has unsaved changes.\n"
                           "Discard them and create a new project?"));
        if (!confirmed)
            return;
    }

    if (m_trussHandle != 0) {
        m_facade.clearTruss(m_trussHandle);
    }

    auto result = m_facade.createTruss("Untitled Truss");
    if (result) {
        m_trussHandle = result.value;
        m_currentFilePath.clear();
        m_isDirty = false;
        emit trussCreated(m_trussHandle);
    } else {
        emit operationFailed(
            QString::fromStdString("Failed to create truss: " + result.errorMessage));
    }
}

// ---------------------------------------------------------------------------
// Open file
// ---------------------------------------------------------------------------

void ProjectController::onOpenFileRequested() {
    if (m_isDirty && m_trussHandle != 0) {
        const bool confirmed = m_confirm.confirm(
            QStringLiteral("Unsaved Changes"),
            QStringLiteral("The current project has unsaved changes.\n"
                           "Discard them and open a different file?"));
        if (!confirmed)
            return;
    }

    const QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open Truss Project"),
        QStringLiteral("."),
        QStringLiteral("Truss files (*.json *.xml);;All files (*)"));

    if (filePath.isEmpty())
        return;  // user cancelled

    auto result = m_facade.loadTruss(std::filesystem::path{filePath.toStdString()});
    if (result) {
        if (m_trussHandle != 0) {
            m_facade.clearTruss(m_trussHandle);
        }
        m_trussHandle = result.value;
        m_currentFilePath = filePath;
        m_isDirty = false;
        emit trussLoaded(m_trussHandle, filePath);
    } else {
        emit operationFailed(QString::fromStdString("Failed to load file: " + result.errorMessage));
    }
}

// ---------------------------------------------------------------------------
// Save / Save-as
// ---------------------------------------------------------------------------

void ProjectController::onSaveRequested() {
    if (m_trussHandle == 0)
        return;

    if (m_currentFilePath.isEmpty()) {
        onSaveAsRequested();
        return;
    }

    auto result = m_facade.saveTruss(m_trussHandle,
                                     std::filesystem::path{m_currentFilePath.toStdString()},
                                     /*overwrite=*/true);

    if (result) {
        m_isDirty = false;
        emit projectSaved(m_currentFilePath);
    } else {
        emit operationFailed(QString::fromStdString("Save failed: " + result.errorMessage));
    }
}

void ProjectController::onSaveAsRequested() {
    if (m_trussHandle == 0)
        return;

    const QString filePath = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Save Truss Project"),
        m_currentFilePath.isEmpty() ? QStringLiteral("untitled.json") : m_currentFilePath,
        QStringLiteral("JSON (*.json);;XML (*.xml);;All files (*)"));

    if (filePath.isEmpty())
        return;  // user cancelled

    auto result = m_facade.saveTruss(m_trussHandle,
                                     std::filesystem::path{filePath.toStdString()},
                                     /*overwrite=*/true);

    if (result) {
        m_currentFilePath = filePath;
        m_isDirty = false;
        emit projectSaved(m_currentFilePath);
    } else {
        emit operationFailed(QString::fromStdString("Save failed: " + result.errorMessage));
    }
}

}  // namespace truss::gui::ctrl
