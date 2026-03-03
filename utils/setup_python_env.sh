#!/bin/bash

# Script to setup Python environment for the P-OLA Framework data processing

echo "=== Python Environment Setup ==="
echo ""

# Check if Python 3 is installed
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 is not installed. Please install Python 3.8 or higher."
    exit 1
fi

echo "Python version: $(python3 --version)"
echo ""

# Go to the project root directory
cd "$(dirname "$0")/.." || exit 1
PROJECT_ROOT=$(pwd)

echo "Project root: $PROJECT_ROOT"
echo ""

# Create virtual environment
echo "[1/3] Creating virtual environment..."
python3 -m venv .venv

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to create virtual environment"
    exit 1
fi

echo "Virtual environment created successfully"
echo ""

# Activate virtual environment
echo "[2/3] Activating virtual environment..."
source .venv/bin/activate

echo "Virtual environment activated"
echo ""

# Install required packages
echo "[3/3] Installing required packages..."
pip install --upgrade pip setuptools wheel
pip install pandas numpy

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to install packages"
    exit 1
fi

echo ""
echo "=== Setup Complete ==="
echo ""
echo "To activate the environment in the future, run:"
echo "  source .venv/bin/activate"
echo ""
echo "To deactivate the environment, run:"
echo "  deactivate"
echo ""
