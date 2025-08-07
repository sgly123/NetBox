@echo off
chcp 65001 >nul
REM NetBox CLI Windows wrapper script
REM Simple command line experience: netbox init MyProject

REM Get script directory
set "SCRIPT_DIR=%~dp0"

REM NetBox CLI Python script path
set "NETBOX_CLI=%SCRIPT_DIR%tools\netbox-cli-v2.py"

REM Check if Python script exists
if not exist "%NETBOX_CLI%" (
    echo Error: Cannot find NetBox CLI script
    echo Expected path: %NETBOX_CLI%
    echo Please run this command in NetBox project root directory
    exit /b 1
)

REM Check if Python is available - try python3 first, then python
python3 --version >nul 2>&1
if not errorlevel 1 (
    set "PYTHON_CMD=python3"
    goto python_found
)

python --version >nul 2>&1
if not errorlevel 1 (
    set "PYTHON_CMD=python"
    goto python_found
)

echo Error: Python not found
echo Please install Python 3.7+: https://www.python.org/downloads/
exit /b 1

:python_found
REM Show NetBox title (except for help commands)
if "%1"=="" goto show_title
if "%1"=="--help" goto skip_title
if "%1"=="-h" goto skip_title
if "%1"=="help" goto skip_title

:show_title
echo NetBox CLI v2.0
echo ==================

:skip_title
REM Execute Python CLI script with all parameters
%PYTHON_CMD% "%NETBOX_CLI%" %*
