/**
 * @file AnalysisResultsPresenter.hpp
 * @brief Presenter for formatting structural analysis results for GUI display
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * This presenter formats raw analysis results into user-friendly display data:
 * - Unit conversions (Pa → MPa, m → mm)
 * - Precision formatting for displacements, stresses, forces
 * - Summary message generation
 * - Result categorization (max values, critical elements)
 * 
 * Architecture: GUI Presentation Layer (MVP Pattern)
 * Dependencies: Domain interfaces (IAnalysisResultsView, ITrussView)
 */

#pragma once

#include <QString>
#include <vector>
#include "core/interfaces/IAnalysisResultsView.hpp"
#include "core/interfaces/ITrussView.hpp"

namespace truss_presenters {

/**
 * @brief Formats analysis results for display in GUI
 * 
 * This presenter handles all formatting logic for analysis results,
 * including unit conversions, precision formatting, and message generation.
 * Follows Clean Architecture: Presentation layer does NOT perform business logic.
 */
class AnalysisResultsPresenter {
public:
    /**
     * @brief Formatted display data for analysis results
     */
    struct DisplayData {
        QString summaryMessage;           // Overall summary
        QString maxDisplacementText;      // "15.234 mm"
        QString maxStressText;            // "125.5 MPa"
        std::vector<QString> nodeDisplacements;  // Per-node displacement strings
        std::vector<QString> memberForces;       // Per-member force strings
        std::vector<QString> memberStresses;     // Per-member stress strings
    };
    
    /**
     * @brief Format analysis results for display
     * 
     * @param results Analysis results view (read-only)
     * @param truss Truss view (read-only)
     * @return DisplayData Formatted strings ready for UI display
     */
    DisplayData formatResults(const truss::core::interfaces::IAnalysisResultsView& results,
                               const truss::core::interfaces::ITrussView& truss) const;
    
    /**
     * @brief Format displacement value
     * 
     * @param displacementMeters Displacement in meters
     * @return QString Formatted string (e.g., "15.234 mm")
     */
    QString formatDisplacement(double displacementMeters) const;
    
    /**
     * @brief Format stress value
     * 
     * @param stressPascals Stress in Pascals
     * @return QString Formatted string (e.g., "125.5 MPa")
     */
    QString formatStress(double stressPascals) const;
    
    /**
     * @brief Format force value
     * 
     * @param forceNewtons Force in Newtons
     * @return QString Formatted string (e.g., "1250.0 N" or "1.25 kN")
     */
    QString formatForce(double forceNewtons) const;
    
    /**
     * @brief Format 2D vector (displacement, force)
     * 
     * @param x X-component
     * @param y Y-component
     * @param unit Unit string (e.g., "mm", "N")
     * @return QString Formatted string (e.g., "(12.3, -5.6) mm")
     */
    QString formatVector2D(double x, double y, const QString& unit) const;
    
    /**
     * @brief Generate success message
     * 
     * @param nodeCount Number of nodes analyzed
     * @param memberCount Number of members analyzed
     * @param maxDisplacement Maximum displacement (meters)
     * @param maxStress Maximum stress (Pascals)
     * @return QString Complete success message
     */
    QString generateSuccessMessage(size_t nodeCount, 
                                     size_t memberCount,
                                     double maxDisplacement,
                                     double maxStress) const;

private:
    // Unit conversion constants
    static constexpr double METERS_TO_MM = 1000.0;
    static constexpr double PASCALS_TO_MPA = 1.0e-6;
    static constexpr double NEWTONS_TO_KN = 0.001;
    
    // Formatting precision
    static constexpr int DISPLACEMENT_PRECISION = 3;
    static constexpr int STRESS_PRECISION = 2;
    static constexpr int FORCE_PRECISION = 1;
};

}  // namespace truss_presenters
