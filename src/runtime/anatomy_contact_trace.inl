#pragma once
// Included inside MeridianAdapter. Request gating stays in the caller.
// These are current CPU solver values, not a native skinned-surface certificate.
static void CaptureAnatomyContactTrace(){
 char path[MAX_PATH]{};SiblingPath(path,"AnatomyContactFrame.json");FILE* file=nullptr;
 if(fopen_s(&file,path,"wb")||!file)return;
 auto number=[&](float value){if(std::isfinite(value))fprintf(file,"%.9g",double(value));else fputs("null",file);};
 auto point=[&](V3 value){fputc('[',file);number(value.x);fputc(',',file);number(value.y);fputc(',',file);number(value.z);fputc(']',file);};
 auto points=[&](const V3* values,int count){fputc('[',file);for(int i=0;i<count;i++){if(i)fputc(',',file);point(values[i]);}fputc(']',file);};
 auto values=[&](const float* data,int count){fputc('[',file);for(int i=0;i<count;i++){if(i)fputc(',',file);number(data[i]);}fputc(']',file);};
 V3 thighs[4];CollisionCapsules(thighs[0],thighs[1],thighs[2],thighs[3]);
 fprintf(file,"{\"schema\":1,\"renderFrame\":%ld,\"coordinateSpace\":\"adapter CPU pelvis-local coordinates\",\"unit\":null,\"nativeSkinnedSurfaceCertificate\":false,\"physicsState\":%d,\"solverReady\":%s,\"inputReady\":%s,\"motionCollisionBonesReady\":%s,\"collisionOverride\":%s,",long(renderFrameSerial),physicsState,pdReady?"true":"false",pdInputReady?"true":"false",motionCollisionBonesReady?"true":"false",collisionCapsuleOverride?"true":"false");
 fputs("\"root\":",file);point(ShaftRoot());fputs(",\"liveRootDirection\":",file);point(LiveRootDirection());
 fprintf(file,",\"rootDriveReady\":%s,\"rootDriveDegrees\":",rootDriveReady?"true":"false");number(rootDriveAngle);
 fputs(",\"rootDriveDegreesPerSecond\":",file);number(rootDriveVelocity);
 const float spring[]={shaftSpring.pitch,shaftSpring.yaw,shaftSpring.pitchVelocity,shaftSpring.yawVelocity};
 fputs(",\"springRadiansAndRadiansPerSecond\":",file);values(spring,4);
 fputs(",\"controlUI\":",file);values(sliderUI,7);fputs(",\"mappedControls\":",file);values(sliderValues,7);
 fputs(",\"physicsUI\":",file);values(physUI,8);fputs(",\"mappedPhysics\":",file);values(physValues,8);
 fputs(",\"glansUI\":",file);number(glansUI);fputs(",\"hangUI\":",file);number(hangUI);
 fputs(",\"constraintRestLength\":",file);number(constraintRestLength);
 fputs(",\"logicalShaftBodyRadius\":",file);number(logicalShaftBodyRadius);
 fputs(",\"pelvisCollarGrowthFactor\":",file);number(PelvisCollarGrowth());
 fputs(",\"actualCollarSurfaceRadius\":null,\"shaftNodes\":",file);points(shaftNodes,shaftNodeCount);
 fputs(",\"solverPositions\":",file);if(pdReady)points(pdPosition,pdCount);else fputs("null",file);
 fputs(",\"solverVelocities\":",file);if(pdReady)points(pdVelocity,pdCount);else fputs("null",file);
 fputs(",\"returnedThighEndpoints\":",file);points(thighs,4);
 fputs(",\"solverThighEndpoints\":",file);if(pdReady)points(pdThigh,4);else fputs("null",file);
 fputs(",\"previousSolverThighEndpoints\":",file);if(pdReady)points(pdOldThigh,4);else fputs("null",file);
 fputs(",\"inputThighEndpoints\":",file);if(pdInputReady)points(pdInput.thigh,4);else fputs("null",file);
 fputs(",\"sourceDefinedContacts\":{\"solverThighRadius\":7.2,\"pelvisLobeOnly\":{\"a\":[3,0,70],\"b\":[5.4,0,86],\"radius\":6.4}},\"returnedEndpointOrigin\":\"CollisionCapsules output; bone-ready alone does not certify accepted bone transforms\"}",file);
 fclose(file);
}
