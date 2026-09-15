param([Parameter(Mandatory=$true)][string]$OriginalPackage)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$manifest=Get-Content (Join-Path $repo 'manifest.json') -Raw | ConvertFrom-Json
if((Get-FileHash $OriginalPackage).Hash -ne $manifest.sourcePackageSHA256){throw 'Wrong original package'}
$fixture=Join-Path ([IO.Path]::GetTempPath()) ('Wolverine-release-test-'+[Guid]::NewGuid().ToString('N'))
$game=Join-Path $fixture 'Game with spaces'
$docs=Join-Path $fixture 'Documents'
foreach($dir in @('Binaries','WGame/CookedPC','WGame/Config')){New-Item -ItemType Directory -Path (Join-Path $game $dir) -Force | Out-Null}
New-Item -ItemType Directory -Path (Join-Path $docs 'Wolverine/WGame/Config') -Force | Out-Null
$package=Join-Path $game 'WGame/CookedPC/CH_Wolverine_Natural_SF.xxx'
Copy-Item $OriginalPackage $package
$fixtures=@{
 (Join-Path $game 'Binaries/Wolverine.exe')='fixture executable, never launched';
 (Join-Path $game 'Binaries/d3d9.dll')='existing proxy fixture';
 (Join-Path $game 'Binaries/WolverineLive.ini')="[Shape]`r`nOverall=57`r`n";
 (Join-Path $game 'WGame/Config/DefaultCheckpoints.ini')="[Checkpoint]`r`nmOutfit=CH_Wolverine_Tank`r`n";
 (Join-Path $docs 'Wolverine/WGame/Config/WCheckpoints.ini')="[Checkpoint]`r`nmOutfit=CH_Wolverine_Tank`r`n"
}
foreach($entry in $fixtures.GetEnumerator()){[IO.File]::WriteAllText($entry.Key,$entry.Value)}
$before=@{}
foreach($file in (@($fixtures.Keys)+@($package))){$before[$file]=(Get-FileHash $file).Hash}
& (Join-Path $repo 'Install.ps1') -GamePath $game -DocumentsPath $docs
if((Get-FileHash $package).Hash -ne $manifest.installedPackageSHA256){throw 'Installed package mismatch'}
$dll=Join-Path $game 'Binaries/d3d9.dll'
if((Get-FileHash $dll).Hash -ne $manifest.runtimeSHA256){throw 'Installed DLL mismatch'}
$ini=Join-Path $game 'Binaries/WolverineLive.ini'
if((Get-FileHash $ini).Hash -ne $before[$ini]){throw 'Settings changed'}
foreach($file in $fixtures.Keys | Where-Object {$_ -match 'Checkpoints.ini$'}){
 if((Get-Content $file -Raw) -notmatch 'mOutfit=CH_Wolverine_Natural'){throw 'Checkpoint replacement failed'}
}
& (Join-Path $repo 'Install.ps1') -Mode Uninstall -GamePath $game -DocumentsPath $docs
foreach($file in $before.Keys){if((Get-FileHash $file).Hash -ne $before[$file]){throw "Restore mismatch: $file"}}
Write-Output "PASS: package reconstruction, installation, settings preservation, both checkpoint configs, and exact uninstall restoration. Fixture retained: $fixture"
