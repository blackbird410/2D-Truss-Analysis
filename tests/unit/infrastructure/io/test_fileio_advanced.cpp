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
