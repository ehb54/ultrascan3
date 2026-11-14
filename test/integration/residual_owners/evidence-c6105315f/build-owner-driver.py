import pathlib,subprocess,shlex
root=pathlib.Path('/tmp/us3-residual-2ddf75d6e');build=root/'build/programs/us_autoflow_analysis'
args=[]
for line in (build/'CMakeFiles/us_autoflow_analysis.dir/flags.make').read_text().splitlines():
 if line.startswith(('CXX_DEFINES =','CXX_INCLUDES =','CXX_FLAGS =')):args+=shlex.split(line.split('=',1)[1])
obj=root/'autoflow-owner-driver.o'
subprocess.run(['/usr/bin/c++']+args+['-I'+str(root/'source/programs'),'-I'+str(root/'source/programs/us_autoflow_analysis'),'-c',str(root/'autoflow-owner-driver.cpp'),'-o',str(obj)],cwd=str(build),check=True)
cmd=shlex.split((build/'CMakeFiles/us_autoflow_analysis.dir/link.txt').read_text())
cmd=[str(obj) if x.endswith('/us_autoflow_analysis_main.cpp.o') else x for x in cmd];cmd[cmd.index('-o')+1]=str(root/'build/bin/us_autoflow_owner_probe')
subprocess.run(cmd,cwd=str(build),check=True)
