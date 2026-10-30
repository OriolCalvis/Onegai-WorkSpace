# Pla de producció amb PixelLab · MotorGraphico · 01/10/2026

Encàrrec d'Oriol: fer tota la interfície del joc amb PixelLab i els sprites que
falten per polir-lo. Decisions seves: **fins a 2.000 generacions**; **els panells
del pergamí passen a ser imatges de PixelLab** (9-slice); els personatges per
capes (races, equip, creació d'aparença) **es mantenen**; PixelLab posa
**monstres, terreny, props i PNJ amb cara pròpia**.

Afegit el mateix dia: la **creació de personatge** també entra, però «només
complements estètics, més personalitzacions de cada base i, sobretot, millores
en la roba i l'equipació» (lot 7), i les pantalles de **títol i creació passen al
pergamí** (dins el lot 1 i el punt 5 del motor).

Estat (01/10, tarda): **SPRITES-01 lliurat** (punts 1 i 2 del motor); l'extensió
encara no apareix a la sessió (Oriol la instal·la); cap generació gastada.

## 0. Com funciona el circuit

```
PixelLab (extensió a l'app de Claude, token al clauer)
   │  create_* → id → get_* fins a «completed» → URL de descàrrega
   ▼
guardar_en_proyecto → MotorGraphico/assets/textures/pixellab/<lot>/…   (brut)
   ▼
tools/pixellab/importar_*.py → atles del motor + manifest (.png + .json)
   ▼
Galeria de revisió (Claude outputs/pixellab/lots/<lot>/index.html) → Oriol diu sí/no
   ▼
Captures del joc compilat amb l'art dins
```

Cada lot es genera, es revisa i s'importa abans de passar al següent. Res es
gasta sense una prova d'estil aprovada (lot 0).

## 1. Convencions del motor (el que PixelLab ha de respectar)

| Peça | Mida al motor | Com es demana a PixelLab |
| --- | --- | --- |
| Baldosa isomètrica | rombe 64×32 al mapa; cel·la 128×64 a `terreno_mundo.png` (es dibuixa a mida 64×32) | `create_isometric_tile(size=64, tile_shape="thin tile")` o `create_tiles_pro(tile_type="isometric", tile_size=64)`; s'escala ×2 veí més proper a la cel·la |
| Actor (PNJ, monstre) | cel·la 64×64, cos de ~40 px, peus a la fila 60; fila de 12 frames: amunt 1–3, dreta 4–6, avall 7–9, esquerra 10–12 (pas, quiet, pas) | `create_character(size=44–48, n_directions=4, view="low top-down")` + `animate_character(walk)`; l'importador tria 3 frames per direcció i munta la fila |
| Monstre gran (elit, cap) | cel·la 96×96 o 128×128, el motor escala per entitat | `size=64–96` |
| Prop / objecte | cel·la 64×64, base a la fila 61 | `create_map_object(width=64, height=64, view="high top-down")` |
| Pickup | cel·la 32×32 dins l'atles de 64 (escala de lectura) | `create_map_object(32, 32)` |
| Icona de la UI | 16×16 (es llegeix a ×1 i ×2) | `create_image_pixen(32, 32, no_background)` → reduir a 16 amb `unzoom_image` o a mà |
| Panell de la UI | qualsevol mida, 9-slice | `create_ui_asset(192–688 px)`; l'importador marca els talls |
| Retrat | 64×64 bust | `create_image_pixen(64, 64)` o `create_portrait_character` (20 gens, només si cal) |
| Paleta | pergamí: tinta #2B1D12, paper #E2CD9C, cuir #4E321D, carmí #9A2A1C | `style_image` amb `S3_tema_pergamino.png` o `color_image_url` |

Vista: el món és isomètric 2:1 vist des de dalt-davant; els actors actuals són
"low top-down" (es veu la cara de front mirant avall). Totes les peces es demanen
amb `outline="single color black outline"`, `shading="basic shading"`, `detail="medium detail"`
per quadrar amb els sprites de capes que ja hi ha.

## 2. Lots i cost estimat

Costos segons la documentació de PixelLab (01/10): personatge estàndard 1;
animació de plantilla 1 per direcció; `create_image_pixen`/`pixflux` 1;
`create_ui_asset`, estats, mode pro i retrats 20–40; `unzoom`/`correct`/`reduce` 0,1;
tiles i objectes de mapa «no especificat» (es mesura al lot 0).

| Lot | Contingut | Gens. (est.) | Peça del motor |
| --- | --- | --- | --- |
| **0 · Prova d'estil** | 1 panell de pergamí, 2 baldoses, 2 props, 1 monstre (slime) amb caminar, 1 PNJ (Luisarda) amb caminar, 4 icones, **1 peça de roba** (un gipó de cuir sobre el cos base, per decidir el mètode del lot 7) | ≈ 65 | importadors escrits contra la sortida real; galeria |
| **1 · UI pergamí** | full (hoja), papir amb corrons, tapes de cuir, pestanya activa/inactiva, tecla, botó de lacre, pista de barra; peces petites (segell, cinta, tatxa, xinxeta, fletxa, filet); **títol i creació**: capçalera del títol (nom del joc en tinta i filet), marc de l'aparença, fitxa d'atributs, orla de la pàgina de resum | ≈ 290 | `BancoMateriales` 9-slice dins `Pergamino::fabrica()` (procedimental de reserva); títol i creació dibuixats pel pergamí (punt 5) |
| **2 · Icones** | les 56 d'`iconos.json` + 10 que falten (accions de combat, moneda, mapa…) | ≈ 90 | mateix manifest; `gen_iconos.py` passa a muntar des de PNG |
| **3 · Terreny** | les 36 baldoses planes de `terreno_mundo.png` + ciutat (empedrat, terra, herba, aigua, moll de fusta), interiors (taverna, sastreria, església, mercat: fusta, pedra, catifa), camí del nord, mapamundi | ≈ 120 | `importar_tiles.py` → cel·les 128×64; el TMX no canvia |
| **4 · Props i objectes** | portes/sortides (avui rombes tenyits), arbres i arbustos, pous, fanals, tanques, parades de mercat, barrils, cartells; pickups (pocions, claus, armes) | ≈ 100 | `RegistroSprites` + atles de props (els 33 de `props_p0` entren ja) |
| **5 · Monstres** | 15 faccions × (criatura + cap) = 30 (l'elit es fa amb tint i escala de la criatura, com fa `Enemy` avui); caminar (4) + repòs (4) + creació (1) = 9 cadascun | ≈ 270 | `RegistroSprites`: `monsterId` → full; `Enemy` amb atles propi |
| **6 · PNJ amb cara** | Luisarda, Ben Kafka, Griffin, Skilla, Venides, Aigren, Xila, la nena del gat, la naga del riu, el duende, la saga, el verdugo (12) + retrat de cadascun | ≈ 150 | `RegistroSprites`: `objectId` → full; retrat al diàleg (rètol de cuir) |
| **7 · Roba, equipació i personalitzacions** | ≈ 110 peces noves per al sistema de capes: 30 de torso (gipons, cotes, túniques, vestits amb més volum i plecs), 15 de cap (capells, caputxes, diademes), 6 capes, 12 armes i 5 de mà esquerra, 8 pentinats nous (els colors els posa el generador), 12 complements (bufanda, bossa, cinturó, ulleres, joies) i 20 personalitzacions de base per raça (banyes, orelles, marques, cues) | ≈ 280 | `tools/pixellab/importar_prenda.py` → capa de 12 frames a `personajes/equipo/`, `pelo/` o `razas/rasgos`; `personajes.json` i `equipo_catalogo.json` la llisten; el motor no canvia (ja munta capes) |
| Marge | repeticions i proves | ≈ 300 | |
| **Total** | | **≈ 1.665 de 2.000** | |

Ordre: 0 → 1 → 4 → 3 → 7 → 5 → 6 → 2 (la UI i els props es veuen a cada pantalla;
la roba es veu a cada partida des de la creació; les icones es poden deixar
per al final perquè les actuals ja compleixen).

### Com entra una peça de roba (lot 7)

Les capes d'avui es pinten per **plantilla**: la peça es dibuixa en coordenades
del frame quiet de cada direcció i `actores_capas.pintar_plantilla` la baixa a
cada frame del pas (`dy`) i fa l'esquerra com a mirall de la dreta. PixelLab
només ha de donar la peça **quieta en tres direccions** (avall, amunt, dreta);
l'importador fa la resta igual que el generador. Dos mètodes, es tria al lot 0:

1. **Vestir el cos base**: `create_character` amb `personaje.png` com a imatge
   de referència i el text de la peça; la peça = diferència entre el resultat i
   el cos base, frame a frame (val si PixelLab respecta el cos; si el redibuixa,
   la diferència és soroll).
2. **Peça sola**: `create_image_pixen(64, 64, no_background)` «gipó de cuir
   aïllat, vista frontal, sense cos» ×3 vistes; l'importador la col·loca sobre
   la plantilla del cos i retalla el que el cos tapa (3 gens per peça; sempre
   funciona, però cal ajustar la posició a mà la primera vegada de cada ranura).

Les cames i els peus es queden procedimentals (la cama es mou a cada frame i
una peça quieta no la segueix bé). Els complements entren a les ranures que hi
ha (cap, torso, capa) fins que calgui una ranura nova al motor.

## 3. Què canvia al motor (sense PixelLab es pot fer ja)

1. **`Game::RegistroSprites`** (sense GL, `motor_level`) — **FET, SPRITES-01 (01/10)**:
   llegeix `assets/textures/sprites.json` — fulls (imatge, cel·la, columnes, files),
   actors per id i per raça (`race:<id>`), props per id, àlies i patrons per prefix
   o sufix. Substitueix les taules fixes `raceNpcFrame` i `boundingtonStoryFrame`
   d'`ApplicationNivel.cpp`. 5 proves. Els monstres (`monster:<id>`) entren al lot 5
   amb la mateixa taula.
2. **`Render::BancoAtlas`** (GL, `motor_core`) — **FET, SPRITES-01**: un `TextureAtlas`
   per full (o per fila d'actor), carregat sota demanda per `TextureManager`.
   `makeMarker` el consulta: PNJ amb full propi caminen amb `HojaPersonaje::animar`,
   props i pickups amb dibuix propi (`props_p0` i `pickups_p0` ja surten al joc);
   42 PNJ col·locats sense full porten roba prestada per àlies.
3. **`Pergamino::BancoMateriales`** (sense GL): `fabrica()` fa 9-slice d'una imatge
   de `assets/textures/pergamino/` quan hi és; si no, el procedimental d'avui.
   Amb proves (mides exactes, cantonades intactes, cos repetit).
4. **`tools/pixellab/`**: `importar_personaje.py`, `importar_tiles.py`,
   `importar_props.py`, `importar_ui.py`, `importar_prenda.py`, `galeria.py`.
   Es validen amb `tools/validar_sprites.py`.
5. **Títol i creació al pergamí**: `TitleScreen` i `CharacterCreationScreen` deixen
   el fons fosc i es dibuixen amb les llistes de traços de `motor_pergamino`
   (títol sobre papir amb botons de lacre; la creació com a pàgines del quadern,
   una pestanya per pas, l'aparença dins un marc i el resum en una orla). Sense
   PixelLab ja es pot fer amb els materials procedimentals; el lot 1 hi posa les
   imatges. No es toquen `NewGameFlow` ni `CharacterCreation` (la lògica dels 15
   passos): només qui els pinta.

## 4. Riscos

- Costos «no especificats» (tiles, objectes): es mesuren al lot 0 abans de decidir
  quantes baldoses.
- L'estil de PixelLab pot no quadrar amb els sprites de capes: `style_image` amb
  `actores_capas.png` i, si cal, `correct_pixelart`/`reduce_colors` a la paleta.
- Les animacions de plantilla són per a humanoides i quadrúpedes (ós, gat, gos,
  cavall, lleó); un slime o un espectre va amb `animate_character(mode="v3")`
  (2–4 gens) o amb repòs només.
- Els monstres grans no caben a 64×64: cel·les pròpies i `setVisualScale`.
