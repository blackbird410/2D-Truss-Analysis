/**
 * @file test_fileio_advanced.cpp
 * @brief Advanced tests for file I/O functionality.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "core/assembly/truss_assembler.hpp"
#include "core/model/truss.hpp"
#include "infrastructure/io/fileio_factory.hpp"
#include "infrastructure/io/json_truss_reader.hpp"
#include "infrastructure/io/json_truss_writer.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace truss::infrastructure::io;
using namespace truss::core;

namespace fs = std::filesystem;

class FileIOAdvancedTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "truss_fileio_advanced_test";
        fs::create_directories(testDir);
    }

    void TearDown() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    std::shared_ptr<Truss> createSimpleTruss() {
        auto truss = std::make_shared<Truss>("Simple");
        truss->addNode(0.0, 0.0, SupportType::Pinned);
        truss->addNode(4.0, 0.0, SupportType::Free);

        MaterialProperties mat{200e9, 7850.0, 250e6, 400e6, "Steel"};
        SectionProperties sec{0.01, 1e-8, 0.01, "Default"};
        truss->addMember(1, 2, mat, sec);
        truss->applyForce(2, 0.0, -5000.0);

        return truss;
    }

    void writeJSONFile(const fs::path& path, const std::string& content) {
        std::ofstream file(path);
        file << content;
    }

    fs::path testDir;
};

TEST_F(FileIOAdvancedTest, ComplexRoundTrip_JSON) {
    auto originalTruss = createSimpleTruss();

    fs::path filepath = testDir / "complex.json";
    auto writer = FileIOFactory::createWriter(filepath);

    FileIOOptions opts;
    opts.prettyPrint = true;
    opts.overwriteExisting = true;

    auto dto = assembly::TrussAssembler::createDTO(*originalTruss);
    ASSERT_NO_THROW(writer->write(dto, filepath, opts));
    ASSERT_TRUE(fs::exists(filepath));

    auto reader = FileIOFactory::createReader(filepath);
    auto loadedDTO = reader->read(filepath, opts);
    auto loadedTruss = assembly::TrussAssembler::assembleTruss(loadedDTO);

    ASSERT_NE(loadedTruss, nullptr);
    EXPECT_EQ(loadedTruss->getName(), "Simple");
    EXPECT_EQ(loadedTruss->getNodeCount(), 2);
    EXPECT_EQ(loadedTruss->getMemberCount(), 1);
}

TEST_F(FileIOAdvancedTest, MalformedJSON_MissingNodes) {
    fs::path filepath = testDir / "malformed.json";

    writeJSONFile(filepath, R"({
        "metadata": {"name": "Invalid"},
        "members": []
    })");

    auto reader = FileIOFactory::createReader(filepath);
    FileIOOptions opts;

    EXPECT_THROW({ auto dto = reader->read(filepath, opts); }, ParseException);
}

TEST_F(FileIOAdvancedTest, MalformedJSON_DuplicateNodeIDs) {
    fs::path filepath = testDir / "duplicate.json";

    writeJSONFile(filepath, R"({
        "metadata": {"name": "Duplicate"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 1, "x": 1.0, "y": 0.0}
        ]
    })");

    auto reader = FileIOFactory::createReader(filepath);
    FileIOOptions opts;

    EXPECT_THROW({ auto dto = reader->read(filepath, opts); }, ParseException);
}

TEST_F(FileIOAdvancedTest, MalformedJSON_UnknownNodeReference) {
    fs::path filepath = testDir / "unknown_node.json";

    writeJSONFile(filepath, R"({
        "metadata": {"name": "Unknown"},
        "nodes": [
            {"id": 1, "x": 0.0, "y": 0.0},
            {"id": 2, "x": 1.0, "y": 0.0}
        ],
        "members": [
            {"startNode": 1, "endNode": 999}
        ]
    })");

    auto reader = FileIOFactory::createReader(filepath);
    FileIOOptions opts;

    EXPECT_THROW({ auto dto = reader->read(filepath, opts); }, ParseException);
}

TEST_F(FileIOAdvancedTest, OverwriteProtection_RefusesWithoutFlag) {
    auto truss = createSimpleTruss();
    fs::path filepath = testDir / "protected.json";

    auto writer = FileIOFactory::createWriter(filepath);
    FileIOOptions opts1;
    opts1.overwriteExisting = true;

    auto dto = assembly::TrussAssembler::createDTO(*truss);
    ASSERT_NO_THROW(writer->write(dto, filepath, opts1));

    FileIOOptions opts2;
    opts2.overwriteExisting = false;

    EXPECT_THROW({ writer->write(dto, filepath, opts2); }, FileWriteException);
}

TEST_F(FileIOAdvancedTest, EmptyFileHandling) {
    fs::path filepath = testDir / "empty.json";
    std::ofstream(filepath).close();

    auto reader = FileIOFactory::createReader(filepath);
    FileIOOptions opts;

    EXPECT_THROW({ auto dto = reader->read(filepath, opts); }, ParseException);
}

TEST_F(FileIOAdvancedTest, FormatDetection) {
    std::vector<std::pair<std::string, FileFormat>> testCases = {{"test.json", FileFormat::JSON},
                                                                 {"test.xml", FileFormat::XML}};

    for (const auto& [filename, expected] : testCases) {
        fs::path filepath = testDir / filename;
        auto reader = FileIOFactory::createReader(filepath);
        ASSERT_NE(reader, nullptr);
    }
}

#include "infrastructure/io/xml_truss_writer.hpp"

// ============================================================================
// Branch coverage tests for JSON/XML writer options paths
// ============================================================================

/**
 * @test JSON writer with prettyPrint=false produces compact (non-indented) output
 */
