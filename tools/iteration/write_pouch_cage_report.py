"""Create a self-contained comparison from original native PNGs and replay data."""
import argparse
import base64
import hashlib
import html
import json
from pathlib import Path
import statistics

def main():
    ap=argparse.ArgumentParser(__doc__)
    for name in ['optimized','cage','motion','matrix','replay','runtime','timing','output']:
        ap.add_argument('--'+name,type=Path,required=True)
    a=ap.parse_args();proof=json.loads((a.replay/'comparison.json').read_text(encoding='utf-8'))
    before=statistics.mean(t['baseline']['meanMs'] for t in proof['trials'])
    after=statistics.mean(t['candidate']['meanMs'] for t in proof['trials'])
    assets={};hashes={}
    for name,path in [('walker',a.optimized),('pouch',a.cage),('motion',a.motion),('matrix',a.matrix)]:
        assets[name]={}
        for p in sorted(path.glob('*.png')):
            b=p.read_bytes()
            if not b.startswith(b'\x89PNG\r\n\x1a\n'):raise ValueError('Expected native PNG: '+str(p))
            assets[name][p.stem]='data:image/png;base64,'+base64.b64encode(b).decode()
            hashes[str(p)]=hashlib.sha256(b).hexdigest()
    keys=sorted(assets['walker'].keys() & assets['pouch'].keys())
    if not keys or not assets['motion']:raise ValueError('Incomplete native comparison')
    runtime=json.loads(a.runtime.read_text(encoding='utf-8'));timing=json.loads(a.timing.read_text(encoding='utf-8'))
    page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Jockstrap pouch implementation and comparison</title>
