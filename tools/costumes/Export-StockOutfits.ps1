param(
 [Parameter(Mandatory=$true)][string]$InputDirectory,
 [Parameter(Mandatory=$true)][string]$OutputDirectory,
 [Parameter(Mandatory=$true)][string]$UModel
)
$ErrorActionPreference='Stop'
$inputRoot=(Resolve-Path -LiteralPath $InputDirectory).Path
$exporter=(Resolve-Path -LiteralPath $UModel).Path
$out=[IO.Path]::GetFullPath($OutputDirectory)
if($out.StartsWith($inputRoot.TrimEnd('\')+'\',[StringComparison]::OrdinalIgnoreCase) -or $out -eq $inputRoot){throw 'Offline output must be outside the installed package directory.'}
New-Item -ItemType Directory -Path $out -Force | Out-Null
$cache=Join-Path $out 'source-links'
New-Item -ItemType Directory -Path $cache -Force | Out-Null
# Some stock textures retain the pre-coalescing cache name. Reference the
# unchanged licensed cache in our private directory; never rename the game file.
foreach($file in (Get-ChildItem -LiteralPath $inputRoot -File | Where-Object {$_.Name -like 'CH_Wolverine_*_SF.xxx' -or $_.Name -in @('Startup_int.xxx','coalesced.tfc')})){
 $link=Join-Path $cache $file.Name
 if(!(Test-Path -LiteralPath $link)){
  try{New-Item -ItemType HardLink -Path $link -Target $file.FullName -ErrorAction Stop | Out-Null}
  catch{Copy-Item -LiteralPath $file.FullName -Destination $link}
 }
 if((Get-FileHash -LiteralPath $link).Hash -ne (Get-FileHash -LiteralPath $file.FullName).Hash){throw "Private source reference differs: $($file.Name)"}
}
$cacheFile=Join-Path $cache 'coalesced.tfc'
if(Test-Path -LiteralPath $cacheFile){
 $alias=Join-Path $cache 'Textures_P0.tfc'
 if(!(Test-Path -LiteralPath $alias)){New-Item -ItemType HardLink -Path $alias -Target $cacheFile | Out-Null}
 if((Get-FileHash -LiteralPath $alias).Hash -ne (Get-FileHash -LiteralPath $cacheFile).Hash){throw 'Private texture cache alias differs.'}
}
$records=@()
foreach($package in (Get-ChildItem -LiteralPath $inputRoot -Filter 'CH_Wolverine_*_SF.xxx' -File | Sort-Object Name)){
 # Natural is the anatomy tool's source body, not another garment candidate.
 if($package.Name -eq 'CH_Wolverine_Natural_SF.xxx'){continue}
 $source=$package.FullName
 $log=Join-Path $out ($package.BaseName+'.log')
 & $exporter -export -all -noanim -nostat "-path=$cache" "-out=$(Join-Path $out 'export')" $source *> $log
 if($LASTEXITCODE -ne 0){throw "Export failed: $($package.Name); inspect $log"}
 $records+=@{name=$package.Name;sha256=(Get-FileHash -LiteralPath $source).Hash.ToLowerInvariant();bytes=(Get-Item -LiteralPath $source).Length}
 Write-Output "Exported $($package.Name)"
}
$shared=foreach($name in @('Startup_int.xxx','coalesced.tfc')){
 $path=Join-Path $inputRoot $name
 if(Test-Path -LiteralPath $path){@{name=$name;sha256=(Get-FileHash -LiteralPath $path).Hash.ToLowerInvariant()}}
}
@{schema='wolverine.stock-wardrobe-inputs/1';packages=$records;sharedInputs=@($shared);exporterSHA256=(Get-FileHash -LiteralPath $exporter).Hash.ToLowerInvariant();referenceOnly=$true;installedGameModified=$false;textureCacheAlias='Private Textures_P0.tfc references unchanged coalesced.tfc';omissions=@('Natural source body is not a wardrobe candidate','UE3 damage/light material parity and native gameplay are not established by offline export')} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $out 'source-provenance.json') -Encoding utf8
