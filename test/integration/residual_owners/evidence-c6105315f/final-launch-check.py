import subprocess,os,time,json,hashlib,pathlib
root=pathlib.Path('/tmp/us3-residual-2ddf75d6e');out=root/'launch-evidence';out.mkdir(exist_ok=True)
env=dict(os.environ,DISPLAY=':43',ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1',US3_SETTINGS_ROOT=str(root/'settings'))
results=[]
for name in ['us_fematch','us_autoflow_analysis','us_query_rmsd','us_reporter_gmp','us_comproject','us_comproject_academic','us_protocoldev']:
 exe=root/'build/bin'/name
 with open(out/(name+'.log'),'w') as log:
  p=subprocess.Popen([str(exe)],env=env,stdout=log,stderr=log)
  time.sleep(45 if name in ('us_comproject', 'us_comproject_academic') else 6)
  w=subprocess.run(['xdotool','search','--onlyvisible','--pid',str(p.pid)],env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE,universal_newlines=True)
  titles=[]
  for wid in w.stdout.split():
   titles.append(subprocess.run(['xdotool','getwindowname',wid],env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE,universal_newlines=True).stdout.strip())
  subprocess.run(['import','-window','root',str(out/(name+'.png'))],env=env)
  ldd=subprocess.run(['ldd',str(exe)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,universal_newlines=True)
  results.append(dict(program=name,sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),alive=p.poll() is None,window_titles=titles,missing_libraries='not found' in ldd.stdout,ldd=ldd.stdout))
  if p.poll() is None:
   p.terminate()
   try:p.wait(timeout=5)
   except subprocess.TimeoutExpired:p.kill();p.wait()
(root/'launch-evidence/results.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps([{k:v for k,v in r.items() if k!='ldd'} for r in results],indent=2))

assert all(r["alive"] and r["window_titles"] and not r["missing_libraries"] for r in results), "Consumer launch failed; see results.json and logs"
