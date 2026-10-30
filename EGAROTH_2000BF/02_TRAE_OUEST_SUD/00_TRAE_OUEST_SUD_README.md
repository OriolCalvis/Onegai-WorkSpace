# TRAE — OCCIDENTE MERIDIONAL (Ecla + Udrax)
> Año: **2000 b.f. | ERA XIII**, inicio de las Catorce Grandes Cruzadas.
> Origen de este fichero: **IA = TRAE**, región OUEST-SUR (ver `EGAROTH_2000BF/README.md` §1).
> Archivos complementarios en esta misma carpeta: `05_naciones.json`, `10_ciudades.json`, `15_zona_mazmorras.json`, `20_catalogo_objetos_unicos.json`.

---

## 1 · Fronteras adyacentes (OBLIGATORIO — coordinar con otras IAs)

| Dirección | IA / Región | Coordinar sobre |
|---|---|---|
| ↑ NORTE | **CLOUDE (Occidente Septentrional / Ascaria-sur)** | Paso de la **Fortaleza de la Hoz** (era13_ev_003_invasion_ecla, 2001). Frontera ~coords 360-440 x 880-950 del SVG. 2 fortalezas Cruzadas + 1 fortaleza Eclana. El paso solo se abre 3 meses al año. |
| → ESTE | **CHATGPT (Oriente Meridional / Tabaxi archipiélago)** | **Mar de los Mil Dones** (~640,430 SVG). Udrax-sur tiene pactos comerciales con puerto Tabaxi occidental (fijar por CHATGPT). Coordinar `puerto_de_la_espera_*` (Udrax-side) ↔ `puerto_tabaxi_*` (ChatGPT side). |
| ↓ SUR | Mar de los Mil Dones + Islas Nagas (ERA XII) | Frontera natural marítima. |
| ← OESTE | Océano / Cordillera del Fin del Mundo (uninhabitable) | No coordinar. |

---

## 2 · Resumen Lore 2000 b.f.

### 2.1 Ecla — Reino de la Alquimia y los Elfos Refugiados
- **Capital:** **Umedan** (~31 mil hab, tier 3). Ciudad alquimia y biología (atributo canon). Universidad Magna Biologica.
- **Lema:** "De la semilla, el veneno — del veneno, la cura".
- **Gobierno:** Triunvirato de 3 (1 alquimista = Cámara Cinabrio, 1 elfo = Consejo Silvano, 1 militar = Mariscal Fronterizo para Cruzadas).
- **Situación ERA XIII:** Acaba de invadírsele la **Franja Norte (CLOUDE ascendente)** → 3 asentamientos en la frontera son Plaza Fuerte Cruzada ocupada militarmente (ver zona_mazmorras). Refugiados de Ascaria pasan por Joren y Edea → ~600 desplazados.
- **Clima:** Selvático al sur; templado continental en el norte; humedales en el centro (Canal de Aguas Negras en Etriath).
- **Razas mayoritarias (atlas canon raceIds):** `race_elf_silvan`, `race_human_vietic`, `race_gnome_deep`, `race_half_elf`.
- **Dioses principales:** `deity_veiled_queen` (patrono Umedan), `deity_morning_light` (Ekait), `deity_arcane_spire` (Universidad de Joren).

### 2.2 Udrax — Confederación de Ciudades Estado (Pacto de los Pozos)
- **Capital:** **Bomengrid** (~32 mil hab, tier 3). Muralla a medio caer (atributo canon).
- **Lema:** "El agua mueve las ruedas, el acero mueve las alianzas".
- **Gobierno:** **Confederación**: cada ciudad tiene 1 voto en la Asamblea de Pozos (7 ciudades mayores + 2 ciudades menores). Alcalde/Baron gobernante.
- **Situación ERA XIII:** Recibe el **Pacto de Sangre 1997 (era13_ev_010)** (tregua 4 años con Naga). Ataques Nagas han cesado pero Ujestrall y Aherhia mantienen alerta. Las **minas de Eottetika (hierro mithril?) se agotan** → conflicto económico interno.
- **Clima:** Estepa occidental en el centro; costa; humedales; montañas escarpadas al norte (frontera Ecla por el Paso de los Danzantes).
- **Razas mayoritarias:** `race_human_mariner`, `race_lizardfolk`, `race_naga_blooded` (sangre mezclada con Naga ERA XII = secretamente influyente), `race_minotaur` (puerto).
- **Dioses principales:** `deity_ocean_serenity` (costas), `deity_deep_king` (minas y mar interior), `deity_iron_oath` (Ejército Confederal), `deity_nocturne` (sectas de los Pozos Susurrantes).

