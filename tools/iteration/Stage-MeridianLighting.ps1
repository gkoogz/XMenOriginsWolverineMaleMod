param(
 [Parameter(Mandatory=$true)][string]$Workspace,
 [Parameter(Mandatory=$true)][string]$Layer,
 [Parameter(Mandatory=$true)][string]$OverlayDirectory
)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
if(!(Test-Path -LiteralPath (Join-Path $root 'meridian-test-room.json')) -and !(Test-Path -LiteralPath (Join-Path $root 'prototype.json'))){throw 'An explicit private Meridian workspace is required.'}
$exe=Join-Path $root 'owned-game/Binaries/Wolverine.exe'
if(Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}){throw 'Close the owned process before changing its stage.'}
$layerPath=(Resolve-Path -LiteralPath $Layer).Path
$receiptPath=Join-Path (Split-Path -Parent $layerPath) 'jungle1_zone01a.packed.json'
$receipt=Get-Content -LiteralPath $receiptPath -Raw | ConvertFrom-Json
if((Get-FileHash -LiteralPath $layerPath).Hash -ne $receipt.nativeSHA256){throw 'Packed layer hash does not match its authoring receipt.'}
$game=Join-Path $root 'owned-game/WGame'
$cooked=Join-Path $game 'CookedPC'
$entry=Get-Item -LiteralPath $cooked
$overlay=[IO.Path]::GetFullPath($OverlayDirectory)
$name='jungle1_zone01a.xxx'
$ownerFile=Join-Path $overlay 'meridian-owner.json'
if(Test-Path -LiteralPath $overlay){
 $owner=Get-Content -LiteralPath $ownerFile -Raw | ConvertFrom-Json
 if($owner.workspace -ne $root -or @($entry.Target)[0] -ne $overlay){throw 'Overlay belongs to a different workspace or is not active.'}
 $destination=Join-Path $overlay $name
 if((Get-Item -LiteralPath $destination).LinkType){throw 'The private layer must not be a link.'}
 Copy-Item -LiteralPath $layerPath -Destination $destination -Force
}else{
 if($entry.LinkType -ne 'Junction'){throw 'The initial private room must have a shared CookedPC junction.'}
 $shared=@($entry.Target)[0]
 if([IO.Path]::GetPathRoot($overlay) -ne [IO.Path]::GetPathRoot($shared)){throw 'Choose an overlay on the same volume as the shared assets, so immutable files can be hard linked without administrator privileges.'}
 $saved=Join-Path $game 'CookedPC-baseline-junction'
 if(Test-Path -LiteralPath $saved){throw 'Baseline junction already exists.'}
 if(![IO.Path]::GetFullPath($saved).StartsWith($root+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Baseline link must remain within the private workspace.'}
 New-Item -ItemType Directory -Path $overlay | Out-Null
 foreach($source in Get-ChildItem -LiteralPath $cooked){
  $target=Join-Path $overlay $source.Name
  $original=Join-Path $shared $source.Name
  if($source.Name -eq $name){Copy-Item -LiteralPath $layerPath -Destination $target}
  elseif($source.PSIsContainer){New-Item -ItemType Junction -Path $target -Target $original | Out-Null}
  else{New-Item -ItemType HardLink -Path $target -Target $original | Out-Null}
 }
 @{schema=1;workspace=$root;shared=$shared;mutableFile=$name;otherFiles='immutable hard links'} | ConvertTo-Json | Set-Content -LiteralPath $ownerFile
 if((Get-FileHash -LiteralPath (Join-Path $overlay $name)).Hash -ne $receipt.nativeSHA256){throw 'Overlay verification failed before activation.'}
 # Only rename the checked junction itself. Never move/delete its shared target.
 Rename-Item -LiteralPath $cooked -NewName 'CookedPC-baseline-junction'
 New-Item -ItemType Junction -Path $cooked -Target $overlay | Out-Null
}
$installed=Join-Path $cooked $name
if((Get-FileHash -LiteralPath $installed).Hash -ne $receipt.nativeSHA256){throw 'Staged layer verification failed.'}
@{schema=1;privateLayer=$installed;overlay=$overlay;sha256=$receipt.nativeSHA256;acceptedRoomModified=$false;sourceReceipt=$receiptPath} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $root 'lighting-stage.json')
Write-Output "Private lighting staged: $installed"
