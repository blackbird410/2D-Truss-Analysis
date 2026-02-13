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
#include <chrono>
#include <sstream>
#include <thread>  // for std::this_thread::get_id()

using namespace truss::infrastructure::io;
using namespace truss::core;

// Test fixture for File I/O tests
class FileIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create unique temporary test directory per test instance
        // Use thread ID and timestamp to avoid collisions in concurrent test runs
        std::ostringstream dirName;
        dirName << "truss_fileio_test_" 
                << std::chrono::system_clock::now().time_since_epoch().count()
                << "_" << std::this_thread::get_id();
        testDir = std::filesystem::temp_directory_path() / dirName.str();
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
    
    // Should throw parse exception (member references non-existent node)
    EXPECT_THROW(reader->read(filepath, options), ParseException);
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

// ============================================================================
// Referential Integrity Tests - CRITICAL FOR STRUCTURAL ANALYSIS
// ============================================================================

TEST_F(FileIOTest, JsonReaderDetectsDuplicateNodeIDs) {
    auto filepath = testDir / "duplicate_nodes.json";
    
    // Create JSON with duplicate node IDs
    std::ofstream file(filepath);
    file << R"({
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 4.0, "y": 0.0, "support": "free"},
            {"id": 1, "x": 2.0, "y": 3.0, "support": "free"}
        ]
    })";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnRead = false;  // Disable domain validation
    
    // Should throw ParseException with clear error message
    try {
        reader->read(filepath, options);
        FAIL() << "Expected ParseException for duplicate node ID";
    } catch (const ParseException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("Duplicate node ID") != std::string::npos);
        EXPECT_TRUE(errorMsg.find("1") != std::string::npos);
    }
}

TEST_F(FileIOTest, XmlReaderDetectsDuplicateNodeIDs) {
    auto filepath = testDir / "duplicate_nodes.xml";
    
    // Create XML with duplicate node IDs
    std::ofstream file(filepath);
    file << R"(<?xml version="1.0"?>
<truss>
    <nodes>
        <node id="1" x="0.0" y="0.0" support="pinned"/>
        <node id="2" x="4.0" y="0.0" support="free"/>
        <node id="1" x="2.0" y="3.0" support="free"/>
    </nodes>
</truss>
)";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should throw ParseException
    try {
        reader->read(filepath, options);
        FAIL() << "Expected ParseException for duplicate node ID";
    } catch (const ParseException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("Duplicate node ID") != std::string::npos);
        EXPECT_TRUE(errorMsg.find("1") != std::string::npos);
    }
}

TEST_F(FileIOTest, JsonReaderDetectsUnknownStartNodeInMember) {
    auto filepath = testDir / "unknown_start_node.json";
    
    // Create JSON where member references non-existent start node
    std::ofstream file(filepath);
    file << R"({
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 4.0, "y": 0.0, "support": "free"}
        ],
        "members": [
            {"startNode": 999, "endNode": 2}
        ]
    })";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should throw ParseException
    try {
        reader->read(filepath, options);
        FAIL() << "Expected ParseException for unknown start node ID";
    } catch (const ParseException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("unknown start node ID") != std::string::npos);
        EXPECT_TRUE(errorMsg.find("999") != std::string::npos);
    }
}

TEST_F(FileIOTest, JsonReaderDetectsUnknownEndNodeInMember) {
    auto filepath = testDir / "unknown_end_node.json";
    
    // Create JSON where member references non-existent end node
    std::ofstream file(filepath);
    file << R"({
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 4.0, "y": 0.0, "support": "free"}
        ],
        "members": [
            {"startNode": 1, "endNode": 888}
        ]
    })";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should throw ParseException
    try {
        reader->read(filepath, options);
        FAIL() << "Expected ParseException for unknown end node ID";
    } catch (const ParseException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("unknown end node ID") != std::string::npos);
        EXPECT_TRUE(errorMsg.find("888") != std::string::npos);
    }
}

