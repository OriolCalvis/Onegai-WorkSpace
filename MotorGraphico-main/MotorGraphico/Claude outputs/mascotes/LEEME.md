# MASC-01 — Mascotes màgiques a Boundington (05/10/2026)

L'Oriol demana animals de companyia amb trets màgics per donar més vida a la
ciutat. Decisions (AskUserQuestion): PixelLab per als animals (el mateix
aspecte que els personatges) i la màgia per codi (llum a terra, partícules,
anells), i les vuit mascotes, una parella per barri.

| Mascota | Barri | Com és | Màgia |
| --- | --- | --- | --- |
| Gat d'alquímia | Casco Antiguo | gat gris amb la cua acabada en cristall hexagonal | hexàgons de cristall que pugen, llum cian |
| Gos de fanal | Distrito Comercial | gos amb un collar de llautó i un fanalet de cristall | espurnes de llum |
| Lleonet de llautó | Barrios Altos | cadell de lleó amb la crinera de llautó encantat | motes daurades |
| Llebrer de la Llum | Barrio Militar | llebrer blanc amb runes octogonals | runes octogonals que pugen |
| Dragonet | Pico Dragón | dragonet vermell amb el ventre i la cua de brasa | brases i bufades de fum |
| Guineu del vent | La Barriada | guineu taronja (esquiva: fuig del jugador) | anell de vent que li volta i espurnes verdes |
| Poni de boira | Surysal | poni gris amb la crinera de boira (esquiu) | boira que es queda on trepitja |
| Osset de molsa | Surysal | cadell d'ós amb molsa i bolets que brillen | espores blaves |

Passegen pel voltant de casa seva (la cel·la de l'objecte `mascota_<id>`),
descansen, dormen a casa, i quan el jugador s'hi acosta les afectuoses hi van
i s'asseuen o borden al seu costat; les esquives (guineu, poni) se n'aparten.
Amb la capa de vida, els coloms tornen a posar-se als terrats: ara al carener
de les cases del kit.

- `masc_01_vitrina.png`: les vuit mascotes amb la seva màgia i la Skilla a escala.
- `masc_02..08_*.gif`: cadascuna al seu barri (captures del joc).

Com es refà: `python3 tools/pixellab/importar_mascotas.py` (fulla i manifest des
de `assets/textures/pixellab/mascotas.json` i els frames de `lote15/`), i
`python3 tools/gen_ciudad.py` (posa les mascotes per barri).
