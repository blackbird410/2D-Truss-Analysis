/**
 * @file test_export_controller.cpp
 * @brief Unit tests for ExportController (Phase 5).
 *
 * Verifies:
 *  - Export request calls facade.exportResults and emits exportCompleted.
 *  - No results handle emits exportFailed without calling facade.
 *  - Facade returning false emits exportFailed.
 *  - resultsHandle is updated via onResultsHandleUpdated.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/export_controller.hpp"
#include "infrastructure/export/export_types.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"
#include "truss/export/export_format.hpp"

#include <QCoreApplication>
#include <QSignalSpy>

#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::gui::ctrl;
using ::testing::_;
using ::testing::An;
using ::testing::NiceMock;
using ::testing::Return;
using truss::ExportFormat;
using truss::application::ResultsHandle;
using truss::infrastructure::export_::ExportOptions;
using truss::test::MockTrussAnalysisFacade;

// ============================================================
// QCoreApplication bootstrap
// ExportController is a QObject subclass with no widget or display
// dependency. QCoreApplication is sufficient; no Qt platform plugin is
// loaded, so this binary runs without a display server in CI.
// ============================================================

namespace {
QCoreApplication& ensureQApp() {
    static int s_argc = 1;
    static char s_argv0[] = "unit_tests";
    static char* s_argv[] = {s_argv0, nullptr};
    static QCoreApplication* s_app = []() -> QCoreApplication* {
        if (QCoreApplication::instance())
            return QCoreApplication::instance();
        return new QCoreApplication(s_argc, s_argv);
    }();
    return *s_app;
}
}  // namespace

// ============================================================
// Fixture
// ============================================================

class ExportControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        ensureQApp();
        ctrl = std::make_unique<ExportController>(facade);
        ctrl->onResultsHandleUpdated(kResultsHandle);
    }
    void TearDown() override { ctrl.reset(); }

    static constexpr std::size_t kResultsHandle = 5;

    NiceMock<MockTrussAnalysisFacade> facade;
    std::unique_ptr<ExportController> ctrl;
};

// ============================================================
// Tests
// ============================================================

TEST_F(ExportControllerTest, Export_Success_CallsFacadeAndEmitsCompleted) {
    // Disambiguate the 4-arg bool overload:
    // bool exportResults(handle, format, const path&, const ExportOptions&)
    EXPECT_CALL(facade,
                exportResults(kResultsHandle,
                              ExportFormat::CSV,
                              An<const std::filesystem::path&>(),
                              An<const ExportOptions&>()))
        .WillOnce(Return(true));

    QSignalSpy spyOk{ctrl.get(), &ExportController::exportCompleted};
    QSignalSpy spyFail{ctrl.get(), &ExportController::exportFailed};
    ctrl->onExportRequested(ExportFormat::CSV, QStringLiteral("/tmp/output.csv"));

    EXPECT_EQ(spyOk.count(), 1);
    EXPECT_EQ(spyFail.count(), 0);
}

TEST_F(ExportControllerTest, Export_FacadeReturnsFalse_EmitsExportFailed) {
    EXPECT_CALL(
        facade,
        exportResults(
            kResultsHandle, _, An<const std::filesystem::path&>(), An<const ExportOptions&>()))
        .WillOnce(Return(false));

    QSignalSpy spy{ctrl.get(), &ExportController::exportFailed};
    ctrl->onExportRequested(ExportFormat::CSV, QStringLiteral("/tmp/output.csv"));

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ExportControllerTest, Export_NoResultsHandle_EmitsExportFailedWithoutCallingFacade) {
    ctrl->onResultsHandleUpdated(0);

    EXPECT_CALL(facade,
                exportResults(_, _, An<const std::filesystem::path&>(), An<const ExportOptions&>()))
        .Times(0);

    QSignalSpy spy{ctrl.get(), &ExportController::exportFailed};
    ctrl->onExportRequested(ExportFormat::CSV, QStringLiteral("/tmp/output.csv"));

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ExportControllerTest, ResultsHandleUpdateChangesTarget) {
    ctrl->onResultsHandleUpdated(99);

    EXPECT_CALL(facade,
                exportResults(std::size_t{99},
                              ExportFormat::JSON,
                              An<const std::filesystem::path&>(),
                              An<const ExportOptions&>()))
        .WillOnce(Return(true));

    QSignalSpy spy{ctrl.get(), &ExportController::exportCompleted};
    ctrl->onExportRequested(ExportFormat::JSON, QStringLiteral("/tmp/out.json"));

    EXPECT_EQ(spy.count(), 1);
}
