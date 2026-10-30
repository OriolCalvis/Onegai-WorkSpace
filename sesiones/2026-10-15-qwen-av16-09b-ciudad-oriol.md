# 2026-10-15 · qwen · AV16-09b la ciudad que dibujó Oriol en Excel

Cola del orquestador (`orquestador/qwen.md`, tarea **1b**, que manda sobre los puntos de la tarea 1). Rama
`agente/qwen/AV16-09b` (sobre `agente/qwen/AV19-06`), worktree `~/onegai-ramas/qwen-AV16-09b`. Commits `96cbb0f`
(la ciudad) y `dfd2ffe` (el contrato de Codex para la puerta de pesca).

## La discrepancia de partida (registrada, no resuelta en silencio)

El Excel de Oriol (`planificacion_alcance_y_vision/mapas/ciudad_centro_oriol_15-10.xlsx`, 20:35) está dibujado sobre
la ciudad **anterior a AV16-09**. Prueba: las 19 coordenadas de la columna «antes» de la ficha coinciden una a una
con `ciudad_centro.json` en `5780418`, y ninguna con la ciudad que entregué a las 19:19 (`puerta_iglesia` (47,52)
vs (42,50), `puerta_herreria` (6,46) vs (6,58), `fuente_viento` (32,33) vs (34,33)…). Claude generó ese Excel a las
10:48, nueve horas antes.

Consecuencia: se aplica la **intención** (puertas a la calle, edificios girados, menos casas, muralla abierta, fuente
fuera de la plaza, camino a Egaroth al sur) y las coordenadas sueltas de su lista no son reproducibles 1:1 sin
deshacer AV16-09, que él ya dio por buena.

## Qué se hizo

- **Muralla abierta donde llega una calle**: 6 huecos al oeste y 6 al este, con su hueco en `ciudad_oeste` /
  `ciudad_este` en las mismas filas y su paso en los dos sentidos (26 pasos). Tabla `HUECOS_MURALLA` en
  `gen_ciudad.py`; `unir_barrios.py` planta los pasos, comprueba que el hueco esté abierto (si no, aborta con el
  motivo) y reescribe `boundington_pasos.json` con las fichas (ya era suyo). Ids propios por hueco
  (`paso_centro_a_oeste_casco`…); la banda del río conserva los ids de siempre.
- **`ciudad_oeste` gana la calle del muro este** (x 29-30, filas 1-62): sin ella los huecos del sur caían entre
  chabolas sin salida. En `ciudad_este` no hizo falta (x=1 ya es transitable de punta a punta).
- **Puertas de la muralla**: norte a 3 casillas (31-33) y sur a 4 (30-33), el ancho de la avenida.
- **Castillo una fila más alto**: puerta en (8,12), sobre el carril de la avenida. La avenida se pinta antes que los
  monumentos, o repinta la fila de la puerta. Banderas a (4,13) y (12,13) para dejar tres casillas de acera libre.
- **Casco con menos casas**: 24 casitas de relleno (31 en AV16-09) y dos calles nuevas de 2 casillas (filas 53-54,
  oeste y este).
- **Fuente** fuera de la plaza del puente: a la placita del Comercial, (50,16).
- **PO-48**: `puerta_ropa_sur` → `interior_pesca_caza.json`, `targetPosition (1,7)` (contrato de Codex). Las otras
  cuatro `_sur` mantienen destino.
- **Sectario** de (27,40) a (19,43), y **mercader** de (8,13) a (16,13) porque era la única casilla delantera de la
  puerta nueva del castillo. Mecanismo nuevo `recolocar_celdas` (por casilla, no por id, declarado en el código):
  los ids con varias instancias se juntarían todos en la misma celda.
- **Regresión mía deshecha**: `camino_a_egaroth` vuelve a (32,62) y la llegada desde el mapamundi también. La había
  movido a (31,31) (sobre el puente) al ejecutar `enganchar_boundington.py` en la cadena de AV16-09: esa herramienta
  aparca la salida en la celda libre más cercana a `playerStart` (comportamiento cerrado en REV-05).
- `tools/comparar_excel_ciudad.py` (nueva, sin `openpyxl`: `zipfile`+`ElementTree`): compara la ciudad generada con
  el Excel de Oriol por suelo (color) y objetos (letra+comentario). Se autovalida: contra el Excel original de
  `out/claude_excel01` da **0 diferencias**.

## Diferencias que quedan contra el Excel de Oriol (y por qué)

**Coincide exactamente:** la muralla de los cuatro bordes (0 diferencias: los 18 huecos del oeste, los 17 del este,
los 3 del sur y los 2 del norte), `puerta_castillo` (8,12), `puerta_opera`/`puerta_universidad`/`puerta_biblioteca`
(fila 11), `puerta_banco_sur` (6,39) y el sectario en (19,43).

**Puertas: misma intención, distinta casilla** (su «después» es la casilla pegada a SU calle; la mía a la mía, porque
AV16-09 movió la retícula). Todas tienen la puerta en la fachada sur, sobre umbral, con calle de 2 o más delante:

