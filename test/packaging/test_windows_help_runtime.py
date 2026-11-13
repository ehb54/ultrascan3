"""Run the real Windows stage script with fixture files and a no-op Qt deployer.

No Windows binaries are executed; the fixture suppresses unrelated downloads.
"""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
DEPLOY = ROOT / "admin/cmake/packaging/windows/WinDeploy.cmake"


class WindowsHelpRuntimeTest(unittest.TestCase):
    def deploy(self, runtime):
        with tempfile.TemporaryDirectory(prefix="us3 help ") as directory:
            root = Path(directory)
            for name in ("build/us.exe", "build/manual.qch", "build/manual.qhc",
                         "tools/assistant.exe", "plugins/platforms/qwindows.dll",
                         "plugins/sqldrivers/qsqlite.dll"):
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"fixture")
            (root / "runtime").mkdir()
            if runtime is not None:
                (root / "runtime/sqlite3.dll").write_bytes(runtime)
            deployer = root / "windeployqt"
            deployer.write_text("#!/bin/sh\nexit 0\n")
            deployer.chmod(0o755)
            manifest = root / "programs.cmake"
            manifest.write_text(
                f'set(US3_APP_PROGRAMS "{root}/build/us.exe")\n'
                f'set(US3_APP_EXECUTABLES "{root}/build/us.exe")\n')
            wrapper = root / "deploy.cmake"
            wrapper.write_text("""set(WIN32 TRUE)
# Block unrelated SoMo downloads without altering production deployment code.
macro(file)
    if(ARGV0 STREQUAL "DOWNLOAD")
        set(${ARGV4} "1;fixture download disabled")
    else()
        _file(${ARGV})
    endif()
endmacro()
""" + f'include("{DEPLOY}")\n')
            result = subprocess.run([
                "cmake", f"-DSTAGE_DIR={root}/stage", f"-DBIN_DIR={root}/build",
                f"-DQCH_DIR={root}/build", f"-DASSISTANT_EXE={root}/tools/assistant.exe",
                f"-DWINDEPLOYQT={deployer}", f"-DVCPKG_PLUGIN_DIR={root}/plugins",
                f"-DVCPKG_BIN_DIR={root}/runtime", f"-DAPP_PROGRAM_MANIFEST={manifest}",
                "-P", str(wrapper),
            ], text=True, capture_output=True)
            if runtime:
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertEqual((root / "stage/bin/sqlite3.dll").read_bytes(), runtime)
            else:
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("sqlite3.dll", result.stderr)
                self.assertIn("missing" if runtime is None else "empty", result.stderr)

    def test_runtime_is_staged(self):
        self.deploy(b"sqlite runtime fixture")

    def test_missing_runtime_fails(self):
        self.deploy(None)

    def test_empty_runtime_fails(self):
        self.deploy(b"")


if __name__ == "__main__":
    unittest.main()
