# Canon ERA XIII — 2000 b.f. al 1990 b.f. (10 años cruzadas)

> **Lectura obligatoria para las 4 IAs.** Eventos confirmados del atlas que se pueden referenciar sin inventar. Si necesitas uno que no esté aquí, marca **`[TBD canon]`**.

## Eras de Onegai (resumen)
```
ERA  I  Primigenia    (100k – 8000 b.f.)
ERA  II Dragones      (8000 – 6000)
ERA III Titanes       (6000 – 4000)
ERA IV  Peregrinación (4000 – 3000)
ERA V   Silencio      (3000 – 2800)
ERA VI  Humana        (2800 – 2600)
ERA VII Huida         (2600 – 2500)
ERA VIII Elfos        (2500 – 2300)
ERA IX  Gnomos        (2300 – 2150)
ERA  X  Forjas        (2150 – 2050)
ERA XI  Crepúsculo    (2050 – 2025)
ERA XII Naga          (2025 – 2000)
ERAXIII Cruzadas      (2000 – 1058)  ← NUESTRO PERIODO (INICIO 2000)
ERA XIV Tormenta      (1058 – 800)
```

## Eventos clave permitidos (2000..1995)

| Año b.f. | ID | Evento | Regiones implicadas |
|---|---|---|---|
| 2000 | era13_ev_001_proclama | **Proclamación 14 Cruzadas en Gran Catedral de Ascaris (Capital Ascaria)**. El Patriarca V convocó a las naciones occidentales y orientales. Cierra templos de deidades no alineadas. | **CLOUDE** (Ascaria) + todas las demás por emisarios. |
| 2000 | era13_ev_002_alistamiento | **5 Grandes Alistamientos en puertos occidentales**. Las naciones Aegroum y Bastrea proveen barcos; 17k hombres se embarcan para la franja Ecla. | CLOUDE (Aegroum/Bastrea) + **TRAE** (Ecla-norte destino). |
| 2001 | era13_ev_003_invasion_ecla | **Primera Cruzada Occidental invade Franja Norte de Ecla**: Desembarco en 3 playas. Construcción de la Fortaleza de la Hoz (frontera Ascaria/Ecla, hoy ~coord 380,945). | **CLOUDE ↔ TRAE frontera común**. |
| 2001 | era13_ev_004_naga_refuerzo | Los Nagas invaden **Udrax-sur** (atacan la colonia pesquera Eabotara y Jggotti). Las ciudades sobreviven por ayuda de sacerdotes de Nocturnsea. | **TRAE** (Udrax). |
| 2002 | era13_ev_005_tratado_tormenta | Tabaxi y Gongorguma firman **Tratado de la Tormenta**: bloquean el paso marítimo oriental. 1ª flota Cruzada queda atrapada en el Mar Rojo durante 9 meses. | **CHATGPT**. |
| 2003 | era13_ev_006_mar_rojo | **Batalla del Mar Rojo (CHATGPT ↔ CLOUDE)**. Cruzados intentan romper el bloqueo. 2/3 de las naves se hunden. | CHATGPT + CLOUDE. |
| 2003 | era13_ev_007_ejercito_verde | **ZCODE**: Choubar y Bosmurg movilizan el Ejército Verde. Entran en Esmua para proteger el Paso de los 9000. | ZCODE (3 naciones). |
| 2004 | era13_ev_008_asedio_ithon | **Batalla de Ithon (2004)**. Cruzados intentan tomar la Isla Elfa Ithon (TRAE-Ecla). 3 meses asedio, se rinden por hambre. | **TRAE** (Ecla). |
| 1999 | era13_ev_009_necromancia | [TBD canon] Primer informe de Necromantes en **Gongorguma (CHATGPT-SUD)** — rumor. | CHATGPT. |
| 1997 | era13_ev_010_pacto_sangre | Pacto de Sangre: Cruzados y Naga hacen tregua por 4 años en Udrax-Sur. | **TRAE** (Udrax). |
| 1995 | era13_ev_011_asaltonubes | **EV-1995-ASALTONUBES** — Último evento permitido en este piloto. Batalla en la frontera Ascaria/Ecla usando aeronaves de Esmua (ZCODE) interceptadas. CLOUDE pierde 4 príncipes. | CLOUDE + TRAE frontera + ZCODE remoto. |

## Deidades canon (17 existentes en `RPG::Catalogs::DeityCatalog`). **No inventar ninguna.**
```
deity_sun_father, deity_moon_mother, deity_veiled_queen, deity_wild_court,
deity_storm_lord, deity_deep_king, deity_ember_seraph, deity_morning_light,
deity_silver_hunt, deity_ocean_serenity, deity_iron_oath, deity_eightfold_path,
deity_laughing_bard, deity_arcane_spire, deity_nocturne, deity_first_flame,
deity_wanderer
```
Si la ciudad necesita un patrón, elige UNO de estos. Si "ninguna encaja" → **`[TBD dios]`** + lo pones como `"deityIds": []` y un comentario en el README.

## Razas canon (43 existentes, RaceCatalog). Resumen cortas:
- Humanos, Elfos (S/E/M/A), Enanos (C/D/S), Semielfos, Semiorcos, Gnomos, Halflings, Tieflings, Dracónidos, Aarakocra, Aasimar, Bugbears, Centauros, Deep Gnomes, Duergar, Drow, Firbolgs, Genasi (f/t/a/a), Goblins, Goliaths, Grung, Hobgoblins, Kenku, Kobolds, Lizardfolk, Loxodon, Minotauros, Nagas (¡las del ERA XII!), Orcos, Quaggoth, Satyr, Scourge, Tabaxis (¡raza!), Tortles, Tritons, Troglodytes, Vedalken, Verdan, Warforged, Yuan-ti.

## Clases canon (61 existentes). NO inventar: Barbarian (6 subclases), Bard (6), Cleric (7), Druid (5), Fighter (7), Monk (6), Paladin (6), Ranger (5), Rogue (6), Sorcerer (4), Warlock (4), Wizard (4). Pnjs: elige classId `class_*_subclass_*` o `class_wandering_blade`, `class_militia_archer` etc si no sabes cuál.