| puerta | Excel | ciudad nueva | por qué |
|---|---|---|---|
| `puerta_joyeria` / `banco` / `sastreria` / `mercado` | fila 24 | fila 25 | la calle del Comercial está en 26-28 |
| `puerta_mercado_sur` | (16,39) | (19,39) | el local se movió al este en AV16-09 |
| `puerta_posada_sur` / `posada` | (26,40) / (48,40) | (48,39) / (56,39) | mismas: movidas y una fila más altas |
| `puerta_banos_sur` | (56,39) | (14,39) | las termas del sur se movieron al oeste en AV16-09 |
| `puerta_coliseo` | (39,42) | (39,39) | la calle del sur del Casco está en 40-42 |
| `puerta_herreria` | (6,50) | (6,58) | girada al sur como pidió; la calle de la herrería es 59-62 |
| `puerta_iglesia` | (47,53) | (42,50) | **conflicto**: mira a **oeste**, a la plaza, con paso recto (PO-44, su propia respuesta). Su Excel la mantiene mirando al sur |
| `puerta_ropa_sur` / `taberna` / `banos` | fila 57 / 60 / 61 | fila 58 | la calle de la herrería está en 59-62 |

**Objetos:**
- `camino_a_egaroth`: en su Excel la letra está borrada. Lo mantengo en (32,62): sin él el mapamundi se queda sin
  vuelta (`validar_enlaces` daría «NO tiene vuelta») y él mismo dice que el camino sigue.
- `fuente_viento`: (50,16) en vez de (28,16). En (28,16) hay una casa de la manzana del Comercial en el trazado
  nuevo; usé su alternativa («otro sitio del barrio norte que no corte el paso»). Queda pendiente `actuacion_fuente`
  («junto a la fuente», `tools/actuaciones.py:36`), que sigue en (30,33).
- Los pasos de la banda del muelle: él marcó la letra en (0,25), (0,28) y (63,28); yo pongo **uno por banda** en la
  fila central (27), porque cada banda es un hueco y cada hueco lleva un paso en los dos sentidos a la misma fila.
  Si los quiere en 25 y 28, es cambiar la fila en `HUECOS_MURALLA` y en `ENLACES`.
- El `anciana` de (6,51) que él borra: no lo borro (hay 4 instancias y las aventuras las buscan por id); además
  `despejar_calles` ya la aparta a (6,52) porque (6,51) es calle.
- `puerta_tienda_abandonada` y `puerta_cerrada_tienda` no están en su Excel: son de AV19-05 (20:00), posterior.
- 79 objetos «movidos» y ~1.200 casillas de suelo distintas: es la divergencia AV16-09 ↔ Excel viejo, no deseos
  suyos sin aplicar. Comparación manzanas con manzanas (árbol principal vs su Excel): 149 casillas y 36 objetos.

## Verificación

- `comparar_excel_ciudad.py --nivel ciudad_centro` → «Muralla (0): la muralla de los cuatro bordes coincide».
- `medir_calles.py ciudad_centro --comprobar --sin-voraz` → `COMPROBAR: bien`, `<25%: 19/19` desde los dos orígenes
  (desvío total 4.0 % y 1.2 %).
- `despejar_calles.py --puertas` bien en las tres ciudades; `--carriles` bien (353 casillas).
- `conectividad.py` EXIT=0: centro 2719/2719, este 1574/1574, oeste 1421/1421, ocaso y surysal 100 %.
- `validar_enlaces.py` → 188 puertas, **1 problema** (el nivel de pesca, que vive en la rama de Codex).
- Build EXIT=0; `ctest -L gl-free` → 7 rojos: 4 los de HEAD y **3 del mismo enlace** (`test_lugares`,
  `test_world_travel_state`, `demo_mundo`).
- `juego --nivel … 3` carga y vuelca frame en ciudad_centro, ciudad_este, ciudad_oeste y ciudad_centro_ocaso.
- El ocaso, regenerado sobre la ciudad nueva, pasa sus tres guardas: 2532/2532 casillas alcanzables, 0 selladas,
  0 solapes, 7 cortes como barrera completa y las 6 rondas limpias.

## Coordinación

- **Las ramas `agente/qwen/AV16-09b` y `agente/codex/AV16-08` se necesitan**: la mía deja la puerta apuntando a un
  nivel que está en la suya, y la suya deja un local sin puerta que lo abra. Juntas cierran el enlace y los 3 rojos
  desaparecen (Codex lo probó en una copia limpia: `validar_todo` 0 errores). Su salida del interior apunta a
  (56,58), que es justo la celda de mi puerta, y es el convenio de `reapuntar_interiores`.
- Posible conflicto de fusión en `assets/proyectos/boundington.json` (las dos ramas añaden niveles al final de la
  lista) y en `assets/catalogs/lugares.json` (su ficha del local nuevo).

## Pendiente / para decidir

- **Cadena oficial**: `regenerar_ciudad.sh` termina con `enganchar_boundington.py`, que aparca la salida a Egaroth
  junto a `playerStart` (REV-05) y por tanto **sobre el puente**, mientras el 1b la pide al sur. Mi cadena es
  `gen_ciudad.py → gen_carteles.py → unir_barrios.py → gen_niveles_av19.py` y no lo ejecuta. Sigue sin estar
  decidido qué hace la cadena oficial (ya lo avisé en AV16-09: sus pasos 3 y 5 pisan interiores hechos a mano y
  rompen beats de Z.AI).
- **`conectividad.py` solo mira la colisión de tiles**, no los objetos que bloquean: da 100 % con una casilla sellada
  por un mueble, un PNJ o el fuego. No lo he tocado (herramienta común); la comprobación con objetos vive en mis
  generadores (`av19_niveles.py`). Candidato a decisión.
- `actuacion_fuente` debería seguir a la fuente a (50,16) o cambiarse de texto: es de `tools/actuaciones.py`.
