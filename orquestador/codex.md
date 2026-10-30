# codex (ChatGPT) — cola de tareas del 19/10: la campaña con todos en escena
**Asignada:** 19/10 por Claude (coordinador), a petición de Oriol: «plantea las tareas necesarias a Qwen y ChatGPT para actualizar el juego con todas las nuevas decisiones y mejoras de mapa más campaña» · **Repo:** `MotorGraphico-main` · **Tipo:** trabajo nuevo, una rama por tarea. La cola del 15/10 está en `orquestador/historial/codex_15-10.md`.

Tu parte: **la campaña de Boundington, escena a escena**, con el método que ya está probado en la Barriada (`out/claude_escenas01/`): los personajes colocados en su mapa, cinemáticas que hacen lo que el texto dice, charlas con todos, varias maneras de resolver cada decisión y una segunda oportunidad al fallar. Todo es JSON y Python de herramientas: **nada de C++**. Los mapas los hace Qwen (su cola, `orquestador/qwen.md`); tú escribes encima.

## 0. Lo que manda (léelo antes)

| Qué | Dónde |
|---|---|
| El guion de Oriol, **canon**: si el juego y el guion discrepan, manda el guion; si el guion no dice algo, se propone y se marca «propuesta» | `Onegai 2/Onegai/OnegaiTimeLine/Campanyes/Canon/Aegroum/Campanya 1 Boundinghton/Boundington los diálogos.md` (día 2: 703–1042; Ocaso y epílogo: 1043–1318) y la carpeta `Subquests/` |
| El principio del 16/10: **múltiples maneras**, **todos presentes**, **sin texto que narre lo que nadie hace**; segunda oportunidad en toda decisión importante | `claude/informe_e_escenas01.md` |
| **El modelo**: la Barriada hecha así (10 PNJ, 6 cinemáticas, 5 salidas, segunda oportunidad, validado, 300 partidas sin atascos) | `out/claude_escenas01/README.md` y `guiones/barriada.py` — **copia su estructura**: un `guiones/<escena>.py` por escena que usa `lib.py`, un `aplicar.py` idempotente, y los cambios por `Adv.pon/cambia/lineas`, nunca editando el JSON a mano |
| El formato de aventuras y de cinemáticas (triggers, `opciones`, charlas, `startBattle` con oleadas y refuerzos, `apariciones`, órdenes `anda/mira/funde/beat/trepa/arde/tumba/emocion`) | `FORMATO_AVENTURAS.md` |
| Los PNJ se hornean con las piezas del jugador: composición en `tools/actores_pnj.py` + `python3 tools/gen_pnj.py`; `validar_pnj.py` exige fila propia a cada PNJ | `tools/actores_pnj.py` (bloque ESCENAS-01 al final, como ejemplo) |
| Cada premio (oro u objeto) en el libro `tools/recompensas_escenas.json`, en el grupo de su alternativa; el presupuesto del día no se mueve | `FORMATO_AVENTURAS.md` «Recompensas de escena» |
| Lo que ya está hecho y no se rehace: día 1 entero, la cripta en cinco mapas (AV19-08), el Encargo C (Xila, tiradas con tercer camino, cueva sin encargo, Verina segunda, epílogo en la barca), la Barriada (ESCENAS-01) | `claude/informe_c_aplicado.md`, `claude/informe_av19_08_cripta_secta.md` |
| La zona de Gorran (sendero, Gola Fosca, aldea, galerías, cámara) la hace **Fable entera**, trama incluida | `claude/encargo_av19_07_aldea_gorran.md` — **no toques** `b2_cueva_*`, `b2_gorran_*`, `b3_gorran*` |

**Base de tus ramas:** `claude/escenas01` si existe con la Barriada commiteada; si no, `claude/AV18-cierre` y pasa antes `python3 out/claude_escenas01/aplicar.py && python3 tools/gen_pnj.py`. Dilo en el INFORME.

