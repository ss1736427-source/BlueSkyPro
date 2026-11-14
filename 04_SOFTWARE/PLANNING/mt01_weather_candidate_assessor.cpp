#include "../WEATHER/weather_candidate_assessor.hpp"

#include <sstream>
#include <utility>

namespace bluesky::weather {

bluesky::planning::CandidateWeatherAssessor makeMt01CandidateWeatherAssessor(
    std::vector<WindField> fields,
    const bluesky::planning::Mission& mission,
    const bluesky::planning::Mt01MappingPipelineResult& pipeline,
    const bluesky::planning::CoverageCameraModel& camera,
    const bluesky::planning::VehicleEquipmentCapabilityProfile& capability,
    bluesky::planning::TrajectoryPerformanceInput basePerformance,
    std::string snapshotId,
    std::string snapshotVersion,
    std::string calculationVersion,
    int maximumVerticalGapM)
{
    using namespace bluesky::planning;
    auto binding = Mt01WeatherPerformanceAdapter::bind(
        mission, pipeline, camera, capability, std::move(basePerformance));
    if (!binding.valid) {
        std::ostringstream error;
        for (std::size_t i = 0; i < binding.diagnostics.size(); ++i) {
            if (i) error << "; ";
            error << binding.diagnostics[i];
        }
        const auto message = error.str();
        return [message](const MissionProblem&, const CandidateSolution&) {
            CandidateWeatherAssessment result;
            result.error = message.empty() ? "MT01_WEATHER_PERFORMANCE_BINDING_FAILED" : message;
            return result;
        };
    }
    return makeCandidateWeatherAssessor(
        std::move(fields), std::move(binding.performance), std::move(snapshotId),
        std::move(snapshotVersion), std::move(calculationVersion), maximumVerticalGapM);
}


} // namespace bluesky::weather
