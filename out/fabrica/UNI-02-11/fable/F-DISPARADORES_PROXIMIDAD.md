# F-DISPARADORES_PROXIMIDAD — activar por cercanía y línea de visión (Fable, 04/11, solo lectura)

Propuesta de Oriol (04/11): condiciones → radio (interior pequeño: al entrar; interior grande: 5; exterior: 10) → sin pared,
edificio, puerta ni «límite a posta» entre el jugador y la casilla activadora (PNJ y objetos pequeños no tapan) → evento.
Amplía `disparador` de F-DISPARADORES.md / D0026. Rutas relativas a MG. Cifras calculadas con TMX + `objects` de los niveles.

**Opinión honesta.** La idea es buena y barata de hacer: el motor ya tiene el Bresenham, la distancia Chebyshev y la
colisión de casilla; solo falta decidir *qué* tapa la vista. Dos problemas reales: (a) hoy **nada distingue un muro de una
farola**: `blocksMovement` lo llevan 960/973 muebles, 211 PNJ, farolas, matas y taburetes; sin un criterio nuevo el radio 10
de la plaza lo taparían pinos y farolas y el de la taberna 14 taburetes. (b) En interiores las paredes **no son tiles**:
son objetos `paret_*` de `mobles_kit` (368 ids; la taberna tiene 0 tiles de colisión). Y un aviso: con «radio + visión» la zona
deja de ser rectángulo; en la plaza, 19 de las 68 casillas pisables de la zona actual **no ven** la casilla (55,53) — hay que
conservar la zona como alternativa o habrá regresiones silenciosas en las 31 cinemáticas con `zona`.

## 1. Qué hay hoy

| Pieza | Dónde | Sirve tal cual |
|---|---|---|
| Línea de visión Bresenham, extremos no cuentan | `src/Game/TableroCombate.cpp:85-106` (`seVen`), misma vista en `PlantillaArea.h:28-32` | sí, pero vive en el tablero de combate; hay que sacarla a una función libre (`Game/LineaDeVision.h`) para no instanciar un tablero |
| Distancia Chebyshev (diagonal = 1) | `TableroCombate.h:18-19` (`distancia()`), niebla `FogOfWar::reveal` radio Chebyshev, `LevelDefinition.h:162` | sí: ya es «la distancia» del juego |
| Colisión de casilla (cualquier capa con `collision`) | `GameSession::tileBlocks` `src/Game/GameSession.cpp:133-150`; fuera del mapa = muro | sí |
| Qué bloquea en combate | `src/Game/GameSessionTablero.cpp:47-61`: tile ∨ objeto `blocksMovement` ∨ enemigo ajeno | **no** para visión: trata farola = muro |
| Catálogo de objetos | `include/Level/ObjectCatalog.h:152-185`: `category {Prop,Enemy,Pickup,Npc}`, `blocksMovement`, `interactable`; sin tamaño ni «tapa vista». Hook `bloquea:vision` existe pero es de estados de combate (`CatalogLoadersObjetos.h:67`) | **falta un campo** |
| Puertas | `puerta_*`/`paso_*`/`salida_*` son `blocksMovement:false` (son transiciones) | la «puerta» de Oriol es la pared TMX del edificio; la puerta-objeto es el hueco, no debe tapar |
| Clasificación del nivel | `assets/levels/*.json`: solo `name`, `map`, `playerStart`, `objects` (+`carriles`/`alturas`/`epoca`); `LevelDefinition.h` sin `interior` | **no existe**; deducible por nombre (`interior_*`, `aldea_*_pN`, `mansion_*`, `casa_*`, `dependencias_*`, minas/cuevas) y por tamaño del TMX |
| Bucle de arranque | `ApplicationCinematica.cpp:88-94` → `laQueEmpieza(nivel, jugador, flag)` `Cinematica.cpp:260-273` | ahí se añade el paso; `app.m_session` ya está a mano para `tileBlocks`/`objectAt` |

Tamaños reales: taberna 15x12=180, casa de té 11x10, herrería 12x10, vacío 13x11, casa del Cojo 18x16=288, iglesia 21x15=315,
dependencias_secta 32x32, mina2_gorran 42x42, ciudad_centro 64x64, ocaso 96x64, recinto Gorran 72x72.

## 2. Diseño concretado

**Orden en `laQueEmpieza`** (sin cambiar la firma pública más que añadir un callback `tapa(x,y)` opcional): 1 `vista`/`requiere`/
`sin` (hoy); 2 `cerca`: Chebyshev(jugador, `foco`) ≤ radio **o** jugador dentro de `zona` (compatibilidad); 3 si vino por radio,
`seVen(jugador, foco, tapa)`; 4 devolver la cinemática → `empieza()` antepone `lleva` y la `accion` (D0026). El `foco` es
`disparador.lleva.a` si existe, si no el centro de la zona (`puntoDeEntrada()` de E1). Sin `lleva.a` ni `zona` → error del validador.

