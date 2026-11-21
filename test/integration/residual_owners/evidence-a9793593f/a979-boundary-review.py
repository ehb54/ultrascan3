import importlib.util,pathlib,json
r=pathlib.Path('/tmp/us3-residual-2ddf75d6e');out=r/'a979-verification'
p=r/'source/test/dependency_layers/test_dependency_layers.py'
s=importlib.util.spec_from_file_location('tests',str(p));m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
m.CHECKER=str(r/'source/scripts/check-dependency-layers.py')
rows=[]
for name,path,text,expected in [('static GUI target','utils/CMakeLists.txt','target_link_libraries(us_utils PRIVATE us_gui_static)\n',1),('intralibrary generated include','gui/moc/moc_us_widget.cpp','#include "../us_widget.h"\n',0)]:
 f=dict(m.VALID);f[path]=f.get(path,'')+text
 code,msg=m.run_checker(f);rows.append(dict(case=name,expected=expected,actual=code,output=msg,passed=code==expected))
(out/'m12-review-probes.json').write_text(json.dumps(rows,indent=2)+'\n');print(json.dumps(rows,indent=2))
