#include "authorization_qualified_environment.hpp"
#include <functional>
#include <string>

namespace bluesky::planning {
namespace {
std::string dependency(const AuthorizationQualifiedEnvironment& r) {
    std::hash<std::string> h;
    std::string s = r.source_environment.snapshot_id + "|" +
                    r.source_environment.snapshot_version + "|" +
                    r.calculation_input_version + "|" +
                    r.qualification_version;
    for (const auto& x : r.restrictions) {
        s += "|" + x.restriction.restriction_id + "|" +
             std::to_string(static_cast<int>(x.authorization_status)) + "|" +
             x.authorization_id + "|" + x.authorization_version;
    }
    return std::to_string(h(s));
}
bool qualifies(const AuthorizationCoverage& c) {
    return c.present && c.valid && c.scope_matches &&
           c.time_matches && c.altitude_matches && c.operation_matches;
}
}
AuthorizationQualifiedEnvironment
AuthorizationQualifiedEnvironmentBuilder::build(
    const ConstrainedEnvironmentSnapshot& environment,
    const std::vector<AuthorizationCoverage>& coverage,
    const std::string& qualification_version) {
    AuthorizationQualifiedEnvironment result;
    result.source_environment = environment;
    result.qualification_version = qualification_version;
    result.calculation_input_version = environment.calculation_input_version;

    for (const auto& restriction : environment.restrictions) {
        AuthorizationQualifiedRestriction qualified;
        qualified.restriction = restriction;

        const AuthorizationCoverage* match = nullptr;
        for (const auto& candidate : coverage) {
            if (candidate.restriction_id == restriction.restriction_id) {
                match = &candidate;
                break;
            }
        }

        if (!restriction.active) {
            qualified.authorization_status = AuthorizationQualificationStatus::Unqualified;
        } else if (match == nullptr) {
            qualified.authorization_status = AuthorizationQualificationStatus::Unqualified;
            result.unresolved_restriction_ids.push_back(restriction.restriction_id);
        } else if (qualifies(*match)) {
            qualified.authorization_status = AuthorizationQualificationStatus::Qualified;
            qualified.authorization_id = match->authorization_id;
            qualified.authorization_version = match->authorization_version;
            qualified.restriction.active = false;
        } else {
            qualified.authorization_status = AuthorizationQualificationStatus::Invalid;
            qualified.authorization_id = match->authorization_id;
            qualified.authorization_version = match->authorization_version;
        }
        result.restrictions.push_back(qualified);
    }

    result.dependency_identity = dependency(result);
    return result;
}
} // namespace bluesky::planning
