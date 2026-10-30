# DECO-01 — Els decorats del kit a Boundington (05/10/2026)

L'Oriol diu «Continua» després de les mascotes. El pas següent era fer els
decorats del carrer amb el kit isomètric, amb la llum, les línies i els
materials de les cases. Fins ara eren dibuixos de PixelLab vistos de cara
(el banc, el fanal, la roba estesa).

Ara surten de `tools/iso/decorats.py` en 4 orientacions. La fulla
`kit_decorats` va l'última a `sprites.json`, així que per a un id que ja
existia (banco_plaza, farola, carro…) guanya el dibuix del kit. Els nivells
fets a mà també el fan servir sense canviar res.

| Decorat | On surt | Què té de tecnomàgic o d'ànima |
| --- | --- | --- |
| Fanal d'alambí | avinguda dels monuments i Distrito Comercial | un alambí de cristall amb el broc de llautó |
| Fanal dels Guardians | cantonades de la plaça | octogonal, cristall i caputxa de coure |
| Fanal de ferro | Casco Antiguo, Barrios Altos i Barrio Militar | gàbia de cristall blau |
| Banc amb braser | plaça del Casco | braser de cristall que crema sense fum |
| Banc | molls del riu, mirant l'aigua | — |
| Parada | plaça i portes dels dos mercats | tendal a ratlles amb serrell, caixes de fruita |
| Jardinera, pou, estendard, noray | carrers, plaça, molls | flors de colors, galleda, banderola de la ciutat |
| Bótes, bótes velles, caixes, sacs, deixalles, xarxa | molls, patis, Barriada | aigua de pluja a la bóta, ampolles buides, peix a la cistella |
| Foguera | Barriada i campaments de Surysal | cercle de pedres |
| Roba estesa | entre cases i barraques | una corda de diverses cel·les només porta pal als extrems |
| Carro de caravana (de lona o a ratlles) | corrals de Surysal | quatre rodes i vela sobre arcs |
| Tanca de fusta | horts de Surysal, amb un portell | — |

**Com es posen.**

- **Orientació.** Bancs, parades, carros, la tanca i el pou miren a
  algun lloc: el centre de la plaça, el riu o el carrer amb més espai
  davant. A la plaça, els bancs del nord miren al sud i els del sud al
  nord. Els carros de caravana segueixen el cercle al voltant de la
  foguera.
- **Fanals.** Van arrambats a la paret d'una casa o a l'aigua, mai a la
  muralla ni a tocar d'una porta. Sempre deixen pas: la cel·la de davant i
  les dels costats queden lliures.
- **Parades del mercat.** Es posen a banda i banda de la porta, quan les
  portes ja són al seu lloc definitiu.
- **Accessibilitat.** Es comprova amb els objectes que fan nosa. Totes les
  portes dels 4 mapes s'hi poden atènyer. L'estendard que tapava la porta
  del castell ara flanqueja l'entrada, un a cada costat.

**Fitxers.**

- `deco_01_vitrina.png`: tots els decorats amb la Skilla a escala, i les
  quatre vistes dels que giren.
- `deco_02_placa.png` i `deco_08_placa_detall.png`: la plaça del Casco
  Antiguo.
- `deco_03_caravanes.png` i `deco_04_horts.png`: els corrals de carros i
  els horts tancats de Surysal.
- `deco_05_barriada.png`: la Barriada.
- `deco_06_avinguda.png`: l'avinguda dels monuments.
- `deco_07_barrios_altos.png`: el passeig del riu a Barrios Altos.

**Com es refà.**

- `python3 tools/iso/exportar_decorats.py` fa la fulla, el catàleg
  `assets/objects/decorados_kit.json` amb les orientacions i els trams de
  corda, i l'entrada al projecte.
- `python3 tools/gen_ciudad.py` posa i gira els decorats als quatre mapes.
  Ho fa `tools/gen_ciudad_deco.py`, l'últim pas abans d'escriure cada
  nivell.

**Què queda de PixelLab.** Els arbres, les mates, les flors, els joncs i la
font del vent. La prova d'arbres procedurals quedava per sota dels de
PixelLab. A més, els arbres pintats ja casen amb la meitat «Stardew» de
l'estil.
