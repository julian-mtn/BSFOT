#!/bin/bash
#
# Installe (si besoin) puis benchmark RELIC sur une ou plusieurs courbes.
#
# Usage :
#   ./run_benchmarks.sh <courbe1> [<courbe2> ...]
#
# Exemples :
#   ./run_benchmarks.sh bn254 bls381
#   ./run_benchmarks.sh bn254 bls381 kss18-638 bls24-317
#
# Le nom de courbe doit correspondre au suffixe du preset RELIC
# (fichier preset/x64-pbc-<suffixe>.sh), sauf pour les quelques alias
# définis ci-dessous (ex: "bls381" -> preset "bls12-381").
#
# Variables d'environnement optionnelles :
#   MODE=0            argument passé à ./main (défaut: 0)
#   RELIC_SRC=~/relic  chemin vers le clone git de RELIC (défaut: ~/relic)
#   NPROC=4           nombre de jobs de compilation (défaut: nproc)

set -uo pipefail

if [ "$#" -eq 0 ]; then
    echo "Usage: $0 <courbe1> [<courbe2> ...]"
    echo "Exemple: $0 bn254 bls381 kss18-638"
    exit 1
fi

CURVES=("$@")
MODE="${MODE:-0}"
RELIC_SRC="${RELIC_SRC:-$HOME/relic}"
JOBS="${NPROC:-$(nproc)}"

RESULTS_DIR="results"
INSTALL_LOG_DIR="$RESULTS_DIR/install_logs"
mkdir -p "$RESULTS_DIR" "$INSTALL_LOG_DIR"

# --- Alias : nom court utilisé pour /opt/relic-<curve> -> suffixe réel du preset ---
declare -A CURVE_PRESET_ALIAS=(
    [bls381]="bls12-381"
    [bls377]="bls12-377"
    [bls446]="bls12-446"
    [bls455]="bls12-455"
    [bls638]="bls12-638"
)

# --- Garde les credentials sudo actifs pendant toute la durée du script ---
echo "Ce script a besoin de 'sudo' pour installer dans /opt — mot de passe si demandé :"
sudo -v
( while true; do sudo -n true; sleep 60; kill -0 "$$" 2>/dev/null || exit; done ) 2>/dev/null &
SUDO_KEEPALIVE_PID=$!
trap 'kill "$SUDO_KEEPALIVE_PID" 2>/dev/null' EXIT

install_curve() {
    local curve="$1"
    local install_dir="/opt/relic-${curve}"

    if [ -f "$install_dir/lib/librelic_s.a" ]; then
        echo "[OK] $curve déjà installé ($install_dir)"
        return 0
    fi

    local preset_suffix="${CURVE_PRESET_ALIAS[$curve]:-$curve}"
    local preset_script="$RELIC_SRC/preset/x64-pbc-${preset_suffix}.sh"

    if [ ! -f "$preset_script" ]; then
        echo "[ERREUR] Aucun preset pour '$curve' (cherché : $preset_script)"
        echo "         Presets x64-pbc disponibles :"
        ls "$RELIC_SRC/preset" 2>/dev/null | grep '^x64-pbc-' | sed 's/^/           /'
        return 1
    fi

    echo "[INSTALL] $curve -> $install_dir (preset : $(basename "$preset_script"))"

    local build_dir="$RELIC_SRC/build-${curve}"
    local log_file="$INSTALL_LOG_DIR/${curve}.log"

    sudo rm -rf "$build_dir" > "$log_file" 2>&1
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
    return 0
}

echo "=== Benchmark RELIC sur ${#CURVES[@]} courbe(s) : ${CURVES[*]} ==="
echo

for curve in "${CURVES[@]}"; do
    echo "--- Courbe : $curve ---"

    if ! install_curve "$curve"; then
        echo
        continue
    fi

    make clean > /dev/null

    if ! make CURVE="$curve" > "$RESULTS_DIR/${curve}_build.log" 2>&1; then
        echo "[ERREUR] Compilation du projet échouée pour $curve — voir $RESULTS_DIR/${curve}_build.log"
        echo
        continue
    fi

    LOGFILE="$RESULTS_DIR/${curve}.log"
    if ./main "$MODE" > "$LOGFILE" 2>&1; then
        echo "[OK] $curve -> $LOGFILE"
        grep -E "Params|OT|KeyGen|Signer|Signature|Verify|Crypto|Total" "$LOGFILE" | sed 's/^/    /'
    else
        echo "[ERREUR] Exécution échouée pour $curve — voir $LOGFILE"
    fi

    echo
done

echo "=== Terminé. Résultats dans $RESULTS_DIR/ ==="