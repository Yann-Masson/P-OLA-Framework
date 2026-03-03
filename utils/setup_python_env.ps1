# Script to setup Python environment for the P-OLA Framework data processing

Write-Host "=== Python Environment Setup ===" -ForegroundColor Green
Write-Host ""

# Check if Python 3 is installed
$pythonExists = $null
try {
    $pythonExists = python --version 2>$null
} catch {
    $pythonExists = $null
}

if ($null -eq $pythonExists) {
    Write-Host "ERROR: Python 3 is not installed. Please install Python 3.8 or higher." -ForegroundColor Red
    exit 1
}

Write-Host "Python version: $(python --version)"
Write-Host ""

# Get the project root directory
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir

Write-Host "Project root: $projectRoot"
Write-Host ""

# Change to project root
Set-Location $projectRoot

# Create virtual environment
Write-Host "[1/3] Creating virtual environment..." -ForegroundColor Cyan
python -m venv .venv

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to create virtual environment" -ForegroundColor Red
    exit 1
}

Write-Host "Virtual environment created successfully" -ForegroundColor Green
Write-Host ""

# Activate virtual environment
Write-Host "[2/3] Activating virtual environment..." -ForegroundColor Cyan
& ".venv\Scripts\Activate.ps1"

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to activate virtual environment" -ForegroundColor Red
    exit 1
}

Write-Host "Virtual environment activated" -ForegroundColor Green
Write-Host ""

# Install required packages
Write-Host "[3/3] Installing required packages..." -ForegroundColor Cyan
python -m pip install --upgrade pip setuptools wheel
python -m pip install pandas numpy

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to install packages" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "=== Setup Complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "To activate the environment in the future, run:" -ForegroundColor Yellow
Write-Host "  .\.venv\Scripts\Activate.ps1"
Write-Host ""
Write-Host "To deactivate the environment, run:" -ForegroundColor Yellow
Write-Host "  deactivate"
Write-Host ""
