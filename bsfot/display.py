#!/usr/bin/env python3

import os
import re
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Patch


# ============================================================
# Configuration
# ============================================================

LOG_DIR = "results/logs"
IMAGE_DIR = "results/images"

METRICS = [
    "BS_Gen",
    "BS_User",
    "BS_Signer",
    "BS_Derive",
    "BS_Verify",
]

METRIC_COLORS = {
    "BS_Gen": "#1D3557",
    "BS_User": "#457B9D",
    "BS_Signer": "#2A9D8F",
    "BS_Derive": "#6D597A",
    "BS_Verify": "#B8C0FF",
}

# Opacité des deux modes
OT_ALPHA = 0.70
DUAL_OT_ALPHA = 1.0


# ============================================================
# Lecture d'un log
# ============================================================

def parse_log(filepath):

    results = {}

    with open(filepath, "r") as file:
        content = file.read()

    # --------------------------------------------------------
    # Taille du message
    # --------------------------------------------------------

    match = re.search(
        r"Message\s*:\s*(\d+)\s*bits",
        content
    )

    if match:
        results["Message"] = int(match.group(1))

    # --------------------------------------------------------
    # Métriques
    # --------------------------------------------------------

    for metric in METRICS + ["crypto_time", "Total"]:

        match = re.search(
            rf"{metric}\s*:\s*([0-9.]+)\s*ms",
            content
        )

        if match:
            results[metric] = float(match.group(1))

    # --------------------------------------------------------
    # Nombre d'itérations
    # --------------------------------------------------------

    match = re.search(
        r"Iterations\s*:\s*(\d+)",
        content
    )

    if match:
        results["Iterations"] = int(match.group(1))

    # --------------------------------------------------------
    # Mode
    # --------------------------------------------------------

    match = re.search(
        r"Mode\s*:\s*(.+)",
        content
    )

    if match:
        results["Mode"] = match.group(1).strip()

    return results


# ============================================================
# Chargement des résultats
# ============================================================

def load_results():

    benchmarks = {}

    if not os.path.isdir(LOG_DIR):
        return benchmarks

    for filename in os.listdir(LOG_DIR):

        if not filename.endswith(".log"):
            continue

        # Ignorer les build logs
        if filename.endswith("_build.log"):
            continue

        # ----------------------------------------------------
        # Détection du mode depuis le nom du fichier
        # ----------------------------------------------------

        if filename.endswith("_dual_OT.log"):

            curve = filename[:-len("_dual_OT.log")]
            mode = "dual_OT"

        elif filename.endswith("_OT.log"):

            curve = filename[:-len("_OT.log")]
            mode = "OT"

        else:
            continue

        filepath = os.path.join(
            LOG_DIR,
            filename
        )

        data = parse_log(filepath)

        # Le nom du fichier fait autorité
        data["Mode"] = mode

        benchmarks[(curve, mode)] = data

    return benchmarks


# ============================================================
# Tri des courbes par temps crypto
# ============================================================

def sort_curves_by_crypto_time(results):

    curves = sorted(
        set(
            curve
            for curve, mode in results.keys()
        )
    )

    def average_crypto_time(curve):

        values = []

        for mode in ["OT", "dual_OT"]:

            data = results.get(
                (curve, mode),
                {}
            )

            if "crypto_time" in data:
                values.append(
                    data["crypto_time"]
                )

        # Si aucun crypto_time n'existe,
        # on place la courbe à la fin.
        if not values:
            return float("inf")

        return sum(values) / len(values)

    return sorted(
        curves,
        key=average_crypto_time
    )


# ============================================================
# Informations générales
# ============================================================

def get_benchmark_info(results):

    if not results:
        return "?", "?"

    first = next(iter(results.values()))

    message_bits = first.get(
        "Message",
        "?"
    )

    iterations = first.get(
        "Iterations",
        "?"
    )

    return message_bits, iterations


# ============================================================
# Graphique empilé OT vs dual_OT
# ============================================================

