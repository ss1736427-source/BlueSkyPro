#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::weather {

// A decoded GFS field is a regular latitude/longitude grid. Axes are normalized
// to ascending order; values use row-major [latitude][longitude] order.
struct WindField {
    int level_agl_m{0};
    int valid_date_utc{0}; // YYYYMMDD
    int valid_time_utc{0}; // HHMM
    std::size_t rows{0};
    std::size_t columns{0};
    std::vector<double> latitudes_deg;
    std::vector<double> longitudes_deg;
    std::vector<double> eastward_mps;
    std::vector<double> northward_mps;
};

enum class WindQuality {
    Good,
    OutsideGrid,
    MissingGridValue,
    InvalidGrid,
    MissingVerticalCoverage,
    VerticalGapTooLarge,
    InconsistentValidTime,
    MissingTemporalCoverage,
    TemporalGapTooLarge
};

struct WindQuery {
    bool valid{false};
    double eastward_mps{0.0};
    double northward_mps{0.0};
    WindQuality quality{WindQuality::InvalidGrid};
    std::string detail;
};

class WindInterpolator final {
public:
    // Bilinear interpolation. No extrapolation and no substitution of missing
    // grid values are permitted.
    static WindQuery horizontal(const WindField& field,
                                double latitude_deg,
                                double longitude_deg);

    // Exact model levels are preferred. Vertical interpolation is allowed only
    // between bracketing AGL levels whose separation does not exceed max_gap_m.
    static WindQuery atTimeAndAltitude(
        const std::vector<WindField>& fields,
        double latitude_deg,
        double longitude_deg,
        double altitude_agl_m,
        std::int64_t valid_time_utc_seconds,
        int maximum_vertical_gap_m = 100,
        std::int64_t maximum_temporal_gap_seconds = 10800);

    static WindQuery atAltitude(const std::vector<WindField>& fields,
                                double latitude_deg,
                                double longitude_deg,
                                double altitude_agl_m,
                                int max_gap_m = 100);
};

} // namespace bluesky::weather
