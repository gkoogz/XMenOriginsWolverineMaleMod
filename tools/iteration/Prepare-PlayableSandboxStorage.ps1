param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004',[string]$StateDirectory=(Join-Path $env:LOCALAPPDATA 'MaleMod/WolverineSandbox'))
$ErrorActionPreference='Stop'
$source=Join-Path (Resolve-Path -LiteralPath $Workspace).Path 'owned-game'
$state=[IO.Path]::GetFullPath($StateDirectory)
$allowed=[IO.Path]::GetFullPath((Join-Path $env:LOCALAPPDATA 'MaleMod'))+[IO.Path]::DirectorySeparatorChar
if(!$state.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)) {throw 'Storage must stay under LOCALAPPDATA/MaleMod.'}
$game=Join-Path $state 'human-game'
if(Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq (Join-Path $game 'Binaries/Wolverine.exe')}) {throw 'Close the human sandbox before preparing storage.'}
$drive=New-Object IO.DriveInfo ([IO.Path]::GetPathRoot($state))
if($drive.AvailableFreeSpace -lt 400MB -and !(Test-Path -LiteralPath (Join-Path $game 'Binaries/Wolverine.exe'))) {throw 'C: needs400MiB for the small playable runtime and writable outputs.'}
New-Item -ItemType Directory -Path $game -Force | Out-Null
foreach($rel in @('Binaries','Engine/Config','RavenShared/Config','WGame/Config','WGame/SaveData-ground-control')) {
 $destination=Join-Path $game $rel
 New-Item -ItemType Directory -Path $destination -Force | Out-Null
 # Only files in these measured small runtime/config trees; no packages/movies.
 foreach($item in Get-ChildItem -LiteralPath (Join-Path $source $rel) -Force) {
  if($rel -eq 'Binaries' -and $item.Name -in @('WolverineRuntime.log','WolverineLive.ini','MaleModSandboxControl.ini') -and (Test-Path -LiteralPath (Join-Path $destination $item.Name))) {continue}
  Copy-Item -LiteralPath $item.FullName -Destination $destination -Recurse -Force
 }
}
foreach($rel in @('Engine/Content','Engine/Localization','Engine/Shaders','WGame/CookedPC','WGame/Localization','WGame/Movies','WGame/Splash')) {
 $destination=Join-Path $game $rel
 $target=Join-Path $source $rel
 if(!(Test-Path -LiteralPath $target)) {
  if($rel -eq 'Engine/Content'){New-Item -ItemType Directory -Path $target -Force | Out-Null}
  else {throw "Required native asset directory missing: $rel"}
 }
 if(Test-Path -LiteralPath $destination) {
  $item=Get-Item -LiteralPath $destination
  if(!($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -or [IO.Path]::GetFullPath([string]@($item.Target)[0]) -ne $target) {throw "Unexpected asset path: $destination"}
 } else {New-Item -ItemType Junction -Path $destination -Target $target | Out-Null}
}
foreach($rel in @('WGame/SaveData','WGame/Logs')) {
 $path=Join-Path $game $rel
 if(Test-Path -LiteralPath $path) {
  $item=Get-Item -LiteralPath $path
  if($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
   $expected=Join-Path $state (Split-Path $rel -Leaf)
   if([IO.Path]::GetFullPath([string]@($item.Target)[0]) -ne $expected) {throw "Unexpected old output link: $path"}
   if(Test-Path -LiteralPath ($path+'.prior-mode-junction')) {throw 'Output-link backup already exists.'}
   # Preserve the old link rather than touch its shared sealed output target.
   Rename-Item -LiteralPath $path -NewName ((Split-Path $path -Leaf)+'.prior-mode-junction')
  }
 }
 New-Item -ItemType Directory -Path $path -Force | Out-Null
}
Copy-Item -LiteralPath (Join-Path $source 'WGame/PCTOC.txt') -Destination (Join-Path $game 'WGame/PCTOC.txt') -Force
Copy-Item -LiteralPath (Join-Path $source 'WGame/SaveData-ground-control/Player.wgameprofile') -Destination (Join-Path $game 'WGame/SaveData/Player.wgameprofile') -Force
@{schema=2;workspace=(Split-Path $source);state=$state;humanGame=$game;ownedSaveData=Join-Path $game 'WGame/SaveData';ownedLogs=Join-Path $game 'WGame/Logs';ownedRuntimeLog=Join-Path $game 'Binaries/WolverineRuntime.log';mutableDrive='C:';assetReads=$source;retailTouched=$false} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $state 'storage.json')
Write-Output "Human runtime and writable outputs: $game"
