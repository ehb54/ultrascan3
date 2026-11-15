import pathlib,subprocess
r=pathlib.Path('/tmp/us3-residual-2ddf75d6e');out=r/'a979-verification'
cmd=['runuser','-u','us3','--','env','DISPLAY=:43','HOME=/home/us3','US3_SETTINGS_ROOT='+str(r/'settings-a979'),'ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:disable_coredump=0','UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1','gdb','-q','-batch','-ex','set pagination off','-ex','set confirm off','-ex','handle SIGPIPE nostop noprint pass','-ex','run','-ex','thread apply all bt full','-ex','generate-core-file '+str(out/'fematch.core'),'--args',str(r/'build/bin/us_fematch')]
with open(out/'fematch-gdb.log','w') as log:p=subprocess.Popen(cmd,stdout=log,stderr=log,start_new_session=True)
(out/'fematch-gdb-launch.json').write_text(__import__('json').dumps({'command':cmd,'pid':p.pid},indent=2))
print(p.pid)
