param([Parameter(Mandatory=$true)][string]$Workspace,[Parameter(Mandatory=$true)][string]$RuntimeProvenance,[switch]$Jeans)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$run=(Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw|ConvertFrom-Json).run
$build=Get-Content -LiteralPath $RuntimeProvenance -Raw|ConvertFrom-Json
if((Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash -ne $build.runtimeSHA256){throw 'Candidate identity mismatch'}
$log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
$output=Join-Path $run 'costume-cases';New-Item -ItemType Directory -Path $output -ErrorAction Stop|Out-Null
$records=@()
$started=$false
function ReadLog {
 for($attempt=0;$attempt -lt 30;$attempt++){
  try{return Get-Content -LiteralPath $log -Raw}catch{Start-Sleep -Milliseconds 100}
 }
 throw 'Current runtime log stayed unavailable'
}
function Command([string]$Key,[string]$Action='Press',[int]$Value=50){& (Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1') -Workspace $root -Key $Key -Action $Action -Value $Value|Out-Null}
function Capture([string]$Id){
 Command Capture
 $ini=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini') -Raw
 if($ini -notmatch 'Revision=(\d+)'){throw 'Capture revision absent'};$revision=$Matches[1]
 $deadline=(Get-Date).AddSeconds(10);$file=$null
 do{if((ReadLog) -match "Private native capture revision=$revision result=00000000"){$file=Get-ChildItem -LiteralPath $run -Filter "*-command-$revision.png"|Select-Object -First 1};if(!$file){Start-Sleep -Milliseconds 100}}while(!$file -and (Get-Date) -lt $deadline)
 if(!$file){throw "Native capture missing: $Id"}
 $path=Join-Path $output ($Id+'.png');Copy-Item -LiteralPath $file.FullName -Destination $path
 $script:records+=@{id=$Id;capture=$path;sha256=(Get-FileHash -LiteralPath $path).Hash;visualReviewed=$false}
 Write-Output "$Id captured"
 @{'schema'='wolverine.costume-cases/1';runtimeSHA256=$build.runtimeSHA256;sourceCommit=$build.sourceCommit;baseCommit=$build.baseCommit;cases=$script:records;hostInput=$false;attachmentGatePassed=$false}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $output 'matrix.json')
}
try{
 # The sealed startup demo ends at frame 350. Do not race its F6/shape input.
 $deadline=(Get-Date).AddSeconds(300)
 do{
  $ready=$false
  if((Test-Path -LiteralPath (Join-Path $run 'previous-runtime.log')) -and (Test-Path -LiteralPath $log)){
   $last=(ReadLog) -split "`n"|Select-String -Pattern 'Private gameplay receipt'|Select-Object -Last 1
   $ready=$last -and $last.Line -match 'frame=(\d+)' -and [int]$Matches[1] -ge 360
  }
  if(!$ready){Start-Sleep -Milliseconds 200}
 }while(!$ready -and (Get-Date) -lt $deadline)
 if(!$ready){throw 'Current native gameplay startup did not finish; refusing wardrobe test'}
 $started=$true
 Command Defaults;Command Overall -Value 50;Command TankTop;Command Resume;Command S Hold;Start-Sleep -Seconds 2;Command S Release;Start-Sleep -Seconds 3;Command Pause
 # Defaults opens the study menu in the sealed acceptance harness. Hide it for
 # fit inspection after the front-facing turn; controls are captured separately.
 $last=(ReadLog) -split "`n"|Select-String -Pattern 'Private gameplay receipt'|Select-Object -Last 1
 if($last.Line -match 'menu=1'){Command F6}
 $bottoms=if($Jeans){@('Naked','Jockstrap','Jeans','JeansOpen')}else{@('Naked','Jockstrap')}
 foreach($top in @('TopNaked','TankTop')){foreach($bottom in $bottoms){
  Command $top;Command $bottom;Command Resume;Start-Sleep -Seconds 2;Command Pause;Capture "$top-$bottom-front"
 }}
 Command TankTop;Command $(if($Jeans){'JeansOpen'}else{'Jockstrap'})
 Command Resume;Command TurnRight Hold;Start-Sleep -Seconds 3;Command TurnRight Release;Start-Sleep -Seconds 1;Command Pause;Capture 'tank-both-oblique'
 Command Resume;Command D Hold;Start-Sleep -Milliseconds 600;Command D Release;Start-Sleep -Seconds 2;Command Pause;Capture 'tank-both-side'
 Command Resume;Command W Hold;Start-Sleep -Seconds 1;Command W Release;Start-Sleep -Seconds 2;Command Pause;Capture 'tank-both-rear'
 Command Resume;Command S Hold;Start-Sleep -Seconds 2;Command Pause;Capture 'tank-both-stride';Command S Release
 Command Resume;Command Space;Start-Sleep -Milliseconds 600;Command Pause;Capture 'tank-both-jump'
 Command Resume;Start-Sleep -Seconds 3;Command Pause;Capture 'tank-both-landed'
 Command Resume;Command F6;Start-Sleep -Seconds 1;Command Pause;Capture 'independent-selectors'
}finally{if($started){Command W Release;Command S Release;Command D Release;Command TurnRight Release;Command Pause}}
