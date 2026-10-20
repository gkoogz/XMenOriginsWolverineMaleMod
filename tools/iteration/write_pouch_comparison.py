"""Embed private native captures and paired CPU evidence in a standalone report."""
import argparse
import base64
import html
import json
from pathlib import Path
import re
import statistics


def average(proof,label):
    return statistics.mean(t[label]['meanMs'] for t in proof['trials'])


def native(directory):
    text=(directory.parent/'runtime.log').read_text(errors='replace')
    text=text.split('Private gameplay receipt frame=1 ',1)[-1]
    values=re.findall(r'Meridian preparation allCalls=120 rejected=(\d+) duplicateHooks=(\d+) mean=([\d.]+)ms p95=([\d.]+)ms p99=([\d.]+)ms max=([\d.]+)ms',text)
    return dict(windows=len(values),meanMs=statistics.mean(float(v[2]) for v in values) if values else None,rejected=sum(int(v[0]) for v in values),duplicateHooks=sum(int(v[1]) for v in values),maxMs=max((float(v[5]) for v in values),default=0))


def main():
    ap=argparse.ArgumentParser(__doc__)
    ap.add_argument('--optimized',type=Path,required=True)
    ap.add_argument('--loft',type=Path,required=True)
    ap.add_argument('--optimization-proof',type=Path,required=True)
    ap.add_argument('--loft-proof',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    p=json.loads(args.optimization_proof.read_text());q=json.loads(args.loft_proof.read_text())
    timings={'Previous solver':average(p,'baseline'),'Optimized solver':average(p,'candidate'),'Oval-section prototype':average(q,'candidate')}
    a=json.loads((args.optimized/'comparison.json').read_text());b=json.loads((args.loft/'comparison.json').read_text())
    images={}
    for key,directory in [('optimized',args.optimized),('loft',args.loft)]:
        images[key]={f.stem:'data:image/png;base64,'+base64.b64encode(f.read_bytes()).decode() for f in directory.glob('*.png')}
    common=set(images['optimized'])&set(images['loft'])
    if len(common)!=12:
        raise SystemExit(f'Expected 12 paired native views, found {len(common)}')
    native_data={'Optimized solver':native(args.optimized),'Oval-section prototype':native(args.loft)}
    bars=''.join(f'<div class="barrow"><span>{name}</span><div class="track"><div class="bar" style="width:{100*value/max(timings.values()):.2f}%"></div></div><strong>{value:.2f} ms</strong></div>' for name,value in timings.items())
    runtime_rows=''.join(f'<tr><td>{name}</td><td>{v["meanMs"]:.2f} ms</td><td>{v["maxMs"]:.2f} ms</td><td>{v["rejected"]}</td><td>{v["duplicateHooks"]}</td><td>{v["windows"]}</td></tr>' for name,v in native_data.items())
    template='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Jockstrap performance and shape comparison</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#f5f4f0;color:#222b32;font:16px/1.55 system-ui,sans-serif}main{max-width:1260px;margin:auto;padding:36px 24px}h1{font-size:34px;line-height:1.2;margin:8px 0 18px}h2{font-size:23px;margin-top:34px}h3{font-size:18px;margin:0 0 8px}p{max-width:95ch}.lede{font-size:19px}.muted{color:#58636b}.badge{display:inline-block;background:#e4e9e4;padding:4px 10px;border-radius:5px;font-size:13px}.warning{background:#fff2cf;padding:16px 20px;border-radius:8px}.barrow{display:grid;grid-template-columns:210px 1fr 110px;gap:15px;align-items:center;margin:15px 0}.track{height:24px;background:#e6e8e8}.bar{height:100%;background:#477e86;min-width:3px}strong{font-weight:650}.grid{display:grid;grid-template-columns:1fr 1fr;gap:18px}.card{background:white;border-radius:10px;padding:18px}.imagebox{overflow:hidden;background:#999;aspect-ratio:4/3;border-radius:5px}.imagebox img{width:100%;display:block;transform:scale(1.85);transform-origin:50% 54%}.full .imagebox img{transform:none}.controls{display:flex;gap:12px;align-items:center;flex-wrap:wrap;margin:14px 0}select,button{font:inherit;padding:8px 12px;background:white;border:1px solid #a8b1b6;border-radius:6px}button[aria-pressed=true]{background:#263b42;color:white}.caption{margin:8px 0 0;color:#58636b;font-size:14px}table{border-collapse:collapse;width:100%;background:white}th,td{text-align:left;padding:11px 12px;border-bottom:1px solid #e2e5e4;vertical-align:top}th{background:#e6eae8}code{overflow-wrap:anywhere;font-size:13px}details{margin:20px 0}summary{cursor:pointer}a{color:#185e75}footer{margin-top:36px;color:#58636b;font-size:14px}@media(max-width:750px){.grid{grid-template-columns:1fr}.barrow{grid-template-columns:145px 1fr 90px;gap:8px}main{padding:22px 14px}h1{font-size:27px}.scroll{overflow:auto}}
</style><main>
<span class="badge">Native engine captures • October 10, 2026 • No retail installation</span>
<h1>A fitted pouch at a practical CPU cost</h1>
<p class="lede">The existing solver is substantially cheaper after exact-work optimizations. My new oval-section prototype demonstrates a much lower CPU cost, but fails the visual comparison: side views reveal ribs and multiple protruding lobes. It is rejected as a replacement.</p>
<div class="warning"><strong>These are development comparisons.</strong> Faster geometry generation does not establish a finished garment. Neither candidate has passed the complete attachment, LOD and campaign gate. The installed game remains unchanged.</div>
<h2>What went wrong, and what changed</h2><p>Your suspicion was partly right. The jaggedness repair added fairing even to followed surfaces. Collision density also grew from 1,504 to 5,056 triangles, while repeated projection, exact support searches and clearance scans multiplied rebuild cost. A failed preparation could run again in another draw hook; the old successful-draw timer hid rejected work. The retail log showed 425 ms mean garment Draw, but that historical log is not directly comparable to the controlled rebuild test below.</p>
<p>The implemented shared fixes reuse identical support calculations, defer a seed scan until it is needed, and stop certification at the first failing triangle. The adapter skips duplicate failed solves only when the frame, controls and actual posed inputs match, and records preparation failures as well as successes. Existing current-source fixes that remove follow fairing and futile retries are retained. This work preserves the old solver's output; it does not repair its garment silhouette.</p>
<h2>Measured CPU savings</h2><p>Same bank of 14 captured 64×40 poses, x86 MSVC /O2, one warm-up per pose, three evaluations per pose and three alternating trial pairs. The final replays ran after closing the owned game. Times include support preparation and a fresh fit/rebuild; they exclude donor skinning, drawing, GPU time and the rest of the game.</p>
@BARS@
<p><strong>@REDUCTION@% less rebuild CPU time</strong> for the optimized solver versus the preceding source baseline. Vertices, receipts and rejection messages match byte for byte. The prototype changes the surface and reduces fitting time by <strong>@LOFTREDUCTION@%</strong> against its paired optimized baseline.</p>
<p class="muted">The source baseline already contains earlier improvements; it is not a paired replay of the entire installed DLL. Five of the 14 poses remain rejected by the old and optimized rebuild path. The prototype produces a surface for all 14, but it does not run the same collision certificate—this is not proof of better collision correctness. Its 16 fitted sections still generate the same 5,056 display triangles.</p>
<h2>Native views</h2>
<p>Compare the front, oblique and side silhouettes, the waist connection and the underside. Both runs use 1280×960 and the same timed native camera/control recipe. Animation phases and camera angles are approximate matches, not registered image pairs. Enlarging uses CSS only; original screenshots are embedded intact.</p>
<div class="controls"><label>Case <select id="case"><option value="default">Default: Overall 50 / Width 50</option><option value="large">Large: Overall 86 / Width 100</option><option value="maximum">Maximum: Overall 100 / Width 100</option><option value="firm">Firm state, default size</option><option value="motion">After walking, default size</option><option value="attachment">Naked attachment reference</option></select></label><span id="angles"><button data-view="front" aria-pressed="true">Front</button><button data-view="oblique" aria-pressed="false">Oblique</button><button data-view="side" aria-pressed="false">Side</button></span><label><input id="full" type="checkbox"> Full frame</label></div>
<div class="grid" id="views"><article class="card"><h3>Optimized walking solver</h3><div class="imagebox"><img id="optimized" alt="Native optimized walking garment"></div><p class="caption" id="optimized-caption"></p></article><article class="card"><h3>New oval-section prototype</h3><div class="imagebox"><img id="loft" alt="Native experimental oval-section garment"></div><p class="caption" id="loft-caption"></p></article></div>
<h2>What the comparison shows</h2>
<div class="scroll"><table><thead><tr><th>Aspect</th><th>Optimized walking solver</th><th>Oval-section prototype</th></tr></thead><tbody>
<tr><td>Construction</td><td>Walk, full contact projection and certification; valid poses follow the last accepted surface.</td><td>Fit 16 elliptical sections around current support samples and join them to the unchanged sewn edge.</td></tr>
<tr><td>CPU cost</td><td>Large improvement, but rebuilds still cost tens of milliseconds.</td><td>Roughly a millisecond for fitting in the paired replay; no iterative global search.</td></tr>
<tr><td>Frame budget</td><td>A 36 ms rebuild alone exceeds a 60 FPS frame budget of 16.7 ms. Savings are substantial but do not establish playable cadence.</td><td>A 1.07 ms fit uses about 6% of that budget before skinning, rendering and other game work. Visual defects prevent adoption.</td></tr>
<tr><td>Front</td><td>Retains the existing broad hanging panel; optimizing arithmetic does not improve that silhouette.</td><td>Rounder lower pouch, but still overly long. The front alone hides serious side-view defects.</td></tr>
<tr><td>Side and oblique</td><td>Shows the wedge/flap depth and the existing sharp transition from waistband to pouch.</td><td>Fails: multiple protruding lobes and ribbed transitions. Regularizing column order did not resolve the underlying sectional shape problem.</td></tr>
<tr><td>Large controls</td><td>Can become an apron-like panel and retains rejection paths.</td><td>Fails: excessive volume and stacked disc-like forms are especially obvious at maximum size.</td></tr>
<tr><td>Contact assurance</td><td>Preserves the existing analytic-support certificate and its limits; this is not proof of contact with every rendered body surface.</td><td>Uses sampled anatomical bounds and ease. Seam blending and interpolation are not continuously collision-certified.</td></tr>
<tr><td>Attachment</td><td>Existing source/binding behavior retained; complete visual gate remains unpassed.</td><td>Sewn vertex positions stay exact in the source test, but triangle orientation, shading, clipping and recruited attachment still need acceptance.</td></tr>
<tr><td>Motion</td><td>May alternate follow, rebuild and rejection. Duplicate failed hook work is now suppressed for identical posed inputs.</td><td>Stateless fit follows current samples with no integrated lag. Brief native movement was captured; longer movement and all LODs are unverified.</td></tr>
</tbody></table></div>
<h2>Native preparation windows</h2><p>Logged 120-call windows after the gameplay marker, including changing controls, failures and duplicate-hook input sampling. These are descriptive CPU timings, not a controlled FPS comparison. A duplicate hook is a skipped repeat of the same failed solve, not another completed garment.</p>
<div class="scroll"><table><thead><tr><th>Candidate</th><th>Mean</th><th>Worst call</th><th>Rejected calls</th><th>Skipped duplicates</th><th>Windows</th></tr></thead><tbody>@NATIVE@</tbody></table></div>
<h2>Recommendation</h2><p><strong>Keep the exact-work optimizations; do not adopt this prototype.</strong> It demonstrates that fitting can be inexpensive, but its surface is visibly unacceptable. Independently enclosing each anatomical slice is a poor garment shape rule. My proposed next design is a small, smooth, pre-shaped pouch cage, deformed by a few anatomical landmarks and corrected locally for contact. That is a design recommendation, not an implemented or validated third candidate.</p>
<p>The next contact work should operate on the small cage. A dense display mesh can remain smooth without becoming thousands of independently solved collision constraints. No FPS promise follows from these cloth timings: the remaining anatomy and game work still consume frame time.</p>
<details><summary>Build identities, scope and verification</summary><p>Optimized native DLL: <code>@AHASH@</code><br>Prototype native DLL: <code>@BHASH@</code></p><p>Optimized Base pin: <code>bd7c4a77387b3f340c1ee2b3aa4a63300519d922</code>, local branch <code>codex/jockstrap-audit-perf</code>. The experimental loft is separate and not selected by the normal adapter. Relevant C++ tests, exact-pose replay and meridian hashes passed. Full Base verification still stops at pre-existing motion/CMake provenance drift. The sandbox source manifest also has a pre-existing sealed_d3d9.hpp hash mismatch, confirmed in committed HEAD. Campaign and supported LOD coverage are incomplete.</p><p>Local report data and screenshots are private generated evidence. No captured assets, settings or game packages were committed. No retail installation, push or release was performed. Both owned runs closed; the reused private room DLL was restored to its prior hash.</p></details>
<footer>Self-contained report: screenshots and comparison data are embedded; no internet, folder selection or server required. Native screenshots were visually reviewed. Automated browser preview of the local HTML was blocked by the browser file-URL policy; the report structure, image payloads and case mappings were checked from source.</footer>
</main><script>
const images=@IMAGES@;let view='front';const chooser=document.getElementById('case');
function draw(){let c=chooser.value;const special={firm:'firm-front',motion:'motion-front',attachment:'attachment-naked'};let key=special[c]||`${c}-${view}`;document.getElementById('angles').style.display=special[c]?'none':'inline';for(const variant of ['optimized','loft']){document.getElementById(variant).src=images[variant][key];document.getElementById(variant+'-caption').textContent=key.replaceAll('-',' ')}document.getElementById('views').classList.toggle('full',document.getElementById('full').checked)}
chooser.onchange=draw;document.getElementById('full').onchange=draw;document.querySelectorAll('[data-view]').forEach(b=>b.onclick=()=>{view=b.dataset.view;document.querySelectorAll('[data-view]').forEach(x=>x.setAttribute('aria-pressed',String(x===b)));draw()});draw();
</script></html>'''
    replacements={'@BARS@':bars,'@REDUCTION@':f'{p["meanReductionPercent"]:.1f}','@LOFTREDUCTION@':f'{q["meanReductionPercent"]:.1f}','@NATIVE@':runtime_rows,'@AHASH@':html.escape(a['runtimeSHA256']),'@BHASH@':html.escape(b['runtimeSHA256']),'@IMAGES@':json.dumps(images)}
    for key,value in replacements.items():template=template.replace(key,value)
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(template,encoding='utf-8')
    args.output.with_suffix('.json').write_text(json.dumps(dict(timings=timings,native=native_data,optimized=a,loft=b,optimization=p,loftComparison=q),indent=2))
    print(args.output)


if __name__=='__main__':
    main()
