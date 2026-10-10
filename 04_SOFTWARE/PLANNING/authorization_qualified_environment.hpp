#pragma once
#include "constrained_open_space.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

enum class AuthorizationQualificationStatus {
    Unqualified,
    Qualified,
    Invalid
};

struct AuthorizationCoverage {
    std::string authorization_id;
    std::string authorization_version;
    std::string restriction_id;
    bool present{false};
    bool valid{false};
    bool scope_matches{false};
    bool time_matches{false};
    bool altitude_matches{false};
    bool operation_matches{false};
};

struct AuthorizationQualifiedRestriction {
    SpatialRestriction restriction;
    AuthorizationQualificationStatus authorization_status{
        AuthorizationQualificationStatus::Unqualified};
    std::string authorization_id;
    std::string authorization_version;
};

struct AuthorizationQualifiedEnvironment {
    ConstrainedEnvironmentSnapshot source_environment;
    std::string qualification_version;
    std::string calculation_input_version;
    std::vector<AuthorizationQualifiedRestriction> restrictions;
    std::vector<std::string> unresolved_restriction_ids;
    std::string dependency_identity;
};

class AuthorizationQualifiedEnvironmentBuilder {
public:
    static AuthorizationQualifiedEnvironment build(
        const ConstrainedEnvironmentSnapshot& environment,
        const std::vector<AuthorizationCoverage>& coverage,
        const std::string& qualification_version);
};

} // namespace bluesky::planning
