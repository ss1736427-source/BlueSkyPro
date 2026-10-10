#include "multi_uav_conflict_resolution.hpp"
#include "multi_uav_sequencing.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace bluesky::planning {
namespace {

constexpr double kEarthRadiusM = 6371000.0;
constexpr double kPi = 3.14159265358979323846;
constexpr double kEps = 1e-9;

double rad(double v) { return v * kPi / 180.0; }

struct Vec2 { double north{0.0}; double east{0.0}; };

Vec2 localDelta(const GeoPoint& a, const GeoPoint& b) {
    const double lat = (rad(a.latitude_deg) + rad(b.latitude_deg)) * 0.5;
    return {rad(b.latitude_deg - a.latitude_deg) * kEarthRadiusM,
            rad(b.longitude_deg - a.longitude_deg) * kEarthRadiusM * std::cos(lat)};
}

double norm(const Vec2& v) { return std::hypot(v.north, v.east); }

const TrajectoryPoint4D* pointAtOrBefore(const TrajectoryResult& t, double time) {
    if (t.points.empty() || time < t.points.front().elapsed_time_s - kEps ||
        time > t.points.back().elapsed_time_s + kEps) return nullptr;
    for (std::size_t i = 1; i < t.points.size(); ++i) {
        if (time <= t.points[i].elapsed_time_s + kEps) return &t.points[i];
    }
    return &t.points.back();
}

bool interpolate(const TrajectoryResult& t, double time, TrajectoryPoint4D& out) {
    if (t.points.size() < 2 || time < t.points.front().elapsed_time_s - kEps ||
        time > t.points.back().elapsed_time_s + kEps) return false;
    for (std::size_t i = 1; i < t.points.size(); ++i) {
        const auto& a = t.points[i - 1];
        const auto& b = t.points[i];
        if (time <= b.elapsed_time_s + kEps) {
            const double dt = b.elapsed_time_s - a.elapsed_time_s;
            const double f = dt > kEps ? (time - a.elapsed_time_s) / dt : 0.0;
            out = a;
            out.waypoint_id = a.waypoint_id + "@" + b.waypoint_id;
            out.position.latitude_deg = a.position.latitude_deg +
                (b.position.latitude_deg - a.position.latitude_deg) * f;
            out.position.longitude_deg = a.position.longitude_deg +
                (b.position.longitude_deg - a.position.longitude_deg) * f;
            out.altitude_m = a.altitude_m + (b.altitude_m - a.altitude_m) * f;
            out.elapsed_time_s = time;
            out.cumulative_energy_wh = a.cumulative_energy_wh +
                (b.cumulative_energy_wh - a.cumulative_energy_wh) * f;
            return true;
        }
    }
    return false;
}


std::vector<TrajectoryResult> applyStartDelays(
    const std::vector<TrajectoryResult>& trajectories,
    const std::vector<MultiUavStartDelayInput>& delays) {
    std::vector<TrajectoryResult> result = trajectories;
    for (auto& trajectory : result) {
        double delay = 0.0;
        for (const auto& item : delays) {
            if (item.uav_id == trajectory.uav_id) {
                delay = item.initial_delay_s;
                break;
            }
        }
        if (delay <= kEps) continue;
        for (auto& point : trajectory.points) point.elapsed_time_s += delay;
        std::ostringstream dep;
        dep << trajectory.dependency_identity << "|START_DELAY:" << std::setprecision(17) << delay;
        trajectory.dependency_identity = dep.str();
    }
    return result;
}

const MultiUavResolutionInput* inputFor(
    const std::vector<MultiUavResolutionInput>& inputs, const std::string& id) {
    for (const auto& i : inputs) if (i.uav_id == id) return &i;
    return nullptr;
}

const MultiUavSeparationInput* separationFor(
    const std::vector<MultiUavSeparationInput>& inputs, const std::string& id) {
    for (const auto& i : inputs) if (i.uav_id == id) return &i;
    return nullptr;
}

bool courseVectorAt(const TrajectoryResult& t, double time, Vec2& v) {
    for (std::size_t i = 1; i < t.points.size(); ++i) {
        const auto& a = t.points[i - 1];
        const auto& b = t.points[i];
        if (time <= b.elapsed_time_s + kEps && time >= a.elapsed_time_s - kEps) {
            const auto d = localDelta(a.position, b.position);
            const double dt = b.elapsed_time_s - a.elapsed_time_s;
            if (dt <= kEps || norm(d) <= kEps) return false;
            v = {d.north / dt, d.east / dt};
            return true;
        }
    }
    return false;
}

// The right/left decision is calculated from the signed course angle:
// the angle from the UAV's LZP (planned course vector) to the line of sight
// from that UAV to the other UAV. Positive clockwise angle = right; negative
// angle = left. This is deliberately expressed as an angle, not inferred from
// an unrelated world-axis convention.
double normalizeAngleDeg(double angle) {
    while (angle > 180.0) angle -= 360.0;
    while (angle <= -180.0) angle += 360.0;
    return angle;
}

double courseAngleFromLzpDeg(const Vec2& lzp_course, const Vec2& relative) {
    const double course_norm = norm(lzp_course);
    const double relative_norm = norm(relative);
    if (course_norm <= kEps || relative_norm <= kEps) return 0.0;
    const double dot = lzp_course.north * relative.north +
                       lzp_course.east * relative.east;
    const double cross = lzp_course.north * relative.east -
                         lzp_course.east * relative.north;
    return normalizeAngleDeg(std::atan2(cross, dot) * 180.0 / kPi);
}

int sideFromCourseAngle(double angle_deg) {
    if (std::abs(angle_deg) <= kEps) return 0;
    return angle_deg > 0.0 ? -1 : 1; // -1 = other right, 1 = other left
}

bool applyOffset(TrajectoryResult& t, double start, double end, double offset) {
    if (end <= start + kEps) return false;
    std::vector<TrajectoryPoint4D> out;
    out.reserve(t.points.size() + 2);
    TrajectoryPoint4D ps, pe;
    if (!interpolate(t, start, ps) || !interpolate(t, end, pe)) return false;
    for (const auto& p : t.points) {
        if (p.elapsed_time_s < start - kEps) out.push_back(p);
    }
    ps.altitude_m += offset;
    ps.waypoint_id += "@RESOLVE_START";
    out.push_back(ps);
    for (const auto& p : t.points) {
        if (p.elapsed_time_s > start + kEps && p.elapsed_time_s < end - kEps) {
            auto q = p;
            q.altitude_m += offset;
            q.waypoint_id += "@RESOLVE";
            out.push_back(q);
        }
    }
    pe.altitude_m += offset;
    pe.waypoint_id += "@RESOLVE_END";
    out.push_back(pe);
    for (const auto& p : t.points) {
        if (p.elapsed_time_s > end + kEps) out.push_back(p);
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return a.elapsed_time_s < b.elapsed_time_s;
    });
    t.points = std::move(out);
    std::ostringstream dep;
    dep << t.dependency_identity << "|VERTICAL_RESOLUTION:" << std::setprecision(17)
        << start << "," << end << "," << offset;
    t.dependency_identity = dep.str();
    return true;
}

} // namespace

