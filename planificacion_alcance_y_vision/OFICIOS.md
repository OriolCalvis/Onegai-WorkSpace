# OFICIOS — datos de los sitios de oficio (D0017)

Carril `oficios_datos`, 15/10/2026. Paquete: `MotorGraphico/out/claude_oficios_datos01/`.
Solo datos: catálogo, tabla de pesca, textos y validador. No se ha tocado ningún nivel ni catálogo de objetos.

## Qué entra

- `assets/catalogs/oficios.json`: 15 oficios, contrato de D0017.
- `assets/catalogs/oficios_tablas.json`: la tabla de pesca (4 sitios, 3 franjas, 17 peces).
- `tools/validar_oficios.py` y `tools/test_validar_oficios.py`: 0 errores, 2 avisos; 29 pruebas en verde.
- `propuesta/objetos_peces.json`: 17 fichas de pez (no aplicadas).
- `propuesta/objetos_oficio_faltantes.json`: 3 fichas de sitios que faltan (no aplicadas).

Aplicar: `bash out/claude_oficios_datos01/aplicar.sh` (solo ficheros nuevos; se niega si ya existe alguno).

## Tabla por oficio

Tirada = característica + competencia (existen en `skills.json`) contra los dados del rival. Paga = oro / exp / reputación
en roce, éxito y crítico. «—» = sin tirada (rival 0: éxito automático). Todas las facciones son `aegroum`
(la del tendero del sitio, o la ciudad si no hay tendero).

| Tanda | Oficio | Sitio (`objectId` → nivel) | Tirada | Coste | Paga (roce · éxito · crítico) | Pifia |
|---|---|---|---|---|---|---|
| 1 | `servir_bar` | `barra_taberna_mig` → `interior_taberna` (3,3) | CAR persuasion vs 2 | 0 | 4/1/0 · 8/2/1 · 12/3/2 | −2 oro |
| 1 | `estudiar_biblioteca` | `mesa_lectura_mig` (4), `estanteria_libros_o3` (3) → `interior_biblioteca` | INT investigacion vs 2 | 3 oro | 0/2/0 · 0/4/0 · 0/6/1 | — |
| 1 | `pescar` | `ponton` → `ciudad_centro` (2), `ciudad_este` (2), `ciudad_surysal`, `prado_del_rio` | DES percepcion vs 2 | 0 (caña) | 0/1/0 · 0/2/0 · 0/4/1 + pez de la tabla | — |
| 2 | `banarse_termas` | `estatua_fuente` → `interior_banos` (sustituto) | — | 6 oro | 0 · efecto +6 PV, +3 fatiga quitada | — |
| 2 | `sudar_vapor` | `brasero_piedras` → `interior_banos_p1` | — | 3 oro | 0 · efecto +3 PV, +4 fatiga quitada | — |
| 2 | `sentarse` | `banco_plaza`, `banco_plaza_o2`, `banco_brasero`, `banco_brasero_o2`, `banco`, `banco_piedra`, `silla`, `silla_o1/2/3`, `taburete` | — | 0 | 0 · efecto +1 PV, +1 fatiga quitada | — |
| 2 | `servir_posada` | `mostrador_posada_mig` → `interior_posada` | CAR persuasion vs 2 | 0 | 4/1/0 · 7/2/1 · 11/3/2 | −2 oro |
| 2 | `servir_te` | `mesa_baja` (4) → `interior_casa_te` | CAR persuasion vs 1 | 0 | 3/1/0 · 6/2/1 · 9/3/2 | −1 oro |
| 2 | `estudiar_aula` | `pupitre_o2` (30) → `interior_universidad` | INT investigacion vs 3 | 6 oro | 0/3/0 · 0/5/1 · 0/8/2 | — |
| 3 | `forjar_daga` | `yunque` → `interior_herreria` (3,4) | CON atletismo vs 2 | 0 | 4/1/0 · 9/2/1 · 15/3/2 + daga | −3 oro |
| 3 | `forjar_espada` | `yunque` → `interior_herreria` | CON atletismo vs 3 | 0 | 8/2/0 · 16/3/1 · 26/5/2 + espada corta | −5 oro |
| 4 | `cuidar_huerto` | **falta** `bancal_huerto` | CON atletismo vs 2 | 0 (pala) | 5/1/0 · 10/2/1 · 16/3/2 | −2 oro |
| 5 | `luchar_coliseo` | `maestro_arena` → `interior_coliseo` (9,7) | CON atletismo vs 4 | 10 oro | 8/3/0 · 25/5/1 · 45/8/3 | −5 oro |
| 5 | `entrenar_muneco` | `muneco` → `interior_coliseo` (4), `interior_armeria`, `interior_cuartel` (2) | CON atletismo vs 2 | 0 | 0/2/0 · 0/3/0 · 0/5/1 | — |
| 6 | `montar_caballo` | **falta** `caballo_de_posta_obj` (montura `caballo_de_posta` de `mounts.json`) | CON atletismo vs 2 | 5 oro | 0/1/0 · 0/2/0 · 0/3/1 | — |

