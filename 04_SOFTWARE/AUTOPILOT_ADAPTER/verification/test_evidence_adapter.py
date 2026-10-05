import unittest

from evidence_adapter import (
    AutopilotTelemetrySourceAdapter,
    BlueSkyEventSourceAdapter,
    C2LinkSourceAdapter,
    EquipmentSourceAdapter,
    GnssRtkNtripSourceAdapter,
    MeasurementInstrumentSourceAdapter,
    OperatorEventSourceAdapter,
    WeatherSourceAdapter,
)


class TestSourceAdapters(unittest.TestCase):
    def setUp(self):
        self.record = {
            "timestamp_ms": 1234,
            "parameter": "link_latency_ms",
            "value": 42.5,
            "unit": "ms",
            "source": "demo-source",
            "quality": "valid",
            "context": {"test_run_id": "TEST-001", "flight_record_id": "FLIGHT-001"},
        }

    def test_c2_adapter_preserves_source_fact(self):
        event = C2LinkSourceAdapter().normalize(self.record)
        self.assertEqual(event.as_dict(), self.record)
        self.assertEqual(C2LinkSourceAdapter().source_metadata()["source_type"], "C2_LINK")

    def test_all_source_categories_have_stable_boundaries(self):
        adapters = [
            (C2LinkSourceAdapter(), "C2_LINK"),
            (AutopilotTelemetrySourceAdapter(), "AUTOPILOT_TELEMETRY"),
            (EquipmentSourceAdapter(), "EQUIPMENT"),
            (GnssRtkNtripSourceAdapter(), "GNSS_RTK_NTRIP"),
            (WeatherSourceAdapter(), "WEATHER"),
            (MeasurementInstrumentSourceAdapter(), "MEASUREMENT_INSTRUMENT"),
            (BlueSkyEventSourceAdapter(), "BLUESKY_EVENT"),
            (OperatorEventSourceAdapter(), "OPERATOR_EVENT"),
        ]
        for adapter, expected_type in adapters:
            self.assertEqual(adapter.source_type, expected_type)
            self.assertEqual(adapter.normalize(self.record).value, 42.5)

    def test_missing_source_field_is_rejected(self):
        invalid = dict(self.record)
        del invalid["unit"]
        with self.assertRaises(ValueError):
            C2LinkSourceAdapter().normalize(invalid)


if __name__ == "__main__":
    unittest.main()
