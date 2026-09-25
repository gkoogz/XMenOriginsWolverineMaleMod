from pathlib import Path
import subprocess,os,json,numpy as np,time
r=Path(__file__).resolve().parent;rows=[]
cases=[
 ('large-gentle',100,2,50,95,480,50,50,70,0,1,False,20),
 ('tight-shift',100,2,50,100,480,50,50,5,.035,0,True,20),
 ('tight-gentle-shift',100,2,50,100,480,50,50,5,.035,1,True,20),
 ('default-off',50,2,50,50,400,50,50,50,0,0,False,20),
 ('stress',70,2,50,50,240,50,50,50,2.5,1,False,20),
 ('erect',50,0,27,100,240,50,50,50,0,0,False,30),
 ('low-fps',100,2,50,95,240,50,50,70,0,1,False,15)]
for name,size,state,angle,scrotum,frames,width,length,hang,drive,pulse,shift,fps in cases:
 variants=['after']
 for variant in variants:
  file=r/(variant+'-'+name+'.bin');exe=r/('baseline-replay.exe' if variant=='before' else 'final-harness.exe');env=os.environ.copy()
  if shift:env['WEIGHT_SHIFT']='1'
  args=[exe,file,size,state,angle,scrotum,frames,width,length,hang,drive,50,50,50,50,0,0,0,0,pulse,fps]
  start=time.perf_counter();run=subprocess.run(list(map(str,args)),env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);elapsed=time.perf_counter()-start;(r/(variant+'-'+name+'.log')).write_bytes(run.stdout)
  print(variant,name,run.returncode,run.stdout.decode(errors='replace'),flush=True)
  if not Path(str(file)+'.series').exists():raise RuntimeError(name)
  a=np.fromfile(str(file)+'.series',np.float32).reshape(-1,92);p=a[:,44:50].reshape(-1,2,3);start_index=int(6*fps);v=np.diff(p,axis=0)*fps;acc=np.diff(v,axis=0)*fps;jerk=np.diff(acc,axis=0)*fps
  m=np.fromfile(str(file)+'.fine-metrics',np.float32).reshape(-1,7)
  row=dict(case=name,variant=variant,exit_code=run.returncode,elapsed_seconds=elapsed,frames=frames,fps=fps,finite=bool(np.isfinite(a).all()),speed_p95=float(np.percentile(np.linalg.norm(v[start_index:],axis=2),95)),jerk_p95=float(np.percentile(np.linalg.norm(jerk[start_index:],axis=2),95)),last2_seconds_motion=np.linalg.norm(np.ptp(p[-int(2*fps):],axis=0),axis=1).tolist(),minimum_triangle_area2=float(m[:,3].min()),invalid_vertices=int(m[:,4].max()),seam_error=float(m[:,6].max()),metrics=[x for x in run.stdout.decode(errors='replace').splitlines() if x.startswith(('BOUNDS','SOLVER','POUCH'))])
  rows.append(row);(r/'validation.json').write_text(json.dumps(rows,indent=2))
  if variant=='after' and (run.returncode or not row['finite'] or row['invalid_vertices'] or row['seam_error']>1e-5):print('NEEDS REVIEW:',name,flush=True)

assert all(x['exit_code']==0 and x['finite'] and x['invalid_vertices']==0 and x['minimum_triangle_area2']>0 and x['seam_error']<=1e-5 for x in rows), 'Validation failed; inspect validation.json'
