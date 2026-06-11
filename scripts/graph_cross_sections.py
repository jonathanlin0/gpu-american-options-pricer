import csv
from pathlib import Path


DATA_PATH = Path("data") / "call_surface.csv"
OUTPUT_PATH = Path("figs") / "cross_sections_graph.png"
FIXED_EXPIRY = 20.0
FIXED_STRIKE = 100.0


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


def plot_cross_sections() -> None:
    import matplotlib.pyplot as plt

    rows = load_call_surface()
    strike_slice = sorted(
        (row for row in rows if row["expiry"] == FIXED_EXPIRY),
        key=lambda row: row["strike"],
    )
    expiry_slice = sorted(
        (row for row in rows if row["strike"] == FIXED_STRIKE),
        key=lambda row: row["expiry"],
    )

    if not strike_slice:
        raise ValueError(f"No rows found for expiry={FIXED_EXPIRY:g}")
    if not expiry_slice:
        raise ValueError(f"No rows found for strike={FIXED_STRIKE:g}")

    figure, axes = plt.subplots(1, 2, figsize=(11, 4.8))

    axes[0].plot(
        [row["strike"] for row in strike_slice],
        [row["price"] for row in strike_slice],
    )
    axes[0].set_title(f"Price by Strike (Expiry = {FIXED_EXPIRY:g})")
    axes[0].set_xlabel("Strike price")
    axes[0].set_ylabel("Call price")

    axes[1].plot(
        [row["expiry"] for row in expiry_slice],
        [row["price"] for row in expiry_slice],
    )
    axes[1].set_title(f"Price by Expiry (Strike = {FIXED_STRIKE:g})")
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