**Antes de empezar (obligatorio):** `orquestador/README.md`, `AGENTS.md`, una rama por tarea (`git worktree add ~/onegai-ramas/codex-<TAREA> -b agente/codex/<TAREA>`), y al acabar cada escena: `python3 tools/validar_todo.py` (0 errores nuevos), `python3 tools/validar_cinematicas.py`, `python3 tools/gen_pnj.py && python3 tools/validar_pnj.py`, `python3 tools/analisis_trama.py` (ninguna bandera exigida sin quien la ponga), y el **explorador**: copia `out/claude_dialogos02/guiones/{motor.py,explora.py}` a `out/sim/`, añade tus PNJ a `COLOCADOS` y corre `python3 out/sim/explora.py 300 7`: **0 atascos y ningún beat nuevo sin ver** (si alguno no se ve, explica por qué o arréglalo). Sesión en `sesiones/`, fila del TABLERO, preguntas a `BANDEJA_PRODUCT_OWNER.md` con PO-xx. Nunca push ni merge.

**Reglas de escritura (las de Oriol):** castellano del guion, frases cortas; cada línea con quién la dice (`Quien, acotación: «texto»`); una charla = temas que se preguntan una vez + «Hablar de otra cosa» + despedida; las tiradas con sus cuatro grados y **tercer camino** en el fallo (AV17-01); un `acto` por decisión con peso (`assets/rules/actos.json`, añade los que falten); **nunca** un beat `auto` que narre que alguien llega, corre, se va o pelea: eso es una cinemática con ese PNJ en el nivel. Los nombres que el vault no da son propuesta (dilo en la sesión).

**Contrato con Qwen:** sus INFORMES traen, por nivel, las **casillas reservadas** para cada personaje, las puertas y las zonas de combate. Tus cinemáticas usan esas casillas; si te falta una, pídesela en su INFORME y pon un marcador provisional (`coloca` en una casilla libre) para no parar.

---
## 1. ESCENAS-02 — Día 2 · El castillo y la noche con Venides (sin esperar a Qwen)
Guion 703–750 y 1004–1042. Mapas de hoy: `interior_castillo` (+p1, p2) y `interior_taberna`.
- **La puerta** (`b2_puerta*` ya existe): dos guardias colocados (`guardia_castillo` ×2) en la puerta; el escudo como charla.
- **Venides baja de la torre** (cinemática `castillo_venides`: Venides, que hoy solo existe en la taberna, aparece en `interior_castillo` por bandera `bnd2_encargo` y baja por la escalera hasta ti; `emocion` de resaca). Sus tres variantes (`bnd_venides_20` / rumores / «lo de Edul») siguen; añade la charla de 4 temas que ya está y **una salida más**: ir con él a ver al capitán (te abre la torre sin que el capitán te eche).
- **El capitán en la torre** (`capitan_guardia` en `interior_castillo_p1` o donde Qwen ponga la mesa de papeles): PNJ colocado; Persuasión 3 dados con tercer camino (hoy: pruebas o Venides). Segunda oportunidad nueva: el sargento de la puerta («al capitán le gustan los hechos: tráele una de esas espadas sin escudo»). Gancho pendiente de AV19-08: **`bnd1_sereno_libre`** (si liberaste al sereno en la cripta, el capitán ya sabe lo de las barricadas y te escucha sin tirada).
- **La noche en la taberna** (1004–1042): la **tranca** y la Furia en su hueco (objetos de Qwen, `apariciones`), **Venides sobrio con la armadura** (variante de su objeto o `emocion`), la vecina de la ventana, los tres jornaleros con la jarra sin tocar; **Xila entra sobrio** por la puerta (cinemática `taberna_xila_entra`) o duerme en el rincón; Rosita apagando las velas una a una (cinemática corta antes de `b2_cierre*`) y, con confianza ≥ 2, «Yo no duermo esta noche». Las tres maneras de informar a Venides ya existen: añade la de **Rosita se lo cuenta por ti** (si le contaste todo al devolver la espada, `bnd2_rosita_lo_sabe`: Venides ya lo sabe cuando llegas) y la de **enseñarle los papeles** sin hablar.
**Entrega:** `out/codex_escenas02/` con `guiones/castillo.py`, `guiones/noche.py`, `aplicar.py`, README con el mapa de banderas y los PNJ nuevos.

