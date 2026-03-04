#!/usr/bin/env python3

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


REQUIRED_COLUMNS = [
    "Timestamp",
    "TempIn",
    "TempOut",
    "TargetTemp",
    "AIPrediction",
    "ActualPower",
    "Reward",
    "ElectricityPrice",
]

MIN_TEMPERATURE = 18.0
MAX_TEMPERATURE = 24.0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot AIRecord CSV outputs from the simulator/training runs."
    )
    parser.add_argument(
        "csv_path",
        nargs="?",
        default="../build/ai_records_training.csv",
        help="Path to AIRecord CSV file (default: ../build/ai_records_training.csv)",
    )
    parser.add_argument(
        "--output",
        "-o",
        default="",
        help="Optional PNG output path. If omitted, plot is shown interactively.",
    )
    return parser.parse_args()


def load_ai_records(csv_path: Path) -> dict[str, list[float]]:
    if not csv_path.exists():
        raise FileNotFoundError(f"CSV file not found: {csv_path}")

    data = {column: [] for column in REQUIRED_COLUMNS}

    with csv_path.open("r", newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)
        if reader.fieldnames is None:
            raise ValueError("CSV file is empty or missing header.")

        missing = [column for column in REQUIRED_COLUMNS if column not in reader.fieldnames]
        if missing:
            raise ValueError(f"Missing columns in CSV: {', '.join(missing)}")

        for row in reader:
            for column in REQUIRED_COLUMNS:
                data[column].append(float(row[column]))

    if not data["Timestamp"]:
        raise ValueError("CSV has no data rows.")

    return data


def make_plot(data: dict[str, list[float]], output_path: Path | None) -> None:
    time_hours = [value / 3600.0 for value in data["Timestamp"]]

    figure, axes = plt.subplots(3, 1, figsize=(12, 10), sharex=True)

    outdoor_line, = axes[0].plot(time_hours, data["TempOut"], label="TempOut")
    indoor_line, = axes[0].plot(time_hours, data["TempIn"], label="TempIn")
    min_temp_line = axes[0].axhline(MIN_TEMPERATURE, label="MinTemp", linestyle=":")
    max_temp_line = axes[0].axhline(MAX_TEMPERATURE, label="MaxTemp", linestyle=":")
    target_line, = axes[0].plot(time_hours, data["TargetTemp"], label="TargetTemp", linestyle="--")
    axes[0].set_ylabel("Temperature")
    axes[0].set_title("AIRecord - Temperature")
    axes[0].grid(True, alpha=0.3)
    axes[0].legend(handles=[indoor_line, outdoor_line, target_line, min_temp_line, max_temp_line])

    axes[1].plot(time_hours, data["AIPrediction"], label="AIPrediction")
    axes[1].plot(time_hours, data["ActualPower"], label="ActualPower", alpha=0.8)
    axes[1].set_ylabel("Power")
    axes[1].set_title("AIRecord - Predicted vs Actual Power")
    axes[1].grid(True, alpha=0.3)
    axes[1].legend()

    axes[2].plot(time_hours, data["Reward"], label="Reward")
    axes[2].set_xlabel("Time (hours)")
    axes[2].set_ylabel("Value")
    axes[2].set_title("AIRecord - Reward and Electricity Price")
    axes[2].grid(True, alpha=0.3)
    axes[2].legend()

    figure.tight_layout()

    if output_path is not None:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        figure.savefig(output_path, dpi=150)
        print(f"Graph saved to: {output_path}")
        plt.close(figure)
    else:
        plt.show()


def main() -> None:
    args = parse_args()
    csv_path = Path(args.csv_path).expanduser().resolve()
    output_path = Path(args.output).expanduser().resolve() if args.output else None

    data = load_ai_records(csv_path)
    make_plot(data, output_path)


if __name__ == "__main__":
    main()