def plot_comparison_stacked(results):

    curves = sort_curves_by_crypto_time(results)

    if not curves:
        return

    x = np.arange(len(curves))
    width = 0.36

    # --------------------------------------------------------
    # Figure
    # --------------------------------------------------------

    fig, ax = plt.subplots(
        figsize=(14, 7)
    )

    # Laisse de la place en haut pour le titre/sous-titre
    fig.subplots_adjust(
        top=0.82,
        bottom=0.12,
        left=0.09,
        right=0.97
    )

    ot_bottom = np.zeros(len(curves))
    dual_bottom = np.zeros(len(curves))

    # --------------------------------------------------------
    # Barres empilées
    # --------------------------------------------------------

    for metric in METRICS:

        ot_values = np.array([
            results.get(
                (curve, "OT"),
                {}
            ).get(metric, 0)
            for curve in curves
        ])

        dual_values = np.array([
            results.get(
                (curve, "dual_OT"),
                {}
            ).get(metric, 0)
            for curve in curves
        ])

        color = METRIC_COLORS[metric]

        # OT
        ax.bar(
            x - width / 2,
            ot_values,
            width,
            bottom=ot_bottom,
            color=color,
            alpha=OT_ALPHA
        )

        # dual_OT
        ax.bar(
            x + width / 2,
            dual_values,
            width,
            bottom=dual_bottom,
            color=color,
            alpha=DUAL_OT_ALPHA
        )

        ot_bottom += ot_values
        dual_bottom += dual_values

    # --------------------------------------------------------
    # Temps total au-dessus des barres
    # --------------------------------------------------------

    for i, curve in enumerate(curves):

        ot_total = sum(
            results.get(
                (curve, "OT"),
                {}
            ).get(metric, 0)
            for metric in METRICS
        )

        dual_total = sum(
            results.get(
                (curve, "dual_OT"),
                {}
            ).get(metric, 0)
            for metric in METRICS
        )

        if ot_total > 0:

            ax.text(
                i - width / 2,
                ot_total,
                f"{ot_total:.1f}",
                ha="center",
                va="bottom",
                fontsize=8
            )

        if dual_total > 0:

            ax.text(
                i + width / 2,
                dual_total,
                f"{dual_total:.1f}",
                ha="center",
                va="bottom",
                fontsize=8
            )

    # --------------------------------------------------------
    # Axes
    # --------------------------------------------------------

    message_bits, iterations = get_benchmark_info(
        results
    )

    ax.set_ylabel(
        "Time (ms)"
    )

    ax.set_xlabel(
        "RELIC curves"
    )

    ax.set_xticks(x)

    ax.set_xticklabels(
        curves,
        rotation=60,
        ha="right"
    )

    # --------------------------------------------------------
    # Titre
    # --------------------------------------------------------

    ax.set_title(
    "BSFOT Execution Time : OT vs dual-OT",
    fontsize=16,
    fontweight="bold",
    pad=28
)
    ax.text(
        0.5,
        1.015,
        f"Message size: {message_bits} bits | "
        f"Benchmark iterations: {iterations}",
        transform=ax.transAxes,
        ha="center",
        va="bottom",
        fontsize=10,
        color="dimgray"
    )

    # --------------------------------------------------------
    # Grille
    # --------------------------------------------------------

    ax.grid(
        axis="y",
        alpha=0.3
    )

    # --------------------------------------------------------
    # Légende des métriques
    # --------------------------------------------------------

    metric_handles = [
        Patch(
            facecolor=METRIC_COLORS[metric],
            alpha=1.0,
            label=metric
        )
        for metric in METRICS
    ]

    legend_metrics = ax.legend(
        handles=metric_handles,
        title="Operations",
        loc="upper left",
        frameon=True
    )

    ax.add_artist(legend_metrics)

    # --------------------------------------------------------
    # Légende des modes
    # --------------------------------------------------------

    mode_handles = [
        Patch(
            facecolor="#555555",
            alpha=OT_ALPHA,
            label="OT"
        ),
        Patch(
            facecolor="#555555",
            alpha=DUAL_OT_ALPHA,
            label="dual_OT"
        ),
    ]

    ax.legend(
        handles=mode_handles,
        title="Mode",
        loc="upper left",
        bbox_to_anchor=(0.0, 0.67),
        frameon=True
    )

    # --------------------------------------------------------
    # Sauvegarde
    # --------------------------------------------------------

    filename = os.path.join(
        IMAGE_DIR,
        "benchmark_OT_vs_dual-OT.png"
    )

    plt.savefig(
        filename,
        dpi=300,
        bbox_inches="tight"
    )

    plt.close()

    print(
        f"[OK] {filename}"
    )


