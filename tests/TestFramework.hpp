/**
 * @file TestFramework.hpp
 * @brief Lightweight unit testing framework for the truss analysis software
 * @author Civil Engineering Software Solutions
 * @version 2.2.0
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <exception>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace truss::testing {

/**
 * @brief Test result status
 */
enum class TestStatus {
    Passed,
    Failed,
    Skipped
};

/**
 * @brief Individual test result
 */
struct TestResult {
    std::string name;
    TestStatus status;
    std::string message;
    double executionTime; // in milliseconds
    
    TestResult(const std::string& testName, TestStatus testStatus, 
               const std::string& msg = "", double time = 0.0)
        : name(testName), status(testStatus), message(msg), executionTime(time) {}
};

/**
 * @brief Test suite results
 */
struct TestSuiteResult {
    std::string suiteName;
    std::vector<TestResult> tests;
    double totalTime;
    
    int getPassedCount() const {
        return static_cast<int>(std::count_if(tests.begin(), tests.end(),
            [](const TestResult& r) { return r.status == TestStatus::Passed; }));
    }
    
    int getFailedCount() const {
        return static_cast<int>(std::count_if(tests.begin(), tests.end(),
            [](const TestResult& r) { return r.status == TestStatus::Failed; }));
    }
    
    int getSkippedCount() const {
        return static_cast<int>(std::count_if(tests.begin(), tests.end(),
            [](const TestResult& r) { return r.status == TestStatus::Skipped; }));
    }
};

/**
 * @brief Assertion exception for failed tests
 */
class AssertionException : public std::exception {
private:
    std::string message;
    
public:
    AssertionException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

/**
 * @brief Main test framework class
 */
class TestFramework {
private:
    std::vector<TestSuiteResult> suiteResults;
    TestSuiteResult* currentSuite;
    bool verbose;
    
public:
    TestFramework(bool verboseOutput = true) : currentSuite(nullptr), verbose(verboseOutput) {}
    
    /**
     * @brief Start a new test suite
     */
    void beginSuite(const std::string& suiteName) {
        suiteResults.emplace_back();
        currentSuite = &suiteResults.back();
        currentSuite->suiteName = suiteName;
        currentSuite->totalTime = 0.0;
        
        if (verbose) {
            std::cout << "\n╭─────────────────────────────────────────────────────╮\n";
            std::cout << "│ " << std::setw(51) << std::left << ("Running: " + suiteName) << " │\n";
            std::cout << "╰─────────────────────────────────────────────────────╯\n";
        }
    }
    
    /**
     * @brief Run a test
     */
    void runTest(const std::string& testName, std::function<void()> testFunc) {
        if (!currentSuite) {
            throw std::runtime_error("No test suite active. Call beginSuite() first.");
        }
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        try {
            testFunc();
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            double executionTime = duration.count() / 1000.0; // Convert to milliseconds
            
            currentSuite->tests.emplace_back(testName, TestStatus::Passed, "", executionTime);
            currentSuite->totalTime += executionTime;
            
            if (verbose) {
                std::cout << "  ✅ " << std::setw(45) << std::left << testName 
                         << " (" << std::fixed << std::setprecision(2) << executionTime << " ms)\n";
            }
            
        } catch (const AssertionException& e) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            double executionTime = duration.count() / 1000.0;
            
            currentSuite->tests.emplace_back(testName, TestStatus::Failed, e.what(), executionTime);
            currentSuite->totalTime += executionTime;
            
            if (verbose) {
                std::cout << "  ❌ " << std::setw(45) << std::left << testName 
                         << " (" << std::fixed << std::setprecision(2) << executionTime << " ms)\n";
                std::cout << "     Reason: " << e.what() << "\n";
            }
            
        } catch (const std::exception& e) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            double executionTime = duration.count() / 1000.0;
            
            currentSuite->tests.emplace_back(testName, TestStatus::Failed, 
                                           "Unexpected exception: " + std::string(e.what()), executionTime);
            currentSuite->totalTime += executionTime;
            
