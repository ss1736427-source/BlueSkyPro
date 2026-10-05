import unittest

from ph4_int_003_adapter_chain_fixture import STATES, execute_fixture


class AdapterChainFixtureTest(unittest.TestCase):
    def test_complete_deterministic_chain(self):
        result, events = execute_fixture()
        self.assertEqual([name for name, _, _ in events], list(STATES))
        self.assertTrue(all(status == "PASS" for _, status, _ in events))
        self.assertEqual(result.lifecycle_state, "EVIDENCE")
        self.assertEqual(result.result, "PASS")
        self.assertEqual(result.reason_code, "OK")
        self.assertEqual(result.source_timestamp, 1000)
        self.assertEqual(result.adapter_timestamp, 1001)


if __name__ == "__main__":
    unittest.main()
