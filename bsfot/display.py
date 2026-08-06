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

    # Taille du message
    match = re.search(
        r"Message\s*:\s*(\d+)\s*bits",
        content
    )

    if match:
        results["Message"] = int(match.group(1))

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

    # Trie les courbes par temps de calcul croissant
    results = dict(
        sorted(
            results.items(),
            key=lambda x: x[1].get("Crypto", float("inf"))
        )
    )

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

    # Affichage du temps de calcul total au-dessus des barres
    for i, curve in enumerate(curves):

        crypto_time = results[curve].get("Crypto", 0)

        plt.text(
            i,
            crypto_time,
            f"{crypto_time:.1f}",
            ha="center",
            va="bottom",
            fontsize=9
        )

    message_bits = next(iter(results.values())).get("Message", "?")

    plt.ylabel("Time (ms)")
    plt.xlabel("RELIC curves")

    plt.title(
        "BSFOT Execution Time by Pairing Curve",
        fontsize=14,
        fontweight="bold"
    )

    plt.suptitle(
        f"Message size: {message_bits} bits",
        fontsize=11,
        color="dimgray"
    )

    plt.legend()

    plt.grid(
        axis="y",
        alpha=0.3
    )

    plt.xticks(
        rotation=60,
        ha="right"
    )

    plt.tight_layout()

    plt.savefig(
        f"{IMAGE_DIR}/benchmark.png",
        dpi=300
    )

    plt.close()

def plot_single_metric(results, metric, color, filename, title):

    # Trie les courbes par temps de calcul croissant
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
        label=metric,
        color=color
    )

    # Affichage du temps au-dessus des barres
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

    plt.ylabel("Time (ms)")
    plt.xlabel("RELIC curves")

    plt.title(
        title,
        fontsize=14,
        fontweight="bold"
    )

    plt.suptitle(
        f"Message size: {message_bits} bits",
        fontsize=11,
        color="dimgray"
    )

    plt.legend()

    plt.grid(
        axis="y",
        alpha=0.3
    )

    plt.xticks(
        rotation=60,
        ha="right"
    )

    plt.tight_layout()

    plt.savefig(
        f"{IMAGE_DIR}/{filename}",
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

    plot_breakdown(results)
    plot_breakdown(results)

    plot_single_metric(
        results,
        "KeyGen",
        "#A8DADC",
        "keygen.png",
        "BSFOT Key Generation Time"
    )

    plot_single_metric(
        results,
        "Verify",
        "#B8C0FF",
        "verify.png",
        "BSFOT Verification Time"
    )

    print("Graphs generated:")
    print(" - results/images/benchmark.png")
    print(" - results/images/keygen_verify.png")


if __name__ == "__main__":
    main()