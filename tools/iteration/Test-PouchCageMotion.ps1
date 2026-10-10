param(
 [Parameter(Mandatory=$true)][string]$Workspace,
 [Parameter(Mandatory=$true)][string]$OutputName,
 [switch]$Naked,
 [ValidateRange(6,24)][int]$Steps=6
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
 Command State 3;if($Naked){Command Naked}else{Command Jockstrap}
 Hold S 1500;Start-Sleep -Seconds 2
 Command Zoom 50 Hold;Start-Sleep -Seconds 1;Capture 'zoom-front';Command Zoom 50 Release
 foreach($scenario in @('walk','sprint','jump')){
  Command S 50 Hold
  if($scenario -eq 'sprint'){Command Shift 50 Hold}
  if($scenario -eq 'jump'){Command Space}
  for($step=0;$step -lt $Steps;$step++){Start-Sleep -Milliseconds 200;Capture ("$scenario-$step")}
  Command S 50 Release;Command Shift 50 Release
  Start-Sleep -Seconds 1
 }
 Hold TurnRight 550;Command S 50 Hold
 for($step=0;$step -lt $Steps;$step++){Start-Sleep -Milliseconds 200;Capture ("oblique-motion-$step")}
 Command S 50 Release
} finally {
 Command Zoom 50 Release;Command Shift 50 Release;Command Space 50 Release;Command S 50 Release;Command W 50 Release;Command TurnRight 50 Release;Command TurnLeft 50 Release
 Command Overall 50;Command Width 50;Command State 3;Command Jockstrap
 [ordered]@{runtimeSHA256=(Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash;captures=$records;naked=[bool]$Naked;recipe='owned native walking sprint jump and oblique motion; captures during held input; not continuous video';hostInput=$false;hostFocus=$false;attachmentGatePassed=$false;continuousContactCertified=$false} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'comparison.json')
}
