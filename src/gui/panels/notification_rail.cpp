/**
 * @file notification_rail.cpp
 * @brief Non-blocking inline notification widget — Phase 2 full implementation.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "notification_rail.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace truss::gui {

// =============================================================================
// NotificationItem — internal widget defined in this translation unit only.
// AUTOMOC processes Q_OBJECT classes in .cpp files when the moc file is
// explicitly included at the bottom via: #include "notification_rail.moc"
// =============================================================================
class NotificationItem : public QWidget {
    Q_OBJECT

public:
    explicit NotificationItem(NotificationSeverity severity,
                              const QString&        message,
                              QWidget*              parent = nullptr)
        : QWidget(parent)
    {
        setObjectName(objectNameForSeverity(severity));
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        // Start collapsed; animation expands to kItemHeight.
        setMaximumHeight(0);

        auto* row = new QHBoxLayout(this);
        row->setContentsMargins(10, 0, 8, 0);
        row->setSpacing(8);

        m_label = new QLabel(message, this);
        m_label->setWordWrap(false);
        m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        m_closeBtn = new QPushButton(QStringLiteral("\u00D7"), this);
        m_closeBtn->setFixedSize(20, 20);
        m_closeBtn->setFocusPolicy(Qt::NoFocus);
        m_closeBtn->setFlat(true);
        m_closeBtn->setToolTip(QStringLiteral("Dismiss"));
        m_closeBtn->setCursor(Qt::PointingHandCursor);

        row->addWidget(m_label, 1);
        row->addWidget(m_closeBtn, 0);

        connect(m_closeBtn, &QPushButton::clicked, this, &NotificationItem::closeRequested);
    }

    /// Update the displayed text without rebuilding the widget.
    void setMessage(const QString& message) { m_label->setText(message); }

Q_SIGNALS:
    void closeRequested();

private:
    static QString objectNameForSeverity(NotificationSeverity s) noexcept
    {
        switch (s) {
            case NotificationSeverity::Info:    return QStringLiteral("notifItem_info");
            case NotificationSeverity::Success: return QStringLiteral("notifItem_success");
            case NotificationSeverity::Warning: return QStringLiteral("notifItem_warning");
            case NotificationSeverity::Error:   return QStringLiteral("notifItem_error");
        }
        return QStringLiteral("notifItem_info");
    }

    QLabel*      m_label{nullptr};
    QPushButton* m_closeBtn{nullptr};
};

// =============================================================================
// NotificationRail
// =============================================================================

NotificationRail::NotificationRail(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("notificationRail"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setVisible(false); // Hidden when no notifications are active.

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(2);
}

NotificationRail::~NotificationRail() = default;

// ---------------------------------------------------------------------------
// Public methods
// ---------------------------------------------------------------------------

int NotificationRail::activeCount() const noexcept
{
    int count = 0;
    for (const auto& ptr : m_items) {
        if (ptr) {
            ++count;
        }
    }
    return count;
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void NotificationRail::showInfo(const QString& message)
{
    addNotification(NotificationSeverity::Info, message);
}

void NotificationRail::showSuccess(const QString& message)
{
    addNotification(NotificationSeverity::Success, message);
}

void NotificationRail::showWarning(const QString& message)
{
    addNotification(NotificationSeverity::Warning, message);
}

void NotificationRail::showError(const QString& message)
{
    addNotification(NotificationSeverity::Error, message);
}

void NotificationRail::clearAll()
{
    // Collect raw pointers before modifying m_items.
    QList<QWidget*> toRemove;
    toRemove.reserve(m_items.size());
    for (auto& ptr : m_items) {
        if (ptr) {
            toRemove.append(ptr.data());
        }
    }
    m_items.clear();
    for (auto* w : toRemove) {
        w->deleteLater();
        Q_EMIT notificationDismissed();
    }
    setVisible(false);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void NotificationRail::addNotification(NotificationSeverity severity,
                                        const QString&        message)
{
    // Evict the oldest item if already at the limit.
    while (m_items.size() >= kMaxItems) {
        removeItem(m_items.first().data());
    }

    auto* item = new NotificationItem(severity, message, this);
    m_layout->addWidget(item);
    m_items.append(QPointer<QWidget>(item));

    // Close button → immediate removal.
    connect(item, &NotificationItem::closeRequested,
            this, [this, item]() { removeItem(item); });

    // Auto-dismiss timer for non-Error severity.
    if (severity != NotificationSeverity::Error) {
        QTimer::singleShot(kAutoDismissMs, this, [this, item]() {
            removeItem(item);
        });
    }

    // Slide-in animation: maximumHeight 0 → kItemHeight over kAnimDurationMs.
    auto* anim = new QPropertyAnimation(item, "maximumHeight", item);
    anim->setDuration(kAnimDurationMs);
    anim->setStartValue(0);
    anim->setEndValue(kItemHeight);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // Ensure the rail is visible once we have content.
    setVisible(true);
}

void NotificationRail::removeItem(QWidget* item)
{
    if (!item) {
        return;
    }

    // Remove from tracking list.
    m_items.removeIf([item](const QPointer<QWidget>& p) {
        return p.data() == item;
    });

    item->deleteLater();
    Q_EMIT notificationDismissed();

    // Hide the rail when no more items remain.
    if (m_items.isEmpty()) {
        setVisible(false);
    }
}

} // namespace truss::gui

// Required for AUTOMOC to process NotificationItem defined in this .cpp file.
#include "notification_rail.moc"