TEST_F(FileIOAdvancedTest, JsonWriter_CompactOutput_NoPrettyPrint) {
    auto truss = createSimpleTruss();
    auto filepath = testDir / "compact.json";

    auto writer = std::make_unique<JsonTrussWriter>();
    FileIOOptions opts;
    opts.prettyPrint = false;
    opts.overwriteExisting = true;
    opts.validateOnWrite = false;

    auto dto = truss::core::assembly::TrussAssembler::createDTO(*truss);
    EXPECT_TRUE(writer->write(dto, filepath, opts));

    // Compact output should NOT contain newlines
    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find('\n') == std::string::npos);
}

/**
 * @test JSON writer with includeMetadata=false produces no metadata section
 */
TEST_F(FileIOAdvancedTest, JsonWriter_NoMetadata_SkipsMetadataSection) {
    auto truss = createSimpleTruss();
    auto filepath = testDir / "no_meta.json";

    auto writer = std::make_unique<JsonTrussWriter>();
    FileIOOptions opts;
    opts.includeMetadata = false;
    opts.overwriteExisting = true;
    opts.validateOnWrite = false;

    auto dto = truss::core::assembly::TrussAssembler::createDTO(*truss);
    EXPECT_TRUE(writer->write(dto, filepath, opts));

    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    // Without metadata flag, "metadata" key should not appear
    EXPECT_TRUE(content.find("\"metadata\"") == std::string::npos);
}

/**
 * @test XML writer with includeMetadata=false produces no metadata element
 */
TEST_F(FileIOAdvancedTest, XmlWriter_NoMetadata_SkipsMetadataElement) {
    auto truss = createSimpleTruss();
    auto filepath = testDir / "no_meta.xml";

    auto writer = std::make_unique<XmlTrussWriter>();
    FileIOOptions opts;
    opts.includeMetadata = false;
    opts.overwriteExisting = true;
    opts.validateOnWrite = false;

    auto dto = truss::core::assembly::TrussAssembler::createDTO(*truss);
    EXPECT_TRUE(writer->write(dto, filepath, opts));

    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    // Without metadata flag, <metadata ...> should not appear
    EXPECT_TRUE(content.find("<metadata") == std::string::npos);
}

/**
 * @test XML writer throws when file exists and overwrite is disallowed
 */
TEST_F(FileIOAdvancedTest, XmlWriter_ExistingFile_ThrowsWithoutOverwrite) {
    auto truss = createSimpleTruss();
    auto filepath = testDir / "exists.xml";

    // Create file first
    auto writer = std::make_unique<XmlTrussWriter>();
    FileIOOptions opts;
    opts.overwriteExisting = true;
    opts.validateOnWrite = false;
    auto dto = truss::core::assembly::TrussAssembler::createDTO(*truss);
    writer->write(dto, filepath, opts);

    // Second write without overwrite should throw
    FileIOOptions opts2;
    opts2.overwriteExisting = false;
    EXPECT_THROW(writer->write(dto, filepath, opts2), FileWriteException);
}
/**
 * @test JSON writer throws FileWriteException when target directory does not exist.
 * Covers the `if (!file.is_open())` branch in json_truss_writer.cpp.
 */
