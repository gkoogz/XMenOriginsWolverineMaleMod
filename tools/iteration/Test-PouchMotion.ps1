param([Parameter(Mandatory=$true)][string]$Workspace,[ValidatePattern('^[a-z0-9-]+$')][string]$OutputName='pouch-motion')
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$run=(Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw | ConvertFrom-Json).run
$out=Join-Path $run $OutputName
if(Test-Path -LiteralPath $out){throw 'Choose a fresh motion output.'}
New-Item -ItemType Directory -Path $out | Out-Null
$tool=Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1'
$records=@()
function Command([string]$Key,[int]$Value=50,[string]$Action='Press'){& $tool -Workspace $root -Key $Key -Value $Value -Action $Action | Out-Null}
function Capture([string]$Label){
 # Keep the game advancing: a paused pose is not a continuous motion test.
 Command Capture
 $control=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini') -Raw
 if($control -notmatch 'Revision=(\d+)'){throw 'Missing capture identity.'}
 $revision=$Matches[1];$deadline=(Get-Date).AddSeconds(5);$source=$null
 do{
  $complete=(Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/WolverineRuntime.log') -Raw) -match "Private native capture revision=$revision result=00000000"
  if($complete){$source=Get-ChildItem -LiteralPath $run -Filter "*-command-$revision.png" | Select-Object -First 1}
  if(!$source){Start-Sleep -Milliseconds 100}
 }while(!$source -and (Get-Date) -lt $deadline)
 if(!$source){throw 'Missing completed unpaused capture.'}
 $path=Join-Path $out ($Label+'.png');Copy-Item -LiteralPath $source.FullName -Destination $path
 $script:records+=[ordered]@{label=$Label;capture=$path;sha256=(Get-FileHash -LiteralPath $path).Hash;paused=$false;visualAccepted=$false}
 [ordered]@{schema=1;runtimeSHA256=(Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash;hostInput=$false;hostFocus=$false;captures=$script:records;fullAttachmentGatePassed=$false} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $out 'motion.json')
 Write-Output "Captured unpaused $Label"
}
try{
 Command Jockstrap;Command State 3
 foreach($size in @(50,100)){
  foreach($key in @('Overall','Width','Length','Scrotum')){Command $key $size}
  Command Resume;Command S 50 Hold;Start-Sleep -Milliseconds 600;Command S 50 Release;Start-Sleep -Seconds 2
  Capture "size-$size-standing"
  Command Space
  for($sample=0;$sample -lt 8;$sample++){Start-Sleep -Milliseconds 160;Capture "size-$size-jump-$sample"}
  Command W 50 Hold
  for($sample=0;$sample -lt 4;$sample++){Start-Sleep -Milliseconds 400;Capture "size-$size-run-$sample"}
  Command W 50 Release;Start-Sleep -Seconds 3;Capture "size-$size-rest"
  # The away run mainly exposes the straps. Return toward the fixed camera
  # so unpaused motion also exposes the pouch itself.
  Command S 50 Hold
  for($sample=0;$sample -lt 4;$sample++){Start-Sleep -Milliseconds 400;Capture "size-$size-run-front-$sample"}
  Command S 50 Release;Start-Sleep -Seconds 3;Capture "size-$size-rest-front"
 }
}finally{
 foreach($key in @('S','W','Space')){Command $key 50 Release}
 Command Resume
}
