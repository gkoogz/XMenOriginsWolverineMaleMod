param([Parameter(Mandatory=$true)][string]$Workspace,
 [string]$VcVars64='C:/BuildTools/VC/Auxiliary/Build/vcvars64.bat')
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath($Workspace)
New-Item -ItemType Directory -Path $root -Force | Out-Null
$dep=Join-Path $root 'dependencies/UEViewer'
$pin='a0bfb468d42be831b126632fd8a0ae6b3614f981'
if(!(Test-Path -LiteralPath $dep)) {
 & git -c core.longpaths=true clone --no-checkout https://github.com/gildor2/UEViewer.git $dep
 if($LASTEXITCODE){throw 'UEViewer dependency fetch failed.'}
 & git -C $dep checkout --detach $pin
 if($LASTEXITCODE){throw 'Pinned LZO source unavailable.'}
}
if((& git -C $dep rev-parse HEAD) -ne $pin -or (& git -C $dep status --porcelain)) {throw 'LZO dependency must be at the exact clean pin.'}
# GPL LZO remains an external authoring-only dependency. No LZO source/binary
# or retail game assets are distributed with this repository or mod release.
$cmd=Join-Path $root 'build-dependencies.cmd'
$launcher=Join-Path $root 'launch_capability_restore.exe'
$lzo=Join-Path $root 'lzo_read.dll'
@('@echo off',('call "'+$VcVars64+'" >nul'),'if errorlevel 1 exit /b 1',('cd /d "'+$root+'"'),
 ('cl /nologo /LD /O2 /MT /I"'+(Join-Path $dep 'libs/include')+'" "'+(Join-Path $dep 'libs/lzo/lzo1x_d2.c')+'" /link /EXPORT:lzo1x_decompress_safe /OUT:"'+$lzo+'"'),
 'if errorlevel 1 exit /b 1',
 ('cl /nologo /O2 /MT /EHsc /std:c++17 "'+(Join-Path $PSScriptRoot 'launch_capability.cpp')+'" /Fe:"'+$launcher+'" /link dbghelp.lib user32.lib ole32.lib uuid.lib')) | Set-Content -LiteralPath $cmd -Encoding ascii
& cmd /c $cmd
if($LASTEXITCODE){throw 'Sandbox dependency build failed.'}
@{schema=1;lzoSourceRepository='https://github.com/gildor2/UEViewer';lzoSourceCommit=$pin;lzoLicense='GPL-2.0-or-later, preserved in external source';lzoSHA256=(Get-FileHash $lzo).Hash;launcherSHA256=(Get-FileHash $launcher).Hash} | ConvertTo-Json | Set-Content (Join-Path $root 'dependencies.json')
