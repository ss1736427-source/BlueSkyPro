#pragma once

#include <string>

namespace bluesky::planning {

struct AcquisitionGeometryInput {
    std::string equipment_id;
    std::string equipment_version;

    // Controlled sensor/calibration parameters.
    double sensor_width_m{0.0};
    double sensor_height_m{0.0};
    double focal_length_m{0.0};
    double image_width_px{0.0};
    double image_height_px{0.0};

    // Controlled mission acquisition requirements.
    double camera_ground_distance_m{0.0};
    double frontal_overlap_ratio{0.0};
    double side_overlap_ratio{0.0};

    // Ground speed used only to derive acquisition timing.
    double ground_speed_mps{0.0};
};

struct AcquisitionGeometryResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;

    double camera_ground_distance_m{0.0};
    double footprint_width_m{0.0};
    double footprint_height_m{0.0};
    double gsd_width_m_per_px{0.0};
    double gsd_height_m_per_px{0.0};
    double track_spacing_m{0.0};
    double image_spacing_m{0.0};
    double trigger_interval_s{0.0};
    double trigger_rate_hz{0.0};
};

class AcquisitionGeometryEngine final {
public:
    static AcquisitionGeometryResult calculate(
        const AcquisitionGeometryInput& input,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
