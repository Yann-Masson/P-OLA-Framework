#!/usr/bin/env bash
# benchmark_all_homes.sh
# Runs a trainer on all 10 GPS data files and prints a summary for each.
#
# Usage:
#   ./scripts/benchmark_all_homes.sh [--timesteps N] [--model ola|p-ola]
#
# Defaults:
#   --timesteps  43200
#   --model      p-ola

set -euo pipefail

# ── Defaults ──────────────────────────────────────────────────────────────────
TIMESTEPS=43200
MODEL="p-ola"
MODEL_NAME=""

# ── Argument parsing ──────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --timesteps)
            TIMESTEPS="$2"
            shift 2
            ;;
        --model)
            MODEL="$2"
            shift 2
            ;;
        --model-name)
            MODEL_NAME="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [--timesteps N] [--model ola|p-ola] [--model-name NAME]"
            exit 0
            ;;
        *)
            echo "Unknown argument: $1  (use --help for usage)" >&2
            exit 1
            ;;
    esac
done

# ── Resolve paths ─────────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
DATA_DIR="$PROJECT_ROOT/data"

case "$MODEL" in
    ola)
        BINARY="$BUILD_DIR/OLA_Trainer"
        ;;
    p-ola)
        BINARY="$BUILD_DIR/P-OLA_Trainer"
        ;;
    *)
        echo "Unknown model '$MODEL'. Choose 'ola' or 'p-ola'." >&2
        exit 1
        ;;
esac

if [[ ! -x "$BINARY" ]]; then
    echo "Binary not found or not executable: $BINARY" >&2
    echo "Build the project first (cmake --build build)." >&2
    exit 1
fi

# ── Run ───────────────────────────────────────────────────────────────────────
echo "========================================"
echo "  Benchmark — model: $MODEL  |  timesteps: $TIMESTEPS"
echo "========================================"
echo ""

for i in $(seq 1 10); do
    DATA_FILE="$DATA_DIR/data_home_${i}_scheduled_GPS.csv"

    if [[ ! -f "$DATA_FILE" ]]; then
        echo "File not found, skipping: $DATA_FILE"
        echo ""
        continue
    fi

    echo "----------------------------------------"
    echo "  data_home_${i}_scheduled_GPS.csv"
    echo "----------------------------------------"

    # Build optional --output flag
    OUTPUT_ARGS=()
    if [[ -n "$MODEL_NAME" ]]; then
        OUTPUT_ARGS=(--output "$MODEL_NAME")
    fi

    OUTPUT=$("$BINARY" \
        "${OUTPUT_ARGS[@]+${OUTPUT_ARGS[@]}}" \
        --w-comfort 0.9 \
        --w-economy 0.1 \
        --timesteps "$TIMESTEPS" \
        --no-save \
        --data "$DATA_FILE" \
        --output-data "./scripts/output/${MODEL}_ai_records_home_${i}.csv" \
        2>&1)

    echo "$OUTPUT" | tail -4
    echo ""
done

echo "========================================"
echo "  Done."
echo "========================================"
