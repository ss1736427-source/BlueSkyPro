param(
    [Parameter(Mandatory=$true)] [string]$SourceRoot,
    [Parameter(Mandatory=$true)] [string]$BackupRoot
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $SourceRoot -PathType Container)) {
    throw "SourceRoot not found: $SourceRoot"
}
if (-not (Test-Path -LiteralPath $BackupRoot -PathType Container)) {
    throw "BackupRoot not found: $BackupRoot"
}

# /L = verification only; /MIR = exact mirror semantics; /XJ = do not traverse junctions.
robocopy $SourceRoot $BackupRoot /MIR /L /XJ /R:0 /W:0
$code = $LASTEXITCODE

if ($code -ge 8) {
    throw "Backup mirror verification failed. Robocopy exit code: $code"
}

Write-Host "Backup mirror verification completed. Robocopy exit code: $code"
exit 0
