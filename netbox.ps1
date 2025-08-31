# NetBox CLI PowerShell wrapper
# Usage: .\netbox.ps1 init MyProject

param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$Arguments
)

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$NetBoxCLI = Join-Path $ScriptDir "tools\netbox-cli-v2.py"

# Check if Python script exists
if (-not (Test-Path $NetBoxCLI)) {
    Write-Host "ERROR: NetBox CLI script not found" -ForegroundColor Red
    Write-Host "Expected: $NetBoxCLI" -ForegroundColor Yellow
    Write-Host "Please run from NetBox root directory" -ForegroundColor Yellow
    exit 1
}

# Find Python executable
$PythonCmd = $null

# Try python3 first
try {
    $null = python3 --version 2>$null
    $PythonCmd = "python3"
} catch {
    # Try python
    try {
        $null = python --version 2>$null
        $PythonCmd = "python"
    } catch {
        Write-Host "ERROR: Python not found" -ForegroundColor Red
        Write-Host "Please install Python 3.7+ from https://www.python.org/downloads/" -ForegroundColor Yellow
        exit 1
    }
}

# Execute Python CLI script
# Note: No longer show title here, Python script will show animation
& $PythonCmd $NetBoxCLI @Arguments
