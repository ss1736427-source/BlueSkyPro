param(
    [Parameter(Mandatory=$true)]
    [string]$SourceRoot,

    [Parameter(Mandatory=$true)]
    [string]$BackupRoot,

    [int]$IntervalMinutes = 1
)

$ErrorActionPreference = 'Stop'

$scriptPath = Join-Path $SourceRoot 'tools\\mirror_to_backup.ps1'
if (-not (Test-Path -LiteralPath $scriptPath -PathType Leaf)) {
    throw "Backup mirror script not found: $scriptPath"
}

$sourceFull = [System.IO.Path]::GetFullPath($SourceRoot).TrimEnd('\\')
$backupFull = [System.IO.Path]::GetFullPath($BackupRoot).TrimEnd('\\')
if ($sourceFull -eq $backupFull) {
    throw "SourceRoot and BackupRoot must be different folders."
}

$taskName = 'BlueSky PRO - Backup Mirror'
$action = New-ScheduledTaskAction -Execute 'powershell.exe' -Argument "-NoProfile -ExecutionPolicy Bypass -File `"$scriptPath`" -SourceRoot `"$SourceRoot`" -BackupRoot `"$BackupRoot`""
$trigger = New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(1) -RepetitionInterval (New-TimeSpan -Minutes $IntervalMinutes) -RepetitionDuration (New-TimeSpan -Days 3650)

Register-ScheduledTask -TaskName $taskName -Action $action -Trigger $trigger -Description 'Exact full clone of the BlueSky PRO repository working tree, including .git, to the local backup storage every minute.' -Force | Out-Null

Write-Host "Scheduled task installed: $taskName"
Write-Host "Interval: $IntervalMinutes minute(s)"
Write-Host "Source: $SourceRoot"
Write-Host "Backup: $BackupRoot"
