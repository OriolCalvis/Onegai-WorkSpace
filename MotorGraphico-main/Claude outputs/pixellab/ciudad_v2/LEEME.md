# Boundington v2 (01/10/2026, BLOQUES-02) — la ciutat redissenyada

**32 generacions més (336 de 2.000 en total; en queden 1.664).**

## Què hi ha de nou

**La ciutat sencera regenerada** amb les regles noves de `tools/gen_ciudad.py`
(v2), els quatre mapes: centre (Casco Antiguo + Distrito Comercial), Barrios
Altos, Barrio Militar + Pico Dragón + la Barriada, i Surysal.

- Lo humil és petit: cases de 2×2 a 3×3, locals de 4×3 a 5×4; lo gran segueix
  gran (castell, òpera, universitat, biblioteca, església, a **tres plantes**).
- **1, 2 o 3 plantes per edifici**: `bloques.json` reparteix alçades amb pesos
  (pedra: 5/3/1, fusta: 2/5/2, nobles 2-3, locals 2-3) amb un hash fix de la
  cantonada de cada edifici: la mateixa ciutat, les mateixes alçades.
- **Variants de façana per edifici**, també amb pesos: cases de pedra que són
  **cases del sud d'Aegroum** (pedra grisa mullada, xemeneia i cortines
  brodades fins a terra en lloc de porta, com diu el cànon), cases de fusta i
  nobles amb **balcons** a les plantes altes (la planta baixa surt del material
  base), i la muralla amb trams **a mig caure** (variant per trams de 3
  cel·les).
- **Porta decorativa** a cada casa sense porta jugable (una cara que dóna a
  carrer, triada per hash). Ja no hi ha ni una cel·la de col·lisió invisible.
- **Carrers**: la rajola `adoquin_limpio` era herba amb pedres i les avingudes
  semblaven prats; ara és empedrat gris (lote 9, 1 generació; afecta tots els
  mapes que la usen).
- **Detalls**: 12 cartells de negoci amb la seva icona (sastreria, joieria,
  mercat, taverna, posada, ferreria, banc, banys, alquimista, armeria,
  coliseu, casa de te) al costat de cada entrada; banderes de la ciutat a les
  portes, al castell i a la plaça; estendard de paret; deixalles darrere de les
  tavernes i barrils trencats als carrerons; estenedors amb roba entre cases
  (ara són props: abans la cel·la «tendedero» era terra llisa).
- **Coloms**: `Game::Bandada` (sense GL) + `Render::BandadaNivel`. Es posen a
  les teulades, s'espanten quan el jugador s'acosta (2,5 cel·les), volen alt
  (128 px, per sobre de qualsevol bloc) fins a una altra teulada llunyana i
  s'hi tornen a posar; l'ombra es queda a terra. Només a mapes amb 12
  teulades o més (als interiors no n'hi ha). 6 frames per espècie, mirall
  inclòs; de moment només colom (gavina pendent).
- **Gent per barri**: PNJ de farciment amb la hoja de races (alquimistes i
  guardians de la llum —els humans d'Aegroum—, mitjans urbans, nans, elfs) i
  els genèrics amb vestit prestat; més al Distrito Comercial i al Casco
  Antiguo, nobles i criats als Barrios Altos, soldats al Barrio Militar,
  pobres a la Barriada. Els PNJ canònics, portes, cartells i passos del nivell
  anterior es conserven (el generador els recol·loca) i les sortides dels
  interiors tornen a apuntar a la seva porta.

Captures: `00` abans/després del centre, `01`-`06` els quatre mapes sencers
(cosits sense HUD), `07` el joc al creuament d'avingudes, `08` coloms en vol,
`09`-`10` el lot 9.

## Fitxers i cadena

`tools/gen_ciudad.py` escriu directament gids de `terreno_mundo` (ja no cal
`remapear_suelos.py`) i fusiona els objectes del nivell existent: per
regenerar n'hi ha prou amb `python3 tools/gen_ciudad.py` (no cal repetir
`gen_interiores`, `gen_carteles` ni `ambientar_boundington`). Nous:
`Bandada.{h,cpp}`, `BandadaNivel.{h,cpp}`, `BandadaTests`, lot 9 i
`importar_pajaros.py`. L'script `aplicar_bloques2.sh` insereix les tres línies
de CMake amb àncora (com l'anterior): `src/Game/Bandada.cpp` sota
`src/Game/Bloques.cpp`, `src/Render/BandadaNivel.cpp` sota
`src/Render/BloquesNivel.cpp`, `include(cmake/BandadaTests.cmake)` sota
`include(cmake/BloquesTests.cmake)`.

**Cal recompilar** (`cmake --build build`), i les partides desades a la ciutat
queden desfasades (posicions d'objectes): partida nova.

## Verificació

`test_bloques` 5/5, `test_bandada` 4/4, `demo_ciudad` (PNJ i cartell
comprables) OK, `tools/conectividad.py` 100 % als quatre mapes amb totes les
portes abastables, `validar_enlaces` 0 problemes, `validar_todo` OK;
`ctest -L gl-free` només amb els fallats de sempre; `linea_base_core` sense
deute nou.

## Pendent

- Coliseu, caserna, església i altres monuments amb sprite propi visible des
  de fora (gran sprite de PixelLab de fins a 400×400 tallat en tires per
  cel·la perquè s'ordeni bé amb la gent): motor nou (`Monumentos`) + 3-6
  generacions per edifici.
- Gavina i algun ocell més (2-3 gens); gossos i gats a la Barriada.
- Més gent amb cara pròpia per barri (5 gens per PNJ amb caminar).
- La Barriada és monòtona (xaboles vermelles sobre fang): variant de xabola de
  fusta grisa (3 gens) i més deixalles.
