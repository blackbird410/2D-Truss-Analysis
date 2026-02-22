/**
 * @file ArgumentParser.hpp
 * @brief CLI argument parser
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Responsibility: Parse command-line arguments into structured format.
 * Does not execute commands or perform validation logic.
 */

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace truss::cli {

/**
 * @brief Parsed command line arguments
 */
struct ParsedArgs {
    std::string commandName;                     // e.g., "example", "help"
    std::map<std::string, std::string> options;  // e.g., {"verbose": "true", "file": "truss.json"}
    std::vector<std::string> positionalArgs;     // e.g., ["truss.json"]
    bool verbose = false;
    bool showHelp = false;
};

/**
 * @brief Argument parser for CLI
 *
 * Supported formats:
 * - Long options: --file=truss.json, --file truss.json
 * - Short options: -f truss.json, -v
 * - Boolean flags: --verbose, -v
 * - Positional: TrussAnalysisCLI example
 */
class ArgumentParser {
public:
    ArgumentParser() = default;
    ~ArgumentParser() = default;

    /**
     * @brief Parse command line arguments
     * @param argc Argument count
     * @param argv Argument vector
     * @return Parsed arguments
     */
    static ParsedArgs parse(int argc, const char* const argv[]);

    /**
     * @brief Get option value with fallback to short form
     * @param args Parsed arguments
     * @param longForm Long option name (e.g., "file")
     * @param shortForm Short option name (e.g., "f")
     * @return Optional value if found
     */
    static std::optional<std::string>
    getOption(const ParsedArgs& args, const std::string& longForm, const std::string& shortForm);

private:
    static bool isOption(const std::string& arg);
    static bool isShortOption(const std::string& arg);
    static bool isLongOption(const std::string& arg);
    static std::string extractOptionName(const std::string& arg);
    static std::string extractOptionValue(const std::string& arg);
};

}  // namespace truss::cli
