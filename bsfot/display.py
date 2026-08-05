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


def plot_breakdown(results):

    curves = list(results.keys())

    bottom = [0] * len(curves)

    plt.figure(figsize=(12, 6))

    colors = [
        "#1D3557",
        "#457B9D",
        "#A8DADC",
        "#2A9D8F",
        "#6D597A",
        "#B8C0FF",
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

    # Affichage du temps total au-dessus de chaque barre
    for i, total in enumerate(bottom):
        plt.text(
            i,
            total,
            f"{total:.1f}",
            ha="center",
            va="bottom",
            fontsize=9
        )

    plt.ylabel("Temps (ms)")
    plt.xlabel("Courbe RELIC")
    plt.title("Temps d'exécution de BSFOT en fonction des courbes de pairing")

    plt.legend()
    plt.grid(
        axis="y",
        alpha=0.3
    )

    plt.xticks(rotation=60, ha="right")

    plt.tight_layout()

    plt.savefig(
        f"{IMAGE_DIR}/benchmark.png",
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

    plot_breakdown(results)

    print("Graphique généré :")
    print(" - results/images/breakdown.png")


if __name__ == "__main__":
    main()