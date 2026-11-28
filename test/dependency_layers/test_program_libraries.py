#!/usr/bin/env python3
"""Negative tests for scripts/check-program-libraries.py.

Each test writes a small tree that is valid except for one violation and checks
that the script rejects it.

usage: test_program_libraries.py <path to check-program-libraries.py>
"""
import os
import subprocess
import sys
import tempfile
import unittest

CHECKER = None

# us_alpha and a source in programs/ are in area low; us_beta is in area high,
# which needs low; us_gamma only links high.
VALID = {
    "programs/CMakeLists.txt": (
        "function(us3_add_program_area name)\nendfunction()\n"
        "us3_add_program_area(us_programs_low)\n"
        "us3_add_program_area(us_programs_high us_programs_low)\n"
        "add_library(us_shared_lib STATIC us_shared.cpp us_shared.h)\n"
        "target_link_libraries(us_programs_low INTERFACE us_shared_lib)\n"
    ),
    "programs/us_alpha/CMakeLists.txt": (
        "set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})\n"
        "add_library(us_alpha_lib STATIC\n    ${src_dir}/us_alpha.cpp ./us_dot.cpp\n)\n"
        "target_link_libraries(us_programs_low INTERFACE us_alpha_lib)\n"
        "add_executable(us_alpha us_alpha_main.cpp ${CMAKE_CURRENT_BINARY_DIR}/us_generated.cpp)\n"
        "target_link_libraries(us_alpha PRIVATE us_programs_low)\n"
    ),
    "programs/us_alpha/us_alpha.pro": "PROGRAM_LIBS = low\nSOURCES = us_alpha_main.cpp\n",
    "programs/us_alpha/us_alpha.h": "",
    "programs/us_beta/CMakeLists.txt": (
        "set(BETA_SOURCES\n    us_beta.cpp\n    )\n"
        "add_library(us_beta_lib STATIC ${BETA_SOURCES})\n"
        "target_link_libraries(us_programs_high INTERFACE us_beta_lib)\n"
        "target_link_libraries(us_beta_lib PUBLIC us_alpha_lib)\n"
        "add_executable(us_beta us_beta_main.cpp)\n"
        "target_link_libraries(us_beta PRIVATE us_programs_high)\n"
    ),
    "programs/us_beta/us_beta.pro": "PROGRAM_LIBS = high\nSOURCES = us_beta_main.cpp\n",
    "programs/us_beta/us_beta.h": '#include "../us_alpha/us_alpha.h"\n',
    "programs/us_gamma/CMakeLists.txt": (
        'file(GLOB GAMMA_SOURCES "*.cpp")\n'
        "add_executable(us_gamma ${GAMMA_SOURCES})\n"
        "target_link_libraries(us_gamma PRIVATE us_programs_high)\n"
    ),
    "programs/us_gamma/us_gamma.pro": "PROGRAM_LIBS = high\nSOURCES = us_gamma.cpp\n",
    "programs/us_gamma/us_gamma.cpp": '#include "../us_beta/us_beta.h"\n#include "../us_alpha/us_alpha.h"\n',
    "programs/us_delta/CMakeLists.txt": "add_executable(us_delta us_delta.cpp)\n",
    "programs/us_delta/us_delta.cpp": "",
    "programlibs/link.pri": "contains( PROGRAM_LIBS, high ) : PROGRAM_LIBS += low\n",
    "programlibs/low/us_programs_low.pro": (
        "SOURCES = ../../programs/us_alpha/us_alpha.cpp \\\n"
        "          ../../programs/us_alpha/us_dot.cpp   \\\n"
        "          ../../programs/us_shared.cpp\n"
        "HEADERS = ../../programs/us_shared.h\n"
    ),
    "programlibs/high/us_programs_high.pro": "SOURCES = ../../programs/us_beta/us_beta.cpp\n",
}


def run_checker(files, *args):
    """Write the tree and run the checker; return (exit code, output)."""
    with tempfile.TemporaryDirectory() as root:
        for path, text in files.items():
            full = os.path.join(root, path)
            os.makedirs(os.path.dirname(full), exist_ok=True)
            with open(full, "w", encoding="utf-8") as handle:
                handle.write(text)
        result = subprocess.run(
            [sys.executable, CHECKER, "--root", root, *args], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True
        )
        return result.returncode, result.stdout


