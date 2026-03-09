#!/usr/bin/env bash
# plot_all_results.sh
# Calls plot_ai_records.py for every CSV in scripts/output/ and saves
# the resulting PNG images to results/outputs/.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

INPUT_DIR="$SCRIPT_DIR/output"
OUTPUT_DIR="$REPO_ROOT/results/outputs"
PLOT_SCRIPT="$REPO_ROOT/data/plot_ai_records.py"

mkdir -p "$OUTPUT_DIR"

echo "Input  : $INPUT_DIR"
echo "Output : $OUTPUT_DIR"
echo ""

for csv_file in "$INPUT_DIR"/*.csv; do
    filename="$(basename "$csv_file" .csv)"
    output_png="$OUTPUT_DIR/${filename}.png"

    echo "Plotting $filename ..."
    python "$PLOT_SCRIPT" "$csv_file" --output "$output_png"
done

echo ""
echo "Done. ${OUTPUT_DIR} contains the generated plots."
