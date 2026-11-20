"""Run the real manual Makefile and makeall.sh with stub tools.

Checks that failed Sphinx and qmake runs fail the build, and that the
manual's qhelpgenerator comes from the selected qmake's Qt.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
MANUAL_MAKEFILE = ROOT / "doc/manual/Makefile"
MAKEALL = ROOT / "makeall.sh"


def write_script(path, body):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("#!/bin/sh\n" + body)
    path.chmod(0o755)
    return path


def stub_generator(path, name, log):
    # Records which generator ran and writes the requested output file.
    return write_script(path, f"""echo {name} >> "{log}"
while [ $# -gt 0 ]; do
  if [ "$1" = -o ]; then echo {name} > "$2"; fi
  shift
done
""")


def stub_qmake(path, libexecs, bins):
    return write_script(path, f"""case "$2" in
  QT_HOST_LIBEXECS|QT_INSTALL_LIBEXECS) echo "{libexecs}" ;;
  QT_HOST_BINS|QT_INSTALL_BINS) echo "{bins}" ;;
esac
""")


class ManualMakefileTest(unittest.TestCase):
    def setUp(self):
        # Make variables cannot hold paths with spaces.
        self.tmp = tempfile.TemporaryDirectory(prefix="us3-manual-")
        self.root = Path(self.tmp.name)
        self.manual = self.root / "manual"
        self.manual.mkdir()
        shutil.copy(MANUAL_MAKEFILE, self.manual / "Makefile")
        (self.manual / "source").mkdir()
        self.install = self.root / "bin"
        self.log = self.root / "generators.log"
        # Writes the files the real Sphinx qthelp builder produces.
        self.sphinx = write_script(self.root / "sphinx", """[ "$1" = --version ] && exit 0
out="$4"
mkdir -p "$out"
touch "$out/manual.qhp" "$out/manual.qhcp"
""")
        self.path_bin = self.root / "path-bin"
        stub_generator(self.path_bin / "qhelpgenerator", "path", self.log)

    def tearDown(self):
        self.tmp.cleanup()

    def make(self, *args):
        env = dict(os.environ, PATH=f"{self.path_bin}:/usr/bin:/bin")
        env.pop("QHELPGEN", None)
        return subprocess.run(
            ["make", "-C", str(self.manual), f"INSTALL_DIR={self.install}", *args],
            env=env, capture_output=True, text=True)

    def generators(self):
        return self.log.read_text().split() if self.log.exists() else []

    def test_failed_sphinx_fails_qthelp(self):
        result = self.make("SPHINXBUILD=false", "qthelp")
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertNotIn("written to", result.stdout)

    def test_failed_sphinx_does_not_package_stale_help(self):
        stale = self.manual / "build/qthelp"
        stale.mkdir(parents=True)
        for name in ("manual.qhp", "manual.qhcp", "manual.qch", "manual.qhc"):
            (stale / name).write_text("stale")
        result = self.make("SPHINXBUILD=false", "QMAKE=false", "qch")
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertEqual(self.generators(), [])
        self.assertFalse(self.install.exists())

    def test_selected_qmake_generator_preferred_over_path(self):
        qt = self.root / "qt6"
        stub_generator(qt / "libexec/qhelpgenerator", "qmake", self.log)
        qmake = stub_qmake(self.root / "qmake", qt / "libexec", qt / "bin")
        result = self.make(f"SPHINXBUILD={self.sphinx}", f"QMAKE={qmake}", "qch")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.generators(), ["qmake", "qmake"])
        self.assertEqual((self.install / "manual.qhc").read_text().strip(), "qmake")

    def test_selected_qmake_without_generator_does_not_use_path(self):
        qt = self.root / "qt6"
        (qt / "bin").mkdir(parents=True)
        qmake = stub_qmake(self.root / "qmake", qt / "libexec", qt / "bin")
        result = self.make(f"SPHINXBUILD={self.sphinx}", f"QMAKE={qmake}", "qch-tools")
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertIn("qhelpgenerator was not found", result.stdout)

    def test_explicit_generator_overrides_qmake(self):
        qt = self.root / "qt6"
        stub_generator(qt / "libexec/qhelpgenerator", "qmake", self.log)
        qmake = stub_qmake(self.root / "qmake", qt / "libexec", qt / "bin")
        explicit = stub_generator(self.root / "explicit/qhelpgenerator", "explicit", self.log)
        result = self.make(f"SPHINXBUILD={self.sphinx}", f"QMAKE={qmake}",
                           f"QHELPGEN={explicit}", "qch")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.generators(), ["explicit", "explicit"])

    def test_path_generator_used_without_usable_qmake(self):
        result = self.make(f"SPHINXBUILD={self.sphinx}", "QMAKE=false", "qch")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.generators(), ["path", "path"])


class MakeallTest(unittest.TestCase):
    def setUp(self):
        # makeall.sh does not quote paths with spaces.
        self.tmp = tempfile.TemporaryDirectory(prefix="us3-makeall-")
        self.root = Path(self.tmp.name)
        self.tree = self.root / "tree"
        for d in ("qwtplot3d", "utils", "gui", "programs/us", "doc/manual"):
            (self.tree / d).mkdir(parents=True)
            (self.tree / d / "project.pro").write_text("")
        shutil.copy(MAKEALL, self.tree / "makeall.sh")
        for name in ("libnames.sh", "appnames.sh", "gui/fix-mac-make.sh"):
            write_script(self.tree / name, "exit 0\n")
        self.bin = self.root / "bin"
        write_script(self.bin / "make", "exit 0\n")
        # Satisfies the Sphinx probe without installing anything.
        write_script(self.bin / "python3", "exit 0\n")
        write_script(self.bin / "doxygen", "exit 0\n")

    def tearDown(self):
        self.tmp.cleanup()

    def qmake(self, version_output, project_status=0):
        write_script(self.bin / "qmake", f"""if [ "$1" = --version ]; then
  printf "{version_output}"
  exit 0
