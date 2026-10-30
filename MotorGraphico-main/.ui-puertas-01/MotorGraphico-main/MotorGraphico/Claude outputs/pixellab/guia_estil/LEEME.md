# Guia d'estil de Boundington (01/10/2026, ESTIL-01)

La guia sencera (bíblia d'estil, gramàtica d'Aegroum, barris a la manera de
Malaz, peces i procediment, monuments, pas a pas, pressupost, decisions) és
el document «Guia d'estil de Boundington» de l'Oriol a Claude; aquesta
carpeta en guarda les imatges i el resum operatiu que necessiten els scripts.

## Les regles en deu línies

1. L'àncora són els personatges (`razas_andando`, `pixellab_personajes`):
   48 px, contorn d'1 px fosc i càlid, 2–3 tons plans per material, detall mitjà.
2. Tot el que no és un personatge es genera amb `create_map_object`
   (view high top-down, outline single color outline, shading basic
   shading, detail medium detail). Baldoses: `create_isometric_tile`.
   Personatges: `create_character` + plantilla d'andar.
3. Tots els prompts acaben amb la fórmula: `16-bit pixel art with clean flat
   shading, crisp 1-pixel dark outline, 2-3 tones per material, simple
   readable shapes, no texture noise, single object centered, no ground,
   transparent background`.
4. Paleta mestra: `tools/pixellab/paleta_mestra.py` →
   `assets/textures/pixellab/estil/paleta_mestra.json` (20 rampes, 117 colors).
5. Mapatge a la paleta en importar: `tools/pixellab/aplicar_paleta.py`.
6. Fons: `tools/pixellab/limpiar_fondo.py` (fons pla, tauler, terra).
7. Blocs vells: `tools/pixellab/graduar_paleta.py` abans del mapatge.
8. Full de contacte abans d'importar: `tools/pixellab/contacto.py peca.png`.
9. Escala: amplada del dibuix = (ample+alt)·32 de l'empremta ±20 %; portes ≥ 44 px.
10. Gramàtica d'Aegroum: calcària càlida amb sòcol fosc, noguera, terracota a
    les cases, coure verdet als públics, pissarra als nobles, llautó, fanals
    de llum blanca-blava, veles i penells, alambins. Altres cultures: mateixa
    bíblia, altra gramàtica.

## Imatges

- `personatges_ancora.png`: els personatges a 4×.
- `paleta_mestra.png`: les 20 rampes.
- `cmp_estils_1x.png`: la prova dels tres procediments (pixen, map_object, Pro Flash).
- `monuments_prova.png`: arena i biblioteca, pixen vs map_object.
- `kit_moodboard.png`: el kit de proves ARTE-03b (gramàtica; estil descartat).

## Lots de prova (no entren al joc)

`assets/textures/pixellab/lote12/` (cases pixen, 35) i `lote13/` (kit: peces
modulars i monuments pixen): material de referència de la gramàtica, no de
l'estil. Pressupost: 462 de 2.000 generacions.
