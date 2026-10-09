param([Parameter(Mandatory=$true)][string]$Workspace,[ValidatePattern('^[a-z0-9-]+$')][string]$OutputName='anterior-motion')
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
 Command Pause;Command Capture
 $control=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini') -Raw
 if($control -notmatch 'Revision=(\d+)'){throw 'Missing command identity.'}
 $revision=$Matches[1]
 $deadline=(Get-Date).AddSeconds(5);$source=$null
 do{
  $complete=(Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/WolverineRuntime.log') -Raw) -match "Private native capture revision=$revision result=00000000"
  if($complete){$source=Get-ChildItem -LiteralPath $run -Filter "*-command-$revision.png" | Select-Object -First 1}
  if(!$source){Start-Sleep -Milliseconds 100}
 }while(!$source -and (Get-Date) -lt $deadline)
 if(!$source){throw 'Missing acknowledged native capture.'}
 $path=Join-Path $out ($Label+'.png');Copy-Item -LiteralPath $source.FullName -Destination $path
 $script:records+=[ordered]@{label=$Label;capture=$path;sha256=(Get-FileHash -LiteralPath $path).Hash;visualAccepted=$false}
 [ordered]@{schema=1;runtimeSHA256=(Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash;hostInput=$false;hostFocus=$false;allControls=100;captures=$script:records;fullAttachmentGatePassed=$false} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $out 'motion.json')
 Write-Output "Captured $Label"
}
try{
 Command Overall 100;Command Width 100;Command Length 100;Command Scrotum 100;Command Angle 50
 foreach($style in @('Naked','Jockstrap')){
  Command $style;Command State 3;Command Resume
  Command S 50 Hold;Start-Sleep -Milliseconds 600;Command S 50 Release;Start-Sleep -Seconds 3
  Capture ($style+'-standing-front')
  # Z is the documented Block alias. Capture the actual native pose rather
  # than claiming that the game exposes a dedicated crouch input.
  Command Resume;Command Block 50 Hold;Start-Sleep -Seconds 1;Capture ($style+'-block')
  Command Block 50 Release;Command Resume;Start-Sleep -Milliseconds 250;Capture ($style+'-release')
  Command Resume;Start-Sleep -Seconds 3;Capture ($style+'-rest-after-block')
  for($cycle=1;$cycle -le 2;$cycle++){
   Command Resume;Command Space;Start-Sleep -Milliseconds 350;Capture ($style+"-jump-$cycle")
   Command Resume;Start-Sleep -Milliseconds 550;Capture ($style+"-landing-$cycle")
   Command Resume;Start-Sleep -Seconds 3;Capture ($style+"-rest-after-landing-$cycle")
  }
  Command Resume;Command D 50 Hold;Start-Sleep -Milliseconds 500;Command D 50 Release;Start-Sleep -Seconds 2;Capture ($style+'-side')
  Command Resume;Command TurnLeft 50 Hold;Start-Sleep -Milliseconds 450;Command TurnLeft 50 Release;Start-Sleep -Seconds 1;Capture ($style+'-oblique')
 }
}finally{
 foreach($key in @('Block','Space','S','D','TurnLeft')){Command $key 50 Release}
 Command Resume
}
