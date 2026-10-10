param(
    [string]$Repo = "ss1736427-source/BlueSky-PRO-Knowledge",
    [string]$AgentExecutable = $env:BS_AGENT_EXECUTABLE,
    [string]$AgentArguments = $env:BS_AGENT_ARGUMENTS
)

$ErrorActionPreference = "Stop"

$env:BS_REPO = $Repo
$env:BS_BRANCH = "main"
$env:BS_POLL_SECONDS = "5"
$env:BS_CI_GRACE_SECONDS = "5"
$env:BS_AGENT_CONTINUE_SECONDS = "5"
if ($AgentExecutable) { $env:BS_AGENT_EXECUTABLE = $AgentExecutable }
if ($AgentArguments) { $env:BS_AGENT_ARGUMENTS = $AgentArguments }

$repoRoot = Split-Path $PSScriptRoot -Parent | Split-Path -Parent
Push-Location $repoRoot
try {
    python "$PSScriptRoot\bluesky_orchestrator.py"
}
finally {
    Pop-Location
}
