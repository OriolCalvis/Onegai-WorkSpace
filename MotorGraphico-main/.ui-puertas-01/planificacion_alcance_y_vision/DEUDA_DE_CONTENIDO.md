# Deuda de contenido aguas arriba (AV12-05)

Medida el 14/10/2026 por Claude Sonnet, sobre `MotorGraphico/assets/catalogs/` tal como está hoy en el Mac de Oriol.
Solo se mide y se propone. **No se ha cambiado ningún catálogo.**
Se repite con `python3 tools/medir_deuda_contenido.py` (paquete `out/claude_deuda01/`).

## 1. Los cuatro números de hoy

| Medida | Plan | Hoy | Por qué difiere |
|---|---:|---:|---|
| Hechizos que ninguna clase ni monstruo usa | 202 de 210 | **205 de 210** | El plan contó la etiqueta `classTags` de la carta (8 hechizos con clase, 202 sin). La clase solo lista 5. Con `classTags` sale 202 otra vez. |
| Monstruos con vida 0 | 432 | **0 de 432** | La medida del plan es anterior a D-03: el cargador buscaba la vida en la raíz de la ficha y no estaba. Hoy lee `defenses.maxHealth` (DEC-02), que está en las 432. |
| Clases sin hechizos | 59 de 61 | **125 de 127** (jugables: 70 de 70) | El catálogo pasó de 61 a 127 clases. Hoy solo tienen hechizos 2 (`class_arcanist`, `class_dawn_voice`), y no son jugables. |
| Referencias rotas | no medido | **187 de 7.994** (27 ids distintos) | Primera medida. 169 cuelgan de 10 deidades que no existen. |

El número del plan que más cambia la imagen es el de la vida: **ese hueco ya está cerrado**. Lo que sí queda abierto en monstruos es otro (sección 3).

## 2. Qué cuenta de verdad

El script usa la regla del C++, no la que parece razonable.

| Cosa | Regla | Dónde se lee |
|---|---|---|
| Vida de un monstruo | `defenses.maxHealth`, si no `derived.vida`, si no `maxHealth` / `max_health` / `health` de la raíz, si no 0 | `CatalogLoadersCriaturas.h` |
| Cartas de una clase | `startingCards.skills` + `spells` (iniciales) + `learnableSkills` (aprendibles). Una ficha nueva las conoce todas | `CatalogLoadersPersonaje.h`, `CharacterCreation.cpp` 275-285 |
| Cartas de un monstruo | `skillIds` | `EnemyCatalogBridge.h` |
| Hechizo | una entrada de `spells.json` | `SkillDefinition.h` |
| Carta | una entrada de `skills.json` | `SkillDefinition.h` |
| Mínimo de cartas de una clase | `handLimit` de su tier en `assets/rules/tier_rules.json` (tier 1 = 10) | `TierRules.h` |
| Referencia rota | un id que no está en la familia de catálogo que le toca (61 campos en la tabla `REFERENCIAS` del script; 54 tienen datos) | `medir_deuda_contenido.py` |

Dos hechos del motor que cambian la lectura:

- **`spells.json` no lo carga nadie en el juego.** `Application.cpp` 389 y `CatalogoCartas::cargar` leen solo `skills.json`. Los 5 hechizos que listan las clases viajan en `startingCards.spells` y se buscarían en un catálogo que no los tiene. El motor tampoco lee `prayerSkillIds` de las deidades (solo lo carga).
- **Todas las clases son de tier 1.** Las cartas de tier 3, 4 y 5 no las puede tener ninguna clase.

## 3. Medida 1 — Hechizos y cartas que nadie usa

### Hechizos (210)

| Grupo | Hechizos |
|---|---:|
| En `spells.json` | 210 |
| Plegarias de una deidad (`deity`, «cuenta para el límite de 3») | 200 |
| Los demás | 10 |
| Usados por una clase | 5 |
| **Huérfanos** | **205** |
| — plegaria de una deidad que existe (`deities.json`) | 100 |
| — plegaria de una deidad que **no** existe | 100 |
| — sin deidad, con `classTags` que la clase no lista | 3 |
| — sin deidad ni etiqueta | 2 |

Huérfanos por tier: tier 1, 101 de 106. Tier 2, 84 de 84. Tier 3, 20 de 20.

