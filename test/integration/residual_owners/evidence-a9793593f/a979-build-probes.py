import pathlib,subprocess,shlex
r=pathlib.Path('/tmp/us3-residual-2ddf75d6e');out=r/'a979-verification'
for name,target in [('a979-make-timestate','us_fematch'),('a979-reporter-auto','us_reporter_gmp')]:
 src=out/(name+'.cpp');subprocess.run(['curl','-fsS','http://10.211.55.2:18768/'+src.name,'-o',str(src)],check=True)
 b=r/'build/programs'/target;d=b/('CMakeFiles/'+target+'.dir');args=[]
 for l in (d/'flags.make').read_text().splitlines():
  if l.startswith(('CXX_DEFINES =','CXX_INCLUDES =','CXX_FLAGS =')):args+=shlex.split(l.split('=',1)[1])
 obj=out/(name+'.o')
 subprocess.run(['/usr/bin/c++']+args+['-I'+str(r/'source/programs'),'-I'+str(r/'source/programs'/target),'-c',str(src),'-o',str(obj)],cwd=str(b),check=True)
 cmd=shlex.split((d/'link.txt').read_text());cmd=[str(obj) if x.endswith('/'+target+'_main.cpp.o') else x for x in cmd];cmd[cmd.index('-o')+1]=str(r/'build/bin'/name)
 subprocess.run(cmd,cwd=str(b),check=True)
 subprocess.run(['chown','us3:us3',str(r/'build/bin'/name)],check=True)
