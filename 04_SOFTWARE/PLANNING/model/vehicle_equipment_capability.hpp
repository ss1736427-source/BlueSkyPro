#pragma once

#include <string>
#include <vector>

namespace bluesky::planning {

struct FlightCapability {
    double minimum_airspeed{};
    double maximum_airspeed{};
    double cruise_speed{};
    double economic_speed{};
    double climb_limit{};
    double descent_limit{};
    double altitude_min{};
    double altitude_max{};
    double turning_limit{};
    double endurance_seconds{};
    double wind_tolerance{};
    bool navigation_sensors_available{};
    bool gnss_denied_recovery_capable{};
    bool return_recovery_capable{};
    std::vector<std::string> c2_requirements;
    std::vector<std::string> environmental_limits;
};

struct EnergyCapability {
    double usable_energy{};
    double battery_state{};
    double battery_health{};
    double degradation_coefficient{};
    double equipment_mass{};
    double equipment_power{};
    double reserve_requirement{};
    double recovery_requirement{};
};

struct PropulsionResourceCapability {
    std::string engine_motor_type;
    std::string propulsion_configuration;
    std::string propeller_configuration;
    double operating_hours{};
    std::vector<std::string> applicable_limits;
    std::vector<std::string> performance_data_references;
};

struct EquipmentCapability {
    std::string equipment_id;
    std::string equipment_version;
    std::string sensor_camera_type;
    double mass{};
    double balance_impact{};
    double electrical_power{};
    std::string interface_protocol;
    std::vector<std::string> supported_modes;
    double field_of_view{};
    double resolution{};
    double gsd{};
    bool stabilization_capable{};
    bool gimbal_capable{};
    bool triggering_capable{};
    bool control_capable{};
    std::vector<std::string> data_output_requirements;
    std::vector<std::string> operational_restrictions;
};

struct VehicleIdentity {
    std::string vehicle_id;
    std::string vehicle_type_model;
    std::string airframe_configuration;
    std::string propulsion_configuration;
    std::string autopilot_type;
    std::string autopilot_version;
    std::vector<std::string> supported_protocols;
    std::string firmware_software_compatibility;
    std::vector<std::string> equipment_interfaces;
};

struct VehicleConfiguration {
    std::string configuration_version;
    std::string firmware_version;
    std::string battery_type;
    std::string battery_configuration;
    std::string propulsion_configuration;
    std::vector<EquipmentCapability> equipment;
    std::string c2_configuration;
    std::vector<std::string> required_equipment;
    std::vector<std::string> operational_limits;
    std::vector<std::string> recovery_capabilities;
    std::string maintenance_resource_state;
    bool valid_for_mission{};
};

struct VehicleEquipmentCapabilityProfile {
    VehicleIdentity identity;
    FlightCapability flight;
    EnergyCapability energy;
    PropulsionResourceCapability propulsion_resource;
    VehicleConfiguration configuration;
};

} // namespace bluesky::planning
