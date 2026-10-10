param(
 [Parameter(Mandatory=$true)][string]$Workspace,
 [Parameter(Mandatory=$true)][string]$OutputName,
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
 foreach($key in @('Overall','Width','Length','Scrotum','Glans','Hang','Angle','Forward','Vertical')){Command $key 50}
 Command State 3;Command Jockstrap
 Hold S 1500;Start-Sleep -Seconds 2
 foreach($case in @(@{id='default';overall=50;width=50},@{id='large';overall=86;width=100},@{id='maximum';overall=100;width=100})){
  Command Overall $case.overall;Command Width $case.width;Start-Sleep -Seconds 2
  Capture ($case.id+'-front')
  Hold TurnRight 450;Start-Sleep -Seconds 1;Capture ($case.id+'-oblique')
  Hold TurnRight 550;Start-Sleep -Seconds 1;Capture ($case.id+'-side')
  Hold TurnLeft 1000;Start-Sleep -Seconds 1
 }
 Command Overall 50;Command Width 50;Command State 1;Start-Sleep -Seconds 2;Capture 'firm-front'
 Command State 3;Hold S 750;Capture 'motion-front'
 if($FullOrbit){
  for($view=0;$view -lt 8;$view++){Hold TurnRight 450;Start-Sleep -Milliseconds 500;Capture ("orbit-$view")}
  Hold LookUp 400;Start-Sleep -Milliseconds 500;Capture 'pitch-a'
  Hold LookDown 800;Start-Sleep -Milliseconds 500;Capture 'pitch-b'
  Hold LookUp 400
 }
 Command Naked;Start-Sleep -Seconds 1;Capture 'attachment-naked'
} finally {
 Command S 50 Release;Command W 50 Release;Command TurnRight 50 Release;Command TurnLeft 50 Release
 Command Overall 50;Command Width 50;Command State 3;Command Jockstrap
 [ordered]@{runtimeSHA256=(Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash;captures=$records;recipe='same timed native camera orbit; not pixel-aligned poses';hostInput=$false;hostFocus=$false;attachmentGatePassed=$false;continuousContactCertified=$false} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'comparison.json')
}
