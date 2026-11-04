# F-DIA1_CONSECUENCIAS — una consecuencia pequeña para cada ⚠ del día 1

Fable, 03/11/2026. Encargo: dar un eco posterior (Gorran, día 3 u Ocaso) a las decisiones del día 1 que hoy no dejan rastro (MATRIZ_DIA1.md, «Hallazgos ⚠», líneas 234-241). Solo propuestas; no se ha tocado ningún JSON. Las ⚠ de las líneas 242 (umbral `confianza_rosita >= 3`) y 243 (arnés sin `goldDelta`/`actos`) no son de este encargo.

Fuentes leídas (solo grep/sed): `assets/adventures/boundington_{primer_dia,dia_gorran,segundo_dia,ocaso}.json`, `assets/catalogs/quests.json`, el canon `Boundington los diálogos.md` (líneas 528-560, 653-712) y D0022/D0023. Nada de lo propuesto contradice el canon: el «Bebe conmigo», el «Alguien que recuerda las caras», el «¡Os acordaréis de mí cuando arda!» y el Rufo de «me dijeron que había pan» están ahí tal cual.

Convenciones: textos sin tildes ni ñ (campos `lines`/`speaker`/`texto`), como ya hacen `segundo_dia` y `ocaso`. Cuando una opción se desdobla en variantes, la original recibe los `notFlags` complementarios para que las condiciones sean excluyentes. Coste: S = una opción/beat con una o dos variantes, sin flags nuevas leídas después; M = toca una tirada o más de un fichero con prueba.

| # | Decisión del día 1 | Dónde se ve | Coste |
|---|---|---|---|
| 1 | Venides: «Confiar en él» | día 3, noche, informe a Venides | S |
| 2 | Pelea: «Proteger al borracho» (con Luisarda) vs «Quedarte mirando» | Gorran, Rosita en la taberna | S |
| 3 | Pelea: «Separarlos» ganada / perdida / huida | día 3, castillo, charla con Venides | S |
| 4 | Camorrista: «matarlo en la calle» / «Echarle a la calle» | día 3 capitán / Ocaso barril | M + S |
| 5 | Sandes: «Solo miraba.» vs «¿Y tu quien eres para mandar?» | Ocaso, sermón, «Acercarte al estrado» | S |
| 6 | Cámara: «Me he perdido.» vs «Tu padre te escribe desde Killville.» | Ocaso, Edul contra el abrevadero | S |
| 7 | Guarida: «Plantarles cara» (`bnd1_guardias_avisados`) | día 3, torre del capitán | S |
| 8 | Verdaderos: «Plantarles cara» vencidos / ganan / huido | Ocaso, el reparto de pan | S |

## 1. Venides, «Confiar en él: Bebe conmigo» (`bnd1_venides_confianza`)

- **Hoy**: nadie lee la flag; «Desconfiar» sí tiene su eco («Ayer no te fiabas de mi. Hoy traes papeles.»).
- **Dónde**: `boundington_segundo_dia.json`, beat `b2_noche_rosita_informa` (~13160), opción «Contarselo todo». Desdoblar: la variante nueva `requires.allFlags: ["bnd1_venides_confianza"]`; la actual añade `notFlags: ["bnd1_venides_confianza"]` (ya lleva `notFlags` desconfianza). Mismos `effects`.
- **Texto** (primera línea nueva; el resto igual):
  `"Venides: «Anoche bebiste conmigo sin pedirme nada. Hoy me lo cuentas todo. Dos veces en dos dias; eso no se me olvida.»"`
- **Coste**: S. Simétrico al eco que ya tiene «Desconfiar».

## 2. Pelea, «Proteger al borracho dormido» (variante con Luisarda) vs «Quedarte mirando»

- **Hoy**: ponen las mismas tres flags. El canon quiere que se distingan solo por `confianza_rosita` «si no la tenía ya por Luisarda»; con Luisarda no queda nada.
- **Día 1**: añadir `setFlag bnd1_pelea_protegido` a las dos opciones «Proteger» de `b1_pelea` (`primer_dia.json` ~26939 y ~26972). «Mirar» no lo pone.
- **Dónde se ve**: `boundington_dia_gorran.json`, beats `bg_skilla_hora_pronto` (~10433) y `bg_skilla_hora`: opción nueva «¿Y el pesado de anoche?» con `requires.allFlags: ["bnd1_pelea_rosita"]`, `notFlags: ["bndg_rosita_pelea_t"]`, `effects: setFlag bndg_rosita_pelea_t`, `sigue: true`. Dos variantes:
  - con `bnd1_pelea_protegido`: `"Rosita: «Arriba, con la cabeza como un bombo. Ha preguntado quien le guardo la espalda y le he dicho que tu. Hoy el guiso lo pagas con eso.»"`
  - sin (`notFlags`): `"Rosita: «Arriba. Y tu anoche de pie, mirandome las manos. Mira menos y ayuda mas, que aqui las sillas vuelan.»"`
