import json
from collections import defaultdict
from pathlib import Path
from statistics import fmean, pstdev

import matplotlib.pyplot as plt


DATA_PATH = Path("data") / "data.jsonl"
OUTPUT_PATH = Path("figs") / "num_options_graph.png"
TARGET_STEPS = 500


def load_timings_by_num_options() -> tuple[list[int], list[float], list[float], list[float], list[float]]:
    cpu_times_by_num_options: dict[int, list[float]] = defaultdict(list)
    gpu_times_by_num_options: dict[int, list[float]] = defaultdict(list)

    with DATA_PATH.open("r", encoding="utf-8") as data_file:
        for line in data_file:
            if not line.strip():
                continue

            run = json.loads(line)
            if run["steps"] != TARGET_STEPS:
                continue

            num_options = run["num_options"]
            cpu_times_by_num_options[num_options].extend(run["cpu_times_ms"])
            gpu_times_by_num_options[num_options].extend(run["gpu_times_ms"])

    if not cpu_times_by_num_options:
        raise ValueError(f"No benchmark rows found for steps={TARGET_STEPS}")

    num_options_values = sorted(cpu_times_by_num_options)
    cpu_means = [fmean(cpu_times_by_num_options[num_options]) for num_options in num_options_values]
    gpu_means = [fmean(gpu_times_by_num_options[num_options]) for num_options in num_options_values]
    cpu_stds = [pstdev(cpu_times_by_num_options[num_options]) for num_options in num_options_values]
    gpu_stds = [pstdev(gpu_times_by_num_options[num_options]) for num_options in num_options_values]

    return num_options_values, cpu_means, gpu_means, cpu_stds, gpu_stds


def plot_num_options_graph() -> None:
    num_options_values, cpu_means, gpu_means, cpu_stds, gpu_stds = load_timings_by_num_options()

    cpu_lower = [mean - std for mean, std in zip(cpu_means, cpu_stds)]
    cpu_upper = [mean + std for mean, std in zip(cpu_means, cpu_stds)]
    gpu_lower = [mean - std for mean, std in zip(gpu_means, gpu_stds)]
    gpu_upper = [mean + std for mean, std in zip(gpu_means, gpu_stds)]

    plt.figure(figsize=(9, 5.5))

    plt.plot(num_options_values, cpu_means, label="CPU")
    plt.fill_between(num_options_values, cpu_lower, cpu_upper, alpha=0.18)

    plt.plot(num_options_values, gpu_means, label="GPU")
    plt.fill_between(num_options_values, gpu_lower, gpu_upper, alpha=0.18)

    plt.title(f"CPU vs GPU Batch Time by Number of Options ({TARGET_STEPS} Steps)")
    plt.xlabel("Number of options")
    plt.ylabel("Batch time (ms)")
    plt.grid(True, linestyle="--", linewidth=0.6, alpha=0.5)
    plt.legend()
    plt.tight_layout()

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(OUTPUT_PATH, dpi=200)


if __name__ == "__main__":
    plot_num_options_graph()
