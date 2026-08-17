# LA BIBLIA del proyecto Onegai — decisiones, registro y por qué

*Última actualización: 15 de agosto de 2026 (formalización como biblia compartida: protocolo de agentes + registro de trabajo. Estado técnico: Fases C0–C4 + D0–D3, 2416 RPG catalogs, 2525 ObjectCatalog, InventoryEngine + ConditionEngine, Fractura #1 CharacterSheet integrada en Player/EnemyBrain)*

> **Para quién es esto.** Para **todo agente** (humano o IA, de cualquier herramienta) que vaya a tocar cualquiera de los cuatro proyectos, y para Oriol dentro de seis meses. No es documentación de uso: cada proyecto ya tiene la suya. Esto es **el registro compartido de las decisiones, sus motivos y el trabajo de cada sesión** — lo que no se puede reconstruir leyendo el código.
>
> **Cómo leerlo.** El PROTOCOLO (abajo) es obligatorio. La Parte 0 dice qué ha hecho cada sesión. La 1, dónde estamos. La 2, qué se decidió y por qué. La 3, las trampas que ya nos mordieron — **léela antes de tocar nada**. La 4, qué queda abierto. La 5, decisiones de canon histórico.

---

# PROTOCOLO DE AGENTES — obligatorio para todos

*Varios agentes atacan este mismo proyecto en paralelo (Claude, Codex, Cursor, ZCode…). Este fichero es el único punto de coordinación. Las reglas:*

1. **Antes de trabajar:** lee esta biblia entera (mínimo: Parte 0, 2.1 y Parte 3). Mira `git log --oneline -10` de los proyectos que vayas a tocar y el final de la Parte 0: ¿hay una sesión anterior o **en curso**?
2. **Si encuentras trabajo ajeno sin commitear** (ficheros modificados/borrados que no son tuyos): **no lo deshagas ni lo commits**. Puede ser una sesión en curso. Documéntalo en tu entrada de la Parte 0 y sigue en otra cosa, o pregunta a Oriol.
3. **Reglas de propiedad (Parte 2.1):** `canon → cartas → motor`, sin aristas de vuelta. Si tu trabajo contradice una decisión de la Parte 2, **no la cambies en silencio**: discútelo con Oriol y registra el cambio con su motivo.
4. **Al terminar cada sesión:** (a) haz commit del proyecto que hayas tocado; (b) **añade una entrada a la Parte 0** (qué, dónde, commits, verificación); (c) actualiza la Parte 1 si el estado general cambió; (d) si descubriste una trampa nueva, añádela a la Parte 3; si tomaste una decisión, a la Parte 2.
5. **La Parte 0 es append-only:** nunca edites ni borres entradas de sesiones ajenas. Corrige solo las tuyas.
6. **No resuelvas discrepancias en silencio.** Lo que no puedas resolver, documéntalo (Parte 4 o tu entrada de la Parte 0). Ver también el protocolo de canon en 5.7.
7. **Convenciones del repo:** commits en español con contexto en el cuerpo; `require()` y nunca `assert()` en demos; sin tildes/ñ en `lines`/`speaker`/`text` de JSONs de aventura (BitmapFont ASCII); `ensure_ascii=False` en generadores Python.

---

# PARTE 0 — REGISTRO DE TRABAJO (append-only, lo más nuevo arriba)

*Cada sesión deja aquí su huella. Compilado inicialmente desde los git logs de los cuatro proyectos el 15/08/2026; las entradas futuras las añade cada agente al terminar.*

| Fecha | Dónde | Qué se hizo | Commits |
|---|---|---|---|
| **17/08 (Codex)** | Software raíz | Se añade GUIA_DE_COLABORACION.md: protocolo práctico para colaborar con agentes sin acceso directo al workspace mediante archivos, fragmentos, árbol de estructura o tareas asíncronas. Respeta canon -> cartas -> motor y remite a la bitácora. | pendiente de publicar |
| **17/08 (Codex)** | Software raíz + configuración de builds | Publicado el punto de entrada visual Onegai-WorkSpace.code-workspace, tareas VS Code, extensiones recomendadas, depuración de juego y presets CMake Debug/Release para macOS y Visual Studio 2022. Verificado: CMake configura y compila demo_mundo; el demo valida 140 localizaciones y 93 niveles. Maven genera JAR puro con Java 25 y repackage desactivado. Se observó un cambio concurrente de java.version 17 a 25 en cartas: no se tocó. Test limpio queda como diagnóstico; fallan cuatro pruebas ya existentes por Spring Boot 3.2.5 sin soporte de bytecode 25 y por datos de combate modificados. | 8950d50 |
| **17/08 (Codex)** | Estructura GitHub del workspace | Publicada la migración aislada que reúne canon, cartas, motor y puente. La copia no modifica los repositorios locales ni copia metadatos .git; excluye builds, caches, entornos, papelera, temporales FUSE y recursos de terceros. Rama publicada: codex/workspace-structure. No se creó PR porque gh no está instalado en este equipo. | ae3bb13 |
| **17/08** | MotorGraphico (ZCode) | **La Senal de la Ceniza CERRADA** (huella declarada arriba y respetada; el mapa unificado de `0bec0bd` NO se tocó, solo se verificó). La aventura del año exacto del mapa (2000 b.f., EV-2000-CRUZADA01E/E y 01/O): 37 beats, 4 skillCheck Nd6, 5 PNJs anclados a canon (clan Neblinia, Gran Consejo, Corte de los Espejos Eternos, universidad teatral de Qethatos, casa Ventobelico), **3 cierres con invariante canónico** (la Primera Cruzada oriental ocurre siempre; el jugador solo decide si el este entra ciego, medio despierto o con ojos abiertos — espejo exacto de la Matanza de Boundington). Capital de Nocturnsea renombrada a **Grytoz (nombre canónico del índice de lugares)**; la propuesta "Umbrahal" queda retirada; ficha real escrita por su autor en `marcadores_pendientes.json` (quedan 11 reservas para otros). Proyecto este_norte: aventura + 17 catálogos → build **1 de 1 jugables** (93 niveles por el cierre transitivo del mundo unido). **Incidente + lección:** mi regeneración inicial revertía el terreno nuevo (`d13b39d`) en mis 55 mapas — detectado antes de commitear, reparado con checkout de HEAD + aplicación solo aditiva de los PNJs con snap a celda transitable. ⚠️ **Pendiente para el dueño de gen_este_norte (yo, próxima sesión): encadenar `remapear_suelos.py` tras cada regeneración.** Verificado: 96 niveles al 100%, mundo unido OK, 138 puertas, 5 demos + juego smoke. | `a3f824f` |
| **15/08 noche (EN CURSO)** | `MotorGraphico/assets/world_experiment/chatgpt/este_sur/` + runtime aislado | **Profundización del Este Sur**: paquete regional desarrollado hasta un diseño jugable de la fase posterior a la Migración: 24 lugares (Taurengrad por fases, caldera de Wulcain y cúpula naga completa de seis estratos), 20 facciones, 11 hitos, 7 ganchos, 3 direcciones visuales y **9 planos de asentamiento**. `world_design.md` fija biomas, economía, bucles y límites de canon; `content_hooks.json`, `visual_direction.json` y `settlement_blueprints.json` los vuelven legibles por herramientas. Se añaden **tres prototipos reales de nivel** (`es_taurengrad`, `es_dhin_thyraxion`, `es_naka_tol`), generador determinista y `demo_este_sur`; los niveles no tocan warps/mapamundi compartidos. El validador comprueba referencias internas de geografía, facciones, eventos, hooks, dirección visual y planos. Verificado: JSON, `py_compile`, `diff --check`, `demo_este_sur` (3/3 TMX+JSON) y **4 paquetes / 91 ids estables**. No se desarrollan la Corte Vampírica, Envidia ni la Era Oscura. **No commit de MotorGraphico** por cambios ajenos concurrentes de Boundington y Este Norte. | — (cambios nuevos aislados) |
| **16/08 madrugada** | MotorGraphico (ZCode) | **Este-Norte CERRADO y jugable**: 22 asentimientos (21 canónicos + capital de Nocturnsea — primero propuesta como "Umbrahal", **retirada al día siguiente a favor del nombre canónico Grytoz**, ver entrada del 17/08) + 33 interiores + catálogo propio de 155 fichas (puertas, tenderos con sabor por nación, 37 marcadores de mapamundi incluidos los del Este-Sur en landmass_2 compartido). 4 estilos por nación sobre la maquinaria de `gen_ciudad.py` sin tocarla. Warps de ida/vuelta con los 4 landmass (2/4/7/8; 8 marcadores nuevos con ids canónicos de geografia.json). `demo_este_norte` recorre los 55 niveles desde el manifiesto; `build_proyecto.py este_norte` da **1 de 1 jugables** (entrada: Guskedor). La base la barrió `a8a3a89` (junto a sus fixes de mapas — integró también mi conectividad por-TMX y respetó mi generador); el cierre es `841b4ae`. Verificado: conectividad **95/95 niveles al 100%**, 129 puertas con vuelta, 11 demos + juego smoke en verde. **Incidente registrado:** carrera de git con el agente del editor (quité su lock rancio de 9h, su commit entró al mismo tiempo, el mío falló en silencio) — resuelto con commit por pathspec. | `a8a3a89` (base), `841b4ae` (cierre) |
| **15/08 noche** | `MotorGraphico/assets/world_experiment/chatgpt/este_sur/` | **Primer contenido jugable del Este Sur**: se separan las capas de la Gran Estepa/Cueva Larga, caldera/Terraza Alta y grutas naga de los Estados posteriores. Se registran Taurengrad (8739–8716), Congreso de Wulcain (8735), la Rebelión de las Corrientes Oscuras como **variante historiográfica** (8642/8542/8300) y la Ruta de las Escamas (7891). Se añaden 7 facciones culturales y validación de geografía/culturas/facciones; `py_compile` + validador OK con **34 ids estables**. **No commit de MotorGraphico**: su índice sigue conteniendo cambios ajenos de Boundington. | — (cambios nuevos aislados) |
| **15/08 noche** | `MotorGraphico/assets/world_experiment/` + `tools/validar_mundo_experimental.py` | **Implementacion real del experimento**: cuatro paquetes regionales aislados, contrato `schema.json` y validador Python sin dependencias. ChatGPT siembra Este Sur con referencias espaciales de Tabaxi Continental, Esmua y Wulcain; Bosmurg y Ashye/Sharium quedan como fronteras compartidas pendientes. Verificado: 4 paquetes, 9 ids estables, `py_compile` y validacion OK. **No commit**: el indice de MotorGraphico contiene cambios ajenos de Boundington ya preparados por otra sesion. | — (cambios nuevos sin commitear, aislados) |
| **15/08 noche** | `Software/` (raíz) + diseño `MotorGraphico` | **Inicio operativo del experimento distribuido de Egaroth**: se confirma la división Claude/Oeste Norte, Trae/Oeste Sur, ZCode/Este Norte y ChatGPT/Este Sur. Se define que es material experimental hasta la integración y que ChatGPT será coordinador de compatibilidad, no autoridad unilateral sobre el canon. Se añade el contrato v0 de intercambio para que las regiones puedan convertirse después en datos jugables sin sobrescribir el runtime actual. | — (docs) |
| **15/08 tarde** | `Software/` (raíz) | **Formalización de esta biblia**: protocolo de agentes obligatorio + este registro; `AGENTS.md` raíz pasa de stub vacío a mandar leer esto. *Corrección posterior de esta misma entrada:* lo que anoté como "3 ficheros borrados sin commitear" era **el artefacto de índice sucio** (trampa de la Parte 3) — los tres ficheros existen en disco, verificado. Suite verde tras `935653d`. | — (docs) |
| **15/08 mañana** | MotorGraphico | **Boundington deja de ser una hoja de cálculo**: ciudad generada desde el canon (avenidas con carriles, Casco Antiguo excavado no trazado, tendederos, Pico Dragón sin calles; SEED 20250812, flood fill garantiza alcanzabilidad). Bitácora ampliada: secciones 2.9–2.15 documentando las Fases C/D (que venían del snapshot inicial), Parte 3 con trampas C/D, Parte 5 (canon histórico) y proto-protocolo 5.7. | `935653d` |
| **13/08 tarde** | MotorGraphico | **Campaña jugable con ventana** (`./juego`): Application carga prologo+Día1 (y luego Día2+Ocaso); NarrativeState vive en Application y sobrevive transiciones; catálogo Nd6 + stats reales del Player (CON3/DES3/INT2/CAR2); HUD muestra el beat activo (`dialogueLines()`) no la cola del log; cuadro de diálogo maxLines 4→6; máquina de transiciones prólogo→D1 (recarga nivel) →D2 (sin recarga, beat `auto`) →Ocaso (recarga). Smoke test + 10 demos PASS. | `18bad1c`, `0b5c70d` |
| **12/08 madrugada** | MotorGraphico | **Puente Nd6↔NarrativeEngine** (`skillCheck` diferido, 4 grados→4 flags) + 9 skills utilitarias en `skills.json` (`gen_skills_utilitarias.py`); **prólogo onírico** (Cap. 1–2 del canon, tiradas Percepción/Conocimiento Arcano); **campaña completa**: Día 2 + Ocaso, 4 guiones / 72 beats / 11 recorridos; `startBattle` narrativo contra BattleState real; fix `copiar_assets` (CMake copiaba al configurar, no al compilar). | `de5273d`, `a8e4e66`, `20e7042`, `ad3f498`, `ac5cd84` |
| **12/08 inicio** | MotorGraphico | Egaroth en el motor (B24: `locations.json`, 19 naciones/95 ciudades/26 zonas); **Día 1 de Boundington** (primera aventura ramificada, 3 finales según las pruebas, la Matanza es ineludible); 4 fixes del exportador (ids prefijados que rompían 1.910 referencias). | `f550b3f`, `aae30a5`, `ecc539b` |
| **12/08** | dndWeebCC | Las 10 deidades tenían nombre de Creador sobre contenido ajeno: corrección id/nombre/fichero sin tocar dominio/hechizos (0/17→10/17 resueltas); fixes de referencias renombradas en Bastrea. | `1911689`, `151cee1` |
| **11/08 noche** | MotorGraphico | `git init` + snapshot inicial "antes de la integración Onegai-Core" — **ya contenía las Fases C/D** (catálogos tipados, InventoryEngine, ConditionEngine, CharacterSheet dual-mode, demo_catalogos_rpg). Tools pasan a paths relativos (`Path(__file__)`), nunca `/Users/admin/`. | `2f801c5`, `8078c0d` |
| **11–12/08** | Onegai-Core | **El puente** (sin git): análisis de los tres proyectos y sus 5 fracturas; atlas/mapamundi (rejilla canónica 2000 b.f., SVG, 19 fichas); `aplicar_canon_al_mapa.py`; inventario sistema-vs-ejemplo (1.858/950 por cierre transitivo). | `ONEGAI_CORE_Analisis_y_Plan.md`, `Onegai-Core/` |
| **11/08** | Onegai 2 | `git init` + snapshot inicial del vault. Reorganizado después **sin commitear**: cronología/índices/mapa ahora en `Onegai/Referència/`, hay un `_to_delete/` sin vaciar. | `22a7087` |
| **05/08** | dndWeebCC | Pantalla de dependencias y validación de aventuras (WS-E, E11). | `a033e9e` |
| **31/07** | dndWeebCC | Motor de combate completo (WS-D, D1–D11); ValidadorAventuraService (E10). | `526baae`, `7996422` |
| **28/07** | dndWeebCC | Migración ES→EN de cartas + constructor de aventuras + nuevas plantillas; guía de git-flow adaptado. | `d213895`, `387457e` |
| **19–23/07** | dndWeebCC | Estado base ONEGAI (sistema de cartas y tiers); wireframe; modularización JS del mapa (13 módulos); retira ventanas flotantes por paneles overlay; lote de prueba Itx + generador de habilidades por clase; blindaje con copia de seguridad automática de `data/` (Z2). | `91ba937`…`2b9eff4` (16 commits hasta `151cee1`) |

---

# PARTE 1 — DÓNDE ESTAMOS

## 1.1 Los cuatro directorios

```
Software/
├── Onegai 2/            proyecto documental — el LORE. Manda siempre.
├── dndWeebCC-master/    motor de cartas — el REGLAMENTO y la GEOMETRÍA del mapa.
├── MotorGraphico-main/  motor gráfico C++ — EJECUCIÓN. No emite nada canónico.
└── Onegai-Core/         el puente. Creado en este trabajo.
```

| | Commits | Estado |
|---|---:|---|
| `MotorGraphico-main` | 13 | rama `master`. 34 demos, 33 niveles, campaña completa jugable |
| `dndWeebCC-master` | 16 | rama `development`. 2.808 cartas |
| `Onegai 2` | 1 | solo la instantánea inicial. **Reorganizado después sin commitear**: la cronología, los índices y el mapa viven ahora en `Onegai/Referència/`, y hay un `_to_delete/` sin vaciar |
| `Onegai-Core` | sin git | 6 scripts puente + atlas + informes |

## 1.2 Qué funciona hoy

**La campaña "Los Perdidos de Boundington" está completa y es jugable**: prólogo + tres días, 4 guiones, 72 beats. Corre en consola (`demo_campana_boundington`) y con ventana (`./juego`, ENTER avanza los beats).

```
demo_campana_boundington   72/72 beats disparados, cero contenido muerto
demo_narrativa_combate     startBattle contra BattleState real
demo_prologo               17 beats con tiradas Nd6
demo_boundington           el primer día, tres finales
demo_mundo                 19 naciones, 95 ciudades, 26 zonas cargadas en C++
conectividad               33 niveles, todos 100% alcanzables
validar_enlaces            41 puertas, todas con vuelta
validar_catalogos_rpg      2.652 entradas, sin fallos
demo_catalogos_rpg         2416 RPG catalogs tipados + 2525 ObjectCatalog
demo_skills                SkillSet + ApplySkillEffect con parche legacy API 4→2 args
demo_enemy_brain           populate_from_monster_catalog E2E (Fractura #1)
demo_battle_nd6            Nd6 contra BattleState: win-rate ≥50% jugador
```

**Motor Nd6 Onegai Ed.2** (implementado en C++ dentro de `MotorGraphico/include/RPG/*`):
- **6 tiers 0..5** (TierRules::kDefaultTiers): cada tier define healthMult, statMult, rarityRank límite `maxEquipRarity` (string) y coste habilidades.
- **DicePoolEngine**: 4 grados Botch/Pass/Success/Critical; magnitudes cerradas por la regla.
- **RandomEngine / IRng unificados**: `Xoroshiro128p` reproducible, sin singleton global.
- **SkillDefinitionEnums**: ~500 habilidades + 210 hechizos parseados desde `assets/catalogs/`.
- **CharacterSheet autoritativo**: no hay stats duplicados en Player/Enemy; estos leen vía `RPG::CharacterSheet* m_sheet` (dual mode: si `m_sheet == nullptr` → fallback legacy).
- **InventoryEngine stateless**: equip/unequip, reglas peso (light/medium/heavy), símbolos compatibles, tier-rarity límite, 2h colisiona OffHand.
- **ConditionEngine stateless**: 3 stacking modes NONE / DURATION / STACK_INTENSITY; tick con `damagePerRound * stacks`; `breaksOnDamage`; recalc modifiers.
- **20 catalogs tipados `RPG::Catalogs::Catalog<T>`** (Skills, Spells, Classes, Races, Backgrounds 50, Passives, Feats, Traits, Deities 10, Conditions 15, Equipment 413, Consumables 31, Monsters 432, Summons 201, Mounts 50, Traps 51, LootTables 201, NPCs 12, Adventures 78, Events 1) — todos satisfacen `ICatalog<T>` común con `has/find/size`.
- **ObjectCatalog** (Render/Mapa): 2525 objetos en `assets/objects/libreria_completa.json` + 13 JSONs parciales. Derivan de dndWeebCC (carpetas `cartas/*` + carpetas raíz `loot npcs personatges aventuras eventos kits/kits_iniciales.json → 50 kit_bg_*`).

## 1.3 Cómo se ejecuta

```bash
cd "…/MotorGraphico-main/MotorGraphico/build"
cmake --build . --target demo_campana_boundington -- -j8 && ./demo_campana_boundington
cmake --build . --target juego -- -j8 && ./juego     # con ventana
```

Para regenerar la ciudad entera: `bash tools/regenerar_ciudad.sh` desde `MotorGraphico/`.

---

# PARTE 2 — LAS DECISIONES Y SUS MOTIVOS

## 2.1 Arquitectura: quién manda en qué

**Decisión.** Los tres proyectos no son tres productos: son **tres consumidores de un mismo activo**, el mundo de Egaroth. Cada uno manda en una capa y solo en una.

| Capa | Propietario | Cómo se resuelve un conflicto |
|---|---|---|
| Nombres, historia, panteón, quién gobierna qué | **Onegai (documental)** | Gana el corpus. Sin excepción. |
| Coordenadas, polígonos, adyacencias | **dndWeebCC** | Gana el mapa: es la única fuente que tiene el dato |
| Reglas, fórmulas, tiers, balance | **dndWeebCC** | Gana el GDD |
| Ejecución | **MotorGraphico** | No manda en nada. Es un renderizador |

**Por qué.** El diagnóstico inicial encontró que cada proyecto guardaba una copia privada y parcial del mundo, y que apenas se mencionaban entre sí: `grep -ri "Egaroth" dndWeebCC-master/data` daba **cero resultados** sobre las 2.808 cartas del catálogo. (Hoy da uno, y es el que añadió `aplicar_canon_al_mapa.py`. Los 7 de `src/main/resources/JSONS/transfons/` sí existían antes, pero no llegaban al catálogo.) Sin una regla de propiedad escrita, cada discrepancia se decide otra vez desde cero.

**Flujo, en una sola dirección:** `canon → cartas → motor`. Sin aristas de vuelta. Si el motor necesita un dato que no existe aguas arriba, es un bug de contenido, no de motor.

## 2.2 El material de ejemplo es desechable

**Decisión de Oriol:** el micro-setting de Llerba y la Tomba (16 facciones inventadas) *"son solo ejemplos, en su momento los borraremos y aportaremos historias canónicas solo"*.

**Consecuencia medida** (`Onegai-Core/reports/inventario_sistema_vs_ejemplo.json`):

| | Cartas |
|---|---:|
| **Sistema** — reglamento y catálogo reutilizable | **1.858** |
| **Ejemplo** — cuelga del micro-setting, se borrará | **950** |

Las 950 son: 432 enemigos, 302 historias, 134 PNJs, 73 aventuras y 1 evento. Se calculan por **cierre transitivo**: quien referencia a una carta de ejemplo, es de ejemplo.

**Por qué importa.** Sin esta separación se invierte esfuerzo en contenido que va a desaparecer. Es la razón de que **no** hayamos arreglado los 1.892 avisos del exportador ni exportado las 302 historias: casi todo eso es material de ejemplo.

⚠️ **Aviso:** existe una clase entera, `medium_de_la_tomba`, con su pasiva y 18 habilidades construidas sobre la Tomba. Si el ejemplo se borra, esa clase se va con él.

## 2.3 El mapa está fechado en 2000 b.f.

**Decisión de Oriol:** el mapa de Egaroth es del año 2000 b.f.

**Por qué es más importante de lo que parece.** 2000 b.f. es el primer año de la **ERA XIII — Las Catorce Grandes Cruzadas**, la era más extensa de la cronología (59 entradas). El mapa no describe Egaroth en general: describe Egaroth el año en que empieza la Primera Cruzada.

Y la fecha permitió una validación mucho más fuerte que la que yo había hecho. La cabecera de la ERA XIII dice que las Cruzadas corrían en **dos frentes paralelos** — occidental (Ascaria, Aegroum, Bastrea, Ecla, Udrax) y oriental (Choubar, Bosmurg, Esmua, Tabaxi, Gongorguma). Las masas de tierra calculadas de la rejilla contienen **exactamente esos dos frentes, disjuntos**.

> Las Cruzadas corrían en dos frentes paralelos **porque estaban en dos continentes separados**. La geografía explica la estructura narrativa de la era.

**Corrección registrada:** mi primera validación usó eventos de 8964, 8400 y 7816 b.f. — entre 6.000 y 7.000 años *antes* del mapa. Encajaban, pero no probaban nada.

**Consecuencia práctica:** la época viaja con el dato (`world_grid.json`, el SVG, las 19 fichas, los TMX). Si algún día hace falta un segundo mapa (el Imperio de Ascaria, el post-Cataclismo), la estructura lo admite: **mapas por época completos, nunca diffs**.

## 2.4 El mapa del motor de cartas es canon; el resto de su lore, no

**Hallazgo que corrigió mi propio análisis.** La revisión 1 concluyó "mundo paralelo" tras auditar los campos `faction` y `location`. Se le pasó `data/mapa/geografia.json`, que es donde vive el mapa, y **ese fichero sí es canon**: 19 naciones, 95 ciudades con coordenadas, 26 zonas. Las 19 naciones coinciden con el índice de lugares de Onegai.

Más aún: `Mapa_Egaroth.svg` del proyecto documental tiene **74 etiquetas de texto y cero polígonos**. El motor de cartas tenía la única representación geométrica del mundo que existía en los tres proyectos, y **no se exportaba a ninguna parte**.

**Lección para futuras auditorías:** una auditoría que solo mira los campos que espera encontrar confirma su propia hipótesis. Los validadores deben recorrer *todos* los ficheros y reportar los que no encajan en ningún esquema conocido.

## 2.5 Las 10 deidades tenían nombre de Creador sobre contenido ajeno

**Ninguna de las 10 cartas de deidad tenía un dominio que correspondiera a su nombre.** La carta "Chronos, Guardián del Tiempo" tenía dominio *forja, artesanía y juramentos* y diez hechizos `forja_*`.

**La prueba de que no era casualidad:** los 12 trasfondos referencian 17 deidades por id y **ninguna existía como carta**. Al cruzar dominio + prefijo de hechizo con el trasfondo que la invoca, las diez casan 1 a 1. Los pares lo cierran: el Mes del Hierro invoca a `la_forja_del_trueno` *y* a `deity_forge_father`, que son justamente las cartas de `trueno_` y `forja_`.

Se corrigieron id, nombre y fichero. **Dominio, hechizos, favor y obligaciones no se tocaron**: ese contenido siempre fue correcto, lo que estaba mal era la etiqueta. Deidades resueltas: **0/17 → 10/17**.

## 2.6 Boundington: la ciudad se genera desde el canon

**Decisión.** Cada párrafo de la descripción canónica de Boundington es, leído de otra forma, **una regla de generación distinta**.

| El canon dice | El generador hace |
|---|---|
| *"calles con carriles para peatones a los lados y espacio para carruajes en el centro"* | `avenida_h/v` con tile de carril propio |
| *"casas de piedra por fuera y madera por dentro"* | alterna materiales: una manzana no lee como bloque |
| *"cuerdas donde cuelgan la ropa"* | tendederos cruzando las callejuelas |
| *"Casco Antiguo: deteriorado y lleno de secretos"* | no se traza, **se excava**: manzana maciza + callejones que tuercen + ruinas transitables |
| *"Barrios Altos: mejor cuidadas, calles más limpias"* | parcelas de tamaños distintos con jardín, adoquín fino |
| *"Pico Dragón: sin orden alguno"* | el generador **deja de trazar calles**: suelta chabolas y lo que queda entre ellas es la calle |

**Por qué se excava en vez de construir.** Empezar lleno y vaciar da un trazado orgánico. Empezar vacío y colocar casas devuelve una retícula — que era exactamente el problema de la versión anterior.

**Determinismo.** `SEED = 20250812`. El azar decide dónde tuerce un callejón, nunca si un sitio es alcanzable: eso lo garantiza un flood fill que excava lo que quede aislado.

## 2.7 La campaña respeta el canon: Boundington cae siempre

**Decisión.** El jugador **no puede evitar la Matanza**. Ocurre, es `EV-1981-BOUNDINGTON`, y ninguna ruta puede impedirlo. Lo único que decide es **cuánta gente sobrevive**.

Los tres desenlaces reproducen la regla que el documento canónico da para el Día 2:

| Lo que averigües | Venides reúne | |
|---|---|---|
| Cera, antorchas y hierro | **20 hombres** | Punto de los Creadores |
| Solo rumores | **10 hombres** | Punto de Sho-Noco |
| Nada | **solo** | — |

`demo_campana_boundington` comprueba ese invariante en los **11 recorridos**: la ciudad arde en todos.

**Ningún personaje está inventado.** Skilla, Venides, Aigren, Luisarda, Xila, Sandes, Edul y Verina salen del documento canónico con su oficio y su papel intactos.

## 2.8 Decisiones del motor

**`startBattle` sigue el patrón de `skillCheck`, salvo en el tiempo.** El propio comentario de `NarrativeResult` ya lo anticipaba (*"aquí aparecerán openShopId, startBattleWith…"*). La diferencia crítica: una tirada Nd6 se resuelve dentro de `applyNarrative` y la flag ya está al volver; **un combate dura turnos**, así que la flag la enciende `syncBattleOutcome`. Si un beat `auto` reaccionara a `flagVictory` en el mismo tick que lanzó el combate, no dispararía nunca.

- Varios combates seguidos se **encolan** (oleadas): perder ese dato convertiría una emboscada de tres oleadas en una.
- Un `monsterId` inexistente enciende `flagDefeat` en vez de callarse. Si no, la aventura espera para siempre una flag que nadie va a poner.

**Una campaña es un `NarrativeState` compartido.** `setAdventure()` cambia el libreto; el estado es del jugador, no de la aventura. El motor ya lo permitía — nadie lo había probado, y por eso `beat_venides_reconoce` era inalcanzable.

**`LocationDefinition` cierra B24.** Las tres clases (nación/ciudad/zona) van en un tipo con discriminador `kind`, con el mismo criterio que `ObjectCategory`: quien pregunta "qué hay en Udrax" no quiere saber de qué clase es cada cosa.

**Los ids explícitos no se prefijan.** El exportador prefijaba *siempre* (`skill_`, `monster_`…), renombrando el 86% de los ids y dejando **1.910 referencias cruzadas apuntando al vacío**. Si el autor puso un id, ese id es la identidad. Solo hay tres colisiones reales entre catálogos y se desempatan a mano.

## 2.9 Dos catálogos paralelos, NO uno solo — RPG tipado vs ObjectCatalog de Render

**Decisión.** El mismo dataset dndWeebCC (2808 cartas) se representa en el motor **en DOS formatos distintos**, cada uno con una responsabilidad. NO se intenta unificar en un "super-catalog."

| Catálogo | Propietario | Formato | Uso |
|---|---|---|---|
| `RPG::Catalogs::*` — 20 tipados (`SkillCatalog`, `MonsterCatalog`…) | `include/RPG/*` | struct Definition con campos fuertemente tipados | Lógica de combate / resolución Nd6 / Inventory / Condition / SkillEffect. `find()` devuelve `const T*` (puntero nulo si no existe). |
| `ObjectCatalog` (`Render/ObjectCatalog.h`) | `include/Render/*` | `ObjectDefinition` plano: `id/category/isPickup/isBlocking/position/isMerchant/…` | Render/mundo: sprites, pickups, interacción, NPCs. Carga desde JSONs de `assets/objects/`. |

**Por qué.** C4/Fase B mostró que la unificación es imposible sin romper uno de los dos lados:
- El monstruo "Acolito de Penumbra" en `RPG::MonsterCatalog` necesita stats[4], maxHealth, 18 campos extra (attacks/phases/skillIds…) para IA/combate.
- El MISMO monstruo en ObjectCatalog es `{category:"enemy", isBlocking:true, isHostile:true, monsterId:"monster_acolito_de_penumbra", sprite:-1}` para colocarle en TileMap.

Unificar → `ObjectDefinition` acaba con 40 campos opcionales. Pérdida. Las búsquedas cruzadas van por `monsterId` / `equipmentId` string referenciando al otro catálogo — simple y verificable.

## 2.10 Fractura #1 — CharacterSheet único: `m_sheet*` puntero externo DUAL-MODE

**Regla de ARCHITECTURE.md** (Oriol): 1 concepto = 1 representación. Aplicado al PJ/Enemigo = **`CharacterSheet` único autoritativo**. NUNCA se duplican stats en `Player.h` / `EnemyBrain.h`.

**Implementación dual-mode (backward-compatible con TODOS los demos antiguos):**
```
struct Player / EnemyBrain {
  RPG::CharacterSheet* m_sheet = nullptr;  // NO propietario; externo
  // legacy fallback (used solo si m_sheet==nullptr):
  int m_health, m_maxHealth, m_tier;
  std::array<int,4> m_stats;
  std::string m_combatant_id;
};
```
- `stat(Stat s)`: `if (m_sheet) return m_sheet->stat(s); else return m_stats[idx];`
- `maxHealth()`: `if (m_sheet) return m_sheet->healthCap(); else return m_maxHealth;`
- `tier()`, `combatant_id()`, `defenses()` — MISMA lógica dual.

**Por qué dual-mode y no borrar legacy:** `demo_battle_nd6`, `demo_skills`, `demo_enemy_sprite` existían SIN CharacterSheet. Exigir `m_sheet != nullptr` rompía 11 ejecutables. Con dual-mode los 34 demos siguen pasando y el código nuevo puede usar `set_character_sheet()` para activar la fuente única.

**Llamada crítica para C4:** `EnemyBrain::populate_from_monster_catalog(mon, tierRules*, defaultClass*)` SOBREESCRIBE tanto legacy como sheet al mismo tiempo (para que los lectores sin refactoring vean el mismo dato):
```cpp
m_health = m_maxHealth = mon.maxHealth ? mon.maxHealth : 1;
m_stats = mon.stats; m_tier = clamp(mon.tier,1..5); m_combatant_id = mon.id;
if (m_sheet) {
  sheet->id/displayName/classId=role/tier/baseStats=mon.stats; equipBonuses=0; equipArmor=0
  sheet->knownSkillIds = mon.skillIds;  push passiveId
  sheet->recompute_derived(*tierRules, defaultClass);   // ← cached HP + defenses
  m_maxHealth = sheet->healthCap(); m_health = ratio(ratio)? healthCap : healthCap;
}
```

## 2.11 `CharacterSheet::recompute_derived()` = valores cacheados SIN argumentos para ICombatant

**Problema.** `CharacterSheet::healthCap(rules, cls)` necesita `TierRules&` y `ClassDefinition*` (toma tier de class y formula TierRules::healthFormula). Pero `ICombatant::maxHealth()` — la interfaz que usa BattleState — **NO toma args**. No se puede pasar rules/cls ahí.

**Decisión.** Añadir campos mutables cacheados + 1 paso de recompute antes de entrar en combate:
```cpp
struct CharacterSheet {
  mutable int cachedHealthCap = 0;
  mutable DefenseBlock cachedDefenses;
  void recompute_derived(const TierRules& rules, const ClassDefinition* cls) const {
    cachedDefenses = defenses(cls);
    cachedHealthCap = healthCap(rules, cls);
  }
  int healthCap() const { return cachedHealthCap; }                // sin args
  const DefenseBlock& defenseValues() const { return cachedDefenses; }
};
```
- Se llama EXPLÍCITAMENTE una vez justo después de setear `classId`/`tier`/`baseStats`/`equipBonuses`.
- No hay invalidez automática. El caller debe volver a `recompute_derived()` tras `equip()`/`unequip()`/`level_up()`.
- Order n = 1 al resolver: NO hay un getter que recalcula sobre la marcha cada vez (evita calcular healthCap 40 veces por turno).

## 2.12 ConditionEngine: 3 modos de stacking + damage = damagePerRound × stacks

**Decisión.** Cada `ConditionDefinition` declara `enum class Stacking { NONE, DURATION, STACK_INTENSITY }` y su `defaultRounds` / `maxStacks` / `damagePerRound`. `ConditionEngine::apply()` implementa la semántica correcta POR DEFECTO:

| Mode | 2ª apply idéntica hace |
|---|---|
| NONE | Borra la anterior y pone 1 stack nuevo = reemplazo (ej: aturdimiento). |
| DURATION | Suma o refresca `roundsRemaining` pero `stacks` no cambia (ej: cansancio ligero). |
| STACK_INTENSITY | +1 stack hasta `maxStacks`. `damagePerRound` es POR STACK (ej: sangrado x5 = 10 damage/tick). |

`ConditionEngine::tick_conditions()` NO resta HP directamente (CharacterSheet no tiene HP actual; BattleState lo maneja). Devuelve `std::vector<TickEvent>{ conditionId, damageInflicted, expired, stacksRemaining }`. Quien llama (BattleState) suma el daño a `health()`.

## 2.13 InventoryEngine usa `rarityRank(string)` para TierRules.maxEquipRarity (NO enum)

**Trampa que mordió 2 veces.** `TierRule.maxEquipRarity` — field en JSON — guarda un string `common/uncommon/rare/epic/legendary` (escritores humanos lo pusieron como palabra). NO es `ItemRarity enum`. Hacer `static_cast<int>(t.maxEquipRarity)` devolvía basura.

**Decisión.** Función helper en TierRules.h:
```cpp
int rarityRank(const std::string& s);
// retorna 0..4 = common..legendary, -1 si desconocido
// -1 = SIN limite (el writer de la clase no lo rellenó).
```
`InventoryEngine::check_rarity(tierRules, tier, rarity)`: si `rarityRank(maxEquipRarity) == -1` → pasa; si no, `rarityRank(rarity) <= rarityRank(maxEquipRarity)`.

Mismo tratamiento case-insensitive + eng/spa para peso: `ligera/light/ligth/lig` → categoría Light; `medi/mediana/medium` → Medium; `pesada/heavy/pesad` → Heavy.

## 2.14 Two-handed: colisión MainHand ↔ OffHand

Regla cerrada para no tener un slot fantasma "2HAND":
- Al equipar con `slot == "TwoHand"` o con `slot_name` = `"MainHand"` + `two_handed == true`: ocupa BOTH slots. OffHand queda "mismo id".
- Al equipar OffHand sin 2h y MainHand ya = 2h → error `"MainHand contiene arma a dos manos"`.
- Al desequipar CUALQUIERA de los dos de un 2h → se desequipa BOTH y los bonuses se restan UNA VEZ (no doble resta).

## 2.15 `convertir_libreria_dnd.py` usa paths RELATIVOS a SCRIPT_DIR (nunca /Users/admin/)

**Decisión.** El script deriva paths desde `Path(__file__).resolve().parent`:
```
SCRIPT_DIR    = MotorGraphico/tools/
MOTOR_DIR     = SCRIPT_DIR.parent                    = MotorGraphico/
REPO_DIR      = MOTOR_DIR.parent                     = MotorGraphico-main/
RAIZ_SOFTWARE = REPO_DIR.parent                      = Software/
  BASE_DND      = RAIZ_SOFTWARE / "dndWeebCC-master/data/cartas"
  BASE_DND_RAIZ = RAIZ_SOFTWARE / "dndWeebCC-master/data"
  BASE_SALIDA   = MOTOR_DIR / "assets/objects"
```
- Verificación TEMPRANA: `if (!BASE_DND.is_dir()) sys.exit(1)` con árbol ASCII printado a stderr.
- `BASE_SALIDA.mkdir(parents=True, exist_ok=True)` (no falla si alguien borra assets/objects/).

Carpetas raíz NO-cartas detectadas robustamente (D0): primero nombre correcto español → fallback catalán/inglés:
- `loot/` (no `loot_tables/`), `npcs/` y `personatges/` (ambas), `aventuras/` (no `aventures/`), `eventos/` (no `events/`), `kits/kits_iniciales.json → 50 kit_bg_*` backgrounds.

## 2.16 Experimento: 4 IAs paralelas CREAN el MUNDO ENTERO de EGAROTH (año 2000 b.f., ERA XIII)

**Decisión de Oriol (15 ago 2026):** Pilotaje extremo de escalabilidad — 4 modelos distintos generan, en paralelo, las 4 macro-regiones de Egaroth **tomando como base la era histórica ERA XIII (2000 b.f. = año del mapa canónico)**. Cada IA produce **ficheros JSON compatibles con `demo_mundo` del MotorGraphico (locations.json)** + un README.md/lore por subregión. Después se compara: coherencia lore, nivel de detalle, errores de continuidad, y si se puede ensamblar el mapa completo **SIN conflictos entre regiones**.

### Reparto (por 4 puntos cardinales ~ balance ciudad = 95/4 ≈ 23-24 ciudades/IA)

| # | IA | Región | Naciones existentes (Atlas 19) | Ciudades objetivo |
|---|---|---|---|---|
| 01 | CLOUDE | **OCCIDENTE SEPTENTRIONAL** (frente occidental Cruzadas: NORTE) | Ascaria, Aegroum, Bastrea → ~30 ciudades → se pasa al siguiente si supera 25 | ~24 |
| 02 | TRAE | **OCCIDENTE MERIDIONAL** (frente occidental Cruzadas: SUR) | **Ecla, Udrax** + si sobran ciudades → picos fronterizos Ascaria del sur | ~24 |
| 03 | ZCODE | **ORIENTE SEPTENTRIONAL** (frente oriental Cruzadas: NORTE) | Choubar, Bosmurg, Esmua | ~24 |
| 04 | CHATGPT | **ORIENTE MERIDIONAL** (frente oriental Cruzadas: SUR) | Tabaxi, Gongorguma + islas marinas | ~24 |

### Contrato OBLIGATORIO que DEBEN cumplir las 4 IAs (documentado en EGAROTH_2000BF/README.md)

1. **Fecha fija**: TODO contenido describe **año 2000 b.f.** (inicio ERA XIII). Ninguna referencia a eventos posteriores a EV-1995-ASALTONUBES (5 años después).
2. **Coordenadas canónicas**: (x,y) en la rejilla 200×150 del `world_grid.json` (Onegai-Core). Si la ciudad ya está en el Atlas, **su (x,y) es canónica y NO se toca**. Si es ciudad NUEVA (antes "Aldea IX"), marcar como `origin: "[IA] TRAE/CLOUDE/ZCODE/CHATGPT"` y no pisar coordenadas ocupadas.
3. **Tier de amenaza**: por nación/ciudad un `tierRange: {min:1, max:5}` (coherente con RPG/TierRules). Frontera Cruzadas tier≥3, capitales tier≥4, pueblos mineros tier=2.
4. **Formato NIVEL 2 (obligatorio mínimo):**
   - Por NACIÓN: nombre, capital, gobierno, bioma general, `eraXIIIEvents[]` al menos 1 relacionado con Cruzadas, `factions[]` ≥3, `dungeons[]` ≥1 (mazmorra tier), lore 3 párrafos.
   - Por CIUDAD: `id`, `name`, `kind="city"`, `nationId`, `coord:{x,y}`, `population`, `primaryActivity`, `faction`, `tier`, `pnjPrincipal{name,role,hook}`, `quests[]` ≥1 hook simple.
5. **Formato de entrega por IA (carpeta `EGAROTH_2000BF/0[1-4]_[IA]_[REGION]/`):**
   - `00_[IA]_[REGION]_README.md` → resumen lore
   - `05_naciones.json` → array de LocationDefinition (tipo nación)
   - `10_ciudades.json`  → array de LocationDefinition (tipo ciudad)
   - `15_zona_mazmorras.json` → array de LocationDefinition zonas + `dungeons[]`
   - `20_catalogo_objetos_unicos.json` → **OPCIONAL**: 5-10 armas/armaduras/reliquias NUEVAS únicas de la zona compatibles EquipmentCatalog JSON (`assets/catalogs/equipment.json` schema).
6. **NO inventar Deidades, razas inteligentes nuevas o clases nuevas.** Se usan las 17 deidades + 43 razas + 61 clases existentes en catalogs tipados. Si falta un dios local, marcar `[TBD Oriol]`.
7. **[MOTORGRAPHICO COMPATIBILIDAD]** Los JSON `naciones.json` + `ciudades.json` DEBEN cargar sin error contra el `demo_mundo` actual. Campo `kind` válidos: `"nation"` / `"city"` / `"zone"` (mismo que `LocationDefinition` del motor). Las extensiones lore van en `metadata: {...}` libre.

**Por qué esta experiencia.** Hasta ahora el equipo trabaja con 1 agente cada vez. Con 4 IAs generando el mismo mundo al mismo tiempo medimos (a) **tamaño de brecha de continuidad**, (b) **gramaje lore útil**, (c) **compatibilidad automática con loaders motor**, y (d) **dificultad ensamblar fronteras entre regiones adyacentes** (la frontera Ascaria-sur/Ecla-norte es la prueba del algodón entre CLOUDE y TRAE). Si este piloto funciona, el siguiente paso es generar 100% el mundo con 200+ ciudades, mazmorras y quests hooks — toda la geometría al menos, y desde Oriol añadir lore específico más fino.

---

# PARTE 3 — TRAMPAS CONOCIDAS

> Todas mordieron durante este trabajo. Léelas antes de tocar nada.

### El parser JSON del motor no decodifica `\uXXXX`

Es una decisión deliberada y documentada en `JsonValue.cpp`. Y `json.dump` de Python escapa así cualquier no-ASCII **por defecto**. Una raya larga (`—`) en el nombre de un barrio bastaba para que el nivel dejara de cargar, con el error inútil *"loadFromFile devolvió error"*.

→ Los generadores usan `ensure_ascii=False` y los títulos llevan un `assert` de ASCII puro.

### `file(COPY ...)` de CMake corre al CONFIGURAR, no al compilar

Quien añadía un asset y hacía `cmake --build .` se encontraba con que `build/assets/` seguía sin el fichero. Le faltaban el Día 2, el Ocaso y el catálogo de enemigos.

→ Hay un target `copiar_assets` (ALL) del que dependen los 7 demos que leen assets.

### El orden de los generadores de ciudad importa y no estaba escrito

`gen_ciudad` reescribe los niveles desde cero y se lleva por delante puertas, carteles y PNJs. Eso hizo fallar a `demo_ciudad` a mitad del trabajo.

→ **Usa siempre `tools/regenerar_ciudad.sh`**, nunca los scripts sueltos. El orden es: tileset → ciudad → interiores → carteles → ambientar.

### Los callejones derivan y se comen la muralla

Un callejón de 62 pasos con 30% de deriva acaba pisando el borde. Abría un boquetón en una ciudad amurallada **sin que nada lo delatara**: el nivel carga y te sales del mapa.

→ La deriva se recorta al interior y `escribir()` comprueba el perímetro entero.

### Un PNJ en celda válida puede estar en el barrio equivocado

Al rehacer la ciudad, Xila y la niña quedaron dentro del Barrio Militar y el sectario en el Distrito Comercial. Seguían siendo celdas transitables, así que nada fallaba — pero el borracho del arrabal predicaba en el cuartel.

→ `ambientar_boundington.py` comprueba transitabilidad, pero **no** semántica de barrio. Si tocas `gen_ciudad.py`, revisa `ANADIDOS` a mano.

### Los validadores con listas escritas a mano dejan de validar

`conectividad.py` tenía las tres ciudades escritas a mano: cualquier nivel nuevo quedaba sin comprobar y nada lo decía. Pasó de cubrir 3 niveles a cubrir 33.

### `assert()` no evalúa en Release

Está documentado en `examples/Check.h` y afectaba a los 20 demos del repo. **Usa `require()`**, nunca `assert()`, en cualquier demo que haga de test.

### Git en este entorno

`MotorGraphico` y `Onegai 2` no tenían control de versiones; se inicializaron el 11/08. El sandbox donde trabaja el agente **no puede borrar ficheros** en la carpeta montada (solo crear, escribir y renombrar), lo que deja `.git/index.lock` colgados. Los commits se hacen con `git commit-tree` + escribir `refs/heads/<rama>` a mano.

### El índice de git queda sucio: ficheros "D" y "??" A LA VEZ

Ha pasado ya **tres veces** (12/08, 13/08, 15/08): el `git status` muestra el mismo fichero como borrado del índice (`D`) Y como untracked (`??`), con decenas de `MM` cuyo contenido en disco es idéntico a HEAD. Asusta ("¿se ha perdido trabajo?") y contamina cualquier diff que quieras hacer.

→ **Arreglo:** `git add -A` resincroniza el índice con el disco y el status queda limpio. **Antes de asustarte, verifica** que el disco coincide con HEAD: `diff <(git show HEAD:fichero) fichero` — hasta ahora nunca se había perdido nada. Causa probable: el sandbox que no puede borrar (ver trampa anterior) interrumpiendo un `git rm`/`reset` a medias.

### Los duplicados " 2" de macOS se cuelan en los commits

El Finder (o el sandbox) crea copias `fichero 2.ext` al duplicar, y acaban committed sin que nadie las mire. Pasó en `de5273d` (`demo_narrativa_nd6 2.cpp`, `gen_skills_utilitarias 2.py`).

→ **Antes de cada commit:** `find . -name "* 2*" -not -path "./.git/*"` y borra lo que salga. Y si el `build/` tiene `assets 2/`, `CMakeFiles 2/`, `CMakeCache 2.txt`: es la misma basura; un `rm -rf build && cmake -S . -B build` la elimina (ha confundido diagnósticos de segfault).

---

### NUEVAS (Fases C/D, Julio/Agosto 2026) — Las que mordieron en la conversión dndWeebCC → C++

Todas han mordido al menos UNA vez y generaron build-errors. LÉERLAS si tocar `include/RPG/*` o el script de conversión.

### Los catálogos son `RPG::Catalogs::EquipmentCatalog`, NUNCA `RPG::EquipmentCatalog`

Error repetido 5 veces hasta que se pilló. `EquipmentCatalog` es un `typedef` en `namespace RPG::Catalogs {}` — NO llega al padre. Si el compilador dice `no type named 'EquipmentCatalog' in namespace 'RPG'` — falta `Catalogs::` y punto.

Correcto:
```cpp
RPG::Catalogs::EquipmentCatalog eq;    // ok
const RPG::EquipmentDefinition* d = RPG::Catalogs::EquipmentCatalog::find(...);  // NO, es un MÉTODO de la instancia
```

### `Catalog<T>::find()` retorna `const T*` (puntero nulo si no existe), NUNCA `Result<T>`

Mismo uso que `CharacterSheet.stat()` pero la confusión fue alta porque `loadFromFile()` sí retorna `Result<int>`. 3 veces se escribió:
```cpp
auto r = cat.find(id);
if (!r.isOk()) ...    // MAL: find retorna const T*, no Result
```
Correcto:
```cpp
const EquipmentDefinition* r = cat.find(id);
if (!r) continue;
r->caBonus;  // OK
```

### `Catalog<T>` NO tiene `begin()/end()` — usa `forEach(std::function<void(const T&)>)`

Están implementados sobre `unordered_map<string, unique_ptr<T>>` sin exponer iteradores STL. `for (auto& x : cat) → no compila`.

Correcto:
```cpp
monsterCat.forEach([&](const RPG::MonsterDefinition& m) { if (!mon && m.maxHealth>0) mon = &m; });
```

### Orden de `RPG::Stat` es **CON/DES/INT/CAR** — NO FUE/DES/CON/INT/CAR de D&D 5e

Aplica a `std::array<int, 4>` de MonsterDefinition, ClassDefinition, EnemyBrain.stats, CharacterSheet.baseStats, equipBonuses, conditionModifiers. Siempre `[0]=CON [1]=DES [2]=INT [3]=CAR`. Si en una tabla esperabas "fuerza 18" — no existe. `EquipBonuses` y `ConditionModifiers` usan el MISMO orden.

Acceso correcto:
```cpp
case RPG::Stat::CON: return m_stats[0];
case RPG::Stat::DES: return m_stats[1];
case RPG::Stat::INT: return m_stats[2];
case RPG::Stat::CAR: return m_stats[3];
```

### `RPG::DefenseBlock` es un `std::array<int, DEFENSE_COUNT> values` — NO tiene campos `.ca / .resFis / .defMental / .precMag`

En PLAYER.h se intentó acceder: `d.ca` → error "no member". Siempre es `values[0] = CA, values[1] = save_física (10+CON), values[2] = save_vol (10+INT), values[3] = DC hechizo (10+CAR)`. Los helpers existen: `b.ca_fisica() / save_fis() / save_vol() / dc_mag()`. Si `CharacterSheet::defenseValues()` devuelve un `RPG::DefenseBlock` y un `ICombatant::DefenseBlock` — **ambos structs son layout idénticos** porque ambos son `std::array<int,4>` — basta `return sheet->defenseValues();` sin convertir.

### `MonsterDefinition.name` NUNCA `.displayName`

CharacterSheet tiene `displayName`. MonsterDefinition / ClassDefinition tienen `.name`. `for_each` sobre los catálogos printando siempre falla si mezclas.

### EnemyBrain constructor SIMPLE: (pos, patMin, patMax, maxHealth, stepInterval)

**NO** toma `(id, stats, tier)` como yo asumí 3 veces. Para inicializarlo desde MonsterDefinition usa `populate_from_monster_catalog()` — que rellena TODOS los campos al mismo tiempo (legacy + sheet):
```cpp
EnemyBrain brain(pos, {8,3}, {12,7}, 1, 0.5f);
brain.set_character_sheet(&sheet);
brain.populate_from_monster_catalog(*mon, &tierRules, defaultClass);
```

### NUNCA hardcodear paths `/Users/admin/...` en scripts Python. Siempre `Path(__file__).resolve()`

Causó que `BASE_DND/BASE_DND_RAIZ/BASE_SALIDA` apuntaran a rutas que no existen en portátil/clon. Solución fija en 2.15.

### `ConditionDefinition.defaultRounds` / `.damagePerRound` — NUNCA `durationRounds` / `.dotMagnitude`

Los campos de `RpgCoreDefinitions.h` se renombraron pero el nombre viejo se quedó en mi cabeza. Se usó el antiguo 2 veces y rompió el build.

### `Result.h` vive en `Core/Errors/Result.h`, NO en `RPG/Result.h`

Lo correcto en InventoryEngine.h y ConditionEngine.h es: `#include "Core/Errors/Result.h"`.

### `TierRule.maxEquipRarity` es un STRING (common/...) — no `static_cast<int>`

Esto mordió 2 veces. Usa `rarityRank(const std::string&)`, retorna -1 = sin límite. Ver 2.13.

### En los mapamundis (mundi_landmass_*), el MAR es transitable y la TIERRA colisiona

El tileset de dos tiles invierte la intuición: gid 1 (lo que parece mar) no lleva propiedad `collision` y gid 2 (la masa de tierra del checker) sí. Y las dimensiones engañan: landmass_2 es 72×144, pero el _4 son 32×16, el _7 16×16 y el _8 8×16 — con anclas copiadas de otro mapa te sales de rango sin error hasta que revienta el índice.

→ La colisión se lee del propio TMX (como en `wire_este_norte.cargar_tmx`), y las posiciones se hacen *snap* a la celda transitable más cercana, nunca a ciegas.

### Un script de doble enlace (ida y vuelta) no es idempotente si solo mira la ida

`wire_este_norte` marcaba "ya enlazado" si el objeto del landmass tenía `targetLevel` y saltaba entero — dejando de reponer la VUELTA en niveles regenerados desde cero (salidas muertas tras cada regen).

→ Regla: el enlace hacia lo regenerable (mis niveles) se repone SIEMPRE; el que puede haber puesto otro agente (landmass compartido) solo se escribe si falta.

---

# PARTE 4 — QUÉ QUEDA ABIERTO

## 4.1 Deuda de contenido (lo más valioso)

| | Medido |
|---|---|
| **Hechizos huérfanos** | **202 de 210** no pertenecen a ninguna clase. Escritos, en disco, inalcanzables en juego |
| **Clases sin hechizos** | **59 de 61** tienen `spells: []`, incluido el Apóstata, cuya descripción promete *"canalizar hechizos devastadores"* |
| **Deidades locales que faltan** | 7: `la_madre_de_los_doce` (la más citada, en 3 trasfondos), `el_hermano_mendigo`, `el_sepulturero_amable`, `la_senora_de_las_mareas`, `la_luna_hueca`, `el_toro_de_gongorguma`, `el_leon_del_alba` |
| **Los Creadores de Egaroth** | Ninguno existe como carta. Egaroth, Chronos, Sofía, Gurkazaal… son otro nivel de divinidad |
| **Pirámide de tiers invertida** | 353 cartas de tier 1 frente a 33 de tier 5. El endgame está vacío |
| **Asentamientos sin nombre** | **41 de 95** siguen siendo marcadores (`Aldea IX`, `Pueblo VI`). Las capitales ya están: solo queda `Capital I` de Ostad, que no tiene capital ni en el canon |

## 4.2 Decisiones pendientes de Oriol

1. **Bosmurg: Teshkorr o Tor'k Hazar.** Puse **Teshkorr** por resolución de época — la cronología la llama "capital de Bosmurg" en `EV-1995-ASALTONUBES`, cinco años después del mapa. La lista canónica de capitales da Tor'k Hazar, sin fecha. Registrado como propuesta en `atlas/overlays/correcciones.json`.
2. **La leyenda de colores de la Hoja 5.** Los biomas están como emojis (🟦🟩🟧🟥) y son legibles por máquina, pero solo Oriol sabe qué elemento es cada color. Sin eso, el mapamundi es tierra/mar plano.
3. **Cuándo se borra el material de ejemplo** y qué pasa con `medium_de_la_tomba`.

## 4.3 Trabajo técnico pendiente

- **El atlas no llega a los mapamundis jugables.** `locations.json` está en el motor y los 8 TMX de mapamundi existen, pero nadie los conecta con `LevelTransition`: no se puede viajar de Boundington a otra nación.
- **`Onegai 2` no ha recibido nada.** El SVG geométrico y las 19 fichas de nación están generados en `Onegai-Core/atlas/salida/`, pero no se han copiado al vault.
- **`Onegai-Core` no tiene git.**
- **Banco de conformidad del reglamento.** El reglamento está implementado **dos veces** (Java en `dndWeebCC`, C++ en `MotorGraphico`) y ya divergieron: Java resuelve pasa/no pasa, C++ en cuatro grados. La solución diseñada y no ejecutada: casos entrada→salida que ambos motores deban pasar, con el mismo PRNG portado a los dos lados.
- **357 referencias cruzadas rotas**, de las cuales 347 son historias de ejemplo. Solo 10 son sistema (las deidades que faltan).
- **Falta `Player::populate_from_class_race_background(cls, race, bg, tierRules)`** (equivalente PJ a `EnemyBrain::populate_from_monster_catalog()`). Enemigo ya está, pero el PJ se crea todavía a mano. Implementarlo en `Player.h` para inicializar: `baseStats = cls.baseStats + race.statMods + bg.skillBonuses[...]`, `classId/raceId/backgroundId`, `startingEquipmentIds` al inventario, `recompute_derived()`, knownSkillIds = `cls.startingSkillIds`.
- **Conectar GameSession / WorldPlayer / BattleState al CharacterSheet único.** Hoy `GameSession.cpp` / `BattleStateOnegai` siguen usando m_health/set_health sin pasar por la hoja. Falta: crear un `CharacterSheet pj_sheet` global en GameSession y pasarlo por `player.set_character_sheet(&pj_sheet)`; BattleState al dañar un PJ debe acceder vía `character_sheet()` si lo hay.
- **432 MonsterDefinition tienen `maxHealth == 0` en JSON real.** El parse lee OK pero el dato está a cero (ej: `monster_acolito_de_penumbra`). `populate_from_monster_catalog()` lo corrige al llamar `recompute_derived()` → TierRules formula; pero el dato fuente es inválido. Si se quiere que el JSON sea canónico debe rellenarse maxHealth por Monster.
- **Extender ConditionEngine con 8 efectos activadores no implementados:** onApply onRemove, grants_advantage/disadvantage, prevent_action, half_speed. Los flags stubs existen en ConditionEngine.h pero `recalc_condition_modifiers()` no los escribe a CharacterSheet todavía. Solo `statMod[4]*stacks` funciona.
- **`Result<T>` está en `Core::Errors::Result.h` y NO tiene namespace.** Usar `Result<T>::Ok(T)` / `.isOk()`. Algún día hay que moverlo a un namespace, pero hoy el código depende de que esté global (120 usos).
- **demo_skills.cpp parche legacy #define ApplySkillEffect ApplySkillEffectLegacy 2 args.** La API real de `SkillExecutor::ApplySkillEffect` es de 4 args. El parche permite no refactorizar, pero el GAMEMACHINE Needs P0-1 SkillExecutor debe deshacerse de él.

## 4.4 Por dónde seguiría

1. **Los 202 hechizos huérfanos.** Es el mayor volumen de trabajo ya escrito y desperdiciado del proyecto, y no depende de ninguna decisión pendiente.
2. **Volcar el SVG y las fichas al vault de Onegai.** Ya están generados; es copiarlos.
3. **Conectar los mapamundis**, para que Boundington deje de ser una isla.
4. **El banco de conformidad**, antes de que las dos implementaciones diverjan más.
5. **[NUEVO C4] Player::populate_from_class_race_background() + enganchar GameSession CharacterSheet único.** — es la única pieza de la Fractura #1 que falta; enemigo ya lo tiene resuelto.
6. **[NUEVO] ConditionEngine: efectos onApply/onRemove y prevent_action/half_speed reales.** Actualmente solo suma statMod.
7. **[NUEVO] Corregir maxHealth=0 de los 432 MonsterDefinition JSON si el dato canónico es el JSON (no el calculado).**

---

# PARTE 5 — ACTUALIZACIÓN DE CANON: NOMBRES, FUNDACIONES Y TIEMPO HISTÓRICO

*Actualización: 15 de agosto de 2026 · conversación con Oriol*

> Esta sección supersede cualquier lectura anterior que trate las naciones modernas como Estados continuos desde la Gran Migración. Las fechas discrepantes de fundación no deben clasificarse automáticamente como errores.

## 5.1 Principio histórico central

Los nombres de reinos como **Gongorguma, Ashye, Gliaddokx, Tabaxi o Udrax** son, en muchos casos, etiquetas de referencia para que podamos orientarnos en el mundo actual. No implican que el mismo Estado, con las mismas fronteras, instituciones y población, exista durante ocho mil años.

Hay que distinguir cuatro niveles:

1. **Población o linaje:** continuidad cultural, biológica o genealógica.
2. **Clan, aldea o ciudad-estado:** primera organización local reconocible.
3. **Confederación o reino antiguo:** unificación regional que puede desaparecer y reaparecer.
4. **Nación histórica posterior:** Estado cuyo nombre y forma política conocemos en épocas más recientes.

Los eventos de la cronología deben indicar cuál de estos niveles describen. Cuando una fuente dice “fundación de Gongorguma” en una era temprana, puede estar resumiendo la unificación ancestral de los clanes orcos, no nombrando literalmente al Estado posterior.

## 5.2 Cómo interpretar las fechas discrepantes

Las fechas siguientes quedan registradas como **posibles fases históricas**, no como contradicciones automáticas:

| Caso | Lectura provisional | Trabajo posterior |
|---|---|---|
| Gliaddokx: 8837 / 8797 b.f. | asentamientos, clanes o unificaciones tempranas de los antepasados de Gliaddokx | asignar nombre histórico a cada fase |
| Naciones naga y lagarto: 8762 / 8742 / 8642 b.f. | formación progresiva de pueblos y territorios preestatales | separar linaje, asentamiento y Estado |
| Sagas y minotauros | varias fechas de desarrollo | no asumir que Ashye o Esmua ya existían con su forma moderna |
| Gongorguma: 8853 / 8200 b.f. | unificación ancestral de clanes frente a fundación o refundación política posterior | definir nombres orcos de época |
| Marcha de las Cuatro Estaciones: 8790 / 5809 b.f. | primera marcha y posterior reorganización, fracaso o intento de reunificación | determinar líderes y nombres de cada etapa |
| Rebelión de las Corrientes Oscuras: 8642 / 8542 / 8300 b.f. | fases sucesivas del conflicto naga | reconstruir la secuencia completa |

La etiqueta moderna en un título de cronología puede mantenerse como índice técnico, pero el cuerpo del evento debe usar el nombre de época cuando esté establecido.

## 5.3 Nombres antiguos confirmados

- **Sharium** es un nombre antiguo de **Ashye**.
- **Knehapnest** es un nombre antiguo, no un residuo que deba eliminarse.
- **Kortarium** es un nombre antiguo relacionado con el territorio que posteriormente será **Udrax**.
- **Tabaxi** tiene una continuidad excepcional vinculada a **Azhira Espiral de Marfil** y al linaje naga/élfico, pero incluso allí la civilización y sus instituciones pueden cambiar aunque sobreviva una figura ancestral.
- La capital de **Ostad** queda abierta; se inventará cuando trabajemos esa fase.

## 5.4 Gyrid / “Grit”

Queda fijado que **Gyrid Voskezorik muere en el Puente de la Traición** y no puede ser salvado. La versión en la que sobrevive debe tratarse como material incorrecto o como una versión descartada, no como una línea temporal paralela vigente.

## 5.5 Alcance de la fase actual

El trabajo prioritario se concentra en:

- la Gran Migración;
- los siglos inmediatamente posteriores;
- aldeas, clanes, linajes y ciudades-estado;
- la formación gradual de las primeras agrupaciones regionales;
- la evolución de nombres, fronteras e identidades antes de la Era Oscura.

Quedan fuera de esta fase, por decisión de Oriol:

- las Cruzadas tardías;
- el Ascenso del Mal;
- Gurkazaal, Nerazis y Eros como trama completa;
- el Gran Cataclismo;
- la Era Oscura desarrollada en profundidad;
- los grandes huecos narrativos posteriores.

## 5.6 Regla para futuros agentes

Antes de marcar dos fechas como contradicción, comprobar:

1. si hablan del mismo pueblo o del mismo Estado;
2. si el nombre utilizado es contemporáneo o editorial;
3. si una fecha describe una aldea, clan, ciudad-estado, confederación, reino o refundación;
4. si el conflicto puede convertirse en una secuencia de fases históricas;
5. si el nombre moderno está ocultando una cadena de nombres antiguos.

No consolidar todavía estas fechas en una única fundación. Primero hay que construir la historia intermedia que explique su evolución.

## 5.7 Protocolo de trabajo compartido

Todo agente que trabaje en Onegai debe:

1. leer esta bitácora antes de analizar o modificar el proyecto;
2. registrar aquí cualquier decisión canónica nueva o cambio de interpretación;
3. anotar qué archivos ha leído, creado o modificado cuando el trabajo sea relevante;
4. separar siempre **canon confirmado**, **propuesta**, **variante histórica**, **borrador** y **material descartado**;
5. no borrar una contradicción sin explicar antes por qué deja de ser válida;
6. dejar constancia de las preguntas que requieran decisión de Oriol.

La bitácora es el registro común entre agentes. El agente que no pueda resolver una discrepancia debe documentarla aquí, no resolverla silenciosamente.

---

# PARTE 6 — EXPERIMENTO DE CREACIÓN DISTRIBUIDA DE EGAROTH

*Inicio: 15 de agosto de 2026 · experimento coordinado por Oriol*

## 6.1 Objetivo

Las cuatro IAs colaborarán para intentar desarrollar el mundo entero de Egaroth como prueba de escala y resistencia del sistema de worldbuilding. Esta primera fase es un **experimento**: el material producido no se considera canon definitivo hasta pasar una revisión de integración.

El objetivo no es que cada agente cree un mundo independiente, sino comprobar si cuatro agentes pueden desarrollar regiones distintas dentro del mismo mundo sin romper:

- la cosmología;
- la cronología;
- la geografía global;
- las culturas y linajes;
- los calendarios;
- las rutas comerciales y migratorias;
- las relaciones entre continentes;
- la continuidad de nombres y Estados a lo largo de los milenios.

## 6.2 División territorial

| Agente | Región asignada | Responsabilidad |
|---|---|---|
| **Claude** | Continente del Oeste Norte | geografía, culturas, clanes, ciudades y evolución histórica del cuadrante noroccidental |
| **Trae** | Continente del Oeste Sur | geografía, culturas, clanes, ciudades y evolución histórica del cuadrante suroccidental |
| **ZCode** | Continente del Este Norte | geografía, culturas, clanes, ciudades y evolución histórica del cuadrante nororiental |
| **ChatGPT** | Continente del Este Sur | geografía, culturas, clanes, ciudades y evolución histórica del cuadrante suroriental; coordinación de integración |

La división es geográfica, no política. Las fronteras, rutas, guerras, migraciones y culturas que atraviesen dos cuadrantes deben documentarse como conexiones compartidas, no apropiarse unilateralmente.

## 6.3 Reglas comunes del experimento

1. Todos los agentes deben leer `BITACORA_DEL_PROYECTO.md` antes de trabajar.
2. Todo resultado debe distinguir **canon previo**, **propuesta experimental**, **variante**, **borrador** y **decisión pendiente**.
3. Ningún agente puede cambiar retroactivamente la cosmología o la cronología global sin registrarlo aquí.
4. Los nombres modernos no deben proyectarse automáticamente ocho mil años hacia el pasado.
5. Cada región debe incluir nombres históricos por era, no solo un nombre nacional permanente.
6. Las razas y culturas deben tener conflictos internos; no deben funcionar como bloques homogéneos.
7. Cada agente debe registrar qué elementos conectan su región con las otras tres.
8. Los nombres de capitales, ríos, montañas y ciudades deben comprobarse contra los índices antes de considerarse definitivos.
9. Si dos agentes inventan versiones incompatibles, se conservan ambas como variantes hasta la revisión de integración.
10. La calidad de la colaboración se medirá también por la capacidad de dejar huecos deliberados en vez de rellenarlos con decisiones incompatibles.

## 6.4 Ficha mínima que debe entregar cada región

Cada agente debe desarrollar su cuadrante mediante una ficha comparable:

- límites naturales y posición en el mapa;
- clima, biomas, costas, ríos y montañas;
- pueblos, linajes y protocivilizaciones;
- nombres antiguos, nombres intermedios y nombres modernos;
- ciudades-estado, confederaciones y reinos;
- recursos y tecnologías;
- religión, Creadores y prácticas rituales;
- lenguas, etnónimos y convenciones de nombres;
- rutas comerciales y migratorias;
- conflictos internos;
- conflictos con otros cuadrantes;
- eventos relevantes por era;
- personajes, familias y facciones;
- lugares sagrados y anomalías;
- estado del material: canon, propuesta o pendiente.

## 6.5 Fases del test

### Fase A — Propuesta independiente

Cada agente crea su región siguiendo la ficha común, sin intentar cerrar todavía todos los detalles globales.

### Fase B — Cruce de fronteras

Se comparan costas, cordilleras, ríos, rutas, migraciones, guerras, nombres y calendarios en las fronteras entre cuadrantes.

### Fase C — Prueba histórica

Se comprueba que una misma región pueda cambiar durante milenios sin conservar artificialmente el mismo Estado, nombre o estructura política.

### Fase D — Integración

Se construye un atlas unificado, se resuelven colisiones y se decide qué propuestas entran en el canon principal.

### Fase E — Prueba de resistencia

Se intenta generar campañas, personajes, mapas y conflictos usando el mundo integrado. Los fallos se registran como problemas de canon o de sistema.

## 6.6 Papel de ChatGPT

ChatGPT desarrollará el **Continente del Este Sur** y actuará como coordinador de integración. Esto no le da autoridad para decidir por los otros agentes: su función será detectar incompatibilidades, mantener las conexiones interregionales y elevar a Oriol las decisiones que afecten al conjunto de Egaroth.

## 6.7 Estado inicial

- Experimento aprobado por Oriol.
- Material experimental separado del canon definitivo.
- División territorial establecida.
- Protocolo común establecido.
- Contrato v0 de intercambio implementado abajo.
- Cuatro paquetes regionales creados y validados por `tools/validar_mundo_experimental.py`.
- Este Sur contiene ya una primera capa verificable de geografía, protocivilizaciones, facciones y cinco hitos. Las fechas en conflicto de Tabaxi se conservan como variante historiográfica explícita.
- El validador también comprueba colecciones de geografía, culturas y facciones; última ejecución: 4 paquetes y 34 ids estables.
- Pendiente: fijar las fronteras exactas de los cuadrantes sobre la rejilla 26×26, recibir los tres paquetes hermanos y acordar nodos compartidos antes de crear ciudades nuevas.

## 6.8 Contrato v0 de intercambio con MotorGraphico

Cada agente entregará su región como un paquete independiente, sin escribir directamente sobre los catálogos jugables actuales. La carpeta de trabajo propuesta es:

```text
MotorGraphico-main/MotorGraphico/assets/world_experiment/<agente>/<region>/
```

Cada paquete debe contener, como mínimo:

```text
manifest.json              # identidad, propietario, estado y versión
geography.json             # biomas, costas, ríos, montañas y puntos de conexión
locations.json             # territorios, ciudades, aldeas y lugares especiales
history.json               # eventos por era y fechas locales/globales
cultures.json              # pueblos, linajes, lenguas y prácticas
factions.json              # clanes, casas, religiones y poderes políticos
connections.json           # rutas y elementos que cruzan otros cuadrantes
README.md                  # decisiones, huecos y notas de integración
```

Reglas del contrato:

1. Todo objeto tendrá un **id estable ASCII**, por ejemplo `east_south_<concepto>_<nombre>`; el nombre visible podrá llevar diacríticos fuera de los ids.
2. Todo lugar tendrá `kind`, `region`, `era_start`, `era_end`, `name_current`, `names_historical` y `status`.
3. `status` solo podrá ser `canon_previo`, `experimental`, `variante`, `borrador` o `pendiente`.
4. Las fechas deberán expresar también el tipo de hito: `settlement`, `clan`, `city_state`, `confederation`, `kingdom`, `refoundation` o `renaming`.
5. Ningún paquete podrá reemplazar `assets/catalogs/locations.json` ni modificar los niveles existentes durante la fase experimental.
6. La geografía debe declarar conexiones de frontera aunque el lugar pertenezca a otro agente.
7. Las coordenadas del motor se tratarán como una capa posterior: primero se valida la geografía semántica, después la rejilla y por último el mapa jugable.
8. Todo nombre que coincida con un nombre existente debe llevar `name_relation`: `same_place`, `ancestor_name`, `renaming`, `dynastic_name`, `editorial_label` o `collision_pending`.

El flujo de integración será:

```text
paquete regional → validación de esquema → revisión de fronteras
→ atlas experimental unificado → conversión a locations/catalogs
→ mapas TMX/JSON → prueba de navegación y campaña
```

El runtime actual sigue siendo la base estable del videojuego. El experimento producirá primero contenido y datos verificables; solo después de la prueba de integración se decidirá qué entra en `assets/` como contenido jugable.

## 6.9 Registro específico de ChatGPT

ChatGPT queda asignado al **Continente del Este Sur**. Su primer entregable debe ser una propuesta regional experimental, no una reescritura del canon global. Debe incluir:

- mapa conceptual del cuadrante y sus fronteras;
- capas de nombres por era;
- pueblos y linajes anteriores a los Estados actuales;
- ciudades-estado y transformaciones políticas;
- recursos, biomas y rutas;
- conexiones explícitas con el Este Norte, Oeste Sur y Oeste Norte;
- un paquete compatible con el contrato v0;
- una lista separada de decisiones que requieren confirmación de Oriol.

# PARTE 7 — PROYECTOS: LA PANTALLA DE ARRANQUE DEL EDITOR Y LA BUILD

## 7.1 El problema

El editor abría siempre `assets/maps/editor_map.tmx` y guardaba siempre ahí.
Mientras hubo un solo mundo eso daba igual. Con el experimento de las cuatro
IAs dejó de darlo: `assets/` acabó con **96 niveles de cuatro autores
mezclados**, y no había forma de decir *«abre Boundington»* ni de mandarle a
nadie una historia sola sin mandarle los 96 niveles y que adivinara.

## 7.2 Decisión: el dueño se declara, no se deduce

**Un proyecto es un pack de contenido** (mapas, niveles, aventuras,
catálogos), declarado en `assets/proyectos/<id>.json`, listado en
`assets/proyectos/index.json`.

La primera versión agrupaba **por prefijo del nombre de fichero**. Se rompió a
la primera: un cuadrante llamó a sus niveles `ciudad_en_*` y Boundington se los
tragó enteros — **86 niveles en vez de 64**, porque también empieza por
`ciudad_`. Con cuatro autores a la vez, de quién es cada cosa no se deduce de
un nombre.

Corolario: si un fichero no está en ningún manifiesto, el índice lo reporta
como **huérfano** en vez de repartirlo a ojo. Que sobre contenido es un aviso;
que se lo quede el proyecto equivocado es un bug silencioso.

## 7.3 Decisión: un fichero índice, no escanear la carpeta

`ProjectIndex::scan()` lee `index.json` y después cada `<id>.json`. No recorre
el directorio porque **este motor evita `<filesystem>` a propósito** —
`TileMap.cpp` lo deja escrito: *«obliga a enlazar stdc++fs en algunos
toolchains»*. No se trae esa dependencia para listar cinco ficheros. Y encaja
con el criterio de 7.2: se declara.

## 7.4 Decisión: `ProjectIndex` es GL-free

Mismo criterio que `EditorState`. El índice no sabe nada de ventanas, así que
`demo_proyectos` lo prueba entero sin abrir una — y por eso se prueba de
verdad. Todo lo que vive dentro de `level_editor.cpp` solo se puede comprobar
compilando con GL delante.

## 7.5 Decisión: «compilar» un proyecto es sacar una build de esa historia

`tools/build_proyecto.py <id>` empaqueta `builds/<id>/` con **solo** lo de ese
pack. No compila C++: eso es `cmake --build` y ya funcionaba; lo que no había
forma de hacer era empaquetar la historia.

Dos comprobaciones que no son obvias y que salieron de romperlo:

- **Cierre transitivo de `targetLevel`.** Un nivel puede llevar por una puerta
  a otro que no está en el manifiesto. Sin arrastrarlo, la build carga bien y
  el jugador cruza una puerta y **se cae al vacío**. Se arrastra y se avisa de
  qué se arrastró.
- **Cada `objectId` debe tener ficha DENTRO de la build.** Un catálogo que se
  queda fuera *no da error al cargar*: da un objeto invisible, que es peor que
  un crash porque no se nota hasta que alguien busca un PNJ que no está.

Resultado: Boundington son **356 KB en 56 ficheros** frente a los **7,7 MB** de
`assets/` — un 4,5 % —, y la campaña pasa **72/72 desde dentro de la carpeta de
build**, que es la única prueba que vale: si pasara solo desde `assets/`, la
build podría estar coja y no se notaría.

> El mensaje del commit `441e133` dice «1,1 MB». Era una medición vieja, de
> antes de que el cierre transitivo dejara de arrastrar los mapamundis. La
> cifra buena es la de aquí, medida con `du -sh builds/boundington`.

`builds/` va a `.gitignore`: son copias de `assets/`, que ya está versionado.
Versionarlas duplica el repo y garantiza que se queden viejas.

## 7.6 Decisión: borrar un proyecto no borra sus niveles

`ProjectIndex::remove()` saca el proyecto del índice y borra su manifiesto, y
**nada más**. Los niveles y mapas los puede estar usando otro proyecto, y un
borrado en cascada lanzado desde un editor es justo la clase de operación que
no se puede deshacer. Quien quiera limpiar `assets/`, que mire `orphans()`.

Orden: **primero el índice, luego el fichero**. Si el borrado del manifiesto
falla, al menos no queda un id en la lista apuntando a algo que el editor va a
intentar abrir.

## 7.7 Dos cosas que salieron de probarlo

- `create()` se niega a escribir si ya hay un manifiesto en disco **aunque no
  esté en el índice**. Con el fichero suelto, `create()` lo daba por libre y lo
  machacaba con un esqueleto vacío.
- `demo_proyectos` dejaba `prueba_tmp` puesto, y acabó **commiteado**. Una
  prueba que ensucia el árbol de trabajo se paga en cada commit posterior. Hoy
  se recoge sola y es idempotente: pasa dos veces seguidas sin reportar
  huérfanos.

## 7.8 Cómo se usa

```bash
./level_editor --proyectos                  # qué hay vivo, y qué le falta a cada uno
./level_editor --proyecto boundington       # abre el suyo; F5 guarda en SUS ficheros
./level_editor --nuevo mi_mundo --prefijo mm_
python3 tools/build_proyecto.py boundington # -> builds/boundington/
python3 tools/build_proyecto.py --todos
```

Reglas de id que `create()` hace cumplir, y por qué:

| Regla | Motivo |
|---|---|
| id solo `[a-z0-9_]` | acaba siendo nombre de fichero **y** prefijo de ids |
| prefijo acaba en `_` | sin él sale `mmtabernero`, que ni se lee ni se busca |
| no duplicar id | pisar un manifiesto ajeno no se avisa solo |

## 7.9 Estado

| Proyecto | Niveles | Aventuras | Jugable |
|---|---:|---:|---|
| `boundington` | 23 | 4 | **sí** |
| `oeste_norte` | 4 | 1 | **sí** |
| `este_norte` | 56 | 0 | no — sin `entrada` |
| `mundo` | 8 | 0 | no — sin `entrada` |
| `este_sur` | 3 | 0 | no — sin `entrada` |

Los tres «no» no son errores: son packs de escenario sin aventura todavía. Lo
que hace el índice es **poder decirlo antes de ofrecer el botón de jugar**.

# PARTE 8 — EL EDITOR: PANTALLA, PROBAR, VOLVER, Y QUE LO DOCUMENTADO EXISTA

## 8.1 Lo que estaba mal

Los proyectos estaban enchufados a `argv`, no a la ventana. Lo que se
entregó como «pantalla de arranque» era una función que imprimía en
stdout y salía.

El motivo tiene nombre y es la lección de esta parte: **lo headless se
puede probar en cualquier sitio, y `level_editor.cpp` —el fichero más
grande del repo— no se podía comprobar de ninguna forma sin GPU.** Así
que lo cómodo acabó siendo lo entregado. No fue pereza puntual: fue el
gradiente del entorno.

## 8.2 La causa antes que el síntoma

`tools/stub_gl/glad/glad.h` es un glad de mentira que declara los
símbolos GL sin dibujar nada, y `tools/comprobar_editor.sh` lo usa para
pasar el compilador entero por `level_editor.cpp` sin tarjeta gráfica.
Hoy pasa limpio.

No sustituye a ejecutarlo —los errores de shader y de estado siguen
necesitando una GPU— pero convierte «no se puede comprobar» en «se
comprueba todo menos GL». Lo cómodo ya no es lo incorrecto.

## 8.3 Decisión: el estado de la pantalla es GL-free

`Editor::ProjectHub` tiene **todo** el estado (qué hay marcado, en qué
modo, qué dijo el build) y `demo_proyectos` lo recorre entero.
`level_editor.cpp` solo pinta lo que el hub diga y le pasa las teclas
traducidas a caracteres.

Clave: **la pantalla no abre niveles**. Devuelve `Open`/`Quit` y el que
llama actúa. Eso es lo que permite probarla sin ventana. Mismo criterio
que `EditorState` y `ProjectIndex`.

## 8.4 Decisión: probar es lanzar el juego como proceso aparte

`P` guarda y lanza `./juego --nivel <este> --catalogo <el del proyecto>`.

Aparte y no dentro porque `Application` monta su propia ventana y su
propio contexto GL, y además un cuelgue probando no se lleva por delante
lo que estabas editando. Es el botón de play de Godot. **Guarda antes** a
propósito: probar lo que hay en disco mientras miras otra cosa en
pantalla es la peor forma de perder una hora.

Para eso `juego.cpp` deja de tener el nivel fijo en el código.

## 8.5 Decisión: dos fases, y se va y se vuelve

`M` (o `ESC`) vuelve a la pantalla de proyectos; cerrar del todo es `ESC`
otra vez, ya allí. Cambiar de proyecto no puede costar relanzar el
binario.

Cada vuelta rehace tileset, mapa y HUD a propósito: dos proyectos no
comparten tileset, y reaprovecharlos era la vía rápida para acabar
editando Boundington con la paleta de otro.

### El fallo que salió de trazar, no de ejecutar

`ESC` sigue físicamente hundido al cambiar de fase, y un `KeyEdge` recién
creado cree que no había nada pulsado. Esa misma pulsación contaba como
flanco nuevo en el primer frame de la pantalla: `ESC` en el editor te
devolvía a la pantalla y **la misma pulsación cerraba el editor**. Se
habría visto como «ESC cierra y ya», sin entender por qué.

`KeyEdge::prime()` da por vistas las teclas ya pulsadas. Se llama al
entrar en cada fase y al volver del juego.

## 8.6 Decisión: letras, no teclas de función

En un portátil Mac las teclas de función son teclas de medios: `F5` de
verdad pide `Fn+F5`, que con las dos manos ocupadas en el mapa no es una
tecla, es una maniobra.

| Antes | Ahora | |
|---|---|---|
| `F5` | **`G`** | Guardar |
| `F6` | **`V`** | Validar |
| `F7` | **`P`** | Probar |
| `F8`/`F9` | **`,`** / **`.`** | Escenario anterior / siguiente |
| `ESC` | **`M`** | Menú de proyectos |

Las teclas de función siguen valiendo como alias, pero **manda la letra**.

## 8.7 Los controles fantasma, y el guardián

La cabecera del editor documentaba un `F7 = cambiar a modo jugador` y un
`F8/F9 = nivel anterior/siguiente` **que no existían**: controles escritos
antes de implementarlos y nunca borrados. Un control fantasma es peor que
uno sin documentar — el usuario pulsa, no pasa nada, y concluye que el
editor está roto.

`tools/verificar_controles.py` lo comprueba **en las dos direcciones**:

- ninguna tecla anunciada sin enlazar (fantasmas);
- ninguna tecla enlazada sin anunciar (funciones que nadie sabe que
  existen, que es trabajo tirado);
- y que el número de herramientas cuadre entre el enum, las teclas, el
  menú y la barra de estado — cuatro sitios que hay que tocar a la vez.

Se ganó el sueldo el mismo día: cazó `[` y `]` (subgrupos de objetos) que
alguien añadió mientras se escribía esto, y un enum que había crecido a 7
mientras la cabecera seguía diciendo 6.

> **Nota sobre trabajar a la vez.** `level_editor.cpp`, `boundington.json`
> y `este_norte.json` cambiaron **bajo los pies** durante esta sesión:
> aparecieron `F8/F9`, una séptima herramienta (`LinkLevel`), `[`/`]` y
> una quinta aventura. Con cuatro agentes y Oriol sobre los mismos
> ficheros, **los comprobadores valen más que la coordinación**: son lo
> único que se entera de un cambio ajeno sin preguntarle a nadie.

## 8.8 Una prueba mía que estaba mal

`demo_proyectos` exigía que Boundington tuviera **exactamente 4**
aventuras, y saltó sola en cuanto Oriol añadió
`boundington_precuela_taberna.json`. Ahora comprueba que las cuatro de la
campaña estén **por nombre** y admite que haya más.

Una prueba que se rompe porque el trabajo avanza no comprueba nada:
estorba. Lo mismo vale para las cifras de esta bitácora — el recuento de
niveles cambia cada hora mientras los cuadrantes se escriben, así que
`FORMATO_PROYECTOS.md` ya no lo fija y remite a
`tools/build_proyecto.py --todos`.

## 8.9 Guías tocadas

| Documento | Qué se hizo |
|---|---|
| `FORMATO_PROYECTOS.md` | **Nuevo.** El manifiesto, las reglas de id y prefijo, y las dos formas de `entrada`. No existía, y por eso `este_norte` rellenó el campo de otra manera |
| `README.md` | Sección de proyectos + tabla de controles de las dos pantallas |
| `examples/level_editor.cpp` | Cabecera al día, sin fantasmas |

## Anexo — Los documentos que hay que leer

| Documento | Qué contiene |
|---|---|
| `ONEGAI_CORE_Analisis_y_Plan.md` | El análisis de los tres proyectos, las 5 fracturas y la hoja de ruta por fases |
| `Onegai-Core/ATLAS_Mapamundi_y_Volcado.md` | La rejilla canónica, los mapamundis y el volcado al proyecto documental |
| `MotorGraphico/FORMATO_AVENTURAS.md` | Beats, flags, `skillCheck` y `startBattle` |
| `MotorGraphico/FORMATO_PROYECTOS.md` | El manifiesto de proyecto, las reglas de id y prefijo, y las dos formas de `entrada` |
| `MotorGraphico/ARCHITECTURE.md` | Las 7 fracturas de coherencia del motor y sus reglas. **Escrito por Oriol, y buenísimo** |
| `MotorGraphico/GAMEMACHINE_NECESIDADES.md` | Mapa de brecha de 24 subsistemas contra el GDD |
| `Onegai 2/Onegai/Referència/GUIA_DEL_PROYECTO.md` | Dónde está cada cosa del corpus documental |
| `dndWeebCC/docs/Sistema_Cartas_Tiers.md` | El GDD: 20 secciones con las fórmulas cerradas |

**Regla de oro heredada de `ARCHITECTURE.md`, que este trabajo aplicó *entre* proyectos y no solo dentro de uno:**

> Un concepto, una representación, un propietario, una dirección.
