/**
 * @file test_console_presenter.cpp
 * @brief Unit tests for ConsolePresenter.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Tests verify that ConsolePresenter correctly formats output and delegates
 * to IApplicationOutput abstraction without adding redundant prefixes.
 *
 * Test Coverage:
 * - displayInfo() passes message without "INFO:" prefix
 * - displayError() passes message without "ERROR:" prefix
 * - displaySuccess() passes message without "SUCCESS:" prefix
 * - displayHeader() formats header correctly
 * - No direct I/O operations (all delegated)
 */

#include "../../../src/application/interfaces/iapplication_output.hpp"
#include "../../../src/cli/presenters/console_presenter.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::cli::presenters;
using namespace truss::application::interfaces;
using ::testing::_;
using ::testing::AllOf;
using ::testing::AtLeast;
using ::testing::HasSubstr;
using ::testing::Not;

/**
 * @brief Mock IApplicationOutput for testing presenter delegation
 */
class MockApplicationOutput : public IApplicationOutput {
public:
    MOCK_METHOD(void, info, (const std::string& message), (override));
    MOCK_METHOD(void, success, (const std::string& message), (override));
    MOCK_METHOD(void, error, (const std::string& message), (override));
    MOCK_METHOD(void, warn, (const std::string& message), (override));
};

/**
 * @brief Test fixture for ConsolePresenter tests
 */
class ConsolePresenterTest : public ::testing::Test {
protected:
    MockApplicationOutput mockOutput;
};

/**
 * @test Verify displayInfo() passes message without "INFO:" prefix
 *
 * Rationale: Logger already adds [INFO] tag, adding "INFO:" creates
 * redundant output like "[INFO] INFO: message"
 */
TEST_F(ConsolePresenterTest, DisplayInfo_NoRedundantPrefix) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, info("Test message")).Times(1);

    presenter.displayInfo("Test message");
}

/**
 * @test Verify displayInfo() does NOT add "INFO:" prefix
 */
TEST_F(ConsolePresenterTest, DisplayInfo_MessagePassedVerbatim) {
    ConsolePresenter presenter(mockOutput);

    // Verify message does NOT contain "INFO:" prefix
    EXPECT_CALL(mockOutput, info(Not(HasSubstr("INFO:")))).Times(1);

    presenter.displayInfo("Usage:");
}

/**
 * @test Verify displayError() passes message without "ERROR:" prefix
 */
TEST_F(ConsolePresenterTest, DisplayError_NoRedundantPrefix) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, error("Invalid input")).Times(1);

    presenter.displayError("Invalid input");
}

/**
 * @test Verify displayError() does NOT add "ERROR:" prefix
 */
TEST_F(ConsolePresenterTest, DisplayError_MessagePassedVerbatim) {
    ConsolePresenter presenter(mockOutput);

    // Verify message does NOT contain "ERROR:" prefix
    EXPECT_CALL(mockOutput, error(Not(HasSubstr("ERROR:")))).Times(1);

    presenter.displayError("Command failed");
}

/**
 * @test Verify displaySuccess() passes message without "SUCCESS:" prefix
 */
TEST_F(ConsolePresenterTest, DisplaySuccess_NoRedundantPrefix) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, success("Analysis complete")).Times(1);

    presenter.displaySuccess("Analysis complete");
}

/**
 * @test Verify displaySuccess() does NOT add "SUCCESS:" prefix
 */
TEST_F(ConsolePresenterTest, DisplaySuccess_MessagePassedVerbatim) {
    ConsolePresenter presenter(mockOutput);

    // Verify message does NOT contain "SUCCESS:" prefix
    EXPECT_CALL(mockOutput, success(Not(HasSubstr("SUCCESS:")))).Times(1);

    presenter.displaySuccess("Operation succeeded");
}

/**
 * @test Verify displayHeader() formats header correctly
 */
TEST_F(ConsolePresenterTest, DisplayHeader_FormatsCorrectly) {
    ConsolePresenter presenter(mockOutput);

    // Verify each line is called separately with expected content
    // Note: The separator line appears twice (top and bottom)
    EXPECT_CALL(mockOutput, info("================================================="))
        .Times(2);  // Called twice - top and bottom separator
    EXPECT_CALL(mockOutput, info("       2D Truss Analysis Software v3.0.0       ")).Times(1);
    EXPECT_CALL(mockOutput, info("   Civil Engineering Software Solutions         ")).Times(1);
    EXPECT_CALL(mockOutput, info("")).Times(1);

    presenter.displayHeader();
}

