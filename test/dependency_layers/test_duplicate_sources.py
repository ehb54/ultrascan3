#!/usr/bin/env python3
"""Negative tests for scripts/check-duplicate-sources.py.

Each test writes a small tree in which every source is inside its directory
except for one, and checks that the script rejects it.

usage: test_duplicate_sources.py <path to check-duplicate-sources.py>
"""
import os
import subprocess
import sys
import tempfile
import unittest

CHECKER = None

# Two programs that share an object library, and sources in programs/ itself.
VALID = {
    "programs/CMakeLists.txt": "add_library(us_shared_objects OBJECT us_shared.cpp ${CMAKE_CURRENT_SOURCE_DIR}/us_more.cpp)\n",
    "programs/us_alpha/CMakeLists.txt": (
        "set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})\n"
        "add_library(us_alpha_objects OBJECT ${src_dir}/us_alpha.cpp parts/us_part.cpp ./us_dot.cpp)\n"
        "add_executable(us_alpha us_alpha_main.cpp ${CMAKE_CURRENT_BINARY_DIR}/us_generated.cpp)\n"
        "target_link_libraries(us_alpha PRIVATE us_alpha_objects)\n"
    ),
    "programs/us_beta/CMakeLists.txt": (
        'file(GLOB BETA_SOURCES "*.cpp")\n'
        "add_executable(us_beta ${BETA_SOURCES})\n"
        "target_link_libraries(us_beta PRIVATE us_alpha_objects us_shared_objects)\n"
    ),
    "programs/us_gamma/us_gamma.cpp": "",
}


def run_checker(files):
    """Write the tree and run the checker; return (exit code, output)."""
    with tempfile.TemporaryDirectory() as root:
        for path, text in files.items():
            full = os.path.join(root, path)
            os.makedirs(os.path.dirname(full), exist_ok=True)
            with open(full, "w", encoding="utf-8") as handle:
                handle.write(text)
        result = subprocess.run(
            [sys.executable, CHECKER, "--root", root], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True
        )
        return result.returncode, result.stdout


class DuplicateSourcesTest(unittest.TestCase):
    def check(self, path, addition):
        files = dict(VALID)
        files[path] += addition
        return run_checker(files)

    def assertRejected(self, path, addition, message):
        code, output = self.check(path, addition)
        self.assertEqual(code, 1, output)
        self.assertIn(message, output)
        self.assertIn("1 source(s)", output)

    def test_valid_tree_passes(self):
        code, output = run_checker(VALID)
        self.assertEqual(code, 0, output)

    def test_comments_are_ignored(self):
        code, output = self.check("programs/us_beta/CMakeLists.txt", "# ../us_alpha/us_alpha.cpp\n#[[\n../us_alpha/us_alpha.cpp\n]]\n")
        self.assertEqual(code, 0, output)

    def test_shared_headers_are_allowed(self):
        code, output = self.check("programs/us_beta/CMakeLists.txt", "target_sources(us_beta PRIVATE ../us_alpha/us_alpha.h)\n")
        self.assertEqual(code, 0, output)

    def test_another_programs_source_is_rejected(self):
        self.assertRejected(
            "programs/us_beta/CMakeLists.txt",
            "set(CROSS\n    ../us_alpha/us_alpha.cpp\n    )\n",
            "programs/us_beta/CMakeLists.txt:5: lists ../us_alpha/us_alpha.cpp",
        )

    def test_a_source_in_programs_itself_is_rejected(self):
        self.assertRejected("programs/us_beta/CMakeLists.txt", "target_sources(us_beta PRIVATE ../us_shared.cpp)\n", "lists ../us_shared.cpp")

    def test_paths_through_variables_are_rejected(self):
        for prefix in (
            "${CMAKE_CURRENT_SOURCE_DIR}/../us_alpha",
            "${CMAKE_CURRENT_LIST_DIR}/../us_alpha",
            "${CMAKE_SOURCE_DIR}/programs/us_alpha",
            "${PROJECT_SOURCE_DIR}/programs/us_alpha",
            "/src/programs/us_alpha",
            "parts/../../us_alpha",
        ):
            path = prefix + "/us_alpha.cpp"
            with self.subTest(path=path):
                self.assertRejected("programs/us_beta/CMakeLists.txt", "target_sources(us_beta PRIVATE %s)\n" % path, "lists " + path)
        self.assertRejected(
            "programs/us_beta/CMakeLists.txt",
            "set(other ${CMAKE_CURRENT_SOURCE_DIR}/../us_alpha)\ntarget_sources(us_beta PRIVATE ${other}/us_alpha.cpp)\n",
            "lists ${other}/us_alpha.cpp",
        )

    def test_a_glob_of_another_directory_is_rejected(self):
        self.assertRejected("programs/us_beta/CMakeLists.txt", 'file(GLOB OTHER "../us_alpha/*.cpp")\n', "lists ../us_alpha/*.cpp")

    def test_a_programs_source_in_the_top_list_is_rejected(self):
        for path in ("us_alpha/us_alpha.cpp", "${CMAKE_CURRENT_SOURCE_DIR}/us_alpha/us_alpha.cpp", "../gui/us_widget.cpp"):
            with self.subTest(path=path):
                self.assertRejected("programs/CMakeLists.txt", "add_library(us_other OBJECT %s)\n" % path, "lists " + path)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    CHECKER = os.path.abspath(sys.argv.pop(1))
    unittest.main(verbosity=2)
