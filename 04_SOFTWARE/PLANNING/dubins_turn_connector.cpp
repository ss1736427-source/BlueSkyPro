#include "dubins_turn_connector.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace bluesky::planning {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;
struct Candidate { TurnPathFamily family; char a; char b; char c; double x; double y; double z; double length; };

double mod2pi(double v) {
    v = std::fmod(v, kTwoPi);
    return v < 0.0 ? v + kTwoPi : v;
}
bool finite(double v) { return std::isfinite(v); }
double cross(const TurnPose2D& a, const TurnPose2D& b, const TurnPose2D& p) {
    return (b.x_m-a.x_m)*(p.y_m-a.y_m)-(b.y_m-a.y_m)*(p.x_m-a.x_m);
}
bool onSegment(const TurnPose2D& a,const TurnPose2D& b,const TurnPose2D& p) {
    constexpr double e=1e-8;
    return std::abs(cross(a,b,p))<=e &&
        p.x_m>=std::min(a.x_m,b.x_m)-e && p.x_m<=std::max(a.x_m,b.x_m)+e &&
        p.y_m>=std::min(a.y_m,b.y_m)-e && p.y_m<=std::max(a.y_m,b.y_m)+e;
}
bool intersects(const TurnPose2D& a,const TurnPose2D& b,const TurnPose2D& c,const TurnPose2D& d) {
    const double ab1=cross(a,b,c),ab2=cross(a,b,d),cd1=cross(c,d,a),cd2=cross(c,d,b);
    if (((ab1>0&&ab2<0)||(ab1<0&&ab2>0)) && ((cd1>0&&cd2<0)||(cd1<0&&cd2>0))) return true;
    return onSegment(a,b,c)||onSegment(a,b,d)||onSegment(c,d,a)||onSegment(c,d,b);
}
bool inside(const std::vector<TurnPose2D>& ring,const TurnPose2D& p) {
    bool result=false;
    for(std::size_t i=0,j=ring.size()-1;i<ring.size();j=i++) {
        const auto& a=ring[i]; const auto& b=ring[j];
        if(onSegment(a,b,p)) return true;
        if(((a.y_m>p.y_m)!=(b.y_m>p.y_m)) &&
           p.x_m<(b.x_m-a.x_m)*(p.y_m-a.y_m)/(b.y_m-a.y_m)+a.x_m) result=!result;
    }
    return result;
}
bool blocked(const TurnPose2D& a,const TurnPose2D& b,const std::vector<TurnPolygon2D>& areas) {
    for(const auto& area:areas) {
        const auto& ring=area.vertices;
        if(inside(ring,a)||inside(ring,b)) return true;
        for(std::size_t i=0;i<ring.size();++i)
            if(intersects(a,b,ring[i],ring[(i+1)%ring.size()])) return true;
    }
    return false;
}
bool solveFour(double alpha,double beta,double d,std::vector<Candidate>& out) {
    double p2,tmp,t,p,q;
    // Left-Straight-Left
    tmp=d+std::sin(alpha)-std::sin(beta);
    p2=2+d*d-2*std::cos(alpha-beta)+2*d*(std::sin(alpha)-std::sin(beta));
    if(p2>=0) { p=std::sqrt(p2); double v=std::atan2(std::cos(beta)-std::cos(alpha),tmp);
        t=mod2pi(-alpha+v); q=mod2pi(beta-v);
        out.push_back({TurnPathFamily::LSL,'L','S','L',t,p,q,t+p+q}); }
    // Right-Straight-Right
    tmp=d-std::sin(alpha)+std::sin(beta);
    p2=2+d*d-2*std::cos(alpha-beta)+2*d*(-std::sin(alpha)+std::sin(beta));
    if(p2>=0) { p=std::sqrt(p2); double v=std::atan2(std::cos(alpha)-std::cos(beta),tmp);
        t=mod2pi(alpha-v); q=mod2pi(-beta+v);
        out.push_back({TurnPathFamily::RSR,'R','S','R',t,p,q,t+p+q}); }
    // Left-Straight-Right
    p2=-2+d*d+2*std::cos(alpha-beta)+2*d*(std::sin(alpha)+std::sin(beta));
    if(p2>=0) { p=std::sqrt(p2); double v=std::atan2(-std::cos(alpha)-std::cos(beta),d+std::sin(alpha)+std::sin(beta))-std::atan2(-2.0,p);
        t=mod2pi(-alpha+v); q=mod2pi(-beta+v);
        out.push_back({TurnPathFamily::LSR,'L','S','R',t,p,q,t+p+q}); }
    // Right-Straight-Left
    p2=d*d-2+2*std::cos(alpha-beta)-2*d*(std::sin(alpha)+std::sin(beta));
    if(p2>=0) { p=std::sqrt(p2); double v=std::atan2(std::cos(alpha)+std::cos(beta),d-std::sin(alpha)-std::sin(beta))-std::atan2(2.0,p);
        t=mod2pi(alpha-v); q=mod2pi(beta-v);
        out.push_back({TurnPathFamily::RSL,'R','S','L',t,p,q,t+p+q}); }
    return !out.empty();
}
TurnPose2D advance(TurnPose2D pose,char mode,double distance,double radius) {
    if(mode=='S') {
        pose.x_m+=distance*std::cos(pose.heading_rad);
        pose.y_m+=distance*std::sin(pose.heading_rad);
    } else {
        const double sign=mode=='L'?1.0:-1.0;
        const double delta=sign*distance/radius;
        pose.x_m+=radius/sign*(std::sin(pose.heading_rad+delta)-std::sin(pose.heading_rad));
        pose.y_m+=-radius/sign*(std::cos(pose.heading_rad+delta)-std::cos(pose.heading_rad));
        pose.heading_rad=mod2pi(pose.heading_rad+delta);
    }
    return pose;
}
}

