#pragma once

#include "mission_lifecycle_terrain_adapter.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace bluesky::planning {

// Decoded, immutable, north-up DEM window. Coordinates describe pixel centres;
// row zero is the southernmost row and column zero the westernmost column.
// The acquisition/decoder is responsible for authenticating bytes and metadata.
struct TerrainDemGrid {
    std::string snapshot_id;
    std::string source_id;
    std::string source_version;
    std::string dataset_id;
    std::string dataset_version;
    std::string horizontal_crs{"EPSG:4326"};
    std::string vertical_datum;
    double origin_latitude_deg{0.0};
    double origin_longitude_deg{0.0};
    double latitude_step_deg{0.0};
    double longitude_step_deg{0.0};
    double horizontal_resolution_m{0.0};
    double vertical_accuracy_m{0.0};
    std::size_t rows{0};
    std::size_t columns{0};
    // Row-major elevations in metres in vertical_datum. NaN denotes NoData.
    std::vector<double> elevation_m;
};

struct TerrainProfileValidationOptions {
    // The route is sampled no farther apart than this distance. Zero selects
    // half the declared DEM horizontal resolution.
    double maximum_sample_gap_m{0.0};
    // Additional clearance margin beyond the DEM's published vertical accuracy.
    double additional_clearance_margin_m{0.0};
    std::size_t maximum_profile_samples{2000000};
};

struct TerrainProfileAssessment {
    TerrainDemSnapshot snapshot;
    TerrainProfileValidationResult validation;
    std::vector<std::string> findings;
};

class TerrainProfileValidator final {
public:
    // Evaluates the complete canonical route against decoded DEM elevations.
    // Route waypoint heights are AGL; DEM heights must share one known vertical
    // datum. Missing cells, incomplete coverage or stale route lineage fail closed.
    static TerrainProfileAssessment evaluate(
        const Route& route,
        const TerrainDemGrid& dem,
        const TerrainProfileValidationOptions& options = {});
};

} // namespace bluesky::planning
