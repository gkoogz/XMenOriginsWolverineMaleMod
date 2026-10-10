param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004',[Parameter(Mandatory=$true)][ValidateSet('Enter','Attack','J','W','A','S','D','TurnLeft','TurnRight','LookUp','LookDown','Zoom','Space','Block','Shift','F6','F8','Up','Down','Left','Right','Capture','Pause','Resume','Defaults','Overall','Width','Length','Scrotum','Glans','Hang','Angle','Forward','Vertical','State','Naked','Jockstrap','TopNaked','TankTop','Jeans','JeansOpen')][string]$Key,[ValidateSet('Press','Hold','Release')][string]$Action='Press',[ValidateRange(0,100)][int]$Value=50)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'Read-SharedSandboxLog.ps1')
if($Key -eq 'State' -and ($Value -lt 1 -or $Value -gt 3)){throw 'State requires 1, 2 or 3; no command was written.'}
if($Value -eq 0 -and $Key -notin @('Length','Glans')){throw 'Only Length and Glans support zero; no command was written.'}
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
 $acknowledgement="Private (?:command(?: capture)?|native capture) revision=$previous(?: |`r|`n)"
 $log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
 $deadline=(Get-Date).AddSeconds(30)
 do {
  $seen=Read-SharedSandboxLog $log
  if ($seen -match $acknowledgement) {break}
  Start-Sleep -Milliseconds 100
 } while ((Get-Date) -lt $deadline)
 if ($seen -notmatch $acknowledgement) {throw 'Previous private command was not acknowledged; refusing to overwrite it.'}
}
$temporary=$file+'.pending'
[IO.File]::WriteAllText($temporary,"[Control]`nRevision=$revision`nKey=$Key`nAction=$Action`nValue=$Value`n",[Text.Encoding]::ASCII)
# Replace atomically; the native read handle may briefly deny replacement.
# Keep the previous complete command intact and retry that short sharing race.
$replaced=$false
for($attempt=0;$attempt -lt 40;$attempt++){
 try{[IO.File]::Replace($temporary,$file,[NullString]::Value,$true);$replaced=$true;break}
 catch [IO.IOException]{if($attempt -eq 39){throw};Start-Sleep -Milliseconds 50}
}
if(!$replaced){throw 'Atomic private command replacement failed.'}
Write-Output "Private engine command $revision`: $Key $Action"