Qué más los nombra (sin que el motor lo lea todavía):

| Fuente | Hechizos huérfanos |
|---|---:|
| `deities.grantedSpells` | 100 |
| `equipment.awakening.unlocks` | 20 |
| `pregen_characters.hechizoIds` | 7 |
| `races.unlocks` | 1 |
| **Sin ningún otro enlace** | **90** |

### Cartas (1.191)

| Grupo | Cartas |
|---|---:|
| Usadas por alguna clase | 935 |
| **Huérfanas** (ninguna clase ni monstruo) | **256** |
| — nombradas por una aventura (`skillCheck`, 9 habilidades) | 9 |
| — nombradas por un personaje ya hecho (`pregen_characters`) | 27 |
| — nombradas por el equipo (`linkedSkill`) | 6 |
| — **sin ningún otro enlace** | **218** |

Huérfanas por tier: 1 → 83 de 745. 2 → 48 de 309. 3 → 47 de 58. 4 → 46 de 46. 5 → 32 de 33. El final del juego no lo puede tener ninguna clase.

## 4. Medida 2 — Monstruos

| Tier | Fichas | Vida 0 | Sin CA | Sin ataque | Sin cartas (`skillIds`) | Ataques | Ataques sin cifra | Vida sobre `healthCap` |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 58 | 0 | 0 | 0 | 58 | 116 | 116 | 0 |
| 2 | 107 | 0 | 0 | 0 | 107 | 214 | 214 | 0 |
| 3 | 144 | 0 | 0 | 0 | 144 | 290 | 290 | 2 |
| 4 | 89 | 0 | 0 | 0 | 89 | 198 | 198 | 20 |
| 5 | 34 | 0 | 0 | 0 | 34 | 82 | 82 | 14 |
| **Total** | **432** | **0** | **0** | **0** | **432** | **900** | **900** | **36** |

Lo que queda abierto, aunque la vida ya esté:

- **Ningún monstruo tiene cartas.** Los 432 llevan `skillIds` vacío. Sus ataques son texto: 900 ataques, ninguno con cifra de daño (`magnitude` / `damage`).
- **Dos vidas en cada ficha.** `defenses.maxHealth` (la que lee el motor) es el doble exacto de `derived.vida`, en las 432.
- **36 monstruos superan el `healthCap` de su tier** (60 de vida contra 35 en tier 3, etc.). Hoy no importa, porque `createEnemy` no pasa `tierRules`. Si alguien las pasa, la vida se recorta. Es el cabo de AV12-04.

## 5. Medida 3 — Clases

| Grupo | Clases |
|---|---:|
| Total | 127 |
| Jugables (`playable` ausente o `true`) | 70 |
| No jugables | 57 |
| **Sin ningún hechizo** | **125** |
| — jugables sin hechizo | 70 de 70 |
| — con hechizo: `class_arcanist` (3), `class_dawn_voice` (2), las dos no jugables | 2 |
| **Con menos cartas que el mínimo de su tier** | **57** |
| — jugables | 0 |

- Las 70 jugables tienen 10 cartas cada una: justo el `handLimit` del tier 1.
- Las 57 no jugables tienen entre 3 y 8.
- Con todas en tier 1, no se puede medir el mínimo de los tiers 2 a 5 (15, 20, 24, 28).

## 6. Medida 4 — Referencias rotas

187 rotas de 7.994 comprobadas.

| Catálogo | Campo | Rotas | Falta |
|---|---|---:|---|
| `spells` | `deity` | 100 | 10 deidades |
| `equipment` | `deity` | 50 | las mismas |
| `backgrounds` | `lore/deities` | 10 | las mismas |
| `adventures` | `deidadIds` | 9 | las mismas |
| `encounter_tables` | `entries/refId` | 13 | monstruos `monster_lobo`, `monster_oso`… (los ids reales no llevan prefijo y estos no existen sin él) |
| `encounter_tables` | `lootTableId` | 1 | `loot_jefe_lobo` |
| `skills` | `evolvesInto` | 2 | `skill_steel_combo_t2`, `skill_precise_strike_t2` |
| `spells` | `evolvesInto` | 1 | `spell_healing_light_t2` |
| `races` | `unlocks` | 1 | `skill_rune_forging_t2` |

