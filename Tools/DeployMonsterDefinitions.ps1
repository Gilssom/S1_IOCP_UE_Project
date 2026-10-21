param(
 [string]$ServerExecutable='E:\Git Fork\GameServer\GameServer\Binary\Debug\GameServer.exe',
 [string]$ExportFile=(Join-Path (Split-Path -Parent $PSScriptRoot) 'Saved\ServerData\MonsterDefinitions.s1monster')
)
$ErrorActionPreference='Stop'
$exe=(Resolve-Path -LiteralPath $ServerExecutable).Path
$source=(Resolve-Path -LiteralPath $ExportFile).Path
& $exe --validate-monster-definitions $source
if($LASTEXITCODE -ne 0){throw 'Server rejected monster definitions; nothing deployed.'}
$target=Join-Path (Split-Path -Parent $exe) 'Config\MonsterDefinitions.s1monster'
$hash=(Get-FileHash -LiteralPath $source).Hash
if((Test-Path -LiteralPath $target) -and (Get-FileHash -LiteralPath $target).Hash -eq $hash){'Definitions match; restart server to apply.';exit 0}
New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force|Out-Null
if(Test-Path -LiteralPath $target){Copy-Item -LiteralPath $target -Destination ($target+'.before_'+(Get-Date -Format 'yyyyMMdd_HHmmss_fff'))}
Copy-Item -LiteralPath $source -Destination ($target+'.pending')
if((Get-FileHash -LiteralPath ($target+'.pending')).Hash -ne $hash){throw 'Copy hash mismatch'}
Move-Item -LiteralPath ($target+'.pending') -Destination $target -Force
"Deployed $target SHA256=$hash. Restart GameServer to apply."
