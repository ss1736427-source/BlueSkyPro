import unittest

from evidence_source_adapter_factory import adapter_for_source
from evidence_source_registry import EVIDENCE_SOURCES, all_source_types, sources_for_domain


class TestRegisteredEvidenceAdapters(unittest.TestCase):
    def test_every_registered_source_has_a_working_boundary(self):
        sample = {
            "timestamp_ms": 1000,
            "parameter": "sample_value",
            "value": 1,
            "unit": "unit",
            "source": "prototype-source",
        }
        self.assertEqual(len(EVIDENCE_SOURCES), 21)
        self.assertEqual(len(all_source_types()), len(set(all_source_types())))
        for definition in EVIDENCE_SOURCES:
            adapter = adapter_for_source(definition.source_type)
            event = adapter.normalize(sample)
            metadata = adapter.source_metadata()
            self.assertEqual(event.context["source_id"], definition.source_id)
            self.assertEqual(event.context["evidence_domain_id"], definition.domain_id)
            self.assertEqual(metadata["source_id"], definition.source_id)
            self.assertEqual(metadata["evidence_domain_id"], definition.domain_id)

    def test_domain_can_have_multiple_sources(self):
        sources = sources_for_domain("EC-15")
        self.assertEqual({source.source_type for source in sources}, {"BLUESKY_EVENT", "OPERATOR_EVENT", "HMI"})

    def test_unknown_source_is_rejected(self):
        with self.assertRaises(KeyError):
            adapter_for_source("NOT_REGISTERED")


if __name__ == "__main__":
    unittest.main()