## 2. ESCENAS-03 — Día 2 · La fiesta dentro de la mansión (empieza con la mansión de hoy; cambia a la de Qwen AV19-12 cuando esté)
Guion 935–1003 y `Subquests/Fiesta_Noche_Disfraces.md`.
- **La calle de noche** (`ciudad_este`, franja 2): el **cobrador** bajo la farola y el **vigilante** en la esquina como PNJ colocados por bandera; **Edul con máscara blanca** en la calle si te citó (`bnd2_edul_buscado`): cinemática `fiesta_edul` (se acerca, «ponte detrás de mí», os lleva hasta la puerta).
- **La escalada**: cinemática con la orden `trepa` (existe) por la ventana del jardín; el traje roto (1d4) ya existe: que **se vea** (icono `emocion` y que la anfitriona lo note).
- **Dentro**: los **40 invitados** colocados por bandera `bnd2_fiesta_dentro` (PNJ ambientales: `invitado_noble` ×6 con variantes, `invitado_mercader` ×4, `seguidor_brinda`, `cuarteto` ×4 (un `musico` con variantes), `criado` ×3, los anfitriones que ya hizo Fable). Cada grupo con su charla corta y su pista (nobles: «la noche siguiente, cerca del Casco»; mercaderes: se van; el violinista: «roba partituras»).
- **Sonsacar al noble** (Engaño 3, 4 con el traje roto): éxito → señalarlo a Venides / chantajearle (existen); **nuevo tercer camino**: emborracharle (pagar tres copas, 6 de oro, Carisma sin tirada) → te lo cuenta igual pero **te recuerda** (día 3 en la plaza).
- **Arriba, el despacho** (`mansion_barrio_alto_p1`): Sigilo 3; documentos; robo del guardarropa (existe); si te pillan, **huida por la ventana del jardín** como cinemática (criado grita, anfitriona «¡Ladrón!», tú corres) en vez de narrarlo; segunda oportunidad: el vigilante te lleva a la taberna pero el cobrador, si le ayudaste (lo denunciaste), te deja volver a entrar por la cocina.
- **Volver a la fiesta** (`b2_fiesta_vuelta`, de Encargo C) con el portero colocado.
**Entrega:** `out/codex_escenas03/` igual que arriba. Los invitados: composiciones en `actores_pnj.py` con `_varia` (máscaras de medio rostro: pide a Pixel la pieza `mascara_media` y mientras tanto `capucha`/`sombrero_pluma`).

## 3. ESCENAS-04 — Ocaso · La plaza del sermón y la Base Militar (necesita AV19-11 de Qwen para la base; la plaza, con `ciudad_centro` de hoy)
Guion 1043–1114.
- **La plaza por la mañana del día 3**: el estrado y las antorchas (objetos de Qwen, `apariciones` por `bnd2_dia_cerrado`), **Sandes, Verina y Edul sobre el estrado** (PNJ nuevos `sandes_plaza`, `verina`, `edul_plaza`: Verina adulta no existe aún; composición con el libro en la mano), cinco Perdidos formando pasillo, vecinos en sillas, niños en la fuente. El sermón como cinemática: Sandes reparte pan (anda entre la gente), Verina **cierra el libro con el dedo dentro** (`emocion`), Edul no levanta la vista. La charla del sermón (`b3_sermon*`) ya existe: cuélgala de la cinemática y añade **el gancho de AV19-08** `bnd1_cripta_sospecha` (Sandes te reconoce de la cripta: «el novicio que no era novicio») y el noble de la fiesta si le emborrachaste.
- **Avisar a Venides** (`b3_avisar_venides`) en la base, no en la taberna: Venides en el patio por la mañana con los suyos haciendo instrucción.
- **Las campanas** (cinemática `base_campanas` al entrar en `base_militar` con `bnd3_avisado`): fundido a rojo (dirección visual §3: el Ocaso naranja), banderas, los **tres exploradores heridos entran por el portón** (anda), Venides ya con la espada fuera, **sus 20/10/0 hombres formados** (soldados visibles por `bnd_venides_20`/`_10`), el **capitán en la torre** gritando (si `bnd2_guardia_preparada`), **Aigren entrando con el carro** (si `bnd1_aigren_redimido`), **Xila junto al pozo**, sobrio con vendas o cantando. Todo lo que hoy son beats `auto` de texto (`b3_venides_arenga_*`, `b3_capitan_ronda`, `b3_aigren_carro`, `b3_xila_*`) pasa a abrirse desde la cinemática con la gente en su sitio.
- **Por dónde** (`b3_por_donde*`): tres puertas reales del patio; la opción es elegir una puerta, no un menú (los beats siguen por `enter`).
- **Primer combate**: los **saqueadores saltan la muralla trasera** (PNJ de escena `saqueador_muralla` ×3 que aparecen y andan hasta el patio; los números por hombres de Venides ya existen) y los **fanáticos en el tejado** (`fanatico_tejado` ×2 en la casilla de altura 1; `arde` para el fuego en las manos). Oleada y «quedarse» como hoy, con la base cayendo por el fuego (cinemática: `arde` sobre los barracones).
**Entrega:** `out/codex_escenas04/`. PNJ nuevos: `verina`, `sandes_plaza`, `edul_plaza`, `soldado_venides` (con variantes hasta 6 y repetido en filas), `explorador_herido` ×3, `saqueador_muralla`, `fanatico_tejado`, `capitan_guardia`.

