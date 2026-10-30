# Encargo C aplicado — C1, C2, C3 y las cinemáticas que ya se podían hacer (Fable, 16/10/2026)

Rama **`agente/fable/ciudad`**, dos commits sobre la ciudad integrada: `a2c3797` (C1 + C2 + C3, JSON de aventura) y `2606ff1` (C4 / AV17-06..08, cinemáticas). Sin push, sin tocar `main` ni las ramas de Qwen. **Ningún cambio de C++.** Oriol (16/10): «a medias, la cueva 100 % sí, lo de trepar, dormir y sentarse te lo compro; termina todo lo que puedas» → la cueva (AV19-04) es de Qwen, las órdenes de motor son de Claude, y aquí está todo lo que era JSON.

Todo lo de los dos commits se reproduce con dos herramientas idempotentes: `tools/parche_c_trama.py` y `tools/parche_c_cinematicas.py` (si alguien regenera las aventuras, se vuelven a pasar y queda igual).

## Lo que cambia para el jugador

### C1 — Xila (X-1..X-5)
- **Ruta del trago** (`b1_xila_habla`): ahora deja a Xila como **aliado débil** (`bnd1_aliado_xila_debil`) y borracho; antes daba la pista y nada más, y el día 2 contaba con él.
- **`b2_xila_resaca`** (día 2, hablarle con `bnd1_xila_borracho` y sin aliado): Persuasión con tres salidas — convencerle, **pagarle un desayuno caliente (6)**, o dejarle dormir. Si sale bien, `bnd1_aliado_xila` + `bnd1_xila_honor` y deja de estar borracho (`xila_sin_alcohol`). Si lo dejas, `bnd2_xila_resaca_cerrada` y la fiesta sigue sin él.
- **Tienda abandonada sin Xila**: `b2_tienda_abandonada` ya no exige haberle visto; con el encargo de Skilla basta.
- **Disfraces** (X-3/X-4): la Investigación de la tienda tiene parcial (traje de actor + bolas de malabares), pifia (herida 1d4 y segunda Investigación) y segunda salida; y **Luisarda vende** el disfraz de músico (40) o se le roba a la Boutique con Sigilo (`b2_boutique_robo_*`, con pagar los 40 como salida del fallo).
- **Cierre sin Xila**: la fiesta y el Ocaso se completan con y sin él (el Ocaso ya tenía `b3_xila_vendas` / `b3_xila_fiesta` por bandera; lo que faltaba era que las banderas se pusieran).

### C2 — Las tiradas (46 hoy, eran 42)
Terceros caminos nuevos: gitanos (`b2_gitanos_recelo`: acarrear agua una franja; `b2_gitanos_puerta_cerrada`: grano 10 + tablas), secta (`b1_sectarios_seguir`: seguir al sectario perdido con Sigilo → `bnd_pista_sectarios`), tienda (arriba), Gorran (`b2_gorran_sigilo_casi` / `_torpe` antes del combate sin matar). Cruce de banderas: **ninguna tirada que abre trama se queda sin tercer camino**; la única cuyo fallo no lee nadie *directamente* es `b1_secta_s2` (Persuasión «Mostrar interés»), porque su tercer camino cuelga de `bnd1_sectarios_resueltos`, que se pone igual con éxito y con fallo.

### C3 — Lo que Oriol aprobó del informe
- «**Irte**» de la mansión / dentro / invitados avisa («esta noche no podrás volver sin pagar») y **se puede volver** por Skilla pagando 10 al portero (`b2_fiesta_vuelta`).
- **Cueva sin encargo** (`b2_cueva_sin_encargo`): entrar en la mazmorra sin que Skilla te haya mandado ya no deja el beat mudo.
- «Privadísima» fuera; el Gorran de la fiesta dice lo que cuesta («Acabar con él (no volverá a ayudarte)») y los papeles ya no se nombran dos veces.
- **Día 3 sin Gorran**: `b3_gorran_sin_jefe` (3 bandidos sin jefe), `b3_bandidos_vencidos` (+20 oro), `b3_bandidos_otros`.
- **Verina**, si la abandonaste y Sandes está abierto: `b3_verina_segunda` (2 cultistas carceleros + 1 devoto).
- **Venides lee la confianza**: de noche, si desconfía (`bnd1_venides_desconfianza`), la opción de contárselo a medias se cierra y aparece «Contárselo todo y enseñarle las pruebas» (con documentos o los de la fiesta).
- **Epílogo** antes de la barca: `b3_epi_compania` / `b3_epi_solo`; la última opción se llama «Cerrar los ojos (fin de la aventura)».
- **Corrección al informe C3**: el hallazgo #1 («la rama del traje roto es inalcanzable») era un **falso positivo**: `b2_trepa_roto` pone `bnd2_roto_*` con una `tablaAleatoria`, que mi cruce no leía. `tools/analisis_trama.py` ya la lee (y ahora también las banderas que encienden las cinemáticas). Los otros 19 hallazgos se mantienen.

