param(
    [switch]$PromptForMapKey
)

$ErrorActionPreference = "Stop"
$exe = Join-Path $PSScriptRoot "appBlueSkyPRO.exe"

if (-not (Test-Path $exe)) {
    Write-Error "appBlueSkyPRO.exe was not found beside this script. Extract the complete ZIP archive first."
    exit 2
}

if ($PromptForMapKey -or -not $env:BLUESKY_MAPTILER_API_KEY) {
    $secure = Read-Host "Enter your MapTiler API key (input is hidden)" -AsSecureString
    $ptr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secure)
    try {
        $env:BLUESKY_MAPTILER_API_KEY = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($ptr)
    } finally {
        [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($ptr)
    }
}

if (-not $env:BLUESKY_MAPTILER_API_KEY) {
    Write-Error "MapTiler API key is required for map loading."
    exit 3
}

Write-Host "Starting BlueSky PRO. Map provider: MapTiler Hybrid v4."
Write-Host "Telemetry remains unavailable unless its complete explicit configuration is supplied."
Push-Location $PSScriptRoot
try {
    & $exe
    exit $LASTEXITCODE
} finally {
    Pop-Location
}
