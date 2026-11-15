import pathlib,json,hashlib,subprocess,shutil
r=pathlib.Path('/tmp/us3-residual-2ddf75d6e');out=r/'a979-verification'
m=json.loads((out/'source-manifest.json').read_text());bad=[n for n,h in m.items() if hashlib.sha256((r/'source'/n).read_bytes()).hexdigest()!=h]
(out/'source-verification.json').write_text(json.dumps(dict(revision='a9793593f',files=len(m),mismatches=bad),indent=2))
assert not bad,bad
p=out/'m12-project';p.mkdir(exist_ok=True)
(p/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(M12Verification NONE)\nenable_testing()\nset(PROJECT_SOURCE_DIR '+str(r/'source')+')\nadd_subdirectory('+str(r/'source/test/dependency_layers')+' dependency_layers)\n')
with open(out/'m12-configure.log','w') as log:subprocess.run(['cmake','-S',str(p),'-B',str(out/'m12-build')],stdout=log,stderr=log,check=True)
with open(out/'m12-standalone-ctest.log','w') as log:subprocess.run(['ctest','--test-dir',str(out/'m12-build'),'-L','dependency_layers','--output-on-failure'],stdout=log,stderr=log)
for profile,base,debug in [('settings-a979','output-a979',False),('settings-a979-abort','output-a979-abort',True)]:
 dest=r/profile
 if dest.exists():raise RuntimeError('profile exists; inspect before reuse')
 shutil.copytree(r/'settings',dest)
 f=dest/'UltraScan3/UltraScan.ini';s=f.read_text().replace(str(r/'output'),str(r/base))
 if debug:s=s.replace('[General]','[General]\ndebug_text=SetSpeedLowAcc=100000',1)
 f.write_text(s)
 for d in ['reports','results']: (r/base/d).mkdir(parents=True,exist_ok=True)
 subprocess.run(['chown','-R','us3:us3',str(dest),str(r/base)],check=True)
print('Source hashes match; M12 result retained in log; isolated profiles ready')
