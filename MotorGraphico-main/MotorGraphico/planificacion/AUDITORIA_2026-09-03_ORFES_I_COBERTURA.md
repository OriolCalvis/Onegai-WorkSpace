# Auditoria del 3/9/2026 — codi orfe i xarxa de proves

> **DOCUMENTO HISTORICO — congelado.** No se actualiza: describe l'auditoria de codi orfe i xarxa de proves del 3 de setembre, foto del 03/09/2026. Els orfes i l'abast es mesuren ara amb `tools/analizar_duplicados.py` (MUERTO/DISIMILAR) i `tools/medir_alcance.py`. De las 15 citas de codigo que contiene, 7 cambiaron despues y se dejan como estaban a propósito: son parte de la foto, no una afirmacion sobre el presente. (H8-04, 29/09/2026.)

Mesurada sobre l'arbre de treball, no sobre el pla. Tres preguntes:
què no arriba al joc, què no té xarxa de proves, i quant queda de la
fase C de veritat. Autor: Claude (Cowork). Els números són reproduïbles
amb els mètodes que es descriuen a cada apartat.

## 1. On és la fase C

Dels **62 jornades** de la fase C (24 paquets):

| | jornades | paquets |
| --- | ---: | ---: |
| Tancats | 19 | 9 |
| Amb feina feta però oberts | 20 | 9 |
| Sense tocar | 23 | 6 |

**Els 20 jornades d'«obert» no són 20 jornades de feina.** Cinc paquets
(C-09, C-10, C-11, C-21, C-22 — 9 jornades) diuen literalment el mateix:
*fet i verificat localment; el tanca el pas per CI*. No els bloqueja
programar res, els bloqueja **una sola porta compartida**. Passar CI una
vegada tanca nou jornades de cop, i és el moviment més barat del tauler.

Els altres quatre sí que tenen feina: C-05 (executar la comprovació sota
ASan), C-07 (no té prova possible fins que hi hagi arnès d'`Application`),
C-14 (només la primera llesca) i C-20 (nucli fet).

Sense tocar: **C-15** (2 j), **C-16** (4 j), **C-17** (6 j), **C-18** (2 j),
**C-23** (4 j), **C-24** (5 j).

## 2. Què segueix sense arribar al joc

Mètode: tancament d'inclusions des de `examples/juego.cpp` i dels `.cpp`
de les llibreries que `juego` enllaça (`motor_core`, `motor_math`,
`motor_level` i les seves dependències públiques). 102 capçaleres al
repositori; **14 no hi arriben**.

Set són esperades (`Editor/*`, `Core/Check.h` que és utilitat de proves,
i `Level/LevelDocumentBridge.h`, escrita avui i encara sense cridador).
Les altres set són el problema que la fase C existeix per resoldre:

| capçalera | línies | paquet | estat |
| --- | ---: | --- | --- |
| `Engine/Scene/Scene.h` + `SceneManager.h` | 242 | C-16 | sense tocar |
| `World/World.h` + `EntityHandle.h` | 241 | C-17 | sense tocar |
| `Core/Memory/HandlePool.h` | 147 | C-18 | sense tocar |
| `Level/WorldTopology.h` | 137 | **cap** | — |
| `Project/DocumentRef.h` | 29 | **cap** | — |

Són ~630 línies escrites, provades i que la partida no executa mai, més
**166 línies que el pla no contempla**: `WorldTopology` només el toquen
`demo_mundo` i el seu test, i `DocumentRef.h` només `StrongIdTests.cpp`.
O tenen amo dins de C-22 («retirada de l'API morta») o cal decidir què
en fem; ara mateix són dos conceptes vius que ningú ha reclamat.

**Matís sobre el post-processat (C-23).** `FogOfWar`, `ColorLUT`,
`DynamicLightMap` i `Framebuffer` SÍ que arriben al joc per inclusió:
`IsometricRenderer` els exposa a la seva interfície (`setFog`, `setLUT`,
`setPostFX`). El que passa és que **`Application.cpp` no crida cap
d'aquests mètodes ni una sola vegada**. No és codi desconnectat, és codi
connectat i mai encès — que és pitjor de detectar, perquè compila,
enllaça i no es nota.

## 3. La xarxa de proves del camí que es juga

Mètode: per a cadascuna de les **57 fonts de producció** que enllaça
`juego`, es mira si alguna prova de `tests/` o alguna demo registrada a
CTest inclou la seva capçalera. És un indicador per inclusió DIRECTA:
infravalora la cobertura real (una demo que inclou `HudManager.h` també
exercita `HudElement`), però no la sobrevalora mai.

Abans d'aquesta sessió: **25 de 57 sense res**. Ara: **22**.

La diferència és una troballa amb solució immediata: hi havia **14 demos
que es construïen i que ningú executava**. Tres són GL-free (enllacen
glad però no criden cap funció GL) i s'han promogut a CTest després de
comprovar-ho amb el criteri que el propi `CMakeLists.txt` documenta —
codi de sortida 0 sense context GL i dues execucions seguides amb sortida
idèntica:

- `demo_bitmap_font`, `demo_hud_update`, `demo_enemy_sprite`
- cobreixen `Enemy`, `HudManager` i `HudWidgets` (i, per inclusió
  transitiva, `HudElement`, `Entity` i `AnimatedEntity`)

Les onze restants necessiten finestra o `tools/` al directori de treball;
no es poden promoure sense decidir abans com se'ls dona un entorn.

**El forat que queda importa més que la seva mida.** De les 22 sense
xarxa, dinou són classes GL (`Shader`, `Window`, `SpriteBatch`,
`IsometricRenderer`, el post-processat…) que necessiten la maquinària de
C-23/C-24. Les altres tres no tenen excusa:

- **`src/Engine/Application.cpp`** — l'orquestrador sencer, zero proves.
  És exactament per això que C-05 i C-07 estan implementats i **no es
  poden tancar**: la seva prova viu dins d'`Application`, i allà no hi ha
  res on penjar-la. Els dos bugs més greus del joc van sobreviure per
  aquí, i el pla ja ho diu al «Per què» de C-24.
- **`src/Game/Morality.cpp`** — GL-free, sense dependències, i el «Fet
  quan» de C-24 el nomena explícitament. És dels pocs que es podrien
  cobrir avui mateix.

## 4. Conclusió operativa

Tres coses, per ordre de rendiment:

1. **Passar CI una vegada.** Tanca C-09, C-10, C-11, C-21 i C-22 — nou
   jornades — sense escriure ni una línia. És l'única cosa que separa
   cinc paquets del verd.
2. **C-16 és el coll d'ampolla estructural.** 4 jornades que desbloquegen
   C-17 (6 j) i C-23 (4 j), i que són el que falta perquè C-07 pugui
   tenir prova i perquè C-24 tingui on agafar-se. És l'únic paquet lliure
   de la fase C amb dependents; qualsevol altre ordre deixa 15 jornades
   esperant.
3. **Decidir sobre `WorldTopology` i `DocumentRef.h`.** 166 línies de
   concepte viu que el pla no contempla. La decisió de la fase C era «cap
   concepte amb dues implementacions vives»; això n'és la variant
   silenciosa: una implementació viva que no fa servir ningú.
