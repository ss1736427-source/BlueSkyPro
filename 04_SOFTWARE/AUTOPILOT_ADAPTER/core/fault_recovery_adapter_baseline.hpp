#pragma once
#include "universal_fault_recovery_adapter.hpp"
namespace bluesky::fault { class FaultRecoveryAdapterBaseline final : public UniversalFaultRecoveryAdapter { public: std::string getSourceId() const override; FaultRecoveryState getState() const override; FaultEvent readEvent() const override; FaultRecoveryResult acceptEvent(const FaultEvent& event) override; private: FaultRecoveryState state_{FaultRecoveryState::Normal}; }; }
