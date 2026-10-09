param([Parameter(Mandatory=$true)][string]$Workspace,[Parameter(Mandatory=$true)][string]$RuntimeProvenance)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$run=(Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw|ConvertFrom-Json).run
$build=Get-Content -LiteralPath $RuntimeProvenance -Raw|ConvertFrom-Json
if((Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash -ne $build.runtimeSHA256){throw 'Candidate identity mismatch'}
$log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
$out=Join-Path $run 'jeans-fit-cases';New-Item -ItemType Directory -Path $out -ErrorAction Stop|Out-Null
$records=@()
function Command([string]$Key,[int]$Value=50,[string]$Action='Press'){& (Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1') -Workspace $root -Key $Key -Value $Value -Action $Action|Out-Null}
function Capture([string]$Id){
 Command Capture
 $control=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini') -Raw
 if($control -notmatch 'Revision=(\d+)'){throw 'Capture revision absent'};$revision=$Matches[1]
 $deadline=(Get-Date).AddSeconds(10);$file=$null
 do{if((Get-Content -LiteralPath $log -Raw) -match "Private native capture revision=$revision result=00000000"){$file=Get-ChildItem -LiteralPath $run -Filter "*-command-$revision.png"|Select-Object -First 1};if(!$file){Start-Sleep -Milliseconds 100}}while(!$file -and (Get-Date) -lt $deadline)
 if(!$file){throw "Capture missing: $Id"};$path=Join-Path $out ($Id+'.png');Copy-Item -LiteralPath $file.FullName -Destination $path
 $script:records+=@{id=$Id;capture=$path;sha256=(Get-FileHash -LiteralPath $path).Hash;visualReviewed=$false}
 @{schema=1;sourceCommit=$build.sourceCommit;baseCommit=$build.baseCommit;runtimeSHA256=$build.runtimeSHA256;cases=$script:records;hostInput=$false;fullAttachmentGatePassed=$false}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $out 'matrix.json')
 Write-Output "$Id captured"
}
try{
 Command Defaults;Command TankTop;Command Resume;Command S 50 Hold;Start-Sleep -Seconds 2;Command S 50 Release;Start-Sleep -Seconds 2;Command Pause
 foreach($size in @(1,25,50,75,100)){
  Command Overall $size;Command Width $size;Command JeansOpen;Command Resume;Start-Sleep -Seconds 3;Command Pause;Capture "open-size-$size"
 }
 Command Length 100;Command Scrotum 100;Command Resume;Start-Sleep -Seconds 3;Command Pause;Capture 'open-all-max'
 Command Jeans;Command Resume;Start-Sleep -Seconds 3;Command Pause;Capture 'closed-all-max'
 Command JeansOpen;Command Resume;Command Block 50 Hold;Start-Sleep -Seconds 2;Command Pause;Capture 'open-all-max-block';Command Block 50 Release
 Command Resume;Start-Sleep -Seconds 3;Command Pause;Capture 'open-all-max-recovery'
 foreach($key in @('Overall','Width','Length','Scrotum')){Command $key 50};Command Resume;Start-Sleep -Seconds 3;Command Pause
}finally{Command Block 50 Release;Command W 50 Release;Command S 50 Release;Command Pause}