Las 10 deidades que faltan: `el_cuervo_bibliotecario`, `el_hermano_mendigo`, `el_leon_del_alba`, `el_sepulturero_amable`, `el_toro_de_gongorguma`, `la_llama_eterna`, `la_luna_hueca`, `la_madre_de_los_doce`, `la_senora_de_las_mareas`, `la_serpiente_de_contratos`. Con ellas escritas, se arreglan **169 de las 187**.

Bien: las 979 referencias de `monsters.loot` existen (28 ids distintos, 27 en `equipment` con precio y 1 en `consumables`). El «incorporar los 28 `lootIds`» de AV12-05 está resuelto en los datos.

## 7. Propuesta

### Lo que se arregla con una fórmula

| Grupo | Fórmula | Alcance | Ficheros de la propuesta |
|---|---|---|---|
| Vida de un monstruo (AV6-06) | `vida = base + paso × tier`; criatura `8 + 4t`, élite `16 + 6t`, jefe `30 + 10t` | acierta 430 de 432; da valor a las casillas vacías (tier 1 y 2 élite y jefe, tier 5 criatura) | `vida_por_tier.csv` |
| Vida: las 2 fichas fuera | las dos pasan a la fórmula | `itx_centinela_de_salmuera` 18→16, `itx_matriarca_de_la_sal` 64→70 | `monsters.propuesto.json`, `monstruos_vida_cambios.csv` |
| CA | `10 + DES + bono de rango` (criatura 0, élite 1, jefe 2) | acierta 431 de 432 | — |
| Defensa mental y resistencia física | `10 + CAR`, `10 + CON` | acierta 430 de 432 | — |
| Precisión mágica | constante 10 (no usa INT) | 432 de 432 | — |
| Hechizos con `classTags` que la clase no lista | añadirlos a `startingCards.spells` de la clase que dice la etiqueta | 3 | `reparto_hechizos.csv` (vía «formula») |

Diez ejemplos de la fórmula de vida y de CA (uno por tier y rango que existe):

| Monstruo | Tier | Rango | Vida hoy | Vida fórmula | CA hoy | CA fórmula |
|---|---:|---|---:|---:|---:|---:|
| `acolito_de_penumbra` | 1 | criatura | 12 | 12 | 11 | 11 |
| `anguila_ancestral` | 2 | criatura | 16 | 16 | 13 | 13 |
| `anguila_ancestral_veterano` | 3 | criatura | 20 | 20 | 14 | 14 |
| `ariete_humano_comandante` | 3 | élite | 34 | 34 | 17 | 17 |
| `capitana_bruna_la_puerta_cerrada` | 3 | jefe | 60 | 60 | 20 | 20 |
| `aprendiz_hueco_veterano` | 4 | criatura | 24 | 24 | 14 | 14 |
| `anguila_ancestral_matriarca` | 4 | élite | 40 | 40 | 18 | 18 |
| `ancla_de_entropia` | 4 | jefe | 70 | 70 | 20 | 20 |
| `aprendiz_hueco_coronado` | 5 | élite | 46 | 46 | 18 | 18 |
| `adalid_del_rey_de_armas` | 5 | jefe | 80 | 80 | 21 | 21 |

Sin fórmula ninguna de las 187 referencias rotas: la pista de «quitar prefijo» no resuelve ni una.

### Lo que necesita a una persona

