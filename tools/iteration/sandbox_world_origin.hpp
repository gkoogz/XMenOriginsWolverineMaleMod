#pragma once
// Adapter-only observation for the authored grey room, enabled exclusively by
// a launcher which checked its floor package hash. The floor is a StaticMesh,
// so the production baked-BSP origin observer cannot see it.
static void MaleModSandboxOrigin(IDirect3DDevice9* d,D3DPRIMITIVETYPE type,UINT nv,UINT count){
 static bool enabled=[](){char value[16]{};return GetEnvironmentVariableA("MALEMOD_SANDBOX_ORIGIN_V1",value,sizeof(value))==1&&value[0]=='1';}();
 if(!enabled||TankCameraSceneActive()||type!=D3DPT_TRIANGLELIST||nv!=40||count!=48||!IsFullResolutionScenePass(d))return;
 auto* layout=GetShaderLayout(d);if(!layout||layout->boneCount||layout->localCount!=4||!layout->viewValid)return;
 IDirect3DVertexBuffer9* vb=nullptr;UINT offset=0,stride=0;if(FAILED(d->GetStreamSource(0,&vb,&offset,&stride))||!vb)return;
 D3DVERTEXBUFFER_DESC desc{};bool measured=stride==12&&SUCCEEDED(vb->GetDesc(&desc))&&desc.Size==480&&offset==0;vb->Release();if(!measured)return;
 float local[16]{};if(FAILED(d->GetVertexShaderConstantF(layout->localRegister,local,4)))return;
 for(unsigned i=0;i<16;i++){if(!_finite(local[i]))return;if(i==12||i==13||i==14)continue;float expected=i==0||i==5?1000.f:i==10?.1f:i==15?1.f:0.f;if(fabsf(local[i]-expected)>1e-4f)return;}
 // Authoring recipe: actor (0,0,2000), scale (1000,1000,.1), no rotation.
 // Its measured LocalToWorld includes the engine's pre-view translation.
 fluidCameraWorld={-local[12],-local[13],2000.f-local[14]};fluidCameraTick=GetTickCount();fluidWorldCameraFrame=renderFrameSerial;
 static bool logged=false;if(!logged){logged=true;Log("Sandbox measured StaticMesh origin camera=(%.3f %.3f %.3f); hash-guarded authored floor",fluidCameraWorld.x,fluidCameraWorld.y,fluidCameraWorld.z);}
}
