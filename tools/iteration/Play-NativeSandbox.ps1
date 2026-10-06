param(
 [string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004',
 [string]$StateDirectory=(Join-Path $env:LOCALAPPDATA 'MaleMod/WolverineSandbox'),
 [switch]$ValidateOnly
)
$ErrorActionPreference='Stop'
try {
$root=(Resolve-Path -LiteralPath $Workspace).Path
$state=[IO.Path]::GetFullPath($StateDirectory)
$storage=Get-Content -LiteralPath (Join-Path $state 'storage.json') -Raw | ConvertFrom-Json
if($storage.schema -ne 2 -or $storage.workspace -ne $root) {throw 'Prepare-PlayableSandboxStorage.ps1 is required for the C: human runtime.'}
$game=Join-Path $state 'human-game'
if($storage.humanGame -ne $game) {throw 'Unexpected human runtime root.'}
$exe=Join-Path $game 'Binaries/Wolverine.exe'
if(!$ValidateOnly) {& (Join-Path $PSScriptRoot 'Apply-PendingSandboxStage.ps1') -Workspace $root -StateDirectory $state}
$floorHash='C84DF9CE0D73089C51F2A4DD9BAE49EFD04D60F5C73AA6C03085678623A5046B'
$activeStagePath=Join-Path $state 'active-stage.json'
if(Test-Path -LiteralPath $activeStagePath) {
 $stage=Get-Content -LiteralPath $activeStagePath -Raw | ConvertFrom-Json
 if($stage.schema -ne 1 -or !$stage.nativeGameplayVerified -or $stage.floorSHA256 -notin @($floorHash,'41B9764DAC2F1E85584FC19BDAC00867288237B179F4D4CA31E0FF9C693F5D2A')) {throw 'Unsupported active native stage contract.'}
 $floorHash=$stage.floorSHA256
}
$contract=@{
 'Binaries/Wolverine.exe'='F2CA60B045279F781E65749AB745924025B9768EF5712D0AC5E4606B2C848872'
 'Binaries/d3d9.dll'='02A654BA91B7AAC7BE3A8E9EF958EF61B178210A028BB50224EFC626AD693FDE'
 'WGame/CookedPC/jungle1_zone01a.xxx'=$floorHash
 'WGame/CookedPC/jungle1_kismet01.xxx'='1D783CC8E991F12766E31BCCF6061BBDC0A41AEA4F7298C1D404A00FC22A333F'
 'WGame/SaveData-ground-control/Player.wgameprofile'='62B3461845CAE6923AAB6B63501D01D99AFA1999C11F602774DD6B15EC913E57'
}
$reproducedContract=Join-Path $root 'sandbox-contract.json'
if(Test-Path -LiteralPath $reproducedContract){
 $rebuilt=Get-Content -LiteralPath $reproducedContract -Raw | ConvertFrom-Json
 if($rebuilt.schema -ne 1 -or !$rebuilt.nativeGameplayVerified -or !$rebuilt.visualReviewPassed){throw 'Rebuilt sandbox requires observed native gameplay and reviewed captures.'}
 $contract=@{};foreach($p in $rebuilt.contract.PSObject.Properties){$contract[$p.Name]=$p.Value}
 foreach($rel in @('Binaries/Wolverine.exe','Binaries/d3d9.dll','WGame/CookedPC/jungle1_zone01a.xxx','WGame/CookedPC/jungle1_kismet01.xxx','WGame/SaveData-ground-control/Player.wgameprofile')){if(!$contract.ContainsKey($rel)){throw "Rebuilt sandbox contract lacks: $rel"}}
}
foreach($rel in $contract.Keys) {
 $path=Join-Path $game $rel
 if (!(Test-Path -LiteralPath $path) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $contract[$rel]) {
  throw "Sandbox checkpoint differs: $rel. Revalidate before launching."
 }
}
$running=Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}
if($running) {throw 'The owned sandbox is already running. Close that window before opening another.'}
$drive=New-Object IO.DriveInfo ([IO.Path]::GetPathRoot($game))
if($drive.AvailableFreeSpace -lt 4194304) {throw 'C: needs at least4MiB free for private saves/logs/config writes.'}
$arguments=@('-windowed','-ResX=1280','-ResY=900','-nohomedir','-seekfreeloading')
$receipt=[ordered]@{
 schema=1;mode='human-play';executable=$exe;workingDirectory=(Split-Path $exe);arguments=$arguments
 actualPhysicalInput=$true;privateDriverEnabled=$false;debugger=$false;timeoutSeconds=$null
 automaticPause=$false;audio='normal engine audio, no mute applied';normalRetailModified=$false
 stateDirectory=$state;source='16affd49da5a122df776969574ca62a984b59425';base='99ff741ea95f18ed84526c35a6c3f38a37d857b6'
 gameDriveFreeBytes=$drive.AvailableFreeSpace;minimumGameDriveFreeBytes=4194304
 contract=$contract;verification='hashes and guarded classic-renderer/input route; visible play not launched by validation'
}
if($ValidateOnly) {$receipt | ConvertTo-Json -Depth 5;return}
if(!(Test-Path -LiteralPath (Join-Path $state 'storage.json'))) {
 throw 'Prepare the owned writable storage with Install-PlayableSandboxShortcut.ps1 first.'
}
# All flags used by the sealed agent driver are absent from the human child.
# This disables its private Ex renderer, input shim, auto menu/capture and pause.
$savedEnvironment=@{}
$names=@(Get-ChildItem Env: | Where-Object {$_.Name -like 'MALEMOD_PRIVATE_*' -or $_.Name -in @('MALEMOD_ISOLATED_D3D9EX','MALEMOD_SEALED_SESSION','MALEMOD_TRACE_BOOT_IO')} | ForEach-Object {$_.Name})
$names+= '__COMPAT_LAYER'
foreach($name in $names) {$savedEnvironment[$name]=[Environment]::GetEnvironmentVariable($name,'Process');[Environment]::SetEnvironmentVariable($name,$null,'Process')}
try {
 [Environment]::SetEnvironmentVariable('__COMPAT_LAYER','RunAsInvoker','Process')
 Copy-Item -LiteralPath (Join-Path $game 'WGame/SaveData-ground-control/Player.wgameprofile') -Destination (Join-Path $game 'WGame/SaveData/Player.wgameprofile') -Force
 Write-Host 'Wolverine Grey Sandbox: Enter at the title, then Continue.'
 Write-Host 'WASD/mouse or controller; Space jump. F6 menu; arrows adjust, Shift larger steps.'
 Write-Host 'Normal audio. No timer or automatic pause. Exit through the game or close its window.'
 $child=Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory (Split-Path $exe) -WindowStyle Normal -PassThru
 $receipt['processID']=$child.Id
 $receipt['processCreatedUTC']=$child.StartTime.ToUniversalTime().ToString('o')
 $receipt['launchedUTC']=[DateTime]::UtcNow.ToString('o')
 $receipt | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $state 'human-play.json')
} finally {
 foreach($name in $names) {[Environment]::SetEnvironmentVariable($name,$savedEnvironment[$name],'Process')}
}
} catch {
 $failureState=[IO.Path]::GetFullPath($StateDirectory)
 New-Item -ItemType Directory -Path $failureState -Force | Out-Null
 $failurePath=Join-Path $failureState 'launch-error.json'
 @{schema=1;utc=[DateTime]::UtcNow.ToString('o');error=$_.Exception.Message;details=$_.ToString();scriptStack=$_.ScriptStackTrace;gameLaunched=($null -ne $child)} | ConvertTo-Json | Set-Content -LiteralPath $failurePath
 if(!$ValidateOnly) {
  Add-Type -AssemblyName PresentationFramework
  [Windows.MessageBox]::Show(($_.Exception.Message+"`n`nDetails: "+$failurePath),'Wolverine Grey Sandbox launch error','OK','Error') | Out-Null
 }
 throw
}
