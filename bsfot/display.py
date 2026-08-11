#!/usr/bin/env python3

import os
import re
import matplotlib.pyplot as plt

LOG_DIR = "results/logs"
IMAGE_DIR = "results/images"

METRICS = [
    "BS_Gen",
    "BS_User",
    "BS_Signer",
    "BS_Derive",
    "BS_Verify",
]

# Couleurs pour chaque métrique
METRIC_COLORS = {
    "BS_Gen": "#1D3557",
    "BS_User": "#457B9D",
    "BS_Signer": "#2A9D8F",
    "BS_Derive": "#6D597A",
    "BS_Verify": "#B8C0FF",
}


def parse_log(filepath):
    results = {}

    with open(filepath, "r") as file:
        content = file.read()

    # Taille du message
    match = re.search(r"Message\s*:\s*(\d+)\s*bits", content)
    if match:
        results["Message"] = int(match.group(1))

    for metric in METRICS + ["crypto_time", "Total"]:
        match = re.search(rf"{metric}\s*:\s*([0-9.]+)\s*ms", content)
        if match:
            results[metric] = float(match.group(1))

    # Nombre d'itérations
    match = re.search(r"Iterations\s*:\s*(\d+)", content)
    if match:
        results["Iterations"] = int(match.group(1))

    # Mode
    match = re.search(r"Mode\s*:\s*(.+)", content)
    if match:
        results["Mode"] = match.group(1).strip()

    return results


def load_results():
    benchmarks = {}

    if not os.path.isdir(LOG_DIR):
        return benchmarks

    for filename in os.listdir(LOG_DIR):

        if not filename.endswith(".log"):
            continue

        if filename.endswith("_build.log"):
            continue

        curve = filename.replace(".log", "")

        benchmarks[curve] = parse_log(
            os.path.join(LOG_DIR, filename)
        )

    return benchmarks


def plot_benchmark(results):

    # Trie par temps crypto croissant
    results = dict(
        sorted(
            results.items(),
            key=lambda x: x[1].get("crypto_time", float("inf"))
        )
    )

    curves = list(results.keys())

    bottom = [0] * len(curves)

    plt.figure(figsize=(12, 6))

    for metric in METRICS:

        values = [
            results[c].get(metric, 0)
            for c in curves
        ]

        plt.bar(
            curves,
            values,
            bottom=bottom,
            label=metric,
            color=METRIC_COLORS[metric]
        )

        bottom = [
            b + v
            for b, v in zip(bottom, values)
        ]

    # Temps total crypto au-dessus des barres
    for i, curve in enumerate(curves):

        crypto_time = results[curve].get("crypto_time", 0)

        plt.text(
            i,
            crypto_time,
            f"{crypto_time:.1f}",
            ha="center",
            va="bottom",
            fontsize=9
        )

    message_bits = next(iter(results.values())).get("Message", "?")
    iterations = next(iter(results.values())).get("Iterations", "?")
    mode = next(iter(results.values())).get("Mode", "?")

    plt.ylabel("Time (ms)")
    plt.xlabel("RELIC curves")

    plt.title(
        "BSFOT Execution Time by Pairing Curve",
        fontsize=14,
        fontweight="bold"
    )

    plt.suptitle(
        f"Mode: {mode}\n"
        f"Message size: {message_bits} bits\n"
        f"Benchmark iterations: {iterations}",
        fontsize=11,
        color="dimgray"
    )

    plt.legend()

    plt.grid(axis="y", alpha=0.3)

    plt.xticks(rotation=60, ha="right")

    plt.tight_layout()

    mode_filename = mode.lower().replace(" ", "")

    plt.savefig(
        f"{IMAGE_DIR}/benchmark_{mode_filename}.png",
        dpi=300
    )

    plt.close()


def plot_single_metric(results, metric, filename, title):

    # Trie par temps crypto croissant
    results = dict(
        sorted(
            results.items(),
            key=lambda x: x[1].get("Crypto", float("inf"))
        )
    )

    curves = list(results.keys())

    values = [
        results[c].get(metric, 0)
        for c in curves
    ]

    plt.figure(figsize=(12, 6))

    plt.bar(
        curves,
        values,
        color=METRIC_COLORS[metric],
        label=metric
    )

    for i, value in enumerate(values):

        plt.text(
            i,
            value,
            f"{value:.1f}",
            ha="center",
            va="bottom",
            fontsize=9
        )

    message_bits = next(iter(results.values())).get("Message", "?")
    iterations = next(iter(results.values())).get("Iterations", "?")
    mode = next(iter(results.values())).get("Mode", "?")

    plt.ylabel("Time (ms)")
    plt.xlabel("RELIC curves")

    plt.title(
        title,
        fontsize=14,
        fontweight="bold"
    )

    plt.suptitle(
        f"Mode: {mode}\n"
        f"Message size: {message_bits} bits\n"
        f"Benchmark iterations: {iterations}",
        fontsize=11,
        color="dimgray"
    )

    plt.legend()

    plt.grid(axis="y", alpha=0.3)

    plt.xticks(rotation=60, ha="right")

    plt.tight_layout()

    mode_filename = mode.lower().replace(" ", "")

    plt.savefig(
        f"{IMAGE_DIR}/{filename.replace('.png', f'_{mode_filename}.png')}",
        dpi=300
    )

    plt.close()


def main():

    os.makedirs(
        IMAGE_DIR,
        exist_ok=True
    )

    results = load_results()

    if not results:
        print("No results found in results/logs/")
        return

    # Graphique empilé
    plot_benchmark(results)

    # Un graphique par métrique
    for metric in METRICS:
        plot_single_metric(
            results,
            metric,
            f"{metric.lower()}.png",
            f"BSFOT {metric} Time"
        )

    print("Graphiques générés:")
    print(f" - {IMAGE_DIR}/benchmark_*.png")

    for metric in METRICS:
        print(f" - {IMAGE_DIR}/{metric.lower()}_*.png")


if __name__ == "__main__":
    main()
