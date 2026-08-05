#!/bin/bash
#
# Installe (si besoin) puis benchmark RELIC sur les courbes définies dans curves.txt.
#
# Usage :
#   ./run_benchmarks.sh
#
# Le fichier curves.txt contient une courbe par ligne.
#
# Exemple curves.txt :
#   bn254
#   bls12-381
#   kss18-638
#
# Variables d'environnement optionnelles :
#   MODE=0             argument passé à ./main (défaut: 0)
#   RELIC_SRC=~/relic  chemin vers le clone git de RELIC
#   NPROC=4            nombre de jobs de compilation

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


MODE="${MODE:-0}"
RELIC_SRC="${RELIC_SRC:-$HOME/relic}"
JOBS="${NPROC:-$(nproc)}"


RESULTS_DIR="results"
LOG_DIR="$RESULTS_DIR/logs"
INSTALL_LOG_DIR="$LOG_DIR/install_logs"
BUILD_LOG_DIR="$LOG_DIR/build_logs"
IMAGE_DIR="$RESULTS_DIR/images"


mkdir -p \
    "$LOG_DIR" \
    "$INSTALL_LOG_DIR" \
    "$BUILD_LOG_DIR" \
    "$IMAGE_DIR"


# Nettoyage des anciens résultats
rm -f "$LOG_DIR"/*.log
rm -f "$BUILD_LOG_DIR"/*.log
rm -f "$IMAGE_DIR"/*.png


# --- Garde les credentials sudo actifs pendant toute la durée du script ---
echo "Ce script a besoin de 'sudo' pour installer dans /opt — mot de passe si demandé :"
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


echo
echo "=== Benchmark ${#CURVES[@]} courbes : ${CURVES[*]} ==="
echo


for curve in "${CURVES[@]}"
do

    #echo "--- Courbe : $curve ---"


    if ! install_curve "$curve"; then
        echo
        continue
    fi


    make clean > /dev/null


    BUILD_LOG="$BUILD_LOG_DIR/${curve}_build.log"


    if ! make CURVE="$curve" > "$BUILD_LOG" 2>&1
    then
        echo "[ERREUR] Compilation échouée pour $curve — voir $BUILD_LOG"
        echo
        continue
    fi


    rm -f "$BUILD_LOG"


    LOGFILE="$LOG_DIR/${curve}.log"


    if ./main "$MODE" > "$LOGFILE" 2>&1
    then
        echo "[OK] $curve -> $LOGFILE"
        echo
        cat "$LOGFILE"

    else
        echo "[ERREUR] Exécution échouée pour $curve — voir $LOGFILE"
    fi


    echo

done


echo "=== Done ==="



# Génération automatique des graphiques

if [ -f "display.py" ]
then

    echo
    echo "=== Génération des graphiques ==="

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