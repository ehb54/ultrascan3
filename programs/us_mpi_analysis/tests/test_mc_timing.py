#!/usr/bin/env python3
"""Exercise the production MC time estimator with a deterministic clock and UBSan.
Run with Python 3 and a C++11 compiler (CXX, or g++); no Qt/MPI build needed.
"""
import os
from pathlib import Path
import subprocess
import tempfile

source = (Path(__file__).resolve().parents[1] / 'parallel_masters.cpp').read_text()
function = source.index('void US_MPI_Analysis::time_mc_iterations()')
start = source.index('   const qint64 elapsed_seconds', function)
end = source.index('   if ( show_mcit )', start)
block = source[start:end].replace(
    '   const int mc_iters_estim        = mc_iteration + mc_iters_left;',
    '   estimate = mc_iteration + mc_iters_left;')
head = '#include <cassert>\n#include <cstdio>\nusing qint64 = long long;\nstruct Clock { long long elapsed; long long secsTo(int) const { return elapsed; } };\nvoid estimate_remaining(long long seconds, int max_walltime, int mc_iteration, int mgroup_count, int& estimate) {\n Clock startTime{seconds}; int currTime=0;\n'
tail = '}\nint main() {\n int result=-1;\n estimate_remaining(0,999999,3,1,result); assert(result==-1);\n estimate_remaining(-1,999999,3,1,result); assert(result==-1);\n estimate_remaining(1,999999,3,1,result); assert(result>=5);\n estimate_remaining(60,2,3,1,result); assert(result==6);\n estimate_remaining(120,2,3,1,result); assert(result==3);\n estimate_remaining(60,3,6,2,result); assert(result==18);\n puts("PASS: zero/backward clock, fast job, normal estimate, exhausted allowance, multiple groups");\n}\n'

with tempfile.TemporaryDirectory() as temp:
    root = Path(temp)
    (root / 'test.cpp').write_text(head + block + tail)
    subprocess.run([os.environ.get('CXX', 'g++'), '-std=c++11', '-fsanitize=undefined',
                    '-fno-sanitize-recover=all', str(root / 'test.cpp'), '-o', str(root / 'test')], check=True)
    subprocess.run([str(root / 'test')], check=True)
