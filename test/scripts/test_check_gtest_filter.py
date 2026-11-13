"""Contract checks for the runner's list-only GoogleTest filter preflight."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

HELPER = Path(__file__).resolve().parents[2] / "scripts/check-gtest-filter.sh"


class FilterPreflightTest(unittest.TestCase):
    def test_filter_results_and_listing_errors(self):
        with tempfile.TemporaryDirectory() as root:
            binary = Path(root) / "fake gtest"
            binary.write_text('''#!/usr/bin/env python3
import sys
if len(sys.argv) != 3 or sys.argv[2] != "--gtest_list_tests":
    sys.exit(9)  # The preflight must never execute a test body.
name = sys.argv[1].removeprefix("--gtest_filter=")
outputs = {
    "valid": "Suite.\\n  Works\\n",
    "parameterized": "Instance/Suite/0.  # TypeParam = int\\n  Works/0  # GetParam() = 7\\n",
    "disabled-test": "Suite.\\n  DISABLED_Slow\\n",
    "disabled-suite": "DISABLED_Suite.\\n  Works\\n",
    "empty": "Note: Google Test filter = does-not-exist\\n",
    "mixed": "DISABLED_Suite.\\n  Works\\nSuite.\\n  Works\\n",
}
if name == "listing-error":
    sys.exit(3)
print(outputs[name], end="")
''')
            binary.chmod(0o755)
            env = dict(os.environ)
            for name, expected in [("valid", 0), ("parameterized", 0), ("mixed", 0),
                                   ("disabled-test", 2), ("disabled-suite", 2),
                                   ("empty", 2), ("listing-error", 2)]:
                with self.subTest(filter=name):
                    result = subprocess.run(["bash", str(HELPER), str(binary), name],
                                            text=True, capture_output=True, env=env)
                    self.assertEqual(result.returncode, expected, result.stderr)
                    if expected:
                        self.assertIn(str(binary), result.stderr)


if __name__ == "__main__":
    unittest.main()
