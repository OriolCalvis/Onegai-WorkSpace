# Sesión 2026-10-15 — Claude (Sonnet), carril `pnj_datos`

Ronda del 15/10 (lanza e integra Claude Cowork). Encargo: qué hace cada PNJ de Boundington, como datos para el motor que programa `pnj_motor`.

## Hecho
- Inventario de PNJ: recorrido de los 116 `assets/levels/*.json` contra los objetos de categoría `npc` (como `ObjectCatalog.cpp`): **108 tipos, 218 PNJ** (79 en la ciudad, 29 en sus interiores, 27 en recuerdos/prólogo, 48 en Este-Norte, 12 en la plaza de las razas, 11 en Origen Norte, 10 en Surysal, 2 de ejemplo).
- `assets/catalogs/rutinas_pnj.json` (nuevo): contrato de `orquestador/ronda_15-10.md` al pie de la letra. Clave exacta para los 108 tipos colocados + `npc_race_*`, `npc_npc_*`, `ciervo`, `herrero`, `tabernero`. Los PNJ con beat `talk` van quieto/vigilar/radio 0-1. Iconos solo de `iconos.json`; frases ≤ 40 caracteres.
- `tools/validar_rutinas_pnj.py` (nuevo) + `tools/test_validar_rutinas_pnj.py` (49 casos, pasan). Sobre los niveles de verdad: 0 errores, 0 avisos.
- Informe `planificacion_alcance_y_vision/RUTINAS_PNJ.md`: tabla por tipo, animaciones para Pixel, dudas para Oriol.
- Paquete `out/claude_pnj_datos01/` (`aplicar.sh`, `nuevos.txt`, `pnj_datos01.tgz`): 3 ficheros nuevos, se niega si alguno existe. Probado sobre una copia limpia: aplica, pasa pruebas y validador, y la segunda vez se niega.

## Incidencias
- `device_bash` dejó de responder a mitad de la sesión (fallo repetido del espacio de trabajo). El resto se hizo en el contenedor con una copia (`device_stage_files`) de niveles, catálogos de objetos, aventuras e iconos, y se devolvió con `device_commit_files`. La copia `/tmp/ag/pnj_datos/MG` de la máquina de Oriol no recibió los ficheros nuevos; la prueba de `aplicar.sh` se hizo en el contenedor, no en la máquina de Oriol. `aplicar.sh` puede haber perdido el bit de ejecución al copiarse: se lanza con `bash`.
- No se tocó nada fuera de las tres rutas permitidas. Sin commits.
