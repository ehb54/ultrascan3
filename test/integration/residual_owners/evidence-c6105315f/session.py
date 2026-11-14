import os,subprocess,pathlib,sys,time,json
root=pathlib.Path('/tmp/us3-residual-2ddf75d6e');out=root/'ui';out.mkdir(exist_ok=True)
if sys.argv[1]=='display':
 for cmd,name in [(['Xvfb',':43','-screen','0','1600x1000x24','-nolisten','tcp'],'xvfb'),(['env','DISPLAY=:43','xfwm4'],'wm')]:
  with open(out/(name+'.log'),'w') as log:subprocess.Popen(['runuser','-u','us3','--']+cmd,stdout=log,stderr=log,start_new_session=True)
  time.sleep(1)
elif sys.argv[1]=='launch':
 name=sys.argv[2];profile=sys.argv[3] if len(sys.argv)>3 else 'settings'
 with open(out/(name+'-'+profile+'.log'),'w') as log:
  p=subprocess.Popen(['runuser','-u','us3','--','env','DISPLAY=:43','HOME=/home/us3','US3_SETTINGS_ROOT='+str(root/profile),'ASAN_OPTIONS=detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1',str(root/'build/bin'/name)],stdout=log,stderr=log,start_new_session=True)
 print(p.pid)
elif sys.argv[1]=='capture':
 subprocess.run(['env','DISPLAY=:43','import','-window','root',str(out/'current.png')],check=True)
