param([Parameter(Mandatory=$true)][string]$InputPackage,[Parameter(Mandatory=$true)][string]$Output,[Parameter(Mandatory=$true)][string]$UpkDirectory)
$ErrorActionPreference='Stop'
$toolDir=(Resolve-Path -LiteralPath $UpkDirectory).Path
[Environment]::CurrentDirectory=$toolDir
$env:PATH="$toolDir\runtimes\win-x64\native;"+$env:PATH
[Runtime.InteropServices.NativeLibrary]::Load("$toolDir\runtimes\win-x64\native\nironcompress.dll")|Out-Null
Get-ChildItem -LiteralPath $toolDir -Filter '*.dll'|ForEach-Object{try{[Reflection.Assembly]::LoadFrom($_.FullName)|Out-Null}catch{}}
[UPK.Explorer.Utils.GameProfiles]::Initialize()
$game=[UPK.Explorer.Utils.GameProfiles]::GetGames([uint16]568,[uint16]101,[UPK.Utils.GameProfiles.Platforms]::PC)|Select-Object -First 1
$folder=$game.AllFolders|Select-Object -First 1
$package=[UPK.Utils.Packages.UPKPackage]::GetPackage([IO.FileInfo]$InputPackage,$folder)
$package.ReadHeader($game.Platform,$game.LZXrecompression,$game.GameId)
$adapter=[UPK.Utils.Adapters.PackageAdapter]::new($package,[UPK.Explorer.Utils.GameProfiles]::GetObjectDescriptors($game),$game)

$records=foreach($e in $adapter.ObjectExports.Values | Where-Object ClassName -eq SkeletalMesh){
 $m=$adapter.ObjectAdapters.Create($e);$null=@($m.GetLODs());$lods=$m.GetType().GetField('_lods',[Reflection.BindingFlags]'NonPublic,Instance').GetValue($m).Items
 $levels=foreach($lod in $lods){@{vertices=$lod.GPUSkin.VertsHalf.Items.Count;sections=@($lod.Sections.Items | ForEach-Object {@{material=[string]$_.MaterialIndex;first=[string]$_.FirstTriangleIndex.Value;triangles=[string]$_.NumTriangles}})}}
 @{export=[string]$e;lodCount=@($lods).Count;levels=@($levels)}
}
@{package=$InputPackage;sha256=(Get-FileHash -LiteralPath $InputPackage).Hash;meshes=@($records)} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $Output -Encoding utf8
