from pathlib import Path
import os,subprocess,json,numpy as np,re,time
r=Path(__file__).resolve().parent;exe=r/'verified-harness.exe';rows=[]
cases=[
 ('intense-max',100,2,50,100,100,100,5,0,3,20,16,{}),
 ('driven-max',100,2,50,100,100,100,5,2.5,1,20,16,{}),
 ('low-fps',100,2,50,95,50,50,70,0,1,15,20,{}),
 ('high-fps',100,2,50,95,50,50,70,0,1,60,12,{}),
 ('semi',50,1,50,100,50,50,5,0,1,30,12,{}),
 ('erect',50,0,27,100,50,50,50,0,0,30,12,{}),
 ('small',0,2,50,0,0,0,5,.035,1,30,12,{}),
 ('restart',50,2,50,50,50,50,50,.3,0,20,30,{'DRIVE_STOP':'6','DRIVE_RESUME':'23'}),
 ('long-rest',50,2,50,50,50,50,50,0,0,20,45,{})]
for name,size,state,angle,scrotum,width,length,hang,drive,pulse,fps,seconds,extra in cases:
 env=os.environ.copy();env.update(extra);env['THROB_LEVEL']=str(pulse);file=r/('stress-'+name+'.bin')
 args=[exe,file,size,state,angle,scrotum,int(seconds*fps),width,length,hang,drive,50,50,50,50,0,0,0,0,int(pulse>0),fps]
 begin=time.perf_counter();run=subprocess.run(list(map(str,args)),env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);log=run.stdout.decode(errors='replace');(r/('stress-'+name+'.log')).write_text(log)
 a=np.fromfile(str(file)+'.series',np.float32).reshape(-1,92);m=np.fromfile(str(file)+'.fine-metrics',np.float32).reshape(-1,7)
 tether=float(re.search(r'SOLVER maximum_tether_ratio ([\d.]+)',log)[1]);gap=float(re.search(r'POUCH gap=([-\d.]+)',log)[1])
 v=np.linalg.norm(np.diff(a[:,44:50].reshape(-1,2,3),axis=0)*fps,axis=2)
 row=dict(case=name,exit=run.returncode,seconds=time.perf_counter()-begin,finite=bool(np.isfinite(a).all()),min_triangle_area2=float(m[:,3].min()),invalid_vertices=int(m[:,4].max()),max_parent_opposed_faces=int(m[:,5].max()),seam_error=float(m[:,6].max()),max_tether_ratio=tether,min_core_gap=gap,last2_speed95=float(np.percentile(v[-2*fps:],95)))
 if name=='restart':row.update(rest_speed95=float(np.percentile(v[20*fps:23*fps],95)),resumed_speed95=float(np.percentile(v[24*fps:],95)))
 row['pass']=bool(run.returncode==0 and row['finite'] and row['min_triangle_area2']>0 and row['invalid_vertices']==0 and row['seam_error']<1e-5 and tether<1.01 and gap>=-1e-4)
 rows.append(row);(r/'stress-report.json').write_text(json.dumps(rows,indent=2));print(json.dumps(row),flush=True)
if not all(x['pass'] for x in rows):raise SystemExit(1)
