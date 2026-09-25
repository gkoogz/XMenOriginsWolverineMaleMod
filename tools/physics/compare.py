from pathlib import Path
import subprocess,os,json,numpy as np,sys,time
from scipy.signal import butter,sosfiltfilt
r=Path(__file__).resolve().parent
variant=sys.argv[1] if len(sys.argv)>1 else 'candidate'
exe=r/os.environ.get('HARNESS','verified-harness.exe')
cases=[('large',100,95,70,0,1,False),('tight',100,100,5,.035,0,True),('tight-pulse',100,100,5,.035,1,True),('rest',50,50,50,0,0,False)]
if len(sys.argv)>2:cases=[c for c in cases if c[0] in sys.argv[2:]]
rows=[]
for name,size,scrotum,hang,drive,pulse,shift in cases:
 fps=30;frames=600;file=r/(variant+'-'+name+'.bin');env=os.environ.copy()
 if shift:env['WEIGHT_SHIFT']='1'
 args=[exe,file,size,2,50,scrotum,frames,50,50,hang,drive,50,50,50,50,0,0,0,0,pulse,fps]
 t=time.perf_counter();run=subprocess.run(list(map(str,args)),env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);(r/(variant+'-'+name+'.log')).write_bytes(run.stdout)
 a=np.fromfile(str(file)+'.series',np.float32).reshape(-1,92);p=a[:,8:50].reshape(-1,14,3)
 v=np.diff(p,axis=0)*fps;acc=np.diff(v,axis=0)*fps;jerk=np.diff(acc,axis=0)*fps
 hf=sosfiltfilt(butter(3,6,fs=fps,btype='highpass',output='sos'),p,axis=0)
 row=dict(case=name,variant=variant,exit=run.returncode,seconds=time.perf_counter()-t,finite=bool(np.isfinite(a).all()))
 for label,points in [('body',slice(12,14)),('rod',slice(2,12))]:
  row[label]=dict(speed95=float(np.percentile(np.linalg.norm(v[180:,points],axis=2),95)),jerk95=float(np.percentile(np.linalg.norm(jerk[180:,points],axis=2),95)),hf_rms=float(np.sqrt(np.mean(hf[180:-30,points]**2))),last2_speed95=float(np.percentile(np.linalg.norm(v[-60:,points],axis=2),95)))
 row['checks']=[x for x in run.stdout.decode(errors='replace').splitlines() if x.startswith(('BOUNDS','SOLVER','POUCH'))]
 rows.append(row);print(json.dumps(row),flush=True)
 (r/(variant+'-report.json')).write_text(json.dumps(rows,indent=2))