**Distancia: Chebyshev.** Es la del alcance, la niebla y el tablero; Manhattan castiga la diagonal (en isométrico la diagonal
de pantalla es un paso cardinal del grid, se notaría raro); euclidiana obliga a float y no aporta nada a 5-10 casillas.

**Radio por nivel.** Campo nuevo opcional en el JSON de nivel `"ambito": "interior_pequeno" | "interior_grande" | "exterior"`
(leído en `LevelLoader`, `LevelDefinition::ambito`). Si falta, se deduce: nombre con prefijo de interior/aldea/mansión/casa/
dependencias/mina/cueva/galería/cámara/grieta/santuario → interior; interior con ancho·alto ≤ **200** → pequeño (taberna 180,
casa de té, herrería, vacío), > 200 → grande (casa del Cojo 288, iglesia 315, cripta 1024, mina2 1764); resto → exterior.
Radios: pequeño = ∞ (dispara al entrar, sin visión: la taberna la taparían 14 taburetes y 5 bancos); grande = 5; exterior = 10.
`disparador.radio` (entero, o `"nivel"`) sobreescribe; `disparador.vision: false` lo desactiva para una cinemática concreta.

**Qué tapa la vista** (`tapa(x,y)`): tile con `collision` (edificios, murallas, roca, agua) ∨ objeto con `tapaVista:true`.
Campo nuevo `tapaVista` en `ObjectDefinition` (defecto `false`); `ObjectCatalog.cpp:142` lo lee junto a `blocksMovement`.
Se pone a `true` en datos: los 368 `paret_*`, `tanca_fusta*`, `empalizada*`, `biombo*`, `puerta_gurkazaal` y portones que
sean objeto, `carromato*`/`carro*` (tapan a un humano), `estanteria*`/`armario*`/`vitrina*`. **No** tapan: PNJ (categoría
`Npc`, aunque bloqueen), enemigos, pickups, farolas, bolardos, matas, flores, taburetes, sillas, mesas, sacos, cajas, braseros.
Alternativa sin campo (heurística por prefijo) descartada: 2972 ids y los kits se regeneran; mejor dato explícito + validador.
En diagonal, Bresenham pasa «entre» dos esquinas de muro: aceptable (es lo que hace el combate hoy; el jugador puede ver por el hueco).

**Arnés y explorador.** `pisaLaZona` y `cineQueEmpieza` siguen usando `puntoDeEntrada()`: el jugador se teletransporta al
foco, distancia 0 y visión trivial → mismo camino de código que el motor, sin copia. Para probar el radio, un `cineQueEmpiezaDesde(id, casilla)`
en el explorador que llame a `laQueEmpieza` con el `tapa` real del nivel (ya carga TMX y objetos) y escriba en el log
«dispara a d=N desde (x,y)» o «tapado por <id>» — es la lista de la sección 3 hecha herramienta.

**Validador** (`tools/validar_cinematicas.py`, ya lee el TMX `:90`): foco pisable y no tapado por tile; `ambito` del nivel
válido o deducible (aviso si se deduce); `radio` 1..20; al menos una casilla pisable con visión al foco a distancia ≤ radio (si
0, la cinemática no arranca nunca: error); aviso si < 50% de las casillas de la `zona` actual ven el foco (caso plaza). En
`validar_objetos`/catálogo: `tapaVista` solo con `blocksMovement` y categoría `prop`.

**Prueba GL-free** (`tests/Game/CinematicaTests.cpp`, `cmake/Cinematica.cmake`): nivel de mentira 12x12 con un muro en la
columna 6 y un `tapa` lambda. `cerca_y_visible_dispara` (jugador a 4, sin muro); `cerca_pero_tapado_no_dispara` (muro entre);
`lejos_no_dispara` (a 6 con radio 5); `pnj_no_tapa` (lambda devuelve false para la casilla del PNJ); `interior_pequeno_dispara_al_entrar`;
`zona_sigue_valiendo_sin_vision`. **Mutaciones**: `<=` → `<` en el radio (falla `cerca_y_visible` a distancia exacta); quitar la
llamada a `seVen` (falla `cerca_pero_tapado`); invertir el `o` de zona por `y` (falla `zona_sigue_valiendo`); umbral 200 → 2000
(falla el test de deducción con la iglesia 315).

## 3. Casos reales (Bresenham de centro a centro, extremos excluidos; tile `collision` ∨ prop `blocksMovement`)

