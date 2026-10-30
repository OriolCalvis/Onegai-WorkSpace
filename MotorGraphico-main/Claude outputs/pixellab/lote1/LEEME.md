# PixelLab · Lot 1 (01/10/2026) — els materials del pergamí, pintats

**17 generacions més (34 de 2.000 en total).** Els panells del tema pergamí ja
no es fabriquen per algoritme: surten d'imatges de PixelLab retallades en
9-slice (`Pergamino::BancoMateriales`), i si falta una imatge es torna al
procedimental de sempre sense que res es trenqui.

## Què es veu al joc

| Captura | |
|---|---|
| `01_hud_exploracion.png` | Retalls del HUD, rètol de la E i pistes de tecla amb el pergamí de PixelLab (dues variants, la llavor tria). |
| `02_dialogo.png` | El full del diàleg i el rètol de cuir del nom. |
| `03_cuaderno.png` | Tapes de cuir amb cantoneres de llautó, pestanyes i la pàgina de pergamí. |
| `04_pausa_papiro.png` | El papir amb corrons de fusta i tapes de llautó. |
| `05_piezas_a.png`, `06_piezas_b.png` | Tot el que ha sortit de PixelLab, bo i dolent. |

## Què s'ha fet servir i què no

Serveixen (7): dos fulls de pergamí, papir, cuir amb cantoneres, cuir llis
(rètol), pestanya inactiva, llom, segell de lacre. **No serveixen** (10): la
pestanya activa (va pintar un llibre), xinxeta, tatxa, cinta (fons no
transparent) i fletxa (peces de 12–16 px: el model no les clava), l'adorn
(el rombe central es repetiria amb el 9-slice), un cuir amb objectes de més
i un full que va sortir com un rotlle. Tot queda a `lote1/generaciones.json`.

## Com entra (per dades)

- `assets/textures/pixellab/materiales.json` diu quina imatge va a quin
  material, quina neteja (`recorta: "mayor"` treu les figuretes que PixelLab
  pinta al voltant; `filas` retalla el llom) i quins talls del 9-slice.
- `tools/pixellab/importar_materiales.py` escriu `assets/textures/pergamino/`
  (les pells + `materiales.json` del motor).
- `Pergamino::BancoMateriales` (sense GL, `motor_pergamino`, 4 proves) tria la
  pell més concreta per material i paràmetres (p. ex. el cuir sense costures
  del rètol), repeteix vores i cos sense estirar, i enfosqueix el dors de la
  pàgina. `Render::PergaminoGL` li demana cada material en lloc de fabricar-lo.

Lliçó dels talls: el cos del 9-slice es repeteix; els filets i les ombres de
cantell pintats han de quedar dins dels talls (el papir ha passat de 12 a 36
px de costat per això).

## Pendent

- Pestanya activa, xinxeta, cinta, tatxa, fletxa i adorn: continuen
  procedimentals (provar amb `create_image_pro` o demanar-les grans i reduir).
- Els panells grans deixen veure la repetició del cos: demanar pells de
  256–384 px amb més cos llis, o un mode «mirall» al retall.
- Títol i creació al pergamí (punt 5 del pla).
