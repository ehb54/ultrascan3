import json,shlex,subprocess,pathlib
root=pathlib.Path.cwd();out=root/'build/c13-audit';out.mkdir(parents=True,exist_ok=True)
(out/'probe.cpp').write_text((root/'test/integration/residual_owners/c13-audit/probe.cpp').read_text())
j=json.load(open('build/baseline-qt6/compile_commands.json'))
x=next(x for x in j if x['file'].endswith('/us_dataIO.cpp') and 'US_UTIL_STATIC' in x['command'])
base=shlex.split(x['command']);base[0]='/opt/homebrew/opt/llvm@18/bin/clang++'
i=base.index('-o');base=base[:i]+base[i+2:];base.remove('-c');base.remove(x['file']);base=[v for v in base if not v.startswith('-O')]
libroot=pathlib.Path('/Users/me3/jetbrains/CLionProjects/ultrascan3-cli/build-mac/vcpkg_installed/arm64-osx-dynamic/lib')
libs=[str(root/'build/baseline-qt6/lib/libus_utils4_static.a')]+[str(libroot/('lib'+n+'.dylib')) for n in ['Qt6Core','Qt6Xml','Qt6Network','Qt6Sql','ssl','crypto','archive','mariadb']]+['-framework','Security','-framework','Foundation','-Wl,-rpath,'+str(libroot)]
for kind in ['pattern','zero']:
 cmd=base+['-O0','-g','-fsanitize=undefined','-fno-sanitize-recover=undefined','-ftrivial-auto-var-init='+kind,x['file'],str(out/'probe.cpp'),'-o',str(out/kind)]+libs
 (out/(kind+'-command.txt')).write_text(shlex.join(cmd)+'\n')
 r=subprocess.run(cmd,capture_output=True,text=True);print(kind,'compile',r.returncode,r.stderr[-2000:])
 if not r.returncode:
  r=subprocess.run([str(out/kind)],capture_output=True,text=True);(out/(kind+'.log')).write_text('exit='+str(r.returncode)+'\n'+r.stdout+r.stderr);print(r.returncode,r.stdout,r.stderr)
