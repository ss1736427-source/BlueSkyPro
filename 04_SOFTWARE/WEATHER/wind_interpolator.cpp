#include "weather_field.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>

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
    const auto strictlyIncreasing=[](const std::vector<double>& axis) {
        return std::adjacent_find(axis.begin(),axis.end(),
            [](double a,double b){ return !std::isfinite(a) || !std::isfinite(b) || b<=a; })
            ==axis.end()
            && std::all_of(axis.begin(),axis.end(),
                [](double v){ return std::isfinite(v); });
    };
    return strictlyIncreasing(f.latitudes_deg) && strictlyIncreasing(f.longitudes_deg);
}

bool bracket(const std::vector<double>& axis, double value,
             std::size_t& low, std::size_t& high, double& fraction)
{
    if (!std::isfinite(value) || axis.size()<2 ||
        value<axis.front() || value>axis.back()) return false;
    auto upper=std::lower_bound(axis.begin(),axis.end(),value);
    if (upper!=axis.end() && std::abs(*upper-value)<=1e-9) {
        const auto index=static_cast<std::size_t>(upper-axis.begin());
        if (index+1<axis.size()) { low=index; high=index+1; fraction=0.0; }
        else { low=index-1; high=index; fraction=1.0; }
        return true;
    }
    if (upper==axis.begin() || upper==axis.end()) return false;
    high=static_cast<std::size_t>(upper-axis.begin());
    low=high-1;
    const double span=axis[high]-axis[low];
    if (!(span>0.0)) return false;
    fraction=(value-axis[low])/span;
    return true;
}

double bilinear(double q00,double q01,double q10,double q11,double x,double y)
{
    return (1.0-x)*(1.0-y)*q00+x*(1.0-y)*q01+(1.0-x)*y*q10+x*y*q11;
}

bool validForecastTime(const WindField& field,std::int64_t& seconds)
{
    const int date=field.valid_date_utc, time=field.valid_time_utc;
    const int year=date/10000, month=(date/100)%100, day=date%100;
    const int hour=time/100, minute=time%100;
    if (year<1970 || month<1 || month>12 || day<1 || hour<0 || hour>23
        || minute<0 || minute>59) return false;
    static constexpr int monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
    const bool leap=(year%4==0 && year%100!=0) || year%400==0;
    if (day>monthDays[month-1]+(month==2 && leap ? 1 : 0)) return false;
    int y=year-(month<=2);
    const int era=(y>=0 ? y : y-399)/400;
    const unsigned yoe=static_cast<unsigned>(y-era*400);
    const int shifted=month+(month>2 ? -3 : 9);
    const unsigned doy=(153*static_cast<unsigned>(shifted)+2)/5
                       +static_cast<unsigned>(day)-1;
    const unsigned doe=yoe*365+yoe/4-yoe/100+doy;
    const std::int64_t days=static_cast<std::int64_t>(era)*146097
                            +static_cast<std::int64_t>(doe)-719468;
    seconds=days*86400+hour*3600+minute*60;
    return true;
}

WindQuery interpolateAltitude(const std::vector<const WindField*>& fields,
                              double latitude,double longitude,double altitude,int maxGap)
{
    WindQuery out;
    if (fields.empty() || !std::isfinite(altitude) || altitude<0.0 || maxGap<=0) {
        out.quality=WindQuality::MissingVerticalCoverage;
        out.detail="no fields or invalid AGL altitude/interpolation policy";
        return out;
    }
    auto ordered=fields;
    std::sort(ordered.begin(),ordered.end(),
        [](const WindField* a,const WindField* b){ return a->level_agl_m<b->level_agl_m; });
    for (std::size_t i=1;i<ordered.size();++i) {
        if (ordered[i-1]->level_agl_m==ordered[i]->level_agl_m) {
            out.quality=WindQuality::InvalidGrid;
            out.detail="duplicate AGL model level";
            return out;
        }
    }
    auto upper=std::lower_bound(ordered.begin(),ordered.end(),altitude,
        [](const WindField* f,double h){ return f->level_agl_m<h; });
    if (upper!=ordered.end() &&
        std::abs(static_cast<double>((*upper)->level_agl_m)-altitude)<=1e-6)
        return WindInterpolator::horizontal(**upper,latitude,longitude);
    if (upper==ordered.begin() || upper==ordered.end()) {
        out.quality=WindQuality::MissingVerticalCoverage;
        out.detail="requested AGL altitude is outside available model levels; extrapolation is disabled";
        return out;
    }
    const WindField& high=**upper;
    const WindField& low=**(upper-1);
    const int gap=high.level_agl_m-low.level_agl_m;
    if (gap<=0 || gap>maxGap) {
        out.quality=WindQuality::VerticalGapTooLarge;
        out.detail="vertical model-level gap exceeds the configured interpolation limit";
        return out;
    }
    std::int64_t lowTime=0,highTime=0;
    if (!validForecastTime(low,lowTime) || !validForecastTime(high,highTime)
        || lowTime!=highTime) {
        out.quality=WindQuality::InconsistentValidTime;
        out.detail="bracketing vertical levels have different or invalid forecast valid times";
        return out;
    }
    const auto a=WindInterpolator::horizontal(low,latitude,longitude);
    if (!a.valid) return a;
    const auto b=WindInterpolator::horizontal(high,latitude,longitude);
    if (!b.valid) return b;
    const double t=(altitude-low.level_agl_m)/gap;
    out.eastward_mps=a.eastward_mps+t*(b.eastward_mps-a.eastward_mps);
    out.northward_mps=a.northward_mps+t*(b.northward_mps-a.northward_mps);
    out.valid=std::isfinite(out.eastward_mps) && std::isfinite(out.northward_mps);
    out.quality=out.valid ? WindQuality::Good : WindQuality::MissingGridValue;
    out.detail=out.valid ? "bounded vertical interpolation passed"
                         : "vertical interpolation produced a non-finite vector";
    return out;
}

} // namespace

