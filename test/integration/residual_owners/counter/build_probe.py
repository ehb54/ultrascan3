#!/usr/bin/env python3
"""Build a real-counter constructor probe using an existing sanitized owner build."""
import argparse
import pathlib
import shlex
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("root", type=pathlib.Path, help="Verification root containing source/ and build/")
args = parser.parse_args()
root = args.root.resolve()
build = root / "build/programs/us_autoflow_analysis"
flags = []
for line in (build / "CMakeFiles/us_autoflow_analysis.dir/flags.make").read_text().splitlines():
    if line.startswith(("CXX_DEFINES =", "CXX_INCLUDES =", "CXX_FLAGS =")):
        flags += shlex.split(line.split("=", 1)[1])
obj = root / "counter-probe.o"
source = pathlib.Path(__file__).with_name("constructor_probe.cpp")
subprocess.run(["/usr/bin/c++"] + flags + ["-c", str(source), "-o", str(obj)], cwd=build, check=True)
command = shlex.split((build / "CMakeFiles/us_autoflow_analysis.dir/link.txt").read_text())
command = [str(obj) if item.endswith("/us_autoflow_analysis_main.cpp.o") else item for item in command]
command[command.index("-o") + 1] = str(root / "build/bin/us_counter_probe")
subprocess.run(command, cwd=build, check=True)
