#include "weather_field.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace bluesky::weather {
namespace {

bool validGrid(const WindField& f)
{
    if (f.rows < 2 || f.columns < 2 ||
        f.latitudes_deg.size() != f.rows ||
        f.longitudes_deg.size() != f.columns ||
        f.eastward_mps.size() != f.rows * f.columns ||
        f.northward_mps.size() != f.rows * f.columns)
        return false;
    if (!std::is_sorted(f.latitudes_deg.begin(), f.latitudes_deg.end()) ||
        !std::is_sorted(f.longitudes_deg.begin(), f.longitudes_deg.end()))
        return false;
    return std::all_of(f.latitudes_deg.begin(), f.latitudes_deg.end(),
                       [](double v) { return std::isfinite(v); }) &&
           std::all_of(f.longitudes_deg.begin(), f.longitudes_deg.end(),
                       [](double v) { return std::isfinite(v); });
}

bool bracket(const std::vector<double>& axis, double value,
             std::size_t& low, std::size_t& high, double& fraction)
{
    if (!std::isfinite(value) || axis.size() < 2 ||
        value < axis.front() || value > axis.back())
        return false;
    auto upper = std::lower_bound(axis.begin(), axis.end(), value);
    if (upper != axis.end() && std::abs(*upper - value) <= 1e-9) {
        const auto index = static_cast<std::size_t>(upper - axis.begin());
        if (index + 1 < axis.size()) {
            low = index;
            high = index + 1;
            fraction = 0.0;
        } else {
            low = index - 1;
            high = index;
            fraction = 1.0;
        }
        return true;
    }
    if (upper == axis.begin() || upper == axis.end())
        return false;
    high = static_cast<std::size_t>(upper - axis.begin());
    low = high - 1;
    const double span = axis[high] - axis[low];
    if (!(span > 0.0))
        return false;
    fraction = (value - axis[low]) / span;
    return true;
}

double bilinear(double q00, double q01, double q10, double q11,
                double x, double y)
{
    return (1.0 - x) * (1.0 - y) * q00 +
           x * (1.0 - y) * q01 +
           (1.0 - x) * y * q10 +
           x * y * q11;
}

} // namespace

WindQuery WindInterpolator::horizontal(const WindField& f,
                                       double latitude_deg,
                                       double longitude_deg)
{
    WindQuery out;
    if (!validGrid(f)) {
        out.quality = WindQuality::InvalidGrid;
        out.detail = "grid dimensions, axes or component arrays are inconsistent";
        return out;
    }

    std::size_t y0 = 0, y1 = 0, x0 = 0, x1 = 0;
    double fy = 0.0, fx = 0.0;
    if (!bracket(f.latitudes_deg, latitude_deg, y0, y1, fy) ||
        !bracket(f.longitudes_deg, longitude_deg, x0, x1, fx)) {
        out.quality = WindQuality::OutsideGrid;
        out.detail = "query point is outside the decoded weather grid";
        return out;
    }

    const auto at = [&f](std::size_t y, std::size_t x) {
        return y * f.columns + x;
    };
    const std::size_t indices[] = {at(y0, x0), at(y0, x1),
                                   at(y1, x0), at(y1, x1)};
    for (const auto i : indices) {
        if (!std::isfinite(f.eastward_mps[i]) ||
            !std::isfinite(f.northward_mps[i])) {
            out.quality = WindQuality::MissingGridValue;
            out.detail = "one or more bilinear corner values are missing";
            return out;
        }
    }

    out.eastward_mps = bilinear(f.eastward_mps[indices[0]],
                                f.eastward_mps[indices[1]],
                                f.eastward_mps[indices[2]],
                                f.eastward_mps[indices[3]], fx, fy);
    out.northward_mps = bilinear(f.northward_mps[indices[0]],
                                 f.northward_mps[indices[1]],
                                 f.northward_mps[indices[2]],
                                 f.northward_mps[indices[3]], fx, fy);
    if (!std::isfinite(out.eastward_mps) || !std::isfinite(out.northward_mps)) {
        out.quality = WindQuality::MissingGridValue;
        out.detail = "interpolated wind vector is not finite";
        return out;
    }
    out.valid = true;
    out.quality = WindQuality::Good;
    out.detail = "bilinear interpolation passed";
    return out;
}

