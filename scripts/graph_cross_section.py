import csv
from pathlib import Path


DATA_PATH = Path("data") / "call_surface.csv"
OUTPUT_PATH = Path("figs") / "cross_sections_graph.png"
TARGET_EXPIRY = 20.0
TARGET_STRIKE = 100.0


def load_call_surface() -> list[dict[str, float]]:
    rows: list[dict[str, float]] = []

    with DATA_PATH.open("r", encoding="utf-8", newline="") as csv_file:
        for row in csv.DictReader(csv_file):
            rows.append(
                {
                    "price": float(row["price"]),
                    "expiry": float(row["expiry"]),
                    "strike": float(row["strike"]),
                }
            )

    if not rows:
        raise ValueError(f"No call-surface rows found in {DATA_PATH}")

    return rows


def nearest_grid_value(
    rows: list[dict[str, float]],
    key: str,
    target: float,
) -> float:
    return min({row[key] for row in rows}, key=lambda value: abs(value - target))


def plot_cross_sections() -> None:
    import matplotlib.pyplot as plt

    rows = load_call_surface()
    fixed_expiry = nearest_grid_value(rows, "expiry", TARGET_EXPIRY)
    fixed_strike = nearest_grid_value(rows, "strike", TARGET_STRIKE)

    strike_slice = sorted(
        (row for row in rows if row["expiry"] == fixed_expiry),
        key=lambda row: row["strike"],
    )
    expiry_slice = sorted(
        (row for row in rows if row["strike"] == fixed_strike),
        key=lambda row: row["expiry"],
    )

    if not strike_slice:
        raise ValueError(f"No rows found for expiry={fixed_expiry:g}")
    if not expiry_slice:
        raise ValueError(f"No rows found for strike={fixed_strike:g}")

    figure, axes = plt.subplots(1, 2, figsize=(11, 4.8))

    axes[0].plot(
        [row["strike"] for row in strike_slice],
        [row["price"] for row in strike_slice],
    )
    axes[0].set_title(f"Price by Strike (Expiry = {fixed_expiry:g})")
    axes[0].set_xlabel("Strike price")
    axes[0].set_ylabel("Call price")

    axes[1].plot(
        [row["expiry"] for row in expiry_slice],
        [row["price"] for row in expiry_slice],
    )
    axes[1].set_title(f"Price by Expiry (Strike = {fixed_strike:g})")
    axes[1].set_xlabel("Expiry")
    axes[1].set_ylabel("Call price")

    for axis in axes:
        axis.grid(True, linestyle="--", linewidth=0.6, alpha=0.5)

    figure.tight_layout()
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(OUTPUT_PATH, dpi=200)


if __name__ == "__main__":
    if not DATA_PATH.exists():
        print(f"{DATA_PATH} does not exist. Run call_surface first to generate it.")
    else:
        plot_cross_sections()
