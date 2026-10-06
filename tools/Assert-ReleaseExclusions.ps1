param([Parameter(Mandatory=$true)][string]$StagingDirectory)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $StagingDirectory).Path
$forbidden=@('tools/iteration','dev/sandbox','docs/ITERATION-SANDBOX.md','docs/SANDBOX-REPRODUCTION.md')
foreach($relative in $forbidden){if(Test-Path -LiteralPath (Join-Path $root $relative)){throw "Developer sandbox must not enter uploaded release: $relative"}}
foreach($item in Get-ChildItem -LiteralPath $root -Recurse -File){if($item.Extension -in @('.wgameprofile','.dmp') -or $item.Name -match '^(sandbox-contract|checkpoint-proof|private-clone-manifest)\.json$'){throw "Private development data in release: $($item.FullName)"}}
Write-Output 'PASS release excludes grey-room tools, profiles and captures.'