- **Coste**: S. Sin `confianza_rosita` (respeta el tope de 2 del día 1).

## 3. Pelea, «Separarlos»: ganada / perdida / huida

- **Hoy**: el día 3 solo mira `bnd1_pelea_lanzada`; Venides contesta igual a los cuatro caminos («¿Que pelea? ...¿Y yo dormia? Pues eso no lo pongo.»).
- **Dónde**: `boundington_segundo_dia.json`, `b2_venides_castillo_s1_a` (~3304), opción «¿Anoche escribiste lo de la pelea?». Tres variantes por `anyFlags`, mismos `effects`: `["bnd1_pelea_ganada"]` / `["bnd1_pelea_perdida","bnd1_pelea_huida"]` / `["bnd1_pelea_rosita"]` (texto actual).
- **Texto** ganada: `"¿Que pelea? ...¿Y yo dormia? Rosita dice que lo tumbaste tu solo. Eso si lo pongo: un hombre que se mete vale por diez que miran."`
- **Texto** perdida/huida: `"¿Que pelea? Ah. Rosita dice que te sacaron a rastras. Eso no lo pongo. Por tu bien, y por el mio."`
- **Coste**: S. El diario (`quests.json` `bnd1_pelea`) ya distingue ganada; con esto el jugador lo oye en juego.

## 4. Camorrista: «Sacarlo y matarlo en la calle» / «Echarle a la calle»

- **Hoy**: `bnd1_camorrista_muerto` solo pesa en moral y Aegroum −5; «Mañana el barrio lo sabrá» no se cumple. `bnd1_camorrista_echado` promete «os acordareis de mi cuando arda» y no vuelve.
- **Muerto, dónde**: `boundington_segundo_dia.json`, `b2_capitan` (~3568), opción «Pedirle que prepare a la guardia [Persuasion]». Variante con `allFlags: ["bnd1_camorrista_muerto"]` y `rivalDice: 4` (la original 3; añade `notFlags` muerto). Pequeña desventaja: un dado rival más.
  `"Capitan: «¿El de la taberna? Esta manana me han traido un muerto con capa gris de la calle de la Humilde. Ya se quien eres. Habla, y rapido.»"` + `"(Tirada de Persuasion.)"`
- **Echado, dónde**: `boundington_ocaso.json`, `b3_barricada_gris` (~4119): beat gemelo con `allFlags: ["bnd1_camorrista_echado"]` (el original añade `notFlags`), misma batalla; segunda línea:
  `"El fanatico del ultimo barril se vuelve: barba de dias, capa gris y la cara aun hinchada de la taberna. «Te dije que te acordarias de mi.»"`
  Aviso: `b3_barricada_gris` exige `bgc_cajas_llegan` (camino C de Gorran); si se quiere en todos los caminos, el mismo texto cabe en `b3_barricada_floja` (~4086).
- **Coste**: M (muerto: toca tirada, pide prueba) + S (echado).

## 5. Sandes, «Solo miraba.» vs «¿Y tu quien eres para mandar?»

- **Hoy**: idénticas; `bnd1_sandes_conocido` y `bnd1_sandes_enfadado` no se leen. En el Ocaso Sandes ya dice «{nombre}. Te recuerdo. Recuerdo todas las caras» a quien tenga `bnd1_sandes_te_conoce`.
- **Día 1**: añadir `setFlag bnd1_sandes_desafiado` a «¿Y tu quien eres para mandar?» (`primer_dia.json` ~22803). Mantener `bnd1_sandes_enfadado` en ambas.
- **Dónde se ve**: `boundington_ocaso.json`, página del sermón (~1033), opción «Acercarte al estrado» con `allFlags: ["bnd1_sandes_te_conoce"]`. Tres variantes excluyentes, mismos `effects`:
  - `allFlags: [sandes_te_conoce, bnd1_sandes_desafiado]`: `"Sandes: «{nombre}. El que preguntaba quien soy yo para mandar. Mira a tu alrededor: esta es la respuesta. Coge pan.»"`
  - `allFlags: [sandes_te_conoce, bnd1_sandes_conocido, bnd1_sandes_enfadado]`, `notFlags: [bnd1_sandes_desafiado]`: `"Sandes: «{nombre}. El que solo miraba. Sigue mirando; hoy hay mucho que ver.»"`
  - la actual: `notFlags: ["bnd1_sandes_enfadado"]`. (`bnd1_sandes_conocido` evita que el «solo miraba» le salga a quien se enfadó con Sandes soltando al sereno en la cripta.)
