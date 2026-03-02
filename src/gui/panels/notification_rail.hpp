/**
 * @file notification_rail.hpp
 * @brief Non-blocking inline notification widget for user feedback.
 *
 * Phase 1 stub — class declaration only.
 * Full implementation in Phase 2 (Qt infrastructure setup).
 *
 * @note Q_OBJECT is added in Phase 2.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QWidget>

namespace truss::gui {

/**
 * @brief Non-modal notification strip shown below the main toolbar.
 *
 * Displays up to 3 simultaneous typed notifications in a QVBoxLayout with
 * slide-in animation (QPropertyAnimation, 150ms on maximumHeight) and
 * auto-dismiss timer (4000ms for Info/Success/Warning; disabled for Error).
 *
 * Severity types determine badge colour per the design system:
 *  - Info:    --accent-primary
 *  - Success: --status-ok
 *  - Warning: --status-warning
 *  - Error:   --status-error (never auto-dismissed)
 *
 * @todo Phase 2: Add Q_OBJECT macro, implement all four show* public slots,
 *       QPropertyAnimation for slide-in, QTimer for auto-dismiss,
 *       close/dismiss button per notification item.
 */
class NotificationRail : public QWidget {
public:
    explicit NotificationRail(QWidget* parent = nullptr) : QWidget(parent) {}

    // TODO Phase 2: public slots:
    //   void showInfo(const QString& message)
    //   void showSuccess(const QString& message)
    //   void showWarning(const QString& message)
    //   void showError(const QString& message)
    //   void clearAll()
};

}  // namespace truss::gui