| Grupo | Qué hay que decidir o escribir | Quién | Tamaño |
|---|---|---|---|
| Plegarias de deidad existente | Nada de contenido: falta que el motor lea `prayerSkillIds`. Es trabajo de código, no de contenido | Claude Cowork (carril motor) | 100 hechizos |
| Las 10 deidades que faltan | Escribirlas: canon, dominio, 10 plegarias ya enlazadas | Oriol o Gemini | 10 fichas; 169 referencias; 100 hechizos |
| Los 2 hechizos sin clase ni deidad | Elegir clase (`spell_natures_grasp`, `spell_veil_step_t2`; este último ya lo desbloquea el linaje Sangre Feérica). El script da 3 candidatas de las 6 clases del método | Oriol | 2 |
| Las 70 clases jugables sin hechizo | Decidir si lanzan hechizos y cuáles; adaptarlas al tono de las 6 referenciadas (método de Oriol del 28/09) | Oriol o Gemini (AV13-21/36) | 70 |
| Clases de tier 2 a 5 | No existen. Sin ellas, 125 cartas de tier 3 a 5 y 104 hechizos de tier 2 y 3 no tienen dueño | Oriol | por decidir |
| Cartas de tier 3 a 5 huérfanas | Dárselas a una clase o descartarlas | Oriol | 125 |
| Cartas de monstruo | `skillIds` vacío en los 432. Convertir las cartas de ejemplo en cartas de monstruo | Gemini (AV13-36) | 432 |
| Cifra de daño de los ataques | 900 ataques sin cifra. No hay regla en los datos para sacarla | Oriol (AV6-06, AV12-04) | 900 |
| Vida: `derived.vida` | Es la mitad de `maxHealth` en las 432. Decidir cuál es la canónica y borrar o recalcular la otra | Oriol | 432 |
| Las 2 fichas fuera de fórmula | ¿Error o intención? (una élite del mar con 18 en vez de 16, una jefa con 64 en vez de 70) | Oriol | 2 |
| Tablas de encuentros | Sustituir 13 `monster_*` por ids reales y crear o cambiar `loot_jefe_lobo` | Oriol | 14 |
| Evoluciones sin destino | Crear `skill_steel_combo_t2`, `skill_precise_strike_t2`, `spell_healing_light_t2`, `skill_rune_forging_t2` o quitar el enlace | Oriol o Gemini | 4 |

## 8. Preguntas para Oriol (a la BANDEJA, las consolida Claude Cowork)

1. **¿Los 200 hechizos divinos van por deidad o por clase?** La carta dice «cuenta para el límite de 3 por personaje», y `maxDivineCards` crece por tier: es la vía de la deidad. *Recomendación:* por deidad. No repartirlos en clases. Eso deja la tarea de AV12-05 («dar valores a los huérfanos con las 6 clases») en 5 hechizos: 3 por fórmula y 2 a elegir.
2. **¿Escribes tú las 10 deidades o las pasamos a Gemini?** Es lo que más arregla: 169 referencias y 100 hechizos.
3. **¿Cuál es la vida buena, `maxHealth` o la mitad (`derived.vida`)?** *Recomendación:* la que lee el motor, y recalcular la otra.
4. **¿Adoptamos la fórmula de vida por rango como regla del creador de monstruos (AV6-06)?** Y, al hacerlo, ¿qué pasa con los 36 monstruos que superan el `healthCap`? Esto es AV12-04.
5. **¿Los 900 ataques de monstruo necesitan cifra de daño?** Hoy el daño está en el texto.

## 9. Lo que no se cuenta

- `locations_trae_test.json`: nadie lo carga. Tiene ids de otra familia (`deity_*`, `race_*`).
- `startingCardsJava` de 5 clases: el cargador no lo lee.
- `adventures.historiaIds` (338) y `backgrounds.characterCreation.*.id` (60 por tipo): apuntan fuera de `assets/catalogs/` o son ids locales, no de catálogo.
- Campos de texto libre (`condition`, `awakening.condition`, `villainProfile`, `propia.referencias`, `classes.propia.familia`).
- Un hechizo que lleva `classTags` pero no está en la lista de la clase **cuenta como huérfano** (es lo que ve el juego). Con la etiqueta saldría 202.
- No se ha medido la dificultad de combate de los monstruos (harness 1 contra 1..n de AV12-05): necesita compilar el motor.
- El mínimo de cartas por tier es el `handLimit`. Si el GDD fija otro mínimo, se cambia en `lee_hand_limit`.

## 10. Cómo se prueba

```
python3 tools/medir_deuda_contenido.py                       # los cuatro números
python3 tools/medir_deuda_contenido.py --md                  # tablas
python3 tools/medir_deuda_contenido.py --json                # todo, con las listas
python3 tools/medir_deuda_contenido.py --proponer /tmp/prop  # las propuestas, fuera de assets/
python3 tools/test_medir_deuda_contenido.py                  # 42 comprobaciones sobre un catálogo de mentira
```

La propuesta (`out/claude_deuda01/propuesta/`) trae: `monsters.propuesto.json` (el catálogo con solo 2 cifras cambiadas), `monstruos_vida_cambios.csv`, `vida_por_tier.csv`, `reparto_hechizos.csv` (205 filas, con vía y arreglo), `referencias_rotas.csv` (187), `clases_pendientes.csv`.
