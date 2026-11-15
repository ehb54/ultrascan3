import os,sys,time,pathlib,subprocess,json,shutil,ctypes
R=pathlib.Path('/tmp/us3-residual-2ddf75d6e'); O=R/'d13-stress';O.mkdir(exist_ok=True)
env=dict(os.environ,DISPLAY=':43',HOME='/home/us3',US3_SETTINGS_ROOT=str(R/'settings-d13-stress'),ASAN_OPTIONS='detect_leaks=0:halt_on_error=1:disable_coredump=0',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
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
 d=lib.XOpenDisplay(b':43');lib.XDefaultRootWindow.argtypes=[ctypes.c_void_p];lib.XDefaultRootWindow.restype=ctypes.c_ulong
 lib.XTranslateCoordinates.argtypes=[ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_int,ctypes.c_int,ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(ctypes.c_ulong)]
 ox=ctypes.c_int();oy=ctypes.c_int();child=ctypes.c_ulong()
 lib.XTranslateCoordinates(d,int(w),lib.XDefaultRootWindow(d),0,0,ctypes.byref(ox),ctypes.byref(oy),ctypes.byref(child))
 lib.XCloseDisplay.argtypes=[ctypes.c_void_p];lib.XCloseDisplay(d)
 click(ox.value+x,oy.value+y)
def capture(name):call(['import','-window','root',str(O/name)])
def activate(w,x=0,y=0):xd('windowmove',w,x+5 if x else 0,y+28 if y else 0);xd('windowactivate',w);time.sleep(.3)

APP_PID=125271
result={'pid':APP_PID,'loads':0,'started':time.time()}
try:
 p=call(['xdotool','search','--all','--onlyvisible','--pid',str(APP_PID),'--name','^Successfully Written$'],False)
 if p.returncode==0:
  xd('windowactivate',p.stdout.decode().splitlines()[-1]);xd('key','Return');time.sleep(.5)
 p=call(['xdotool','search','--all','--onlyvisible','--pid',str(APP_PID),'--name','^Finite Element Data/Residuals Viewer$'],False)
 if p.returncode==0:
  w=p.stdout.decode().splitlines()[-1];activate(w,290,271);wc(w,141,592)
 main=waitwin('Compare Experimental Data to Sums of Finite Element Solutions')
 for iteration in range(3):
  activate(main);wc(main,95,13)
  w=waitwin('Load Run Data from DB');activate(w,430,193);wc(w,116,321);wc(w,629,487)
  time.sleep(1);activate(main);wc(main,95,85)
  w=waitwin('Load Distribution Model');activate(w,196,200);wc(w,289,61);xd('type','2609300625');time.sleep(.4);wc(w,289,83);wc(w,582,487);time.sleep(.7)
  activate(main);wc(main,295,85)
  w=waitwin('Finite Element Data/Residuals Viewer',90);activate(w,290,271)
  capture('resumed-load-%d.png'%(iteration+1))
  wc(w,141,592);result['loads']+=1
 activate(main);wc(main,330,627);time.sleep(2)
 result['status']='Completed UI sequence; inspect original gdb log for exit status'
except Exception as e:
 result['status']='STOPPED';result['error']=str(e);capture('resumed-stopped.png')
result['elapsed_seconds']=round(time.time()-result['started'],2)
(O/'resumed.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
