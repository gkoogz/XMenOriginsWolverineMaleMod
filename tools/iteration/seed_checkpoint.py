"""Seed only an owned engine-generated Raven v568 developer profile.

The typed numeric recipe is a public synthetic reference state, not a shipped
save or imported player history. No unobserved field names/units are invented.
"""
import argparse,hashlib,json,pathlib,struct

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--workspace',type=pathlib.Path,required=True);args=ap.parse_args()
    root=args.workspace.resolve();p=root/'owned-game/WGame/SaveData/Player.wgameprofile'
    b=bytearray(p.read_bytes())
    if len(b)!=2748 or b[20:32]!=bytes.fromhex('0000abcd00000aa00000011c') or b[:20]!=hashlib.sha1(b[20:]).digest():raise SystemExit('Unsupported or invalid freshly generated native profile')
    backup=root/'engine-default-profile.wgameprofile'
    if backup.exists():raise SystemExit('Retained engine default exists; choose a fresh bootstrap')
    backup.write_bytes(b)
    values={27:2,37:13,38:13,204:0x08000000,500:53,1020:0}
    values.update({i:0x04000000 for i in range(501,543)})
    values.update({501:0x0e000000,502:0x0e000000,503:0x0c000000,518:0x0e000000})
    values.update({i:0x06000000 for i in range(536,541)})
    values.update({i:128 for i in range(1001,1008)})
    changes=[]
    for field,value in sorted(values.items()):
        marker=struct.pack('>I',field)+b'\x01'
        if b.count(marker)!=1:raise SystemExit('Unsupported typed field binding: '+str(field))
        at=b.index(marker)+5;before=bytes(b[at:at+4]);after=struct.pack('>I',value)
        b[at:at+4]=after;changes.append({'fieldID':field,'beforeBytes':before.hex(),'afterBytes':after.hex()})
    b[:20]=hashlib.sha1(b[20:]).digest();p.write_bytes(b)
    proof={'schema':1,'engineDefaultSHA256':hashlib.sha256(backup.read_bytes()).hexdigest(),'syntheticSHA256':hashlib.sha256(b).hexdigest(),'privateProfileCopied':False,'typedIntegerOverrides':changes,'fieldSemantics':'opaque observed reference control representation; no invented IDs/units','nativeGameplayObserved':False}
    (root/'profile-seed.json').write_text(json.dumps(proof,indent=2)+'\n');print(json.dumps(proof))
if __name__=='__main__':main()
