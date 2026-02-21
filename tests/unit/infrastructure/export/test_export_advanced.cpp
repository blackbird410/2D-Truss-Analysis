/**
 * @file test_export_advanced.cpp
 * @brief Advanced tests for export functionality
 * @version 3.0.0
 * @date 2026-02-21
 */

#include <gtest/gtest.h>
#include "infrastructure/export/csv_exporter.hpp"
#include "infrastructure/export/json_exporter.hpp"
#include "infrastructure/export/xml_exporter.hpp"
#include "infrastructure/export/exporter_factory.hpp"
#include "core/model/Truss.hpp"
#include "core/analysis/AnalysisOrchestrator.hpp"
#include "core/analysis/SolverFactory.hpp"
#include <fstream>
#include <filesystem>

using namespace truss::infrastructure::export_;
using namespace truss::core;
using namespace truss::core::analysis;

namespace fs = std::filesystem;

class ExportAdvancedTest : public ::testing::Test {
protected:
    void SetUp() override {
        testOutputDir = "test_output_export_advanced";
        fs::create_directories(testOutputDir);
    }
    
    void TearDown() override {
        if (fs::exists(testOutputDir)) {
            fs::remove_all(testOutputDir);
        }
    }
    
    std::unique_ptr<Truss> createSimpleTruss() {
        auto truss = std::make_unique<Truss>("Simple");
        auto n1 = truss->addNode(0.0, 0.0, SupportType::Pinned);
        auto n2 = truss->addNode(4.0, 0.0, SupportType::RollerX);
        auto n3 = truss->addNode(2.0, 3.0, SupportType::Free);
        truss->addMember(n1, n2);
        truss->addMember(n1, n3);
        truss->addMember(n2, n3);
        n3->setAppliedForce(0.0, -15000.0);
        return truss;
    }
    
    AnalysisResults analyzeAndGetResults(Truss& truss) {
        auto solver = SolverFactory::createDirectSolver();
        AnalysisOrchestrator orchestrator(std::move(solver), std::make_unique<validation::TrussValidator>());
        return orchestrator.analyze(truss);
    }
    
    std::string readFile(const fs::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    std::string testOutputDir;
};

TEST_F(ExportAdvancedTest, SelectiveSectionExport_DisableSomeSections) {
    auto truss = createSimpleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    CSVExporter exporter;
    fs::path filepath = fs::path(testOutputDir) / "selective.csv";
    
    ExportOptions opts;
    opts.includeGeometry = true;
    opts.includeProperties = false;
    opts.includeLoads = false;
    opts.includeDisplacements = true;
    opts.includeMemberForces = false;
    
    bool success = exporter.exportResults(*truss, results, filepath, opts);
    
    ASSERT_TRUE(success);
    ASSERT_TRUE(fs::exists(filepath));
    
    std::string content = readFile(filepath);
    EXPECT_NE(content.find("GEOMETRY"), std::string::npos);
    EXPECT_NE(content.find("NODAL DISPLACEMENTS"), std::string::npos);
    EXPECT_EQ(content.find("MATERIAL AND SECTION PROPERTIES"), std::string::npos);
}

TEST_F(ExportAdvancedTest, TSVDelimiterExport) {
    auto truss = createSimpleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    CSVExporter exporter;
    fs::path filepath = fs::path(testOutputDir) / "data.tsv";
    
    ExportOptions opts;
    opts.delimiter = "\t";
    opts.includeGeometry = true;
    
    bool success = exporter.exportResults(*truss, results, filepath, opts);
    
    ASSERT_TRUE(success);
    std::string content = readFile(filepath);
    EXPECT_NE(content.find("\t"), std::string::npos);
}

TEST_F(ExportAdvancedTest, ScientificNotationExport) {
    auto truss = createSimpleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    CSVExporter exporter;
    fs::path filepath = fs::path(testOutputDir) / "scientific.csv";
    
    ExportOptions opts;
    opts.useScientificNotation = true;
    opts.precision = 4;
    opts.includeProperties = true;
    
    bool success = exporter.exportResults(*truss, results, filepath, opts);
    
    ASSERT_TRUE(success);
    std::string content = readFile(filepath);
    EXPECT_TRUE(content.find("e+") != std::string::npos || 
                content.find("E+") != std::string::npos ||
                content.find("e-") != std::string::npos);
}

TEST_F(ExportAdvancedTest, MultiFormatConsistency) {
    auto truss = createSimpleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    CSVExporter csvExporter;
    JSONExporter jsonExporter;
    XMLExporter xmlExporter;
    
    fs::path csvPath = fs::path(testOutputDir) / "consistency.csv";
    fs::path jsonPath = fs::path(testOutputDir) / "consistency.json";
    fs::path xmlPath = fs::path(testOutputDir) / "consistency.xml";
    
    ExportOptions opts;
    opts.includeGeometry = true;
    opts.includeDisplacements = true;
    opts.precision = 6;
    
    ASSERT_TRUE(csvExporter.exportResults(*truss, results, csvPath, opts));
    ASSERT_TRUE(jsonExporter.exportResults(*truss, results, jsonPath, opts));
    ASSERT_TRUE(xmlExporter.exportResults(*truss, results, xmlPath, opts));
    
    EXPECT_TRUE(fs::exists(csvPath));
    EXPECT_TRUE(fs::exists(jsonPath));
    EXPECT_TRUE(fs::exists(xmlPath));
    
    EXPECT_GT(fs::file_size(csvPath), 0);
    EXPECT_GT(fs::file_size(jsonPath), 0);
    EXPECT_GT(fs::file_size(xmlPath), 0);
}

TEST_F(ExportAdvancedTest, InvalidPathHandling) {
    auto truss = createSimpleTruss();
    auto results = analyzeAndGetResults(*truss);
    
    CSVExporter exporter;
    fs::path invalidPath = "/nonexistent_directory_12345/output.csv";
    
    ExportOptions opts;
    bool success = exporter.exportResults(*truss, results, invalidPath, opts);
    
    EXPECT_FALSE(success);
    EXPECT_FALSE(exporter.getLastError().empty());
}

TEST_F(ExportAdvancedTest, FactoryCreatesAllFormats) {
    std::vector<ExportFormat> formats = {
        ExportFormat::CSV,
        ExportFormat::JSON,
        ExportFormat::XML,
        ExportFormat::HTML,
        ExportFormat::LaTeX,
        ExportFormat::TXT
    };
    
    for (const auto& format : formats) {
        auto exporter = ExporterFactory::create(format);
        ASSERT_NE(exporter, nullptr);
    }
}

TEST_F(ExportAdvancedTest, FormatDetectionFromExtension) {
    EXPECT_EQ(ExporterFactory::detectFormat("test.csv"), ExportFormat::CSV);
    EXPECT_EQ(ExporterFactory::detectFormat("test.json"), ExportFormat::JSON);
    EXPECT_EQ(ExporterFactory::detectFormat("test.xml"), ExportFormat::XML);
    EXPECT_EQ(ExporterFactory::detectFormat("test.html"), ExportFormat::HTML);
}
