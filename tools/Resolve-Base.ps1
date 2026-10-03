param([switch]$Diagnostic)
$ErrorActionPreference='Stop'
$repo=Split-Path $PSScriptRoot -Parent
$lockPath=Join-Path $repo 'dependencies/base.lock.json'
$lock=Get-Content -Raw -LiteralPath $lockPath | ConvertFrom-Json
if($lock.schemaVersion -ne 1 -or $lock.commit -notmatch '^[0-9a-f]{40}$' -or $lock.repository -ne 'https://github.com/gkoogz/MaleModBase.git'){throw 'Invalid pinned Base dependency contract'}
$baseCheckout=$env:MALEMOD_BASE_PATH
if(-not $baseCheckout){
 $managed=Join-Path $repo 'dependencies/MaleModBase'
 $existing=Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'ChatGPT/MaleMod'
 if(Test-Path -LiteralPath (Join-Path $managed '.git')){$baseCheckout=$managed}
 elseif(Test-Path -LiteralPath (Join-Path $existing '.git')){$baseCheckout=$existing}
 else {
  if(Test-Path -LiteralPath $managed){throw 'Managed Base path exists without a Git checkout; refusing to overwrite it'}
  & git clone --no-checkout $lock.repository $managed | Out-Host
  if($LASTEXITCODE -ne 0){throw 'Cannot clone pinned Base dependency'}
  & git -C $managed checkout --detach $lock.commit | Out-Host
  if($LASTEXITCODE -ne 0){throw 'Pinned Base commit unavailable'}
  $baseCheckout=$managed
 }
}
$baseCheckout=(Resolve-Path -LiteralPath $baseCheckout).Path
foreach($header in @('surface/pelvic_frame.hpp','surface/garment_support.hpp','garments/jockstrap.hpp','garments/numerical_support.hpp','controls/presentation.hpp')){
 if(-not (Test-Path -LiteralPath (Join-Path $baseCheckout ('include/malemod/'+$header)))){throw "Base shared API missing: $header"}
}
if(-not $Diagnostic){
 $head=& git -C $baseCheckout rev-parse HEAD
 if($LASTEXITCODE -ne 0 -or $head -ne $lock.commit){throw 'Base checkout differs from pinned revision; preserve local work and select the exact dependency with MALEMOD_BASE_PATH'}
 $changes=& git -C $baseCheckout status --porcelain --untracked-files=all -- include src CMakeLists.txt assets/garments provenance/garments.json
 if($LASTEXITCODE -ne 0 -or $changes){throw 'Pinned Base runtime inputs have uncommitted changes'}
}
Write-Output $baseCheckout
