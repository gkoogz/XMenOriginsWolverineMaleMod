param([Parameter(Mandatory=$true)][string]$GamePath,
 [Parameter(Mandatory=$true)][string]$Workspace,
 [Parameter(Mandatory=$true)][string]$ReleaseDirectory,
 [Parameter(Mandatory=$true)][string]$BaseRepository,
 [string]$VcVars32='C:/BuildTools/VC/Auxiliary/Build/vcvars32.bat',
 [string]$VcVars64='C:/BuildTools/VC/Auxiliary/Build/vcvars64.bat',
 [string]$DirectXSdk='C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)')
$ErrorActionPreference='Stop'
$stock=(Resolve-Path -LiteralPath $GamePath).Path.TrimEnd('\')
$root=[IO.Path]::GetFullPath($Workspace).TrimEnd('\')
if((Test-Path -LiteralPath $root) -or $root.StartsWith($stock+'\',[StringComparison]::OrdinalIgnoreCase)) {throw 'Choose a NEW workspace outside the retail game.'}
$required=@{
 'Binaries/Wolverine.exe'='f2ca60b045279f781e65749ab745924025b9768ef5712d0ac5e4606b2c848872'
 'WGame/CookedPC/WGame.xxx'='960d188d9f8da9c6e0ef2856c2412f1aa5ff82f81c3752e891a9c09ac3d62d2f'
 'WGame/CookedPC/rgame_p.xxx'='740cdba270ca73616b2d409dcdea2567280c9d56503b4ed348c3b82823adec23'
 'WGame/CookedPC/jungle1_zone01.xxx'='a14b172a60ff96cfc86d77fbddb1c58c3f84973fc90aa12b2125452b0ba7e6f4'
 'WGame/CookedPC/jungle1_kismet01.xxx'='8e46ec738c37ca6c9ff0ad25bb3037c0d0684e2607260e69b9467c9230e099ea'
}
foreach($rel in $required.Keys){if((Get-FileHash -LiteralPath (Join-Path $stock $rel)).Hash -ne $required[$rel]){throw "Unsupported stock binding: $rel"}}
$owned=Join-Path $root 'owned-game'
New-Item -ItemType Directory -Path $owned -Force | Out-Null
# Copy native dependencies, NEVER retail saves, ModBackups, logs or user files.
# Copies, not hardlinks/junctions: package authoring cannot write to retail.
foreach($rel in @('Engine','RavenShared','WGame/CookedPC','WGame/Localization','WGame/Movies','WGame/Splash')){
 $dest=Join-Path $owned $rel
 New-Item -ItemType Directory -Path (Split-Path $dest) -Force | Out-Null
 Copy-Item -LiteralPath (Join-Path $stock $rel) -Destination $dest -Recurse
}
foreach($rel in @('Binaries','WGame/Config','WGame/SaveData','WGame/SaveData-ground-control','WGame/Logs')){New-Item -ItemType Directory -Path (Join-Path $owned $rel) -Force | Out-Null}
foreach($item in Get-ChildItem -LiteralPath (Join-Path $stock 'Binaries') -File){if($item.Name -eq 'Wolverine.exe' -or ($item.Extension -eq '.dll' -and $item.Name -ne 'd3d9.dll') -or $item.Name -eq 'KynapseRuntimeConfig.xml'){Copy-Item -LiteralPath $item.FullName -Destination (Join-Path $owned 'Binaries')}}
foreach($item in Get-ChildItem -LiteralPath (Join-Path $stock 'WGame/Config') -Filter 'Default*.ini'){Copy-Item -LiteralPath $item.FullName -Destination (Join-Path $owned 'WGame/Config')}
Copy-Item -LiteralPath (Join-Path $stock 'WGame/PCTOC.txt') -Destination (Join-Path $owned 'WGame/PCTOC.txt')
& (Join-Path $ReleaseDirectory 'Install.ps1') -GamePath $owned -DocumentsPath (Join-Path $root 'documents')
# The verified studio bootstrap uses retail WGame script classes, not the
# release's campaign script patch. Character/WStart patches remain installed.
$wgame=Join-Path $owned 'WGame/CookedPC/WGame.xxx'
(Get-Item -LiteralPath $wgame).IsReadOnly=$false
Copy-Item -LiteralPath (Join-Path $stock 'WGame/CookedPC/WGame.xxx') -Destination $wgame -Force
foreach($name in @('DefaultEngine.ini','DefaultUI.ini','DefaultCheckpoints.ini')){
 $path=Join-Path $owned ('WGame/Config/'+$name);(Get-Item $path).IsReadOnly=$false
 $s=Get-Content -LiteralPath $path -Raw
 if($name -eq 'DefaultEngine.ini'){$s=[regex]::Replace($s,'(?m)^([+.]?StartupMovies=)', ';$1')}
 if($name -eq 'DefaultUI.ini'){$s=$s.Replace('UI_SkinDefault.DefaultSkin','DefaultUISkin.DefaultSkin')}
 if($name -eq 'DefaultCheckpoints.ini'){
  $s=[regex]::Replace($s,'(?ms)(\[Skydive_01 WUIDataProvider_Checkpoint\].*?)(?=\r?\n\[|\z)',{param($m) [regex]::Replace([regex]::Replace($m.Value,'(?m)^mLevel\s*=.*$', 'mLevel=jungle1_kismet01'),'(?m)^mBinkIntro\s*=.*$','mBinkIntro=')})
 }
 Set-Content -LiteralPath $path -Value $s -NoNewline -Encoding ascii
}
$room=Join-Path $root 'native-stream-room';New-Item -ItemType Directory -Path $room | Out-Null
Copy-Item -LiteralPath (Join-Path $stock 'WGame/CookedPC/jungle1_zone01.xxx') -Destination (Join-Path $room 'jungle1_zone01.stock.xxx')
& (Join-Path $PSScriptRoot 'Build-SandboxDependencies.ps1') -Workspace $root -VcVars64 $VcVars64
& python (Join-Path $PSScriptRoot 'build_private_runtime.py') --output (Join-Path $root 'private-runtime') --base $BaseRepository --source-ref 16affd49da5a122df776969574ca62a984b59425 --vcvars $VcVars32 --directx-sdk $DirectXSdk
if($LASTEXITCODE){throw 'Private runtime build failed.'}
$dll=Join-Path $owned 'Binaries/d3d9.dll';(Get-Item $dll).IsReadOnly=$false
Copy-Item (Join-Path $root 'private-runtime/d3d9.dll') $dll -Force
& (Join-Path $PSScriptRoot 'Prepare-NativeSandbox.ps1') -Workspace $root
@{schema=1;stockFiles=$required;retailModified=$false;profilesCopied=$false;runtimeProvenance='private-runtime/provenance.json';next='Create-SandboxCheckpoint.ps1';nativeGameplayVerified=$false} | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $root 'vanilla-bootstrap.json')
Write-Output 'Owned assets regenerated. Next create a fresh local checkpoint; no save is shipped.'
