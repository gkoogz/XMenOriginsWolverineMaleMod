param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004')
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
& (Join-Path $PSScriptRoot 'Open-NativeSandbox.ps1') -Workspace $root -Seconds 300 -Active
$active=Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw | ConvertFrom-Json
$log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
$cases=[Collections.Generic.List[object]]::new()
function Command([string]$key,[string]$action='Press',[int]$value=50) {
 & (Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1') -Workspace $root -Key $key -Action $action -Value $value
}
function CaptureCase([string]$name) {
 Command Capture
 Start-Sleep -Milliseconds 600
 $capture=Get-ChildItem -LiteralPath $active.run -Filter '*.png' | Sort-Object LastWriteTime | Select-Object -Last 1
 $cases.Add(@{name=$name;capture=$capture.FullName;capturedUTC=$capture.LastWriteTimeUtc.ToString('o')})
 Write-Output "Native case: $name $($capture.Name)"
}
function CpuSample([string]$name) {
 $receipt=Get-Content -LiteralPath (Join-Path $active.run 'guard.txt') -Raw
 if ($receipt -notmatch '\{"processID":(\d+),"processCreationTime":(\d+)') {throw 'Exact child receipt missing.'}
 $childID=[int]$Matches[1];$created=[long]$Matches[2]
 $process=Get-Process -Id $childID
 if ($process.StartTime.ToUniversalTime().ToFileTimeUtc() -ne $created) {throw 'Child identity mismatch.'}
 $before=$process.TotalProcessorTime.TotalSeconds;$clock=[Diagnostics.Stopwatch]::StartNew()
 Start-Sleep -Seconds 2
 $process.Refresh();$wall=$clock.Elapsed.TotalSeconds
 $cases.Add(@{name=$name;cpuSecondsPerSecond=($process.TotalProcessorTime.TotalSeconds-$before)/$wall;sampleSeconds=$wall})
}
try {
 $deadline=(Get-Date).AddSeconds(60)
 do {
  $text=Get-Content -LiteralPath $log -Raw -ErrorAction SilentlyContinue
  $nativeStarted=Test-Path -LiteralPath (Join-Path $active.run 'guard.txt')
  if ($nativeStarted) {$nativeStarted=(Get-Content -LiteralPath (Join-Path $active.run 'guard.txt') -Raw) -match '\{"processID":\d+,"processCreationTime":\d+'}
  if ($nativeStarted -and $text -match 'Private gameplay receipt frame=1 ') {break}
  Start-Sleep -Milliseconds 250
 } while ((Get-Date) -lt $deadline)
 if ($text -notmatch 'Private gameplay receipt frame=1 ') {throw 'No observed native player within deadline.'}
 Command Defaults;Start-Sleep -Seconds 6;CaptureCase idle-naked;CpuSample idle-naked-cpu
 Command W Hold;Start-Sleep -Seconds 3;CaptureCase walking;CpuSample moving-cpu;Command W Release
 Command TurnLeft Hold;Start-Sleep -Seconds 3;Command TurnLeft Release;CaptureCase turned
 Command LookUp Hold;Start-Sleep -Seconds 1;Command LookUp Release;CaptureCase camera-up
 Command Zoom Hold;Start-Sleep -Seconds 2;CaptureCase camera-zoom;Command Zoom Release
 Command Jockstrap;Start-Sleep -Seconds 8;CaptureCase jockstrap
 Command Overall Press 100;Start-Sleep -Seconds 8;CaptureCase size-max-jockstrap
 Command Overall Press 1;Start-Sleep -Seconds 8;CaptureCase size-min-jockstrap
 Command Defaults;Start-Sleep -Seconds 5
 Command F6;Start-Sleep -Seconds 2;CaptureCase menu
 Command Down;Start-Sleep -Milliseconds 400;Command Down;Start-Sleep -Milliseconds 400;Command Down;Start-Sleep -Milliseconds 400;Command Right
 Start-Sleep -Seconds 2;CaptureCase live-overall;Command F6
 Command Pause;Start-Sleep -Seconds 1;CpuSample paused-cpu
 Command Resume;Start-Sleep -Seconds 3;CaptureCase resumed
 @{'schema'=1;cases=$cases;hostInput=$false;hostFocus=$false;solver='installed16aff/Base99ff unchanged';fluidFloorContactVerified=$false} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $active.run 'verification.json')
} finally {
 & (Join-Path $PSScriptRoot 'Close-NativeSandbox.ps1') -Workspace $root
}
