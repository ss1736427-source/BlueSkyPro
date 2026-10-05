# EC-14 P0 evidence status

The common evidence validator now accepts the canonical `EvidenceCapture` event envelope (`timestamp_utc`, `source_id`, `evidence_domain_id`) while retaining compatibility with the normalized package event fields. `manifest.sha256` is now verified against `manifest.json`.

This closes the P0 schema/integrity mismatch between the canonical source event recorder and the evidence-package validator. EC-14 remains PARTIAL overall until controlled execution, complete package evidence, and physical/operational verification are demonstrated.
