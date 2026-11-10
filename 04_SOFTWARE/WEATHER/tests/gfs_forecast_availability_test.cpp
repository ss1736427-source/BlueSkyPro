#include "gfs_forecast_availability_service.hpp"
#include <cassert>
using bluesky::weather::GfsForecastAvailabilityService;
int main(){
 QString error;const auto url=GfsForecastAvailabilityService::buildAtmosDirectoryUrl("2026-11-06T00:00:00Z",&error);
 assert(url.isValid()&&error.isEmpty());
 assert(url.toString()=="https://nomads.ncep.noaa.gov/pub/data/nccf/com/gfs/prod/gfs.20261106/00/atmos/");
 error.clear();assert(GfsForecastAvailabilityService::buildAtmosDirectoryUrl("2026-11-06T01:00:00Z",&error).isEmpty()&&!error.isEmpty());
 const QByteArray html=R"(<a href="gfs.t00z.pgrb2.0p25.f000">x</a><a href="gfs.t00z.pgrb2.0p25.f000.idx">idx</a><a href="gfs.t00z.pgrb2.0p25.f120">x</a><a href="gfs.t00z.pgrb2.0p25.f123">x</a><a href="gfs.t00z.pgrb2.0p25.f384">x</a><a href="gfs.t00z.pgrb2.0p25.f121">bad cadence</a><a href="gfs.t06z.pgrb2.0p25.f006">other</a>)";
 bool recognized=false;const auto hours=GfsForecastAvailabilityService::parseAvailableForecastHours(html,"00",&recognized);
 assert(recognized&&hours.size()==4&&hours.contains(0)&&hours.contains(120)&&hours.contains(123)&&hours.contains(384));
 assert(!hours.contains(121)&&!hours.contains(6));
 recognized=true;assert(GfsForecastAvailabilityService::parseAvailableForecastHours("<html>none</html>","00",&recognized).isEmpty()&&!recognized);
 const QDateTime now(QDate(2026,11,9),QTime(14,30),Qt::UTC);
 const auto cycles=GfsForecastAvailabilityService::recentCandidateCyclesUtc(now,2,&error);
 assert(error.isEmpty()&&cycles.size()==7);
 assert(cycles[0]=="2026-11-09T12:00:00Z"&&cycles[1]=="2026-11-09T06:00:00Z"&&cycles[2]=="2026-11-09T00:00:00Z");
 assert(cycles[3]=="2026-11-08T18:00:00Z"&&cycles.last()=="2026-11-08T00:00:00Z");
 error.clear();assert(GfsForecastAvailabilityService::recentCandidateCyclesUtc(now,0,&error).isEmpty()&&!error.isEmpty());

 // Availability-listing metadata has its own bounded TTL; GRIB products do not.
 const auto checked=QDateTime(QDate(2026,11,9),QTime(14,0),Qt::UTC);
 assert(GfsForecastAvailabilityService::availabilityCacheTtlSeconds()==600);
 assert(GfsForecastAvailabilityService::isAvailabilityCacheFresh(
     checked,checked.addSecs(599)));
 assert(!GfsForecastAvailabilityService::isAvailabilityCacheFresh(
     checked,checked.addSecs(600)));
 assert(!GfsForecastAvailabilityService::isAvailabilityCacheFresh(
     checked,checked.addSecs(-1)));
 assert(!GfsForecastAvailabilityService::isAvailabilityCacheFresh(
     checked,checked.addSecs(1),0));
 assert(!GfsForecastAvailabilityService::isAvailabilityCacheFresh(
     QDateTime{},checked));
 return 0;
}
