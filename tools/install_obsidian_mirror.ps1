param(
    [Parameter(Mandatory=$true)]
    [string]$SourceRoot,

    [Parameter(Mandatory=$true)]
    [string]$ObsidianMirror,

    [int]$IntervalMinutes = 1
)

$ErrorActionPreference = 'Stop'

$scriptPath = Join-Path $SourceRoot 'tools\mirror_to_obsidian.ps1'
if (-not (Test-Path -LiteralPath $scriptPath -PathType Leaf)) {
    throw "Mirror script not found: $scriptPath"
}

$taskName = 'BlueSky PRO - Obsidian Mirror'
$action = New-ScheduledTaskAction -Execute 'powershell.exe' -Argument "-NoProfile -ExecutionPolicy Bypass -File `"$scriptPath`" -SourceRoot `"$SourceRoot`" -ObsidianMirror `"$ObsidianMirror`""
$trigger = New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(1) -RepetitionInterval (New-TimeSpan -Minutes $IntervalMinutes) -RepetitionDuration (New-TimeSpan -Days 3650)

Register-ScheduledTask -TaskName $taskName -Action $action -Trigger $trigger -Description 'Mirror the BlueSky PRO working tree into the Obsidian safety copy every minute.' -Force | Out-Null

Write-Host "Scheduled task installed: $taskName"
Write-Host "Interval: $IntervalMinutes minute(s)"
Write-Host "Source: $SourceRoot"
Write-Host "Mirror: $ObsidianMirror"
