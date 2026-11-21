import pathlib,shutil,subprocess
root=pathlib.Path('/tmp/us3-residual-2ddf75d6e')
dest=root/'settings-abort'
if dest.exists():raise SystemExit('Abort profile already exists; inspect before changing')
shutil.copytree(root/'settings',dest)
f=dest/'UltraScan3/UltraScan.ini'
s=f.read_text().replace(str(root/'output'),str(root/'output-abort'))
assert '[General]' in s
assert 'debug_text=' not in s
s=s.replace('[General]','[General]\ndebug_text=SetSpeedLowAcc=100000',1)
f.write_text(s)
(root/'output-abort/reports').mkdir(parents=True)
(root/'output-abort/results').mkdir()
subprocess.run(['chown','-R','us3:us3',str(dest),str(root/'output-abort')],check=True)
print('Created isolated Abort profile and empty output directory')