Notas de los números:

- Vara de medir: pan 8, cerveza 6, poción 30, daga 60, espada corta 120, caña de pescar 15. Actuar en la calle da 10–25 de oro.
  Servir da 8 de oro por turno; la pesca, unos 4–8 de oro por pez común vendido.
- El que forja cobra un porcentaje (7–25 %) del valor del arma; el arma entra en el stock del tendero (`objeto.tendero`: `aigren`,
  que ya vende `espada_corta` y `daga`). No hay mineral: el motor de forja puede añadir un coste en objetos más adelante.
- Estudiar y las termas no dan oro: cuestan oro y dan exp, PV o fatiga. La «carta de lore» o «subir una competencia» de D0017 no
  tiene ficha en ningún catálogo: lo decide el motor con el grado (ver dudas).
- `efecto.fatiga`: positivo quita fatiga, negativo la pone. `efecto.pv` negativo = daño (la arena: −3 PV, −2 fatiga).
- Los rangos de `oficios.json` son **una propuesta mía** (D0017 no los fija): aprendiz 0 exp (+0 dados), oficial 20 (+1), maestro 60 (+2),
  gran maestro 140 (+3). Con 2–4 exp por turno, oficial llega a los 6–8 turnos.

## Textos

Cada oficio lleva `aceptar`, `exito`, `fallo`, `critico` y `pifia`, de 90 caracteres como mucho (lo comprueba el validador).
Voz seca y corta, como Rosita y Aigren. Llevan nombres de Boundington: Rosita (Taberna Humilde), Aigren (Lingote de Oro),
la Arena Roja, las termas, la anfitriona (La Hoja Verde). En los oficios sin tirada el `fallo` y la `pifia` no salen nunca; están
puestos porque el contrato los pide.

## Pesca

Se pesca en cualquier `ponton`. Como el `objectId` es el mismo en seis niveles, el motor elige el sitio por (nivel, x, y), que
está en `oficios_tablas.json` (`pesca.sitios.*.donde`). Franjas: mañana, tarde, noche. Grado mínimo: común = roce,
raro = éxito, legendario = crítico. Venta = mitad del precio de la ficha (lo que paga el tendero).

| Sitio | Dónde | Comunes | Raros | Legendario (franja) |
|---|---|---|---|---|
| `muelle` El muelle del puerto | `ciudad_centro` (48,29) | sardina del puerto, lubina del muelle, boga plateada, anguila de fango | salmón de Aegroum (mañana), esturión del puerto (tarde y noche) | Esturión dorado, 150 (noche) |
| `ribera` La ribera de Boundington | `ciudad_centro` (16,31), `ciudad_este` (6,29) y (22,31) | gobio del río, boga plateada, carpa de acequia, anguila de fango | trucha de las Marcas (mañana), lucio viejo (tarde y noche) | Anguila del farol, 130 (noche) |
| `vado` El vado de Surysal | `ciudad_surysal` (30,20) | tenca del vado, gobio del río, cangrejo de río, anguila de fango | trucha de las Marcas (mañana y tarde), pez luna del vado (noche) | Carpa coronada, 110 (mañana) |
| `prado` El prado del río | `prado_del_rio` (14,10) | gobio del río, cangrejo de río, carpa de acequia, anguila de fango | trucha de las Marcas (mañana), lucio viejo (tarde y noche) | Siluro del prado, 120 (noche) |

