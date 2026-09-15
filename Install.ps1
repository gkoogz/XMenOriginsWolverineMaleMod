#requires -Version 5.1
[CmdletBinding()]
param(
    [ValidateSet('Install','Uninstall')][string]$Mode='Install',
    [string]$GamePath,
    [string]$DocumentsPath=[Environment]::GetFolderPath('MyDocuments')
)

$ErrorActionPreference='Stop'
Set-StrictMode -Version 2
$version='0.5'
$sourceHash='6C7F2551F7022FE66CB2483C31DF2BF837E487B56E73DC6FC34F305E167AA79F'
$targetHash='4290BD9CB75C27401C56A8AF54679C8AD1D9FC0FFD41560B7BE2324D52288DF2'
$patchHash='069C4F0F78CAAE2AE173057D0EFAE039BEEC60C23410555F31745B2BFCC86370'
$runtimeHash='A2D35D83259FA1A931F663B27FBFF57B372181843FB3CA466FE569B7EF8D89AF'

function Hash([string]$Path) {
    (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}

function Write-Bytes([string]$Path,[byte[]]$Bytes,[bool]$ReadOnly) {
    if(Test-Path -LiteralPath $Path){(Get-Item -LiteralPath $Path).IsReadOnly=$false}
    try {[IO.File]::WriteAllBytes($Path,$Bytes)}
    finally {if(Test-Path -LiteralPath $Path){(Get-Item -LiteralPath $Path).IsReadOnly=$ReadOnly}}
}

function Resolve-GamePath([string]$Requested) {
    if($Requested){return [IO.Path]::GetFullPath($Requested.Trim('"'))}
    $candidates=@(
        'C:\Games\X-Men Origins Wolverine',
        (Join-Path ${env:ProgramFiles(x86)} 'Steam\steamapps\common\X-Men Origins Wolverine'),
        (Join-Path $env:ProgramFiles 'Steam\steamapps\common\X-Men Origins Wolverine')
    ) | Where-Object {$_}
    foreach($candidate in $candidates){
        if(Test-Path -LiteralPath (Join-Path $candidate 'Binaries\Wolverine.exe')){
            return [IO.Path]::GetFullPath($candidate)
        }
    }
    return [IO.Path]::GetFullPath((Read-Host 'Game folder containing Binaries and WGame').Trim('"'))
}

$GamePath=Resolve-GamePath $GamePath
$DocumentsPath=[IO.Path]::GetFullPath($DocumentsPath)
$gameExe=[IO.Path]::GetFullPath((Join-Path $GamePath 'Binaries\Wolverine.exe'))
if(-not(Test-Path -LiteralPath $gameExe -PathType Leaf)){
    throw 'Wolverine.exe was not found. Select the X-Men Origins Wolverine installation folder.'
}
if(Get-Process -Name Wolverine -ErrorAction SilentlyContinue | Where-Object {$_.Path -and [IO.Path]::GetFullPath($_.Path) -eq $gameExe}){
    throw 'Close X-Men Origins: Wolverine before installing or uninstalling.'
}

$packageTarget=[IO.Path]::GetFullPath((Join-Path $GamePath 'WGame\CookedPC\CH_Wolverine_Natural_SF.xxx'))
$runtimeTarget=[IO.Path]::GetFullPath((Join-Path $GamePath 'Binaries\d3d9.dll'))
$settingsTarget=[IO.Path]::GetFullPath((Join-Path $GamePath 'Binaries\WolverineLive.ini'))
$defaultCheckpoints=[IO.Path]::GetFullPath((Join-Path $GamePath 'WGame\Config\DefaultCheckpoints.ini'))
$playerCheckpoints=[IO.Path]::GetFullPath((Join-Path $DocumentsPath 'Wolverine\WGame\Config\WCheckpoints.ini'))
$patchSource=Join-Path $PSScriptRoot 'payload\Natural.wbx'
$runtimeSource=Join-Path $PSScriptRoot 'payload\d3d9.dll'
$backupRoot=[IO.Path]::GetFullPath((Join-Path $GamePath 'WGame\ModBackups\BigDickLoganMod-v0.5'))
$statePath=Join-Path $backupRoot 'state.json'

if(-not $packageTarget.StartsWith($GamePath,[StringComparison]::OrdinalIgnoreCase) -or
   -not $runtimeTarget.StartsWith($GamePath,[StringComparison]::OrdinalIgnoreCase) -or
   -not $backupRoot.StartsWith($GamePath,[StringComparison]::OrdinalIgnoreCase)){
    throw 'Resolved installation path escaped the selected game directory.'
}

if($Mode -eq 'Uninstall') {
    if(-not(Test-Path -LiteralPath $statePath)){throw "No v0.5 installer state exists at $backupRoot."}
    $state=Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
    if($state.Version -ne $version -or [IO.Path]::GetFullPath($state.GamePath) -ne $GamePath){throw 'Backup state belongs to a different version or game folder.'}
    if($state.Status -eq 'Uninstalled'){Write-Output 'Big Dick Logan Mod v0.5 is already uninstalled. Backups were retained.';return}
    foreach($record in @($state.Files)) {
        $target=[string]$record.Target
        $backup=[string]$record.Backup
        if(-not(Test-Path -LiteralPath $backup)){throw "Required backup is missing: $backup"}
        if((Hash $backup) -ne $record.OriginalHash){throw "Backup checksum failed: $backup"}
        if(Test-Path -LiteralPath $target){
            $current=Hash $target
            if($record.InstalledHash -and $current -ne $record.InstalledHash){throw "File changed after installation and will not be overwritten automatically: $target"}
        }
        Write-Bytes $target ([IO.File]::ReadAllBytes($backup)) ([bool]$record.OriginalReadOnly)
        if((Hash $target) -ne $record.OriginalHash){throw "Restore verification failed: $target"}
    }
    foreach($record in @($state.OptionalFiles)) {
        $target=[string]$record.Target
        if($record.OriginalExisted){
            $backup=[string]$record.Backup
            if(-not(Test-Path -LiteralPath $backup) -or (Hash $backup) -ne $record.OriginalHash){throw "Optional backup verification failed: $backup"}
            if((Test-Path -LiteralPath $target) -and -not $record.Mutable -and $record.InstalledHash -and (Hash $target) -ne $record.InstalledHash){throw "File changed after installation and will not be overwritten automatically: $target"}
            Write-Bytes $target ([IO.File]::ReadAllBytes($backup)) ([bool]$record.OriginalReadOnly)
        } elseif(Test-Path -LiteralPath $target) {
            if(-not $record.Mutable -and $record.InstalledHash -and (Hash $target) -ne $record.InstalledHash){throw "File changed after installation and will not be removed automatically: $target"}
            Remove-Item -LiteralPath $target
        }
    }
    $state.Status='Uninstalled'
    $state | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $statePath -Encoding UTF8
    Write-Output 'Big Dick Logan Mod v0.5 uninstalled. Original files restored; verified backups retained.'
    return
}

if(-not(Test-Path -LiteralPath $packageTarget -PathType Leaf)){throw "Missing Natural character package: $packageTarget"}
if((Hash $packageTarget) -ne $sourceHash){
    if((Hash $packageTarget) -eq $targetHash){throw 'The v0.5 character package is already present, but no matching installer state exists. Restore the original package before using this installer.'}
    throw 'Unsupported or modified Natural character package. Verify/restore the original PC game file before installing.'
}
foreach($payload in @($patchSource,$runtimeSource,(Join-Path $PSScriptRoot 'tools\PatchCodec.cs'))){if(-not(Test-Path -LiteralPath $payload -PathType Leaf)){throw "Release payload is incomplete: $payload"}}
if((Hash $patchSource) -ne $patchHash){throw 'Character patch checksum failed. Download or extract a fresh release.'}
if((Hash $runtimeSource) -ne $runtimeHash){throw 'Runtime checksum failed. Download or extract a fresh release.'}
if(Test-Path -LiteralPath $statePath){
    $old=Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
    if($old.Status -eq 'Installed'){throw 'Installer state says v0.5 is already installed.'}
    if($old.Status -ne 'Uninstalled' -or [IO.Path]::GetFullPath($old.GamePath) -ne $GamePath){throw "An incompatible or interrupted backup exists at $backupRoot. Retain it and resolve that installation before retrying."}
    Write-Output 'A verified prior uninstall was found; retained originals will be reused.'
}

if(-not('WolverinePatchCodec' -as [type])){Add-Type -Path (Join-Path $PSScriptRoot 'tools\PatchCodec.cs')}
$stage=Join-Path ([IO.Path]::GetTempPath()) ('BigDickLoganMod-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $stage | Out-Null
try {
    $stagedPackage=Join-Path $stage 'CH_Wolverine_Natural_SF.xxx'
    [WolverinePatchCodec]::Apply($packageTarget,$patchSource,$stagedPackage)
    if((Hash $stagedPackage) -ne $targetHash){throw 'Reconstructed character package checksum failed; no game files were changed.'}

    $encoding=[Text.Encoding]::GetEncoding(28591)
    $configChanges=@()
    foreach($config in @($defaultCheckpoints,$playerCheckpoints)) {
        if(-not(Test-Path -LiteralPath $config -PathType Leaf)){continue}
        $text=$encoding.GetString([IO.File]::ReadAllBytes($config))
        $pattern='(?m)^([ \t]*mOutfit[ \t]*=[ \t]*)CH_Wolverine_[A-Za-z0-9_]+([ \t]*\r?)$'
        $count=[regex]::Matches($text,$pattern).Count
        if($count -gt 0){
            $changed=$encoding.GetBytes([regex]::Replace($text,$pattern,'${1}CH_Wolverine_Natural${2}'))
            $configChanges+=@{Target=$config;Bytes=$changed;Count=$count}
        }
    }

    $requiredTargets=@($packageTarget)+@($configChanges|ForEach-Object {$_.Target})
    foreach($target in $requiredTargets){
        $readOnly=(Get-Item -LiteralPath $target).IsReadOnly
        try{(Get-Item -LiteralPath $target).IsReadOnly=$false;$stream=[IO.File]::Open($target,'Open','ReadWrite','None');$stream.Dispose()}
        finally{(Get-Item -LiteralPath $target).IsReadOnly=$readOnly}
    }

    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    $files=@()
    foreach($target in $requiredTargets){
        $safeName=if($target -eq $packageTarget){'CH_Wolverine_Natural_SF.original'}else{([IO.Path]::GetFileName($target)+'.original')}
        $backup=Join-Path $backupRoot $safeName
        if(Test-Path -LiteralPath $backup){
            if((Hash $backup) -ne (Hash $target)){throw "Retained backup differs from the current original: $backup"}
        } else {Copy-Item -LiteralPath $target -Destination $backup}
        $files+=@{Target=$target;Backup=$backup;OriginalHash=(Hash $target);OriginalReadOnly=(Get-Item -LiteralPath $target).IsReadOnly;InstalledHash=''}
    }
    $optional=@()
    foreach($target in @($runtimeTarget,$settingsTarget)){
        $exists=Test-Path -LiteralPath $target -PathType Leaf
        $backup=Join-Path $backupRoot (([IO.Path]::GetFileName($target))+'.original')
        $hash=if($exists){Hash $target}else{''}
        $readOnly=if($exists){(Get-Item -LiteralPath $target).IsReadOnly}else{$false}
        if($exists){
            if(Test-Path -LiteralPath $backup){
                if((Hash $backup) -ne $hash){throw "Retained optional backup differs from the current original: $backup"}
            } else {Copy-Item -LiteralPath $target -Destination $backup}
        }
        $optional+=@{Target=$target;Backup=$backup;OriginalExisted=$exists;OriginalHash=$hash;OriginalReadOnly=$readOnly;InstalledHash='';Mutable=($target -eq $settingsTarget)}
    }
    $state=@{Version=$version;GamePath=$GamePath;DocumentsPath=$DocumentsPath;Status='Pending';Files=$files;OptionalFiles=$optional}
    $state | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $statePath -Encoding UTF8

    try {
        Write-Bytes $packageTarget ([IO.File]::ReadAllBytes($stagedPackage)) ([bool]$files[0].OriginalReadOnly)
        $files[0].InstalledHash=Hash $packageTarget
        if($files[0].InstalledHash -ne $targetHash){throw 'Installed character package verification failed.'}
        for($i=0;$i -lt $configChanges.Count;$i++){
            $record=$files[$i+1]
            Write-Bytes $record.Target $configChanges[$i].Bytes ([bool]$record.OriginalReadOnly)
            $record.InstalledHash=Hash $record.Target
            Write-Output "Updated $($configChanges[$i].Count) checkpoint outfit entries in $($record.Target)."
        }
        Write-Bytes $runtimeTarget ([IO.File]::ReadAllBytes($runtimeSource)) $false
        $optional[0].InstalledHash=Hash $runtimeTarget
        if($optional[0].InstalledHash -ne $runtimeHash){throw 'Installed runtime verification failed.'}
        # Existing settings are deliberately preserved. If none exist, the runtime creates them on first launch.
        if(Test-Path -LiteralPath $settingsTarget){$optional[1].InstalledHash=Hash $settingsTarget}
        $state.Status='Installed'
        $state | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $statePath -Encoding UTF8
    } catch {
        $failure=$_
        foreach($record in $files){Write-Bytes $record.Target ([IO.File]::ReadAllBytes($record.Backup)) ([bool]$record.OriginalReadOnly)}
        foreach($record in $optional){
            if($record.OriginalExisted){Write-Bytes $record.Target ([IO.File]::ReadAllBytes($record.Backup)) ([bool]$record.OriginalReadOnly)}
            elseif(Test-Path -LiteralPath $record.Target){Remove-Item -LiteralPath $record.Target}
        }
        $state.Status='Uninstalled'
        $state | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $statePath -Encoding UTF8
        throw $failure
    }
    Write-Output 'Big Dick Logan Mod v0.5 installed. Launch normally and press F6 for the menu.'
    Write-Output "Verified backups: $backupRoot"
} finally {
    $tempFile=Join-Path $stage 'CH_Wolverine_Natural_SF.xxx'
    if(Test-Path -LiteralPath $tempFile){Remove-Item -LiteralPath $tempFile -Force}
    if(Test-Path -LiteralPath $stage){Remove-Item -LiteralPath $stage}
}
