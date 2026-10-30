# SESIÓN 2026-09-03 — Codex — C-11: un solo formato de proyecto

- **Dónde:** `MotorGraphico-main/MotorGraphico/`, `TABLERO.md` y este registro.
- **Commits:** MotorGraphico `31bf68e` (8 archivos propios: herramientas, pruebas, módulo CMake, seguimiento y parche de integración).
- **Qué se hizo:** continuación solicitada por Oriol del trabajo sobre `planificacion/PLA_DEFINITIU.md`. Se releyó el tablero y se reclamó C-11 antes de crear archivos. C-12 ya tenía implementación y prueba de mutaciones ajenas en el workspace. Ante «tornaho a provar», se volvió a comprobar la reserva de C-20: la fila de zcode sobre GameSession/Application sigue presente; esas rutas no se tocaron.

## Terminado en el workspace y cómo

1. **Un único lector/escritor nativo:** `ProjectIndex` delega carga y creación a `ProjectSerializer`. El contrato es Project v1: claves inglesas, `schemaVersion: 1` y `settings` anidado. Los formatos antiguos o mixtos se rechazan con el comando de migración; los campos desconocidos tampoco se descartan silenciosamente.
2. **Migración real de los cinco manifiestos:** `boundington`, `este_norte`, `este_sur`, `mundo` y `oeste_norte`. Todos los metadatos y referencias de contenido conservan su valor. Solo se omite `_nota`, anotación editorial del contrato anterior. Copia previa de originales en `/var/folders/87/43f3pv591cq8y6r4px7fy4lc0000gn/T/motorgraphico-proyectos-v0-of_ryp7r`.
3. **Herramienta reproducible:** `tools/migrar_proyectos.py` previsualiza por defecto, valida el lote completo antes de escribir, crea backup y reemplaza cada archivo atómicamente con `--write`. Reejecutarla produce cero cambios. No acepta ids inseguros, versiones desconocidas, campos sin migración ni referencias duplicadas.
4. **Consumidores compatibles:** `build_proyecto.py` y `validar_todo.py` usan el mismo contrato Python. El índice y el validador resuelven entradas contra la raíz de assets indicada, incluidas copias de packs. El builder conserva el contrato independiente de `build.json`.
5. **Índice estable al editar:** crear/borrar conserva los ids de otros proyectos con manifiestos ausentes o inválidos; el índice vacío permite crear el primer proyecto. Los nombres con comillas, saltos y Unicode se guardan mediante el serializador.
6. **Contrato, esquema y plan:** actualizado `FORMATO_PROYECTOS.md`, el esquema Project v1 y `planificacion/seguimiento/C-11.json`. Regenerados `PLA_DEFINITIU.md`, `gantt_corte_b.puml` y `render/pla.html` desde el generador. El plan muestra acciones, evidencia y pendientes; C-11 permanece abierto hasta integración Git y CI.

## Evidencia

- La prueba nativa fallaba antes de migrar con «falta schemaVersion» en el primer manifiesto real. Después carga los cinco, compara bytes al serializar, compara el resultado de ProjectIndex y comprueba las entradas.
- `test_project_manifests` incluye creación/borrado, índice vacío, referencias inválidas conservadas, rechazo legacy/mixto/desconocido y entradas sobre una raíz temporal.
- `test_project_migration`: **6/6 casos Python**; preservación de campos, vista previa, idempotencia, validación de todo el lote, índice vacío y resolución de entradas.
- Build local de `suite_gl_free`: correcta. CTest completo: **56/56, 0 fallos**. Tras los ajustes finales de validación y rutas se recompilaron y ejecutaron las tres pruebas `test_project_*`: **3/3**. La cifra de la suite incluye trabajo previo y concurrente, sin atribuírselo a esta sesión.
- **Comparación independiente del empaquetador:** ejecutado el builder anterior de HEAD sobre los manifiestos originales y el nuevo sobre los migrados, con los mismos assets y salidas temporales. Los cinco `build.json` son exactamente iguales. Boundington y Este Norte son jugables; Este Sur, Mundo y Oeste Norte conservan sus carencias previas. La migración no arregla ni empeora ese contenido.
- `demo_proyectos` pasa en un sandbox temporal con assets copiados. `validar_proyectos`: 0 errores y 2 avisos previos por entrada vacía.
- Logs locales: `/tmp/motorgraphico-c11-ctest.log`, `/tmp/motorgraphico-c11-compatibility.log`, `/tmp/motorgraphico-c11-final-build.log`.

## Versionado y convivencia

La base Project/Map, el serializador, los tests previos y gran parte de CMake/planificación ya estaban sin versionar. La regla 2 de la BITACORA prohíbe commitear ese trabajo ajeno. Tampoco sería coherente commitear los manifiestos migrados sin su nuevo lector. Por eso los cambios coordinados quedan aplicados en el workspace y preservados como **diff propio sobre la base encontrada**, `planificacion/seguimiento/C-11-integracion.patch`, con instrucciones y SHA-256 en el archivo `.md` vecino.

El commit propio contiene las dos herramientas nuevas, las dos pruebas nuevas, su módulo CMake, el seguimiento y el parche. No incluye íntegra la arquitectura previa ni la migración de datos aislada. El include CMake propio también queda en el parche: otra sesión preparó simultáneamente el hunk C-22 en el índice compartido. Se preservó esa preparación y el commit se hizo por rutas propias bajo el mutex de git-safe, sin incorporar C-22. El parche se comprobó con `git-safe.sh apply --reverse --check` contra el workspace, sin aplicarlo de nuevo.

Copia local de la base de los archivos compartidos: `/var/folders/87/43f3pv591cq8y6r4px7fy4lc0000gn/T/codex-c11-baseline-gddkffwo`. Los duplicados macOS ajenos encontrados (`Check 2.h` y el SVG duplicado de planificación) no se añadieron ni se borraron.

## Pendiente explícito

- Versionar con su autor la arquitectura previa y después incorporar los cambios coordinados de C-11; retirar el parche una vez integrados los archivos normales.
- Ejecutar CI para cerrar formalmente C-11. No se ha ejecutado ni declarado CI remoto.
- C-20 continúa pendiente de conectar el aviso de daño al combate; GameSession/Application siguen reservados en TABLERO.

- **Deja esperando a Oriol:** nada nuevo; los pendientes técnicos quedan en el plan y el tablero.

- **Comprobación final:** migrador sobre los assets reales: 5 manifiestos validados, 0 cambios; `git diff --check` y `git diff --cached --check` limpios. Parche inverso comprobado sin modificar el workspace.