| Caso | Nivel, ámbito deducido, radio | Foco | Qué pasa |
|---|---|---|---|
| `plaza_sermon` | ciudad_centro 64x64, exterior, 10 | (55,53) estrado (`verina_estrado` encima) | 199 casillas pisables en el cuadrado de radio 10; **68 ven**, 131 tapadas: 121 por edificios (tiles), 8 por `pino`, 2 por `farola`. Dentro de la zona actual [49,49,61,58] ven 49 de 68. Llegando desde el sur por (55,63) (d=10, tapado) dispara en (55,56), d=3; en línea recta desde `playerStart` (32,32), en (53,51), d=2. Con `tapaVista` (pinos/farolas no tapan) suben a 78 vistas. Conclusión: el radio 10 casi nunca es el que dispara, disparan las bocacalles; la plaza funciona, pero conviene foco en el suelo delante del estrado, no en el estrado. |
| `bgd_velas` | mina2_gorran 42x42, interior grande, 5 | (21,30) | 104 pisables, 92 ven, 12 tapadas por roca (tiles) y 1 por `carro`. Desde `playerStart` (21,38) hay visión (d=8) pero dispara al llegar a (21,35), d=5. Con radio 10: 297 pisables, 210 ven; tapan `puerta_gurkazaal`, `cama_pobre_*`, `cajas`, `farolillo`, `carro` (ninguno debería salvo la puerta → `tapaVista`). PNJ en el radio (`devoto_puerta`, `candela`, `mia_atada`…) no tapan: bien. |
| interior grande: `cripta_estudio` | dependencias_secta 32x32, 5 | (11,18) centro de zona [8,15,14,21] | 113 pisables, 112 ven solo con tiles; con props, `sacos` (6) y `mesa` (5) tapan 12 → sin `tapaVista` serían falsos negativos. Desde `playerStart` (29,7) d=18: dispara al entrar en el radio en (16,13), d=5. |
| interior pequeño: `venides_llama` | interior_taberna 15x12=180, al entrar | (5,7) centro de [4,5,6,9] | 0 tiles tapan; 31 de 109 casillas tapadas por `taburete` (14), `banco`, `barra_taberna_*`. Desde `playerStart` (1,10) d=4 **tapado por un taburete**: con visión no dispararía al entrar. Confirma «pequeño = al entrar, sin visión». La iglesia (315) y la casa del Cojo (288) quedan como grandes con 5. |

## 4. Riesgos y cambio del plan

- Regresión en las 31 `zona` si el radio sustituye a la zona: mantener `zona ∨ radio` y que el validador avise (<50% visto).
- `tapaVista` toca datos de kits regenerados (`mobles_kit`, `decorados_kit`, `pixellab_objetos`): hay que meterlo en el generador, no a mano.
- Rendimiento: 113 cinemáticas × Bresenham ≤ 10 pasos por frame es nada; pero filtrar antes por nivel y por flags (ya se hace).
- Caminos: `rutaAndando` puede no llegar al foco aunque haya visión (ventana, reja); D0026 ya cae a fundido.
- Plan: **E1** igual; **E2** suma `radio`/`vision` al parser y la vista a `laQueEmpieza` (+ `Game/LineaDeVision.h` extraído de
  `TableroCombate::seVen`, que pasa a llamarla); **E3** suma los chequeos de visión/ámbito; nuevo **E2b** `ambito` en
  `LevelDefinition`/`LevelLoader` + `tapaVista` en `ObjectCatalog` y generadores de kits; **E4-E6** igual; E6 pone foco a `plaza_sermon`
  delante del estrado y `tapaVista` a `puerta_gurkazaal`. Siete pasos, el nuevo es pequeño pero toca tres sitios.

## 5. Decisiones para Oriol (recomendada primero)

- **D-D. ¿Qué tapa la vista?** 1) Tiles con colisión + objetos con campo nuevo `tapaVista:true` (paredes, vallas, carros, armarios);
  PNJ y el resto no. 2) Tiles + todo `blocksMovement` salvo `Npc` (cero datos nuevos, pero farolas, taburetes y sacos tapan: la taberna y la cripta fallan).
- **D-E. ¿La zona sigue valiendo?** 1) Sí: dispara por zona **o** por radio+visión; el validador avisa cuando la zona ve poco el foco.
  2) El radio sustituye la zona: más simple, pero 19 casillas de la plaza dejan de disparar y hay que revisar las 31 a mano.
- **D-F. ¿Cómo se clasifica el nivel?** 1) Campo `ambito` opcional en el JSON del nivel, deducido por nombre y umbral 200 casillas si falta
  (taberna pequeño, casa del Cojo e iglesia grandes). 2) Solo deducción, sin campo: ahorra 150 ediciones pero un interior raro (coliseo, ópera) no se puede corregir.
- **D-G. ¿Radio del interior pequeño?** 1) Al entrar al nivel, sin visión (taburetes tapan el `playerStart` de la taberna). 2) Radio 5 con visión
  igual que el grande, aceptando que haya que entrar más.
