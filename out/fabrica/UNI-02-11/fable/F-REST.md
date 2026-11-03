# F-REST — Restaurar zonas perdidas (Fable, 03/11)

Worktree: `~/onegai-ramas/UNI-RESTAURA` · rama `agente/fable/UNI-RESTAURA` · base `e43db51d`. Motor: `MotorGraphico-main/MotorGraphico/` (= `MG`).
Copia de Oriol (solo lectura): `/Users/admin/Documents/Documentos - Oriol Os (2)/Software/MotorGraphico-main/MotorGraphico/` (= `OR`).

Reglas: solo datos, assets y Python (nada de C++). Git solo con `bash herramientas/git-safe.sh -c core.checkStat=minimal …`, sin push ni merge. JSON de aventura sin tildes/ñ en `lines`/`speaker`/`text`. Python: `ensure_ascii=False`. No toques la vault (`Onegai 2/`). Lee solo los trozos que necesites (`grep`, `sed -n`).

## Tareas (una por commit, trailer `Fabrica-E: F-REST.<n>`)
1. **afueras_ocaso**: 34 objetos de decoración de una versión vieja no están en `MG/assets/levels/afueras_boundington_ocaso.json`. Detalle: `grep -n -A12 "afueras" out/fabrica/UNI-02-11/ZONAS_PARA_FABLE.md`. Aplica los que encajen sobre el nivel actual (no recuperes el nombre viejo `afueras_ocaso`).
2. **ciudad_centro**: `OR/assets/levels/ciudad_centro.json` tiene ~60 `cartel_*`, `estrado_sermon` y 2 puertas que `MG/…` no. Antes, mira si `MG/src/HUD/CartelesPuertas.cpp` ya genera los carteles en juego; trae solo lo que falte de verdad.
3. **Botín**: 12 cofres `botin_*` de `OR` no están en `MG`. Regeneralos con `MG/tools/repartir_botin.py`; no los copies a mano. Ojo PO-57 (aceptada): el botín del día 3 va a los mapas del Ocaso.
4. **Copias de Finder « 2»**: `out/fabrica/UNI-02-11/copias_finder/` (`RESUMEN.txt` y los `.diff`). Di si alguna contiene trabajo perdido; si sí, propón el parche (no lo apliques si es C++).
5. **Imágenes**: `ojos_grande.png` → regenerar con `MG/tools/capa_ojos_iris.py`; `con.png` → renombrar (CON es reservado en Windows) y actualizar referencias; `Untitled-1.psd` → fuera del repo (solo dilo).

## Comprobación
`cd MG && python3 tools/validar_todo.py` antes y después (mismos errores o menos). Para 3: `python3 tools/validar_botin.py`.

## Entrega
`out/fabrica/UNI-02-11/fable/F-REST_RELEVO.md` en la copia de Oriol: por tarea, estado (hecha / descartada / bloqueada), commit, comando y resultado. Máximo 60 líneas.
