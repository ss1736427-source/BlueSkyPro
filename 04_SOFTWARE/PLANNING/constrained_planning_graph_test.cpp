#include "constrained_planning_graph.hpp"
#include <cassert>
using namespace bluesky::planning;
int main() {
    PlanningGraph source;
    source.start_node="S"; source.goal_node="G";
    source.nodes.push_back({"S",0,0,{59.0,30.0},100});
    source.nodes.push_back({"G",1,1,{59.01,30.01},100});
    source.edges.push_back({"S","G",10});
    ConstrainedEnvironmentSnapshot env;
    env.snapshot_id="ENV-1"; env.snapshot_version="1"; env.calculation_input_version="1";
    SpatialRestriction r;
    r.restriction_id="R-1"; r.source_id="TEST"; r.snapshot_version="1";
    r.polygon={{58.99,29.99},{58.99,30.02},{59.02,30.02},{59.02,29.99}};
    env.restrictions.push_back(r);
    auto blocked=ConstrainedPlanningGraphBuilder::build(source,env);
    assert(blocked.graph.edges.empty()); assert(blocked.rejected_edges==1);
    env.restrictions.clear();
    auto open=ConstrainedPlanningGraphBuilder::build(source,env);
    assert(open.graph.edges.size()==1);
    return 0;
}