## 4. ESCENAS-05 — Ocaso · Verina, Rosita y Sandes en el Casco ardiendo (necesita AV19-09 de Qwen)
Guion 1115–1211.
- **La barricada de Verina** (salida oeste del Casco ardiendo, casillas de Qwen): dos cultistas la sujetan (PNJ `cultista_carcelero` colocados, Verina atada: variante `verina_atada`); con Venides, **él corre** (cinemática, no hay que decidir); sin él, mirarla (Percepción 1) y liberarla o seguir. Liberada: se frota las muñecas, mira a Venides (cinemática corta, `emocion`), y las tres salidas de hoy (`b3_verina_*`) más la **segunda** de Encargo C (`b3_verina_segunda`). Si la dejas, al volver la cabeza ya no está (aparición apagada).
- **Rosita en su calle**: en la esquina de la Taberna Humilde, **una docena de Perdidos con antorchas** (PNJ de escena ×12, variantes) y **Rosita de espaldas con la Furia ardiendo** (variante `skilla_furia`, `arde` sobre la espada si se puede); cinemática `rosita_calle`: os pasa sin volverse; las dos opciones de hoy; **se la ve de lejos mientras seguís** (los Perdidos y ella se quedan en el nivel con rutina de pelea hasta que salís: orden `patrulla` corta). Si desconfía: taberna cerrada, tranca puesta (objeto).
- **Pulga en el tejado** (si `bnd1_carterista_amigo`): silba (cinemática: aparece en una casilla de altura, `emocion`), y el **patio de los toneles** de Qwen es un camino de verdad que rodea la plaza.
- **Sandes en el estrado ardiendo**: PNJ `sandes_ocaso` colocado, **dos sectarios en los portales** y **dos fanáticos en lo alto de las escaleras** (uno si `bnd1_guarida_exterminada`); las tres aperturas de hoy (Venides / Xila / Verina) y la oferta, como cinemática + charla; al llegar a 0, **se va por la iglesia** (cinemática `sandes_huye_casco`: anda hasta la puerta de la iglesia y desaparece; los fanáticos bajan). Añade la **cuarta apertura** del guion que falta: «¿Y tú qué ganas?» → «Todo. Por eso salgo antes de la señal» (línea 1198).
**Entrega:** `out/codex_escenas05/`. PNJ nuevos: `verina_atada`, `skilla_furia`, `perdido_antorcha` (×12 con variantes), `sandes_ocaso`, `pulga_tejado`.