# ============================================================
# Graphique individuel par métrique
# ============================================================

def plot_single_metric_comparison(
    results,
    metric
):

    curves = sort_curves_by_crypto_time(results)

    if not curves:
        return

    x = np.arange(len(curves))
    width = 0.36

    ot_values = np.array([
        results.get(
            (curve, "OT"),
            {}
        ).get(metric, 0)
        for curve in curves
    ])

    dual_values = np.array([
        results.get(
            (curve, "dual_OT"),
            {}
        ).get(metric, 0)
        for curve in curves
    ])

    fig, ax = plt.subplots(
        figsize=(14, 7)
    )

    fig.subplots_adjust(
        top=0.82,
        bottom=0.12,
        left=0.09,
        right=0.97
    )

    # --------------------------------------------------------
    # OT
    # --------------------------------------------------------

    bars_ot = ax.bar(
        x - width / 2,
        ot_values,
        width,
        label="OT",
        color=METRIC_COLORS[metric],
        alpha=OT_ALPHA
    )

    # --------------------------------------------------------
    # dual_OT
    # --------------------------------------------------------

    bars_dual = ax.bar(
        x + width / 2,
        dual_values,
        width,
        label="dual_OT",
        color=METRIC_COLORS[metric],
        alpha=DUAL_OT_ALPHA
    )

    # --------------------------------------------------------
    # Valeurs
    # --------------------------------------------------------

    for bars in [bars_ot, bars_dual]:

        for bar in bars:

            value = bar.get_height()

            if value > 0:

                ax.text(
                    bar.get_x() + bar.get_width() / 2,
                    value,
                    f"{value:.1f}",
                    ha="center",
                    va="bottom",
                    fontsize=8
                )

    # --------------------------------------------------------
    # Informations
    # --------------------------------------------------------

    message_bits, iterations = get_benchmark_info(
        results
    )

    ax.set_ylabel(
        "Time (ms)"
    )

    ax.set_xlabel(
        "RELIC curves"
    )

    ax.set_xticks(x)

    ax.set_xticklabels(
        curves,
        rotation=60,
        ha="right"
    )

    # --------------------------------------------------------
    # Titre + sous-titre
    # --------------------------------------------------------

    ax.set_title(
        f"{metric} : OT vs dual-OT\n",
        fontsize=15,
        fontweight="bold",
        pad=15
    )

    ax.text(
            0.5,
            1.015,
            f"Message size: {message_bits} bits | "
            f"Benchmark iterations: {iterations}",
            transform=ax.transAxes,
            ha="center",
            va="bottom",
            fontsize=10,
            color="dimgray"
    )

    # --------------------------------------------------------
    # Légende
    # --------------------------------------------------------

    ax.legend(
        loc="upper left",
        frameon=True
    )

    # --------------------------------------------------------
    # Grille
    # --------------------------------------------------------

    ax.grid(
        axis="y",
        alpha=0.3
    )

    # --------------------------------------------------------
    # Sauvegarde
    # --------------------------------------------------------

    filename = os.path.join(
        IMAGE_DIR,
        f"{metric.lower()}_OT_vs_dual-OT.png"
    )

    plt.savefig(
        filename,
        dpi=300,
        bbox_inches="tight"
    )

    plt.close()

    print(
        f"[OK] {filename}"
    )


# ============================================================
# Main
# ============================================================

def main():

    os.makedirs(
        IMAGE_DIR,
        exist_ok=True
    )

    results = load_results()

    if not results:

        print(
            "[ERREUR] Aucun résultat OT / dual_OT trouvé "
            f"dans {LOG_DIR}/"
        )

        return

    # --------------------------------------------------------
    # Génération
    # --------------------------------------------------------

    print()
    print("=== Génération des graphiques ===")

    # Graphique empilé
    plot_comparison_stacked(
        results
    )

    # Graphiques individuels
    for metric in METRICS:

        plot_single_metric_comparison(
            results,
            metric
        )

    print()
    print("=== Graphiques générés ===")
    print(f"Répertoire : {IMAGE_DIR}/")


# ============================================================
# Entrée
# ============================================================

if __name__ == "__main__":
    main()