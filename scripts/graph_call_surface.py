import csv
from pathlib import Path


DATA_PATH = Path("data") / "call_surface.csv"
OUTPUT_PATH = Path("figs") / "call_surface_graph.png"


def load_call_surface() -> tuple[list[float], list[float], list[float]]:
    prices: list[float] = []
    expiries: list[float] = []
    strikes: list[float] = []

    with DATA_PATH.open("r", encoding="utf-8", newline="") as csv_file:
        rows = csv.DictReader(csv_file)
        for row in rows:
            prices.append(float(row["price"]))
            expiries.append(float(row["expiry"]))
            strikes.append(float(row["strike"]))

    if not prices:
        raise ValueError(f"No call-surface rows found in {DATA_PATH}")

    return prices, expiries, strikes


def plot_call_surface() -> None:
    import matplotlib.pyplot as plt

    prices, expiries, strikes = load_call_surface()

    figure = plt.figure(figsize=(9, 6.5))
    axes = figure.add_subplot(projection="3d")
    surface = axes.plot_trisurf(
        expiries,
        strikes,
        prices,
        cmap="viridis",
        linewidth=0.1,
        antialiased=True,
    )

    axes.set_title("Call Price Surface")
    axes.set_xlabel("Expiry")
    axes.set_ylabel("Strike price")
    axes.set_zlabel("Call price")
    axes.set_xlim(min(expiries), max(expiries))
    axes.set_ylim(min(strikes), max(strikes))
    axes.view_init(elev=28, azim=135)
    figure.colorbar(surface, ax=axes, shrink=0.65, pad=0.1, label="Call price")
    figure.tight_layout()

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(OUTPUT_PATH, dpi=200)


if __name__ == "__main__":
    if not DATA_PATH.exists():
        print(f"{DATA_PATH} does not exist. Run call_surface first to generate it.")
    else:
        plot_call_surface()