- **Coste**: S. Da uso a las dos flags huérfanas.

## 6. Cámara de Sandes, «Me he perdido.» vs «Tu padre te escribe desde Killville.»

- **Hoy**: mismas flags (`bnd1_sandes_te_conoce`, `bnd1_sandes_se_va`). La segunda es un golpe personal con una pista ganada (`bnd1_sandes_killville_oido`); merece que Sandes se acuerde.
- **Día 1**: añadir `setFlag bnd1_sandes_carta_dicha` a esa opción (`primer_dia.json` ~20918 y ~21065). «Me he perdido» no deja rastro: esa es la diferencia (callar no te pone en la lista).
- **Dónde se ve**: `boundington_ocaso.json`, `b3_edul` (~4980): beat gemelo con `allFlags: ["bnd1_sandes_carta_dicha"]` (el original añade `notFlags`), mismas opciones; tercera línea:
  `"Guardia, mirandote: «Y Sandes dice otra cosa: que si aparece el de la carta de Killville, arda con el. Asi que tu veras.»"`
- **Coste**: S. Slot distinto del de la propuesta 5 para no encadenar tres condiciones en el sermón.

## 7. Guarida, «Plantarles cara» (`bnd1_guardias_avisados`)

- **Hoy**: «arriba alguien grita llamando a la guardia» y nadie lo recuerda.
- **Dónde**: `boundington_segundo_dia.json`, `b2_capitan` (~3568): opción nueva «Anoche llamaron a la guardia a la iglesia vieja. ¿Fue alguien?» con `allFlags: ["bnd1_guardias_avisados"]`, `notFlags: ["bnd2_capitan_t_iglesia"]`, `effects: setFlag bnd2_capitan_t_iglesia, setFlag bnd2_pista_guardia`, `sigue: true`.
  `"Capitan: «Fue una ronda. Volvieron diciendo que no habia nada y oliendo a incienso. Dos de esos hoy no han venido a formar.»"`
- Nota: `bnd2_pista_guardia` hoy solo la pone «¿Cuantos hombres tienes?» y tampoco la lee nadie; ponerla aquí es por coherencia, no da ventaja. Si se quiere una ventaja real, que esta opción marque también `bnd2_capitan_hombres` (ahorra una pregunta).
- **Coste**: S.

## 8. Verdaderos, «Plantarles cara»: vencidos / ganan / huido

- **Hoy**: solo moral; «el que planta cara no sabe nunca si ganó» (el diario sí lo dice: `quests.json` `bnd1_verdaderos`, 472-477, pero no sale en juego).
- **Dónde**: `boundington_ocaso.json`, dos beats `auto` nuevos en la plaza tras el sermón, `lugar: ciudad_centro`, `allFlags: ["bnd3_sermon_visto"]`, `notFlags: ["bnd3_rufo_visto"]`, `effects: setFlag bnd3_rufo_visto`:
  - `b3_rufo_pan`, `anyFlags: ["bnd1_verdaderos_vencidos"]`: `"Entre los que cogen pan, Rufo, con la nariz torcida. Te ve, deja el pan en la cesta y se va calle abajo sin mirar atras."`
  - `b3_cabecilla_pan`, `anyFlags: ["bnd1_verdaderos_ganan","bnd1_verdaderos_huido"]`: `"Entre los que reparten pan, el cabecilla de la puerta azul con capa gris nueva. Te senala con la barbilla y se rie."`
- Encaja con el canon (Rufo: «me dijeron que habia pan» = el culto recluta con pan) y con el `b3_sermon_po54` de los unidos, que no se toca.
- **Coste**: S.

## Para quien lo ejecute (E, con prueba y mutación)

- Flags nuevas del día 1: `bnd1_pelea_protegido` (2), `bnd1_sandes_desafiado` (5), `bnd1_sandes_carta_dicha` (6). Todas se ponen junto a las existentes, sin quitar ninguna: ninguna partida guardada se rompe.
- Flags nuevas de cierre: `bndg_rosita_pelea_t`, `bnd2_capitan_t_iglesia`, `bnd3_rufo_visto`.
- Comprobación mínima por propuesta: dos guardados fabricados (con y sin la flag) y ver que sale la variante correcta y nunca las dos; para la 4 (muerto), además, que `rivalDice` sea 4 solo con `bnd1_camorrista_muerto`.
- Orden sugerido si hay que elegir: 1, 3, 5 (una opción desdoblada cada una, cero riesgo), luego 2, 7, 8, y al final 4 y 6 (beats gemelos).
