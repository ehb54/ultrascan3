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
    def stage(self, profile, candidates, success):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "build").mkdir()
            for name in candidates:
                path = root / "build" / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"fixture package")
            result = subprocess.run(
                ["/bin/bash", "-c", SCRIPT], cwd=root,
                env={**os.environ, "PROFILE": profile},
                text=True, capture_output=True,
            )
            staged = list((root / "artifact-upload").iterdir())
            self.assertEqual(result.returncode == 0, success, result.stdout + result.stderr)
            self.assertEqual(len(staged), 1 if success else 0)
            if success:
                self.assertEqual(staged[0].read_bytes(), b"fixture package")
            return result.stdout

    def test_zero(self):
        self.assertIn("no package produced", self.stage("APP", [], False))

    def test_one_each_format(self):
        for extension in ("tar.xz", "tar.gz", "pkg", "exe"):
            with self.subTest(extension=extension):
                self.stage("APP", [f"preset with spaces/UltraScan3-test.{extension}"], True)

    def test_multiple(self):
        names = ["a/UltraScan3-old.pkg", "b/UltraScan3-new.pkg"]
        output = self.stage("APP", names, False)
        self.assertIn("expected exactly one package for profile APP, found 2", output)
        for name in names:
            self.assertIn(name, output)

    def test_profile_filter(self):
        names = ["app/UltraScan3-test.tar.xz", "hpc/UltraScan3-HPC-test.tar.xz"]
        for profile in ("APP", "HPC"):
            with self.subTest(profile=profile):
                self.stage(profile, names, True)
        self.stage("HPC", names[:1], False)
        self.stage("APP", names[1:], False)
        self.stage("HPC", ["a/UltraScan3-HPC-old.tar.xz", "b/UltraScan3-HPC-new.tar.xz"], False)


if __name__ == "__main__":
    unittest.main()