WindQuery WindInterpolator::atAltitude(const std::vector<WindField>& fields,
                                       double latitude_deg,
                                       double longitude_deg,
                                       double altitude_agl_m,
                                       int max_gap_m)
{
    WindQuery out;
    if (fields.empty() || !std::isfinite(altitude_agl_m) ||
        altitude_agl_m < 0.0 || max_gap_m <= 0) {
        out.quality = WindQuality::MissingVerticalCoverage;
        out.detail = "no fields or invalid AGL altitude/interpolation policy";
        return out;
    }

    std::vector<const WindField*> ordered;
    ordered.reserve(fields.size());
    for (const auto& f : fields)
        ordered.push_back(&f);
    std::sort(ordered.begin(), ordered.end(),
              [](const WindField* a, const WindField* b) {
                  return a->level_agl_m < b->level_agl_m;
              });

    for (std::size_t i = 1; i < ordered.size(); ++i) {
        if (ordered[i - 1]->level_agl_m == ordered[i]->level_agl_m) {
            out.quality = WindQuality::InvalidGrid;
            out.detail = "duplicate AGL model level";
            return out;
        }
        if (ordered[i - 1]->valid_date_utc != ordered[i]->valid_date_utc ||
            ordered[i - 1]->valid_time_utc != ordered[i]->valid_time_utc) {
            out.quality = WindQuality::InconsistentValidTime;
            out.detail = "vertical levels belong to different forecast valid times";
            return out;
        }
    }

    auto upper = std::lower_bound(ordered.begin(), ordered.end(), altitude_agl_m,
        [](const WindField* f, double h) { return f->level_agl_m < h; });

    if (upper != ordered.end() &&
        std::abs(static_cast<double>((*upper)->level_agl_m) - altitude_agl_m) <= 1e-6)
        return horizontal(**upper, latitude_deg, longitude_deg);

    if (upper == ordered.begin() || upper == ordered.end()) {
        out.quality = WindQuality::MissingVerticalCoverage;
        out.detail = "requested AGL altitude is outside available model levels; extrapolation is disabled";
        return out;
    }

    const WindField& high = **upper;
    const WindField& low = **(upper - 1);
    const int gap = high.level_agl_m - low.level_agl_m;
    if (gap <= 0 || gap > max_gap_m) {
        out.quality = WindQuality::VerticalGapTooLarge;
        out.detail = "vertical model-level gap exceeds the configured interpolation limit";
        return out;
    }

    if (low.valid_date_utc != high.valid_date_utc ||
        low.valid_time_utc != high.valid_time_utc) {
        out.quality = WindQuality::InconsistentValidTime;
        out.detail = "bracketing levels have different forecast valid times";
        return out;
    }

    const WindQuery a = horizontal(low, latitude_deg, longitude_deg);
    if (!a.valid) return a;
    const WindQuery b = horizontal(high, latitude_deg, longitude_deg);
    if (!b.valid) return b;

    const double t = (altitude_agl_m - low.level_agl_m) / gap;
    out.eastward_mps = a.eastward_mps + t * (b.eastward_mps - a.eastward_mps);
    out.northward_mps = a.northward_mps + t * (b.northward_mps - a.northward_mps);
    out.valid = std::isfinite(out.eastward_mps) && std::isfinite(out.northward_mps);
    out.quality = out.valid ? WindQuality::Good : WindQuality::MissingGridValue;
    out.detail = out.valid ? "bounded vertical interpolation passed" :
                             "vertical interpolation produced a non-finite vector";
    return out;
}

} // namespace bluesky::weather