---

## 3 · Tier 2000 b.f.

| Zona | Tier min | Tier max | Nota |
|---|---|---|---|
| Frontera Norte Ecla (Ascaria-Ecla, CLOUDE frontera) | 3 | 5 | Fortaleza de la Hoz; Cruzados |
| Frontera Sur Udrax (Mar Mil Dones - Naga) | 3 | 4 | Pacto Sangre a punto de expirar |
| Ciudades medianas Ecla (Apeartel, Edea, Etriath) | 2 | 3 | Facil inicial |
| Ciudades medianas Udrax (Ugrolas, Aherhia, Aclosos, Eottetika) | 2 | 3 | Minas crisis + refugiados |
| Capitales (Umedan, Bomengrid) | 3 | 5 | Peligros políticos |
| Interiores (Aldea) | 1 | 2 | Nuevo jugador |

---

## 4 · Facciones (canon de 05_naciones.json)

### 4.1 Ecla
| ID | Nombre | Rol | Líder | Alineación |
|---|---|---|---|---|
| `faction_ecla_camara_cinabrio` | La Cámara Cinabrio | Gremio Alquimistas / Alcalde Umedan | **Gran Alquimista Elenya Silvanos** (elfo silvano, `class_wizard_transmutation`) | neutral_good |
| `faction_ecla_consejo_silvano` | Consejo Silvano de los Elfos | Defensa de la Selva y colonias Élficas | **Archidruid Vordir** (elfo silvano, `class_druid_moon`) | neutral |
| `faction_ecla_mariscalato` | Mariscalato Fronterizo | Defensa Frontera Norte (Ascaria) | **Mariscal Rook Cinderveil** (humano tiefling, `class_fighter_battle_master`) | lawful_neutral |
| `faction_ecla_refugiados` | Círculo del Refugio de Joren | Red ayuda a refugiados Cruzados | **Matriarca Alya Ashye** (humano, `class_cleric_life`) | neutral_good |

### 4.2 Udrax
| ID | Nombre | Rol | Líder | Alineación |
|---|---|---|---|---|
| `faction_udrax_asamblea_pozos` | Asamblea de los 9 Pozos | Gobierno Confederación | **Consul Varran Halfdran** (humano, `class_paladin_devotion`) | lawful_neutral |
| `faction_udrax_gremio_minero` | Gremio Minas y Herrería | Minas Eottetika y puertos de hierro | **Gremial Maestra Brunhild Stonehand** (enana de colina, `class_barbarian_berserker`) | lawful_neutral |
| `faction_udrax_contrabandistas_pozos` | Los Susurrantes del Pozo | Mercado negro de Eabotara/Jggotti (pozos que susurran → sustancias Nagas / prohibidas) | **"El Silbador"** (yuan-ti malison, `class_rogue_thief`) | chaotic_neutral |
| `faction_udrax_orden_puente` | Orden del Puente Milenario | Guardianes de Aclosos (puente estratégico, canon trait) | **Prioste Delgar Ironvein** (duergar, `class_cleric_war`) | lawful_good |
| `faction_udrax_secta_estimar_nox` | Secta Estimar Nox | Secta Nocturna / Pozos de Bomengrid (secretamente quieren revivir rituales Naga ERA XII) | **[TBD Oriol: nombre sacerdote nocturno]** | chaotic_evil |

---

