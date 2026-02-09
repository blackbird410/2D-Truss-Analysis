/**
 * @file test_exporter_factory.cpp
 * @brief Unit tests for ExporterFactory
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include <gtest/gtest.h>
#include "infrastructure/export/exporter_factory.hpp"
#include "infrastructure/export/csv_exporter.hpp"
#include "infrastructure/export/json_exporter.hpp"
#include "infrastructure/export/xml_exporter.hpp"
#include "infrastructure/export/html_exporter.hpp"
#include "infrastructure/export/latex_exporter.hpp"
#include "infrastructure/export/text_exporter.hpp"
#include <filesystem>
#include <memory>

using namespace truss::infrastructure::export_;
namespace fs = std::filesystem;

/**
 * @brief Test fixture for ExporterFactory tests
 */
class ExporterFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup required - factory is stateless
    }
    
    void TearDown() override {
        // No cleanup required
    }
};

// ============================================================================
// CREATION TESTS
// ============================================================================

/**
 * @brief Test CSV exporter creation
 */
TEST_F(ExporterFactoryTest, CreateCSVExporter) {
    auto exporter = ExporterFactory::create(ExportFormat::CSV);
    ASSERT_NE(exporter, nullptr);
    
    // Verify it's actually a CSVExporter by checking type (via dynamic_cast)
    auto* csvExporter = dynamic_cast<CSVExporter*>(exporter.get());
    EXPECT_NE(csvExporter, nullptr) << "Created exporter should be CSVExporter";
}

/**
 * @brief Test TSV exporter creation (should return CSVExporter)
 */
TEST_F(ExporterFactoryTest, CreateTSVExporter) {
    auto exporter = ExporterFactory::create(ExportFormat::TSV);
    ASSERT_NE(exporter, nullptr);
    
    // TSV format should also create CSVExporter (with different delimiter)
    auto* csvExporter = dynamic_cast<CSVExporter*>(exporter.get());
    EXPECT_NE(csvExporter, nullptr) << "TSV format should create CSVExporter";
}

/**
 * @brief Test JSON exporter creation
 */
TEST_F(ExporterFactoryTest, CreateJSONExporter) {
    auto exporter = ExporterFactory::create(ExportFormat::JSON);
    ASSERT_NE(exporter, nullptr);
    
    auto* jsonExporter = dynamic_cast<JSONExporter*>(exporter.get());
    EXPECT_NE(jsonExporter, nullptr) << "Created exporter should be JSONExporter";
}

/**
 * @brief Test XML exporter creation
 */
TEST_F(ExporterFactoryTest, CreateXMLExporter) {
    auto exporter = ExporterFactory::create(ExportFormat::XML);
    ASSERT_NE(exporter, nullptr);
    
    auto* xmlExporter = dynamic_cast<XMLExporter*>(exporter.get());
    EXPECT_NE(xmlExporter, nullptr) << "Created exporter should be XMLExporter";
}

/**
 * @brief Test HTML exporter creation
 */
TEST_F(ExporterFactoryTest, CreateHTMLExporter) {
    auto exporter = ExporterFactory::create(ExportFormat::HTML);
    ASSERT_NE(exporter, nullptr);
    
    auto* htmlExporter = dynamic_cast<HTMLExporter*>(exporter.get());
    EXPECT_NE(htmlExporter, nullptr) << "Created exporter should be HTMLExporter";
}

/**
 * @brief Test LaTeX exporter creation
 */
TEST_F(ExporterFactoryTest, CreateLaTeXExporter) {
    auto exporter = ExporterFactory::create(ExportFormat::LaTeX);
    ASSERT_NE(exporter, nullptr);
    
    auto* latexExporter = dynamic_cast<LaTeXExporter*>(exporter.get());
    EXPECT_NE(latexExporter, nullptr) << "Created exporter should be LaTeXExporter";
}

/**
 * @brief Test Text exporter creation
 */
TEST_F(ExporterFactoryTest, CreateTextExporter) {
    auto exporter = ExporterFactory::create(ExportFormat::TXT);
    ASSERT_NE(exporter, nullptr);
    
    auto* textExporter = dynamic_cast<TextExporter*>(exporter.get());
    EXPECT_NE(textExporter, nullptr) << "Created exporter should be TextExporter";
}

/**
 * @brief Test all exporters return unique instances
 */
TEST_F(ExporterFactoryTest, UniquenessOfCreatedExporters) {
    auto exporter1 = ExporterFactory::create(ExportFormat::JSON);
    auto exporter2 = ExporterFactory::create(ExportFormat::JSON);
    
    EXPECT_NE(exporter1.get(), exporter2.get())
        << "Factory should create independent instances";
}

/**
 * @brief Test all exporters implement IResultsExporter interface
 */
