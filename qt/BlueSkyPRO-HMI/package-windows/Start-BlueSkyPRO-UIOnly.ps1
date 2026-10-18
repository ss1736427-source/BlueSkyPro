$ErrorActionPreference = "Stop"
$exe = Join-Path $PSScriptRoot "appBlueSkyPRO.exe"
if (-not (Test-Path $exe)) {
    Write-Error "appBlueSkyPRO.exe was not found beside this script. Extract the complete ZIP archive first."
    exit 2
}
Remove-Item Env:BLUESKY_MAVLINK_CHANNEL_ID -ErrorAction SilentlyContinue
Write-Host "Starting BlueSky PRO UI. MapTiler requires BLUESKY_MAPTILER_API_KEY."
Push-Location $PSScriptRoot
try {
    & $exe
    exit $LASTEXITCODE
} finally {
    Pop-Location
}
