#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GAME_DIR="${SCRIPT_DIR}/../game"

export PVSNESLIB_HOME="/c/dev/snes/tools/pvsneslib"
export PATH="/c/msys64/ucrt64/bin:/c/msys64/usr/bin:${PATH:-}"

cd "${GAME_DIR}"

echo
printf '===========================\n'
printf 'Cleaning and rebuilding...\n'
printf '===========================\n'
echo

make clean
make

echo
printf '===========================\n'
printf 'Launching Mesen...\n'
printf '===========================\n'
echo

"/c/dev/snes/tools/mesen/Mesen.exe" "${GAME_DIR}/star_merchants.sfc"
