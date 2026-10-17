param(
    [switch]$PromptForMapKey
)

$ErrorActionPreference = "Stop"
$exe = Join-Path $PSScriptRoot "appBlueSkyPRO.exe"

if (-not (Test-Path $exe)) {
    Write-Error "appBlueSkyPRO.exe was not found beside this script. Extract the complete ZIP archive first."
    exit 2
}

if ($PromptForMapKey -or -not $env:BLUESKY_YANDEX_MAPS_API_KEY) {
    $secure = Read-Host "Enter a valid Yandex Maps Tiles API key (input is hidden)" -AsSecureString
    $ptr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secure)
    try {
        $env:BLUESKY_YANDEX_MAPS_API_KEY = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($ptr)
    } finally {
        [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($ptr)
    }
}

if (-not $env:BLUESKY_YANDEX_MAPS_API_KEY) {
    Write-Error "Yandex Maps API key is required for map tile loading."
    exit 3
}

Write-Host "Starting BlueSky PRO. Map provider: Yandex Maps."
Write-Host "Telemetry is disabled unless its full explicit configuration is supplied."
Push-Location $PSScriptRoot
try {
    & $exe
    exit $LASTEXITCODE
} finally {
    Pop-Location
}
