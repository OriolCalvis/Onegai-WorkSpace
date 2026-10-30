# PixelLab · Lot 0 (01/10/2026) — les primeres generacions, dins del joc

**17 generacions gastades de 2.000.** Tot el que hi ha aquí ha sortit de PixelLab
avui i ja es veu al joc compilat.

## Com s'hi arriba (important per a les sessions següents)

PixelLab es crida des del **navegador integrat de l'app de Claude** (la xarxa del
teu Mac, no la del contenidor): s'obre `https://api.pixellab.ai/mcp/docs` i des
d'aquella pàgina es fa `fetch` a `/mcp` (mateix origen, sense CORS) amb el token
Bearer, en JSON-RPC (`initialize`, `tools/list`, `tools/call`). Les baixades
venen pel mateix camí en base64. **L'extensió .mcpb no cal.** El token no es
guarda en cap fitxer.

## Què s'ha generat

| Peça | Eina | Cost | On és al joc |
|---|---|---|---|
| Slime (bola) | `create_image_pixen` 48×48 | 1 | `pixellab_personajes` fila 1 (bot al caminar) — sala d'exemple |
| Slime humanoide (descartat) | `create_character` | 1 | `lote0/slime_character/` — lliçó: `create_character` força un cos |
| Luisarda, 4 direccions | `create_character` size=40 | 1 | `pixellab_personajes` fila 2 — sastreria |
| Luisarda caminant (6 frames × 4) | `animate_character` template walk | 4 | zancades de la fila |
| Empedrat i herba | `create_isometric_tile` size=64 | 2 | `terreno_mundo.png` gid 17 i 1 — ciutat |
| Parada de mercat i pou | `create_map_object` 64×64 | 2 | `pixellab_props` — ciutat (24,30) i (35,34) |
| Panell de pergamí 192×128 | `create_image_pixen` | 1 | pendent: `BancoMateriales` 9-slice |
| 4 icones (espasa, escut, poció, moneda) | `create_image_pixen` 32×32 | 4 | pendent: `gen_iconos.py` |
| Gipó de cuir sobre el cos base | `edit_image_pixen` | 1 | prova del lot 7: **vàlida** (silueta intacta píxel a píxel; la peça s'extreu per diferència) |

Mesures apreses: `create_character size=40` → llenç 56×56 amb cos de ~40 px
(encaixa a la cel·la de 64); `size=48` → 68×68. `create_isometric_tile size=64`
→ bloc 64×64 amb la cara superior a y=23..55. Rajoles i objectes de mapa costen
1 generació cadascun.

## Com entra al joc (tot per dades)

- `tools/pixellab/importar_tiles.py` — cara superior del bloc → cel·la 128×64 del gid.
- `tools/pixellab/importar_props.py` — PNG → fulla de props 64×64 (base a la fila 61) + manifest + alta a `sprites.json`.
- `tools/pixellab/importar_actores.py` — quiet en 4 direccions + cicle de caminar → fila de 12 frames (mirall per a l'esquerra, bot per a bitxos rodons, zancades triades per obertura de cames).
- `assets/textures/pixellab/{tiles,props,actores}.json` diuen què s'importa; `lote0/generaciones.json` apunta cada generació (id, eina, cost).
- Un actor de PixelLab amb el mateix id que un de capes **el substitueix** només per ser en una fulla posterior de l'índex (Luisarda).

## Captures

`03_panel_iconos_prenda.png`, `04_baldosas_props_luisarda.png` (el que ha sortit
de PixelLab), `05_juego_slimes.png`, `06_juego_ciudad_pozo_parada.png`,
`07_juego_luisarda_sastreria.png` (dins del joc).

## Comprovacions

`test_registro_sprites` 5/5; `test_object_catalog`, `test_validar_todo`
(connectivitat de la ciutat amb el pou i la parada), `test_aventuras_monsterid`
OK; la resta de `ctest -L gl-free` amb els mateixos fallats de sempre.
