#!/usr/bin/env bash
# Prueba los cambios del motor de una pasada, en el mismo orden que CI:
#   1. guardia de la ley del repositorio (tools/linea_base_core.py)
#   2. configurar con el preset de macOS (solo si hace falta)
#   3. compilar la suite GL-free (y los tres ejecutables reales con --todo)
#   4. ctest --label-regex gl-free
#
#   ./tools/probar_cambios.sh                # Debug, guardia + suite gl-free
#   ./tools/probar_cambios.sh --todo         # ademas juego, motor_editor, level_editor
#   ./tools/probar_cambios.sh --release      # preset macos-release
#   ./tools/probar_cambios.sh --rapido       # guardia --rapido (sin medir todo)
#   ./tools/probar_cambios.sh -R test_world  # solo las pruebas que casen con el regex
#
# La guardia no corta: se compila y prueba igual y su fallo se informa al
# final. Compilar o probar en rojo si corta. No toca git.
set -uo pipefail
cd "$(dirname "$0")/.."

PRESET=macos-debug
TODO=0
GUARDIA_ARGS=()
FILTRO=()
while [ $# -gt 0 ]; do
  case "$1" in
    --release) PRESET=macos-release ;;
    --todo)    TODO=1 ;;
    --rapido)  GUARDIA_ARGS+=(--rapido) ;;
    -R)        shift; FILTRO=(-R "$1") ;;
    -h|--help) sed -n '2,15p' "$0"; exit 0 ;;
    *) echo "Opcion desconocida: $1" >&2; exit 2 ;;
  esac
  shift
done

BUILD="out/build/$PRESET"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"
INICIO=$SECONDS

paso() { printf '\n\033[1;34m== %s ==\033[0m\n' "$1"; }
falla() { printf '\n\033[1;31mFALLA: %s\033[0m\n' "$1" >&2; exit "${2:-1}"; }

paso "1/4 Guardia de la ley del repositorio"
# No corta: si la guardia falla, se sigue compilando y probando para ver el
# resto, y el veredicto se da al final.
GUARDIA_SALIDA="$(python3 tools/linea_base_core.py ${GUARDIA_ARGS[@]+"${GUARDIA_ARGS[@]}"} 2>&1)"
GUARDIA_RC=$?
echo "$GUARDIA_SALIDA" | grep -E "DEUDA NUEVA|^linea_base_core" || echo "$GUARDIA_SALIDA" | tail -5

paso "2/4 Configurar ($PRESET)"
if [ -f "$BUILD/CMakeCache.txt" ]; then
  echo "Ya configurado en $BUILD; CMake se reconfigura solo si cambia CMakeLists."
else
  cmake --preset "$PRESET" || falla "cmake no configura"
fi

paso "3/4 Compilar"
OBJETIVOS=(suite_gl_free)
[ "$TODO" -eq 1 ] && OBJETIVOS+=(juego motor_editor level_editor)
for t in "${OBJETIVOS[@]}"; do
  echo "-- $t"
  cmake --build "$BUILD" --target "$t" -j "$JOBS" || falla "no compila $t"
done

paso "4/4 Pruebas GL-free"
ctest --test-dir "$BUILD" --output-on-failure --label-regex gl-free -j "$JOBS" \
  ${FILTRO[@]+"${FILTRO[@]}"} || falla "hay pruebas en rojo"

if [ "$GUARDIA_RC" -ne 0 ]; then
  falla "compila y las pruebas pasan, pero la guardia ha fallado (paso 1); veta commit y push" "$GUARDIA_RC"
fi
printf '\n\033[1;32mTODO VERDE\033[0m (%ss, preset %s)\n' "$((SECONDS - INICIO))" "$PRESET"
