/**
 * @file test_fileio.cpp
 * @brief Google Test unit tests for File I/O services
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-13
 */

#include <gtest/gtest.h>
#include "infrastructure/io/fileio_factory.hpp"
#include "infrastructure/io/json_truss_reader.hpp"
#include "infrastructure/io/json_truss_writer.hpp"
#include "infrastructure/io/xml_truss_reader.hpp"
#include "infrastructure/io/xml_truss_writer.hpp"
#include "core/model/Truss.hpp"
#include <filesystem>
#include <fstream>

using namespace truss::infrastructure::io;
using namespace truss::core;

// Test fixture for File I/O tests
class FileIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test directory
        testDir = std::filesystem::temp_directory_path() / "truss_fileio_test";
        std::filesystem::create_directories(testDir);
    }
    
    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }
    
    // Helper: Create a simple test truss
    std::shared_ptr<Truss> createSimpleTruss() {
        auto truss = std::make_shared<Truss>("Simple Test Truss");
        
        // Add nodes
        truss->addNode(0.0, 0.0, SupportType::Pinned);  // Node 1
        truss->addNode(4.0, 0.0, SupportType::RollerY); // Node 2
        truss->addNode(2.0, 3.0, SupportType::Free);    // Node 3
        
        // Add members with correct constructors
        MaterialProperties material{200e9, 7850.0, 250e6, 400e6, "Steel"};
        SectionProperties section{0.01, 1e-8, 0.01, "Default"};
        truss->addMember(1, 3, material, section);
        truss->addMember(2, 3, material, section);
        
        // Add load
        truss->applyForce(3, 0.0, -10000.0);
        
        return truss;
    }
    
    // Helper: Verify two trusses are equivalent
    void verifyTrussEquivalence(const Truss& t1, const Truss& t2) {
        EXPECT_EQ(t1.getName(), t2.getName());
        EXPECT_EQ(t1.getNodeCount(), t2.getNodeCount());
        EXPECT_EQ(t1.getMemberCount(), t2.getMemberCount());
        
        // Verify nodes
        const auto& nodes1 = t1.getNodes();
        const auto& nodes2 = t2.getNodes();
        for (size_t i = 0; i < nodes1.size(); ++i) {
            EXPECT_NEAR(nodes1[i]->getX(), nodes2[i]->getX(), 1e-9);
            EXPECT_NEAR(nodes1[i]->getY(), nodes2[i]->getY(), 1e-9);
            EXPECT_EQ(nodes1[i]->getSupportType(), nodes2[i]->getSupportType());
        }
        
        // Verify members
        const auto& members1 = t1.getMembers();
        const auto& members2 = t2.getMembers();
        for (size_t i = 0; i < members1.size(); ++i) {
            EXPECT_EQ(members1[i]->getStartNode()->getId(), 
                     members2[i]->getStartNode()->getId());
            EXPECT_EQ(members1[i]->getEndNode()->getId(), 
                     members2[i]->getEndNode()->getId());
            EXPECT_NEAR(members1[i]->getMaterial().youngModulus,
                       members2[i]->getMaterial().youngModulus, 1e-6);
            EXPECT_NEAR(members1[i]->getSection().area,
                       members2[i]->getSection().area, 1e-9);
        }
    }
    
    std::filesystem::path testDir;
};

// ============================================================================
// Factory Tests
// ============================================================================

TEST_F(FileIOTest, FactoryCreatesJsonReader) {
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    ASSERT_NE(reader, nullptr);
    EXPECT_TRUE(reader->supportsFormat(FileFormat::JSON));
    EXPECT_EQ(reader->getFormat(), FileFormat::JSON);
}

TEST_F(FileIOTest, FactoryCreatesXmlReader) {
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    ASSERT_NE(reader, nullptr);
    EXPECT_TRUE(reader->supportsFormat(FileFormat::XML));
    EXPECT_EQ(reader->getFormat(), FileFormat::XML);
}

TEST_F(FileIOTest, FactoryCreatesJsonWriter) {
    auto writer = FileIOFactory::createWriter(FileFormat::JSON);
    ASSERT_NE(writer, nullptr);
    EXPECT_TRUE(writer->supportsFormat(FileFormat::JSON));
    EXPECT_EQ(writer->getFormat(), FileFormat::JSON);
}

TEST_F(FileIOTest, FactoryCreatesXmlWriter) {
    auto writer = FileIOFactory::createWriter(FileFormat::XML);
    ASSERT_NE(writer, nullptr);
    EXPECT_TRUE(writer->supportsFormat(FileFormat::XML));
    EXPECT_EQ(writer->getFormat(), FileFormat::XML);
}

TEST_F(FileIOTest, FactoryDetectsFormatFromExtension) {
    EXPECT_EQ(FileIOFactory::detectFormat("model.json"), FileFormat::JSON);
    EXPECT_EQ(FileIOFactory::detectFormat("model.xml"), FileFormat::XML);
    EXPECT_EQ(FileIOFactory::detectFormat("model.txt"), FileFormat::Auto);
}