## 5. ESCENAS-06 — Ocaso · Las Afueras, el combate final y la huida (necesita AV19-10 de Qwen)
Guion 1212–1318.
- **Las Afueras al ocaso**: `b3_afueras` deja de ser texto: al entrar, cinemática `afueras_llegada` con lo que corresponda por banderas: **Zarela a caballo** (PNJ `zarela_caballo`, o Zarela con la orden `anda` más rápida) y **sus guerreros con antorchas y hondas abriendo pasillo** desde los huertos (si `bnd2_guerreros_gitanos` y no delatada; con `bnd2_saq_convertidos`, también Teo y los suyos); **Gorran y sus hombres cargando** por un costado (si `bnd2_gorran_perdonado` o pagado; `b3_gorran_sin_jefe` si murió). Fable entrega los PNJ de Gorran: usa `gorran` y `bandido_de_gorran` tal cual.
- **La barricada en llamas** (las cinemáticas `barricada_llamas`/`barricada_rota` de Encargo C ya existen: muévelas a `afueras_ocaso`): derribarla, romperla a distancia, o el hueco sin frascos (si Edul te explicó la mezcla); herida 1d4 al derribarla.
- **Edul contra el abrevadero** con tres guardias de cintas grises (PNJ colocados); liberado: las tres variantes de hoy; con Verina, **el abrazo** (cinemática: ella anda hasta él, `emocion` ×2); si te aliaste con Sandes, Edul furioso (existe). Edul **siempre se ofrece a ir contigo** (regla de Oriol del 19/10, como en la cripta).
- **Venides de rodillas junto al molino** (si `bnd2_venides_sin_aviso`): tres guardias, el yelmo en el suelo (objeto), y las tres salidas (existen).
- **El combate final** junto al árbol grande: Venides peleando con Sandes **antes de que llegues** (cinemática `final_venides`: las cadenas, `emocion`), y la **compañía de Rosita** (`b3_compania`, refuerzo con `golpeMortal` ya existe): que **se vea llegar** — trueno sin nube (`funde` blanco 0,2 s), fuego negro (`arde` sobre las cadenas si el motor lo permite; si no, `emocion`), la flecha de luz, el aullido, y los tres PNJ (`mago_de_las_tinieblas`, `guardabosques` en una casilla de altura, `guerrero_demoniaco`) apareciendo entre Sandes y tú. Sandes al caer: **se deshace en cadenas y humo** (aparición apagada + `funde`).
- **La huida**: en la linde, **los carros de la Barriada** enganchados (objetos + Zarela) o nadie; Venides se para a mirar la ciudad (cinemática al entrar en `huida_boundington`: «Boundington entera como una línea naranja»); los finales de hoy (`b3_huida_*`, `b3_aliado_fuera`, `b3_caida`) y el **tercer final** de Fable (`b3_huida_galerias`, lo pone él). El epílogo en la barca ya está (Encargo C).
**Entrega:** `out/codex_escenas06/`.

## 6. ESCENAS-07 — Lo que queda del guion del día 2 (al final, sin dependencias)
- Herrería con Edul (822–845): Edul ya aparece por bandera; añade **los cuatro estados de Aigren** (se fue / cripta / convencido con la daga / se va hoy con el hijo: el hijo como PNJ `hijo_de_aigren` que sube al carro en una cinemática corta) y «¿Qué hay en esos frascos?» con los frascos de Qwen.
- Luisarda (846–853): los tres desenlaces con la Boutique **cerrada de verdad** (puerta con cartel, aparición), el sastre de enfrente colocado.
- Xila en el escalón (854–863) y la tienda abandonada (864–871): que el beat salte **al entrar en `tienda_abandonada`**, no en la calle; Xila entra contigo (cinemática) y se prueba el sombrero (`emocion`).
- Surysal: el forjador y la vecina ya están (ESCENAS-01); añade **el instructor de capa gris** por las tardes (PNJ `instructor_perdido`, franja 1) con su charla y la opción de seguirle (Sigilo → pista del puente).
- **PO-54, el traidor del día 1**: `bnd1_unido_al_culto` y `bnd1_verdaderos_unido` siguen sin lector; escribe los lectores del día 2 y 3 (Sandes te trata como suyo; la Barriada te cierra la puerta) y déjalo marcado «propuesta» hasta que Oriol conteste.
**Entrega:** `out/codex_escenas07/`.

---
## Orden y ritmo
1 → 2 → 7 → 3 → 4 → 5 (3, 4 y 5 cuando Qwen entregue AV19-11, AV19-09 y AV19-10; mientras, 1, 2 y 7 no dependen de nadie). Cada paquete entra por `aplicar.py` sobre la rama anterior; Claude integra en `claude/escenas0N`. Si una escena pide una orden de cinemática que el motor no tiene (nadar, montar a caballo, disparar), **no la inventes**: `emocion` o `anda`, y la petición a Claude en el INFORME.

