#requires -Version 5.1
[CmdletBinding()]
param([string]$ArchivePath)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$m=Get-Content (Join-Path $root 'manifest.json') -Raw|ConvertFrom-Json
$name='WolverineAnatomyTool-v'+$m.version+'.zip'
$temporary=$null
try {
 if(-not $ArchivePath){
  $temporary=Join-Path ([IO.Path]::GetTempPath()) ('wolverine-payload-'+[guid]::NewGuid().ToString('N'))
  New-Item -ItemType Directory $temporary|Out-Null
  & gh release download ('v'+$m.version) --repo gkoogz/XMenOriginsWolverineMaleMod --pattern $name --dir $temporary
  if($LASTEXITCODE){throw 'Release download failed. Authenticate using gh auth login.'}
  $ArchivePath=Join-Path $temporary $name
 }
 Add-Type -AssemblyName System.IO.Compression.FileSystem
 $archive=[IO.Compression.ZipFile]::OpenRead([IO.Path]::GetFullPath($ArchivePath))
 try {
  $prefix='WolverineAnatomyTool-v'+$m.version+'/payload/'
  $specs=@()
  foreach($p in $m.payload.PSObject.Properties){$specs+=@{Name=$p.Name;Hash=$p.Value}}
  foreach($p in $m.idleClips.PSObject.Properties){$specs+=@{Name=('WolverineIdle/'+$p.Name);Hash=$p.Value}}
  $destination=[IO.Path]::GetFullPath((Join-Path $root 'payload'))
  # Validate every release payload before writing any local payload files.
  foreach($s in $specs){
   $path=[IO.Path]::GetFullPath((Join-Path $destination $s.Name))
   if(-not $path.StartsWith(($destination+'\'),[StringComparison]::OrdinalIgnoreCase)){throw 'Invalid payload destination'}
   $entry=$archive.GetEntry($prefix+$s.Name)
   if(-not $entry){throw ('Missing release payload: '+$s.Name)}
   $inputStream=$entry.Open();$sha=[Security.Cryptography.SHA256]::Create()
   try{$hash=[BitConverter]::ToString($sha.ComputeHash($inputStream)).Replace('-','')}finally{$sha.Dispose();$inputStream.Dispose()}
   if($hash -ne $s.Hash){throw ('Release payload checksum mismatch: '+$s.Name)}
   if((Test-Path -LiteralPath $path) -and (Get-FileHash -LiteralPath $path).Hash -ne $s.Hash){throw ('Different local payload already exists: '+$path)}
  }
  foreach($s in $specs){
   $path=Join-Path $destination $s.Name
   if(Test-Path -LiteralPath $path){continue}
   New-Item -ItemType Directory (Split-Path $path -Parent) -Force|Out-Null
   [IO.Compression.ZipFileExtensions]::ExtractToFile($archive.GetEntry($prefix+$s.Name),$path,$false)
   if((Get-FileHash -LiteralPath $path).Hash -ne $s.Hash){throw ('Extracted payload checksum mismatch: '+$s.Name)}
  }
  Write-Output ('Verified '+$specs.Count+' local release payload files in '+$destination)
 }finally{$archive.Dispose()}
}finally{
 if($temporary){
  $file=Join-Path $temporary $name
  if(Test-Path -LiteralPath $file){Remove-Item -LiteralPath $file}
  Remove-Item -LiteralPath $temporary
 }
}