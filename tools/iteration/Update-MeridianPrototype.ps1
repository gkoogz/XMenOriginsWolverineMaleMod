param(
 [string]$Workspace=(Join-Path $env:LOCALAPPDATA 'MaleMod/MeridianPrototype'),
 [Parameter(Mandatory=$true)][string]$CandidateBuild,
 [Parameter(Mandatory=$true)][string]$NativeEvidence,
 [switch]$ValidateOnly
)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$build=(Resolve-Path -LiteralPath $CandidateBuild).Path
$game=Join-Path $root 'owned-game'
$exe=Join-Path $game 'Binaries/Wolverine.exe'
if(Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}){throw 'Close the playable prototype before updating.'}
$old=Get-Content -LiteralPath (Join-Path $root 'prototype.json') -Raw | ConvertFrom-Json
if($old.accepted -ne $false -or $old.mode -ne 'meridian-developer-play'){throw 'Only the unaccepted developer prototype can be updated.'}
foreach($property in $old.contract.PSObject.Properties){
 $path=[IO.Path]::GetFullPath((Join-Path $game $property.Name))
 if(!$path.StartsWith($game+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Invalid contract path.'}
 if((Get-FileHash -LiteralPath $path).Hash -ne $property.Value){throw "Existing prototype identity differs: $($property.Name)"}
}
$provenance=Get-Content -LiteralPath (Join-Path $build 'provenance.json') -Raw | ConvertFrom-Json
$candidate=Join-Path $build 'd3d9.dll'
$hash=(Get-FileHash -LiteralPath $candidate).Hash
if(!$provenance.meridianCandidate -or $hash -ne $provenance.runtimeSHA256){throw 'Candidate identity differs.'}
$evidence=Get-Content -LiteralPath $NativeEvidence -Raw | ConvertFrom-Json
if($evidence.runtimeSHA256 -ne $hash -or !$evidence.nativeGameplayObserved -or !$evidence.completeSampledMatrix -or $evidence.reportedDrawRejections -ne 0){throw 'Candidate lacks matching sampled native evidence.'}
foreach($capture in $evidence.reviewedCaptures){
 if((Get-FileHash -LiteralPath $capture.path).Hash -ne $capture.sha256){throw 'Reviewed capture differs.'}
}
if(@($evidence.reviewedCaptures).Count -lt 3){throw 'Front, side and oblique evidence is required.'}
$settings=@{}
foreach($name in @('WolverineLive.ini','TeachingFluid.ini')){
 $path=Join-Path $game "Binaries/$name"
 if(Test-Path -LiteralPath $path){$settings[$name]=(Get-FileHash -LiteralPath $path).Hash}
}
if($ValidateOnly){Write-Output 'Developer candidate, existing runtime and sampled native evidence verified; settings will be preserved.';return}
$backup=Join-Path $root ('rollback-performance-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item -LiteralPath (Join-Path $game 'Binaries/d3d9.dll') -Destination (Join-Path $backup 'd3d9.dll')
foreach($name in @('prototype.json','runtime-provenance.json','meridian-test-room.json','default-native.png','performance-native-evidence.json')){
 if(Test-Path -LiteralPath (Join-Path $root $name)){Copy-Item -LiteralPath (Join-Path $root $name) -Destination (Join-Path $backup $name)}
}
@'
param([switch]$ValidateOnly)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$game=Join-Path $root 'owned-game'
$exe=Join-Path $game 'Binaries/Wolverine.exe'
if(Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}){throw 'Close the prototype first.'}
$old=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'prototype.json') -Raw | ConvertFrom-Json
if((Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'd3d9.dll')).Hash -ne $old.contract.'Binaries/d3d9.dll'){throw 'Backup identity differs.'}
if($old.nativeMatrixSHA256 -and (Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'performance-native-evidence.json')).Hash -ne $old.nativeMatrixSHA256){throw 'Backup native evidence differs.'}
if($ValidateOnly){Write-Output 'Previous prototype backup verified; current settings will be preserved.';return}
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'd3d9.dll') -Destination (Join-Path $game 'Binaries/d3d9.dll') -Force
foreach($name in @('prototype.json','runtime-provenance.json','meridian-test-room.json','default-native.png','performance-native-evidence.json')){
 if(Test-Path -LiteralPath (Join-Path $PSScriptRoot $name)){Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination (Join-Path $root $name) -Force}
}
if((Get-FileHash -LiteralPath (Join-Path $game 'Binaries/d3d9.dll')).Hash -ne $old.contract.'Binaries/d3d9.dll'){throw 'Restored runtime differs.'}
Write-Output 'Previous prototype restored; current settings preserved.'
'@ | Set-Content -LiteralPath (Join-Path $backup 'Restore-PreviousPrototype.ps1')
try{
 Copy-Item -LiteralPath $candidate -Destination (Join-Path $game 'Binaries/d3d9.dll') -Force
 Copy-Item -LiteralPath (Join-Path $build 'provenance.json') -Destination (Join-Path $root 'runtime-provenance.json') -Force
 Copy-Item -LiteralPath $NativeEvidence -Destination (Join-Path $root 'performance-native-evidence.json') -Force
 $preview=$evidence.reviewedCaptures | Where-Object {$_.view -eq 'front-default'} | Select-Object -First 1
 if($preview){Copy-Item -LiteralPath $preview.path -Destination (Join-Path $root 'default-native.png') -Force}
 $old.contract.'Binaries/d3d9.dll'=$hash.ToLowerInvariant()
 $old.sourceBuild=$build
 $old.buildProvenanceSHA256=(Get-FileHash -LiteralPath (Join-Path $build 'provenance.json')).Hash
 $old.nativeMatrix=Join-Path $root 'performance-native-evidence.json'
 $old.nativeMatrixSHA256=(Get-FileHash -LiteralPath $old.nativeMatrix).Hash
 $old.rollback=$backup
 $old.settingsPreservedSHA256=$settings['WolverineLive.ini']
 $old.updatedUTC=[DateTime]::UtcNow.ToString('o')
 $old | Add-Member -Force -NotePropertyName performanceUpdate -NotePropertyValue $evidence.summary
 $old | Add-Member -Force -NotePropertyName preservedSettings -NotePropertyValue $settings
 $old.limitations=@($evidence.limitations)
 $old | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $root 'prototype.json')
 $marker=Get-Content -LiteralPath (Join-Path $root 'meridian-test-room.json') -Raw | ConvertFrom-Json
 $marker.candidateSHA256=$hash
 $marker | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $root 'meridian-test-room.json')
 foreach($name in $settings.Keys){if((Get-FileHash -LiteralPath (Join-Path $game "Binaries/$name")).Hash -ne $settings[$name]){throw 'Settings changed during update.'}}
 & (Join-Path $PSScriptRoot 'Play-MeridianPrototype.ps1') -Workspace $root -ValidateOnly | Out-Null
 & (Join-Path $backup 'Restore-PreviousPrototype.ps1') -ValidateOnly
 Write-Output "Updated developer prototype: $hash; rollback: $backup"
}catch{
 & (Join-Path $backup 'Restore-PreviousPrototype.ps1')
 throw
}
