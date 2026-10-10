#include "authorization_qualified_environment.hpp"
#include <cassert>

using namespace bluesky::planning;

int main() {
    ConstrainedEnvironmentSnapshot environment;
    environment.snapshot_id = "ENV-1";
    environment.snapshot_version = "42";
    environment.calculation_input_version = "calc-42";

    SpatialRestriction restriction;
    restriction.restriction_id = "CITY-1";
    restriction.source_id = "AIRSPACE";
    restriction.snapshot_version = "A-7";
    environment.restrictions.push_back(restriction);

    auto missing = AuthorizationQualifiedEnvironmentBuilder::build(
        environment, {}, "AUTH-QUAL-1");
    assert(missing.unresolved_restriction_ids.size() == 1);
    assert(missing.restrictions.front().authorization_status ==
           AuthorizationQualificationStatus::Unqualified);
    assert(missing.restrictions.front().restriction.active);

    AuthorizationCoverage valid;
    valid.authorization_id = "AUTH-77";
    valid.authorization_version = "3";
    valid.restriction_id = "CITY-1";
    valid.present = true;
    valid.valid = true;
    valid.scope_matches = true;
    valid.time_matches = true;
    valid.altitude_matches = true;
    valid.operation_matches = true;

    auto qualified = AuthorizationQualifiedEnvironmentBuilder::build(
        environment, {valid}, "AUTH-QUAL-1");
    assert(qualified.unresolved_restriction_ids.empty());
    assert(qualified.restrictions.front().authorization_status ==
           AuthorizationQualificationStatus::Qualified);
    assert(!qualified.restrictions.front().restriction.active);

    valid.scope_matches = false;
    auto out_of_scope = AuthorizationQualifiedEnvironmentBuilder::build(
        environment, {valid}, "AUTH-QUAL-1");
    assert(out_of_scope.restrictions.front().authorization_status ==
           AuthorizationQualificationStatus::Invalid);
    assert(out_of_scope.restrictions.front().restriction.active);

    auto repeat = AuthorizationQualifiedEnvironmentBuilder::build(
        environment, {valid}, "AUTH-QUAL-1");
    assert(out_of_scope.dependency_identity == repeat.dependency_identity);
    return 0;
}
