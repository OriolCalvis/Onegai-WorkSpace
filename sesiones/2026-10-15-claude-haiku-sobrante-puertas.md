# Sesión Claude Haiku — AV14-24 (inventario sobrante) y AV14-20 (validador de puertas)

**Fecha:** 15/10/2026 · **Modelo:** Haiku, esfuerzo medio · **Lanzada por:** Claude Cowork
**Repo:** `MotorGraphico-main/MotorGraphico` · **Copia de trabajo:** `$HOME/ag/haiku/MG` (idéntica a la de Oriol al empezar; comprobado con diff -rq sobre tools, assets/levels, src y tests)

## Qué se hizo

1. **AV14-24 (solo lectura).** Ejecuté `tools/buscar_sobrante.py` (sin cambios). Salen 11 cabeceras sin referencias fuera de su familia (la tarea decía 13), y 8 grupos de cuerpos repetidos. Escribí `planificacion_alcance_y_vision/INVENTARIO_SOBRANTE.md`. No se cambió ningún fichero del motor.
2. **AV14-20.** Nuevos `tools/validar_puertas.py` y `tools/test_validar_puertas.py`. Llamada en `tools/validar_todo.py` como aviso. Paquete en `out/claude_puertas02/` (ver nota abajo).

## Pruebas

- `python3 tools/test_validar_puertas.py`: 18 pruebas, todas OK (una de ellas es una prueba de humo sobre el repo real).
- Mutantes: quitar la colisión de tiles, la comprobación de la puerta de vuelta o los objetos bloqueantes hace fallar 4, 1 y 1 pruebas respectivamente.
- Copia limpia de `MotorGraphico` en `$HOME/ag/haiku/prueba`: `aplicar.sh` aplica 3 ficheros, `validar_todo.py` da 0 errores y 22 avisos (10 de antes + 12 nuevos), y una segunda aplicación se niega (`YA EXISTE` / `CAMBIADO`).
- `validar_puertas.py` sobre los niveles reales: 116 niveles, 219 puertas, 12 avisos.

## Hallazgos

- El campo se llama `targetPosition` (no `targetSpawn`): así lo lee `LevelLoader.cpp`. Sin él, la llegada es el `playerStart` del destino.
- `tools/validar_enlaces.py` ya comprueba puertas, pero solo mira colisión de tiles, solo los niveles `ciudad_`/`interior_`/`plaza_` y no está en `validar_todo`. No ve los avisos de objetos bloqueantes. Conviene decidir si se retira o se unifica.
- `out/claude_puertas01/` ya tenía un paquete PUERTAS-01 de otro día (`aplicar.sh` y `puertas01.tgz`, ficheros de SaveGame). Al preparar mi paquete escribí en esa carpeta `nuevos.txt`, `preimagen.md5` y `postimagen.md5`, que pueden ser los de PUERTAS-01 (`out/` no está en git, no puedo recuperarlos). Los moví a `out/claude_puertas02/`. No he tocado `aplicar.sh` ni `puertas01.tgz`, pero el `aplicar.sh` de PUERTAS-01 ya no encuentra sus md5. Hay que decidir cómo recuperarlo.

## Ficheros que no toqué (por orden)

TABLERO.md, REVISAR.md, BANDEJA_PRODUCT_OWNER.md, dades.py, `Application.*`. No añadí fila en TABLERO: la orden de lanzamiento lo prohibía.
