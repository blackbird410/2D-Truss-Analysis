/**
 * @file notification_rail.hpp
 * @brief Non-blocking inline notification widget for user feedback.
 *
 * Phase 2: Full implementation — QPropertyAnimation slide-in, auto-dismiss
 * timer, per-item close button, severity-colour objectName QSS integration.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#pragma once

#include <QPointer>
#include <QWidget>

class QVBoxLayout;

namespace truss::gui {

/**
 * @brief Severity level of a notification banner.
 */
enum class NotificationSeverity : std::uint8_t {
    Info    = 0, ///< Informational (blue accent)
    Success = 1, ///< Operation succeeded (green)
    Warning = 2, ///< Non-critical warning (amber)
    Error   = 3, ///< Error — never auto-dismissed (red)
};

/**
 * @brief Non-modal notification strip shown below the main toolbar.
 *
 * Displays up to kMaxItems simultaneous typed notifications in a QVBoxLayout.
 * Each banner slides in via a QPropertyAnimation (kAnimDurationMs ms,
 * maximumHeight 0 → kItemHeight) and auto-dismisses after kAutoDismissMs ms
 * for Info, Success and Warning severity.  Error banners persist until the
 * user presses the close button or clearAll() is called.
 *
 * QSS object names applied to each item widget:
 *  - notifItem_info    — Info severity
 *  - notifItem_success — Success severity
 *  - notifItem_warning — Warning severity
 *  - notifItem_error   — Error severity
 */
class NotificationRail : public QWidget {
    Q_OBJECT

public:
    explicit NotificationRail(QWidget* parent = nullptr);
    ~NotificationRail() override;

    NotificationRail(const NotificationRail&)            = delete;
    NotificationRail& operator=(const NotificationRail&) = delete;

    /// Maximum number of banners displayed simultaneously.
    static constexpr int kMaxItems       = 3;
    /// Slide-in animation duration in milliseconds.
    static constexpr int kAnimDurationMs = 150;
    /// Auto-dismiss delay (ms) for Info / Success / Warning.
    static constexpr int kAutoDismissMs  = 4000;
    /// Fixed height of a single notification item in pixels.
    static constexpr int kItemHeight     = 52;

    /// Return the number of active notification items currently displayed.
    [[nodiscard]] int activeCount() const noexcept;

Q_SIGNALS:
    /**
     * @brief Emitted each time a notification is removed
     *        (auto-dismiss or user close or clearAll).
     */
    void notificationDismissed();

public Q_SLOTS:
    /// Show an informational banner (auto-dismissed after kAutoDismissMs).
    void showInfo(const QString& message);
    /// Show a success banner (auto-dismissed after kAutoDismissMs).
    void showSuccess(const QString& message);
    /// Show a warning banner (auto-dismissed after kAutoDismissMs).
    void showWarning(const QString& message);
    /// Show an error banner — persists until closed manually.
    void showError(const QString& message);
    /// Remove all active notifications immediately (no animation).
    void clearAll();

private:
    void addNotification(NotificationSeverity severity, const QString& message);
    void removeItem(QWidget* item);

    QVBoxLayout*             m_layout{nullptr};
    QList<QPointer<QWidget>> m_items;
};

} // namespace truss::gui
