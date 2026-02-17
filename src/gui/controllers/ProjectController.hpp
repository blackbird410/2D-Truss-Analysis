/**
 * @file ProjectController.hpp
 * @brief Controller for managing project file lifecycle
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * This controller manages project file operations:
 * - New project creation
 * - Project file loading (JSON/XML/CSV/etc.)
 * - Project file saving
 * - Save As operations
 * - Unsaved changes tracking
 * - Project close with confirmation
 * 
 * Coordinates with TrussApplicationService for file I/O operations.
 * 
 * Architecture: GUI Controller Layer (MVP Pattern)
 * Dependencies: TrussApplicationService
 */

#pragma once

#include <QObject>
#include <QString>
#include "application/TrussApplicationService.hpp"

namespace truss_controllers {

/**
 * @brief Manages project file lifecycle
 * 
 * This controller handles new/open/save operations for truss projects,
 * coordinating with TrussApplicationService for I/O operations and
 * managing current project state.
 */
class ProjectController : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Construct ProjectController
     * 
     * @param trussService Reference to TrussApplicationService
     * @param parent Qt parent object
     */
    explicit ProjectController(
        truss::application::TrussApplicationService& trussService,
        QObject* parent = nullptr);
    
    /**
     * @brief Get current project filepath
     * 
     * @return QString Current filepath (empty if unsaved new project)
     */
    QString getCurrentFilepath() const { return m_currentFilepath; }
    
    /**
     * @brief Get current truss handle
     * 
     * @return truss::application::TrussHandle Current handle
     */
    truss::application::TrussHandle getCurrentTruss() const { return m_currentHandle; }
    
    /**
     * @brief Check if project has unsaved changes
     * 
     * @return true if unsaved changes exist
     */
    bool hasUnsavedChanges() const { return m_hasUnsavedChanges; }
    
    /**
     * @brief Mark project as having unsaved changes
     */
    void markAsModified();
    
    /**
     * @brief Mark project as saved
     */
    void markAsSaved();

public slots:
    /**
     * @brief Handle request to create new project
     */
    void onNewProject();
    
    /**
     * @brief Handle request to open existing project
     * 
     * @param filepath Path to project file
     */
    void onOpenProject(const QString& filepath);
    
    /**
     * @brief Handle request to save current project
     * 
     * Saves to current filepath. If no filepath exists, emits saveAsRequested.
     */
    void onSaveProject();
    
    /**
     * @brief Handle request to save project to new location
     * 
     * @param filepath Path to save project file
     */
    void onSaveProjectAs(const QString& filepath);
    
    /**
     * @brief Handle request to close current project
     * 
     * Checks for unsaved changes and prompts if needed.
     */
    void onCloseProject();

signals:
    /**
     * @brief Emitted when project is opened successfully
     * 
     * @param handle Handle to opened truss
     * @param filepath Path to opened file
     */
    void projectOpened(truss::application::TrussHandle handle, const QString& filepath);
    
    /**
     * @brief Emitted when project is saved successfully
     * 
     * @param filepath Path where project was saved
     */
    void projectSaved(const QString& filepath);
    
    /**
     * @brief Emitted when project is closed
     */
    void projectClosed();
    
    /**
     * @brief Emitted when new project is created
     * 
     * @param handle Handle to new empty truss
     */
    void projectCreated(truss::application::TrussHandle handle);
    
    /**
     * @brief Emitted when operation fails
     * 
     * @param errorMessage User-friendly error message
     */
    void operationFailed(const QString& errorMessage);
    
    /**
     * @brief Emitted when Save As dialog is needed
     * 
     * View should show file dialog and call onSaveProjectAs with result.
     */
    void saveAsRequested();
    
    /**
     * @brief Emitted when unsaved changes confirmation is needed
     * 
     * View should show dialog asking user to save/discard/cancel.
     */
    void unsavedChangesConfirmationRequested();
    
    /**
     * @brief Emitted to update status message
     * 
     * @param message Status message
     */
    void statusMessageChanged(const QString& message);

private:
    truss::application::TrussApplicationService& m_trussService;
    truss::application::TrussHandle m_currentHandle;
    QString m_currentFilepath;
    bool m_hasUnsavedChanges;
    
    /**
     * @brief Save project to specified filepath
     * 
     * @param filepath Path to save to
     * @return true if successful
     */
    bool saveToFile(const QString& filepath);
};

}  // namespace truss_controllers
