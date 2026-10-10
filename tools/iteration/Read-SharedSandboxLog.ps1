# The native CRT logger opens and closes the log for each record. Its short
# sharing window must not abort an otherwise successful native regression.
function Read-SharedSandboxLog([string]$Path,[switch]$Lines) {
 for($attempt=0;$attempt -lt 40;$attempt++) {
  try {
   $stream=[IO.File]::Open($Path,[IO.FileMode]::Open,[IO.FileAccess]::Read,([IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete))
   try {$reader=[IO.StreamReader]::new($stream);try {$text=$reader.ReadToEnd()} finally {$reader.Dispose()}} finally {$stream.Dispose()}
   if($Lines){return $text -split '\r?\n'}
   return $text
  } catch [IO.IOException] {
   if($attempt -eq 39){throw}
   Start-Sleep -Milliseconds 50
  }
 }
}

# A complete command-named native PNG remains valid evidence if the logger
# loses its separate line to a concurrent CRT append. Never accept a partial
# file merely because it exists: require the PNG signature and final IEND.
function Get-CompletedSandboxCapture([string]$Directory,[uint32]$Revision) {
 $candidate=Get-ChildItem -LiteralPath $Directory -Filter "*-command-$Revision.png" | Select-Object -First 1
 if(!$candidate){return $null}
 try {
  $stream=[IO.File]::Open($candidate.FullName,[IO.FileMode]::Open,[IO.FileAccess]::Read,([IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete))
  try {
   if($stream.Length -lt 20){return $null}
   $head=[byte[]]::new(8);$tail=[byte[]]::new(12)
   if($stream.Read($head,0,8) -ne 8){return $null}
   $stream.Seek(-12,[IO.SeekOrigin]::End)|Out-Null
   if($stream.Read($tail,0,12) -ne 12){return $null}
   if([BitConverter]::ToString($head) -ne '89-50-4E-47-0D-0A-1A-0A' -or [BitConverter]::ToString($tail) -ne '00-00-00-00-49-45-4E-44-AE-42-60-82'){return $null}
   return $candidate
  } finally {$stream.Dispose()}
 } catch [IO.IOException] {return $null}
}
