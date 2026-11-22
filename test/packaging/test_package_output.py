"""Test package target -> receipt -> Bash/PowerShell -> Actions handoff."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
MODULE = ROOT / "admin/cmake/packaging"


class PackageOutputTest(unittest.TestCase):
    def command(self, args, success=True, **kwargs):
        result = subprocess.run(args, text=True, capture_output=True, **kwargs)
        self.assertEqual(result.returncode == 0, success, result.stdout + result.stderr)
        return result

    def test_target_receipt_and_reporters(self):
        for extension in ("tar.xz", "tar.gz", "pkg", "exe"):
            with self.subTest(extension=extension), tempfile.TemporaryDirectory(prefix="package output ") as d:
                root = Path(d)
                package = root / f"build/UltraScan3-test.{extension}"
                (root / "payload").write_text("produced by this target")
                (root / "CMakeLists.txt").write_text(f'''
cmake_minimum_required(VERSION 3.21)
project(PackageOutput NONE)
include("{MODULE}/PackageOutput.cmake")
add_custom_target(package_fixture
 COMMAND ${{CMAKE_COMMAND}} -E copy "${{CMAKE_SOURCE_DIR}}/payload" "{package}"
 VERBATIM)
us3_record_package_output(package_fixture "{package}")
''')
                self.command(["cmake", "-S", str(root), "-B", str(root / "build")])
                self.command(["cmake", "--build", str(root / "build"), "--target", "package_fixture"])
                receipt = root / "build/package-path.txt"
                self.assertEqual(receipt.read_text(), str(package) + "\n")
                reporters = [["/bin/bash", str(ROOT / "scripts/report-package.sh")]]
                if shutil.which("pwsh"):
                    reporters.append(["pwsh", "-NoProfile", "-File", str(ROOT / "scripts/report-package.ps1")])
                for index, reporter in enumerate(reporters):
                    output = root / f"github-output-{index}"
                    self.command(reporter + [str(receipt)], env={**os.environ, "GITHUB_OUTPUT": str(output)})
                    self.assertEqual(output.read_text(), f"package_path={package}\n")
                # Failure must clear the old success receipt and publish nothing new.
                (root / "payload").write_bytes(b"")
                self.command(["cmake", "--build", str(root / "build"), "--target", "package_fixture"], success=False)
                self.assertFalse(receipt.exists())

    def test_reporters_reject_invalid_receipts(self):
        reporters = [["/bin/bash", str(ROOT / "scripts/report-package.sh")]]
        if shutil.which("pwsh"):
            reporters.append(["pwsh", "-NoProfile", "-File", str(ROOT / "scripts/report-package.ps1")])
        for reporter in reporters:
            with self.subTest(reporter=reporter), tempfile.TemporaryDirectory() as d:
                root = Path(d)
                empty = root / "empty.exe"
                empty.touch()
                receipt = root / "receipt.txt"
                for content in (None, "", str(root / "missing.exe"), str(empty), str(root),
                                str(empty) + "\npackage_path=other"):
                    with self.subTest(content=content):
                        if content is not None:
                            receipt.write_text(content + "\n")
                        output = root / "github-output"
                        self.command(reporter + [str(receipt)], success=False,
                                     env={**os.environ, "GITHUB_OUTPUT": str(output)})
                        self.assertFalse(output.exists())

    def test_real_archive_targets_emit_receipts(self):
        cases = [("linux", "LinuxPkg.cmake", "deploy_linux", "package_linux_tarball", "_stage/UltraScan3"),
                 ("linux", "LinuxHpcPkg.cmake", "deploy_linux_hpc", "package_linux_hpc_tarball", "_hpc_stage/UltraScan3-HPC"),
                 ("macos", "MacHpcPkg.cmake", "deploy_macos_hpc", "package_macos_hpc_tarball", "_hpc_stage/UltraScan3-HPC")]
        for platform, module, deploy, target, stage in cases:
            with self.subTest(target=target), tempfile.TemporaryDirectory() as d:
                root = Path(d)
                (root / "CMakeLists.txt").write_text(f'''
cmake_minimum_required(VERSION 3.21)
project(ArchiveOutput NONE)
set(APPLE {"TRUE" if platform == "macos" else "FALSE"})
set(US3_VERSION_STRING fixture)
set(US3_ARCH_LABEL x64)
file(MAKE_DIRECTORY "${{CMAKE_BINARY_DIR}}/{stage}")
file(WRITE "${{CMAKE_BINARY_DIR}}/{stage}/fixture" "payload")
add_custom_target({deploy})
include("{MODULE}/{platform}/{module}")
''')
                build = root / "build"
                self.command(["cmake", "-S", str(root), "-B", str(build)])
                self.command(["cmake", "--build", str(build), "--target", target])
                path = Path((build / "package-path.txt").read_text().strip())
                self.assertTrue(path.is_file())
                self.assertGreater(path.stat().st_size, 0)


if __name__ == "__main__":
    unittest.main()
