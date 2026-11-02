# BlueSky PRO native Windows performance collector
# Run in a separate PowerShell window while reproducing map stutter.
# Requires Windows 10/11 performance counters. Does not change application settings.

param(
    [int]$IntervalSeconds = 1,
    [string]$OutputPath = ".\BlueSkyPRO-windows-performance.csv"
)

$ErrorActionPreference = "Stop"
$rows = [System.Collections.Generic.List[object]]::new()

function Get-CounterPaths {
    param([string]$SetName, [string]$CounterName)
    try {
        $set = Get-Counter -ListSet $SetName -ErrorAction Stop
        $counter = $set.Counter | Where-Object { $_ -like "*\$CounterName" } | Select-Object -First 1
        if (-not $counter) { return @() }
        return @($counter)
    } catch {
        Write-Warning "Counter set unavailable: $SetName ($($_.Exception.Message))"
        return @()
    }
}

$counterPaths = @()
$counterPaths += Get-CounterPaths -SetName "GPU Engine" -CounterName "Utilization Percentage"
$counterPaths += Get-CounterPaths -SetName "GPU Process Memory" -CounterName "Dedicated Usage"
$counterPaths += Get-CounterPaths -SetName "GPU Process Memory" -CounterName "Shared Usage"

$processNames = @("appBlueSkyPRO", "QtWebEngineProcess", "msedgewebview2")
foreach ($name in $processNames) {
    try {
        $instances = (Get-Counter -ListSet "Process").Instances | Where-Object { $_ -like "$name*" }
        foreach ($instance in $instances) {
            foreach ($counter in @("% Processor Time", "Working Set - Private", "Private Bytes")) {
                $counterPaths += "\Process($instance)\$counter"
            }
        }
    } catch {
        Write-Warning "Could not enumerate process counters for $name"
    }
}

$counterPaths = @($counterPaths | Sort-Object -Unique)
if ($counterPaths.Count -eq 0) {
    throw "No supported performance counters were found. Check Windows Performance Counter availability."
}

Write-Host "BlueSky PRO Windows collector started"
Write-Host "Sampling interval: $IntervalSeconds second(s)"
Write-Host "Counters: $($counterPaths.Count)"
Write-Host "Output: $([IO.Path]::GetFullPath($OutputPath))"
Write-Host "Start the map test now. Press Ctrl+C to stop and save the CSV."

try {
    while ($true) {
        $sample = Get-Counter -Counter $counterPaths -SampleInterval $IntervalSeconds -MaxSamples 1
        foreach ($item in $sample.CounterSamples) {
            $rows.Add([pscustomobject]@{
                Timestamp = $sample.Timestamp.ToString("o")
                Counter = $item.Path
                Value = [math]::Round($item.CookedValue, 3)
                Status = $item.Status
            })
        }
        if ($rows.Count -ge 1000) {
            $rows | Export-Csv -Path $OutputPath -NoTypeInformation -Encoding UTF8
            $rows.Clear()
        }
    }
}
finally {
    if ($rows.Count -gt 0) {
        if (Test-Path $OutputPath) {
            $rows | Export-Csv -Path $OutputPath -NoTypeInformation -Append -Encoding UTF8
        } else {
            $rows | Export-Csv -Path $OutputPath -NoTypeInformation -Encoding UTF8
        }
    }
    Write-Host "Collector stopped. Data saved to $([IO.Path]::GetFullPath($OutputPath))"
}
