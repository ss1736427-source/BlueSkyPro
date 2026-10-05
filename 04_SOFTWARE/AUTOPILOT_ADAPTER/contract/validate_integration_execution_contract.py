from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[3]
CONTRACT_DIR = ROOT / "04_SOFTWARE/AUTOPILOT_ADAPTER/contract"
API = CONTRACT_DIR / "universal_autopilot_api.yaml"
EXECUTION = CONTRACT_DIR / "BLUESKY_UNIVERSAL_INTEGRATION_EXECUTION_CONTRACT_001.md"

REQUIRED_CHAIN = [
    "Canonical Mission Package",
    "Compatibility Resolution",
    "Mission Compilation",
    "Adapter Dispatch",
    "C2 Transport",
    "Vehicle/Aircraft",
    "State & Mission Read-back",
    "Semantic Comparison",
    "Evidence Record",
]
REQUIRED_STATES = [
    "PREPARE", "RESOLVE", "COMPILE", "VALIDATE", "CONNECT",
    "DISPATCH", "ACKNOWLEDGE", "READ_BACK", "COMPARE", "RELEASE", "EVIDENCE",
]
REQUIRED_RESULTS = [
    "execution_id", "vehicle_id", "mission_id", "adapter_id",
    "lifecycle_state", "result", "reason_code", "source_timestamp",
    "adapter_timestamp", "configuration_id", "evidence_reference",
]

errors = []

if not API.is_file():
    errors.append(f"missing API contract: {API}")
if not EXECUTION.is_file():
    errors.append(f"missing execution contract: {EXECUTION}")

if not errors:
    api = API.read_text(encoding="utf-8")
    execution = EXECUTION.read_text(encoding="utf-8")

    if "api: bluesky.universal_autopilot" not in api:
        errors.append("unexpected universal API identity")
    for state in ["REQUESTED", "VALIDATING", "REJECTED", "DISPATCHED", "ACKNOWLEDGED", "EXECUTING", "COMPLETED", "FAILED", "CANCELLED", "TIMEOUT", "UNKNOWN"]:
        if f"      - {state}" not in api:
            errors.append(f"missing command lifecycle state: {state}")

    chain_line = next((line for line in execution.splitlines() if line.startswith("`Canonical Mission Package")), "")
    for item in REQUIRED_CHAIN:
        if item not in chain_line:
            errors.append(f"execution chain missing: {item}")
    for state in REQUIRED_STATES:
        if f"`{state}`" not in execution:
            errors.append(f"execution lifecycle state missing: {state}")
    for field in REQUIRED_RESULTS:
        if f"`{field}`" not in execution:
            errors.append(f"minimum result field missing: {field}")

    for forbidden in ("ArduPilot", "PX4", "MAVLink", "OEM protocol types"):
        if forbidden in execution and "forbidden" not in execution.lower():
            errors.append(f"protocol coupling check failed: {forbidden}")

if errors:
    print("INTEGRATION_EXECUTION_CONTRACT: FAIL")
    for error in errors:
        print(f"- {error}")
    sys.exit(1)

print("INTEGRATION_EXECUTION_CONTRACT: PASS")
print("execution_chain=9 stages")
print("lifecycle_states=11")
print("verification_status=CONTRACT_ONLY")
