#include "gfs_forecast_availability_service.hpp"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <utility>
namespace bluesky::weather {
namespace {
const QString kRoot=QStringLiteral("https://nomads.ncep.noaa.gov/pub/data/nccf/com/gfs/prod/");
constexpr int kMaxListingBytes=4*1024*1024;
void setErrorText(QString *out,const QString &s){if(out)*out=s;}
bool validCycle(const QString &s,QDateTime *out=nullptr){
 if(s.size()!=20||!s.endsWith(QLatin1Char('Z')))return false;
 const auto dt=QDateTime::fromString(s,Qt::ISODate);
 if(!dt.isValid()||dt.toUTC().toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'"))!=s)return false;
 const int h=dt.time().hour();
 if((h!=0&&h!=6&&h!=12&&h!=18)||dt.time().minute()!=0||dt.time().second()!=0)return false;
 if(out)*out=dt.toUTC(); return true;
}
QVariantList variants(const std::vector<int>&v){QVariantList r;for(int x:v)r.push_back(x);return r;}
QVariantList variants(const QList<int>&v){QVariantList r;for(int x:v)r.push_back(x);return r;}
}
GfsForecastAvailabilityService::GfsForecastAvailabilityService(QObject *p):QObject(p),m_network(this){}
QString GfsForecastAvailabilityService::state()const{return m_state;}
QString GfsForecastAvailabilityService::lastError()const{return m_lastError;}
QUrl GfsForecastAvailabilityService::buildAtmosDirectoryUrl(const QString&s,QString*error){
 setErrorText(error, QString());
 QDateTime dt;if(!validCycle(s,&dt)){setErrorText(error,QStringLiteral("Cycle must be a valid UTC 00Z/06Z/12Z/18Z timestamp."));return{};}
 return QUrl(kRoot+QStringLiteral("gfs.%1/%2/atmos/").arg(dt.date().toString("yyyyMMdd"),dt.time().toString("hh")));
}
QSet<int> GfsForecastAvailabilityService::parseAvailableForecastHours(const QByteArray&html,const QString&cycle,bool*recognized){
 if(recognized)*recognized=false;
 if(cycle!="00"&&cycle!="06"&&cycle!="12"&&cycle!="18")return{};
 const QString pattern=QString::fromLatin1(R"(href=["']gfs\.t%1z\.pgrb2\.0p25\.f([0-9]{3})["'])").arg(cycle);
 const auto matches=QRegularExpression(pattern,QRegularExpression::CaseInsensitiveOption).globalMatch(QString::fromUtf8(html));
 QSet<int> result;int count=0;
 auto it=matches;while(it.hasNext()){const auto m=it.next();bool ok=false;const int h=m.captured(1).toInt(&ok);if(!ok||h>384||(h>120&&h%3))continue;result.insert(h);++count;}
 if(recognized)*recognized=count>0;return result;
}
QStringList GfsForecastAvailabilityService::recentCandidateCyclesUtc(const QDateTime&nowUtc,int days,QString*error){
 setErrorText(error, QString());
 if(!nowUtc.isValid()||days<1||days>14){setErrorText(error,QStringLiteral("Valid UTC time and lookbackDays 1..14 are required."));return{};}
 const auto now=nowUtc.toUTC();QStringList out;
 for(int d=0;d<days;++d){const QDate date=now.date().addDays(-d);for(int h:{18,12,6,0}){const QDateTime t(date,QTime(h,0),Qt::UTC);if(t<=now)out<<t.toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'"));}}
 return out;
}
bool GfsForecastAvailabilityService::checkMissionForecast(const QString&start,const QString&end,int days){
 if(m_reply){setError(QStringLiteral("A GFS availability check is already running."));return false;}
 GfsForecastPlanRequest validation;validation.mission_start_utc=start.toStdString();validation.mission_end_utc=end.toStdString();
 const auto valid=GfsForecastPlanner::build(validation);
 if(valid.status==GfsForecastPlanStatus::InvalidRequest){setError(QString::fromStdString(valid.reason));setState("INVALID_REQUEST");emit checkFailed(m_lastError,{});return false;}
 QString err;const auto cycles=recentCandidateCyclesUtc(QDateTime::currentDateTimeUtc(),days,&err);
 if(!err.isEmpty()){setError(err);setState("INVALID_REQUEST");emit checkFailed(err,{});return false;}
 m_candidates.clear();m_candidateDiagnostics.clear();m_candidateIndex=0;m_missionStartUtc=start;m_missionEndUtc=end;
 for(const auto&cycle:cycles){
  GfsForecastPlanRequest req;req.mission_start_utc=start.toStdString();req.mission_end_utc=end.toStdString();req.candidates.push_back({cycle.toStdString(),true});
  auto plan=GfsForecastPlanner::build(req);
  if(plan.ready())m_candidates.push_back({cycle,std::move(plan)});
  else m_candidateDiagnostics.push_back(QVariantMap{{"cycle_time_utc",cycle},{"status","DOES_NOT_BRACKET_WINDOW"}});
 }
 if(m_candidates.isEmpty()){const QString why="No recent GFS cycle can bracket the complete mission window within F000-F384.";setError(why);setState("NO_COVERING_CYCLE");emit checkFailed(why,diagnostics());return false;}
 setError({});setState("CHECKING_NOMADS");checkNextCandidate();return true;
}
void GfsForecastAvailabilityService::checkNextCandidate(){
 if(m_reply)return;
 if(m_candidateIndex>=m_candidates.size()){const QString why="No single published GFS run contains every forecast file required by the mission window.";setError(why);setState("NO_COMPLETE_RUN");emit checkFailed(why,diagnostics());return;}
 const auto&c=m_candidates.at(m_candidateIndex);QString err;const QUrl url=buildAtmosDirectoryUrl(c.cycleTimeUtc,&err);
 if(!url.isValid()||url.isEmpty()){failCheck(err.isEmpty()?"Unable to build NOMADS directory URL.":err);return;}
 QNetworkRequest req(url);req.setHeader(QNetworkRequest::UserAgentHeader,"BlueSkyPRO-GFSAvailability/0.1");req.setRawHeader("Accept","text/html");
 req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);req.setTransferTimeout(20000);
 m_reply=m_network.get(req);
 connect(m_reply,&QNetworkReply::downloadProgress,this,[this,cycle=c.cycleTimeUtc](qint64 n,qint64){if(n>kMaxListingBytes)failCheck(QStringLiteral("NOMADS directory listing exceeded 4 MiB for %1.").arg(cycle));});
 connect(m_reply,&QNetworkReply::finished,this,&GfsForecastAvailabilityService::finishCurrentCandidate);
 emit progressChanged(m_candidateIndex,m_candidates.size(),c.cycleTimeUtc);
}
void GfsForecastAvailabilityService::finishCurrentCandidate(){
 if(!m_reply)return;
 QNetworkReply *reply=m_reply.data();const int status=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
 const auto neterr=reply->error();const QString nettext=reply->errorString();const QByteArray body=reply->readAll();
 const QString cycle=m_candidates.at(m_candidateIndex).cycleTimeUtc;reply->deleteLater();m_reply=nullptr;
 if(status==404){m_candidateDiagnostics.push_back(QVariantMap{{"cycle_time_utc",cycle},{"status","NOT_PUBLISHED"},{"http_status",status}});++m_candidateIndex;checkNextCandidate();return;}
 // Unknown provider/network failures stop the scan; they are not treated as missing runs.
 if(neterr!=QNetworkReply::NoError||status!=200){failCheck(QStringLiteral("NOMADS availability is indeterminate for %1 (HTTP %2): %3").arg(cycle).arg(status).arg(nettext));return;}
 if(body.isEmpty()||body.size()>kMaxListingBytes){failCheck(QStringLiteral("NOMADS returned an empty or oversized directory listing for %1.").arg(cycle));return;}
 bool recognized=false;const auto available=parseAvailableForecastHours(body,cycle.mid(11,2),&recognized);
 if(!recognized){failCheck(QStringLiteral("NOMADS response for %1 did not contain recognized GFS 0.25-degree files.").arg(cycle));return;}
 const auto&candidate=m_candidates.at(m_candidateIndex);QList<int> missing;
 for(int h:candidate.plan.forecast_lead_hours)if(!available.contains(h))missing.push_back(h);
 if(missing.isEmpty()){
  m_candidateDiagnostics.push_back(QVariantMap{{"cycle_time_utc",cycle},{"status","COMPLETE"},{"required_file_count",static_cast<int>(candidate.plan.forecast_lead_hours.size())}});
  QVariantMap result{{"status","READY"},{"selected_cycle_utc",cycle},{"forecast_lead_hours",variants(candidate.plan.forecast_lead_hours)},
   {"mission_start_utc",m_missionStartUtc},{"mission_end_utc",m_missionEndUtc},{"source_directory_url",buildAtmosDirectoryUrl(cycle).toString()},
   {"availability_checked_at_utc",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
   {"verification_level","NOMADS_DIRECTORY_LISTING_FILE_PRESENCE"},{"download_validation","PENDING_GRIB2_DOWNLOAD_AND_DECODE"},
   {"candidate_diagnostics",diagnostics()}};
  setError({});setState("READY");emit planReady(result);return;
 }
 m_candidateDiagnostics.push_back(QVariantMap{{"cycle_time_utc",cycle},{"status","INCOMPLETE"},
  {"required_file_count",static_cast<int>(candidate.plan.forecast_lead_hours.size())},{"missing_forecast_lead_hours",variants(missing)}});
 ++m_candidateIndex;checkNextCandidate();
}
void GfsForecastAvailabilityService::cancel(){
 if(!m_reply)return;QPointer<QNetworkReply>r=m_reply;m_reply=nullptr;if(r){r->disconnect(this);r->abort();r->deleteLater();}
 setError("GFS availability check cancelled.");setState("CANCELLED");emit checkFailed(m_lastError,diagnostics());
}
void GfsForecastAvailabilityService::failCheck(const QString&why){
 if(m_reply){QPointer<QNetworkReply>r=m_reply;m_reply=nullptr;if(r){r->disconnect(this);r->abort();r->deleteLater();}}
 setError(why);setState("ERROR");emit checkFailed(why,diagnostics());
}
QVariantList GfsForecastAvailabilityService::diagnostics()const{return m_candidateDiagnostics;}
void GfsForecastAvailabilityService::setState(const QString&s){if(m_state==s)return;m_state=s;emit stateChanged();}
void GfsForecastAvailabilityService::setError(const QString&s){if(m_lastError==s)return;m_lastError=s;emit lastErrorChanged();}
} // namespace bluesky::weather
