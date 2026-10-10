# BlueSky PRO confirmed-version save
[CmdletBinding()]
param(
    [string]$Message = ""
)

$ErrorActionPreference = "Stop"
Set-Location (Split-Path -Parent $PSScriptRoot)

Write-Host "=== BlueSky PRO: SAVE VERSION ===" -ForegroundColor Cyan

$branch = (git branch --show-current).Trim()
if ($branch -ne "main") {
    throw "Рабочая ветка должна быть main. Текущая: $branch"
}

git fetch origin main --quiet

$local = (git rev-parse main).Trim()
$remote = (git rev-parse origin/main).Trim()
$counts = (git rev-list --left-right --count "main...origin/main").Trim().Split(" ", [System.StringSplitOptions]::RemoveEmptyEntries)
$ahead = [int]$counts[0]
$behind = [int]$counts[1]

if ($behind -gt 0) {
    throw "main отстаёт от origin/main на $behind коммит(ов). Сначала выполните: git pull --ff-only"
}

$status = @(git status --porcelain)
if ($status.Count -eq 0) {
    Write-Host "Нет изменений для сохранения." -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "Изменения:"
$status | ForEach-Object { Write-Host "  $_" }

Write-Host ""
Write-Host "Проверка пробелов/конфликтных маркеров..." -ForegroundColor DarkCyan
git diff --check

$changedQml = @(git diff --name-only -- '*.qml' '*.ui.qml')
if ($changedQml.Count -gt 0) {
    $qmlformat = Get-Command qmlformat -ErrorAction SilentlyContinue
    if ($null -ne $qmlformat) {
        Write-Host "Проверка QML..." -ForegroundColor DarkCyan
        foreach ($file in $changedQml) {
            & $qmlformat.Source --check $file
            if ($LASTEXITCODE -ne 0) {
                throw "QML-проверка не пройдена: $file"
            }
        }
    } else {
        Write-Host "qmlformat не найден; QML-проверка пропущена." -ForegroundColor Yellow
    }
}

# Design Studio workspace files are local state and are never included
# in a confirmed source-code version.
git add -A
git reset -- '*.qtds' 2>$null

$staged = @(git diff --cached --name-only)
if ($staged.Count -eq 0) {
    Write-Host "После исключения локальных *.qtds изменений для сохранения нет." -ForegroundColor Yellow
    exit 0
}

if ([string]::IsNullOrWhiteSpace($Message)) {
    $Message = "save: confirmed BlueSky PRO version $(Get-Date -Format 'yyyy-MM-dd HH:mm')"
}

Write-Host ""
Write-Host "Сохраняю:"
$staged | ForEach-Object { Write-Host "  $_" }

git commit -m $Message
if ($LASTEXITCODE -ne 0) { throw "Commit не создан." }

git push origin main
if ($LASTEXITCODE -ne 0) { throw "Push в origin/main не выполнен." }

$sha = (git rev-parse --short HEAD).Trim()
Write-Host ""
Write-Host "SAVE POINT: $sha" -ForegroundColor Green
Write-Host "Версия сохранена в origin/main." -ForegroundColor Green