## 5 · Mazmorras Nivel 2 (1 dungeon / nación mínimo → 4)

| ID dungeon | Nombre | Nación | Coord lógica | Tier | Tema |
|---|---|---|---|---|---|
| `dungeon_hoz_ocavernes_inferiores` | Las Cavernas Inferiores Fortaleza de la Hoz | Ecla (Frontera Norte) | `{x:420, y:910}` | 4 | Fortaleza Cruzada excavada en roca; celdas; crypta con prisioneros; posible jefe: comandante Cruzado tirano |
| `dungeon_ecla_ithon_tumba_primera_elfa` | Tumba Primera Reina Elfa de Ithon | Ecla (Isla Ithon) | `{x:205, y:400}` | 3 | Cripta pre-ERA-XIII; guardianes élficos no muertos, reliquia `objetos_capa_luna` |
| `dungeon_udrax_pozos_susurrantes_nexo` | El Nexo Profundo - Pozos de Eabotara | Udrax | `{x:560, y:528}` | 3 | Túneles Naga ERA XII; conectan con otras ciudades de pozos (Jggotti, Bomengrid). Jefe: Naga Medusa menor. |
| `dungeon_udrax_muralla_fantasma` | La Muralla Fantasma - Tramo Hundido Bomengrid | Udrax (Bomengrid) | `{x:718, y:704}` | 4 | La "muralla a medio caer" (canon trait) tiene tramos ocultos; no-muertos confederados antiguos; jefe: Wight Lord del primer cónsul. |

---

## 6 · ERA XIII eventos locales (canon referenciado en 05_naciones.json)

| ID | Año | Título | Afecta |
|---|---|---|---|
| `era13_ev_003_invasion_ecla` | 2001 | Invasión Franja Norte Ecla → Fortaleza de la Hoz | Ecla ↔ CLOUDE |
| `era13_ev_004_naga_refuerzo` | 2001 | Ataques Naga Udrax-sur (Eabotara, Jggotti) | Udrax |
| `era13_ev_008_asedio_ithon` | 2004 | Asedio 3 meses Isla Ithon por Cruzados → rendición por hambre | Ecla |
| `era13_ev_010_pacto_sangre` | 1997 | Pacto Sangre 4 años entre Nagas y Udrax | Udrax-sur |
| `era13_ev_011_asaltonubes` | 1995 | EV Asalto a Nubes → Edea recibe las aeronaves caídas | Edea (Ecla) |

---

## 7 · Mapa ASCII aproximado (posiciones canon SVG y nuevas TRAE + frontera CLOUDE)
```
                                      ↑ NORTE (CLOUDE = Ascaria-sur)
      ┌─────────────────────────────────────────────────────────────────────────────────┐
      │                      FORTALEZA DE LA HOZ (frontera TRAE↔CLOUDE, plaza Cruzada) │
  350 │                                                                                   │
      │       █ ITHON (elven)  ───Mar interior──────────── MAR DE LOS MIL DONES ↓ SUD │
  400 │                                                                                   │
      │                 UMEDAN (cap. Ecla, 31k, Alq&Bio)                                  │
  450 │                                                                                   │
      │                    APEARTEL (gremio)  ULUTH  AHERHIA (feria)  EABOTARA (Pozo ♂)│
  500 │                                                                                   │
      │                   EKAIT (Santuario discordia)                                      │
  550 │                                                                                   │
      │            JOREN (refugios exil)  EDEA (Biblioteca Prohibida)  EOTTETIKA (minas)│
  600 │                                                                                   │
      │                  ETRIATH (Canal Aguas Negras)                                     │
  650 │                                                                                   │
      │                                                  UGROLAS (refugios)  ACLOSOS Puente│
  700 │                      BOMENGRID (cap. Udrax, 32k, Muralla medio caer)              │
  750 │                                                                                   │
      │                                              UJESTRALL Santuario discordia       │
  800 │                                                                                   │
      │                                                                   ↓ → ESTE CHATGPT│
      └─────────────────────────────────────────────────────────────────────────────────┘
                                                 ↓ SUR = archipiélago Naga
    █ : ciudad EXISTENTE EN ATLAS (16 ATLAS_CANON + 8 NUEVAS origin=TRAE → total 24)
```

