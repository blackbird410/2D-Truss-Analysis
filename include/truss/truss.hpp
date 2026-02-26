/**
 * @file truss.hpp
 * @brief Main public API header for 2D Truss Analysis library
 *
 * This is the primary include file for external users of the library.
 * It provides access to all public classes and types.
 *
 * Usage:
 * @code
 * #include <truss/truss.hpp>
 *
 * // Use classes and functions from truss namespace
 * @endcode
 */

#pragma once

// Core domain types
#include "core/analysis_results.hpp"
#include "core/member.hpp"
#include "core/node.hpp"
#include "core/truss.hpp"
#include "core/types.hpp"

// Analysis interface
#include "analysis/analysis_facade.hpp"
#include "analysis/analysis_options.hpp"

// Export interface
#include "export/export_format.hpp"

/**
 * @namespace truss
 * @brief Main namespace for the 2D Truss Analysis library
 *
 * All public API classes and functions are in this namespace.
 */
namespace truss {

// Version information
constexpr const char* VERSION = "3.0.0";
constexpr int VERSION_MAJOR = 3;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;

}  // namespace truss
