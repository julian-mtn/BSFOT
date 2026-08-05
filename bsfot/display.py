#!/usr/bin/env python3

import os
import re
import matplotlib.pyplot as plt


LOG_DIR = "results/logs"
IMAGE_DIR = "results/images"

METRICS = [
    "Params",
    "OT",
    "KeyGen",
    "Signer",
    "Signature",
    "Verify"
]


def parse_log(filepath):
    results = {}

    with open(filepath, "r") as file:
        content = file.read()

    for metric in METRICS + ["Crypto", "Total"]:
        match = re.search(
            rf"{metric}\s*:\s*([0-9.]+)\s*ms",
            content
        )

        if match:
            results[metric] = float(match.group(1))

    return results


def load_results():
    benchmarks = {}

    if not os.path.isdir(LOG_DIR):
        return benchmarks

    for filename in os.listdir(LOG_DIR):

        if not filename.endswith(".log"):
            continue

        # Ignore les logs de compilation
        if filename.endswith("_build.log"):
            continue

        curve = filename.replace(".log", "")

        benchmarks[curve] = parse_log(
            os.path.join(LOG_DIR, filename)
        )

    return benchmarks


def plot_total(results):

    curves = list(results.keys())

    totals = [
        results[c]["Total"]
        for c in curves
        if "Total" in results[c]
    ]

    curves = [
        c for c in curves
        if "Total" in results[c]
    ]

    plt.figure(figsize=(8, 5))

    bars = plt.bar(
        curves,
        totals,
        color="steelblue"
    )

    plt.ylabel("Temps (ms)")
    plt.xlabel("Courbe RELIC")
    plt.title("Temps total du protocole")

    plt.grid(
        axis="y",
        alpha=0.3
    )

    for bar, value in zip(bars, totals):
        plt.text(
            bar.get_x() + bar.get_width() / 2,
            value,
            f"{value:.1f}",
            ha="center",
            va="bottom"
        )

    plt.tight_layout()

    plt.savefig(
        f"{IMAGE_DIR}/total_time.png",
        dpi=300
    )

    plt.close()


def plot_breakdown(results):

    curves = list(results.keys())

    bottom = [0] * len(curves)

    plt.figure(figsize=(10, 6))

    colors = [
        "royalblue",
        "orange",
        "green",
        "red",
        "purple",
        "brown"
    ]

    for metric, color in zip(METRICS, colors):

        values = [
            results[c].get(metric, 0)
            for c in curves
        ]

        plt.bar(
            curves,
            values,
            bottom=bottom,
            label=metric,
            color=color
        )

        bottom = [
            b + v
            for b, v in zip(bottom, values)
        ]

    plt.ylabel("Temps (ms)")
    plt.xlabel("Courbe RELIC")
    plt.title("Décomposition du temps d'exécution")

    plt.legend()
    plt.grid(
        axis="y",
        alpha=0.3
    )

    plt.tight_layout()

    plt.savefig(
        f"{IMAGE_DIR}/breakdown.png",
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
        print("Aucun résultat trouvé dans results/logs/")
        return

    plot_total(results)
    plot_breakdown(results)

    print("Graphiques générés :")
    print(" - results/images/total_time.png")
    print(" - results/images/breakdown.png")


if __name__ == "__main__":
    main()