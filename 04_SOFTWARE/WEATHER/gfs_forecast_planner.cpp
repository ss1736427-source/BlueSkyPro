#include "gfs_forecast_planner.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <iterator>

namespace bluesky::weather {
namespace {

struct ParsedUtc {
    std::int64_t seconds{0};
};

bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int daysInMonth(int year, int month)
{
    static constexpr std::array<int, 12> days{
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (month < 1 || month > 12)
        return 0;
    if (month == 2 && isLeapYear(year))
        return 29;
    return days[static_cast<std::size_t>(month - 1)];
}

// Proleptic Gregorian calendar conversion, with 1970-01-01 as day zero.
std::int64_t daysFromCivil(int year, unsigned month, unsigned day)
{
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yearOfEra = static_cast<unsigned>(year - era * 400);
    const int shiftedMonth = static_cast<int>(month) + (month > 2 ? -3 : 9);
    const unsigned dayOfYear =
        (153 * static_cast<unsigned>(shiftedMonth) + 2) / 5 + day - 1;
    const unsigned dayOfEra =
        yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    return static_cast<std::int64_t>(era) * 146097
        + static_cast<std::int64_t>(dayOfEra) - 719468;
}

bool digits(const std::string& value, std::size_t offset, std::size_t count)
{
    if (offset + count > value.size())
        return false;
    for (std::size_t i = offset; i < offset + count; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(value[i])))
            return false;
    }
    return true;
}

int numberAt(const std::string& value, std::size_t offset, std::size_t count)
{
    int number = 0;
    for (std::size_t i = offset; i < offset + count; ++i)
        number = number * 10 + (value[i] - '0');
    return number;
}

bool parseUtc(const std::string& value, ParsedUtc& result)
{
    if (value.size() != 20 || value[4] != '-' || value[7] != '-'
        || value[10] != 'T' || value[13] != ':' || value[16] != ':'
        || value[19] != 'Z'
        || !digits(value, 0, 4) || !digits(value, 5, 2)
        || !digits(value, 8, 2) || !digits(value, 11, 2)
        || !digits(value, 14, 2) || !digits(value, 17, 2)) {
        return false;
    }

    const int year = numberAt(value, 0, 4);
    const int month = numberAt(value, 5, 2);
    const int day = numberAt(value, 8, 2);
    const int hour = numberAt(value, 11, 2);
    const int minute = numberAt(value, 14, 2);
    const int second = numberAt(value, 17, 2);

    if (year < 1970 || month < 1 || month > 12
        || day < 1 || day > daysInMonth(year, month)
        || hour > 23 || minute > 59 || second > 59) {
        return false;
    }

    result.seconds = daysFromCivil(
        year, static_cast<unsigned>(month), static_cast<unsigned>(day)) * 86400
        + hour * 3600 + minute * 60 + second;
    return true;
}

bool validSynopticCycle(const std::string& cycleUtc, ParsedUtc& parsed)
{
    if (!parseUtc(cycleUtc, parsed))
        return false;
    const int hour = numberAt(cycleUtc, 11, 2);
    return hour % 6 == 0 && cycleUtc.substr(14, 5) == "00:00";
}

std::vector<int> forecastLeadSchedule()
{
    std::vector<int> leads;
    leads.reserve(209);
    for (int hour = 0; hour <= 120; ++hour)
        leads.push_back(hour);
    for (int hour = 123; hour <= 384; hour += 3)
        leads.push_back(hour);
    return leads;
}

bool bracketWindow(
    const std::vector<int>& schedule,
    double startLead,
    double endLead,
    std::size_t& first,
    std::size_t& last)
{
    if (startLead < 0.0 || endLead < startLead || endLead > 384.0)
        return false;

    const auto lower = std::upper_bound(schedule.begin(), schedule.end(), startLead);
    if (lower == schedule.begin())
        first = 0;
    else if (lower == schedule.end())
        first = schedule.size() - 1;
    else
        first = static_cast<std::size_t>(std::distance(schedule.begin(), lower) - 1);

    const auto upper = std::lower_bound(schedule.begin(), schedule.end(), endLead);
    if (upper == schedule.end())
        return false;
    last = static_cast<std::size_t>(std::distance(schedule.begin(), upper));
    return first <= last;
}

} // namespace

GfsForecastPlan GfsForecastPlanner::build(const GfsForecastPlanRequest& request)
{
    GfsForecastPlan result;
    ParsedUtc missionStart;
    ParsedUtc missionEnd;
    if (!parseUtc(request.mission_start_utc, missionStart)
        || !parseUtc(request.mission_end_utc, missionEnd)
        || missionEnd.seconds < missionStart.seconds) {
        result.status = GfsForecastPlanStatus::InvalidRequest;
        result.reason = "Mission window must be valid UTC timestamps with end >= start.";
        return result;
    }

    if (request.candidates.empty()) {
        result.status = GfsForecastPlanStatus::NoAvailableCycle;
        result.reason = "No provider-confirmed GFS forecast cycles were supplied.";
        return result;
    }

    std::vector<std::pair<std::string, ParsedUtc>> available;
    available.reserve(request.candidates.size());
    for (const auto& candidate : request.candidates) {
        ParsedUtc cycle;
        if (!validSynopticCycle(candidate.cycle_time_utc, cycle)) {
            result.status = GfsForecastPlanStatus::InvalidCycleCandidate;
            result.reason = "A candidate cycle has an invalid UTC timestamp or is not a 00/06/12/18Z cycle.";
            return result;
        }
        if (candidate.data_available)
            available.emplace_back(candidate.cycle_time_utc, cycle);
    }

    if (available.empty()) {
        result.status = GfsForecastPlanStatus::NoAvailableCycle;
        result.reason = "None of the supplied GFS cycles is confirmed available.";
        return result;
    }

    std::sort(available.begin(), available.end(),
        [](const auto& left, const auto& right) {
            return left.second.seconds < right.second.seconds;
        });

    const auto schedule = forecastLeadSchedule();
    for (auto it = available.rbegin(); it != available.rend(); ++it) {
        const double startLead =
            static_cast<double>(missionStart.seconds - it->second.seconds) / 3600.0;
        const double endLead =
            static_cast<double>(missionEnd.seconds - it->second.seconds) / 3600.0;

        std::size_t first = 0;
        std::size_t last = 0;
        if (!bracketWindow(schedule, startLead, endLead, first, last))
            continue;

        result.status = GfsForecastPlanStatus::Ready;
        result.selected_cycle_utc = it->first;
        result.forecast_lead_hours.assign(
            schedule.begin() + static_cast<std::ptrdiff_t>(first),
            schedule.begin() + static_cast<std::ptrdiff_t>(last + 1));
        result.reason = "One confirmed GFS run brackets the complete mission window.";
        return result;
    }

    result.status = GfsForecastPlanStatus::NoSingleRunCoverage;
    result.reason = "No single confirmed GFS run brackets the complete mission window within F000-F384.";
    return result;
}

} // namespace bluesky::weather
