param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004')
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$exe=Join-Path $root 'owned-game/Binaries/Wolverine.exe'
if (Get-CimInstance Win32_Process -Filter "Name='Wolverine.exe'" | Where-Object {$_.ExecutablePath -eq $exe}) {throw 'Close owned scene before configuring its native input.'}
foreach ($name in @('DefaultInput.ini','WInput.ini')) {
 $path=Join-Path $root ('owned-game/WGame/Config/'+$name)
 if(!(Test-Path -LiteralPath $path)){continue}
 $text=Get-Content -LiteralPath $path -Raw
 $text=[regex]::Replace($text,'(?ms)^; MaleMod private camera begin.*?^; MaleMod private camera end\r?\n','')
 $prefix=if ($name -eq 'DefaultInput.ini') {'.'} else {''}
 $lines=@('; MaleMod private camera begin',
  ($prefix+'Bindings=(Name="R",Command="Axis aTurn Speed=-1.0 AbsoluteAxis=100")'),
  ($prefix+'Bindings=(Name="T",Command="Axis aTurn Speed=1.0 AbsoluteAxis=100")'),
  ($prefix+'Bindings=(Name="Y",Command="Axis aLookup Speed=0.8 AbsoluteAxis=100")'),
  ($prefix+'Bindings=(Name="U",Command="Axis aLookup Speed=-0.8 AbsoluteAxis=100")'),
  ($prefix+'Bindings=(Name="V",Command="SetCameraZoomAmount 1000 | OnRelease SetCameraZoomAmount 0")'),
  '; MaleMod private camera end') -join "`r`n"
 $text=$text.Replace('[Engine.PlayerInput]',"[Engine.PlayerInput]`r`n$lines")
 Set-Content -LiteralPath $path -Value $text -NoNewline -Encoding ascii
}