WindQuery WindInterpolator::horizontal(const WindField& f,double latitude,double longitude)
{
    WindQuery out;
    if (!validGrid(f)) {
        out.quality=WindQuality::InvalidGrid;
        out.detail="grid dimensions, axes or component arrays are inconsistent";
        return out;
    }
    std::size_t y0=0,y1=0,x0=0,x1=0;
    double fy=0.0,fx=0.0;
    if (!bracket(f.latitudes_deg,latitude,y0,y1,fy) ||
        !bracket(f.longitudes_deg,longitude,x0,x1,fx)) {
        out.quality=WindQuality::OutsideGrid;
        out.detail="query point is outside the decoded weather grid";
        return out;
    }
    const auto at=[&f](std::size_t y,std::size_t x){ return y*f.columns+x; };
    const std::size_t ids[]={at(y0,x0),at(y0,x1),at(y1,x0),at(y1,x1)};
    for (auto id:ids) {
        if (!std::isfinite(f.eastward_mps[id]) || !std::isfinite(f.northward_mps[id])) {
            out.quality=WindQuality::MissingGridValue;
            out.detail="one or more bilinear corner values are missing";
            return out;
        }
    }
    out.eastward_mps=bilinear(f.eastward_mps[ids[0]],f.eastward_mps[ids[1]],
                              f.eastward_mps[ids[2]],f.eastward_mps[ids[3]],fx,fy);
    out.northward_mps=bilinear(f.northward_mps[ids[0]],f.northward_mps[ids[1]],
                               f.northward_mps[ids[2]],f.northward_mps[ids[3]],fx,fy);
    if (!std::isfinite(out.eastward_mps) || !std::isfinite(out.northward_mps)) {
        out.quality=WindQuality::MissingGridValue;
        out.detail="interpolated wind vector is not finite";
        return out;
    }
    out.valid=true;
    out.quality=WindQuality::Good;
    out.detail="bilinear interpolation passed";
    return out;
}

WindQuery WindInterpolator::atAltitude(const std::vector<WindField>& fields,
                                       double latitude,double longitude,
                                       double altitude,int maxGap)
{
    std::vector<const WindField*> pointers;
    pointers.reserve(fields.size());
    std::int64_t expectedTime=0;
    bool hasTime=false;
    for (const auto& field:fields) {
        std::int64_t current=0;
        if (!validForecastTime(field,current) || (hasTime && current!=expectedTime)) {
            WindQuery out;
            out.quality=WindQuality::InconsistentValidTime;
            out.detail="vertical fields belong to different or invalid forecast valid times";
            return out;
        }
        expectedTime=current;
        hasTime=true;
        pointers.push_back(&field);
    }
    return interpolateAltitude(pointers,latitude,longitude,altitude,maxGap);
}

WindQuery WindInterpolator::atTimeAndAltitude(
    const std::vector<WindField>& fields,double latitude,double longitude,
    double altitude,std::int64_t targetTime,int maxVerticalGap,
    std::int64_t maxTemporalGap)
{
    WindQuery out;
    if (fields.empty() || maxTemporalGap<=0) {
        out.quality=WindQuality::MissingTemporalCoverage;
        out.detail="forecast fields or temporal interpolation policy are missing";
        return out;
    }
    std::map<std::int64_t,std::vector<const WindField*>> byTime;
    for (const auto& field:fields) {
        std::int64_t validTime=0;
        if (!validForecastTime(field,validTime)) {
            out.quality=WindQuality::InconsistentValidTime;
            out.detail="one or more forecast fields have an invalid valid-time stamp";
            return out;
        }
        byTime[validTime].push_back(&field);
    }
    const auto upper=byTime.lower_bound(targetTime);
    if (upper!=byTime.end() && upper->first==targetTime)
        return interpolateAltitude(upper->second,latitude,longitude,altitude,maxVerticalGap);
    if (upper==byTime.begin() || upper==byTime.end()) {
        out.quality=WindQuality::MissingTemporalCoverage;
        out.detail="requested trajectory time is outside available forecast valid times; extrapolation is disabled";
        return out;
    }
    const auto lower=std::prev(upper);
    const std::int64_t gap=upper->first-lower->first;
    if (gap<=0 || gap>maxTemporalGap) {
        out.quality=WindQuality::TemporalGapTooLarge;
        out.detail="forecast valid-time gap exceeds the configured temporal interpolation limit";
        return out;
    }
    const auto before=interpolateAltitude(lower->second,latitude,longitude,altitude,maxVerticalGap);
    if (!before.valid) return before;
    const auto after=interpolateAltitude(upper->second,latitude,longitude,altitude,maxVerticalGap);
    if (!after.valid) return after;
    const double fraction=static_cast<double>(targetTime-lower->first)/static_cast<double>(gap);
    out.eastward_mps=before.eastward_mps+fraction*(after.eastward_mps-before.eastward_mps);
    out.northward_mps=before.northward_mps+fraction*(after.northward_mps-before.northward_mps);
    out.valid=std::isfinite(out.eastward_mps) && std::isfinite(out.northward_mps);
    out.quality=out.valid ? WindQuality::Good : WindQuality::MissingGridValue;
    out.detail=out.valid ? "bounded temporal and vertical interpolation passed"
                         : "temporal interpolation produced a non-finite wind vector";
    return out;
}

} // namespace bluesky::weather