## INFORMES (uno por tarea, aquí debajo)

### ESCENAS-02 — Codex, 19/10

Implementado en `agente/codex/ESCENAS-02` desde bfc5a22. Paquete `out/codex_escenas02/`, README con banderas, posiciones y evidencia. 300 partidas semilla 7: 0 atascos, cuatro fases 300. Cinemáticas y PNJ correctos; siete contratos e idempotencia. validar_todo: solo el error previo de la daga de Aigren. **Abierto:** AV19-08 no localizado: el lector bnd1_sereno_libre se prueba con estado de entrada, pero no hay productor en esta base. Analizador lo señala y no se considera superada esa puerta. No push ni merge.

### ESCENAS-03 — Codex, 19/10

Rama sucesiva agente/codex/ESCENAS-03 sobre aa35ae8. Fiesta con 40 asistentes, conversaciones, Edul, escalada, despacho real, copas, fuga y cocina; paquete out/codex_escenas03. 300 partidas semilla 7, 0 atascos y todos los beats nuevos vistos. 24 cinemáticas y 298 PNJ correctos; contratos/idempotencia OK; analisis_trama 0 referencias nuevas. validar_todo conserva la incidencia previa de la daga. Máscara media y adaptación AV19-12 pendientes; el recuerdo del noble usa redacción propuesta. Sin C++, Gorran, push ni merge.

### ESCENAS-07 — Codex, 19/10

Rama sucesiva agente/codex/ESCENAS-07 sobre fa1dd54. Paquete out/codex_escenas07: estados de Aigren y carro, Luisarda cerrada, Xila y vestuario, instructor de tarde, lectores PO-54 propuestos y oferta de Edul al liberarlo. 300 partidas semilla 7: 0 atascos; cartel infrecuente cubierto con prueba dirigida, demás nuevos vistos. 27 cinemáticas, 306 PNJ, conectividad, contratos e idempotencia correctos; sin referencias nuevas sin productor. validar_todo conserva solo el fallo previo de la daga y 22 avisos. PO-54 pendiente de Oriol; compañía física de Edul para 06. AV19-08 no localizado en Software. 04–06 siguen esperando contratos Qwen (AV19-09 en curso, sin informe). No push ni merge.

**Commits locales del lote:** ESCENAS-02 `aa35ae8`; ESCENAS-03 `fa1dd54`; ESCENAS-07 `7373471`. Las tres ramas son sucesivas y los worktrees están limpios. Sin push ni merge.

### ESCENAS-04 — Codex, 21/10

Base actualizada a 2bdbdfe; paquete out/codex_escenas04. Sermón y ocho variantes de combate localizadas; campanas, arenga y rutas por cinemáticas. 300 partidas, 0 atascos; contenido 0 errores, 48 cinemáticas y 323 PNJ correctos. **Base provisional en interior_cuartel:** base_militar está ausente; consultado con Oriol. Patio/torre/puertas reales pendientes, sin declarar esa parte visual cerrada. No se modifican TMX ni Gorran. Sigue el anclaje de Casco/salida/Afueras con las decisiones del 21/10.


### ESCENAS-05 — Codex, 21/10

Rama sucesiva agente/codex/ESCENAS-05: paquete out/codex_escenas05. Verina (90,19) y captores, carrera previa de Venides, Rosita/12 Perdidos, Pulga, Sandes/huida en iglesia. Referencia de colores y Excel inspeccionada; geometría intacta. 300 partidas sin atascos, todos los nuevos vistos, contenido 0 errores, 56 cinemáticas y 335 PNJ correctos. Ligaduras/espada/estrado con acabado provisional. Sigue ESCENAS-06 para imponer final → Afueras → huida.


### ESCENAS-06 — Codex, 21/10