/**
 * @test Verify displayHeader() uses multiple info() calls (one per line)
 */
TEST_F(ConsolePresenterTest, DisplayHeader_MultipleLinesCalls) {
    ConsolePresenter presenter(mockOutput);

    // Should make 5 separate calls to info() - one for each header line plus empty line
    EXPECT_CALL(mockOutput, info(_)).Times(5);

    presenter.displayHeader();
}

/**
 * @test Verify empty message handling
 */
TEST_F(ConsolePresenterTest, DisplayInfo_EmptyMessageHandled) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, info("")).Times(1);

    presenter.displayInfo("");
}

/**
 * @test Verify special characters passed correctly
 */
TEST_F(ConsolePresenterTest, DisplayInfo_SpecialCharactersPreserved) {
    ConsolePresenter presenter(mockOutput);

    const std::string specialMsg = "Test: 100% complete! (✓)";
    EXPECT_CALL(mockOutput, info(specialMsg)).Times(1);

    presenter.displayInfo(specialMsg);
}

/**
 * @test Verify multiple consecutive calls work correctly
 */
TEST_F(ConsolePresenterTest, MultipleDisplayInfoCalls_AllDelegated) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, info("Line 1")).Times(1);
    EXPECT_CALL(mockOutput, info("Line 2")).Times(1);
    EXPECT_CALL(mockOutput, info("Line 3")).Times(1);

    presenter.displayInfo("Line 1");
    presenter.displayInfo("Line 2");
    presenter.displayInfo("Line 3");
}

// ============================================================================
// Extended branch coverage tests — displayTrussStatistics and
// displayAnalysisResults (previously untested)
// ============================================================================

#include "../../../src/core/interfaces/ianalysis_results_view.hpp"
#include "../../../src/core/interfaces/itruss_view.hpp"

using truss::core::Real;
using truss::core::interfaces::IAnalysisResultsView;
using truss::core::interfaces::ITrussView;

// ---- Simple stubs (avoid GMock teardown issues for ref-returning methods) ----

struct StubTrussView : ITrussView {
    std::string name{"Bridge"};
    const std::string& getName() const override { return name; }
    std::vector<truss::core::interfaces::NodeView> getNodeViews() const override { return {}; }
    size_t getNodeCount() const override { return 4; }
    std::vector<truss::core::interfaces::MemberView> getMemberViews() const override { return {}; }
    size_t getMemberCount() const override { return 5; }
    size_t getTotalDofs() const override { return 8; }
    size_t getFreeDofs() const override { return 6; }
    size_t getConstrainedDofs() const override { return 2; }
};

struct StubResultsView : IAnalysisResultsView {
    bool converged{true};
    std::vector<Real> forces{5000.0, -3000.0};
    std::vector<Real> stresses{1e6, -0.6e6};
    std::vector<Real> utilizations{0.4, 0.24};
    std::vector<Real> displacements{0.001, -0.002, 0.0, 0.0};
    std::vector<Real> reactions{};
    std::vector<std::vector<Real>> stiffness{};

    const std::vector<Real>& getDisplacements() const override { return displacements; }
    const std::vector<Real>& getReactions() const override { return reactions; }
    const std::vector<Real>& getMemberForces() const override { return forces; }
    const std::vector<Real>& getMemberStresses() const override { return stresses; }
    const std::vector<Real>& getUtilizationRatios() const override { return utilizations; }
    const std::vector<std::vector<Real>>& getStiffnessMatrix() const override { return stiffness; }
    bool hasConverged() const override { return converged; }
    int getIterations() const override { return 1; }
    Real getResidualNorm() const override { return 0.0; }
    Real getConditionNumber() const override { return 100.0; }
    size_t getTotalDofs() const override { return 4; }
    size_t getFreeDofs() const override { return 2; }
    size_t getConstrainedDofs() const override { return 2; }
    Real getTotalStrain() const override { return 1e-5; }
    Real getMaxDisplacement() const override { return 0.002; }
    Real getMaxStress() const override { return 1e6; }
};

