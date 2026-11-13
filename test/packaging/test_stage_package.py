"""Exercise the actual composite-action staging script in isolated directories."""
import os
from pathlib import Path
import subprocess
import tempfile
import textwrap
import unittest

ACTION = Path(__file__).resolve().parents[2] / ".github/actions/build-package/action.yml"
STEP = ACTION.read_text().split("    - name: Stage package\n", 1)[1]
SCRIPT = textwrap.dedent(STEP.split("      run: |\n", 1)[1].split("    # overwrite:true", 1)[0])


class StagePackageTest(unittest.TestCase):
    def stage(self, selected, candidates, success, payload=b"fixture package"):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in candidates:
                path = root / "build" / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(payload if name == selected else b"stale package")
            result = subprocess.run(
                ["/bin/bash", "-c", SCRIPT], cwd=root,
                env={**os.environ, "RUNNER_OS": "Linux",
                     "PACKAGE_PATH": str(root / "build" / selected) if selected else ""},
                text=True, capture_output=True,
            )
            staged = list((root / "artifact-upload").iterdir())
            self.assertEqual(result.returncode == 0, success, result.stdout + result.stderr)
            self.assertEqual(len(staged), 1 if success else 0)
            if success:
                self.assertEqual(staged[0].name, Path(selected).name)
                self.assertEqual(staged[0].read_bytes(), payload)
            return result.stdout

    def test_no_reported_path(self):
        self.assertIn("did not report", self.stage(None, ["old/UltraScan3-stale.exe"], False))

    def test_one_each_format(self):
        for extension in ("tar.xz", "tar.gz", "pkg", "exe"):
            with self.subTest(extension=extension):
                name = f"preset with spaces/UltraScan3-test.{extension}"
                self.stage(name, [name], True)

    def test_stale_outputs_do_not_affect_selection(self):
        for name in ("z/UltraScan3-new.tar.xz", "z/UltraScan3-HPC-new.tar.xz"):
            with self.subTest(name=name):
                self.stage(name, ["a/UltraScan3-old.pkg", "b/UltraScan3-HPC-old.tar.xz", name], True)

    def test_missing_output_does_not_fall_back(self):
        self.stage("new/UltraScan3-new.exe", ["old/UltraScan3-stale.exe"], False)

    def test_empty_output_fails(self):
        name = "new/UltraScan3-new.exe"
        self.stage(name, [name], False, payload=b"")


if __name__ == "__main__":
    unittest.main()