class ProgramLibrariesTest(unittest.TestCase):
    def run_with(self, path, addition="", replace=None):
        files = dict(VALID)
        if replace:
            self.assertIn(replace[0], files[path])
            files[path] = files[path].replace(*replace)
        files[path] += addition
        return run_checker(files)

    def assertRejected(self, message, path, addition="", replace=None):
        code, output = self.run_with(path, addition, replace)
        self.assertEqual(code, 1, output)
        self.assertIn(message, output)
        self.assertIn("1 program library violation(s)", output)

    def test_valid_tree_passes(self):
        code, output = run_checker(VALID)
        self.assertEqual(code, 0, output)

    def test_affected_programs_include_transitive_dependents(self):
        code, output = run_checker(VALID, "--affected-programs", "programs/us_alpha")
        self.assertEqual(code, 0, output)
        self.assertEqual(output.splitlines(), ["programs/us_alpha", "programs/us_beta", "programs/us_gamma"])

    def test_affected_programs_from_root_level_source_or_header(self):
        dependents = ["programs/us_alpha", "programs/us_beta", "programs/us_gamma"]
        for shared in ("programs/us_shared.cpp", "programs/us_shared.h"):
            with self.subTest(shared=shared):
                code, output = run_checker(VALID, "--affected-programs", shared)
                self.assertEqual(code, 0, output)
                self.assertEqual(output.splitlines(), dependents)

                code, output = run_checker(VALID, "--affected-programs", shared, "programs/us_delta/us_delta.cpp")
                self.assertEqual(code, 0, output)
                self.assertEqual(output.splitlines(), sorted(dependents + ["programs/us_delta"]))

    def test_affected_programs_do_not_include_dependencies(self):
        code, output = run_checker(VALID, "--affected-programs", "programs/us_beta")
        self.assertEqual(code, 0, output)
        self.assertEqual(output.splitlines(), ["programs/us_beta", "programs/us_gamma"])

    def test_affected_programs_follow_new_area_dependencies(self):
        files = dict(VALID)
        files["programlibs/top/us_programs_top.pro"] = "SOURCES = ../../programs/us_delta/us_delta.cpp\n"
        files["programlibs/link.pri"] += "contains( PROGRAM_LIBS, top ) : PROGRAM_LIBS += high\n"
        files["programs/us_delta/us_delta.pro"] = "PROGRAM_LIBS = top\n"
        code, output = run_checker(files, "--affected-programs", "programs/us_alpha")
        self.assertEqual(code, 0, output)
        self.assertEqual(output.splitlines(), ["programs/us_alpha", "programs/us_beta", "programs/us_delta", "programs/us_gamma"])

    def test_changed_program_without_area_is_still_built(self):
        code, output = run_checker(VALID, "--affected-programs", "programs/us_delta")
        self.assertEqual(code, 0, output)
        self.assertEqual(output.splitlines(), ["programs/us_delta"])

    def test_comments_are_ignored(self):
        code, output = self.run_with("programs/us_beta/CMakeLists.txt", "# ../us_alpha/us_alpha.cpp\n#[[\n../us_alpha/us_alpha.cpp\n]]\n")
        self.assertEqual(code, 0, output)
        code, output = self.run_with("programs/us_beta/us_beta.pro", "# SOURCES += ../us_alpha/us_alpha.cpp\n")
        self.assertEqual(code, 0, output)
        code, output = self.run_with("programs/us_delta/us_delta.cpp", '// #include "../us_beta/us_beta.h"\n/* #include "../us_beta/us_beta.h" */\n')
        self.assertEqual(code, 0, output)

    def test_another_programs_source_in_cmake_is_rejected(self):
        self.assertRejected(
            "programs/us_delta/CMakeLists.txt:3: lists ../us_alpha/us_alpha.cpp",
            "programs/us_delta/CMakeLists.txt",
            "set(CROSS\n    ../us_alpha/us_alpha.cpp\n    )\n",
        )

    def test_a_source_in_programs_itself_is_rejected(self):
        self.assertRejected(
            "lists ../us_shared.cpp", "programs/us_delta/CMakeLists.txt", "target_sources(us_delta PRIVATE ../us_shared.cpp)\n"
        )

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
                self.assertRejected("lists " + path, "programs/us_delta/CMakeLists.txt", "target_sources(us_delta PRIVATE %s)\n" % path)
        self.assertRejected(
            "lists ${other}/us_alpha.cpp",
            "programs/us_delta/CMakeLists.txt",
            "set(other ${CMAKE_CURRENT_SOURCE_DIR}/../us_alpha)\ntarget_sources(us_delta PRIVATE ${other}/us_alpha.cpp)\n",
        )

    def test_a_glob_of_another_directory_is_rejected(self):
        self.assertRejected("lists ../us_alpha/*.cpp", "programs/us_delta/CMakeLists.txt", 'file(GLOB OTHER "../us_alpha/*.cpp")\n')

    def test_a_programs_source_in_the_top_list_is_rejected(self):
        for path in ("us_alpha/us_alpha.cpp", "${CMAKE_CURRENT_SOURCE_DIR}/us_alpha/us_alpha.cpp", "../gui/us_widget.cpp"):
            with self.subTest(path=path):
                self.assertRejected("lists " + path, "programs/CMakeLists.txt", "add_library(us_other OBJECT %s)\n" % path)

    def test_another_programs_source_in_qmake_is_rejected(self):
        for path in ("../us_alpha/us_alpha.cpp", "$$PWD/../us_alpha/us_alpha.cpp"):
            with self.subTest(path=path):
                self.assertRejected(
                    "programs/us_gamma/us_gamma.pro: lists " + path, "programs/us_gamma/us_gamma.pro", "SOURCES += %s\n" % path
                )

    def test_a_source_missing_from_qmake_is_rejected(self):
        self.assertRejected(
            "programs/us_beta/us_new.cpp is in the CMake library but not in the qmake one",
            "programs/us_beta/CMakeLists.txt",
            replace=("us_beta.cpp\n", "us_beta.cpp\n    us_new.cpp\n"),
        )

    def test_a_source_missing_from_cmake_is_rejected(self):
        self.assertRejected(
            "programs/us_beta/us_new.cpp is in the qmake library but not in the CMake one",
            "programlibs/high/us_programs_high.pro",
            "SOURCES += ../../programs/us_beta/us_new.cpp\n",
        )

    def test_different_needs_are_rejected(self):
        code, output = self.run_with("programlibs/link.pri", replace=("+= low", "+="))
        self.assertEqual(code, 1, output)
        self.assertIn("us_programs_high: needs ['low'] in CMake but [] in qmake", output)

    def test_different_program_links_are_rejected(self):
        self.assertRejected(
            "programs/us_gamma: links ['high'] in CMake but ['low'] in qmake",
            "programs/us_gamma/us_gamma.pro",
            replace=("PROGRAM_LIBS = high", "PROGRAM_LIBS = low"),
        )

    def test_an_include_of_an_unlinked_area_is_rejected(self):
        self.assertRejected(
            "programs/us_delta/us_delta.cpp: includes a header of us_beta, but does not link us_programs_high",
            "programs/us_delta/us_delta.cpp",
            '#include "../us_beta/us_beta.h"\n',
        )

    def test_an_upward_cmake_member_link_is_rejected(self):
        self.assertRejected(
            "us_alpha_lib: links us_beta_lib, but low does not depend on high",
            "programs/us_alpha/CMakeLists.txt",
            "target_link_libraries(us_alpha_lib PUBLIC us_beta_lib)\n",
        )

    def test_an_upward_include_between_areas_is_rejected(self):
        self.assertRejected(
            "programs/us_alpha/us_alpha.h: includes a header of us_beta, but does not link us_programs_high",
            "programs/us_alpha/us_alpha.h",
            '#include "../us_beta/us_beta.h"\n',
        )


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    CHECKER = os.path.abspath(sys.argv.pop(1))
    unittest.main(verbosity=2)