/**
 * @test displayTrussStatistics calls info() for each statistic field
 */
TEST_F(ConsolePresenterTest, DisplayTrussStatistics_OutputsAllFields) {
    ConsolePresenter presenter(mockOutput);
    StubTrussView trussView;


    // Expect multiple info() calls covering all fields
    EXPECT_CALL(mockOutput, info(_)).Times(::testing::AnyNumber());  // catch-all for unlisted calls
    EXPECT_CALL(mockOutput, info(HasSubstr("Truss Statistics"))).Times(1);
    EXPECT_CALL(mockOutput, info(HasSubstr("Bridge"))).Times(1);
    EXPECT_CALL(mockOutput, info(HasSubstr("4"))).Times(::testing::AtLeast(1));
    EXPECT_CALL(mockOutput, info(HasSubstr("5"))).Times(::testing::AtLeast(1));

    presenter.displayTrussStatistics(trussView);
}

/**
 * @test displayAnalysisResults — converged case with tension and compression members
 */
TEST_F(ConsolePresenterTest, DisplayAnalysisResults_Converged_TensionAndCompression) {
    ConsolePresenter presenter(mockOutput);
    StubResultsView resultsView;  // converged=true, forces={5000,-3000}

    // Converged → "Yes"; force > 0 → "(Tension)"; force < 0 → "(Compression)"
    EXPECT_CALL(mockOutput, info(_)).Times(::testing::AnyNumber());  // catch-all
    EXPECT_CALL(mockOutput, info(HasSubstr("Yes"))).Times(1);
    EXPECT_CALL(mockOutput, info(HasSubstr("Tension"))).Times(1);
    EXPECT_CALL(mockOutput, info(HasSubstr("Compression"))).Times(1);

    presenter.displayAnalysisResults(resultsView);
}

/**
 * @test displayAnalysisResults — not-converged path covers the "No" branch
 */
TEST_F(ConsolePresenterTest, DisplayAnalysisResults_NotConverged_ShowsNo) {
    ConsolePresenter presenter(mockOutput);
    StubResultsView resultsView;
    resultsView.converged = false;
    resultsView.forces.clear();
    resultsView.stresses.clear();
    resultsView.utilizations.clear();
    resultsView.displacements.clear();

    EXPECT_CALL(mockOutput, info(_)).Times(::testing::AnyNumber());  // catch-all
    EXPECT_CALL(mockOutput, info(HasSubstr("Converged: No"))).Times(1);

    presenter.displayAnalysisResults(resultsView);
}

/**
 * @test displayAnalysisResults with 8 displacements triggers std::min cap to 6
 *
 * Covers the branch where displacements.size() > 6, so numDofs is capped at 6
 * and only 3 nodes (DOFs 0-5) are displayed.
 */
TEST_F(ConsolePresenterTest, DisplayAnalysisResults_ManyDisplacements_Cap6Dofs) {
    ConsolePresenter presenter(mockOutput);
    StubResultsView resultsView;
    // 8 displacements → std::min(8, 6) = 6, only 3 nodes shown
    resultsView.displacements = {0.001, -0.002, 0.003, -0.004, 0.005, -0.006, 0.007, -0.008};
    resultsView.forces = {1000.0};
    resultsView.stresses = {5e5};
    resultsView.utilizations = {0.5};

    EXPECT_CALL(mockOutput, info(_)).Times(::testing::AnyNumber());
    EXPECT_CALL(mockOutput, info(HasSubstr("Node 3"))).Times(1);

    presenter.displayAnalysisResults(resultsView);
}

/**
 * @test displayAnalysisResults with zero-force member covers the force==0 path.
 *
 * force == 0 is NOT > 0, so it falls through to the "(Compression)" label.
 */
TEST_F(ConsolePresenterTest, DisplayAnalysisResults_ZeroForceMember_ShowsCompression) {
    ConsolePresenter presenter(mockOutput);
    StubResultsView resultsView;
    resultsView.forces = {0.0};
    resultsView.stresses = {0.0};
    resultsView.utilizations = {0.0};
    resultsView.displacements = {};

    EXPECT_CALL(mockOutput, info(_)).Times(::testing::AnyNumber());
    EXPECT_CALL(mockOutput, info(HasSubstr("Compression"))).Times(1);

    presenter.displayAnalysisResults(resultsView);
}