fi
exit {project_status}
""")

    def run_makeall(self):
        env = dict(os.environ, PATH=f"{self.bin}:/usr/bin:/bin")
        return subprocess.run(["bash", "makeall.sh", "-j1"], cwd=self.tree,
                              env=env, capture_output=True, text=True)

    def test_succeeds_when_qmake_and_make_succeed(self):
        self.qmake("QMake version 3.1\\nUsing Qt version 6.9.3 in /qt/lib\\n")
        result = self.run_makeall()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_rejects_missing_qt_version(self):
        self.qmake("")
        result = self.run_makeall()
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertIn("Wrong qmake", result.stdout)

    def test_rejects_unsupported_qt_major_version(self):
        for version in ("4.8.7", "56.0.0"):
            with self.subTest(version=version):
                self.qmake(f"QMake version 3.1\\nUsing Qt version {version} in /qt/lib\\n")
                result = self.run_makeall()
                self.assertNotEqual(result.returncode, 0, result.stdout)
                self.assertIn("Wrong qmake", result.stdout)

    def test_qmake_failure_fails_build_even_when_make_succeeds(self):
        self.qmake("QMake version 3.1\\nUsing Qt version 5.15.3 in /qt/lib\\n", project_status=3)
        result = self.run_makeall()
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertIn("qmake failed in utils", result.stdout)
        self.assertIn("*** [qmake] Error in utils", (self.tree / "build.log").read_text())

    def manual_make(self, tools_status):
        # doc/manual's qch-tools target exits with tools_status; building the
        # manual itself fails, so an attempt without the tools is an error.
        self.make_log = self.root / "make.log"
        write_script(self.bin / "make", f"""echo "$(basename "$PWD") $*" >> "{self.make_log}"
case "$PWD" in
  */doc/manual)
    case " $* " in
      *" qch-tools "*) exit {tools_status} ;;
      *) exit 2 ;;
    esac ;;
esac
exit 0
""")

    def test_manual_skipped_when_help_tools_missing(self):
        self.qmake("QMake version 3.1\\nUsing Qt version 6.9.3 in /qt/lib\\n")
        self.manual_make(tools_status=2)
        result = self.run_makeall()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("the help manual will not be built", result.stdout)
        manual_calls = [line for line in self.make_log.read_text().splitlines()
                        if line.startswith("manual ")]
        self.assertEqual(manual_calls, ["manual -s qch-tools"])

    def test_manual_built_when_help_tools_present(self):
        self.qmake("QMake version 3.1\\nUsing Qt version 6.9.3 in /qt/lib\\n")
        self.manual_make(tools_status=0)
        result = self.run_makeall()
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertIn("***ERROR*** building doc/manual", result.stdout)
        self.assertIn("manual -j1", self.make_log.read_text())


if __name__ == "__main__":
    unittest.main()
