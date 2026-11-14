"""Reproduce the scan flag omission using exact production function bodies.

Set CXX to clang++ and PKG_CONFIG_PATH for Qt6Core/Qt6Xml. Run from repo root.
This is a focused helper probe, not an instrumented real-owner workflow.
"""
import os
from pathlib import Path
import shlex
import subprocess


def function(path, signature):
    source = Path(path).read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end] + "\n"


out = Path("build/c11-audit")
out.mkdir(parents=True, exist_ok=True)
body = '#include "us_astfem_math.h"\n'
for name in ("pointCount", "scanCount"):
    body += function("utils/us_dataIO.cpp", "int US_DataIO::EditedData::" + name + "(")
body += function("utils/us_astfem_math.cpp", "void US_AstfemMath::initSimData( US_DataIO::RawData& simdata,")
body += '''
int main() {
 US_DataIO::EditedData ed{}; US_DataIO::RawData sim{};
 ed.cell="1"; ed.channel="A"; ed.xvalues={6.0,6.1};
 US_DataIO::Scan scan{}; scan.rvalues={1.0,2.0}; ed.scanData.append(scan);
 US_AstfemMath::initSimData(sim,ed,0.0);
 return sim.scanData[0].nz_stddev ? 1 : 0;
}
'''
flags = shlex.split(subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "Qt6Core", "Qt6Xml"], text=True))
libdir = subprocess.check_output(
    ["pkg-config", "--variable=libdir", "Qt6Core"], text=True).strip()
for name, text in (("probe", body), ("control", body.replace(
        "sscan.temperature      = escan->temperature;",
        "sscan.nz_stddev = false;\n      sscan.temperature      = escan->temperature;"))):
    source = out / (name + ".cpp")
    source.write_text(text)
    cmd = [os.environ.get("CXX", "clang++"), "-std=c++17", "-O0", "-g",
           "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
           "-ftrivial-auto-var-init=pattern", "-Iutils", str(source),
           "-o", str(out / name)] + flags + ["-Wl,-rpath," + libdir]
    (out / (name + "-command.txt")).write_text(shlex.join(cmd) + "\n")
    subprocess.run(cmd, check=True)
    result = subprocess.run([str(out / name)], capture_output=True, text=True)
    (out / (name + ".log")).write_text(
        "exit=" + str(result.returncode) + "\n" + result.stdout + result.stderr)
    print(name, result.returncode)
    if name == "probe":
        assert result.returncode != 0 and "not a valid value for type 'bool'" in result.stderr
    else:
        assert result.returncode == 0
