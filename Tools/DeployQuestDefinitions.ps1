param(
 [string]$ServerExecutable='E:\Git Fork\GameServer\GameServer\Binary\Debug\GameServer.exe',
 [string]$ExportFile=(Join-Path (Split-Path -Parent $PSScriptRoot) 'Saved\QuestExports\QuestDefinitions.s1quest')
)
$ErrorActionPreference='Stop'
$exe=(Resolve-Path -LiteralPath $ServerExecutable).Path
$source=(Resolve-Path -LiteralPath $ExportFile).Path
# Actual server binary validates its registered monster/item IDs without starting DB/network.
& $exe --validate-quest-definitions $source
if($LASTEXITCODE -ne 0){throw 'Server rejected definitions; deployment not changed.'}
$directory=Join-Path (Split-Path -Parent $exe) 'Config'
[void](New-Item -ItemType Directory -Path $directory -Force)
$target=Join-Path $directory 'QuestDefinitions.s1quest'
$hash=(Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
if((Test-Path -LiteralPath $target) -and (Get-FileHash -LiteralPath $target).Hash -eq $hash){Write-Output 'Definitions already match; restart server to load.';exit 0}
if(Test-Path -LiteralPath $target){Copy-Item -LiteralPath $target -Destination ($target+'.before_'+(Get-Date -Format 'yyyyMMdd_HHmmss_fff'))}
$temporary=$target+'.pending'
Copy-Item -LiteralPath $source -Destination $temporary
if((Get-FileHash -LiteralPath $temporary).Hash -ne $hash){throw 'Deployment copy verification failed.'}
Move-Item -LiteralPath $temporary -Destination $target -Force
Write-Output "Deployed definitions: $target SHA256=$hash. Restart server to apply."