TEST_F(ExporterFactoryTest, AllExportersImplementInterface) {
    std::vector<ExportFormat> formats = {
        ExportFormat::CSV,
        ExportFormat::TSV,
        ExportFormat::JSON,
        ExportFormat::XML,
        ExportFormat::HTML,
        ExportFormat::LaTeX,
        ExportFormat::TXT
    };
    
    for (const auto& format : formats) {
        auto exporter = ExporterFactory::create(format);
        ASSERT_NE(exporter, nullptr) 
            << "Exporter for format " << static_cast<int>(format) << " should not be null";
        
        // Verify it implements the interface (compile-time check via unique_ptr<IResultsExporter>)
        IResultsExporter* interface = exporter.get();
        EXPECT_NE(interface, nullptr) 
            << "Exporter should implement IResultsExporter interface";
    }
}

// ============================================================================
// FORMAT DETECTION TESTS
// ============================================================================

/**
 * @brief Test CSV format detection
 */
TEST_F(ExporterFactoryTest, DetectCSVFormat) {
    EXPECT_EQ(ExporterFactory::detectFormat("results.csv"), ExportFormat::CSV);
    EXPECT_EQ(ExporterFactory::detectFormat("results.CSV"), ExportFormat::CSV);
    EXPECT_EQ(ExporterFactory::detectFormat("/path/to/results.csv"), ExportFormat::CSV);
}

/**
 * @brief Test TSV format detection
 */
TEST_F(ExporterFactoryTest, DetectTSVFormat) {
    EXPECT_EQ(ExporterFactory::detectFormat("results.tsv"), ExportFormat::TSV);
    EXPECT_EQ(ExporterFactory::detectFormat("results.tab"), ExportFormat::TSV);
    EXPECT_EQ(ExporterFactory::detectFormat("results.TSV"), ExportFormat::TSV);
    EXPECT_EQ(ExporterFactory::detectFormat("results.TAB"), ExportFormat::TSV);
}

/**
 * @brief Test JSON format detection
 */
TEST_F(ExporterFactoryTest, DetectJSONFormat) {
    EXPECT_EQ(ExporterFactory::detectFormat("results.json"), ExportFormat::JSON);
    EXPECT_EQ(ExporterFactory::detectFormat("results.JSON"), ExportFormat::JSON);
    EXPECT_EQ(ExporterFactory::detectFormat("/path/to/results.json"), ExportFormat::JSON);
}

/**
 * @brief Test XML format detection
 */
TEST_F(ExporterFactoryTest, DetectXMLFormat) {
    EXPECT_EQ(ExporterFactory::detectFormat("results.xml"), ExportFormat::XML);
    EXPECT_EQ(ExporterFactory::detectFormat("results.XML"), ExportFormat::XML);
}

/**
 * @brief Test HTML format detection
 */
TEST_F(ExporterFactoryTest, DetectHTMLFormat) {
    EXPECT_EQ(ExporterFactory::detectFormat("results.html"), ExportFormat::HTML);
    EXPECT_EQ(ExporterFactory::detectFormat("results.htm"), ExportFormat::HTML);
    EXPECT_EQ(ExporterFactory::detectFormat("results.HTML"), ExportFormat::HTML);
    EXPECT_EQ(ExporterFactory::detectFormat("results.HTM"), ExportFormat::HTML);
}

/**
 * @brief Test LaTeX format detection
 */
TEST_F(ExporterFactoryTest, DetectLaTeXFormat) {
    EXPECT_EQ(ExporterFactory::detectFormat("results.tex"), ExportFormat::LaTeX);
    EXPECT_EQ(ExporterFactory::detectFormat("results.latex"), ExportFormat::LaTeX);
    EXPECT_EQ(ExporterFactory::detectFormat("results.TEX"), ExportFormat::LaTeX);
    EXPECT_EQ(ExporterFactory::detectFormat("results.LATEX"), ExportFormat::LaTeX);
}

/**
 * @brief Test text format detection
 */
TEST_F(ExporterFactoryTest, DetectTextFormat) {
    EXPECT_EQ(ExporterFactory::detectFormat("results.txt"), ExportFormat::TXT);
    EXPECT_EQ(ExporterFactory::detectFormat("results.TXT"), ExportFormat::TXT);
}

/**
 * @brief Test unknown format defaults to CSV
 */
TEST_F(ExporterFactoryTest, DetectUnknownFormatDefaultsToCSV) {
    EXPECT_EQ(ExporterFactory::detectFormat("results.unknown"), ExportFormat::CSV);
    EXPECT_EQ(ExporterFactory::detectFormat("results.xyz"), ExportFormat::CSV);
    EXPECT_EQ(ExporterFactory::detectFormat("results"), ExportFormat::CSV);
    EXPECT_EQ(ExporterFactory::detectFormat(""), ExportFormat::CSV);
}

