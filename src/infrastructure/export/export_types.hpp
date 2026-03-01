/**
 * @file export_types.hpp
 * @brief Export format types and options for results exporters.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include <string>
#include "truss/export/export_format.hpp"  // MUST be outside namespace

namespace truss::infrastructure::export_ {

/**
 * @brief Export section options
 *
 * Controls which sections of analysis results are included in the export.
 * All options default to true (include all data) except useScientificNotation.
 */
struct ExportOptions {
    bool includeGeometry{true};         ///< Include node coordinates and member connectivity
    bool includeProperties{true};       ///< Include material and section properties
    bool includeLoads{true};            ///< Include applied loads
    bool includeDisplacements{true};    ///< Include nodal displacements
    bool includeMemberForces{true};     ///< Include member forces
    bool includeReactions{true};        ///< Include support reactions
    bool includeStresses{true};         ///< Include member stresses
    bool includeUtilization{true};      ///< Include utilization ratios
    bool includeMetadata{true};         ///< Include analysis metadata (timestamp, version, etc.)
    bool useScientificNotation{false};  ///< Use scientific notation for numbers
    int precision{6};                   ///< Number of decimal places for floating-point values
    std::string delimiter{","};         ///< Delimiter for CSV/TSV formats (default: comma)
};

}  // namespace truss::infrastructure::export_
