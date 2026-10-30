# Relevo E-CAN3
Agente y sesión: MiMo v2.6 (opencode) · 29-10-2026 · sesión nº 1
Estado: hecho
Pasos: 1 hecho · 2 hecho

## Hallazgos (con evidencia)
### P1 · perdido_devoto — *lo dice la vault*: 3 peleas, todas Día 1
- Día 1 · Casco Antiguo, «Los sectarios»: «Si insistes, pelean: 3 Devotos, y alguien avisa a la guardia» (`Boundington los diálogos.md:397`). Franja: mañana — «Ven esta noche a la iglesia» (:393) y la vieja: «vienen cada mañana» (:402).
- Día 1 · La guarida (noche): «Plantarles cara» → «Combate: 3 Devotos. Alguien avisa a la guardia» (:482); «De parte del herrero» → «3 Devotos y 1 Fanático» (:495).
- Día 1 · Pico Dragón, «Los Verdaderos»: «Por la tarde o la noche» (:519); fallo de Persuasión → «Combate: 5 Devotos» (:536).
- En el juego @ `agente/claude/F-C3`: `boundington_dia_gorran.json:3072,7032,20709` y `boundington_ocaso.json:2235,2457` (monsterId); doc `FORMATO_AVENTURAS.md:584-585`.
- **Propuesta:** dejarlo como está (es enemigo de set-piece, no PNJ con spawn); solo cuadrar franja Día 1 = mañana/tarde-noche según el nivel.

### P1 · cultista_carcelero — *no lo dice la vault*
- Búsquedas: `grep -ri "carceler|cárcel|carcel|prisión"` en `Onegai 2/` → 0 «carcelero»; lo único carcelero de Boundington es Xila: «Me escapé de la prisión» (`diálogos.md:600`) y la prisión de Ranmont (otra era). En Boundington no hay cárcel documentada.
- En el juego @ `agente/claude/F-C3`: `boundington_primer_dia.json:13794,14331,15063,15185,15238,15368` y `boundington_ocaso.json:2448,2531,2620,2709,3795,4828`; doc `FORMATO_ACCIONES_MONSTRUOS.md:9` (fallback de ObjectCatalog).
- **Propuesta:** renombrar/quitar «carcelero» o ligarlo al único cautiverio real del capítulo: Verina retenida en la Base Militar («Ocaso · La Base Militar y Verina retenida», `diálogos.md:1081-1157`) — *propuesta*.

### P1 · criado_fiesta — *lo dice la vault solo como decorado*
- Día 2 · La fiesta en el Barrio Alto, de noche: «un cuarteto tocando junto a la chimenea, **sirvientes con bandejas de copas y aperitivos**, unas cuarenta personas con máscaras» (`diálogos.md:937`). Sin nombre ni diálogo propio.
- En el juego @ `agente/claude/F-C3`: `boundington_segundo_dia.json:11963,12056,12149` («target»), `cinematicas.json:442,3656,3664`, `levels/mansion_barrio_alto.json:1988-2002`, `objects/boundington_npcs.json:911`, `textures/pnj_boundington.json:7197` (+variantes ~2/~3/~4), `out/codex_escenas03/guiones/fiesta.py:12`.
- **Propuesta:** dejarlo como PNJ de ambiente sin diálogo (la vault no le da líneas) — *propuesta*.

### P2 · músicos y nobles de la fiesta — *sí, la vault les da papel y diálogo* (todos `Boundington los diálogos.md`, «Día 2 · La fiesta» › «Dentro»)
- El cuarteto (:989): **Violinista:** «Llevamos tres horas tocando lo mismo porque el anfitrión no distingue una pieza de otra. Si vas a robar algo, roba partituras.»
- Los nobles (:986): «La noche siguiente, cerca del Casco Antiguo. Ellos se reúnen; nosotros ya no estaremos en la ciudad.» **Pista.**
- Noble que brinda, éxito de Engaño (:993): «Cuando arda el Casco Antiguo, mis almacenes de Surysal valrán el triple. La luz es muy buena para los precios.» → señalar a Venides `bnd2_noble_senalado` (:994); chantaje: «Treinta de oro y aquí no ha pasado nada.» (:995).
- Contexto: los disfraces-músico dan línea del jugador (:956, :979); `grep -ri "músic|noble"` en `Capítols/` y `Personatges/` → 0.
- **Propuesta:** dar esas 3 líneas a los PNJ mudos (un músico del cuarteto y el noble del brindis) — *propuesta*.

## SIGUIENTE ACCIÓN
Ninguna: E-CAN3 hecha. Orden para quien recoja: no rehacer nada; Claude debe volcar estas 3 secciones en `orquestador/DECISIONES_PENDIENTES.md` (`INDICE.md:20`) cuando estén los tres E-CAN*.

## Dudas para Oriol
- `cultista_carcelero` no existe en la vault — en el juego sí (6 apariciones en `boundington_primer_dia.json`) — ¿renombramos a «cultista de la barricada/cautivo» o lo ligamos a Verina retenida?
- Los 3 músicos: la vault solo da 1 línea para el cuarteto entero (:989) — ¿la reparto entre los 3 o dejo hablar solo al violinista?
