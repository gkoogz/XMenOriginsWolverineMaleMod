#requires -Version 5.1
[CmdletBinding()]
param([string]$GamePath='C:\Games\X-Men Origins Wolverine',[switch]$Rollback)
$ErrorActionPreference='Stop'
Set-StrictMode -Version 2
$GamePath=[IO.Path]::GetFullPath($GamePath.Trim('"'))
$manifest=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'upgrade-0.8.json') -Raw | ConvertFrom-Json
$exe=Join-Path $GamePath 'Binaries\Wolverine.exe'
$target=Join-Path $GamePath 'Binaries\d3d9.dll'
$package=Join-Path $GamePath 'WGame\CookedPC\CH_Wolverine_Natural_SF.xxx'
$backupDir=Join-Path $GamePath 'WGame\ModBackups\WolverineAnatomyTool-v0.8-upgrade'
$backup=Join-Path $backupDir 'previous-runtime.dll'
$statePath=Join-Path $backupDir 'upgrade-state.json'
function Hash([string]$p){(Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash}
if(-not(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Select a game folder containing Binaries\Wolverine.exe.'}
if(Get-Process -Name Wolverine -ErrorAction SilentlyContinue){throw 'Close Wolverine before upgrading or rolling back.'}
if((Hash $package) -ne $manifest.packageSHA256){throw 'This upgrade requires the exact Revision 161 character package. No files were changed.'}
if($Rollback){
    if(-not(Test-Path -LiteralPath $statePath)){throw 'No v0.8 upgrade backup was found.'}
    $state=Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
    if($state.GamePath -ne $GamePath -or (Hash $backup) -ne $state.PreviousHash){throw 'Backup identity or checksum mismatch.'}
    $current=Hash $target
    if($current -eq $state.PreviousHash){Write-Output 'The previous runtime is already restored.';return}
    if($current -ne $manifest.runtimeSHA256){throw 'Runtime changed after upgrade; rollback stopped to preserve that change.'}
    (Get-Item -LiteralPath $target).IsReadOnly=$false
    [IO.File]::WriteAllBytes($target,[IO.File]::ReadAllBytes($backup))
    (Get-Item -LiteralPath $target).IsReadOnly=[bool]$state.OriginalReadOnly
    if((Hash $target) -ne $state.PreviousHash){throw 'Restoration verification failed.'}
    $state.Status='RolledBack';$state | ConvertTo-Json | Set-Content -LiteralPath $statePath -Encoding UTF8
    Write-Output 'Previous runtime restored. Character package and saved settings were preserved.';return
}
$source=Join-Path $PSScriptRoot 'payload\d3d9.dll'
if((Hash $source) -ne $manifest.runtimeSHA256){throw 'The v0.8 runtime checksum does not match the manifest.'}
$current=Hash $target
if($current -eq $manifest.runtimeSHA256){Write-Output 'v0.8 is already installed.';return}
if($manifest.acceptedRuntimeSHA256 -notcontains $current){throw 'This upgrade requires a supported v0.7-series runtime. No files were changed.'}
New-Item -ItemType Directory -Path $backupDir -Force | Out-Null
if(Test-Path -LiteralPath $backup){if((Hash $backup) -ne $current){throw 'Existing backup is incompatible; it was preserved.'}}
else{Copy-Item -LiteralPath $target -Destination $backup}
$settings=Join-Path $GamePath 'Binaries\WolverineLive.ini'
$settingsBackup=Join-Path $backupDir 'WolverineLive-before-v0.8.ini'
if((Test-Path -LiteralPath $settings) -and -not(Test-Path -LiteralPath $settingsBackup)){Copy-Item -LiteralPath $settings -Destination $settingsBackup}
$readOnly=(Get-Item -LiteralPath $target).IsReadOnly
$state=@{Version='0.8';GamePath=$GamePath;Status='Pending';OriginalReadOnly=$readOnly;PreviousHash=$current;InstalledHash=$manifest.runtimeSHA256}
$state | ConvertTo-Json | Set-Content -LiteralPath $statePath -Encoding UTF8
try{
    (Get-Item -LiteralPath $target).IsReadOnly=$false
    [IO.File]::WriteAllBytes($target,[IO.File]::ReadAllBytes($source))
    if((Hash $target) -ne $manifest.runtimeSHA256){throw 'Installed runtime checksum failed.'}
    (Get-Item -LiteralPath $target).IsReadOnly=$readOnly
    $state.Status='Installed';$state | ConvertTo-Json | Set-Content -LiteralPath $statePath -Encoding UTF8
}catch{
    [IO.File]::WriteAllBytes($target,[IO.File]::ReadAllBytes($backup));(Get-Item -LiteralPath $target).IsReadOnly=$readOnly
    throw
}
Write-Output 'v0.8 installed. Launch normally; F6 shows R33 DISTINCT STATES. Package and saved settings were preserved.'
