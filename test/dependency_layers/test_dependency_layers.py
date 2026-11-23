#!/usr/bin/env python3
"""Negative tests for scripts/check-dependency-layers.py.

Each test writes a small tree that is valid except for one forbidden
dependency and checks that the script rejects it, so a passing run of the
script on the repository means the rule is enforced, not merely unbroken.

usage: test_dependency_layers.py <path to check-dependency-layers.py>
"""
import os
import subprocess
import sys
import tempfile
import unittest

CHECKER = None

# A tree with every layer and no forbidden dependency.
VALID = {
    "library.pri": "INCLUDEPATH += ../qwt/src\n",
    "utils/CMakeLists.txt": "add_library(us_utils us_base.cpp)\ntarget_link_libraries(us_utils PUBLIC Qt::Core)\n",
    "utils/libus_utils.pro": "include ( ../library.pri )\nSOURCES = us_base.cpp\n",
    "utils/us_base.h": "#ifndef US_BASE_H\n#define US_BASE_H\n#endif\n",
    "utils/us_base.cpp": '#include "us_base.h"\n#include <QString>\n',
    "gui/CMakeLists.txt": "add_library(us_gui us_widget.cpp)\ntarget_link_libraries(us_gui PUBLIC us_utils)\n",
    "gui/libus_gui.pro": "include ( ../library.pri )\nLIBS += -lus_utils\nSOURCES = us_widget.cpp\n",
    "gui/us_widget.h": '#ifndef US_WIDGET_H\n#define US_WIDGET_H\n#include "us_base.h"\n#endif\n',
    "gui/us_widget.cpp": '#include "us_widget.h"\n',
    "programs/us_shared.h": "#ifndef US_SHARED_H\n#define US_SHARED_H\n#endif\n",
    "programs/us_alpha/CMakeLists.txt": "add_executable(us_alpha us_alpha.cpp ../us_beta/us_beta_part.cpp)\n",
    "programs/us_alpha/us_alpha.h": '#include "us_widget.h"\n',
    "programs/us_alpha/us_alpha.cpp": '#include "us_alpha.h"\n#include "../us_beta/us_beta_part.h"\n#include "us_shared.h"\n',
    "programs/us_beta/us_beta.pro": "SOURCES = us_beta.cpp\n",
    "programs/us_beta/us_beta.cpp": '#include "us_beta_part.h"\n#include "us_shared.h"\n',
    "programs/us_beta/us_beta_part.h": "#ifndef US_BETA_PART_H\n#define US_BETA_PART_H\n#endif\n",
    "programs/us_beta/us_beta_part.cpp": '#include "us_beta_part.h"\n',
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


class DependencyLayersTest(unittest.TestCase):
    def assertRejected(self, files, message):
        code, output = run_checker(files)
        self.assertEqual(code, 1, output)
        self.assertIn(message, output)

    def test_valid_tree_passes(self):
        code, output = run_checker(VALID)
        self.assertEqual(code, 0, output)

    def test_commented_out_dependencies_are_ignored(self):
        files = dict(VALID)
        files["gui/us_widget.cpp"] = (
            '#include "us_widget.h"\n'
            '// #include "../programs/us_alpha/us_alpha.h"\n'
            '/* #include "us_alpha.h"\n'
            '   #include "us_beta_part.h" */\n'
        )
        files["programs/us_beta/us_beta.cpp"] += '// #include "../us_alpha/us_alpha.h"\n'
        code, output = run_checker(files)
        self.assertEqual(code, 0, output)

    def test_gui_including_a_programs_path_is_rejected(self):
        files = dict(VALID)
        files["gui/us_widget.cpp"] += '#include "programs/us_alpha/us_alpha.h"\n'
        self.assertRejected(files, "gui includes programs/us_alpha/us_alpha.h: a path into programs/")

    def test_dot_dot_include_staying_inside_the_library_passes(self):
        files = dict(VALID)
        files["gui/moc/moc_us_widget.cpp"] = '#include "../us_widget.h"\n'
        files["utils/moc/moc_us_base.cpp"] = '#include "../moc/../us_base.h"\n'
        code, output = run_checker(files)
        self.assertEqual(code, 0, output)

    def test_nested_library_file_climbing_out_is_rejected(self):
        files = dict(VALID)
        files["utils/moc/moc_us_base.cpp"] = '#include "../../gui/us_widget.h"\n'
        self.assertRejected(files, 'utils/moc/moc_us_base.cpp:1: utils includes ../../gui/us_widget.h: a path that climbs out')
        files = dict(VALID)
        files["gui/moc/moc_us_widget.cpp"] = '#include "../../programs/us_alpha/us_alpha.h"\n'
        self.assertRejected(files, "a path into programs/")

    def test_library_climbing_out_with_dot_dot_is_rejected(self):
        files = dict(VALID)
        files["utils/us_base.cpp"] += '#include "../programs/us_alpha/us_alpha.h"\n'
        self.assertRejected(files, "utils includes ../programs/us_alpha/us_alpha.h")
        files = dict(VALID)
        files["gui/us_widget.cpp"] += '#include "../somewhere/us_other.h"\n'
        self.assertRejected(files, 'a path that climbs out of the library with "../"')

    def test_library_including_a_program_only_header_is_rejected(self):
        files = dict(VALID)
        files["gui/us_widget.h"] = files["gui/us_widget.h"].replace(
            '#include "us_base.h"\n', '#include "us_base.h"\n#include "us_alpha.h"\n'
        )
        self.assertRejected(files, "gui/us_widget.h:4: gui includes us_alpha.h: a header that exists only under programs/")

    def test_utils_including_a_gui_header_is_rejected(self):
        files = dict(VALID)
        files["utils/us_base.cpp"] += '#include "us_widget.h"\n'
        self.assertRejected(files, "utils includes us_widget.h: a header that exists only in gui/")

    def test_library_build_naming_programs_is_rejected(self):
        files = dict(VALID)
        files["gui/CMakeLists.txt"] += "target_sources(us_gui PRIVATE ../programs/us_alpha/us_alpha.cpp)\n"
        self.assertRejected(files, "gui/CMakeLists.txt:3: the gui build names programs/")
        files = dict(VALID)
        files["library.pri"] += "INCLUDEPATH += ../programs\n"
        self.assertRejected(files, "library.pri:2: the utils build names programs/")

    def test_utils_build_linking_gui_is_rejected(self):
        files = dict(VALID)
        files["utils/libus_utils.pro"] += "LIBS += -lus_gui\n"
        self.assertRejected(files, "utils/libus_utils.pro:3: the utils build links the GUI library")
        files = dict(VALID)
        files["utils/CMakeLists.txt"] += "target_link_libraries(us_utils PRIVATE UltraScan3::Gui)\n"
        self.assertRejected(files, "the utils build links the GUI library")
        for link in ("target_link_libraries(us_utils PRIVATE us_gui_static)\n", "LIBS += -lus_gui_static\n"):
            files = dict(VALID)
            files["utils/CMakeLists.txt"] += link
            self.assertRejected(files, "the utils build links the GUI library")

    def test_program_include_cycle_is_rejected(self):
        files = dict(VALID)
        files["programs/us_beta/us_beta.cpp"] += '#include "../us_alpha/us_alpha.h"\n'
        self.assertRejected(files, "depend on each other in a cycle: us_alpha, us_beta")

    def test_program_cycle_through_a_bare_include_is_rejected(self):
        files = dict(VALID)
        files["programs/us_beta/us_beta.cpp"] += '#include "us_alpha.h"\n'
        self.assertRejected(files, "depend on each other in a cycle: us_alpha, us_beta")

    def test_program_cycle_through_a_source_list_is_rejected(self):
        files = dict(VALID)
        files["programs/us_beta/us_beta.pro"] += "SOURCES += ../us_alpha/us_alpha.cpp\n"
        self.assertRejected(files, "depend on each other in a cycle: us_alpha, us_beta")

    def test_three_program_cycle_is_rejected(self):
        files = dict(VALID)
        files["programs/us_gamma/us_gamma.h"] = '#include "../us_alpha/us_alpha.h"\n'
        files["programs/us_beta/us_beta.cpp"] += '#include "../us_gamma/us_gamma.h"\n'
        self.assertRejected(files, "depend on each other in a cycle: us_alpha, us_beta, us_gamma")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    CHECKER = os.path.abspath(sys.argv.pop(1))
    unittest.main(verbosity=2)
