import pathlib,subprocess,tarfile
r=pathlib.Path('/tmp/us3-residual-2ddf75d6e/a979-verification');s=r/'clean-source';s.mkdir(exist_ok=True)
subprocess.run(['curl','-fsS','http://10.211.55.2:18768/a979-boundary-source.tar','-o',str(r/'boundary-source.tar')],check=True)
with tarfile.open(str(r/'boundary-source.tar')) as t:t.extractall(str(s))
p=r/'m12-clean-project';p.mkdir(exist_ok=True)
(p/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(M12Clean NONE)\nenable_testing()\nset(PROJECT_SOURCE_DIR '+str(s)+')\nadd_subdirectory('+str(s/'test/dependency_layers')+' dependency_layers)\n')
with open(r/'m12-clean-configure.log','w') as log:subprocess.run(['cmake','-S',str(p),'-B',str(r/'m12-clean-build')],stdout=log,stderr=log,check=True)
with open(r/'m12-clean-ctest.log','w') as log:subprocess.run(['ctest','--test-dir',str(r/'m12-clean-build'),'-L','dependency_layers','--output-on-failure'],stdout=log,stderr=log,check=True)
print('Clean-source M12 CTest passed')
