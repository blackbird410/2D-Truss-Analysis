#include "AnalysisResultsPresenter.hpp"

#include <cmath>

namespace truss_presenters {

AnalysisResultsPresenter::DisplayData AnalysisResultsPresenter::formatResults(
    const truss::core::interfaces::IAnalysisResultsView& results,
    const truss::core::interfaces::ITrussView& truss) const {
    DisplayData data;

    // Summary message
    data.summaryMessage = generateSuccessMessage(truss.getNodeCount(),
                                                 truss.getMemberCount(),
                                                 results.getMaxDisplacement(),
                                                 results.getMaxStress());

    // Max displacement
    data.maxDisplacementText = formatDisplacement(results.getMaxDisplacement());

    // Max stress
    data.maxStressText = formatStress(results.getMaxStress());

    // Get node and member views
    auto nodeViews = truss.getNodeViews();
    auto memberViews = truss.getMemberViews();

    // Per-node displacements
    const auto& displacements = results.getDisplacements();
    for (size_t i = 0; i < nodeViews.size(); ++i) {
        const auto& nodeView = nodeViews[i];
        size_t dofIndex = i * 2;  // 2 DOFs per node

        QString nodeText = QString("Node %1: %2")
                               .arg(nodeView.id)
                               .arg(formatVector2D(displacements[dofIndex] * METERS_TO_MM,
                                                   displacements[dofIndex + 1] * METERS_TO_MM,
                                                   "mm"));

        data.nodeDisplacements.push_back(nodeText);
    }

    // Per-member forces and stresses
    const auto& forces = results.getMemberForces();
    const auto& stresses = results.getMemberStresses();

    for (size_t i = 0; i < memberViews.size(); ++i) {
        const auto& memberView = memberViews[i];

        QString forceText = QString("Member %1: %2").arg(memberView.id).arg(formatForce(forces[i]));

        QString stressText =
            QString("Member %1: %2").arg(memberView.id).arg(formatStress(stresses[i]));

        data.memberForces.push_back(forceText);
        data.memberStresses.push_back(stressText);
    }

    return data;
}

QString AnalysisResultsPresenter::formatDisplacement(double displacementMeters) {
    double displacementMm = displacementMeters * METERS_TO_MM;
    return QString("%1 mm").arg(displacementMm, 0, 'f', DISPLACEMENT_PRECISION);
}

QString AnalysisResultsPresenter::formatStress(double stressPascals) {
    double stressMPa = stressPascals * PASCALS_TO_MPA;
    return QString("%1 MPa").arg(stressMPa, 0, 'f', STRESS_PRECISION);
}

QString AnalysisResultsPresenter::formatForce(double forceNewtons) {
    // Use kN for large forces
    if (std::abs(forceNewtons) >= 1000.0) {
        double forceKN = forceNewtons * NEWTONS_TO_KN;
        return QString("%1 kN").arg(forceKN, 0, 'f', FORCE_PRECISION);
    } else {
        return QString("%1 N").arg(forceNewtons, 0, 'f', FORCE_PRECISION);
    }
}

QString AnalysisResultsPresenter::formatVector2D(double x, double y, const QString& unit) {
    return QString("(%1, %2) %3")
        .arg(x, 0, 'f', DISPLACEMENT_PRECISION)
        .arg(y, 0, 'f', DISPLACEMENT_PRECISION)
        .arg(unit);
}

QString AnalysisResultsPresenter::generateSuccessMessage(size_t nodeCount,
                                                         size_t memberCount,
                                                         double maxDisplacement,
                                                         double maxStress) {
    return QString("Analysis completed successfully!\n"
                   "Nodes: %1, Members: %2\n"
                   "Max displacement: %3\n"
                   "Max stress: %4")
        .arg(nodeCount)
        .arg(memberCount)
        .arg(formatDisplacement(maxDisplacement))
        .arg(formatStress(maxStress));
}

}  // namespace truss_presenters
