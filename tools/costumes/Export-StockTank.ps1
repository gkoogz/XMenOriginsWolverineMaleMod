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

$e=$adapter.ObjectExports.Values | Where-Object ClassName -eq SkeletalMesh | Select-Object -First 1
$m=$adapter.ObjectAdapters.Create($e);$null=@($m.GetLODs());$lod=$m.GetType().GetField('_lods',[Reflection.BindingFlags]'NonPublic,Instance').GetValue($m).Items[0]
$s=$lod.Sections.Items | Where-Object MaterialIndex -eq 4
$chunk=$lod.Chunks.Items[$s.ChunkIndex.Value];$bones=@($chunk.Bones.Items | ForEach-Object {[int]$_.Value});$vs=$lod.GPUSkin.VertsHalf.Items
$ids=@($lod.IndexBuffer.Indices16.Items | Select-Object -Skip $s.FirstTriangleIndex.Value -First (3*$s.NumTriangles) | ForEach-Object {[int]$_.Value})
$vertices=foreach($id in ($ids|Sort-Object -Unique)){
 $v=$vs[$id];@{id=$id;p=@([double]$v.Pos.X,[double]$v.Pos.Y,[double]$v.Pos.Z);n=@([int]$v.Normal.X,[int]$v.Normal.Y,[int]$v.Normal.Z);uv=@([int]$v.UVs.Items[0].U,[int]$v.UVs.Items[0].V);bone=@($v.BoneIndices|ForEach-Object{$bones[[int]$_]});weight=@($v.BoneWeights|ForEach-Object{[int]$_})}
}
@{schema='wolverine.stock-tank/1';packageHash=(Get-FileHash -LiteralPath $InputPackage).Hash;mesh="CH_Wolverine_Alkali";first=$s.FirstTriangleIndex.Value;triangles=$s.NumTriangles;vertices=@($vertices);indices=$ids;palette=$bones}|ConvertTo-Json -Depth 8 -Compress | Set-Content -LiteralPath $Output -Encoding utf8
"Exported $($vertices.Count) vertices / $($s.NumTriangles) triangles"