/**
 * @brief Test format detection with complex paths
 */
TEST_F(ExporterFactoryTest, DetectFormatWithComplexPaths) {
    EXPECT_EQ(ExporterFactory::detectFormat("/home/user/project.name/results.json"), 
              ExportFormat::JSON);
    EXPECT_EQ(ExporterFactory::detectFormat("../relative/path/to/results.xml"), 
              ExportFormat::XML);
    EXPECT_EQ(ExporterFactory::detectFormat("./results.html"), 
              ExportFormat::HTML);
}

/**
 * @brief Test format detection with filesystem::path objects
 */
TEST_F(ExporterFactoryTest, DetectFormatWithPathObject) {
    fs::path csvPath = "results.csv";
    fs::path jsonPath = "results.json";
    fs::path xmlPath = "results.xml";
    
    EXPECT_EQ(ExporterFactory::detectFormat(csvPath), ExportFormat::CSV);
    EXPECT_EQ(ExporterFactory::detectFormat(jsonPath), ExportFormat::JSON);
    EXPECT_EQ(ExporterFactory::detectFormat(xmlPath), ExportFormat::XML);
}

// ============================================================================
// EXTENSION RETRIEVAL TESTS
// ============================================================================

/**
 * @brief Test extension retrieval for all formats
 */
TEST_F(ExporterFactoryTest, GetExtensionForAllFormats) {
    EXPECT_EQ(ExporterFactory::getExtension(ExportFormat::CSV), ".csv");
    EXPECT_EQ(ExporterFactory::getExtension(ExportFormat::TSV), ".tsv");
    EXPECT_EQ(ExporterFactory::getExtension(ExportFormat::JSON), ".json");
    EXPECT_EQ(ExporterFactory::getExtension(ExportFormat::XML), ".xml");
    EXPECT_EQ(ExporterFactory::getExtension(ExportFormat::HTML), ".html");
    EXPECT_EQ(ExporterFactory::getExtension(ExportFormat::LaTeX), ".tex");
    EXPECT_EQ(ExporterFactory::getExtension(ExportFormat::TXT), ".txt");
}

/**
 * @brief Test extension strings start with dot
 */
TEST_F(ExporterFactoryTest, ExtensionsStartWithDot) {
    std::vector<ExportFormat> formats = {
        ExportFormat::CSV, ExportFormat::TSV, ExportFormat::JSON,
        ExportFormat::XML, ExportFormat::HTML, ExportFormat::LaTeX,
        ExportFormat::TXT
    };
    
    for (const auto& format : formats) {
        std::string ext = ExporterFactory::getExtension(format);
        EXPECT_FALSE(ext.empty()) << "Extension should not be empty";
        EXPECT_EQ(ext[0], '.') << "Extension should start with dot";
    }
}

// ============================================================================
// FORMAT NAME TESTS
// ============================================================================

/**
 * @brief Test format name retrieval for all formats
 */
TEST_F(ExporterFactoryTest, GetFormatNameForAllFormats) {
    EXPECT_EQ(ExporterFactory::getFormatName(ExportFormat::CSV), 
              "Comma-Separated Values");
    EXPECT_EQ(ExporterFactory::getFormatName(ExportFormat::TSV), 
              "Tab-Separated Values");
    EXPECT_EQ(ExporterFactory::getFormatName(ExportFormat::JSON), 
              "JavaScript Object Notation");
    EXPECT_EQ(ExporterFactory::getFormatName(ExportFormat::XML), 
              "Extensible Markup Language");
    EXPECT_EQ(ExporterFactory::getFormatName(ExportFormat::HTML), 
              "HyperText Markup Language");
    EXPECT_EQ(ExporterFactory::getFormatName(ExportFormat::LaTeX), 
              "LaTeX Document");
    EXPECT_EQ(ExporterFactory::getFormatName(ExportFormat::TXT), 
              "Plain Text");
}

/**
 * @brief Test format names are non-empty
 */
TEST_F(ExporterFactoryTest, FormatNamesAreNonEmpty) {
    std::vector<ExportFormat> formats = {
        ExportFormat::CSV, ExportFormat::TSV, ExportFormat::JSON,
        ExportFormat::XML, ExportFormat::HTML, ExportFormat::LaTeX,
        ExportFormat::TXT
    };
    
    for (const auto& format : formats) {
        std::string name = ExporterFactory::getFormatName(format);
        EXPECT_FALSE(name.empty()) 
            << "Format name should not be empty for format " 
            << static_cast<int>(format);
    }
}

// ============================================================================
// INTEGRATION/WORKFLOW TESTS
// ============================================================================

/**
 * @brief Test complete workflow: detect → create → verify type
 */
