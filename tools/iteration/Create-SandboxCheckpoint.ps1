param([Parameter(Mandatory=$true)][string]$Workspace,[switch]$VisualReviewed)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$game=Join-Path $root 'owned-game';$exe=Join-Path $game 'Binaries/Wolverine.exe'
if(Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}){throw 'Close owned child first.'}
$profile=Join-Path $game 'WGame/SaveData/Player.wgameprofile'
$run=Join-Path $root ('checkpoint-'+(Get-Date -Format yyyyMMdd-HHmmss));New-Item -ItemType Directory $run | Out-Null
if(!$VisualReviewed){
 if(Test-Path $profile){throw 'Fresh checkpoint generation requires empty owned SaveData. Preserve it separately before rerunning.'}
 $saved=@{};$flags=@{MALEMOD_ISOLATED_D3D9EX='1';MALEMOD_PRIVATE_MENU='engine-default';MALEMOD_PRIVATE_CAPTURE=(Join-Path $run 'default');MALEMOD_PRIVATE_TIMEOUT_MS='20000';MALEMOD_PRIVATE_START_PAUSED='0';MALEMOD_PRIVATE_TEST_CONTROLS='0';MALEMOD_TRACE_BOOT_IO='0'}
 foreach($k in $flags.Keys){$saved[$k]=[Environment]::GetEnvironmentVariable($k,'Process');[Environment]::SetEnvironmentVariable($k,$flags[$k],'Process')}
 try{& (Join-Path $root 'launch_capability_restore.exe') (Join-Path $run 'engine.dmp') $exe -windowed -ResX=640 -ResY=480 -unattended -nohomedir -seekfreeloading *> (Join-Path $run 'guard.txt')}finally{foreach($k in $flags.Keys){[Environment]::SetEnvironmentVariable($k,$saved[$k],'Process')}}
 if(!(Test-Path $profile)){throw 'Engine did not generate its default owned profile.'}
 & python (Join-Path $PSScriptRoot 'seed_checkpoint.py') --workspace $root *> (Join-Path $run 'seed.txt')
 if($LASTEXITCODE){throw 'Typed synthetic reference checkpoint could not be seeded.'}
 $flags.MALEMOD_PRIVATE_MENU='new-game';$flags.MALEMOD_PRIVATE_TIMEOUT_MS='90000';$flags.MALEMOD_PRIVATE_CAPTURE=Join-Path $run 'native'
 foreach($k in $flags.Keys){[Environment]::SetEnvironmentVariable($k,$flags[$k],'Process')}
 try{& (Join-Path $root 'launch_capability_restore.exe') (Join-Path $run 'reference.dmp') $exe -windowed -ResX=640 -ResY=480 -unattended -nohomedir -seekfreeloading *> (Join-Path $run 'reference-guard.txt')}finally{foreach($k in $flags.Keys){[Environment]::SetEnvironmentVariable($k,$saved[$k],'Process')}}
 $log=Get-Content -LiteralPath (Join-Path $game 'Binaries/WolverineRuntime.log') -Raw
 if($log -notmatch 'Private gameplay receipt frame=1 ' -or !(Test-Path $profile)){throw 'Fresh native checkpoint failed; inspect owned guard/log/captures.'}
 Copy-Item $profile (Join-Path $game 'WGame/SaveData-ground-control/Player.wgameprofile')
 @{'schema'=1;'profileGeneratedByEngine'=$true;'syntheticTypedReferenceRecipe'='profile-seed.json';'privateUserProfileCopied'=$false;'nativeGameplayObserved'=$true;'visualReviewPassed'=$false;'run'=$run} | ConvertTo-Json | Set-Content (Join-Path $root 'checkpoint-proof.json')
 Write-Output 'Review generated native captures, then run this script with -VisualReviewed.'
 return
}
$proof=Get-Content (Join-Path $root 'checkpoint-proof.json') -Raw | ConvertFrom-Json
if(!$proof.profileGeneratedByEngine -or !$proof.nativeGameplayObserved){throw 'Native fresh checkpoint proof required.'}
$proof.visualReviewPassed=$true
$proof | ConvertTo-Json | Set-Content (Join-Path $root 'checkpoint-proof.json')
$contract=@{}
foreach($rel in @('Binaries/Wolverine.exe','Binaries/d3d9.dll','WGame/CookedPC/jungle1_zone01a.xxx','WGame/CookedPC/jungle1_kismet01.xxx','WGame/SaveData-ground-control/Player.wgameprofile')){$contract[$rel]=(Get-FileHash -LiteralPath (Join-Path $game $rel)).Hash}
@{schema=1;nativeGameplayVerified=$true;visualReviewPassed=$true;contract=$contract;checkpointProof='checkpoint-proof.json'} | ConvertTo-Json -Depth 4 | Set-Content (Join-Path $root 'sandbox-contract.json')
Write-Output 'Reviewed owned contract frozen. Prepare playable storage or start sealed iteration.'
