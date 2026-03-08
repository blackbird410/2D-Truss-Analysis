/**
 * @file test_notification_rail.cpp
 * @brief Unit tests for NotificationRail (Phase 2 — Qt Infrastructure).
 *
 * Coverage:
 *  - Initial state (hidden, count == 0)
 *  - All four severity show* slots add items and make the widget visible
 *  - Maximum kMaxItems cap: oldest item is evicted when limit is exceeded
 *  - clearAll() removes all items and hides the widget
 *  - notificationDismissed() signal emitted for each removal via clearAll()
 *  - Error notifications persist across the event-loop (no auto-dismiss)
 *
 * Note: Auto-dismiss timer behaviour (kAutoDismissMs = 4 000 ms) is tested
 *       via a dedicated slow test gated behind the TRUSS_SLOW_TESTS macro to
 *       avoid inflating the default test-suite run time.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "gui/panels/notification_rail.hpp"

#include <QApplication>
#include <QSignalSpy>
#include <QTest>

#include <gtest/gtest.h>

using truss::gui::NotificationRail;
using truss::gui::NotificationSeverity;

// ---------------------------------------------------------------------------
// QApplication bootstrap
//
// unit_tests uses GTest::gtest_main, so we cannot replace main().
//
// Important constraints:
//  1. QApplication requires argc >= 1 and a valid argv[0].
//  2. Qt does not support creating a second QApplication after the first has
//     been destroyed.  Using a SetUpTestSuite / TearDownTestSuite pair that
//     news/deletes the instance therefore breaks if ANY other test suite in
//     the same binary does the same thing and runs first.
//
// Solution: use a function-local static so QApplication is constructed exactly
// once on first call and lives until process exit (static storage duration),
// regardless of how many test suites request it.
// ---------------------------------------------------------------------------
namespace {
// Returns a QApplication that lives for the entire process lifetime.
// If another test suite in the same binary already created a QApplication
// (e.g. TrussEditControllerTest), that instance is reused.  This avoids
// the Qt restriction that only one QApplication can exist per process.
QApplication& ensureQApp() {
    static int argc = 1;
    static char argv0[] = "unit_tests";
    static char* argv[] = {argv0, nullptr};
    // Use a pointer so we can conditionally assign an existing instance.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    static QApplication* s_app = []() -> QApplication* {
        if (auto* existing = qobject_cast<QApplication*>(QCoreApplication::instance())) {
            return existing;  // reuse — another suite created it first
        }
        return new QApplication(argc, argv);  // created once, never deleted
    }();
    return *s_app;
}
}  // namespace

class NotificationRailTest : public ::testing::Test {
public:
    static void SetUpTestSuite() {
        ensureQApp();  // idempotent — creates at most once per process
    }

protected:
    void SetUp() override { m_rail = std::make_unique<NotificationRail>(nullptr); }

    void TearDown() override { m_rail.reset(); }

    std::unique_ptr<NotificationRail> m_rail;
};

// ---------------------------------------------------------------------------
// Initial state
// ---------------------------------------------------------------------------

TEST_F(NotificationRailTest, InitialStateIsHiddenAndEmpty) {
    EXPECT_EQ(m_rail->activeCount(), 0);
    EXPECT_FALSE(m_rail->isVisible());
}

TEST_F(NotificationRailTest, ObjectNameIsSet) {
    EXPECT_EQ(m_rail->objectName(), QStringLiteral("notificationRail"));
}

// ---------------------------------------------------------------------------
// Adding notifications
// ---------------------------------------------------------------------------

TEST_F(NotificationRailTest, ShowInfoAddsOneItem) {
    m_rail->showInfo(QStringLiteral("Test info"));
    EXPECT_EQ(m_rail->activeCount(), 1);
}

TEST_F(NotificationRailTest, ShowSuccessAddsOneItem) {
    m_rail->showSuccess(QStringLiteral("Test success"));
    EXPECT_EQ(m_rail->activeCount(), 1);
}

TEST_F(NotificationRailTest, ShowWarningAddsOneItem) {
    m_rail->showWarning(QStringLiteral("Test warning"));
    EXPECT_EQ(m_rail->activeCount(), 1);
}

TEST_F(NotificationRailTest, ShowErrorAddsOneItem) {
    m_rail->showError(QStringLiteral("Test error"));
    EXPECT_EQ(m_rail->activeCount(), 1);
}

TEST_F(NotificationRailTest, RailBecomesVisibleAfterFirstNotification) {
    ASSERT_FALSE(m_rail->isVisible());
    m_rail->showInfo(QStringLiteral("Hello"));
    EXPECT_TRUE(m_rail->isVisible());
}

TEST_F(NotificationRailTest, MultipleNotificationsAccumulate) {
    m_rail->showInfo(QStringLiteral("One"));
    m_rail->showSuccess(QStringLiteral("Two"));
    m_rail->showWarning(QStringLiteral("Three"));
    EXPECT_EQ(m_rail->activeCount(), 3);
}

// ---------------------------------------------------------------------------
// Max-item eviction
// ---------------------------------------------------------------------------

TEST_F(NotificationRailTest, FourthNotificationEvictsOldest) {
    m_rail->showInfo(QStringLiteral("First"));
    m_rail->showInfo(QStringLiteral("Second"));
    m_rail->showInfo(QStringLiteral("Third"));
    ASSERT_EQ(m_rail->activeCount(), NotificationRail::kMaxItems);

    // Adding a fourth item should evict the first, keeping the count at kMaxItems.
    m_rail->showError(QStringLiteral("Fourth — evicts first"));
    EXPECT_EQ(m_rail->activeCount(), NotificationRail::kMaxItems);
}

TEST_F(NotificationRailTest, EvictionEmitsDismissSignal) {
    QSignalSpy spy(m_rail.get(), &NotificationRail::notificationDismissed);

    m_rail->showInfo(QStringLiteral("A"));
    m_rail->showInfo(QStringLiteral("B"));
    m_rail->showInfo(QStringLiteral("C"));
    // kMaxItems reached; next add evicts the oldest.
    m_rail->showInfo(QStringLiteral("D"));

    // Exactly one eviction should have occurred.
    EXPECT_EQ(spy.count(), 1);
}

// ---------------------------------------------------------------------------
// clearAll()
// ---------------------------------------------------------------------------

TEST_F(NotificationRailTest, ClearAllRemovesAllItems) {
    m_rail->showInfo(QStringLiteral("A"));
    m_rail->showSuccess(QStringLiteral("B"));
    m_rail->showWarning(QStringLiteral("C"));
    ASSERT_EQ(m_rail->activeCount(), 3);

    m_rail->clearAll();
    EXPECT_EQ(m_rail->activeCount(), 0);
}

TEST_F(NotificationRailTest, ClearAllHidesRail) {
    m_rail->showInfo(QStringLiteral("Visible"));
    ASSERT_TRUE(m_rail->isVisible());

    m_rail->clearAll();
    EXPECT_FALSE(m_rail->isVisible());
}

TEST_F(NotificationRailTest, ClearAllEmitsDismissSignalForEachItem) {
    QSignalSpy spy(m_rail.get(), &NotificationRail::notificationDismissed);

    m_rail->showInfo(QStringLiteral("A"));
    m_rail->showSuccess(QStringLiteral("B"));
    m_rail->showError(QStringLiteral("C"));
    ASSERT_EQ(m_rail->activeCount(), 3);

    m_rail->clearAll();
    EXPECT_EQ(spy.count(), 3);
}

TEST_F(NotificationRailTest, ClearAllOnEmptyRailDoesNotCrash) {
    ASSERT_EQ(m_rail->activeCount(), 0);
    EXPECT_NO_THROW(m_rail->clearAll());
}

// ---------------------------------------------------------------------------
// Mixed severity
// ---------------------------------------------------------------------------

TEST_F(NotificationRailTest, MixedSeverityCountsCorrectly) {
    m_rail->showInfo(QStringLiteral("Info"));
    m_rail->showError(QStringLiteral("Error"));

    EXPECT_EQ(m_rail->activeCount(), 2);
    EXPECT_TRUE(m_rail->isVisible());

    m_rail->clearAll();

    EXPECT_EQ(m_rail->activeCount(), 0);
    EXPECT_FALSE(m_rail->isVisible());
}

// ---------------------------------------------------------------------------
// Slow / timer tests — only compiled when TRUSS_SLOW_TESTS is defined
// (e.g. cmake -DTRUSS_SLOW_TESTS=ON or individual CI job)
// ---------------------------------------------------------------------------
#ifdef TRUSS_SLOW_TESTS

TEST_F(NotificationRailTest, InfoAutoDismissesAfterTimeout) {
    QSignalSpy spy(m_rail.get(), &NotificationRail::notificationDismissed);

    m_rail->showInfo(QStringLiteral("I will vanish"));
    ASSERT_EQ(m_rail->activeCount(), 1);

    // Wait slightly beyond the auto-dismiss threshold.
    QTest::qWait(NotificationRail::kAutoDismissMs + 500);

    EXPECT_EQ(m_rail->activeCount(), 0);
    EXPECT_FALSE(m_rail->isVisible());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(NotificationRailTest, ErrorDoesNotAutoDismiss) {
    m_rail->showError(QStringLiteral("This should stay"));
    ASSERT_EQ(m_rail->activeCount(), 1);

    // Wait longer than the normal auto-dismiss window.
    QTest::qWait(NotificationRail::kAutoDismissMs + 500);

    EXPECT_EQ(m_rail->activeCount(), 1);
    EXPECT_TRUE(m_rail->isVisible());
}

#endif  // TRUSS_SLOW_TESTS