Rama sucesiva agente/codex/ESCENAS-06 sobre e2f5a69; integración inicial 2bdbdfe. Final al sur del Oeste antes de Afueras, puerta condicionada sin cambiar su salida, Edul cautivo exacto (79,33), excepción furiosa, abrazo/oferta y presencia exterior; huida física. 14 encuentros lógicos/25 variantes localizados; Gorran intacto. 300 partidas sin atascos, contenido 0 errores/22 avisos previos, 74 cinemáticas y 356 PNJ correctos. Oriol confirma captura única de Edul con traslado y escenario dentro/fuera del cuartel: cuartel.py mueve rutas/combates al exterior de ciudad_oeste. Arte y cinemática de compañía durante combate siguen limitados por los recursos/contrato existentes. Detalle y evidencia en out/codex_escenas06/README.md. Sin C++, TMX, push ni merge.

**Resolución de Oriol durante el cierre de 06:** «Escenari de dins i fora cuartel» y «Sí, una sola captura amb trasllat». Aplicado en cuartel.py y edul.py de 06. Las reservas de Base Militar/traslado de los informes anteriores quedan resueltas; solo persisten los límites artísticos indicados.


**Lote 21/10, commits locales sucesivos:** ESCENAS-04 `27d03f9`; ESCENAS-05 `e2f5a69`; ESCENAS-06 `cc3c954`. Base `claude/integracion-21-10` `2bdbdfe`, no la antigua escenas01. Sin push ni merge. Propuestas/arte pendientes en los README.

### GORRAN-V2-PLAN — Codex, 22/10

Encargo directo de Oriol: planteamiento de campaña ampliado (25apartados y consecuencias), objetivo de dos hitos entre Gorran y Boundington y acceso aTier2. Rama agente/codex/GORRAN-V2-PLAN, base claude/cripta-25 8c2843b; paquete planificacion/gorran_v2. El hito de Gorran no existe aún en la ejecución y el Tier2 exige además nivel20: calibración pendiente. No se aplican aquí21niveles ni cambios de aventura.

Auditoría de dosv7, hojaOtros idéntica:158celdasrío+48puente han perdido su semántica;4carteles convertidos en antorcha y4anclas de mapas futuros sin objeto. Evidencia por celda y hashes. Contratos de17plantas+4mapas y9escaleras, coordenadas/tamaños aún sin decidir como null; reparto de ocho barracas siguepropuesta. La plantilla original de títulos no está en la copia/historia: se entrega adaptación de trabajo identificada como tal, basada en el patrón funcional de CRIPTA-25.

25apartados desarrollan escolta, reloj3/6, sospecha/captura, tres amigos, farsa, Cojo fugitivo, prueba conlectores, rutas/fallos y Ocaso. Revisión independiente corrigió reloj tras resolver/matar aGorran, cronología deJoel y enlaces de recuperación. PO-55 agrupa lo pendiente. verificar_plan.py pasa; ningún asset/C++/canonmodificado; sinpush/merge. Las300partidas corresponden al futuro contenido jugable, no se atribuyen a este documento.

Commit local: `25fac97` en `agente/codex/GORRAN-V2-PLAN`.

### GORRAN-MAPAS-02 — Codex,22/10

Rama agente/codex/GORRAN-MAPAS-02 desde claude/gorran-v2 6b6368b. Petición directa: fortaleza72×72 de una planta, mina42×42 terminada y galerías42×42 con más contenido. Recinto continuo con patio y17dependencias; mina10zonas; galerías12zonas y dos circuitos. Nueva ruta física mina→mina2→galería minera→grieta→casa nueva del Oeste (confirmada por Oriol); fachada anónima y puerta(25,35),sin cambiar calles. No se edita trama ni C++.

Entrega out/codex_gorran_mapas02: Excel ciudad11 y7vistas en colores; notas,planos y validaciones. Todo el suelo útil conectado con objetos; ida/vuelta, llegadas libres, una planta, idempotencia y regeneración general comprobadas. Reimportación simulada de7mapas:0objetos quitados y0casillas cambiadas. Contenido0errores/24avisos; puertas14avisos iguales a base,0nuevos;319PNJ y46cinemáticas correctos.300partidas sin atascos. Los usos de nuevas salas y la huida delCojo son notas preparadas para la futura trama, no acciones implementadas. Sinpush/merge.

Commit local: `e50331e`.
