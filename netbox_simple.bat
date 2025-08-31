@echo off
REM NetBox CLI Windows wrapper - Simple version
REM Usage: netbox init MyProject

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "NETBOX_CLI=%SCRIPT_DIR%tools\netbox-cli-v2.py"

REM Check if script exists
if not exist "%NETBOX_CLI%" (
    echo ERROR: NetBox CLI script not found
    echo Expected: %NETBOX_CLI%
    echo Please run from NetBox root directory
    pause
    exit /b 1
)

REM Try python3 first, then python
python3 --version >nul 2>&1
if not errorlevel 1 (
    python3 "%NETBOX_CLI%" %*
    goto end
)

python --version >nul 2>&1
if not errorlevel 1 (
    python "%NETBOX_CLI%" %*
    goto end
)

echo ERROR: Python not found
echo Please install Python 3.7+ from https://www.python.org/downloads/
pause
exit /b 1

:end
