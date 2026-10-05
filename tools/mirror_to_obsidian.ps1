param(
    [Parameter(Mandatory=$true)]
    [string]$SourceRoot,

    [Parameter(Mandatory=$true)]
    [string]$ObsidianMirror
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $SourceRoot -PathType Container)) {
    throw "SourceRoot does not exist: $SourceRoot"
}

if (-not (Test-Path -LiteralPath $ObsidianMirror -PathType Container)) {
    New-Item -ItemType Directory -Path $ObsidianMirror -Force | Out-Null
}

# Exact mirror of repository content into a dedicated Obsidian mirror folder.
# Git metadata is excluded; all tracked repository content, including .github, is mirrored.
# The destination must be a separate folder from SourceRoot.
$sourceFull = [System.IO.Path]::GetFullPath($SourceRoot).TrimEnd('\\')
$destFull   = [System.IO.Path]::GetFullPath($ObsidianMirror).TrimEnd('\\')
if ($sourceFull -eq $destFull) {
    throw "SourceRoot and ObsidianMirror must be different folders."
}

robocopy $SourceRoot $ObsidianMirror /MIR /FFT /R:2 /W:2 /XJ /XD "$SourceRoot\.git" "$ObsidianMirror\.git" | Out-Host

$code = $LASTEXITCODE
if ($code -ge 8) {
    throw "robocopy failed with exit code $code"
}

Write-Host "BlueSky PRO exact mirror completed: $SourceRoot -> $ObsidianMirror"
