param([string]$Workspace='E:/MaleModBuilds/wolverine-sandbox-capability-20261004')
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $Workspace).Path
$active=Get-Content -LiteralPath (Join-Path $root 'active-run.json') -Raw | ConvertFrom-Json
$guard=Get-Content -LiteralPath (Join-Path $active.run 'guard.txt') -Raw
$running=$false;$childID=$null
if ($guard -match '\{"processID":(\d+),"processCreationTime":(\d+)') {
 $childID=[int]$Matches[1];$created=[long]$Matches[2]
 $process=Get-Process -Id $childID -ErrorAction SilentlyContinue
 $running=$process -and $process.StartTime.ToUniversalTime().ToFileTimeUtc() -eq $created -and $process.Path -eq $active.ownedExecutable
}
$log=Get-Content -LiteralPath (Join-Path $root 'owned-game/Binaries/WolverineRuntime.log') -Raw -ErrorAction SilentlyContinue
$ready=$running -and $log -match 'Private gameplay receipt frame=1 '
$pause=[regex]::Matches($log,'Private command revision=\d+ key=(Pause|Resume)')
$paused=$pause.Count -gt 0 -and $pause[$pause.Count-1].Groups[1].Value -eq 'Pause'
$timings=[regex]::Matches($log,'Private native present timing mean=([\d.]+)ms')
$mean=if ($timings.Count) {[double]$timings[$timings.Count-1].Groups[1].Value} else {$null}
@{schema=1;running=[bool]$running;ready=[bool]$ready;paused=[bool]($running -and $paused);processID=$childID;run=$active.run;lastMeanPresentMs=$mean;fluidFloorContacts='unsupported: StaticMesh absolute origin not verified';persistentDirtyCloth='excluded: installed16aff/Base99ff only';hostInput=$false;hostFocus=$false} | ConvertTo-Json
