param([switch]$Upgrade,[Parameter(Mandatory=$true)][string]$OriginalNatural,[Parameter(Mandatory=$true)][string]$OriginalWGame,[string]$ModifiedNatural)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$base=Join-Path ([IO.Path]::GetTempPath()) ('release-14-test-'+[guid]::NewGuid().ToString('N'))
$game=Join-Path $base 'Game with spaces';$docs=Join-Path $base 'Documents'
foreach($d in @('Binaries','WGame/CookedPC','WGame/Config')){New-Item -ItemType Directory (Join-Path $game $d) -Force|Out-Null}
New-Item -ItemType Directory (Join-Path $docs 'Wolverine/WGame/Config') -Force|Out-Null
$source=if($Upgrade){$ModifiedNatural}else{$OriginalNatural}
Copy-Item $source (Join-Path $game 'WGame/CookedPC/CH_Wolverine_Natural_SF.xxx')
Copy-Item $OriginalWGame (Join-Path $game 'WGame/CookedPC/WGame.xxx')
Set-Content (Join-Path $game 'Binaries/Wolverine.exe') 'fixture only'
Set-Content (Join-Path $game 'Binaries/d3d9.dll') 'prior proxy'
Set-Content (Join-Path $game 'Binaries/R14-skin-natural.png') 'prior texture'
New-Item -ItemType Directory (Join-Path $game 'Binaries/WolverineIdle') -Force | Out-Null
Set-Content (Join-Path $game 'Binaries/WolverineIdle/standard-01.wav') 'prior idle audio'
Set-Content (Join-Path $game 'Binaries/WolverineLive.ini') '[Shape] Overall=57'
Set-Content (Join-Path $game 'WGame/Config/DefaultCheckpoints.ini') 'mOutfit=CH_Wolverine_Tank'
Set-Content (Join-Path $docs 'Wolverine/WGame/Config/WCheckpoints.ini') 'mOutfit=CH_Wolverine_Tank'
$before=@{};Get-ChildItem $base -Recurse -File|ForEach-Object {$before[$_.FullName]=(Get-FileHash $_.FullName).Hash}
# This fixture uses a text placeholder executable and never the live game.
# Mock only the process guard so fixture checks can run during gameplay.
if((Get-Content (Join-Path $game 'Binaries/Wolverine.exe') -Raw).Trim() -ne 'fixture only'){throw 'Not a fixture executable'}
function Get-Process {
 [CmdletBinding()]param([string]$Name)
 if($Name -ne 'Wolverine'){throw 'Unexpected fixture process query'}
}
& (Join-Path $repo 'Install.ps1') -GamePath $game -DocumentsPath $docs
$m=Get-Content (Join-Path $repo 'manifest.json') -Raw|ConvertFrom-Json
foreach($name in @('d3d9.dll','R14-skin-natural.png','R14-skin-erect.png')){if((Get-FileHash (Join-Path $game ('Binaries/'+$name))).Hash -ne $m.payload.$name){throw 'Payload mismatch'}}
foreach($clip in $m.idleClips.PSObject.Properties){if((Get-FileHash (Join-Path $game ('Binaries/WolverineIdle/'+$clip.Name))).Hash -ne $clip.Value){throw "Idle clip mismatch: $($clip.Name)"}}
if((Get-FileHash (Join-Path $game 'WGame/CookedPC/WGame.xxx')).Hash -ne $m.installedWGameSHA256){throw 'WGame mismatch'}
if((Get-FileHash (Join-Path $game 'WGame/CookedPC/CH_Wolverine_Natural_SF.xxx')).Hash -ne $m.installedPackageSHA256){throw 'Natural mismatch'}
& (Join-Path $repo 'Install.ps1') -Mode Uninstall -GamePath $game -DocumentsPath $docs
foreach($p in $before.Keys){if((Get-FileHash $p).Hash -ne $before[$p]){throw "Restore mismatch $p"}}
if(Test-Path (Join-Path $game 'Binaries/R14-skin-erect.png')){throw 'New texture not removed'}
if(Test-Path (Join-Path $game 'Binaries/WolverineIdle/custom-01.wav')){throw 'New idle clip not removed'}
"PASS: upgrade=$Upgrade reconstruction, install, existing texture backup, settings and exact restoration. $base"