MultiUavResolutionResult MultiUavConflictResolver::resolve(
    const std::vector<TrajectoryResult>& source,
    const std::vector<MultiUavSeparationInput>& separation,
    const std::vector<MultiUavResolutionInput>& inputs,
    const std::string& calculation_version) {
    MultiUavResolutionResult result;
    result.calculation_version = calculation_version;
    result.trajectories = source;

    std::ostringstream dep;
    dep << calculation_version;
    for (const auto& t : source) dep << "|T:" << t.uav_id << ":" << t.dependency_identity;
    for (const auto& i : inputs) {
        dep << "|R:" << i.uav_id << ":" << std::setprecision(17)
            << i.vertical_resolution_step_m << "," << i.minimum_altitude_m << "," << i.maximum_altitude_m;
    }
    result.dependency_identity = dep.str();

    if (source.size() < 2) {
        result.status = MultiUavResolutionStatus::Resolved;
        return result;
    }

    const auto initial = MultiUavConflictDetector::evaluate(
        result.trajectories, separation, calculation_version + ".initial");
    if (initial.status == MultiUavConflictStatus::Clear) {
        result.status = MultiUavResolutionStatus::Resolved;
        return result;
    }

    // First correction stage: resolve the calculated collision by the minimum
    // necessary ground start delay, bounded by the fixed 5 s planning window.
    std::vector<MultiUavStartDelayInput> delay_inputs;
    delay_inputs.reserve(result.trajectories.size());
    for (const auto& t : result.trajectories) {
        delay_inputs.push_back({t.uav_id, 0.0, 1.0, 5.0});
    }
    const auto sequencing = MultiUavSequencer::resolveByStartDelay(
        result.trajectories, separation, delay_inputs, calculation_version + ".start_delay");
    if (sequencing.status == MultiUavSequencingStatus::Resolved) {
        result.trajectories = applyStartDelays(result.trajectories, sequencing.scheduled_delays);
        bool applied = false;
        for (const auto& d : sequencing.scheduled_delays) {
            if (d.initial_delay_s > kEps) {
                applied = true;
                result.findings.push_back({MultiUavResolutionFindingCode::StartDelayApplied, d.uav_id,
                                           MultiUavResolutionDirection::Up, d.initial_delay_s,
                                           "minimum ground start delay applied within 5 s collision-resolution window"});
            }
        }
        if (applied) {
            result.dependency_identity += "|START_DELAY_WINDOW:5";
            result.status = MultiUavResolutionStatus::Resolved;
            return result;
        }
    }
    if (sequencing.status == MultiUavSequencingStatus::Unresolved &&
        sequencing.unresolved_conflicts.empty()) {
        result.status = MultiUavResolutionStatus::Unresolved;
        return result;
    }
    if (sequencing.status == MultiUavSequencingStatus::Unresolved) {
        result.trajectories = applyStartDelays(result.trajectories, sequencing.scheduled_delays);
    }
    if (initial.findings.empty()) {
        result.status = MultiUavResolutionStatus::Unresolved;
        return result;
    }

    const auto conflict = initial.findings.front();
    const auto* asep = separationFor(separation, conflict.uav_a);
    const auto* bsep = separationFor(separation, conflict.uav_b);
    const double vsep = std::max(asep ? asep->minimum_vertical_separation_m : 0.0,
                                 bsep ? bsep->minimum_vertical_separation_m : 0.0);
    std::size_t ia = result.trajectories.size();
    std::size_t ib = result.trajectories.size();
    for (std::size_t i = 0; i < result.trajectories.size(); ++i) {
        if (result.trajectories[i].uav_id == conflict.uav_a) ia = i;
        if (result.trajectories[i].uav_id == conflict.uav_b) ib = i;
    }
    if (ia == result.trajectories.size() || ib == result.trajectories.size()) {
        result.status = MultiUavResolutionStatus::Unresolved;
        result.residual_conflicts = initial.findings;
        return result;
    }

    Vec2 va, vb;
    TrajectoryPoint4D pa, pb;
    if (!courseVectorAt(result.trajectories[ia], conflict.time_s, va) ||
        !courseVectorAt(result.trajectories[ib], conflict.time_s, vb) ||
        !interpolate(result.trajectories[ia], conflict.time_s, pa) ||
        !interpolate(result.trajectories[ib], conflict.time_s, pb)) {
        result.status = MultiUavResolutionStatus::Unresolved;
        result.findings.push_back({MultiUavResolutionFindingCode::AmbiguousCourseSide, "", MultiUavResolutionDirection::Up,
                                   conflict.time_s, "course side cannot be determined"});
        return result;
    }

    if (std::abs(pa.altitude_m - pb.altitude_m) > vsep + kEps) {
        result.residual_conflicts = initial.findings;
        result.status = MultiUavResolutionStatus::Unresolved;
        return result;
    }

    const auto rel_ab = localDelta(pa.position, pb.position);
    const double angle_b_from_a_deg = courseAngleFromLzpDeg(va, rel_ab);
    const int side_b_from_a = sideFromCourseAngle(angle_b_from_a_deg);
    const auto rel_ba = localDelta(pb.position, pa.position);
    const double angle_a_from_b_deg = courseAngleFromLzpDeg(vb, rel_ba);
    const int side_a_from_b = sideFromCourseAngle(angle_a_from_b_deg);
    if (side_b_from_a == 0 || side_a_from_b == 0 || side_b_from_a == side_a_from_b) {
        result.status = MultiUavResolutionStatus::Unresolved;
        result.findings.push_back({MultiUavResolutionFindingCode::AmbiguousCourseSide, "", MultiUavResolutionDirection::Up,
                                   conflict.time_s, "right/left course side is ambiguous from signed course angle relative to LZP"});
        return result;
    }

    // If the other UAV is right, this UAV is the left-hand UAV and descends.
    // If the other UAV is left, this UAV is the right-hand UAV and climbs.
    const bool aIsRight = side_b_from_a < 0;
    const bool bIsRight = side_a_from_b < 0;
    const auto* ai = inputFor(inputs, conflict.uav_a);
    const auto* bi = inputFor(inputs, conflict.uav_b);
    if (!ai || !bi || ai->vertical_resolution_step_m <= 0.0 ||
        bi->vertical_resolution_step_m <= 0.0 ||
        ai->maximum_altitude_m < ai->minimum_altitude_m ||
        bi->maximum_altitude_m < bi->minimum_altitude_m) {
        result.status = MultiUavResolutionStatus::Infeasible;
        result.findings.push_back({MultiUavResolutionFindingCode::VerticalResolutionBlocked, "",
                                   MultiUavResolutionDirection::Up, conflict.time_s,
                                   "invalid vertical resolution limits"});
        return result;
    }

    const double stepA = ai->vertical_resolution_step_m;
    const double stepB = bi->vertical_resolution_step_m;
    const double offsetA = aIsRight ? stepA : -stepA;
    const double offsetB = bIsRight ? stepB : -stepB;
    const double newAltA = pa.altitude_m + offsetA;
    const double newAltB = pb.altitude_m + offsetB;
    if (newAltA < ai->minimum_altitude_m - kEps || newAltA > ai->maximum_altitude_m + kEps ||
        newAltB < bi->minimum_altitude_m - kEps || newAltB > bi->maximum_altitude_m + kEps) {
        result.status = MultiUavResolutionStatus::Infeasible;
        result.findings.push_back({MultiUavResolutionFindingCode::VerticalResolutionBlocked, "",
                                   MultiUavResolutionDirection::Up, conflict.time_s,
                                   "vertical resolution violates configured altitude limits"});
        return result;
    }

    // Resolve only the current trajectory segment, ending at its next planned point.
    double endA = result.trajectories[ia].points.back().elapsed_time_s;
    double endB = result.trajectories[ib].points.back().elapsed_time_s;
    for (std::size_t k = 1; k < result.trajectories[ia].points.size(); ++k)
        if (conflict.time_s <= result.trajectories[ia].points[k].elapsed_time_s + kEps) { endA = result.trajectories[ia].points[k].elapsed_time_s; break; }
    for (std::size_t k = 1; k < result.trajectories[ib].points.size(); ++k)
        if (conflict.time_s <= result.trajectories[ib].points[k].elapsed_time_s + kEps) { endB = result.trajectories[ib].points[k].elapsed_time_s; break; }
    const double end = std::min(endA, endB);
    if (!applyOffset(result.trajectories[ia], conflict.time_s, end, offsetA) ||
        !applyOffset(result.trajectories[ib], conflict.time_s, end, offsetB)) {
        result.status = MultiUavResolutionStatus::Unresolved;
        result.residual_conflicts = initial.findings;
        return result;
    }

    result.findings.push_back({MultiUavResolutionFindingCode::SameAltitudeConflict, conflict.uav_a,
                               MultiUavResolutionDirection::Up, conflict.time_s, "same-altitude conflict resolved by vertical profile"});
    result.findings.push_back({MultiUavResolutionFindingCode::RightOfWayAssignment, conflict.uav_a,
                               aIsRight ? MultiUavResolutionDirection::Up : MultiUavResolutionDirection::Down,
                               conflict.time_s, aIsRight ? "right-by-course UAV climbs +1 m" : "left-by-course UAV descends -1 m"});
    result.findings.push_back({MultiUavResolutionFindingCode::RightOfWayAssignment, conflict.uav_b,
                               bIsRight ? MultiUavResolutionDirection::Up : MultiUavResolutionDirection::Down,
                               conflict.time_s, bIsRight ? "right-by-course UAV climbs +1 m" : "left-by-course UAV descends -1 m"});

    const auto recheck = MultiUavConflictDetector::evaluate(
        result.trajectories, separation, calculation_version + ".recheck");
    if (recheck.status == MultiUavConflictStatus::Clear) {
        result.status = MultiUavResolutionStatus::Resolved;
        return result;
    }

    result.status = MultiUavResolutionStatus::Unresolved;
    result.residual_conflicts = recheck.findings;
    result.findings.push_back({MultiUavResolutionFindingCode::ResidualConflict, "",
                               MultiUavResolutionDirection::Up, conflict.time_s,
                               "conflict remains after pre-flight vertical resolution"});
    return result;
}
} // namespace bluesky::planning
