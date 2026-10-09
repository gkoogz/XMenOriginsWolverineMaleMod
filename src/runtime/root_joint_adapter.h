#pragma once
#include <malemod/surface/root_contact.hpp>
// Development opt-in. The current observed capsule output must also be valid.
// Normal play preserves the reference kinematic first-link branch.
static bool RootJointRequested(){
 static bool requested=[](){char value[8]{};return GetEnvironmentVariableA("MALEMOD_ROOT_CONTACTS",value,sizeof(value))==1&&value[0]=='1';}();
 return requested;
}
static bool rootJointContactsReady=false;
