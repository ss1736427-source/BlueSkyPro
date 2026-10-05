from pathlib import Path
import subprocess
import sys


def test_ph4_int_006_evidence_session_chain():
    script = Path(__file__).with_name("ph4_int_006_evidence_session_chain.py")
    result = subprocess.run([sys.executable, str(script)], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
    assert "PH4_INT_006_EVIDENCE_SESSION_CHAIN: PASS" in result.stdout
