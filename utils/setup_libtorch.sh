#!/bin/bash

# Script to setup LibTorch for macOS (ARM64)
# Automatically downloads and installs LibTorch for macOS with CPU support

echo "=== LibTorch macOS Setup ==="
echo ""

# LibTorch download URL (macOS ARM64 CPU version)
URL="https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-2.10.0.zip"
ZIP_FILE="libtorch-macos-arm64-2.10.0.zip"
LIBTORCH_DIR="libtorch"

# Check if zip file already exists
if [ ! -f "$ZIP_FILE" ]; then
    echo -e "\033[33mDownloading LibTorch from PyTorch.org...\033[0m"
    echo -e "\033[90mURL: $URL\033[0m"
    echo -e "\033[33mThis may take a few minutes...\033[0m"
    echo ""

    if curl -L "$URL" -o "$ZIP_FILE"; then
        echo -e "\033[32mDownload completed successfully!\033[0m"
    else
        echo -e "\033[31mERROR: Failed to download LibTorch!\033[0m"
        echo -e "\033[33mYou can manually download from: $URL\033[0m"
        echo -e "\033[33mSave it as: $ZIP_FILE in this directory\033[0m"
        exit 1
    fi
else
    echo -e "\033[32mLibTorch zip file already exists, skipping download.\033[0m"
fi

echo ""

# Delete existing target LibTorch directory if it exists
if [ -d "$LIBTORCH_DIR" ]; then
    echo -e "\033[33mExisting $LIBTORCH_DIR directory found, deleting...\033[0m"
    rm -rf "$LIBTORCH_DIR"
fi

# Extract the LibTorch
echo -e "\033[32mExtracting LibTorch...\033[0m"
unzip -q "$ZIP_FILE"

echo ""
echo -e "\033[36mNext steps:\033[0m"
echo -e "\033[37m1. Reload CMake project\033[0m"
echo -e "\033[37m2. Build your project\033[0m"
echo ""
