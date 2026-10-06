param([Parameter(Mandatory=$true)][string]$Workspace,[ValidateRange(20,1800)][int]$Seconds=600,[switch]$AcceptanceControls,[switch]$Active,[string]$RunDirectory,[string]$LauncherPath=(Join-Path $env:LOCALAPPDATA 'MaleMod/WolverineSandbox/launch_capability_restore.exe'))
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
if(!$PSBoundParameters.ContainsKey('LauncherPath') -and (Test-Path -LiteralPath (Join-Path $root 'launch_capability_restore.exe'))){$LauncherPath=Join-Path $root 'launch_capability_restore.exe'}
$exe=Join-Path $root 'owned-game/Binaries/Wolverine.exe'
$launcher=[IO.Path]::GetFullPath($LauncherPath)
if (!(Test-Path -LiteralPath $exe) -or !(Test-Path -LiteralPath $launcher)) { throw 'Prepared native clone and compiled private launcher with audio restoration required; see ITERATION-SANDBOX.md.' }
$running=Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}
if ($running) { throw 'The owned sandbox is already running.' }
$run=if ($RunDirectory) {[IO.Path]::GetFullPath($RunDirectory)} else {Join-Path $root ('runs/'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
if (!$run.StartsWith((Join-Path $root 'runs')+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) {throw 'Run must stay under owned workspace/runs.'}
New-Item -ItemType Directory -Path $run -Force | Out-Null
$oldLog=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
if (Test-Path -LiteralPath $oldLog) {
 # User-play storage may direct this exact owned log to C:. Preserve that link
 # across sealed iterations; moving it would restore unwanted E: log growth.
 if ((Get-Item -LiteralPath $oldLog).Attributes -band [IO.FileAttributes]::ReparsePoint) {
  [IO.File]::WriteAllText((Join-Path $run 'previous-runtime.log'),[IO.File]::ReadAllText($oldLog))
 } else {Move-Item -LiteralPath $oldLog -Destination (Join-Path $run 'previous-runtime.log') -Force}
}
$control=Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini'
Set-Content -LiteralPath $control -Value "[Control]`nRevision=0" -Encoding ascii
$env:MALEMOD_ISOLATED_D3D9EX='1'
$env:MALEMOD_PRIVATE_MENU='new-game'
$env:MALEMOD_PRIVATE_CAPTURE=Join-Path $run 'native'
$env:MALEMOD_PRIVATE_TIMEOUT_MS=[string]($Seconds*1000)
$env:MALEMOD_PRIVATE_TEST_CONTROLS=if ($AcceptanceControls) {'1'} else {'0'}
$env:MALEMOD_PRIVATE_START_PAUSED=if ($Active -or $AcceptanceControls) {'0'} else {'1'}
$env:MALEMOD_TRACE_BOOT_IO='0'
# The private checkpoint always provides the same native player spawn.
$sourceProfile=Join-Path $root 'owned-game/WGame/SaveData-ground-control/Player.wgameprofile'
Copy-Item -LiteralPath $sourceProfile -Destination (Join-Path $root 'owned-game/WGame/SaveData/Player.wgameprofile') -Force
# Invocation is synchronous and bounded; the native launcher creates only a
# noninteractive station, handles its exact child, and captures its own output.
Write-Output "Native captures and diagnostics: $run"
& $launcher (Join-Path $run 'engine.dmp') $exe '-windowed' '-ResX=640' '-ResY=480' '-unattended' '-nohomedir' '-seekfreeloading' *> (Join-Path $run 'guard.txt')
$launcherStatus=$LASTEXITCODE
$guard=Get-Content -LiteralPath (Join-Path $run 'guard.txt') -Raw
$exitMatches=[regex]::Matches($guard,'\{"exitCode":(\d+)\}')
$status=if ($exitMatches.Count) {[int]$exitMatches[$exitMatches.Count-1].Groups[1].Value} else {$launcherStatus}
Copy-Item -LiteralPath (Join-Path $root 'owned-game/Binaries/WolverineRuntime.log') -Destination (Join-Path $run 'runtime.log') -ErrorAction SilentlyContinue
@{schema=1;exitCode=$status;boundedTimeout=($status -eq 124);seconds=$Seconds;hostInput=$false;hostFocus=$false;audio='exact owned process sessions only'} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $run 'run.json')
if ($status -ne 0 -and $status -ne 124) { throw "Native child failed with status $status; see guard.txt." }
