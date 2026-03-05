#!/usr/bin/env python3

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches


REQUIRED_COLUMNS = [
    "Timestamp",
    "TempIn",
    "ForecastTemp_h0",
    "PrefMinTemp",
    "PrefMaxTemp",
    "AIPrediction",
    "ActualPower",
    "Reward",
    "ElectricityPrice",
]

OPTIONAL_COLUMNS = [
    "UserPresent_h0",
    "UserDistanceKm",
]


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

    all_columns = REQUIRED_COLUMNS + OPTIONAL_COLUMNS
    data: dict[str, list[float]] = {col: [] for col in all_columns}

    with csv_path.open("r", newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)
        if reader.fieldnames is None:
            raise ValueError("CSV file is empty or missing header.")

        missing = [col for col in REQUIRED_COLUMNS if col not in reader.fieldnames]
        if missing:
            raise ValueError(f"Missing columns in CSV: {', '.join(missing)}")

        available_optional = [col for col in OPTIONAL_COLUMNS if col in reader.fieldnames]

        for row in reader:
            for col in REQUIRED_COLUMNS:
                data[col].append(float(row[col]))
            for col in available_optional:
                data[col].append(float(row[col]))

    if not data["Timestamp"]:
        raise ValueError("CSV has no data rows.")

    # Remove optional columns that were not in the file
    for col in OPTIONAL_COLUMNS:
        if not data[col]:
            del data[col]

    return data


def make_plot(data: dict[str, list[float]], output_path: Path | None) -> None:
    time_hours = [v / 3600.0 for v in data["Timestamp"]]

    has_presence = "UserPresent_h0" in data
    has_distance = "UserDistanceKm" in data

    figure, axes = plt.subplots(4, 1, figsize=(14, 14), sharex=True)

    # ── Panel 1 : Temperatures ─────────────────────────────────────────────────
    ax = axes[0]
    ax.fill_between(
        time_hours,
        data["PrefMinTemp"],
        data["PrefMaxTemp"],
        alpha=0.15,
        color="green",
        label="Comfort zone",
    )
    ax.plot(time_hours, data["PrefMinTemp"], color="green", linestyle="--", linewidth=0.8, label="PrefMin")
    ax.plot(time_hours, data["PrefMaxTemp"], color="green", linestyle="--", linewidth=0.8, label="PrefMax")
    ax.plot(time_hours, data["ForecastTemp_h0"], color="steelblue", linewidth=1.2, label="Outdoor temp")
    ax.plot(time_hours, data["TempIn"], color="tomato", linewidth=1.5, label="Indoor temp")
    ax.set_ylabel("Temperature (°C)")
    ax.set_title("Temperature")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right", fontsize=8)

    # ── Panel 2 : User presence + distance ────────────────────────────────────
    ax = axes[1]
    ax.set_title("User presence")
    ax.set_ylabel("Present (0/1)")
    ax.set_ylim(-0.05, 1.15)
    ax.grid(True, alpha=0.3)

    legend_handles = []

    if has_presence:
        ax.fill_between(
            time_hours,
            data["UserPresent_h0"],
            step="post",
            alpha=0.35,
            color="mediumpurple",
            label="Present (h0)",
        )
        (presence_line,) = ax.step(
            time_hours,
            data["UserPresent_h0"],
            where="post",
            color="mediumpurple",
            linewidth=1.2,
        )
        legend_handles.append(mpatches.Patch(color="mediumpurple", alpha=0.6, label="Present (h0)"))

    if has_distance:
        ax2 = ax.twinx()
        ax2.set_ylabel("Distance (km)", color="darkorange")
        (dist_line,) = ax2.plot(
            time_hours,
            data["UserDistanceKm"],
            color="darkorange",
            linewidth=1.2,
            label="Distance (km)",
        )
        ax2.tick_params(axis="y", labelcolor="darkorange")
        legend_handles.append(dist_line)

    if legend_handles:
        ax.legend(handles=legend_handles, loc="upper right", fontsize=8)

    if not has_presence and not has_distance:
        ax.text(0.5, 0.5, "No presence/distance data", transform=ax.transAxes,
                ha="center", va="center", color="gray")

    # ── Panel 3 : Electricity price ────────────────────────────────────────────
    ax = axes[2]
    ax.fill_between(time_hours, data["ElectricityPrice"], alpha=0.25, color="goldenrod")
    ax.plot(time_hours, data["ElectricityPrice"], color="goldenrod", linewidth=1.2, label="Electricity price")
    ax.set_ylabel("Price")
    ax.set_title("Electricity price")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right", fontsize=8)

    # ── Panel 4 : Heater power ─────────────────────────────────────────────────
    ax = axes[3]
    ax.fill_between(time_hours, data["ActualPower"], alpha=0.25, color="firebrick", step="post")
    ax.step(time_hours, data["ActualPower"], where="post", color="firebrick", linewidth=1.2, label="Heater power")
    ax.set_xlabel("Time (hours)")
    ax.set_ylabel("Power")
    ax.set_title("Heater power")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right", fontsize=8)

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