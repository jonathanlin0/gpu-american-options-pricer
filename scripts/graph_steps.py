import json
from collections import defaultdict
from pathlib import Path
from statistics import fmean, pstdev

import matplotlib.pyplot as plt


DATA_PATH = Path("data") / "data.jsonl"
OUTPUT_PATH = Path("figs") / "steps_graph.png"
TARGET_SIZE = "medium"


def load_timings_by_step() -> tuple[list[int], list[float], list[float], list[float], list[float]]:
    cpu_times_by_step: dict[int, list[float]] = defaultdict(list)
    gpu_times_by_step: dict[int, list[float]] = defaultdict(list)

    with DATA_PATH.open("r", encoding="utf-8") as data_file:
        for line in data_file:
            if not line.strip():
                continue

            run = json.loads(line)
            if run["size"] != TARGET_SIZE:
                continue

            steps = run["steps"]
            cpu_times_by_step[steps].extend(run["cpu_times_ms"])
            gpu_times_by_step[steps].extend(run["gpu_times_ms"])

    if not cpu_times_by_step:
        raise ValueError(f"No benchmark rows found for size={TARGET_SIZE!r}")

    steps_values = sorted(cpu_times_by_step)
    cpu_means = [fmean(cpu_times_by_step[steps]) for steps in steps_values]
    gpu_means = [fmean(gpu_times_by_step[steps]) for steps in steps_values]
    cpu_stds = [pstdev(cpu_times_by_step[steps]) for steps in steps_values]
    gpu_stds = [pstdev(gpu_times_by_step[steps]) for steps in steps_values]

    return steps_values, cpu_means, gpu_means, cpu_stds, gpu_stds


def plot_steps_graph() -> None:
    steps_values, cpu_means, gpu_means, cpu_stds, gpu_stds = load_timings_by_step()

    cpu_lower = [mean - std for mean, std in zip(cpu_means, cpu_stds)]
    cpu_upper = [mean + std for mean, std in zip(cpu_means, cpu_stds)]
    gpu_lower = [mean - std for mean, std in zip(gpu_means, gpu_stds)]
    gpu_upper = [mean + std for mean, std in zip(gpu_means, gpu_stds)]

    plt.figure(figsize=(9, 5.5))

    plt.plot(steps_values, cpu_means, label="CPU")
    plt.fill_between(steps_values, cpu_lower, cpu_upper, alpha=0.18)

    plt.plot(steps_values, gpu_means, label="GPU")
    plt.fill_between(steps_values, gpu_lower, gpu_upper, alpha=0.18)

    plt.title("CPU vs GPU Batch Time by Binomial Steps (size = Medium)")
    plt.xlabel("Number of binomial steps")
    plt.ylabel("Batch time (ms)")
    plt.grid(True, which="both", linestyle="--", linewidth=0.6, alpha=0.5)
    plt.legend()
    plt.tight_layout()

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(OUTPUT_PATH, dpi=200)


if __name__ == "__main__":
    plot_steps_graph()
