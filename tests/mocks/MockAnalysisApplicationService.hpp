/**
 * @file MockAnalysisApplicationService.hpp
 * @brief GoogleMock implementation of IAnalysisService for unit testing
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * This mock enables isolated testing of controllers that depend on
 * IAnalysisService without requiring the full AnalysisApplicationService.
 * 
 * Usage Example:
 * @code
 * TEST_F(AnalysisControllerTest, AnalyzeSucceeds) {
 *     MockAnalysisApplicationService mockService;
 *     EXPECT_CALL(mockService, analyze(_, _))
 *         .WillOnce(Return(Result<ResultsHandle>::Success(1)));
 *     // ... test controller behavior
 * }
 * @endcode
 */

#pragma once

#include <gmock/gmock.h>
#include "application/interfaces/IAnalysisService.hpp"
#include "application/Result.hpp"
#include "core/model/Truss.hpp"
#include "core/interfaces/IAnalysisResultsView.hpp"
#include "infrastructure/export/exporter_factory.hpp"

/**
 * @brief GoogleMock implementation of IAnalysisService
 * 
 * Provides mock methods for all IAnalysisService interface methods,
 * enabling behavior verification and isolated unit testing.
 */
class MockAnalysisApplicationService : public truss::application::IAnalysisService {
public:
    ~MockAnalysisApplicationService() override = default;
    
    // Interface methods (6 pure virtuals from IAnalysisService)
    
    MOCK_METHOD(truss::application::Result<truss::application::ResultsHandle>, 
                analyze, 
                (const truss::core::Truss& truss, const truss::core::analysis::AnalysisOptions& options), 
                (override));
    
    MOCK_METHOD(const truss::core::interfaces::IAnalysisResultsView&, 
                getResultsView, 
                (truss::application::ResultsHandle handle), 
                (const, override));
    
    MOCK_METHOD(truss::application::Result<bool>, 
                exportResults, 
                (truss::application::ResultsHandle handle,
                 truss::infrastructure::export_::ExportFormat format,
                 const std::filesystem::path& filepath,
                 const truss::core::Truss& truss,
                 const truss::infrastructure::export_::ExportOptions& options), 
                (override));
    
    MOCK_METHOD(truss::application::Result<bool>, 
                exportResults, 
                (truss::application::ResultsHandle handle,
                 const std::filesystem::path& filepath,
                 const truss::core::Truss& truss,
                 const truss::infrastructure::export_::ExportOptions& options), 
                (override));
    
    MOCK_METHOD(bool, 
                clearResults, 
                (truss::application::ResultsHandle handle), 
                (override));
    
    MOCK_METHOD(void, 
                clearAll, 
                (), 
                (override));
    
    MOCK_METHOD(bool, 
                isValidHandle, 
                (truss::application::ResultsHandle handle), 
                (const, override));
};

