# PixelLab · Bloques (01/10/2026) — edificis amb volum: parets verticals amb portes i finestres

**23 generacions més (304 de 2.000 en total; en queden 1.696).**

## Què canvia

Fins ara un edifici era la seva teulada pintada al terra (els tiles
`viejo_casa_piedra`, `viejo_tienda`, `viejo_muralla`…). Ara cada cel·la
d'aquests tiles aixeca un **bloc**: teulada a l'alçada del mur i les dues
cares que miren a càmera, en vertical. Les cares que no es veuen les tapa el
bloc veí (es pinten en ordre x+y com tot el que hi ha al mapa), així que
**només es veuen per fora de l'edifici**. Les portes i les finestres van
integrades a la paret:

- **porta**: la cara que mira a la cel·la on el nivell té un objecte
  `puerta_*` (el forat d'una cel·la davant de cada edifici de Boundington);
- **finestra o paret llisa**: la resta de cares exposades, amb un repartiment
  fix (la mateixa ciutat, les mateixes finestres);
- els murs i els setos no tenen ni portes ni finestres.

Quan la porta ja és a la paret, l'objecte `puerta_*` del forat ja no es
dibuixa com una porta solta (sortia duplicada, davant de la de la paret):
ara és un **llindar** (`umbral_puerta`, un prop pla a terra, 1 generació)
on el jugador trepitja per entrar. Les sortides `salida_*` dels interiors no
canvien. Els blocs just davant del jugador es pinten translúcids (com els
tiles sòlids d'OcclusionRules, una fila més) perquè no l'amaguin en passar
darrere d'una casa.

Els interiors guanyen de pas: l'anell `viejo_muralla` de cada habitació és
ara un mur de pedra d'una banda (`06_interior_posada_muros.png`).

## Les 20 generacions del lot 8 (+ 1 llindar)

`create_isometric_tile` (size 64, forma «block», 1 generació cadascun):
7 materials × 3 blocs (`pp` cares llises, `dv` porta a l'esquerra +
finestra a la dreta, `vd` al revés) per a pedra, fusta (entramat),
noble, castell, església i xabola; murs i setos només `pp`
(`01_bloques_lote8_20_generaciones.png`; 2 proves de forma prèvies no
s'usen però compten). `tools/pixellab/importar_bloques.py` retalla cada cara
del bloc que la té i compon les 9 combinacions `<izq><der>` per material,
en una o dues bandes (64×96 per cel·la, base del bloc a l'última fila):
**72 frames** a `assets/textures/pixellab_bloques.png`
(`02_umbral_y_hoja_compuesta.png`).

Quin tile aixeca quin bloc ho diu `assets/textures/bloques.json`
(material, bandes, si té finestres i si té portes): es pot canviar sense
tocar C++.

## Motor

- `Game::Bloques` (`include/Game/Bloques.h`, GL-free): llegeix `bloques.json`
  i planifica, per a cada mapa amb el tileset `terreno_mundo`, quina cel·la
  aixeca bloc i amb quines cares. `test_bloques` (4 casos, etiqueta gl-free).
- `Render::BloquesNivel`: una `StaticEntity` per bloc amb la fulla
  `pixellab_bloques` del BancoAtlas, recolzada a la base del rombe com un
  personatge, a la cua del renderer; `atenua()` cada frame.
- `Application`: `init()` el carrega; `loadLevel()` munta els blocs **abans**
  dels marcadors (les portes surten del World, no del LevelDefinition, que
  ja s'ha mogut a la sessió) i canvia el sprite dels `puerta_*`/`on_puerta_*`
  que ja són a la paret pel llindar; `render()` atenua; `shutdown()` desmunta.
- Nivells: els 15 cartells de Boundington i el de la casa de te de
  `ciudad_este` passen d'estar davant de la porta (tapaven el forat) a
  estar al costat de l'entrada (`04_boundington_sastreria.png`,
  `05_puerta_en_la_pared_y_umbral_x3.png`). `tools/conectividad.py` segueix
  a 100 % amb totes les portes abastables.

## IMPORTANT: cal recompilar

El `build/juego` del Mac és de les 07:29 d'avui, anterior a totes les
entregues d'avui: ni els tiles de PixelLab al terra ni els blocs es veuen
sense recompilar (la correcció de files del tileset i els blocs són C++, i
`build/assets` es copia en compilar). Amb el teu flux habitual
(`cmake --build build`) ja surt tot.

## CMakeLists.txt (3 línies, les insereix `aplicar_bloques.sh` amb àncora)

El teu `CMakeLists.txt` porta canvis teus sense commit, així que no se
substitueix: l'script hi afegeix `src/Game/Bloques.cpp` (motor_level, sota
`src/Game/RegistroSprites.cpp`), `src/Render/BloquesNivel.cpp` (motor_core,
sota `src/Render/BancoAtlas.cpp`) i `include(cmake/BloquesTests.cmake)` (sota
`include(cmake/RegistroSpritesTests.cmake)`). Si l'script diu que el fitxer
ha canviat, són aquestes tres línies a mà.

## Verificació

`test_bloques` 4/4; `ctest -L gl-free`: només fallen els de sempre
(test_gdb12_screens, test_g14 enllaç Linux, test_catalog_content,
test_marcar_paquetes, linea_base_core amb els 35 de deute previ; cap de nou);
`tools/linea_base_core.py` no afegeix deute; capturs de Boundington
(plaça i sastreria), interior de la posada, Gongorguma (xaboles) i
Bomengrid (pedra, portes `on_puerta_`).

## Pendent / per decidir

- Les tavernes i mercats dels pobles generats de xaboles són taques de
  `viejo_piedra_vieja` (tile de terra amb col·lisió), no edificis: continuen
  plans amb la porta solta. Si vols, passen a bloc amb un tipus més a
  `bloques.json`.
- Teulades: la de cada material és la del cub de PixelLab (plana a pedra i
  castell, a dues aigües a noble, creu a l'església). Variants de teulada o
  cantonades (2-3 generacions per material) si vols més relleu.
- Els blocs de pedra són força foscos (blau-gris): es poden regenerar més
  clars (3 generacions).
- Els cartells penjants queden al costat de la porta, drets a terra; si els
  vols a la paret caldria un prop «cartell a la paret» (1 generació) i
  posar-los a la cel·la del mur.