TurnConnectorResult DubinsTurnConnector::build(const TurnConnectorInput& in) {
    TurnConnectorResult result;
    result.minimum_turn_radius_m=in.minimum_turn_radius_m;
    const auto validPose=[](const TurnPose2D& p){return finite(p.x_m)&&finite(p.y_m)&&finite(p.heading_rad);};
    if(!validPose(in.start)||!validPose(in.finish)||!finite(in.minimum_turn_radius_m)||
       in.minimum_turn_radius_m<=0||!finite(in.maximum_sample_spacing_m)||
       in.maximum_sample_spacing_m<=0) {
        result.diagnostic="TURN_INPUT_INVALID: finite poses, positive turn radius and positive sample spacing are required.";
        return result;
    }
    for(const auto& area:in.prohibited_areas) {
        if(area.vertices.size()<3) {
            result.diagnostic="TURN_EXCLUSION_GEOMETRY_INVALID: every prohibited polygon requires at least three vertices.";
            return result;
        }
        for(const auto& p:area.vertices) if(!validPose(p)) {
            result.diagnostic="TURN_EXCLUSION_GEOMETRY_INVALID: polygon coordinates must be finite.";
            return result;
        }
    }

    const double dx=in.finish.x_m-in.start.x_m,dy=in.finish.y_m-in.start.y_m;
    const double distance=std::hypot(dx,dy),radius=in.minimum_turn_radius_m;
    const double theta=distance>1e-9?std::atan2(dy,dx):in.start.heading_rad;
    const double alpha=mod2pi(in.start.heading_rad-theta);
    const double beta=mod2pi(in.finish.heading_rad-theta);
    std::vector<Candidate> candidates;
    solveFour(alpha,beta,distance/radius,candidates);
    if(candidates.empty()) {
        result.diagnostic="TURN_PATH_UNAVAILABLE: no feasible LSL/RSR/LSR/RSL Dubins solution; CCC families are not implemented.";
        return result;
    }
    std::sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){return a.length<b.length;});

    for(const auto& candidate:candidates) {
        TurnPose2D pose=in.start;
        std::vector<TurnPose2D> points{pose};
        bool collision=false;
        const char modes[3]={candidate.a,candidate.b,candidate.c};
        const double lengths[3]={candidate.x*radius,candidate.y*radius,candidate.z*radius};
        for(int part=0;part<3&&!collision;++part) {
            const auto steps=std::max<std::size_t>(1,static_cast<std::size_t>(std::ceil(lengths[part]/in.maximum_sample_spacing_m)));
            const double step=lengths[part]/static_cast<double>(steps);
            for(std::size_t i=0;i<steps;++i) {
                const auto next=advance(pose,modes[part],step,radius);
                if(blocked(pose,next,in.prohibited_areas)) { collision=true; break; }
                points.push_back(next);
                pose=next;
            }
        }
        if(collision) continue;
        const double endpointError=std::hypot(pose.x_m-in.finish.x_m,pose.y_m-in.finish.y_m);
        const double headingError=std::abs(std::remainder(pose.heading_rad-in.finish.heading_rad,kTwoPi));
        if(endpointError>std::max(0.05,in.maximum_sample_spacing_m*0.02)||headingError>1e-5) continue;
        // Snap only the final floating-point residue to the exact requested pose.
        points.back()=in.finish;
        result.feasible=true;
        result.family=candidate.family;
        result.path_length_m=candidate.length*radius;
        result.points=std::move(points);
        result.diagnostic="TURN_CONNECTOR_FEASIBLE: validated geometric Dubins connector; vehicle dynamics evidence, wind, terrain, obstacles, NOTAM and complete-route validation remain separate hard gates.";
        return result;
    }
    result.diagnostic="TURN_CONNECTOR_BLOCKED: all supported candidate paths intersect prohibited geometry or fail endpoint closure.";
    return result;
}

} // namespace bluesky::planning
