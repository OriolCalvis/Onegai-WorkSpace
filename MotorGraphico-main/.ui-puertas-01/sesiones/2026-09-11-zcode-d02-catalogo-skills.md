# 11/09/2026 — D-02: el catálogo de skills/spells lee lo que dicen las cartas

- **Commits:** MotorGraphico `86bf869` (7 ficheros propios; del CMakeLists solo el hunk propio).
- **Deja esperando a Oriol:** nada nuevo; CI remota pendiente como en C-11/C-16/C-20/C-22.

## Encargo y coordinación

«Coge el Gantt del MotorGraphico y sigue programando» (Oriol, 10/09). Con DEC-01 del mismo día (editor primero), el paquete de entrada era E-01 — pero ProjectSerializer.cpp (donde vive la serialización del mapa) acaba de ser reclamado por la sesión concurrente de codex, y un E-01 a medias crearía exactamente la pérdida de datos silenciosa que ese paquete existe para evitar. Se toma en su lugar D-02: cero solape con la zona reclamada, desbloqueado por D-01 (hecho) y DEC-02, y dependencia directa del camino editor (E-16 y G-13 dependen de D-02). Fila reclamada en TABLERO antes de tocar nada.

## Cambio

`CatalogLoader<SkillDefinition>` leía cinco campos y rellenaba el resto con defaults; las 492 skills cargaban todas idénticas. Ahora lee lo que la carta declara (DEC-02: manda el asset):

- **Nuevos campos** en `SkillDefinition`: `defense_stat` (crudo, para HUD y selectores de E-14), `cost_resource`/`cost_amount`, `effect_scaling` (crudo; tiparlo es D-11) y `range` (nuevo enum `SkillRange`, con los alias corto/medio/largo del lote antiguo). `expected_fields()` ampliado — D-01 exige declarar todo lo que se lee.
- **defenseStat → save_attribute** según GDD §5 + Ed. 3 (2.17 regla 4): defensa_mental→CAR, resistencia_fisica→CON, precisión_mágica→INT; «CA» y vacío→NONE porque la física sin prefijo se defiende en enfrentada con el mayor de DES/CON (SkillExecutor ya lo hacía así). El desaparellamiento saveAttribute/defenseStat es justo el que la prueba de D-10 quiere cazar.
- **Magnitudes por grado** en las dos formas del asset: array [B,P,S,C] y objeto {botch,partial,success,critical}. Las 9 utilitarias narrativas llevan el objeto a cero (un check no hace daño) y caían en silencio al fallback de daño {0,2,4,8}. Sin declaración (null en 483/492 skills, 210/210 spells y también en las cartas madre de dndWeebCC — el campo lo añade el exportador), la base se deriva del tier de la propia carta: **base = 2+2·tier**, que reproduce la escalera de `MAGNITUDE_PRESETS` (el mapeo legacy-dados del GDD §7.6 regla 4). Tier 1 queda EXACTAMENTE como antes: los números plantados de la campaña no cambian. El valor explícito de la carta siempre gana a la derivación.
- **castingStat vacío** (424 cartas marciales) pasa de INT a DES (stat del ataque básico, GDD §7.1). Verificado que ningún contenido publicado depende del default viejo: los skillCheck de las 9 aventuras usan solo utilitarias con stat explícito.

## Verificación

- `test_catalog_content` **5/5** (nuevo, en CTest con etiqueta gl-free): tres cartas concretas con sus valores escritos en el test — `abrazo_de_brasas` (t1, resistencia_fisica→CON, melee, cost none/0, descanso_corto, {0,2,4,8}), `itx_tejedora_de_mareas_ofensiva_t5` (t5, CA→NONE, medium, {0,6,12,24}) y `percepcion` (objeto a cero, {0,0,0,0}) — más `alba_anunciar_el_dia` (hechizo CAR/self/activo) y la propiedad del paquete sobre el catálogo entero: 6 magnitudes SUCCESS distintas (0+4+6+8+10+12).
- **Falsabilidad por mutación** (los tres mutantes cazados y revertidos): fallback uniforme base 4 → 2 fails; forma objeto ignorada → 2 fails; defenseStat sin mapear → 1 fail en save_attribute.
- Suite gl-free **61/66**: los 5 Not Run (test_world, test_battle_state, test_game_session, test_hud_minimap, test_level_document_bridge) no compilan por `EditorState.h` en edición concurrente de la sesión de codex (`markClean()` duplicado + llave extra) — documentado, no tocado (regla 2). Críticos en verde: `test_catalog_contract` (acepta los 4 campos nuevos), `demo_catalogos_rpg` (mismos conteos), `demo_skills`, `demo_campana_boundington`, `test_boundington_tutorial`.
- Incidente propio y lección: un `git checkout --` de limpieza tras las mutaciones me borró mis cambios del loader (nunca commiteados); reaplicados y reverificados 5/5 antes del commit. Con trabajo propio sin commitear, la limpieza de mutaciones debe ir sobre copia de seguridad, no sobre git.

## Límites y trabajo preservado

- El puente de combate (`Game::Skill`/`ApplySkillEffect`) sigue construyendo definiciones esquelet con stats hardcodeados: hacerlo data-driven es G-13; los enemigos desde catálogo son D-08. D-02 garantiza que lo que el catálogo dice llega a `SkillDefinition`.
- «Condición aplicada» del criterio: el asset NO la declara machine-readable (mechanicTags ∩ conditions = ∅, verificado; vive en la prosa de description). No se inventa; el loader ya lee applyConditionId si las cartas lo traen algún día, y D-09 cablea el executor. Por DEC-02 no es deuda de contenido a rellenar a mano.
- Commit quirúrgico por índice: del `CMakeLists.txt` (que lleva trabajo en curso ajeno: EditorDocumentIO, C-17, test_city_regeneration) solo se commitea el hunk propio (`include(cmake/CatalogContentTests.cmake)`); el árbol de trabajo queda intacto para esa sesión.
- E-01 queda pendiente de que se libere ProjectSerializer; la fila del TABLERO ya no está.
