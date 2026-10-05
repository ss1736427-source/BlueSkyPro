#include "multi_uav_conflict.hpp"
#include <cassert>

using namespace bluesky::planning;

static TrajectoryResult make(const std::string&id,double lat,double delay){
    TrajectoryResult t;
    t.status=TrajectoryStatus::Feasible; t.uav_id=id; t.dependency_identity=id;
    t.points={{"A",{lat,0},100,delay,0},{"B",{lat,0.001},100,delay+10,1}};
    return t;
}
int main(){
    auto a=make("U1",0.0,0.0);
    auto b=make("U2",0.0,0.0);
    std::vector<MultiUavSeparationInput> s={{"U1",20,5,0},{"U2",20,5,0}};
    auto c=MultiUavConflictDetector::evaluate({a,b},s,"C1");
    assert(c.status==MultiUavConflictStatus::Conflict);
    b=make("U2",0.001,0.0);
    c=MultiUavConflictDetector::evaluate({a,b},s,"C1");
    assert(c.status==MultiUavConflictStatus::Clear);
    b=make("U2",0.0,20.0);
    c=MultiUavConflictDetector::evaluate({a,b},s,"C1");
    assert(c.status==MultiUavConflictStatus::Clear);
    b=make("U2",0.0,0.0); b.points[0].altitude_m=110; b.points[1].altitude_m=110;
    c=MultiUavConflictDetector::evaluate({a,b},s,"C1");
    assert(c.status==MultiUavConflictStatus::Clear);
    return 0;
}
