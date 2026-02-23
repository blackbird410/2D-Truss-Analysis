/**
 * @file DataTableWidget.hpp
 * @brief Widget for displaying truss data in tables (Clean Architecture compliant)
 */

#pragma once

#include "application/truss_application_service.hpp"

#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

namespace truss::gui {

/**
 * @brief Widget for displaying truss data in tabular format
 *
 * This widget follows Clean Architecture principles:
 * - Uses dependency injection for services
 * - Does not own or directly access Domain entities
 * - Accesses data through interface abstraction (ITrussView)
 * - Read-only widget
 */
class DataTableWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construct DataTableWidget with dependency injection
     * @param trussService Application service for truss data
     * @param parent Qt parent widget
     */
    explicit DataTableWidget(application::TrussApplicationService& trussService,
                             QWidget* parent = nullptr);

public slots:
    /**
     * @brief Update all data tables for a specific truss
     * @param trussHandle Handle to the truss to display
     */
    void updateTables(application::TrussHandle trussHandle);

    /**
     * @brief Clear all table data
     */
    void clearTables();

private:
    void setupUI();
    void updateNodesTable(application::TrussHandle trussHandle);
    void updateMembersTable(application::TrussHandle trussHandle);
    void updateLoadsTable(application::TrussHandle trussHandle);

    // Service dependency (injected)
    application::TrussApplicationService& m_trussService;

    // Current state
    application::TrussHandle m_currentTrussHandle;

    // UI components
    QTableWidget* m_nodesTable;
    QTableWidget* m_membersTable;
    QTableWidget* m_loadsTable;
};

}  // namespace truss::gui
