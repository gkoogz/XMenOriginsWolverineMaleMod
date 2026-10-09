"""Make labeled sheets and a self-contained gallery from private stock renders."""
import argparse
import base64
import hashlib
import html
import io
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--renders',type=Path,nargs='+',required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    by_mesh={}
    for folder in args.renders:
        receipt=json.loads((folder/'renders.json').read_text(encoding='utf-8-sig'))
        for record in receipt['outfits']:
            for c in record['captures']:
                assert sha(Path(c['path']))==c['sha256']
                assert Image.open(c['path']).size==(900,1100)
            by_mesh[record['mesh']]=record
    # Later directories replace an earlier low-mip or diagnostic iteration.
    records=list(by_mesh.values())
    main=[r for r in records if r['mainOutfit']]
    extra=[r for r in records if not r['mainOutfit']]
    main.sort(key=lambda r:('BonusSkin' in r['mesh'],r['mesh']))
    font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',25)
    title=ImageFont.truetype('C:/Windows/Fonts/segoeuib.ttf',36)
    def sheet(items,name,heading,cols=4):
        rows=(len(items)+cols-1)//cols
        result=Image.new('RGB',(cols*450,95+rows*615),(23,29,36))
        draw=ImageDraw.Draw(result)
        draw.text((24,18),heading,font=title,fill='white')
        draw.text((24,62),'Stock meshes and textures | neutral offline lighting | not refitted',font=font,fill=(195,208,220))
        for i,r in enumerate(items):
            capture=next(c for c in r['captures'] if c['view']=='front')
            image=Image.open(capture['path']).convert('RGB').resize((450,550),Image.Resampling.LANCZOS)
            x=i%cols*450;y=95+i//cols*615
            result.paste(image,(x,y))
            label=r['label']
            if not r['mainOutfit']:
                label={'CH_Wolverine_BonusSkin2_test':'Bonus 2 test mesh','wolverine_blueandyellow':'Blue/yellow draft mesh','WolverineJungleNOCLAWSMesh':'Jungle - no claws','WeaponXHelmetMesh_DONOTUSE':'Unused Weapon X helmet'}.get(r['mesh'],label)
            draw.text((x+12,y+560),label,font=font,fill='white')
        result.save(args.output/name)
    sheet([r for r in main if 'BonusSkin' not in r['mesh']],'campaign-outfits.png','Campaign character looks',3)
    sheet([r for r in main if 'BonusSkin' in r['mesh']],'bonus-outfits.png','Bonus suits',3)
    sheet(extra,'auxiliary-meshes.png','Auxiliary assets - not extra playable outfits',4)
    css='''body{margin:0;background:#141b22;color:#e7edf4;font:16px system-ui}header,main{max-width:1400px;margin:auto;padding:24px}h1{font-size:32px}p{line-height:1.6;color:#b9c8d6}nav{display:flex;gap:8px;flex-wrap:wrap}button{border:1px solid #54697c;background:#263442;color:white;padding:10px 18px;border-radius:8px;cursor:pointer}button.active{background:#375d79}#cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(285px,1fr));gap:18px}article{background:#202c37;border-radius:12px;overflow:hidden}article img{width:100%;display:block;cursor:zoom-in}article h2{font-size:20px;margin:16px}article small{display:block;margin:0 16px 16px;color:#b9c8d6}dialog{background:#18232c;border:1px solid #789;color:white;max-width:95vw;max-height:95vh}dialog img{max-width:90vw;max-height:85vh}dialog::backdrop{background:#000c}'''
    gallery=[]
    for r in records:
        item=dict(mesh=r['mesh'],label=r['label'],main=r['mainOutfit'],images={})
        for c in r['captures']:
            buffer=io.BytesIO()
            Image.open(c['path']).convert('RGB').save(buffer,format='WEBP',quality=92)
            item['images'][c['view']]='data:image/webp;base64,'+base64.b64encode(buffer.getvalue()).decode()
        gallery.append(item)
    js='''const data=DATA;let view='front',extras=false;const cards=document.getElementById('cards');function show(){cards.replaceChildren();for(const r of data.filter(r=>extras?!r.main:r.main)){const a=document.createElement('article'),img=document.createElement('img'),h=document.createElement('h2'),s=document.createElement('small');img.src=r.images[view]||r.images.front;img.alt=r.label+' '+view;h.textContent=r.label;s.textContent=r.mesh+(r.main?'':' | auxiliary / test asset');img.onclick=()=>{document.getElementById('large').src=img.src;document.querySelector('dialog').showModal()};a.append(img,h,s);cards.append(a)}}for(const b of document.querySelectorAll('[data-view]'))b.onclick=()=>{view=b.dataset.view;document.querySelectorAll('[data-view]').forEach(x=>x.classList.toggle('active',x===b));show()};document.getElementById('extras').onclick=()=>{extras=!extras;document.getElementById('extras').classList.toggle('active',extras);show()};document.querySelector('dialog').onclick=e=>{if(e.target.tagName!=='IMG')document.querySelector('dialog').close()};show();'''.replace('DATA',json.dumps(gallery))
    document='<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>Wolverine stock wardrobe</title><style>'+css+'</style><header><h1>Wolverine stock wardrobe</h1><p>'+str(len(main))+' primary outfit meshes. Actual exported stock geometry and UV textures, in bind pose under the same neutral lighting. These are offline reference renders: no enlarged-body refit, UE3 material parity, animation or gameplay acceptance is claimed.</p><nav><button class="active" data-view="front">Front</button><button data-view="oblique">Three-quarter</button><button data-view="back">Back</button><button id="extras">Auxiliary assets</button></nav><p>Click an image to enlarge. Auxiliary assets include two 2-bone draft meshes and an unused helmet; their presence does not prove they are playable costumes. The no-claws Jungle variation retains the normal character skeleton.</p></header><main id="cards"></main><dialog><button>Close</button><img id="large"></dialog><script>'+js+'</script>'
    (args.output/'outfit-gallery.html').write_text(document,encoding='utf-8')
    (args.output/'catalog.json').write_text(json.dumps(dict(schema='wolverine.stock-wardrobe-catalog/1',mainLooks=len(main),auxiliaryMeshes=len(extra),captures=sum(len(r['captures']) for r in records),offline=True,refitApplied=False,nativeVerified=False,outfits=records),indent=2)+'\n')
    print('Catalog:',len(main),'main looks,',len(extra),'auxiliary meshes,',sum(len(r['captures']) for r in records),'review renders')


if __name__=='__main__':main()
