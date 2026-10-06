param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004',[ValidateRange(20,1800)][int]$Seconds=600)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$exe=Join-Path $root 'owned-game/Binaries/Wolverine.exe'
if (Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}) {
 & (Join-Path $PSScriptRoot 'Close-NativeSandbox.ps1') -Workspace $root
 $deadline=(Get-Date).AddSeconds(8)
 do {
  $running=Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}
  if (!$running) {break};Start-Sleep -Milliseconds 200
 } while ((Get-Date) -lt $deadline)
 if ($running) {throw 'Original owned child did not close; refusing concurrent reset.'}
}
& (Join-Path $PSScriptRoot 'Open-NativeSandbox.ps1') -Workspace $root -Seconds $Seconds
$active=Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw | ConvertFrom-Json
$log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
$deadline=(Get-Date).AddSeconds(60)
do {
 $text=Get-Content -LiteralPath $log -Raw -ErrorAction SilentlyContinue
 $guard=Get-Content -LiteralPath (Join-Path $active.run 'guard.txt') -Raw -ErrorAction SilentlyContinue
 if ($guard -match 'processCreationTime' -and $text -match 'Private gameplay receipt frame=1 ') {break}
 Start-Sleep -Milliseconds 250
} while ((Get-Date) -lt $deadline)
if ($text -notmatch 'Private gameplay receipt frame=1 ') {throw 'Reset native player did not become ready.'}
& (Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1') -Workspace $root -Key Defaults
Write-Output 'Owned checkpoint restored; Full Floppy, size50, Naked and default physics requested.'
