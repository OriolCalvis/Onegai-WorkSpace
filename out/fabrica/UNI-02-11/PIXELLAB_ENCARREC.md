# Encàrrec PixelLab (03/11/2026)

Jo no tinc connexió a PixelLab: no hi ha cap connector per a Claude, ni configurat ni al registre. Hi ha dues maneres de treballar:
- **A:** generes tu (o la IA de PixelLab) i deixes els fitxers a `MotorGraphico/assets/textures/pixellab/lote16/`; jo els importo i els poso al joc.
- **B:** si PixelLab té una API o un MCP amb clau, la configures tu (jo no toco credencials) i em dius el nom de la variable d'entorn.

**Estil (és el mateix per a tot):** `assets/textures/pixellab/estil/ancora_personatges.png` i `paleta_mestra.png`; personatge de referència: `lote0/luisarda`. Contorn fosc, fons transparent.

Els crèdits són limitats, així que l'ordre va de **barat i amb impacte** a **car**. Cada bloc es pot tallar on vulguis.

## P1 — Icones petites (1 generació cadascuna)
| Què | Mida | Quants | Noms / carpeta | Per a què |
|---|---|---|---|---|
| Emblemes dels llocs | 16×16 | 11 | `lote16/iconos/lugar_*.png` (llista a `orquestador/pixel.md` §3b) | Cartells de portes i mapa (avui `emblema: ""`). **Laúd** i **pergamino**: cal la prova dels cinc segons. |
| Tipus de dany | 32×32 | 11 | `lote16/iconos/dano_<tipus>.png` (cortante, contundente, perforante, fuego, frio, rayo, acido, veneno, sagrado, profano, psiquico) | Combat i fitxes |
| Economia del torn | 32×32 | 5 | `lote16/iconos/turno_<accion\|movimiento\|canalizacion\|reaccion\|terminar>.png` | Barra del combat |

## P2 — Pentinats decorats (3 generacions cadascun: abaix, dreta i amunt)
Mètode del lot 7: `edit_image` sobre els fotogrames quiets del cos base (com les peces de `prendas.json`). Jo els extrec per diferència amb `tools/pixellab/importar_prendas.py`. Fes **una tirada** per vista; si no t'agrada, es retoca després.
Proposta (fantasia medieval, que encaixi amb Boundington):
1. `trena_perles`: trena llarga amb perles
2. `monyo_agulles`: monyo alt amb agulles de fusta
3. `cua_cinta`: cua alta amb cinta de color
4. `corona_trenes`: trenes enrotllades al cap
5. `melena_flors`: melena solta amb flors petites
6. `rastes_anelles`: rastes amb anelles de metall
7. `cresta_plomes`: cresta curta amb dues plomes
8. `tonsura`: tonsura de monjo (per al clergat de la Iglesia de la Luz)

Lliura'ls a `lote16/peinados/<id>_<abajo|derecha|arriba>.png`.

## P3 — Elements que amplien el joc (1 generació cadascun)
| Què | Mida | Per a què |
|---|---|---|
| Barricada de fusta i carro bolcat | prop gran | Ocaso; el «cultista de la barricada» (D0022 6.3) |
| Campana gran i campanar petit | prop | Ocaso (avís de Xila, `b3_xila_avisa`) |
| 4 props de la vila de Gorran: cabana, tanca trencada, foguera i rètol | prop | Els «líos» del dia 2 |
| Icones dels objectes de botiga que no en tenen | 32×32 | Botigues (`assets/objects/boundington_objetos.json`, camp de botiga) |

## P4 — Animacions (el més car; només si queden crèdits)
Del que ja estava encarregat el 14/10 (`orquestador/pixel.md` §1, 2 i 2b): `sit` i `lean` per a la tabernera i 2 parroquians; el `walk` de 6 fotogrames per als cossos base humà i elf; i els 3 amics de 757 sense caputxa. Lliurament a `lote16/<id>/` amb els noms de PixelLab (`south.png`, `walk_south_1.png`…).

## Què faig jo quan arribin
Importar (`importar_iconos.py`, `importar_prendas.py`, `importar_props.py`, `importar_actores.py`), afegir-los al joc i als catàlegs, posar els emblemes a `lugares.json` i passar els validadors. T'ensenyaré les captures.
