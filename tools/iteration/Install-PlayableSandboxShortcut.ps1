param(
 [string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004',
 [string]$StateDirectory=(Join-Path $env:LOCALAPPDATA 'MaleMod/WolverineSandbox')
)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$state=[IO.Path]::GetFullPath($StateDirectory)
$localRoot=[IO.Path]::GetFullPath((Join-Path $env:LOCALAPPDATA 'MaleMod'))
if(!$state.StartsWith($localRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) {throw 'Writable state must stay under LOCALAPPDATA/MaleMod.'}
& (Join-Path $PSScriptRoot 'Prepare-PlayableSandboxStorage.ps1') -Workspace $root -StateDirectory $state
$game=Join-Path $state 'human-game'
& (Join-Path $PSScriptRoot 'Play-NativeSandbox.ps1') -Workspace $root -StateDirectory $state -ValidateOnly | Out-Null
$desktop=[Environment]::GetFolderPath('DesktopDirectory')
$linkPath=Join-Path $desktop 'Wolverine Grey Sandbox.lnk'
$shell=New-Object -ComObject WScript.Shell
$link=$shell.CreateShortcut($linkPath)
$script=Join-Path $PSScriptRoot 'Play-NativeSandbox.ps1'
$powershell=Join-Path $env:SystemRoot 'System32/WindowsPowerShell/v1.0/powershell.exe'
if((Test-Path -LiteralPath $linkPath) -and $link.Arguments -notlike '*Play-NativeSandbox.ps1*') {throw 'An unrelated shortcut already uses this name.'}
$link.TargetPath=$powershell
$link.Arguments='-NoProfile -ExecutionPolicy Bypass -File "'+$script+'" -Workspace "'+$root+'" -StateDirectory "'+$state+'"'
$link.WorkingDirectory=Join-Path $game 'Binaries'
$link.IconLocation=(Join-Path $game 'Binaries/Wolverine.exe')+',0'
$link.WindowStyle=1
$link.Description='Native grey gameplay stage. Enter, Continue. F6 menu, arrows adjust. Normal input/audio; no automatic pause or timeout.'
$link.Save()
$readback=$shell.CreateShortcut($linkPath)
if($readback.TargetPath -ne $powershell -or $readback.Arguments -ne $link.Arguments -or $readback.WorkingDirectory -ne $link.WorkingDirectory -or $readback.WindowStyle -ne 1) {throw 'Shortcut readback mismatch.'}
@{schema=1;shortcut=$linkPath;target=$readback.TargetPath;arguments=$readback.Arguments;cwd=$readback.WorkingDirectory;windowStyle=$readback.WindowStyle;gameLaunched=$false;createdUTC=[DateTime]::UtcNow.ToString('o')} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $state 'shortcut-verification.json')
Write-Output "Ready: $linkPath"
