/**
 * @file argument_parser.cpp
 * @brief Implements the argument parser.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "argument_parser.hpp"

#include <algorithm>

namespace truss::cli {

ParsedArgs ArgumentParser::parse(int argc, const char* const argv[]) {
    ParsedArgs result;

    // Skip program name (argv[0])
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Check for help flag
        if (arg == "-h" || arg == "--help") {
            result.showHelp = true;
            result.commandName = "help";
            continue;
        }

        // Check for verbose flag
        if (arg == "-v" || arg == "--verbose") {
            result.verbose = true;
            result.options["verbose"] = "true";
            continue;
        }

        // Check for other options
        if (isOption(arg)) {
            std::string optionName = extractOptionName(arg);
            std::string optionValue = extractOptionValue(arg);

            // If value not in same argument, check next argument
            if (optionValue.empty() && i + 1 < argc && !isOption(argv[i + 1])) {
                optionValue = argv[++i];
            }

            result.options[optionName] = optionValue.empty() ? "true" : optionValue;
        } else {
            // Positional argument - first one is command name
            if (result.commandName.empty()) {
                result.commandName = arg;
            } else {
                result.positionalArgs.push_back(arg);
            }
        }
    }

    // Default command if none specified
    if (result.commandName.empty() && !result.showHelp) {
        result.commandName = "help";  // Show help by default
    }

    return result;
}

bool ArgumentParser::isOption(const std::string& arg) {
    return isShortOption(arg) || isLongOption(arg);
}

bool ArgumentParser::isShortOption(const std::string& arg) {
    return arg.size() >= 2 && arg[0] == '-' && arg[1] != '-';
}

bool ArgumentParser::isLongOption(const std::string& arg) {
    return arg.size() >= 3 && arg[0] == '-' && arg[1] == '-';
}

std::string ArgumentParser::extractOptionName(const std::string& arg) {
    if (isLongOption(arg)) {
        // Extract from --name or --name=value
        size_t start = 2;
        size_t end = arg.find('=');
        return arg.substr(start, end == std::string::npos ? end : end - start);
    } else if (isShortOption(arg)) {
        // Extract from -n or -n=value
        size_t start = 1;
        size_t end = arg.find('=');
        return arg.substr(start, end == std::string::npos ? end : end - start);
    }
    return "";
}

std::string ArgumentParser::extractOptionValue(const std::string& arg) {
    size_t pos = arg.find('=');
    if (pos != std::string::npos && pos + 1 < arg.size()) {
        return arg.substr(pos + 1);
    }
    return "";
}

std::optional<std::string> ArgumentParser::getOption(const ParsedArgs& args,
                                                     const std::string& longForm,
                                                     const std::string& shortForm) {
    // Try long form first
    auto it = args.options.find(longForm);
    if (it != args.options.end()) {
        return it->second;
    }

    // Try short form if long form not found
    it = args.options.find(shortForm);
    if (it != args.options.end()) {
        return it->second;
    }

    // Not found
    return std::nullopt;
}

}  // namespace truss::cli
