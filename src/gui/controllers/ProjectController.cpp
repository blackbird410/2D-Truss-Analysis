#include "ProjectController.hpp"
#include <filesystem>
#include <stdexcept>

namespace truss_controllers {

ProjectController::ProjectController(
    truss::application::ITrussService* trussService,
    QObject* parent)
    : QObject(parent)
    , m_trussService(trussService)
    , m_currentHandle(0)
    , m_currentFilepath()
    , m_hasUnsavedChanges(false)
{
    if (!m_trussService) {
        throw std::invalid_argument("ProjectController: null service pointer");
    }
}

void ProjectController::markAsModified() {
    m_hasUnsavedChanges = true;
}

void ProjectController::markAsSaved() {
    m_hasUnsavedChanges = false;
}

void ProjectController::onNewProject() {
    // Check for unsaved changes
    if (m_hasUnsavedChanges) {
        emit unsavedChangesConfirmationRequested();
        // User will need to respond and call this again if they want to proceed
        return;
    }
    
    // Create new empty truss
    auto result = m_trussService->createTruss("Untitled");
    
    if (result.success) {
        m_currentHandle = result.value;
        m_currentFilepath.clear();
        m_hasUnsavedChanges = false;
        
        emit projectCreated(m_currentHandle);
        emit statusMessageChanged("New project created");
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
    }
}

void ProjectController::onOpenProject(const QString& filepath) {
    // Check for unsaved changes
    if (m_hasUnsavedChanges) {
        emit unsavedChangesConfirmationRequested();
        return;
    }
    
    // Check if file exists
    if (!std::filesystem::exists(filepath.toStdString())) {
        emit operationFailed(QString("File not found: %1").arg(filepath));
        return;
    }
    
    emit statusMessageChanged("Opening project...");
    
    // Load truss from file
    auto result = m_trussService->loadTruss(filepath.toStdString());
    
    if (result.success) {
        m_currentHandle = result.value;
        m_currentFilepath = filepath;
        m_hasUnsavedChanges = false;
        
        emit projectOpened(m_currentHandle, filepath);
        emit statusMessageChanged(QString("Project opened: %1").arg(filepath));
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
        emit statusMessageChanged("Failed to open project");
    }
}

void ProjectController::onSaveProject() {
    if (m_currentHandle == 0) {
        emit operationFailed("No project to save");
        return;
    }
    
    // If no filepath exists, request Save As
    if (m_currentFilepath.isEmpty()) {
        emit saveAsRequested();
        return;
    }
    
    // Save to existing filepath
    if (saveToFile(m_currentFilepath)) {
        emit projectSaved(m_currentFilepath);
        emit statusMessageChanged(QString("Project saved: %1").arg(m_currentFilepath));
    }
}

void ProjectController::onSaveProjectAs(const QString& filepath) {
    if (m_currentHandle == 0) {
        emit operationFailed("No project to save");
        return;
    }
    
    if (filepath.isEmpty()) {
        emit operationFailed("No filepath specified");
        return;
    }
    
    if (saveToFile(filepath)) {
        m_currentFilepath = filepath;
        emit projectSaved(filepath);
        emit statusMessageChanged(QString("Project saved: %1").arg(filepath));
    }
}

void ProjectController::onCloseProject() {
    // Check for unsaved changes
    if (m_hasUnsavedChanges) {
        emit unsavedChangesConfirmationRequested();
        return;
    }
    
    // Clear current state
    m_currentHandle = 0;
    m_currentFilepath.clear();
    m_hasUnsavedChanges = false;
    
    emit projectClosed();
    emit statusMessageChanged("Project closed");
}

bool ProjectController::saveToFile(const QString& filepath) {
    emit statusMessageChanged("Saving project...");
    
    auto result = m_trussService->saveTruss(
        m_currentHandle,
        filepath.toStdString()
    );
    
    if (result.success) {
        m_hasUnsavedChanges = false;
        return true;
    } else {
        emit operationFailed(QString::fromStdString(result.errorMessage));
        emit statusMessageChanged("Save failed");
        return false;
    }
}

}  // namespace truss_controllers