TEST_F(FileIOTest, FactoryCreatesReaderFromPath) {
    auto jsonReader = FileIOFactory::createReader(testDir / "test.json");
    EXPECT_EQ(jsonReader->getFormat(), FileFormat::JSON);
    
    auto xmlReader = FileIOFactory::createReader(testDir / "test.xml");
    EXPECT_EQ(xmlReader->getFormat(), FileFormat::XML);
}

TEST_F(FileIOTest, FactoryThrowsOnAutoFormat) {
    EXPECT_THROW(FileIOFactory::createReader(FileFormat::Auto), std::invalid_argument);
    EXPECT_THROW(FileIOFactory::createWriter(FileFormat::Auto), std::invalid_argument);
}

TEST_F(FileIOTest, FactoryThrowsOnUnknownExtension) {
    EXPECT_THROW(FileIOFactory::createReader(testDir / "test.txt"), std::invalid_argument);
    EXPECT_THROW(FileIOFactory::createWriter(testDir / "test.txt"), std::invalid_argument);
}

// ============================================================================
// JSON Writer Tests
// ============================================================================

TEST_F(FileIOTest, JsonWriterCreatesValidFile) {
    auto truss = createSimpleTruss();
    auto writer = FileIOFactory::createWriter(FileFormat::JSON);
    
    auto filepath = testDir / "test.json";
    FileIOOptions options;
    options.validateOnWrite = false;
    options.overwriteExisting = true;
    
    EXPECT_TRUE(writer->write(*truss, filepath, options));
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(FileIOTest, JsonWriterThrowsOnExistingFileWithoutOverwrite) {
    auto truss = createSimpleTruss();
    auto writer = FileIOFactory::createWriter(FileFormat::JSON);
    
    auto filepath = testDir / "test.json";
    
    // First write
    FileIOOptions options1;
    options1.overwriteExisting = true;
    options1.validateOnWrite = false;
    writer->write(*truss, filepath, options1);
    
    // Second write without overwrite should throw
    FileIOOptions options2;
    options2.overwriteExisting = false;
    options2.validateOnWrite = false;
    EXPECT_THROW(writer->write(*truss, filepath, options2), FileWriteException);
}

TEST_F(FileIOTest, JsonWriterIncludesMetadata) {
    auto truss = createSimpleTruss();
    auto writer = FileIOFactory::createWriter(FileFormat::JSON);
    
    auto filepath = testDir / "test.json";
    FileIOOptions options;
    options.includeMetadata = true;
    options.overwriteExisting = true;
    options.validateOnWrite = false;
    
    writer->write(*truss, filepath, options);
    
    // Read file and verify metadata exists
    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("metadata") != std::string::npos);
    EXPECT_TRUE(content.find("Simple Test Truss") != std::string::npos);
}

// ============================================================================
// JSON Reader Tests
// ============================================================================

TEST_F(FileIOTest, JsonReaderThrowsOnNonexistentFile) {
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    auto filepath = testDir / "nonexistent.json";
    
    EXPECT_THROW(reader->read(filepath), FileNotFoundException);
}

TEST_F(FileIOTest, JsonReaderThrowsOnInvalidJson) {
    auto filepath = testDir / "invalid.json";
    
    // Create invalid JSON file
    std::ofstream file(filepath);
    file << "{ invalid json content";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    EXPECT_THROW(reader->read(filepath), ParseException);
}

TEST_F(FileIOTest, JsonReaderThrowsOnMissingNodes) {
    auto filepath = testDir / "missing_nodes.json";
    
    // Create JSON without nodes section
    std::ofstream file(filepath);
    file << R"({"metadata": {"name": "Test"}, "members": []})";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    EXPECT_THROW(reader->read(filepath), ParseException);
}

// ============================================================================
// JSON Round-Trip Tests
// ============================================================================

TEST_F(FileIOTest, JsonRoundTripPreservesData) {
    auto originalTruss = createSimpleTruss();
    auto filepath = testDir / "roundtrip.json";
    
    // Write
    auto writer = FileIOFactory::createWriter(FileFormat::JSON);
    FileIOOptions writeOptions;
    writeOptions.validateOnWrite = false;
    writeOptions.overwriteExisting = true;
    writer->write(*originalTruss, filepath, writeOptions);
    
    // Read
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    FileIOOptions readOptions;
    readOptions.validateOnRead = false;
    auto loadedTruss = reader->read(filepath, readOptions);
    
    // Verify equivalence
    verifyTrussEquivalence(*originalTruss, *loadedTruss);
}

TEST_F(FileIOTest, JsonRoundTripWithPrettyPrint) {
    auto truss = createSimpleTruss();
    auto filepath = testDir / "pretty.json";
    
    // Write with pretty print
    auto writer = FileIOFactory::createWriter(FileFormat::JSON);
    FileIOOptions options;
    options.prettyPrint = true;
    options.indentSize = 2;
    options.overwriteExisting = true;
    options.validateOnWrite = false;
    writer->write(*truss, filepath, options);
    
    // Verify file contains newlines (pretty-printed)
    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find('\n') != std::string::npos);
}

