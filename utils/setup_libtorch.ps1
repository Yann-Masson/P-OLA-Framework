# Script to setup LibTorch for Windows
# Automatically downloads and installs LibTorch for Windows with CUDA support

Write-Host "=== LibTorch Windows Setup ===" -ForegroundColor Cyan

# Detect CUDA version
Write-Host "`nDetecting CUDA installation..." -ForegroundColor Yellow
$cudaPath = $null
if (Test-Path "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.1") {
    $cudaPath = "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.1"
    Write-Host "Found CUDA 13.1 at: $cudaPath" -ForegroundColor Green
    $cudaVersion = "131"
} else {
    Write-Host "ERROR: CUDA 13.1 not found! Please install CUDA 13.1 from NVIDIA's website." -ForegroundColor Red
    Write-Host "Download CUDA 13.1: https://developer.nvidia.com/cuda-downloads" -ForegroundColor Yellow
    exit 1
}

# LibTorch download URL (CUDA 13.0 is the latest stable with good compatibility)
# Note: CUDA 13.x LibTorch binaries may not be available yet, using CUDA 13.0 which works with CUDA 13.x
Write-Host "`nNote: Using LibTorch with CUDA 13.0" -ForegroundColor Cyan

$url = "https://download.pytorch.org/libtorch/cu130/libtorch-win-shared-with-deps-2.10.0%2Bcu130.zip"
$zipFile = "libtorch-shared-with-deps.zip"
$libtorchDir = "libtorch"

# Check if zip file already exists
if (-Not (Test-Path $zipFile)) {
    Write-Host "Downloading LibTorch from PyTorch.org..." -ForegroundColor Yellow
    Write-Host "URL: $url" -ForegroundColor Gray
    Write-Host "This may take a few minutes..." -ForegroundColor Yellow

    try {
        Invoke-WebRequest -Uri $url -OutFile $zipFile -UseBasicParsing
        Write-Host "Download completed successfully!" -ForegroundColor Green
    }
    catch {
        Write-Host "ERROR: Failed to download LibTorch!" -ForegroundColor Red
        Write-Host "Error details: $_" -ForegroundColor Red
        Write-Host "`nYou can manually download from: $url" -ForegroundColor Yellow
        Write-Host "Save it as: $zipFile in this directory" -ForegroundColor Yellow
        exit 1
    }
} else {
    Write-Host "LibTorch zip file already exists, skipping download." -ForegroundColor Green
}

# Delete existing target LibTorch directory if it exists
if (Test-Path $libtorchDir) {
    Write-Host "Existing $libtorchDir directory found, deleting..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $libtorchDir
}

# Extract the new LibTorch (it extracts to "libtorch" folder)
Write-Host "Extracting LibTorch..." -ForegroundColor Green
Expand-Archive -Path $zipFile -DestinationPath . -Force

Write-Host "`nNext steps:" -ForegroundColor Cyan
Write-Host "1. Reload CMake project" -ForegroundColor White
Write-Host "2. Build your project" -ForegroundColor White
