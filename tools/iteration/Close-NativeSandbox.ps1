param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004')
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$active=Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw | ConvertFrom-Json
$run=[IO.Path]::GetFullPath($active.run)
if (!$run.StartsWith((Join-Path $root 'runs')+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) {throw 'Invalid owned run path.'}
# The exact native launcher holds its original child HANDLE and consumes this
# request. No process search, PID reuse or retail process termination occurs.
Set-Content -LiteralPath (Join-Path $run 'stop.request') -Value 'close exact owned child'
Write-Output "Close requested for isolated run: $run"