Precios de ficha / venta: comunes 8–16 / 4–8; raros 40–70 / 20–35; legendarios 220–300 / 110–150.
Los peces **no existen** como objetos: sus fichas están en `propuesta/objetos_peces.json` (mismo formato que `pan`).
El validador comprueba que cada pez exista como objeto o tenga ficha propuesta con el mismo nombre y precio.

## Objetos que faltan en los niveles (no puestos)

| Falta | Para | Dónde tendría sentido | Hoy |
|---|---|---|---|
| `tina_banos` (tina de las termas) | `banarse_termas` | `interior_banos` / `interior_banos_p1` | Se usa `estatua_fuente` (Fuente de la ninfa) como sustituto |
| `bancal_huerto` | `cuidar_huerto` (tanda 4) | `ciudad_este` (entre los frutales) o un rincón de `ciudad_oeste` | No hay huerto en ningún nivel; `jardinera` y `mata` son decorado |
| `caballo_de_posta_obj` (animal interactuable) | `montar_caballo` (tanda 6) | junto a la Taberna Rica o en `ciudad_este` | Los `mascota_*` no son interactuables (los mueve `VidaNivel`). Hay 50 monturas en `mounts.json`, ninguna como objeto |
| Mesa o tablón de inscripción (opcional) | `luchar_coliseo` | `interior_coliseo` | Sirve `maestro_arena` (PNJ con tienda) |
| Fichas de los 17 peces | `pescar` | catálogo de objetos | `propuesta/objetos_peces.json` |
| Mineral/lingote (opcional) | forja con coste en objetos | catálogo de objetos | No existe |

Sitios con poca cosa pero que sirven: `barra_taberna_mig` (1 solo, a propósito: la barra tiene tres tramos con jarras y
botellas; si los tres fueran sitio se podría servir tres veces seguidas). `silla_noble*` del castillo se deja fuera de `sentarse`.

## Avisos para quien haga el motor

1. **El descanso hoy es por `objectId`** (`Actuacion.cpp`: `claveDelSitio(sitio.id)`). Con `ponton` en seis niveles, `banco_plaza`
   o `silla` en muchos, pescar o sentarse en uno bloquearía todos. El descanso de oficios tiene que ir por (nivel, x, y).
2. **Varios oficios en un mismo sitio**: `yunque` (daga y espada), `maestro_arena` (luchar y, si se quiere, entrenar), `mesa_lectura_mig`
   y `estanteria_libros_o3`. La hoja «Trabajar» tiene que listar los oficios del sitio y dejar elegir.
3. **Un sitio con muchas casillas** (`pupitre_o2`, 30; `silla`, 12 en la biblioteca): una sola ficha de objeto vale para todas.
4. Faltan en `ObjectCatalog.h` el campo `oficio` y su lectura; hoy solo existe `actuacion`.
5. `fatiga`: no he encontrado ningún sistema que la lleve. El catálogo la nombra porque D0017 la nombra.

## Dudas para Oriol

1. **Rangos**: ¿vale 20 / 60 / 140 de exp para oficial, maestro y gran maestro, con +1 / +2 / +3 dados de oficio?
2. **Estudiar**: ¿qué da exactamente, una competencia que sube, una carta de lore o solo experiencia? Hoy solo exp + algo de reputación.
3. **Bañarse y sentarse sin tirada** (éxito automático): ¿bien, o prefieres una tirada también ahí?
4. **¿Un oficio por sitio o varios?** La herrería ofrece daga y espada; ¿quieres elegir cada vez?
5. **Bancal y caballo**: ¿dónde quieres el huerto y el establo? Hace falta decidir los niveles antes de la tanda 4 y la 6.
6. **El legendario**: ¿siempre con el mismo sitio y la misma franja (como está), o con una probabilidad baja en cualquiera?
