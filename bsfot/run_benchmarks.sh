#!/bin/bash

set -uo pipefail

CURVES_FILE="curves.txt"

if [ ! -f "$CURVES_FILE" ]; then
    echo "[ERREUR] Fichier $CURVES_FILE introuvable."
    exit 1
fi

mapfile -t CURVES < <(
    grep -v '^#' "$CURVES_FILE" | grep -v '^$'
)

if [ "${#CURVES[@]}" -eq 0 ]; then
    echo "[ERREUR] Aucune courbe définie dans $CURVES_FILE"
    exit 1
fi

# ============================================================
# Mode
# ============================================================

MODE="${1:-${MODE:-0}}"

if [[ "$MODE" != "0" && "$MODE" != "1" ]]; then
    echo "[ERREUR] MODE doit être 0 (Classic OT) ou 1 (Dual-Mode OT)"
    exit 1
fi

if [[ "$MODE" == "0" ]]; then
    MODE_NAME="OT"
else
    MODE_NAME="dual_OT"
fi

# ============================================================
# Taille du message
# ============================================================

MESSAGE_BITS="${2:-${MESSAGE_BITS:-}}"

if [[ -z "$MESSAGE_BITS" ]]; then
    echo "[ERREUR] Taille de message manquante."
    echo "Usage : $0 <MODE> <MESSAGE_BITS>"
    echo
    echo "Exemples :"
    echo "  $0 0 128    # Classic OT, message de 128 bits"
    echo "  $0 1 256    # Dual-Mode OT, message de 256 bits"
    exit 1
fi

if ! [[ "$MESSAGE_BITS" =~ ^[0-9]+$ ]] || [ "$MESSAGE_BITS" -le 0 ]; then
    echo "[ERREUR] MESSAGE_BITS doit être un entier positif."
    exit 1
fi

# ============================================================
# Configuration
# ============================================================

RELIC_SRC="${RELIC_SRC:-$HOME/relic}"
JOBS="${NPROC:-$(nproc)}"

RESULTS_DIR="results"
LOG_DIR="$RESULTS_DIR/logs"
INSTALL_LOG_DIR="$LOG_DIR/install_logs"
BUILD_LOG_DIR="$LOG_DIR/build_logs"
IMAGE_DIR="$RESULTS_DIR/images"

# ============================================================
# Création des dossiers
# ============================================================

mkdir -p \
    "$LOG_DIR" \
    "$INSTALL_LOG_DIR" \
    "$BUILD_LOG_DIR" \
    "$IMAGE_DIR"

# ============================================================
# Nettoyage des logs de compilation uniquement
# ============================================================

rm -f "$BUILD_LOG_DIR"/*.log

# ============================================================
# Sudo
# ============================================================

echo
echo "Ce script nécessite sudo pour installer les bibliothèques RELIC manquantes dans /opt"

sudo -v

(
    while true
    do
        sudo -n true
        sleep 60
        kill -0 "$$" 2>/dev/null || exit
    done
) 2>/dev/null &

SUDO_KEEPALIVE_PID=$!

trap 'kill "$SUDO_KEEPALIVE_PID" 2>/dev/null' EXIT

# ============================================================
# Installation RELIC
# ============================================================

install_curve()
{
    local curve="$1"
    local install_dir="/opt/relic-${curve}"

    if [ -f "$install_dir/lib/librelic_s.a" ]; then
        echo "[OK] $curve déjà installé ($install_dir)"
        return 0
    fi

    local preset_script="$RELIC_SRC/preset/x64-pbc-${curve}.sh"

    if [ ! -f "$preset_script" ]; then
        echo "[ERREUR] Aucun preset RELIC trouvé pour '$curve'"
        echo "         Cherché : $preset_script"
        return 1
    fi

    echo "[INSTALL] $curve -> $install_dir"

    local build_dir="$RELIC_SRC/build-${curve}"
    local log_file="$INSTALL_LOG_DIR/${curve}.log"

    rm -rf "$build_dir" > "$log_file" 2>&1
    mkdir -p "$build_dir"

    (
        cd "$build_dir" &&
        "$preset_script" .. &&
        cmake -DCMAKE_INSTALL_PREFIX="$install_dir" . &&
        make -j"$JOBS" &&
        sudo make install
    ) >> "$log_file" 2>&1

    if [ ! -f "$install_dir/lib/librelic_s.a" ]; then
        echo "[ERREUR] Installation de $curve échouée — voir $log_file"
        return 1
    fi

    echo "[OK] $curve installé."
    echo
}

# ============================================================
# Benchmark
# ============================================================

echo
echo "============================================================"
echo "Benchmark ${#CURVES[@]} courbes"
echo "Mode : $MODE_NAME"
echo "Taille message : $MESSAGE_BITS bits"
echo "Courbes : ${CURVES[*]}"
echo "============================================================"
echo

for curve in "${CURVES[@]}"
do

    echo "=== Courbe : $curve ==="

    # --------------------------------------------------------
    # Installation RELIC
    # --------------------------------------------------------

    if ! install_curve "$curve"; then
        echo
        continue
    fi

    # --------------------------------------------------------
    # Compilation
    # --------------------------------------------------------

    make clean > /dev/null

    BUILD_LOG="$BUILD_LOG_DIR/${curve}_build.log"

    if ! make CURVE="$curve" MESSAGE_BITS="$MESSAGE_BITS" > "$BUILD_LOG" 2>&1
    then
        echo "[ERREUR] Compilation échouée pour $curve — voir $BUILD_LOG"
        echo
        continue
    fi

    # La compilation est terminée,
    # on peut supprimer son log temporaire.

    rm -f "$BUILD_LOG"

    # --------------------------------------------------------
    # Benchmark
    # --------------------------------------------------------

    # Le mode et la taille du message font partie du nom du fichier.
    #
    # MODE=0 :
    #   BLS12_381_OT_128bits.log
    #
    # MODE=1 :
    #   BLS12_381_dual_OT_256bits.log

    LOGFILE="$LOG_DIR/${curve}_${MODE_NAME}.log"

    if ./main "$MODE" "$MESSAGE_BITS" > "$LOGFILE" 2>&1
    then

        echo "[OK] $curve [$MODE_NAME, ${MESSAGE_BITS} bits] -> $LOGFILE"
        echo

        cat "$LOGFILE"

    else

        echo "[ERREUR] Exécution échouée pour $curve — voir $LOGFILE"

    fi

    echo

done

# ============================================================
# Fin du benchmark
# ============================================================

echo "========================== Done ============================"
echo "Mode exécuté : $MODE_NAME"
echo "Taille message : $MESSAGE_BITS bits"
echo "Logs disponibles dans : $LOG_DIR"
echo "============================================================"

# ============================================================
# Génération automatique des graphiques
# ============================================================

if [ -f "display.py" ]
then


    python3 display.py

    if [ $? -eq 0 ]
    then
        echo "[OK] Graphiques générés dans $IMAGE_DIR"
    else
        echo "[ERREUR] Impossible de générer les graphiques."
    fi

else

    echo "[INFO] display.py introuvable, graphiques ignorés."

fi