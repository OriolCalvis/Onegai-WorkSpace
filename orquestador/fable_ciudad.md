# Encargo D — La ciudad reajustada, integrada (Fable, 16/10/2026)

Rama **`agente/fable/ciudad`** (sale de `claude/AV18-cierre`), tres commits: `beace32` (fusión de la cadena de Qwen), `3c14eec` (botín sobre la ciudad nueva, Zurrón del Vado, fuente, PNJ) y `3e5281c` (lugares). Sin push, sin tocar `main` ni las ramas de Qwen. **Nada de C++.** Paquete en `MotorGraphico/out/fable_ciudad01/` (planos antes/después, Excel de la ciudad integrada, `aplicar.sh`).

## D0 — Diagnóstico
La tabla del encargo es correcta: las siete ramas de Qwen cuelgan de `agente/qwen/AV19-03` (10 commits, `444ceab`…`9d8d177`) y ninguna era ancestro de `claude/AV18-cierre`. `git merge --no-commit agente/qwen/AV19-03` dio **6 conflictos reales**, no 13 (los otros 7 ficheros se fusionaron solos):

| Fichero | Quién lo cambia | Cómo se resolvió |
|---|---|---|
| `assets/levels/ciudad_centro.json`, `ciudad_este.json`, `ciudad_surysal.json` | Qwen (geometría nueva) / `664b160` (cofres nativos) | **geometría de Qwen**; el botín se vuelve a repartir encima |
| `assets/levels/interior_iglesia_s1.json` | Qwen (`loot` estático) / `664b160` (cofres nativos) | Qwen, y luego `repartir_botin.py` quita las etiquetas viejas y pone las nuevas |
| `assets/proyectos/boundington.json` | Qwen (6 niveles nuevos, forma canónica) / `664b160` (`oficios_objetos.json`) | el de Qwen + `oficios_objetos`, `botin_boundington`, `libreria_armas`, `pesca_caza` e `interior_pesca_caza` (Qwen había partido de un manifiesto anterior a AV18-03) |
| `tools/repartir_botin.py` | los dos (el de Qwen escribía `loot`) | **el nuestro** (contrato nativo) |

Lo que no estaba en el encargo y hacía falta: **`agente/codex/AV16-08`** (el local de pesca y caza al que apunta `puerta_ropa_sur`). Fusionarla entera choca con `iconos.png` (binario), `LugaresTests.cpp` (C++) y el manifiesto, así que se trajeron **solo** el nivel, su TMX, su catálogo y su hoja de PNJ (que `gen_pnj.py` hornea después en `pnj_boundington`). La fusión completa de AV16-08 queda para quien integre, con esa nota.

## D1 — Integrado
- Geometría de Qwen tal cual (ciudad con amplitud + tu Excel del 15/10, muralla abierta, afueras, tienda abandonada, mansión, Ocaso, montaña). No hizo falta regenerar: los JSON y TMX de Qwen son los generados; `despejar_calles --puertas/--carriles` y `medir_calles --comprobar` confirman que están limpios.
- `repartir_botin.py` sobre la ciudad nueva: **47 instancias**, ciudad / interiores / cripta / **afueras** al 100 % del presupuesto en los tres días (variantes nuevas `botin_afueras_*`). La **cueva** sigue sin niveles (AV19-04). Los 17 lotes estáticos de Qwen (AV18-05) se descartan; `prado_del_rio.json` limpiado de sus etiquetas.
- Pendientes de Qwen cerrados: `actuacion_fuente` pasa de (30,33) a **(52,16)**, pegada a la fuente de (50,16) y fuera del carril (`tools/actuaciones.py` + regenerado). `camino_a_egaroth` está en (32,62), al sur, como pedía 1b (no se ejecutó `enganchar_boundington.py`). PNJ horneados de nuevo (`gen_pnj.py`: 270 PNJ, 344 aspectos) porque las colocaciones cambiaron.

