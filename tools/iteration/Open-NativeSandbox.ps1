param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004',[ValidateRange(20,1800)][int]$Seconds=600,[switch]$AcceptanceControls,[switch]$Active,[switch]$TitleOnly,[ValidateRange(640,3840)][int]$RenderWidth=640,[ValidateRange(480,2160)][int]$RenderHeight=480)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$exe=Join-Path $root 'owned-game/Binaries/Wolverine.exe'
if (Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}) {throw 'Owned sandbox already running.'}
$run=Join-Path $root ('runs/'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'-'+[guid]::NewGuid().ToString('N').Substring(0,6))
New-Item -ItemType Directory -Path $run | Out-Null
$shell=(Get-Process -Id $PID).Path
$argsList=@('-NoProfile','-File',('"'+(Join-Path $PSScriptRoot 'Start-NativeSandbox.ps1')+'"'),'-Workspace',('"'+$root+'"'),'-Seconds',[string]$Seconds,'-RenderWidth',[string]$RenderWidth,'-RenderHeight',[string]$RenderHeight,'-RunDirectory',('"'+$run+'"'))
if ($AcceptanceControls) {$argsList+='-AcceptanceControls'}
if ($Active) {$argsList+='-Active'}
if ($TitleOnly) {$argsList+='-TitleOnly'}
$worker=Start-Process -FilePath $shell -ArgumentList $argsList -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $run 'worker.txt') -RedirectStandardError (Join-Path $run 'worker-error.txt')
@{schema=1;run=$run;workerPID=$worker.Id;workerCreatedUTC=$worker.StartTime.ToUniversalTime().ToString('o');ownedExecutable=$exe;seconds=$Seconds} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $root 'active-run.json')
Write-Output "Opened isolated native sandbox: $run"