// ============================================================================
// XML Writer Tests
// ============================================================================

TEST_F(FileIOTest, XmlWriterCreatesValidFile) {
    auto truss = createSimpleTruss();
    auto writer = FileIOFactory::createWriter(FileFormat::XML);
    
    auto filepath = testDir / "test.xml";
    FileIOOptions options;
    options.validateOnWrite = false;
    options.overwriteExisting = true;
    
    EXPECT_TRUE(writer->write(*truss, filepath, options));
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(FileIOTest, XmlWriterThrowsOnExistingFileWithoutOverwrite) {
    auto truss = createSimpleTruss();
    auto writer = FileIOFactory::createWriter(FileFormat::XML);
    
    auto filepath = testDir / "test.xml";
    
    // First write
    FileIOOptions options1;
    options1.overwriteExisting = true;
    options1.validateOnWrite = false;
    writer->write(*truss, filepath, options1);
    
    // Second write without overwrite should throw
    FileIOOptions options2;
    options2.overwriteExisting = false;
    options2.validateOnWrite = false;
    EXPECT_THROW(writer->write(*truss, filepath, options2), FileWriteException);
}

// ============================================================================
// XML Reader Tests
// ============================================================================

TEST_F(FileIOTest, XmlReaderThrowsOnNonexistentFile) {
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    auto filepath = testDir / "nonexistent.xml";
    
    EXPECT_THROW(reader->read(filepath), FileNotFoundException);
}

TEST_F(FileIOTest, XmlReaderThrowsOnInvalidXml) {
    auto filepath = testDir / "invalid.xml";
    
    // Create invalid XML file
    std::ofstream file(filepath);
    file << "<truss><unclosed>";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    EXPECT_THROW(reader->read(filepath), ParseException);
}

TEST_F(FileIOTest, XmlReaderThrowsOnMissingNodes) {
    auto filepath = testDir / "missing_nodes.xml";
    
    // Create XML without nodes section
    std::ofstream file(filepath);
    file << R"(<?xml version="1.0"?><truss><metadata name="Test"/></truss>)";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    EXPECT_THROW(reader->read(filepath), ParseException);
}

// ============================================================================
// XML Round-Trip Tests
// ============================================================================

TEST_F(FileIOTest, XmlRoundTripPreservesData) {
    auto originalTruss = createSimpleTruss();
    auto filepath = testDir / "roundtrip.xml";
    
    // Write
    auto writer = FileIOFactory::createWriter(FileFormat::XML);
    FileIOOptions writeOptions;
    writeOptions.validateOnWrite = false;
    writeOptions.overwriteExisting = true;
    writer->write(*originalTruss, filepath, writeOptions);
    
    // Read
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    FileIOOptions readOptions;
    readOptions.validateOnRead = false;
    auto loadedTruss = reader->read(filepath, readOptions);
    
    // Verify equivalence
    verifyTrussEquivalence(*originalTruss, *loadedTruss);
}

// ============================================================================
// Validation Tests
// ============================================================================

TEST_F(FileIOTest, ReaderValidationDetectsInvalidTruss) {
    auto filepath = testDir / "invalid_truss.json";
    
    // Create JSON with invalid truss (member with invalid node IDs)
    std::ofstream file(filepath);
    file << R"({
        "nodes": [{"id": 1, "x": 0.0, "y": 0.0, "support": "free"}],
        "members": [{"startNode": 1, "endNode": 999}]
    })";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnRead = true;
    
    // Should throw validation exception (member references non-existent node)
    EXPECT_THROW(reader->read(filepath, options), ValidationException);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(FileIOTest, HandlesEmptyTruss) {
    auto truss = std::make_shared<Truss>("Empty Truss");
    truss->addNode(0.0, 0.0, SupportType::Pinned);
    
    auto filepath = testDir / "empty.json";
    
    auto writer = FileIOFactory::createWriter(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnWrite = false;
    options.overwriteExisting = true;
    
    EXPECT_NO_THROW(writer->write(*truss, filepath, options));
}

TEST_F(FileIOTest, HandlesLargeTruss) {
    auto truss = std::make_shared<Truss>("Large Truss");
    
    // Create 100 nodes
    for (int i = 1; i <= 100; ++i) {
        truss->addNode(static_cast<Real>(i), 0.0, SupportType::Free);
    }
    
    auto filepath = testDir / "large.json";
    
    auto writer = FileIOFactory::createWriter(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnWrite = false;
    options.overwriteExisting = true;
    
    EXPECT_NO_THROW(writer->write(*truss, filepath, options));
    
    // Verify file size is reasonable
    auto fileSize = std::filesystem::file_size(filepath);
    EXPECT_GT(fileSize, 1000);  // Should be more than 1KB for 100 nodes
}

