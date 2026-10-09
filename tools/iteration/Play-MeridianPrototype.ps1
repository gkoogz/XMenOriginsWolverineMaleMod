param(
 [string]$Workspace=(Join-Path $env:LOCALAPPDATA 'MaleMod/MeridianPrototype'),
 [switch]$ValidateOnly
)
$ErrorActionPreference='Stop'
# Explicitly unaccepted developer play. This never stages into retail or the
# accepted human sandbox, and never changes Play-NativeSandbox's release gate.
$root=(Resolve-Path -LiteralPath $Workspace).Path
$manifest=Get-Content -LiteralPath (Join-Path $root 'prototype.json') -Raw | ConvertFrom-Json
if($manifest.schema -ne 1 -or $manifest.accepted -ne $false -or $manifest.mode -ne 'meridian-developer-play') {throw 'Not an identified meridian developer prototype.'}
$game=Join-Path $root 'owned-game'
$exe=Join-Path $game 'Binaries/Wolverine.exe'
foreach($property in $manifest.contract.PSObject.Properties){
 $file=[IO.Path]::GetFullPath((Join-Path $game $property.Name))
 if(!$file.StartsWith($game+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Invalid prototype contract path.'}
 if((Get-FileHash -LiteralPath $file).Hash -ne $property.Value){throw "Prototype identity changed: $($property.Name)"}
}
foreach($required in @('Binaries/Wolverine.exe','Binaries/d3d9.dll','WGame/CookedPC/jungle1_zone01a.xxx','WGame/SaveData-ground-control/Player.wgameprofile')){
 if(!$manifest.contract.PSObject.Properties[$required]){throw "Missing prototype contract: $required"}
}
if($manifest.contract.'WGame/CookedPC/jungle1_zone01a.xxx' -notin @('41B9764DAC2F1E85584FC19BDAC00867288237B179F4D4CA31E0FF9C693F5D2A','0597813DB97FF0A36ABECE64C1D79835FBB9398249A4DC4C75BEF17812FFBEB4','10ADB25651B10B14DB1EE490FEEB17B0AE300ED558C2B507D16F9C2A5789D5EB')){throw 'Unrecognized authored room origin.'}
if(Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}){throw 'This prototype is already open.'}
$receipt=[ordered]@{schema=1;mode='meridian-developer-play';accepted=$false;attachmentGatePassed=$false;executable=$exe;runtimeSHA256=$manifest.contract.'Binaries/d3d9.dll';normalRuntimeModified=$false;automaticInput=$false;automaticPause=$false;timeoutSeconds=$null;limitations=$manifest.limitations}
if($ValidateOnly){$receipt | ConvertTo-Json -Depth 6;return}
$names=@(Get-ChildItem Env: | Where-Object {$_.Name -like 'MALEMOD_PRIVATE_*' -or $_.Name -in @('MALEMOD_ISOLATED_D3D9EX','MALEMOD_SEALED_SESSION','MALEMOD_TRACE_BOOT_IO','MALEMOD_MERIDIAN_RAW_CAPTURE','MALEMOD_MERIDIAN_LIGHT_TRACE')} | ForEach-Object {$_.Name})
$names+=@('__COMPAT_LAYER','MALEMOD_MERIDIAN_CANDIDATE','MALEMOD_SANDBOX_ORIGIN_V1')
$saved=@{};foreach($name in $names){$saved[$name]=[Environment]::GetEnvironmentVariable($name,'Process');[Environment]::SetEnvironmentVariable($name,$null,'Process')}
try{
 $env:__COMPAT_LAYER='RunAsInvoker';$env:MALEMOD_MERIDIAN_CANDIDATE='1';$env:MALEMOD_SANDBOX_ORIGIN_V1='1'
 Copy-Item -LiteralPath (Join-Path $game 'WGame/SaveData-ground-control/Player.wgameprofile') -Destination (Join-Path $game 'WGame/SaveData/Player.wgameprofile') -Force
 Write-Host 'Jockstrap developer prototype: Enter at title, then Continue. F6 controls; WASD/mouse or controller.'
 Write-Host 'Start with Overall/Width/Angle 50. Extreme settings remain rejected; this is not the accepted runtime.'
 $child=Start-Process -FilePath $exe -ArgumentList @('-windowed','-ResX=1280','-ResY=960','-nohomedir','-seekfreeloading') -WorkingDirectory (Split-Path $exe) -WindowStyle Normal -PassThru
 $receipt.processID=$child.Id;$receipt.launchedUTC=[DateTime]::UtcNow.ToString('o')
 $receipt | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $root 'human-play.json')
}finally{foreach($name in $names){[Environment]::SetEnvironmentVariable($name,$saved[$name],'Process')}}
