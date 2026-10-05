param(
    [Parameter(Mandatory=$true)]
    [string]$SourceRoot,

    [Parameter(Mandatory=$true)]
    [string]$BackupRoot
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $SourceRoot -PathType Container)) {
    throw "SourceRoot does not exist: $SourceRoot"
}

if (-not (Test-Path -LiteralPath $BackupRoot -PathType Container)) {
    New-Item -ItemType Directory -Path $BackupRoot -Force | Out-Null
}

$sourceFull = [System.IO.Path]::GetFullPath($SourceRoot).TrimEnd('\\')
$backupFull = [System.IO.Path]::GetFullPath($BackupRoot).TrimEnd('\\')
if ($sourceFull -eq $backupFull) {
    throw "SourceRoot and BackupRoot must be different folders."
}

# Exact filesystem clone of the local Git working tree.
# Unlike the Obsidian mirror, .git is intentionally INCLUDED so the backup can restore the Git repository.
# /MIR synchronizes additions, modifications and deletions.
robocopy $SourceRoot $BackupRoot /MIR /FFT /R:2 /W:2 /XJ | Out-Host

$code = $LASTEXITCODE
if ($code -ge 8) {
    throw "robocopy failed with exit code $code"
}

Write-Host "BlueSky PRO exact backup clone completed: $SourceRoot -> $BackupRoot"