## D2 — Lo que pasa y lo que no
| Comprobación | Resultado |
|---|---|
| `medir_calles.py ciudad_centro --comprobar --sin-voraz` | **19/19 puertas < 25 %** desde el inicio (desvío total 4,0 %; peor: `puerta_mercado_sur` 20 %, `puerta_banos_sur` 16 %) y 19/19 a 0 % desde la plaza del Casco |
| `despejar_calles.py --puertas` / `--carriles` | bien (todas las puertas miran a una calle de ≥ 2 con la casilla de delante libre; 353 casillas de carril) |
| `conectividad.py` | 100 %, 26 puertas alcanzables (afueras, tienda, mansión incluidas) |
| `validar_puertas.py` | 12 avisos, **ninguno de la ciudad** (mundi_landmass, ejemplos, recuerdo_taberna) |
| `validar_enlaces.py` | 0 problemas |
| `validar_botin.py` | 47 instancias, 0 errores |
| `validar_pnj.py` | 0 problemas tras `gen_pnj.py` |
| `validar_todo.py` | 1 error **heredado** (`recompensas_escenas.json`: `b2_aigren_orden_entrega` paga una daga fuera del libro, AV18-08) |
| `validar_lugares.py` | 4 errores «local repetido en puertas» (banco, mercado, baños, posada): es lo que **PO-48 decidió** (las puertas duplicadas llevan al mismo local). El validador va por detrás de PO-48; no es de esta rama |
| `simular_economia.py` | 47 cofres; 8 de 9 márgenes dentro. `explora_todo` día 3 sale a 84 (margen 40–80): el botín de las afueras ya cuenta. Se recalibra al final, cuando entre la cueva; **no** se tocan los márgenes dos veces |
| C++ gl-free (compilado en el contenedor con estos datos) | `test_project_levels`, `test_contenedores` 18/18, `test_world_travel_state`, `test_rutinas_pnj`, `test_mascotes`, `test_object_and_loot_catalog`, `test_catalog_contract`: verdes. `test_lugares` **7/8**: el que falla fija `todos().size() == 17` y ahora hay 18 lugares (el Zurrón); AV16-08 trae ese test actualizado, se cierra al fusionarla |

## D3 — El reajuste: lo que se ve y lo que propongo
Excel nuevo de la ciudad integrada: `out/fable_ciudad01/ciudad_centro_integrada.xlsx` (hoja «Mapa», con `excel_a_nivel.py` se devuelve al juego). Planos: `ciudad_centro_despues.png` y los cinco sitios `antes_*` / `despues_*` (plaza, comercial, casco sur, muelles, puente; los «antes» son los que Qwen dejó de la ciudad de antes de AV16-09).

**Primero, por qué «sigue sin reajustarse» aunque esté reajustada.** `comparar_excel_ciudad.py` contra tu Excel del 15/10 da **1.208 casillas de suelo distintas y 246 objetos** distintos. Tu Excel está dibujado sobre la ciudad anterior a AV16-09 y Qwen aplicó la *intención* (girar edificios, menos casas, fuente fuera, muralla abierta, castillo en (8,12)), no el dibujo. Las puertas y pasos coinciden con tu Excel en lo que importa (castillo, ópera, universidad, biblioteca, joyería, banco, muralla: 6+6 huecos), pero **el suelo no es tu suelo**. Si lo que quieres es la ciudad casilla a casilla como la dibujaste, el camino es otro: `excel_a_nivel.py tu.xlsx --nivel ciudad_centro` sobre **este** Excel nuevo (que ya tiene la geometría de Qwen), no sobre el del 15/10. Eso es la primera pregunta de abajo.

**Por barrio, con el criterio de AV16-09:**

