#!/usr/bin/env bash
# Prueba de rendimiento reproducible de Onegai (dndWeebCC).
#
# Con la app arrancada (./mvnw spring-boot:run), recorre las rutas principales N veces
# y reporta por ruta: peticiones OK/KO, tiempo medio, mínimo, máximo y p95.
# Ejecuta esto antes y después de un cambio gordo y compara — si el p95 sube, el log
# de peticiones (logs/onegai.log, "PETICION LENTA") te dirá exactamente dónde.
#
# Uso:  bash scripts/prueba_rendimiento.sh [repeticiones] [base_url]
#       bash scripts/prueba_rendimiento.sh 20 http://localhost:8080

set -u
REPETICIONES="${1:-10}"
BASE="${2:-http://localhost:8080}"

RUTAS=(
  "/"
  "/personatges"
  "/personatges/crear"
  "/cartas/clases"
  "/cartas/razas"
  "/cartas/transfondos"
  "/cartas/habilidades"
  "/cartas/armas"
  "/cartas/hechizos"
  "/cartas/deidades"
  "/cartas/enemigos"
  "/cartas/invocaciones"
  "/cartas/trampas"
  "/cartas/condiciones"
  "/historias"
  "/npcs"
  "/tesoros"
  "/aventuras"
  "/aventuras/crear"
  "/eventos"
  "/mapa"
  "/diagnostico"
)

if ! curl -s -o /dev/null --max-time 5 "$BASE/"; then
  echo "✗ No hay respuesta en $BASE — ¿está arrancada la app? (./mvnw spring-boot:run)"
  exit 1
fi

echo "Prueba de rendimiento: $REPETICIONES peticiones por ruta contra $BASE"
echo "(las dos primeras pasadas calientan cachés de Thymeleaf; compara runs completas)"
printf "%-28s %5s %5s %8s %8s %8s %8s\n" "RUTA" "OK" "KO" "media" "min" "max" "p95"

TOTAL_OK=0; TOTAL_KO=0
for ruta in "${RUTAS[@]}"; do
  tiempos=()
  ok=0; ko=0
  for _ in $(seq 1 "$REPETICIONES"); do
    salida=$(curl -s -o /dev/null -w "%{http_code} %{time_total}" --max-time 30 "$BASE$ruta")
    codigo="${salida%% *}"
    segundos="${salida##* }"
    ms=$(awk -v s="$segundos" 'BEGIN { printf "%d", s * 1000 }')
    if [ "$codigo" -ge 200 ] && [ "$codigo" -lt 400 ]; then
      ok=$((ok + 1))
    else
      ko=$((ko + 1))
    fi
    tiempos+=("$ms")
  done
  # estadísticas con awk (media, min, max, p95 sobre la lista ordenada)
  stats=$(printf "%s\n" "${tiempos[@]}" | sort -n | awk '
    { valores[NR] = $1; suma += $1 }
    END {
      media = suma / NR
      p95i = int(NR * 0.95); if (p95i < 1) p95i = 1
      printf "%d %d %d %d", media, valores[1], valores[NR], valores[p95i]
    }')
  read -r media minimo maximo p95 <<< "$stats"
  printf "%-28s %5d %5d %6dms %6dms %6dms %6dms\n" "$ruta" "$ok" "$ko" "$media" "$minimo" "$maximo" "$p95"
  TOTAL_OK=$((TOTAL_OK + ok)); TOTAL_KO=$((TOTAL_KO + ko))
done

echo
echo "TOTAL: $TOTAL_OK OK · $TOTAL_KO KO. Revisa /diagnostico para el desglose vivo"
echo "y logs/onegai.log para el porqué de cada lenta o fallida."
[ "$TOTAL_KO" -eq 0 ] && echo "✓ Run limpia." || echo "✗ Hay rutas fallando — míralas arriba."
