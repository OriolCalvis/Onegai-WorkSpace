# ISO-01 — Estil Habbo + Stardew amb un kit procedural (03/10/2026)

L'Oriol proposa l'estètica d'Habbo Hotel (isomètrica 2:1 nítida, mobles que es
giren en 2 o 4 posicions) amb els edificis i decorats de Stardew Valley (cases
acollidores, plenes de detalls, paleta càlida), i que cada peça es pugui girar
(una imatge per cada orientació). Decisions (AskUserQuestion): es gira cada peça,
com els mobles d'Habbo (la càmera queda fixa; el generador orienta les portes
cap al carrer); primera prova = comparar camins amb la mateixa casa en 4 vistes.

- `habbo_01_comparacio_camins.png`: camí 1, kit procedural en Python
  (`tools/iso/`): les 4 vistes surten exactes i coherents, 0 generacions; camí
  2, PixelLab Pro Flash girant la casa pixen: les vistes canvien de càmera i
  tornen a pintar la llosa (4 generacions); camí 3 (PixelLab amb el prompt
  d'estil) pendent de tenir el Mac connectat.
- `habbo_02_kit_edificis_4_vistes.png`: caseta 2x2, casa 3x2 de pissarra,
  taverna 4x3 (porxo, terrassa, bótes, rètol), ferreria 4x3 (forja oberta amb
  foc, enclusa, abeurador, eines) i botiga 3x3 (aparador amb tendal, caixes de
  fruita), cadascuna en les 4 orientacions sobre el rombe de la seva empremta.
- `habbo_03_kit_temple_guardians.png`: temple octagonal amb cúpula de coure,
  fanal de cristall, anell flotant i dues torres romboides de punta de cristall
  (la geometria d'Aegroum), 7x5, en 4 vistes.
- `habbo_04_carrer_kit_al_joc.png` i `habbo_05_carrer_i_placa_kit_vs_blocs.png`:
  el carrer del Casco Antiguo amb les cases del kit orientades cap al carrer
  (rajoles edifici_o0..o3), la ferreria, la taverna i la botiga del kit; al
  costat, les cases de blocs que queden. `habbo_00_abans_pixellab.png`: el
  mateix carrer amb les peces de PixelLab (lot 14).

Com es fa: `python3 tools/iso/exportar_kit.py && python3 tools/pixellab/importar_monumentos.py
&& python3 tools/gen_ciudad.py`.

# ISO-02 — Tota Boundington amb el kit (03/10/2026)

L'Oriol tria el kit procedural («m'encanta! apostem per aquest nivell artístic
100%») i «tota la ciutat, cases i locals»: totes les cases i tots els locals
dels 4 mapes surten del kit, per barri, i cada local amb el seu ofici i la
porta cap al carrer. Adeu a les cases de blocs.

- `habbo_06_districte_comercial.png`: cases populars, nobles de pissarra, el
  banc de coure amb balcons i lucanes, el riu i l'òpera.
- `habbo_07_casco_antiguo.png`: el Casco Antiguo dens, amb la taverna i la
  seva terrassa.
- `habbo_08_catedral_i_banys.png`: la catedral en ruïnes (PixelLab) envoltada
  de cases del kit; els banys amb la cúpula sobre tambor i la llanterna.
- `habbo_09_barrios_altos.png` i `habbo_10_casa_de_te.png`: cases nobles als
  jardins; la casa de te amb para-sols.
- `habbo_11_barrio_militar.png`: barracons en filera, l'armeria i el castell.
- `habbo_12_pico_dragon.png` i `habbo_13_barriada.png`: cabanes de fusta;
  barraques de taulons amb teulada de llauna o de tela, roba estesa i fogueres.
- `habbo_14_surysal.png`: les tendes del vado (de paret, pavelló rodó, amb
  tendal), més altes que un personatge.
- `habbo_15_families_4_vistes.png` i `habbo_16_oficis.png`: les famílies i els
  11 oficis del catàleg (taverna, ferreria, botiga, mercat, joieria, banc,
  sastreria, posada, banys, casa de te, armeria).

Què diu què: `tools/iso/cataleg_kit.py` (famílies, mides, variants, oficis, la
porta que diu l'ofici, el terra de cada família), `tools/gen_ciudad_kit.py`
(orienta els locals i les cases cap al carrer). Per refer-ho tot:
`python3 tools/iso/exportar_kit.py && python3 tools/iso/rajoles_kit.py &&
python3 tools/pixellab/importar_tiles.py assets/textures/pixellab/tiles.json &&
python3 tools/actualizar_tileset_tmx.py && python3 tools/pixellab/importar_monumentos.py
&& python3 tools/gen_ciudad.py`.