TEST_F(FileIOTest, XmlReaderDetectsUnknownNodeInMember) {
    auto filepath = testDir / "unknown_node.xml";
    
    // Create XML where member references non-existent nodes
    std::ofstream file(filepath);
    file << R"(<?xml version="1.0"?>
<truss>
    <nodes>
        <node id="1" x="0.0" y="0.0" support="pinned"/>
        <node id="2" x="4.0" y="0.0" support="free"/>
    </nodes>
    <members>
        <member startNode="1" endNode="777"/>
    </members>
</truss>
)";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should throw ParseException
    try {
        reader->read(filepath, options);
        FAIL() << "Expected ParseException for unknown node ID in member";
    } catch (const ParseException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("unknown") != std::string::npos);
        EXPECT_TRUE(errorMsg.find("777") != std::string::npos);
    }
}

TEST_F(FileIOTest, JsonReaderDetectsUnknownNodeInLoad) {
    auto filepath = testDir / "unknown_load_node.json";
    
    // Create JSON where load references non-existent node
    std::ofstream file(filepath);
    file << R"({
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 2, "x": 4.0, "y": 0.0, "support": "free"}
        ],
        "loads": [
            {"nodeId": 555, "fx": 0.0, "fy": -1000.0}
        ]
    })";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should throw ParseException
    try {
        reader->read(filepath, options);
        FAIL() << "Expected ParseException for unknown node ID in load";
    } catch (const ParseException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("unknown node ID") != std::string::npos);
        EXPECT_TRUE(errorMsg.find("555") != std::string::npos);
    }
}

TEST_F(FileIOTest, XmlReaderDetectsUnknownNodeInLoad) {
    auto filepath = testDir / "unknown_load_node.xml";
    
    // Create XML where load references non-existent node
    std::ofstream file(filepath);
    file << R"(<?xml version="1.0"?>
<truss>
    <nodes>
        <node id="1" x="0.0" y="0.0" support="pinned"/>
        <node id="2" x="4.0" y="0.0" support="free"/>
    </nodes>
    <loads>
        <load nodeId="666" fx="0.0" fy="-1000.0"/>
    </loads>
</truss>
)";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should throw ParseException
    try {
        reader->read(filepath, options);
        FAIL() << "Expected ParseException for unknown node ID in load";
    } catch (const ParseException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("unknown node ID") != std::string::npos);
        EXPECT_TRUE(errorMsg.find("666") != std::string::npos);
    }
}

TEST_F(FileIOTest, JsonReaderRequiresExplicitNodeIDs) {
    auto filepath = testDir / "missing_node_id.json";
    
    // Create JSON with node missing ID field
    std::ofstream file(filepath);
    file << R"({
        "nodes": [
            {"x": 0.0, "y": 0.0, "support": "pinned"}
        ]
    })";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should throw ParseException - node ID is mandatory
    EXPECT_THROW(reader->read(filepath, options), ParseException);
}

TEST_F(FileIOTest, XmlReaderRequiresExplicitNodeIDs) {
    auto filepath = testDir / "missing_node_id.xml";
    
    // Create XML with node missing ID attribute
    std::ofstream file(filepath);
    file << R"(<?xml version="1.0"?>
<truss>
    <nodes>
        <node x="0.0" y="0.0" support="pinned"/>
    </nodes>
</truss>
)";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::XML);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should throw ParseException - ID attribute is mandatory
    EXPECT_THROW(reader->read(filepath, options), ParseException);
}

TEST_F(FileIOTest, JsonValidFileWithNonSequentialIDsSucceeds) {
    auto filepath = testDir / "non_sequential.json";
    
    // Create JSON with non-sequential node IDs (100, 200, 300)
    std::ofstream file(filepath);
    file << R"({
        "nodes": [
            {"id": 100, "x": 0.0, "y": 0.0, "support": "pinned"},
            {"id": 200, "x": 4.0, "y": 0.0, "support": "free"},
            {"id": 300, "x": 2.0, "y": 3.0, "support": "free"}
        ],
        "members": [
            {"startNode": 100, "endNode": 300},
            {"startNode": 200, "endNode": 300}
        ],
        "loads": [
            {"nodeId": 300, "fx": 0.0, "fy": -10000.0}
        ]
    })";
    file.close();
    
    auto reader = FileIOFactory::createReader(FileFormat::JSON);
    FileIOOptions options;
    options.validateOnRead = false;
    
    // Should succeed - non-sequential IDs are valid
    auto truss = reader->read(filepath, options);
    ASSERT_NE(truss, nullptr);
    EXPECT_EQ(truss->getNodeCount(), 3);
    EXPECT_EQ(truss->getMemberCount(), 2);
}

