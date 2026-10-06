param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004',[string]$StateDirectory=(Join-Path $env:LOCALAPPDATA 'MaleMod/WolverineSandbox'))
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$state=[IO.Path]::GetFullPath($StateDirectory)
$pendingPath=Join-Path $state 'pending-stage.json'
if(!(Test-Path -LiteralPath $pendingPath)) {return}
$pending=Get-Content -LiteralPath $pendingPath -Raw | ConvertFrom-Json
if($pending.schema -ne 1 -or !$pending.nativeGameplayVerified) {throw 'Pending stage has not passed its native verification.'}
$candidate=[IO.Path]::GetFullPath($pending.candidate)
if(!$candidate.StartsWith($state+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) {throw 'Candidate must stay in private C: storage.'}
$target=Join-Path $root 'owned-game/WGame/CookedPC/jungle1_zone01a.xxx'
$ownedExes=@((Join-Path $root 'owned-game/Binaries/Wolverine.exe'),(Join-Path $state 'human-game/Binaries/Wolverine.exe'))
if(Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -in $ownedExes}) {throw 'Close the owned sandbox before applying its next stage.'}
$candidateHash=(Get-FileHash -LiteralPath $candidate -Algorithm SHA256).Hash
if($candidateHash -ne $pending.afterSHA256) {throw 'Candidate stage hash mismatch.'}
$current=(Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash
if($current -ne $pending.beforeSHA256 -and $current -ne $candidateHash) {throw 'Installed stage differs from the recorded update boundary.'}
if((Get-Item -LiteralPath $candidate).Length -ne 32768 -or (Get-Item -LiteralPath $target).Length -ne 32768) {throw 'Expected the verified equal-size 32KiB stage replacement.'}
if($current -ne $candidateHash) {
 $backup=Join-Path $state 'stage-before-studio.xxx'
 if(!(Test-Path -LiteralPath $backup)) {Copy-Item -LiteralPath $target -Destination $backup}
 if((Get-FileHash -LiteralPath $backup -Algorithm SHA256).Hash -ne $pending.beforeSHA256) {throw 'Original stage backup mismatch.'}
 Copy-Item -LiteralPath $candidate -Destination $target -Force
 if((Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -ne $candidateHash) {throw 'Applied stage hash mismatch.'}
}
@{schema=1;stage=$pending.stage;floorSHA256=$candidateHash;nativeGameplayVerified=$true;appliedUTC=[DateTime]::UtcNow.ToString('o');retailTouched=$false;settingsChanged=$false} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $state 'active-stage.json')
Write-Output 'Studio lighting/backdrop applied to the owned native stage.'
