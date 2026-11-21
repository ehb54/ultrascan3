import pathlib,shutil,subprocess
r=pathlib.Path('/tmp/us3-residual-2ddf75d6e');out=r/'a979-verification'
p=r/'settings-a979-d14';shutil.copytree(r/'settings-a979',p)
f=p/'UltraScan3/UltraScan.ini';f.write_text(f.read_text().replace(str(r/'output-a979'),str(r/'output-a979-d14')))
d=r/'output-a979-d14/results/multicell-demo-ra';shutil.copytree(r/'output/results/multicell-demo-ra',d)
(r/'output-a979-d14/reports').mkdir()
subprocess.run(['chown','-R','us3:us3',str(p),str(r/'output-a979-d14')],check=True)
with open(out/'d14-timestate-fixture.log','w') as log:
 subprocess.run(['runuser','-u','us3','--','env','ASAN_OPTIONS=detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1',str(r/'build/bin/a979-make-timestate'),str(d/'multicell-demo-ra.RA.1.A.123.auc'),str(d/'multicell-demo-ra.time_state.tmst')],stdout=log,stderr=log,check=True)
print('Local-only TimeState fixture prepared; no experiment XML/speed-step list; database unchanged')
