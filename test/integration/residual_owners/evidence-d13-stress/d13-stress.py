import os,sys,time,pathlib,subprocess,json,shutil,ctypes
R=pathlib.Path('/tmp/us3-residual-2ddf75d6e'); O=R/'d13-stress';O.mkdir(exist_ok=True)
env=dict(os.environ,DISPLAY=':44',HOME='/home/us3',US3_SETTINGS_ROOT=str(R/'settings-d13-stress'),ASAN_OPTIONS='detect_leaks=0:halt_on_error=1:disable_coredump=0',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
def call(args,check=True):return subprocess.run(args,env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=check,timeout=15)
def xd(*a):return call(['xdotool']+list(map(str,a))).stdout.decode().strip()
def waitwin(title,timeout=40):
 end=time.time()+timeout
 while time.time()<end:
  p=call(['xdotool','search','--all','--onlyvisible','--pid',str(APP_PID),'--name','^'+title+'$'],False)
  if p.returncode==0:return p.stdout.decode().splitlines()[-1]
  time.sleep(.25)
 raise RuntimeError('Window timeout: '+title)
def click(x,y):xd('mousemove',x,y,'click',1,'mousemove',0,0);time.sleep(.35)
def wc(w,x,y):
 lib=ctypes.CDLL('libX11.so.6');lib.XOpenDisplay.restype=ctypes.c_void_p
 d=lib.XOpenDisplay(b':44');lib.XDefaultRootWindow.argtypes=[ctypes.c_void_p];lib.XDefaultRootWindow.restype=ctypes.c_ulong
 lib.XTranslateCoordinates.argtypes=[ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_int,ctypes.c_int,ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_ulong)]
 ox=ctypes.c_int();oy=ctypes.c_int();child=ctypes.c_ulong()
 lib.XTranslateCoordinates(d,int(w),lib.XDefaultRootWindow(d),0,0,ctypes.byref(ox),ctypes.byref(oy),ctypes.byref(child))
 lib.XCloseDisplay.argtypes=[ctypes.c_void_p];lib.XCloseDisplay(d)
 click(ox.value+x,oy.value+y)
def capture(name):call(['import','-window','root',str(O/name)])
def activate(w,x=0,y=0):xd('windowmove',w,x+5 if x else 0,y+28 if y else 0);xd('windowactivate',w);time.sleep(.3)
if not (R/'settings-d13-stress').exists():
 shutil.copytree(R/'settings-a979',R/'settings-d13-stress')
 p=R/'settings-d13-stress/UltraScan3/UltraScan.ini';p.write_text(p.read_text().replace(str(R/'output-a979'),str(R/'output-d13-stress')))
 subprocess.run(['chown','-R','us3:us3',str(R/'settings-d13-stress')],check=True)
if not pathlib.Path('/tmp/.X11-unix/X44').exists():
 subprocess.Popen(['runuser','-u','us3','--','Xvfb',':44','-screen','0','1600x1000x24','-nolisten','tcp'],stdout=open(O/'xvfb.log','w'),stderr=subprocess.STDOUT,start_new_session=True);time.sleep(1)
 subprocess.Popen(['runuser','-u','us3','--','env','DISPLAY=:44','xfwm4'],stdout=open(O/'wm.log','w'),stderr=subprocess.STDOUT,start_new_session=True);time.sleep(1)
subprocess.run(['chown','-R','us3:us3',str(O)],check=True)
start=int(sys.argv[1]);count=int(sys.argv[2])
for cycle in range(start,start+count):
 log=O/('cycle-%02d.log'%cycle); result={'cycle':cycle,'started':time.time(),'loads':0}
 cmd=['runuser','-u','us3','--','env']+[k+'='+env[k] for k in ['DISPLAY','HOME','US3_SETTINGS_ROOT','ASAN_OPTIONS','UBSAN_OPTIONS']]+['gdb','-q','-batch','-ex','set pagination off','-ex','set confirm off','-ex','handle SIGPIPE nostop noprint pass','-ex','run','-ex','thread apply all bt full','-ex','generate-core-file '+str(O/('cycle-%02d.core'%cycle)),'--args',str(R/'build/bin/us_fematch')]
 with open(log,'w') as f:p=subprocess.Popen(cmd,stdout=f,stderr=f,start_new_session=True)
 try:
  APP_PID=None
  for attempt in range(100):
   for proc in pathlib.Path('/proc').iterdir():
    if not proc.name.isdigit():continue
    try:
     if (proc/'exe').resolve()==R/'build/bin/us_fematch' and b'DISPLAY=:44' in (proc/'environ').read_bytes().split(b'\0'):APP_PID=int(proc.name)
    except OSError:pass
   if APP_PID:break
   time.sleep(.1)
  if not APP_PID:raise RuntimeError('No stress instance started')
  result['pid']=APP_PID
  main=waitwin('Compare Experimental Data to Sums of Finite Element Solutions');activate(main)
  for iteration in range(2):
   activate(main);wc(main,95,13)
   w=waitwin('Load Run Data from DB');activate(w,430,193);wc(w,116,321);wc(w,629,487)
   time.sleep(1);activate(main);wc(main,95,85)
   w=waitwin('Load Distribution Model');activate(w,196,200);wc(w,289,61);xd('type','2609300625');time.sleep(.4);wc(w,289,83);wc(w,582,487);time.sleep(.7)
   activate(main);wc(main,295,85)
   w=waitwin('Finite Element Data/Residuals Viewer',90);activate(w,290,271)
   capture('cycle-%02d-load-%d.png'%(cycle,iteration+1))
   # Explicitly exercise bitmap create/destroy on every load.
   wc(w,20,286);waitwin('Residuals Pixel Map');activate(w,290,271);wc(w,20,286)
   wc(w,141,592);time.sleep(.5)
   result['loads']+=1
  activate(main);wc(main,330,627)
  p.wait(timeout=20)
  text=log.read_text()
  result['normal_exit']='exited normally' in text and not any(s in text for s in ['Program received signal','ERROR: AddressSanitizer','runtime error:'])
  result['gdb_exit']=p.returncode
  if not result['normal_exit']:raise RuntimeError('Abnormal process exit; inspect retained debugger log')
  result['status']='PASS'
 except Exception as e:
  result['status']='STOPPED';result['error']=str(e);capture('cycle-%02d-stopped.png'%cycle)
 result['elapsed_seconds']=round(time.time()-result['started'],2)
 (O/('cycle-%02d.json'%cycle)).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
 if result['status']!='PASS':sys.exit(1)