<style>body{font:16px/1.55 system-ui,sans-serif;background:#111820;color:#eef2f6;max-width:1240px;margin:auto;padding:28px}h1{font-size:32px;line-height:1.2}h2{margin-top:38px}p{max-width:90ch}small,.muted{color:#bcc8d4}strong{color:#fff}.metrics,.pair{display:grid;grid-template-columns:1fr 1fr;gap:18px}.metric{padding:20px;background:#1e2a37;border-radius:10px}.metric b{display:block;font-size:30px;color:#85dfbc}table{width:100%;border-collapse:collapse}td,th{padding:12px;text-align:left;vertical-align:top;border-bottom:1px solid #3a4654}select,button{font:inherit;padding:8px;margin:4px;background:#25384b;color:white;border:1px solid #6b7c90;border-radius:5px}figure{margin:0}img{display:block;width:100%;height:auto;background:#999}figcaption{padding:8px 0}code{overflow-wrap:anywhere}a{color:#9ad8ff}.note{border-left:4px solid #e8b569;padding:12px 18px;background:#282b31}@media(max-width:720px){.pair,.metrics{grid-template-columns:1fr}body{padding:15px}table{font-size:14px}}</style>
<h1>Jockstrap pouch implementation and comparison</h1>
<p>The new pouch uses a small pre-shaped cage with local contact corrections. It keeps one continuous silhouette instead of walking and repeatedly repairing thousands of cloth triangles. The fixed waistband, straps, material coordinates and sewn boundary remain.</p>
<div class="metrics"><div class="metric"><b>@AFTER@ ms</b>mean offline fitting cost, down from @BEFORE@ ms for the optimized walker. @SAVING@% less CPU work in the paired replay.</div><div class="metric"><b>@FPS@ FPS</b>measured settled private room cadence (@FRAME@ ms mean) with the complete optimization package. This is not a 60 FPS or retail campaign claim.</div></div>
<p class="note">Implemented and built; retail has not been replaced. The new fit is approximate. The pouch remained visible in the reviewed room samples. The contact audit covers sampled interior supports only. Complete attachment, all LODs, campaign transitions and human play feel remain unverified; native moving shadow bands remain visible. A flat-color diagnostic showed attenuation after the cloth shader; no geometry smoothing or shadow-removal workaround is used to hide it.</p>
<h2>Compare the same view</h2><label>Size and angle <select id="view"></select></label>
<div class="pair"><figure><img id="old" alt="Optimized walking solver native capture"><figcaption>Optimized walking solver</figcaption></figure><figure><img id="new" alt="New pre-shaped pouch native capture"><figcaption>New pre-shaped pouch</figcaption></figure></div>
<p class="muted">These are original native room captures. Camera commands and requested controls match, but animation phase and camera positions are not pixel-aligned.</p>
<h2>All captured angles and movement</h2><label>Sequence <select id="group"><option value="pouch">Pouch views and orbit</option><option value="motion">Walking sprint and jump samples</option><option value="matrix">Size, state and angle matrix</option></select></label><label>Capture <select id="extra"></select></label><button id="previous">Previous</button><button id="next">Next</button><figure><img id="detail" alt="Selected native pouch review capture"><figcaption id="caption"></figcaption></figure>
<p class="muted">Movement samples were taken during held engine input. They are individual frames, not continuous video or proof that animation feels smooth. The zoom command produced no meaningful close-up.</p>
<h2>Why the old approach became expensive</h2><p>The jagged-shape repair added repeated global collision checks and fairing, while the cloth collision mesh grew from 1,504 to 5,056 triangles. The solver could rebuild up to six times and then enter another fallback. The new pouch avoids that repair loop: it starts with a coherent cap and adjusts a small control cage. A separate graphics-buffer readback stall also needed fixing.</p>
<h2>Comparison from several angles</h2>
<table><thead><tr><th>Aspect</th><th>Optimized walker</th><th>New pouch</th></tr></thead><tbody>
<tr><td>Work per fit</td><td>Repeated walks, chart transforms, projection, fallback and whole-triangle contact checks.</td><td>408 cage controls receive local sample corrections, then interpolate the existing render mesh.</td></tr>
<tr><td>Shape</td><td>Attempts close conformity; fairing and fallback can create jagged or distinct lobes.</td><td>One coherent pouch bridges the contents. Less anatomical detail and some extra fabric volume are intentional.</td></tr>
<tr><td>Contact</td><td>Whole-triangle acceptance when its certification succeeds; only 9 of these 14 inputs passed that route.</td><td>All 14 inputs emit a surface; zero outside proxy samples in the audited interior. This is a weaker, sampled guarantee, not equivalent certification.</td></tr>
<tr><td>Seam and shading</td><td>Retains the measured bindings and aliases.</td><td>Sewn positions stay exact. UV aliases and straps are retained. A fixed outward seam orientation prevents the cap flipping into the body during a stride. Original material lighting is retained. An unlit-color diagnostic distinguished later native attenuation from missing cloth; the speculative local-shadow bypass was removed.</td></tr>
<tr><td>Motion and controls</td><td>Full fine skin evaluation every frame; contents simulation at 240 Hz.</td><td>Contents at 120 Hz while covered, with the same 24 contact iterations. Fine skin deformation updates on alternate rendered frames; controls and active clinical sequences bypass this cadence. Skeletal animation and cloth drawing continue every frame.</td></tr>
<tr><td>Other savings</td><td>Repeated support queries and in-place mapped-buffer evaluation.</td><td>Exact-input support cache, bit-identical Newton fixed-point exit, and a persistent CPU geometry copy. Only owned positions and shading bytes are published; native UVs and weights remain untouched. This matched 72 complete replay frames byte-for-byte and removed the repeated mapped-buffer readback.</td></tr>
<tr><td>Rejected experiments</td><td colspan="2">The earlier oval loft was faster but had visible lobes and ribs. Removing hidden skin detail changed exposed skin by up to 2.99 source units. Serial geometry scheduling was slower. None of those shortcuts is adopted.</td></tr>
</tbody></table>
<h2>Evidence and limits</h2>
<p>The paired fit test uses 14 measured poses, three repetitions per pose and three alternating baseline/candidate runs. Native preparation includes input transforms, proxy construction, fitting, normals and upload, and is roughly 3 ms. These fitting figures are not whole-frame timing.</p>
<p>The settled timing sample is taken after the view, movement and control sweeps, with default controls restored. This measures whole-frame cadence over @FRAMES@ frames, separately from the paired fitting benchmark. The 120-frame means ranged from @MIN@ to @MAX@ ms; these are not individual-frame percentiles. The old slow prototype reached roughly 52-60 ms per frame; that is a separate native run, not a controlled paired comparison.</p>
<p>The contact audit checks cross-section polygons against current proxy samples and excludes the first two sewn display rows. It does not certify every cloth triangle against the complete rendered body. Physics replay covered all three states, minimum/default/maximum size and width, and four frame intervals; all recorded values remained finite and the recorded lobe gaps stayed positive.</p>
<p>Source tests and the new provenance check pass. Full repository verification still reports existing motion extraction/provenance drift, and the sandbox recipe checker reports the existing sealed factory hash mismatch. Those unrelated failures were not hidden by refreshing their hashes.</p>
<details><summary>Normal build identity and evidence hashes</summary><p>DLL SHA256 <code>@DLL@</code></p><p>Base revision <code>@BASE@</code></p><p>Test-only native DLL SHA256 <code>@NATIVE@</code>; run <code>@RUN@</code>. The sealed harness inherits an earlier source baseline with the named current garment/physics/buffer overlays; this is not complete binary parity of the normal DLL across campaign behavior.</p><p>Source commit plus explicitly recorded overlays: <code>@SOURCE@</code></p><pre id="proof" style="white-space:pre-wrap;overflow-wrap:anywhere"></pre></details>
<script>const images=@IMAGES@,hashes=@HASHES@,keys=@KEYS@;const q=id=>document.getElementById(id);function options(e,values){e.replaceChildren(...values.map(v=>{let o=document.createElement('option');o.value=v;o.textContent=v.replaceAll('-',' ');return o}))}options(q('view'),keys);q('view').value=keys.includes('default-side')?'default-side':keys[0];function pair(){q('old').src=images.walker[q('view').value];q('new').src=images.pouch[q('view').value]}q('view').onchange=pair;pair();function detail(){q('detail').src=images[q('group').value][q('extra').value];q('caption').textContent=q('extra').value.replaceAll('-',' ')}function group(){options(q('extra'),Object.keys(images[q('group').value]));detail()}q('group').onchange=group;q('extra').onchange=detail;for(let [id,d] of [['previous',-1],['next',1]])q(id).onclick=()=>{let e=q('extra');e.selectedIndex=(e.selectedIndex+d+e.options.length)%e.options.length;detail()};group();q('proof').textContent=JSON.stringify(hashes,null,2);</script></html>'''
    for key,value in {'FRAMES':str(timing['frames']),'MIN':f"{timing['windowMinMs']:.2f}",'MAX':f"{timing['windowMaxMs']:.2f}",'NATIVE':timing['runtimeSHA256'],'RUN':timing['run'],'FPS':f"{1000/timing['settledMeanMs']:.1f}",'FRAME':f"{timing['settledMeanMs']:.2f}",'AFTER':f'{after:.2f}','BEFORE':f'{before:.2f}','SAVING':f'{(1-after/before)*100:.1f}','DLL':runtime['runtimeSHA256'],'BASE':runtime['baseCommit'],'SOURCE':runtime['sourceCommit'],'IMAGES':json.dumps(assets),'HASHES':json.dumps(hashes),'KEYS':json.dumps(keys)}.items():
        page=page.replace('@'+key+'@',value)
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(page,encoding='utf-8')
    (a.output.with_suffix('.json')).write_text(json.dumps(dict(imageCount=len(hashes),hashes=hashes,fitBeforeMs=before,fitAfterMs=after,runtime=runtime,timing=timing,previewVerified=False),indent=2)+'\n')
    print(f'Wrote {a.output}; {len(hashes)} original embedded PNGs; browser preview not verified')

if __name__=='__main__':main()
