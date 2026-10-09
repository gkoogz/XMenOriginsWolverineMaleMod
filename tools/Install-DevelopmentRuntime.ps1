param(
 [string]$GamePath='C:/Games/X-Men Origins Wolverine',
 [Parameter(Mandatory=$true)][string]$CandidateBuild,
 [Parameter(Mandatory=$true)][string]$NativeEvidence,
 [string]$PreviousDevelopmentBackup,
 [switch]$Development,
 [switch]$ValidateOnly
)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$game=(Resolve-Path -LiteralPath $GamePath).Path
$build=(Resolve-Path -LiteralPath $CandidateBuild).Path
$target=Join-Path $game 'Binaries/d3d9.dll'
function Hash($path){(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
if(Get-Process -Name Wolverine -ErrorAction SilentlyContinue){throw 'Close Wolverine before installing or restoring.'}
if(!(Test-Path -LiteralPath (Join-Path $game 'Binaries/Wolverine.exe'))){throw 'Unsupported game path.'}
$release=Get-Content -LiteralPath (Join-Path $repo 'manifest.json') -Raw | ConvertFrom-Json
$pin=Get-Content -LiteralPath (Join-Path $repo 'dependencies/base.lock.json') -Raw | ConvertFrom-Json
$production=Get-Content -LiteralPath (Join-Path $build 'provenance.json') -Raw | ConvertFrom-Json
$native=Get-Content -LiteralPath $NativeEvidence -Raw | ConvertFrom-Json
$candidate=Join-Path $build 'd3d9.dll'
if($production.schema -ne 'wolverine.production-build/1' -or $production.dirtySourceIncluded -or $production.privateFactoryIncluded -or $production.sandboxInputHooksIncluded -or $production.sandboxWorldOriginIncluded){throw 'Only a clean production runtime can be installed here.'}
if((Hash $candidate) -ne $production.runtimeSHA256 -or $production.baseCommit -ne $pin.commit){throw 'Production runtime or Base pin mismatch.'}
if($native.sourceCommit -ne $production.sourceCommit -or $native.baseCommit -ne $production.baseCommit -or !$native.nativeGameplayObserved -or !$native.completeSampledMatrix -or $native.reportedDrawRejections -ne 0){throw 'Matching sampled native source evidence is required.'}
if(!$native.fullAttachmentGatePassed -and !$Development){throw 'Full attachment acceptance is incomplete. Only an explicitly selected development installation can proceed, retaining the accepted runtime.'}
$currentHash=Hash $target;$previous=$null
if($currentHash -ne $release.runtimeSHA256){
 if(!$PreviousDevelopmentBackup){throw 'Supply the exact prior development backup to reconcile this installed runtime.'}
 $previous=(Resolve-Path -LiteralPath $PreviousDevelopmentBackup).Path
 $backupRoot=(Resolve-Path -LiteralPath (Join-Path $game 'WGame/ModBackups')).Path.TrimEnd('\')+'\'
 if(!$previous.StartsWith($backupRoot,[StringComparison]::OrdinalIgnoreCase)){throw 'Previous development receipt must belong to this game.'}
 $prior=Get-Content -LiteralPath (Join-Path $previous 'state.json') -Raw | ConvertFrom-Json
 if($prior.status -ne 'Installed' -or $prior.mode -ne 'user-requested-development-update' -or $prior.target -ne $target -or $prior.installedSHA256 -ne $currentHash){throw 'Previous development receipt does not identify the installed runtime.'}
 if((Hash (Join-Path $previous 'd3d9.dll')) -ne $prior.originalSHA256){throw 'Previous exact rollback has changed.'}
 if($prior.originalSHA256 -ne $release.runtimeSHA256 -and $prior.acceptedBaselineSHA256 -ne $release.runtimeSHA256){throw 'Previous update is not tied to the accepted release.'}
}
foreach($pair in @(
 @('WGame/CookedPC/CH_Wolverine_Natural_SF.xxx',$release.installedPackageSHA256),
 @('WGame/CookedPC/WGame.xxx',$release.installedWGameSHA256),
 @('WGame/CookedPC/WStart.xxx',$release.installedWStartSHA256)
)) {if((Hash (Join-Path $game $pair[0])) -ne $pair[1]){throw "Installed package differs: $($pair[0])"}}
foreach($name in @('R14-skin-natural.dds','R14-skin-erect.dds','SharedBody-normal.dds','SharedBody-specular.dds','MenuTank-skin-blend33.png','MenuTank-skin-blend67.png')){
 if((Hash (Join-Path $game ('Binaries/'+$name))) -ne $release.payload.$name){throw "Installed material differs: $name"}
}
if((Hash $native.matrix) -ne $native.matrixSHA256){throw 'Native regression receipt changed.'}
foreach($capture in $native.reviewedCaptures){if((Hash $capture.path) -ne $capture.sha256.ToLowerInvariant()){throw 'Reviewed native capture changed.'}}
if(@($native.reviewedCaptures).Count -lt 3){throw 'Reviewed front, side and oblique captures required.'}
$protected=@{}
foreach($relative in @('Binaries/WolverineLive.ini','Binaries/TeachingFluid.ini','Binaries/TankCameraPresets.txt','WGame/Config/DefaultCheckpoints.ini')){
 $path=Join-Path $game $relative;if(Test-Path -LiteralPath $path){$protected[$path]=Hash $path}
}
foreach($folder in @((Join-Path $game 'Binaries/WolverineIdle'),(Join-Path $game 'WGame/SaveData'),(Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Wolverine/WGame'))){
 if(Test-Path -LiteralPath $folder){Get-ChildItem -LiteralPath $folder -File -Recurse | ForEach-Object {$protected[$_.FullName]=Hash $_.FullName}}
}
if($ValidateOnly){Write-Output 'Production identity, accepted baseline, materials and sampled native evidence verified. No files changed.';return}
$backup=Join-Path $game ('WGame/ModBackups/Meridian-development-'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'-'+[guid]::NewGuid().ToString('N').Substring(0,6))
if(Test-Path -LiteralPath $backup){throw 'Retained backup already exists.'}
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item -LiteralPath $target -Destination (Join-Path $backup 'd3d9.dll')
$oldHash=Hash $target;$readOnly=(Get-Item -LiteralPath $target).IsReadOnly
Copy-Item -LiteralPath (Join-Path $build 'provenance.json') -Destination (Join-Path $backup 'production-provenance.json')
Copy-Item -LiteralPath $NativeEvidence -Destination (Join-Path $backup 'native-evidence.json')
$state=[ordered]@{schema=1;mode='user-requested-development-update';target=$target;originalSHA256=$oldHash;installedSHA256=$production.runtimeSHA256;sourceCommit=$production.sourceCommit;baseCommit=$production.baseCommit;originalReadOnly=$readOnly;fullAttachmentGatePassed=[bool]$native.fullAttachmentGatePassed;acceptedRelease=$false;acceptedBaselineSHA256=$release.runtimeSHA256;previousDevelopmentBackup=$previous;status='Pending';protectedFiles=$protected}
$statePath=Join-Path $backup 'state.json'
$state | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $statePath
@'
param([switch]$ValidateOnly)
$ErrorActionPreference='Stop'
$statePath=Join-Path $PSScriptRoot 'state.json'
$state=Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
function Hash($path){(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
if(Get-Process -Name Wolverine -ErrorAction SilentlyContinue){throw 'Close Wolverine before restoring.'}
$backup=Join-Path $PSScriptRoot 'd3d9.dll'
if((Hash $backup) -ne $state.originalSHA256){throw 'Retained runtime checksum mismatch.'}
if((Hash $state.target) -ne $state.installedSHA256){throw 'Runtime changed since this installation; refusing to overwrite another update.'}
if($ValidateOnly){Write-Output 'Exact rollback source and installed runtime verified.';return}
(Get-Item -LiteralPath $state.target).IsReadOnly=$false
Copy-Item -LiteralPath $backup -Destination $state.target -Force
(Get-Item -LiteralPath $state.target).IsReadOnly=[bool]$state.originalReadOnly
if((Hash $state.target) -ne $state.originalSHA256){throw 'Restored checksum mismatch.'}
$state.status='Restored';$state | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $statePath
Write-Output 'Previous runtime restored exactly. Saves, settings and audio preserved.'
'@ | Set-Content -LiteralPath (Join-Path $backup 'Restore.ps1')
$pending=$target+'.meridian-pending'
try{
 Copy-Item -LiteralPath $candidate -Destination $pending
 if((Hash $pending) -ne $production.runtimeSHA256){throw 'Staged runtime checksum mismatch.'}
 (Get-Item -LiteralPath $target).IsReadOnly=$false
 [IO.File]::Replace($pending,$target,[NullString]::Value,$true)
 (Get-Item -LiteralPath $target).IsReadOnly=$readOnly
 if((Hash $target) -ne $production.runtimeSHA256){throw 'Installed runtime checksum mismatch.'}
 foreach($entry in $protected.GetEnumerator()){if((Hash $entry.Key) -ne $entry.Value){throw 'Protected settings/save/audio file changed.'}}
 $state.status='Installed';$state.installedUTC=(Get-Date).ToUniversalTime().ToString('o')
 $state | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $statePath
 & (Join-Path $backup 'Restore.ps1') -ValidateOnly
 Write-Output "Installed development runtime: $($production.runtimeSHA256)"
 Write-Output "Exact rollback: $(Join-Path $backup 'Restore.ps1')"
}catch{
 (Get-Item -LiteralPath $target).IsReadOnly=$false
 Copy-Item -LiteralPath (Join-Path $backup 'd3d9.dll') -Destination $target -Force
 (Get-Item -LiteralPath $target).IsReadOnly=$readOnly
 $state.status='FailedAndRestored';$state | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $statePath
 throw
}finally{if(Test-Path -LiteralPath $pending){Remove-Item -LiteralPath $pending}}
