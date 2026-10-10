param(
 [Parameter(Mandatory=$true)][string]$Workspace,
 [Parameter(Mandatory=$true)][string]$OutputName,
 [ValidateRange(1,15)][int]$SettleSeconds=5,
 [string[]]$Cases=@(),
 [switch]$FullOrbit
)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'Read-SharedSandboxLog.ps1')
if($OutputName -notmatch '^[a-z0-9-]+$'){throw 'Use a simple output name.'}
$root=(Resolve-Path -LiteralPath $Workspace).Path
$activeRun=Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw | ConvertFrom-Json
$run=$activeRun.run
$output=Join-Path $run $OutputName
if(Test-Path -LiteralPath $output){throw 'Comparison already exists.'}
New-Item -ItemType Directory -Path $output | Out-Null
$log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
$deadline=(Get-Date).AddSeconds(240)
do {
 $freshLog=(Test-Path -LiteralPath $log) -and ((Get-Item -LiteralPath $log).LastWriteTimeUtc -ge [datetime]$activeRun.workerCreatedUTC)
 $owned=Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $activeRun.ownedExecutable}
 $text=if($freshLog -and $owned){Read-SharedSandboxLog $log}else{''}
 $ready=@([regex]::Matches($text,'Private gameplay receipt frame=(\d+) ') | Where-Object {[int]$_.Groups[1].Value -ge 360}).Count -gt 0
 if(!$ready){Start-Sleep -Milliseconds 250}
}while(!$ready -and (Get-Date) -lt $deadline)
if(!$ready){throw 'Gameplay did not reach its settled startup checkpoint.'}
function Command([string]$Key,[int]$Value=50,[string]$Action='Press'){
 & (Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1') -Workspace $root -Key $Key -Value $Value -Action $Action | Out-Null
}
function Hold([string]$Key,[int]$Milliseconds){Command $Key 50 Hold;Start-Sleep -Milliseconds $Milliseconds;Command $Key 50 Release}
$records=[Collections.Generic.List[object]]::new()
function Capture([string]$Name){
 Command Capture
 $control=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini') -Raw
 if($control -notmatch 'Revision=(\d+)'){throw 'Capture revision missing.'}
 $revision=$Matches[1];$until=(Get-Date).AddSeconds(30)
 do {
  $text=Read-SharedSandboxLog $log
  $image=Get-CompletedSandboxCapture $run ([uint32]$revision)
  if(!$image){Start-Sleep -Milliseconds 100}
 }while(!$image -and (Get-Date) -lt $until)
 if(!$image){throw 'Native capture failed.'}
 $target=Join-Path $output "$Name.png";Copy-Item -LiteralPath $image.FullName -Destination $target
 $records.Add([ordered]@{name=$Name;revision=$revision;logReceipt=($text -match "Private native capture revision=$revision result=00000000");source=$image.FullName;capture=$target;sha256=(Get-FileHash -LiteralPath $target).Hash})
 Write-Output "Captured $Name"
}
try {
 Command Resume
 foreach($case in @(@{id='default';size=50;angle=50;state=3},@{id='minimum';size=1;angle=50;state=3},@{id='maximum';size=100;angle=50;state=3},@{id='firm';size=50;angle=50;state=1},@{id='intermediate';size=50;angle=50;state=2},@{id='angle-low';size=50;angle=1;state=3},@{id='angle-high';size=50;angle=100;state=3})){
  if($Cases.Count -and $case.id -notin $Cases){continue}
  foreach($key in @('Overall','Width','Length','Scrotum')){Command $key $case.size}
  Command Angle $case.angle;Command State $case.state;Command Naked
  Hold S 1000;Start-Sleep -Seconds $SettleSeconds;Capture ($case.id+'-front')
  Hold TurnRight 700;Start-Sleep -Seconds 2;Capture ($case.id+'-oblique')
  Hold TurnRight 1000;Start-Sleep -Seconds 2;Capture ($case.id+'-side')
  Hold TurnLeft 1700
 }
} finally {
 Command S 50 Release;Command TurnRight 50 Release;Command TurnLeft 50 Release
 foreach($key in @('Overall','Width','Length','Scrotum','Angle')){Command $key 50}
 Command State 3;Command Jockstrap
 [ordered]@{runtimeSHA256=(Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash;captures=$records;hostInput=$false;hostFocus=$false;visualAccepted=$false} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'comparison.json')
}
