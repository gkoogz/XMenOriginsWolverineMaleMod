param(
 [Parameter(Mandatory=$true)][string]$SourceWorkspace,
 [Parameter(Mandatory=$true)][string]$Destination,
 [Parameter(Mandatory=$true)][string]$CandidateRuntime
)
$ErrorActionPreference='Stop'
# Private developer derivative of a room built from this repository's recipe.
# Neither the accepted room nor retail receives the experimental DLL.
$sourceGame=Join-Path (Resolve-Path -LiteralPath $SourceWorkspace).Path 'owned-game'
$targetRoot=[IO.Path]::GetFullPath($Destination)
$candidate=(Resolve-Path -LiteralPath $CandidateRuntime).Path
if(Test-Path -LiteralPath $targetRoot){throw 'Choose a new owned output directory.'}
if(!(Test-Path -LiteralPath (Join-Path $sourceGame 'Binaries/Wolverine.exe'))){throw 'Reproduce the native room recipe before creating its experimental derivative.'}
$game=Join-Path $targetRoot 'owned-game'
New-Item -ItemType Directory -Path (Join-Path $game 'Binaries'),(Join-Path $game 'WGame') -Force | Out-Null
Get-ChildItem -LiteralPath (Join-Path $sourceGame 'Binaries') -File |
 Where-Object {$_.Extension -in @('.exe','.dll','.ini','.dds','.png','.xml')} |
 ForEach-Object {Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $game 'Binaries') -Force}
foreach($name in @('Engine','RavenShared')){
 New-Item -ItemType Junction -Path (Join-Path $game $name) -Target (Join-Path $sourceGame $name) | Out-Null
}
foreach($name in @('CookedPC','Movies','Localization','Splash')){
 New-Item -ItemType Junction -Path (Join-Path $game "WGame/$name") -Target (Join-Path $sourceGame "WGame/$name") | Out-Null
}
foreach($name in @('Config','SaveData-ground-control')){
 Copy-Item -LiteralPath (Join-Path $sourceGame "WGame/$name") -Destination (Join-Path $game 'WGame') -Recurse -Force
}
New-Item -ItemType Directory -Path (Join-Path $game 'WGame/SaveData'),(Join-Path $game 'WGame/Logs') -Force | Out-Null
Copy-Item -LiteralPath $candidate -Destination (Join-Path $game 'Binaries/d3d9.dll') -Force
@{schema=1;sourceRoom=$sourceGame;candidateSHA256=(Get-FileHash -LiteralPath $candidate).Hash;
  accepted=$false;normalRuntimeModified=$false;hostInput=$false;hostFocus=$false} |
 ConvertTo-Json | Set-Content -LiteralPath (Join-Path $targetRoot 'meridian-test-room.json')
Write-Output "Created unaccepted private meridian room: $targetRoot"
