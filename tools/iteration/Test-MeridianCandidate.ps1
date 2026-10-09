param(
 [Parameter(Mandatory=$true)][string]$Workspace,
 [Parameter(Mandatory=$true)][string]$Base,
 [Parameter(Mandatory=$true)][string]$RuntimeProvenance,
 [ValidateRange(2,15)][int]$SettleSeconds=4,
 [ValidatePattern('^[a-z0-9-]+$')][string]$OutputName='meridian-cases'
)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$runInfo=Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw | ConvertFrom-Json
$run=$runInfo.run
$provenance=Get-Content -LiteralPath $RuntimeProvenance -Raw | ConvertFrom-Json
$runtime=Join-Path $root 'owned-game/Binaries/d3d9.dll'
$actual=(Get-FileHash -LiteralPath $runtime).Hash
if(!$provenance.meridianCandidate -or $actual -ne $provenance.runtimeSHA256){throw 'Running candidate identity differs from the supplied build provenance.'}
$profilePath=Join-Path (Resolve-Path -LiteralPath $Base).Path 'profiles/meridian-regression.json'
$profile=Get-Content -LiteralPath $profilePath -Raw | ConvertFrom-Json
if($profile.schema -ne 'malemod.meridian-regression/1'){throw 'Unsupported shared case contract.'}
$logPath=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
$gameplayFrames=[regex]::Matches((Get-Content -LiteralPath $logPath -Raw),'Private gameplay receipt frame=(\d+) ')
if(!($gameplayFrames | Where-Object {[int]$_.Groups[1].Value -ge 90})){throw 'Wait for the real gameplay pawn to complete its owned-room startup.'}
$output=Join-Path $run $OutputName
if(Test-Path -LiteralPath $output){throw 'This run already has a matrix; choose a new native run.'}
New-Item -ItemType Directory -Path $output | Out-Null
$inputTool=Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1'
function Command([string]$Key,[int]$Value=50,[string]$Action='Press') {& $inputTool -Workspace $root -Key $Key -Value $Value -Action $Action | Out-Null}
$cases=@($profile.cases)
foreach($state in 0..2){$cases+= [pscustomobject]@{id="mechanical-$state";overall=50;width=50;restAngle=50;state=$state}}
$cases+= [pscustomobject]@{id='naked-default';overall=50;width=50;restAngle=50;state=2;naked=$true}
$records=@()
try {
 # Turn toward the fixed camera using the owned engine input channel, then settle.
 Command Resume;Command S 50 Hold;Start-Sleep -Seconds 2;Command S 50 Release;Start-Sleep -Seconds $SettleSeconds;Command Pause
 foreach($case in $cases){
  $before=(Get-Content -LiteralPath $logPath).Count
  Command Overall $case.overall;Command Width $case.width;Command Angle $case.restAngle
  # PowerShell supplies an intrinsic Length=1 on scalar objects. Check the
  # authored property itself, or an omitted length silently becomes minimum.
  Command Length $(if($null -ne $case.PSObject.Properties['length']){[int]$case.length}else{50})
  Command Scrotum $(if($null -ne $case.scrotum){[int]$case.scrotum}else{50})
  if($null -ne $case.state){Command State ([int]$case.state+1)}else{Command State 3}
  if($case.naked){Command Naked}else{Command Jockstrap}
  Command Resume;Start-Sleep -Seconds $SettleSeconds;Command Pause;Command Capture
  $control=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini') -Raw
  if($control -notmatch 'Revision=(\d+)'){throw 'Capture revision missing.'}
  $revision=$Matches[1];$deadline=(Get-Date).AddSeconds(5);$capture=$null
  do {$complete=(Get-Content -LiteralPath $logPath -Raw) -match "Private native capture revision=$revision result=00000000";if($complete){$capture=Get-ChildItem -LiteralPath $run -Filter "*-command-$revision.png" | Select-Object -First 1};if(!$capture){Start-Sleep -Milliseconds 100}}while(!$capture -and (Get-Date) -lt $deadline)
  if(!$capture){throw 'The owned engine did not produce the requested capture.'}
  $destination=Join-Path $output ($case.id+'.png');Copy-Item -LiteralPath $capture.FullName -Destination $destination
  $newLines=@(Get-Content -LiteralPath $logPath | Select-Object -Skip $before)
  $errors=@($newLines | Where-Object {$_ -match 'Meridian (live pose|candidate) rejected|Meridian draw failure'})
  $records+=[ordered]@{id=$case.id;requested=$case;capture=$destination;captureSHA256=(Get-FileHash -LiteralPath $destination).Hash;reportedErrors=$errors;visualReviewed=$false}
  Write-Output "$($case.id): captured; reported rejection lines=$($errors.Count)"
  [ordered]@{schema=1;runtimeSHA256=$actual;profileSHA256=(Get-FileHash -LiteralPath $profilePath).Hash;nativeGameplayObserved=$true;visualAccepted=$false;attachmentGatePassed=$false;campaignChecked=$false;hostInput=$false;cases=$records} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'matrix.json')
 }
} finally {
 Command S 50 Release;Command W 50 Release
 # Leave this owned candidate paused at the neutral, clothed state for review.
 Command Overall 50;Command Width 50;Command Length 50;Command Scrotum 50;Command Angle 50;Command State 3;Command Jockstrap;Command Resume
 Start-Sleep -Seconds 2;Command Pause
}
