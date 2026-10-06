param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004',[Parameter(Mandatory=$true)][ValidateSet('W','A','S','D','TurnLeft','TurnRight','LookUp','LookDown','Zoom','Space','Shift','F6','F8','Up','Down','Left','Right','Capture','Pause','Resume','Defaults','Overall','Naked','Jockstrap')][string]$Key,[ValidateSet('Press','Hold','Release')][string]$Action='Press',[ValidateRange(1,100)][int]$Value=50)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$file=Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini'
if (!(Test-Path -LiteralPath $file)) { throw 'Start the prepared native sandbox first.' }
$exe=Join-Path $root 'owned-game/Binaries/Wolverine.exe'
$running=Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}
if (!$running) { throw 'The owned sandbox is not running.' }
$old=Get-Content -LiteralPath $file -Raw
$revision=if ($old -match 'Revision=(\d+)') {[uint32]$Matches[1]+1} else {1}
if ($revision -gt 1) {
 $previous=$revision-1
 $log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
 $deadline=(Get-Date).AddSeconds(5)
 do {
  $seen=Get-Content -LiteralPath $log -Raw
  if ($seen -match "Private command(?: capture)? revision=$previous(?: |`r|`n)") {break}
  Start-Sleep -Milliseconds 100
 } while ((Get-Date) -lt $deadline)
 if ($seen -notmatch "Private command(?: capture)? revision=$previous(?: |`r|`n)") {throw 'Previous private command was not acknowledged; refusing to overwrite it.'}
}
$temporary=$file+'.pending'
[IO.File]::WriteAllText($temporary,"[Control]`nRevision=$revision`nKey=$Key`nAction=$Action`nValue=$Value`n",[Text.Encoding]::ASCII)
Move-Item -LiteralPath $temporary -Destination $file -Force
Write-Output "Private engine command $revision`: $Key $Action"