            if (verbose) {
                std::cout << "  ❌ " << std::setw(45) << std::left << testName 
                         << " (" << std::fixed << std::setprecision(2) << executionTime << " ms)\n";
                std::cout << "     Unexpected exception: " << e.what() << "\n";
            }
        }
    }
    
    /**
     * @brief Skip a test
     */
    void skipTest(const std::string& testName, const std::string& reason = "") {
        if (!currentSuite) {
            throw std::runtime_error("No test suite active. Call beginSuite() first.");
        }
        
        currentSuite->tests.emplace_back(testName, TestStatus::Skipped, reason, 0.0);
        
        if (verbose) {
            std::cout << "  ⏭️  " << std::setw(45) << std::left << testName << " (skipped)\n";
            if (!reason.empty()) {
                std::cout << "     Reason: " << reason << "\n";
            }
        }
    }
    
    /**
     * @brief Generate summary report
     */
    void generateReport() {
        int totalTests = 0;
        int totalPassed = 0;
        int totalFailed = 0;
        int totalSkipped = 0;
        double totalTime = 0.0;
        
        std::cout << "\n\n╭═════════════════════════════════════════════════════════════════════════════╮\n";
        std::cout << "│                              TEST RESULTS SUMMARY                          │\n";
        std::cout << "╰═════════════════════════════════════════════════════════════════════════════╯\n\n";
        
        for (const auto& suite : suiteResults) {
            int passed = suite.getPassedCount();
            int failed = suite.getFailedCount();
            int skipped = suite.getSkippedCount();
            
            totalTests += static_cast<int>(suite.tests.size());
            totalPassed += passed;
            totalFailed += failed;
            totalSkipped += skipped;
            totalTime += suite.totalTime;
            
            std::string status = (failed == 0) ? "✅ PASS" : "❌ FAIL";
            
            std::cout << "📦 " << std::setw(35) << std::left << suite.suiteName 
                     << " │ " << status << " │ "
                     << passed << "/" << (passed + failed) << " passed";
            
            if (skipped > 0) {
                std::cout << " (" << skipped << " skipped)";
            }
            std::cout << " │ " << std::fixed << std::setprecision(2) << suite.totalTime << " ms\n";
            
            // Show failed tests
            if (failed > 0) {
                for (const auto& test : suite.tests) {
                    if (test.status == TestStatus::Failed) {
                        std::cout << "   └─ ❌ " << test.name << ": " << test.message << "\n";
                    }
                }
            }
        }
        
        std::cout << "\n" << std::string(79, '─') << "\n";
        std::cout << "📊 TOTAL RESULTS:\n";
        std::cout << "   Tests Run: " << totalTests << "\n";
        std::cout << "   ✅ Passed: " << totalPassed << "\n";
        if (totalFailed > 0) {
            std::cout << "   ❌ Failed: " << totalFailed << "\n";
        }
        if (totalSkipped > 0) {
            std::cout << "   ⏭️  Skipped: " << totalSkipped << "\n";
        }
        std::cout << "   ⏱️  Total Time: " << std::fixed << std::setprecision(2) << totalTime << " ms\n";
        
        double successRate = totalTests > 0 ? (static_cast<double>(totalPassed) / totalTests * 100.0) : 0.0;
        std::cout << "   📈 Success Rate: " << std::fixed << std::setprecision(1) << successRate << "%\n";
        
        if (totalFailed == 0) {
            std::cout << "\n🎉 ALL TESTS PASSED! 🎉\n";
        } else {
            std::cout << "\n⚠️  SOME TESTS FAILED - See details above\n";
        }
    }
    
    /**
     * @brief Check if all tests passed
     */
    bool allTestsPassed() const {
        for (const auto& suite : suiteResults) {
            if (suite.getFailedCount() > 0) {
                return false;
            }
        }
        return true;
    }
    
    /**
     * @brief Get total number of failed tests
     */
    int getTotalFailedTests() const {
        int total = 0;
        for (const auto& suite : suiteResults) {
            total += suite.getFailedCount();
        }
        return total;
    }
};

// Assertion macros
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #condition << " (expected true)"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #condition << " (expected false)"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if (!((expected) == (actual))) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #expected << " == " << #actual \
               << " (expected: " << (expected) << ", actual: " << (actual) << ")"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #expected << " != " << #actual \
               << " (both values: " << (expected) << ")"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

#define ASSERT_NEAR(expected, actual, tolerance) \
    do { \
        double diff = std::abs(static_cast<double>(expected) - static_cast<double>(actual)); \
        if (diff > static_cast<double>(tolerance)) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #expected << " ≈ " << #actual \
               << " (expected: " << (expected) << ", actual: " << (actual) \
               << ", tolerance: " << (tolerance) << ", diff: " << diff << ")"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

#define ASSERT_GT(value1, value2) \
    do { \
        if (!((value1) > (value2))) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #value1 << " > " << #value2 \
               << " (" << (value1) << " > " << (value2) << ")"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

#define ASSERT_LT(value1, value2) \
    do { \
        if (!((value1) < (value2))) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #value1 << " < " << #value2 \
               << " (" << (value1) << " < " << (value2) << ")"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

#define ASSERT_THROWS(expression, exception_type) \
    do { \
        bool threw_expected = false; \
        try { \
            (expression); \
        } catch (const exception_type&) { \
            threw_expected = true; \
        } catch (...) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #expression << " threw wrong exception type " \
               << "(expected: " << #exception_type << ")"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
        if (!threw_expected) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #expression << " did not throw " << #exception_type; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

#define ASSERT_NO_THROW(expression) \
    do { \
        try { \
            (expression); \
        } catch (...) { \
            std::stringstream ss; \
            ss << "Assertion failed: " << #expression << " threw an unexpected exception"; \
            throw truss::testing::AssertionException(ss.str()); \
        } \
    } while(0)

} // namespace truss::testing
