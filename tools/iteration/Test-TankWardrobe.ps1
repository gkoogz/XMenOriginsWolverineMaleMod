param([Parameter(Mandatory=$true)][string]$Workspace,[Parameter(Mandatory=$true)][string]$RuntimeProvenance)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$run=(Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw|ConvertFrom-Json).run
$build=Get-Content -LiteralPath $RuntimeProvenance -Raw|ConvertFrom-Json
if((Get-FileHash -LiteralPath (Join-Path $root 'owned-game/Binaries/d3d9.dll')).Hash -ne $build.runtimeSHA256){throw 'Candidate identity mismatch'}
$log=Join-Path $root 'owned-game/Binaries/WolverineRuntime.log'
$out=Join-Path $run 'tank-wardrobe-cases';New-Item -ItemType Directory -Path $out -ErrorAction Stop|Out-Null
$records=@()
function Command([string]$Key,[int]$Value=50){& (Join-Path $PSScriptRoot 'Set-NativeSandboxInput.ps1') -Workspace $root -Key $Key -Value $Value|Out-Null}
function Capture([string]$Id){
 Command Capture
 $ini=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/MaleModSandboxControl.ini') -Raw
 if($ini -notmatch 'Revision=(\d+)'){throw 'Capture revision absent'};$revision=$Matches[1]
 $deadline=(Get-Date).AddSeconds(10);$file=$null
 do{if((Get-Content -LiteralPath $log -Raw) -match "Private native capture revision=$revision result=00000000"){$file=Get-ChildItem -LiteralPath $run -Filter "*-command-$revision.png"|Select-Object -First 1};if(!$file){Start-Sleep -Milliseconds 100}}while(!$file -and (Get-Date) -lt $deadline)
 if(!$file){throw "Capture missing: $Id"}
 $path=Join-Path $out ($Id+'.png');Copy-Item -LiteralPath $file.FullName -Destination $path
 $script:records+=@{id=$Id;capture=$path;sha256=(Get-FileHash -LiteralPath $path).Hash;visualReviewed=$false}
 @{schema=1;scene='licensed native WStart tank';sourceCommit=$build.sourceCommit;baseCommit=$build.baseCommit;runtimeSHA256=$build.runtimeSHA256;cases=$script:records;hostInput=$false;hostFocus=$false;nativeTankObserved=$true;fullAttachmentGatePassed=$false}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $out 'matrix.json')
 Write-Output "$Id captured"
}
try{
 $deadline=(Get-Date).AddSeconds(90)
 while((Get-Content -LiteralPath $log -Raw) -notmatch 'menu tank anatomy draw active'){
  if((Get-Date) -gt $deadline){throw 'Native tank scene did not load'};Start-Sleep -Milliseconds 250
 }
 Command Defaults;Command Resume
 foreach($top in @('TopNaked','TankTop')){foreach($bottom in @('Naked','Jockstrap','Jeans','JeansOpen')){
  Command $top;Command $bottom;Start-Sleep -Seconds 2;Capture ($top+'-'+$bottom)
 }}
 Command TankTop;Command JeansOpen
 foreach($size in @(1,25,50,75,100)){
  Command Overall $size;Command Width $size;Start-Sleep -Seconds 2;Capture ('open-size-'+$size)
 }
 Command Overall 50;Command Width 50
 foreach($state in @(1,2,3)){Command State $state;Start-Sleep -Seconds 2;Capture ('open-state-'+$state)}
 Command Defaults;Command TankTop;Command JeansOpen;Start-Sleep -Seconds 2;Capture 'final-default'
}finally{Command Resume}