## 8 · Ciudades: 24 por región (16 ATLAS_CANON heredadas, 8 nuevas origin TRAE)

Año 2000 b.f. Lista completa (nombres en mayúscula si CAPITAL; +NUEVA marca `*`):
1. **UMEDAN** (Ecla, capital) — 30k / tier3 / Universidad Alquimia
2. **Edea** (Ecla) — 6k / tier2 / Biblioteca Prohibida
3. **Ekait** (Ecla) — 12k / tier2 / Santuario Discordia
4. **Joren** (Ecla) — 12k / tier2 / Refugio Exiliados
5. **Etriath** (Ecla) — 9k / tier2 / Canal Aguas Negras
6. **Apeartel** (Ecla) — 6k / tier2 / Gremio Poderoso
7. **Ithon** (Ecla) — 9k / tier1 / Is. Elfa, Colonia pesq. (ASALTO 2004)
8. **Uluth** (Ecla) — 800 / tier1 / puerto Nago-norte
9. **Port Ember**  * (Ecla-nueva) — 1.2k / tier2 / Puerto refugiados Frontera Hoz
10. **Hearthstead**  * (Ecla-nueva) — 600 / tier1 / Granja abastece Umedan
11. **Mithril Tor**  * (Ecla-nueva) — 1.8k / tier3 / Torre Mago (ARCANE SPIRE en cordillera)
12. **Silverbow**  * (Ecla-nueva) — 900 / tier2 / Alcázar fronteizo con refugio étnico
13. **BOMENGRID** (Udrax, capital) — 30k / tier3 / Muralla medio caer
14. **Ugrolas** (Udrax) — 9k / tier2 / refugio exiliados
15. **Ujestrall** (Udrax) — 9k / tier2 / Santuario discordia
16. **Aclosos** (Udrax) — 6k / tier2 / Puente Estratégico
17. **Aherhia** (Udrax) — 6k / tier2 / Feria Anual Famosa
18. **Eottetika** (Udrax) — 12k / tier2 / Minas agotándose
19. **Jggotti** (Udrax) — 800 / tier1 / Pozo que susurra
20. **Eabotara** (Udrax) — 300 / tier1 / Pozo que susurra (ataques Naga)
21. **Wait Harbor**  * (Udrax-nueva) — 2.4k / tier3 / Puerto pacto Sangre ↔ Naga
22. **Midgate**  * (Udrax-nueva) — 1.5k / tier2 / Paso Aéreo de Carros entre Eottetika ↔ Bomengrid
23. **Narrows End**  * (Udrax-nueva) — 1.1k / tier2 / Aduana y Gobernación Confederal Frontera Mar
24. **Boneglass Keep** * (Udrax-nueva) — 700 / tier3 / Plaza Fuerte del Borde Sur (Pacto Sangre Vigilancia)

* = origin TRAE (no existe en atlas canon / era Aldea IX pendiente)

---

## 9 · Coordinación explícita pendiente con otras IAs (cuando entreguen sus 00_*_README.md)

Preguntar a CLOUDE (Frontera Fortaleza Hoz):
- ¿Nombre de los 2 pueblos que pusisteis dentro del lado Ascaria del paso? (Si inventa, coordinar ids para que no se solapen con `Port Ember` TRAE-side.)
- ¿Commander de la Fortaleza? Poner como jefe de `dungeon_hoz_ocavernes_inferiores`.

Preguntar a CHATGPT (Mar Mil Dones ↔ Tabaxi):
- Coordinar `Wait Harbor (Udrax 21)` ↔ `Puerto Occidental Tabaxi` ~id y coord adyacente.
- ¿Hay una fortaleza Tabaxi al sur visible desde Boneglass Keep (24)? Si la hay, añadir dungeon compartido.