TEST_F(ExporterFactoryTest, CompleteWorkflowDetectCreateVerify) {
    // Test CSV workflow
    auto csvFormat = ExporterFactory::detectFormat("results.csv");
    auto csvExporter = ExporterFactory::create(csvFormat);
    EXPECT_NE(dynamic_cast<CSVExporter*>(csvExporter.get()), nullptr);
    
    // Test JSON workflow
    auto jsonFormat = ExporterFactory::detectFormat("results.json");
    auto jsonExporter = ExporterFactory::create(jsonFormat);
    EXPECT_NE(dynamic_cast<JSONExporter*>(jsonExporter.get()), nullptr);
    
    // Test XML workflow
    auto xmlFormat = ExporterFactory::detectFormat("results.xml");
    auto xmlExporter = ExporterFactory::create(xmlFormat);
    EXPECT_NE(dynamic_cast<XMLExporter*>(xmlExporter.get()), nullptr);
}

/**
 * @brief Test round-trip: format → extension → detect → format
 */
TEST_F(ExporterFactoryTest, RoundTripFormatExtensionDetect) {
    std::vector<ExportFormat> formats = {
        ExportFormat::CSV, ExportFormat::TSV, ExportFormat::JSON,
        ExportFormat::XML, ExportFormat::HTML, ExportFormat::LaTeX,
        ExportFormat::TXT
    };
    
    for (const auto& originalFormat : formats) {
        // Get extension
        std::string ext = ExporterFactory::getExtension(originalFormat);
        
        // Create path with extension
        std::string path = "test" + ext;
        
        // Detect format from path
        auto detectedFormat = ExporterFactory::detectFormat(path);
        
        // Should match original
        EXPECT_EQ(detectedFormat, originalFormat)
            << "Round-trip failed for format " << static_cast<int>(originalFormat);
    }
}

/**
 * @brief Test factory produces working exporters (smoke test)
 */
TEST_F(ExporterFactoryTest, FactoryProducesWorkingExporters) {
    // This is a smoke test - we just verify exporters can be created
    // and don't crash. Detailed functionality is tested in individual
    // exporter test suites.
    
    std::vector<ExportFormat> formats = {
        ExportFormat::CSV, ExportFormat::JSON, ExportFormat::XML,
        ExportFormat::HTML, ExportFormat::LaTeX, ExportFormat::TXT
    };
    
    for (const auto& format : formats) {
        EXPECT_NO_THROW({
            auto exporter = ExporterFactory::create(format);
            EXPECT_NE(exporter, nullptr);
        }) << "Factory should create exporter without throwing for format " 
           << static_cast<int>(format);
    }
}

// ============================================================================
// DETERMINISM TESTS
// ============================================================================

/**
 * @brief Test factory behavior is deterministic
 */
TEST_F(ExporterFactoryTest, DeterministicBehavior) {
    // Same format should produce same type consistently
    for (int i = 0; i < 10; ++i) {
        auto exporter1 = ExporterFactory::create(ExportFormat::JSON);
        auto exporter2 = ExporterFactory::create(ExportFormat::JSON);
        
        // Should be same concrete type (both JSONExporter)
        EXPECT_EQ(typeid(*exporter1).hash_code(), typeid(*exporter2).hash_code());
        
        // But different instances
        EXPECT_NE(exporter1.get(), exporter2.get());
    }
}

/**
 * @brief Test format detection is case-insensitive
 */
TEST_F(ExporterFactoryTest, FormatDetectionCaseInsensitive) {
    EXPECT_EQ(ExporterFactory::detectFormat("file.csv"), 
              ExporterFactory::detectFormat("file.CSV"));
    EXPECT_EQ(ExporterFactory::detectFormat("file.json"), 
              ExporterFactory::detectFormat("file.JSON"));
    EXPECT_EQ(ExporterFactory::detectFormat("file.xml"), 
              ExporterFactory::detectFormat("file.XML"));
    EXPECT_EQ(ExporterFactory::detectFormat("file.html"), 
              ExporterFactory::detectFormat("file.HTML"));
}

/**
 * @brief Test no side effects from repeated calls
 */
TEST_F(ExporterFactoryTest, NoSideEffectsFromRepeatedCalls) {
    // Multiple calls should not affect subsequent behavior
    for (int i = 0; i < 100; ++i) {
        auto format = ExporterFactory::detectFormat("test.json");
        auto exporter = ExporterFactory::create(format);
        auto ext = ExporterFactory::getExtension(format);
        auto name = ExporterFactory::getFormatName(format);
        
        // Results should be consistent
        EXPECT_EQ(format, ExportFormat::JSON);
        EXPECT_NE(exporter, nullptr);
        EXPECT_EQ(ext, ".json");
        EXPECT_EQ(name, "JavaScript Object Notation");
    }
}
