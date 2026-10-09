param([Parameter(Mandatory=$true)][string]$Workspace,[Parameter(Mandatory=$true)][string]$RuntimeProvenance)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$run=(Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw|ConvertFrom-Json).run
$build=Get-Content -LiteralPath $RuntimeProvenance -Raw|ConvertFrom-Json
if((Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash -ne $build.runtimeSHA256){throw 'Candidate identity mismatch'}
$log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
$out=Join-Path $run 'jeans-motion';New-Item -ItemType Directory -Path $out -ErrorAction Stop|Out-Null
$records=@()
$motionLogStart=0
function Command([string]$Key,[string]$Action='Press'){& (Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1') -Workspace $root -Key $Key -Action $Action|Out-Null}
function ReadLog {for($i=0;$i -lt 30;$i++){try{return Get-Content -LiteralPath $log -Raw}catch{Start-Sleep -Milliseconds 100}};throw 'Log unavailable'}
function Capture([string]$Id){
 Command Capture
 $ini=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini') -Raw
 if($ini -notmatch 'Revision=(\d+)'){throw 'Capture revision missing'};$revision=$Matches[1]
 $deadline=(Get-Date).AddSeconds(10);$file=$null
 do{if((ReadLog) -match "Private native capture revision=$revision result=00000000"){$file=Get-ChildItem -LiteralPath $run -Filter "*-command-$revision.png"|Select-Object -First 1};if(!$file){Start-Sleep -Milliseconds 100}}while(!$file -and (Get-Date) -lt $deadline)
 if(!$file){throw "Capture missing: $Id"};$path=Join-Path $out ($Id+'.png');Copy-Item -LiteralPath $file.FullName -Destination $path
 $nativeFrame=if($file.Name -match 'native-frame-(\d+)') {[int]$Matches[1]}else{-1}
 $script:records+=@{id=$Id;nativeFrame=$nativeFrame;capturedUTC=$file.LastWriteTimeUtc.ToString('o');capture=$path;sha256=(Get-FileHash -LiteralPath $path).Hash;visualReviewed=$false}
 Write-Output "$Id captured"
}
$last=(ReadLog) -split "`n"|Select-String 'Private gameplay receipt'|Select-Object -Last 1
if(!$last -or $last.Line -notmatch 'frame=(\d+)' -or [int]$Matches[1] -lt 360){throw 'Run Test-Costumes first; startup not complete'}
try{
 $motionLogStart=(ReadLog).Length
 Command Defaults;Command TankTop;Command JeansOpen;Command Resume;Command S Hold;Start-Sleep -Seconds 1;Command S Release;Start-Sleep -Seconds 2
 $last=(ReadLog) -split "`n"|Select-String 'Private gameplay receipt'|Select-Object -Last 1
 if($last.Line -match 'menu=1'){Command F6}
 Capture 'rest'
 Command S Hold
 for($i=0;$i -lt 6;$i++){Start-Sleep -Milliseconds 250;Capture ('stride-'+$i)}
 Command S Release
 for($i=0;$i -lt 8;$i++){Start-Sleep -Milliseconds 250;Capture ('settle-'+$i)}
 Command Pause;Capture 'settled'
 $currentLog=ReadLog
 if($currentLog.Length -lt $motionLogStart){throw 'Runtime log rotated during motion sequence'}
 $samples=@();foreach($line in ($currentLog.Substring($motionLogStart) -split "`n")){
  if($line -match 'Jeans secondary frame=(\d+) fly=\(([-.\d]+) ([-.\d]+)\) belt=\(([-.\d]+) ([-.\d]+)\)'){
   $angles=@([double]$Matches[2],[double]$Matches[3],[double]$Matches[4],[double]$Matches[5]);
   if([Math]::Abs($angles[0]) -gt .05237 -or [Math]::Abs($angles[1]) -gt .05237 -or [Math]::Abs($angles[2]) -gt .03492 -or [Math]::Abs($angles[3]) -gt .03492){throw 'Secondary motion exceeded travel bound'}
   $samples+=@{frame=[int]$Matches[1];angles=$angles}
  }
 }
 if($samples.Count -lt 3){throw 'Insufficient observed secondary-motion samples'}
 @{schema='wolverine.jeans-motion/1';runtimeSHA256=$build.runtimeSHA256;sourceCommit=$build.sourceCommit;baseCommit=$build.baseCommit;captures=$records;samples=$samples;hostInput=$false;attachmentGatePassed=$false;bodyCollisionVerified=$false}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $out 'matrix.json')
}finally{Command S Release;Command Pause}
