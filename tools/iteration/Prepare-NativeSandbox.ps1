param([Parameter(Mandatory=$true)][string]$Workspace)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$owned=Join-Path $root 'owned-game'
if (!(Test-Path -LiteralPath (Join-Path $owned 'Binaries/Wolverine.exe'))) { throw 'An initialized owned native clone is required; see docs/ITERATION-SANDBOX.md.' }
if ($owned -eq 'C:\Games\X-Men Origins Wolverine') { throw 'Retail installation is forbidden.' }
$running=Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq (Join-Path $owned 'Binaries/Wolverine.exe')}
if ($running) { throw 'Close the owned sandbox before preparing its packages.' }
$env:MALEMOD_SANDBOX_WORKSPACE=$root
$layers=@('jungle1_zone01','jungle1_vista01','jungle1_vista02','jungle1_vista02_B','jungle1_vista03','jungle1_vista03_B','jungle1_pp_fog')
foreach ($layer in $layers) {
 & python (Join-Path $PSScriptRoot 'author_native_layer.py') --name $layer --empty
 if ($LASTEXITCODE) { throw "Native authoring failed: $layer" }
 & python (Join-Path $PSScriptRoot 'pack_private_layer.py') $layer authored-native-layers
 if ($LASTEXITCODE) { throw "Native packing failed: $layer" }
}
& python (Join-Path $PSScriptRoot 'author_native_layer.py') --name jungle1_zone01a --studio
if ($LASTEXITCODE) { throw 'Floor authoring failed.' }
& python (Join-Path $PSScriptRoot 'pack_private_layer.py') jungle1_zone01a authored-native-layers
if ($LASTEXITCODE) { throw 'Floor packing failed.' }
& python (Join-Path $PSScriptRoot 'isolate_native_story.py')
if ($LASTEXITCODE) { throw 'Story isolation failed.' }