### C4 / AV17-06..08 — Cinco cinemáticas con las órdenes que ya hay
Claude ya había metido en el motor (`763e2ad`, AV17-10) `patrulla`, `trepa`/`baja`, `arde`/`apaga`, `tumba`/`levanta` y `emocion` (bocadillo con un icono del atlas `iconos.json`). Con eso, sin pedir arte:

| Cinemática | Nivel | Cuándo | Qué se ve |
|---|---|---|---|
| `xila_desprecio` | ciudad_oeste | `bnd1_xila_maltratado` | Xila se vuelve de espaldas y rumia (icono `enfurecido`) antes de `b1_xila_resentido` |
| `fiesta_salon` | mansión | al entrar con `bnd2_fiesta_dentro` | la **anfitriona** y el **mercader** (nuevos en el nivel, solo visibles la noche de la fiesta) salen a recibirte (`encantado`, `oro`) y vuelven al salón |
| `barricada_llamas` | afueras | `bnd3_barricada_vista` | el paso central de la tapia (19..21, 13) arde con fuego blanco al oír los gritos |
| `barricada_rota` | afueras | `bnd3_barricada_resuelta` | el fuego se apaga casilla a casilla, el centro el último |
| `epilogo_barca` | cualquiera → prado del río | `bnd3_epi_empezado` | fundido a blanco, la barca de Sho-Noco con él en la popa y tú en la proa (como en la subida del prólogo), y vuelta; **`b3_barca` espera** a `cine_epilogo_barca_vista` |

Sho-Noco y su barca vuelven a verse en el epílogo (`apariciones`: `visibleSi alguna [bnd_prologo_barca_viene, bnd3_epi_empezado]`, oculto desde que se va **salvo** en el epílogo).

**Lo que no se hizo y por qué**: la cueva (Gorran y sus bandidos saliendo de las lonas) espera al nivel de Qwen (AV19-04); los Perdidos **patrullando** el Ocaso son rutinas (`rutinas_pnj.json`), no cinemáticas (una cinemática se ve una vez y para al jugador); la arenga de Venides en el cuartel pide a Venides como objeto en `interior_cuartel` (hoy solo está el sargento) y la auditoría «solo texto» de `claude/plan_jugado_16-10.md` (qué PNJ hablan sin estar en el nivel) es la que debería decidir esos añadidos de golpe, no uno a uno.

## Validación
- `validar_todo.py`: **0 errores**, 22 avisos (los de siempre: mundi_landmass, recuerdo_taberna). Para llegar a 0 hubo que anotar en `tools/recompensas_escenas.json` los beats que pagan (disfraz parcial / segunda, bandidos sin jefe, Venides con pruebas) y renumerar las opciones de Venides; de paso se absorbió el error heredado de `b2_aigren_orden_entrega` (68 premios, auditoría OK).
- `validar_cinematicas.py`: 11 cinemáticas, 0 errores, 0 avisos. `gen_pnj.py` + `validar_pnj.py`: 270 PNJ, 346 aspectos (los dos anfitriones nuevos horneados).
- `analisis_trama.py`: ninguna bandera exigida sin quien la ponga (quedan `bnd_prologo_kit_listo`, que la enciende el motor, y `pre_bnd_forzar_tragedia`, de la precuela); 46 tiradas, ninguna con fallo sin lectura salvo `b1_secta_s2` (explicado arriba).
- Rutas simuladas con el cruce de banderas: sobrio / trago / desprecio llegan las tres a la fiesta y al Ocaso con y sin Xila. **La partida en Windows la hace Oriol** con esta rama (la cinemática del epílogo carga `prado_del_rio` desde el Ocaso: es lo primero que conviene mirar).
- Aventuras: 263 / 154 / 102 beats (primer día / segundo / Ocaso).

## Para Oriol
1. **El traidor del día 1** (PO-54 en la bandeja): `bnd1_unido_al_culto` y `bnd1_verdaderos_unido` siguen sin que nadie las lea.
2. `agente/fable/ciudad` → `claude/AV18-cierre`: `out/fable_ciudad01/aplicar.sh` (ya incluye estos commits al estar en la misma rama). Después, `git push` de la rama que decidas.
3. Al entrar la cueva de Qwen: `python3 tools/repartir_botin.py` y `python3 tools/parche_c_trama.py` (retarget de `b2_cueva_llegada` a `cueva_gorran`, hoy fuera de alcance).