| Barrio | Qué está bien | Qué sigue apretado o escondido | Propuesta |
|---|---|---|---|
| **Norte (castillo, ópera, universidad, biblioteca)** | Calle principal de 3; las cuatro puertas a 0 % de desvío; puerta del castillo en (8,12) | Los cuatro edificios ocupan 4 bloques de 12×7 sin nada entre ellos: el barrio es un pasillo. `actuacion_opera` (25,12) queda a 3 casillas de la puerta de la ópera (22,11), bien | Nada estructural. Dos bancos y una farola a los lados de cada puerta grande (no delante), para que el pasillo no parezca vacío |
| **Comercial (fila 14–20)** | Carriles en las filas 14 y 20; `fuente_viento` en (50,16) con su sitio de actuar al lado; puertas del banco (19,24) y joyería (7,24) a la calle | `parada_mercado` ×3 y `jardinera` ×2 dejan la calle en **una** casilla en varios puntos (lista de `despejar_calles`); la placita de la fuente queda pegada al carril de la fila 14 | Mover las 3 paradas a la fila 16 (fuera del carril) y las jardineras a los flancos de la fuente |
| **Muelles y puente (filas 27–33)** | El puente (31–32, 27–33) está despejado; `camino_a_egaroth` al sur; pasos a este y oeste en las filas 27/28 | `bolardo` y `bandera_ciudad` estrechan el muelle a 1 casilla junto al puente | Bolardo al borde del agua (fila 26/34), bandera a un lado del puente |
| **Casco Antiguo (plaza y sur)** | Plaza de 14×12 despejada; iglesia a 12 pasos y 0 % desde la plaza (antes 44 % → AV16-09 lo arregló); 24 casitas de relleno (antes ~100); `puerta_iglesia` a 28 del inicio | Tres puertas con desvío desde el inicio: `mercado_sur` 20 %, `banos_sur` 16 %, `coliseo` 14 % — todas por **rodear la plaza**, no por objetos. Dentro de la plaza hay una docena de objetos sueltos (PNJ y bancos) que la hacen parecer un patio | Un carril diagonal que cruce la plaza de (24,43) a (40,55) baja los tres desvíos a < 10 %; los bancos a los bordes de la plaza |
| **Oeste y este (`ciudad_oeste`, `ciudad_este`)** | Pasos en los dos sentidos en las filas 13, 19, 27, 49 y 61 por cada lado; conectividad 100 % | Tu Excel marcaba **seis** huecos por lado (con dos en el muelle, filas 25 y 28, a tres casillas); Qwen dejó **cinco** y uno solo en el muelle (fila 27). Es la única diferencia de pasos que `comparar_excel` señala | Si quieres los seis, el sexto va en la fila 33 (a la altura del embarcadero), no pegado al de la 27 |
| **Qué se ve desde la calle** | `despejar_calles --puertas`: ninguna puerta tapada; los carteles de negocio (`cartel_*`) están en la casilla de al lado de su puerta | Lo que estrecha calles son sobre todo **PNJ** (guardianes ×2, niño ×2, anciana ×2, estudiante, guardia, medianos): se mueven, pero paran en el carril. `arbol_roble` y `arbol_seco` estrechan dos calles del Casco | Que las rutinas de esos PNJ (`rutinas_pnj.json`) tengan `mira: calle` y radio 1; y los dos árboles, una casilla hacia dentro de la manzana |

Nada de esto se ha aplicado: son propuestas sobre el Excel nuevo. Se hacen en el generador (`gen_ciudad.py`) o por Excel (`excel_a_nivel.py`), una sola vez, cuando digas cuáles.

## Añadido (20:50): los pasos entre zonas, a todo el hueco
Oriol: «hay que expandir las casillas que te permiten ir a otra zona». Cada paso era una casilla y el hueco de la muralla tiene tres (el del muelle oeste, cuatro; las salidas de las afueras y la montaña, cinco). `tools/ensanchar_pasos.py` (nuevo, idempotente) clona cada salida de zona a zona a todas las casillas abiertas del hueco, hasta 5 contiguas, en los dos sentidos y con el destino desplazado igual (si en el otro lado esa casilla también está abierta; si no, se llega a la de siempre). No toca las puertas de los interiores (la salida de una tienda es su puerta). Resultado: **82 casillas de paso más** (12 huecos de muralla × 2 sentidos, la surysal↔oeste, las afueras, la montaña, la plaza de las razas); el botín se volvió a repartir para que ningún cofre quede en un acceso (46 instancias). `validar_puertas`, `validar_enlaces`, `conectividad`, `despejar_calles --puertas/--carriles` y `medir_calles --comprobar`: igual que antes. Commit en `agente/fable/ciudad`. Cuando se regenere la ciudad: `gen_ciudad.py` → `unir_barrios.py` → **`ensanchar_pasos.py`** → `repartir_botin.py`.

## Preguntas para Oriol (con respuesta por defecto)
1. **¿Ciudad «por intención» (la de ahora) o casilla a casilla como tu dibujo?** Si es casilla a casilla, dibuja sobre `out/fable_ciudad01/ciudad_centro_integrada.xlsx` (ya tiene la ciudad nueva) y yo la devuelvo con `excel_a_nivel.py`; el del 15/10 ya no sirve de base. Por defecto: la de ahora, con las seis propuestas de la tabla que apruebes.
2. **¿`agente/fable/ciudad` va a `claude/AV18-cierre`?** Comandos en `out/fable_ciudad01/aplicar.sh`. Por defecto: sí, cuando hayas probado el build de Windows con esta rama.
3. Para quien integre después: fusionar **`agente/codex/AV16-08` entera** (iconos, `LugaresTests.cpp` a 18, `gen_interiores`) y pasar el validador de lugares por PO-48.

## Lo que no se tocó
C++, el formato de niveles, el sistema de botín, la cueva (AV19-04), el lore y los nombres (PO-48).
