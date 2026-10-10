$ErrorActionPreference = "Stop"

$AgentExecutable = $env:BS_AGENT_EXECUTABLE
$AgentArguments = $env:BS_AGENT_ARGUMENTS

if (-not $AgentExecutable) {
    Write-Error "BS_AGENT_EXECUTABLE is not set. Set it to the approved external agent executable."
    exit 1
}

if (-not (Test-Path -LiteralPath $AgentExecutable -PathType Leaf)) {
    $resolved = Get-Command $AgentExecutable -ErrorAction SilentlyContinue
    if ($resolved) {
        $AgentExecutable = $resolved.Source
    } else {
        Write-Error "Agent executable not found: $AgentExecutable"
        exit 1
    }
}

$agentArgs = if ($AgentArguments) { $AgentArguments -split '\s+' | Where-Object { $_ } } else { @() }

$prompt = @"
BlueSky PRO development continuation.

Verified repository: $($env:BS_REPO)
Branch: $($env:BS_BRANCH)
Verified HEAD SHA: $($env:BS_CURRENT_SHA)

Read these repository files before acting:
- $($env:BS_PROTOCOL)
- $($env:BS_WORKING_RULES)

Verify local HEAD matches the supplied SHA. Continue the current project work package from the recorded repository state.

Select and execute the next smallest concrete technical step that is supported by the existing project documents and does not require a user decision. This may be documentation decomposition, traceability, controlled-record preparation, implementation, or verification, as appropriate to the current work package.

Do not make product or architectural decisions. If a decision listed by the protocol is genuinely required and cannot be resolved from existing approved material, stop and return exit code 42.

Do not stop with exit code 0 merely because the user did not name a clause, file, or exact action: first inspect the recorded current work package, open checkpoints, existing records, and repository history and continue the next deterministic technical step. Only return 0 without a new commit when the current work package has no remaining deterministic technical work or when the protocol explicitly requires stopping.

Run appropriate tests after changes and do not claim CI success unless it belongs to the exact resulting SHA.
When no user decision is required, continue automatically. When a user decision is required, return 42.

Reason for this continuation: $($env:BS_CI_REASON)
"@

function Quote-CmdArgument([string]$Value) {
    if ($Value -notmatch '[\s"]') { return $Value }
    return '"' + ($Value -replace '(\\*)"', '$1$1\"' -replace '(\\+)$', '$1$1') + '"'
}

$promptFile = Join-Path $env:TEMP ("bluesky-agent-prompt-{0}.txt" -f [guid]::NewGuid().ToString("N"))

try {
    [System.IO.File]::WriteAllText($promptFile, $prompt, [System.Text.UTF8Encoding]::new($false))

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true

    $isCmdScript = [System.IO.Path]::GetExtension($AgentExecutable).ToLowerInvariant() -eq ".cmd"
    if ($isCmdScript) {
        $quotedAgent = Quote-CmdArgument $AgentExecutable
        $quotedArgs = ($agentArgs | ForEach-Object { Quote-CmdArgument $_ }) -join ' '
        $psi.FileName = $env:ComSpec
        $psi.Arguments = "/d /c call $quotedAgent $quotedArgs"
    } else {
        $psi.FileName = $AgentExecutable
        $psi.Arguments = ($agentArgs | ForEach-Object { Quote-CmdArgument $_ }) -join ' '
    }

    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $psi
    try {
        if (-not $process.Start()) {
            Write-Error "Failed to start agent: $AgentExecutable"
            exit 1
        }

        $stdoutTask = $process.StandardOutput.ReadToEndAsync()
        $stderrTask = $process.StandardError.ReadToEndAsync()
        $process.StandardInput.Write($prompt)
        $process.StandardInput.Close()
        $process.WaitForExit()

        $stdout = $stdoutTask.GetAwaiter().GetResult()
        $stderr = $stderrTask.GetAwaiter().GetResult()
        if ($stdout) { [Console]::Out.Write($stdout) }
        if ($stderr) { [Console]::Error.Write($stderr) }

        exit $process.ExitCode
    } finally {
        $process.Dispose()
    }
} finally {
    Remove-Item -LiteralPath $promptFile -Force -ErrorAction SilentlyContinue
}