TEST_F(FileIOAdvancedTest, JsonWriter_BadDirectory_ThrowsFileWriteException) {
    auto truss = createSimpleTruss();
    auto writer = std::make_unique<JsonTrussWriter>();
    fs::path badPath = "/nonexistent_dir_xyz_never_created_rnd/output.json";
    FileIOOptions opts;
    opts.validateOnWrite = false;
    auto dto = truss::core::assembly::TrussAssembler::createDTO(*truss);
    EXPECT_THROW(writer->write(dto, badPath, opts), FileWriteException);
}

/**
 * @test XML writer throws FileWriteException when target directory does not exist.
 * Covers the `if (result != XML_SUCCESS)` branch in xml_truss_writer.cpp.
 */
TEST_F(FileIOAdvancedTest, XmlWriter_BadDirectory_ThrowsFileWriteException) {
    auto truss = createSimpleTruss();
    auto writer = std::make_unique<XmlTrussWriter>();
    fs::path badPath = "/nonexistent_dir_xyz_never_created_rnd/output.xml";
    FileIOOptions opts;
    opts.validateOnWrite = false;
    auto dto = truss::core::assembly::TrussAssembler::createDTO(*truss);
    EXPECT_THROW(writer->write(dto, badPath, opts), FileWriteException);
}

// ============================================================================
// Tests for io_types.hpp inline utility functions
// ============================================================================

TEST(IoTypesTest, GetFileExtension_Auto_ReturnsEmpty) {
    // FileFormat::Auto has no explicit case → default branch returns ""
    EXPECT_EQ(getFileExtension(FileFormat::Auto), "");
}

TEST(IoTypesTest, DetectFileFormat_UnknownExtension_ReturnsAuto) {
    // Neither .json nor .xml → returns Auto
    EXPECT_EQ(detectFileFormat("file.csv"), FileFormat::Auto);
    EXPECT_EQ(detectFileFormat("file.txt"), FileFormat::Auto);
    EXPECT_EQ(detectFileFormat("noext"), FileFormat::Auto);
}

TEST(IoTypesTest, GetFormatName_Auto_ReturnsAuto) {
    EXPECT_EQ(getFormatName(FileFormat::Auto), "Auto");
}

TEST(IoTypesTest, GetFormatName_InvalidValue_ReturnsUnknown) {
    auto invalid = static_cast<FileFormat>(99);
    EXPECT_EQ(getFormatName(invalid), "Unknown");
}

// ---------------------------------------------------------------------------
// FileIOFactory utility method branch coverage
// ---------------------------------------------------------------------------

TEST(FileIOFactoryUtilTest, IsFormatSupported_JSONAndXML_ReturnsTrue) {
    EXPECT_TRUE(FileIOFactory::isFormatSupported(FileFormat::JSON));
    EXPECT_TRUE(FileIOFactory::isFormatSupported(FileFormat::XML));
}

TEST(FileIOFactoryUtilTest, IsFormatSupported_Auto_ReturnsFalse) {
    EXPECT_FALSE(FileIOFactory::isFormatSupported(FileFormat::Auto));
}

TEST(FileIOFactoryUtilTest, GetExtension_JSONandXML) {
    EXPECT_EQ(FileIOFactory::getExtension(FileFormat::JSON), ".json");
    EXPECT_EQ(FileIOFactory::getExtension(FileFormat::XML), ".xml");
}

TEST(FileIOFactoryUtilTest, GetFormatName_JSONandXML) {
    EXPECT_EQ(FileIOFactory::getFormatName(FileFormat::JSON), "JSON");
    EXPECT_EQ(FileIOFactory::getFormatName(FileFormat::XML), "XML");
}

TEST(FileIOFactoryUtilTest, CreateWriter_UnknownFormat_Throws) {
    auto unknown = static_cast<FileFormat>(99);
    EXPECT_THROW(FileIOFactory::createWriter(unknown), std::invalid_argument);
}

TEST(FileIOFactoryUtilTest, CreateReader_UnknownFormat_Throws) {
    auto unknown = static_cast<FileFormat>(99);
    EXPECT_THROW(FileIOFactory::createReader(unknown), std::invalid_argument);
}