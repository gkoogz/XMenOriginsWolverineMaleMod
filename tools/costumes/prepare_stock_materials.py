"""Validate licensed stock DDS exports and embed them in a private build.

Git contains the recipe and hashes, not stock pixel assets. Set
MALEMOD_STOCK_MAPS to UEViewer's Startup_int/Texture2D DDS export directory.
"""
import argparse,hashlib,json,os,shutil
from pathlib import Path

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--runtime',type=Path,required=True);a=ap.parse_args()
 root=Path(os.environ.get('MALEMOD_STOCK_MAPS',''))
 manifest=json.loads(Path(__file__).with_name('stock_materials.json').read_text())
 out=a.runtime/'stock-materials';out.mkdir(exist_ok=True)
 rows=[]
 for item in manifest['maps']:
  source=root/item['name']
  if not source.is_file() or hashlib.sha256(source.read_bytes()).hexdigest()!=item['sha256']:
   raise SystemExit('Missing or changed licensed DDS: '+str(source)+'; see docs/STOCK-CLOTHING-REFIT.md')
  shutil.copyfile(source,out/item['name'])
  rows.append(str(item['resource'])+' RCDATA "stock-materials/'+item['name']+'"')
 (a.runtime/'stock_materials.rc').write_text('\n'.join(rows)+'\n')
 (out/'provenance.json').write_text(json.dumps(manifest,indent=2)+'\n')
 print('Validated and embedded '+str(len(rows))+' original stock material maps')
if __name__=='__main__':main()
