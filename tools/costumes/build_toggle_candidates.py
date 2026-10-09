"""Top/Bottom candidate reference crops; does not claim separated garments."""
import argparse
import base64
import io
import json
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont

TOP=[
 ('Button-up shirt','CH_Wolverine_Barfight','Rolled sleeves; extract shirt from combined clothes material'),
 ('Leather jacket','CH_Wolverine_Casino','Separate jacket section; tank underneath'),
 ('Field tank','CH_Wolverine_Jungle','Worn appearance variant of our existing tank'),
 ('Brown / yellow suit top','CH_Wolverine_BonusSkin1','Needs waist separation; mask and gloves shown'),
 ('Blue / yellow suit top','CH_Wolverine_BonusSkin2','Needs waist separation; mask and gloves shown'),
 ('X-Force suit top','CH_Wolverine_BonusSkin3','Needs waist separation; mask and gloves shown'),
]
BOTTOM=[
 ('Brown trousers + boots','CH_Wolverine_Barfight','Extract lower garment from combined clothes material'),
 ('Cargo trousers + boots','CH_Wolverine_Jungle','Separate pants section with field gear'),
 ('Brown / yellow suit bottoms','CH_Wolverine_BonusSkin1','Needs waist separation; includes tall boots'),
 ('Blue / yellow suit bottoms','CH_Wolverine_BonusSkin2','Needs waist separation; includes tall boots'),
 ('X-Force suit bottoms','CH_Wolverine_BonusSkin3','Needs waist separation; includes tall boots'),
]


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--catalog',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);args=ap.parse_args()
    catalog=json.loads(args.catalog.read_text(encoding='utf-8-sig'))
    bank={r['mesh']:r for r in catalog['outfits']}
    args.output.mkdir(parents=True,exist_ok=True)
    font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',27)
    small=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',19)
    title=ImageFont.truetype('C:/Windows/Fonts/segoeuib.ttf',40)
    data=[]
    for group,items in [('Top',TOP),('Bottom',BOTTOM)]:
        tile_height=500 if group=='Top' else 670
        sheet=Image.new('RGB',(1800,100+tile_height*2),(23,29,36));draw=ImageDraw.Draw(sheet)
        draw.text((24,14),group+' toggle candidates',font=title,fill='white')
        draw.text((24,65),'Original outfit reference crops - separation and enlarged-body refit pending',font=small,fill=(195,208,220))
        for i,(label,key,note) in enumerate(items):
            images={}
            for c in bank[key]['captures']:
                im=Image.open(c['path']).convert('RGB')
                crop=(0,65,900,655) if group=='Top' else (150,445,750,1045)
                im=im.crop(crop)
                buffer=io.BytesIO();im.save(buffer,format='WEBP',quality=94)
                images[c['view']]='data:image/webp;base64,'+base64.b64encode(buffer.getvalue()).decode()
                if c['view']=='front':
                    im=im.resize((600,393 if group=='Top' else 600),Image.Resampling.LANCZOS)
                    x=i%3*600;y=100+i//3*tile_height
                    sheet.paste(im,(x,y));draw.text((x+14,y+im.height+8),str(i+1)+'. '+label,font=font,fill='white')
                    # Notes wrap manually for readable list cards.
                    if group=='Top':draw.text((x+14,y+im.height+46),note.split(';')[0],font=small,fill=(195,208,220))
            data.append(dict(group=group,label=label,mesh=key,note=note,images=images,refitImplemented=False,garmentSeparationImplemented=False))
        sheet.save(args.output/(group.lower()+'-candidates.png'))
    js='''const data=DATA;let group='Top',view='front';function show(){const root=document.getElementById('cards');root.replaceChildren();for(const r of data.filter(x=>x.group===group)){const a=document.createElement('article'),img=document.createElement('img'),h=document.createElement('h2'),p=document.createElement('p');img.src=r.images[view];img.alt=r.label;img.onclick=()=>{document.getElementById('large').src=img.src;document.querySelector('dialog').showModal()};h.textContent=r.label;p.textContent=r.note;a.append(img,h,p);root.append(a)}}for(const b of document.querySelectorAll('[data-group]'))b.onclick=()=>{group=b.dataset.group;document.querySelectorAll('[data-group]').forEach(x=>x.classList.toggle('active',x===b));show()};for(const b of document.querySelectorAll('[data-view]'))b.onclick=()=>{view=b.dataset.view;document.querySelectorAll('[data-view]').forEach(x=>x.classList.toggle('active',x===b));show()};document.querySelector('dialog').onclick=e=>{if(e.target.tagName!=='IMG')document.querySelector('dialog').close()};show();'''.replace('DATA',json.dumps(data))
    document='''<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>Top and Bottom candidates</title><style>body{margin:0;background:#141b22;color:#e7edf4;font:16px system-ui}header,main{max-width:1450px;margin:auto;padding:24px}p{line-height:1.6;color:#bac9d8}nav{display:flex;gap:8px;flex-wrap:wrap;margin:14px 0}button{background:#253542;color:white;border:1px solid #638097;border-radius:8px;padding:12px 20px;cursor:pointer}.active{background:#3e657f}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(330px,1fr));gap:20px}article{border-radius:12px;background:#202c37;overflow:hidden}article img{width:100%;cursor:zoom-in}article h2,article p{margin:16px}article h2{font-size:22px}dialog{max-width:95vw;max-height:95vh;background:#18232c;color:white}dialog img{max-width:90vw;max-height:85vh}dialog::backdrop{background:#000c}</style><header><h1>New Top and Bottom candidates</h1><p>Actual game outfit meshes and textures, cropped to compare candidate garment regions. These are reference renders, not implemented mix-and-match outfits. Existing Naked, Tank Top, Jockstrap, Jeans and Jeans (open) are excluded; Field tank is explicitly a worn appearance variant.</p><nav><button class="active" data-group="Top">Top candidates</button><button data-group="Bottom">Bottom candidates</button></nav><nav><button class="active" data-view="front">Front</button><button data-view="oblique">Three-quarter</button><button data-view="back">Back</button></nav><p>A shared body-follow refit factor can reuse measured body bindings, while each garment retains its source ease and protected details. Full suits need authored waist boundaries before independent toggles can work. Stock mask/glove/boot details shown here are candidates too; no native implementation is implied.</p></header><main id="cards"></main><dialog><button>Close</button><img id="large"></dialog><script>'''+js+'</script>'
    (args.output/'toggle-candidates.html').write_text(document,encoding='utf-8')
    (args.output/'candidates.json').write_text(json.dumps(dict(schema='wolverine.toggle-candidates/1',topCandidates=[dict(label=l,mesh=k,note=n) for l,k,n in TOP],bottomCandidates=[dict(label=l,mesh=k,note=n) for l,k,n in BOTTOM],referenceOnly=True,refitImplemented=False,garmentSeparationImplemented=False),indent=2)+'\n')
    print('6 Top and5 Bottom candidates; offline reference crops only.')


if __name__=='__main__':main()
