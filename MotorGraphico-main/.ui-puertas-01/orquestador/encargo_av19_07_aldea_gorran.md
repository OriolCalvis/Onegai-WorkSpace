# AV19-07 — La aldea de Gorran: cuatro mapas y la misión de la espada (encargo, Fable, 18/10/2026)

Oriol (18/10) decide ampliar la zona de Gorran: ya no es «una cueva con enemigos» sino una **mini aldea criminal viva** dentro de la montaña. Este documento convierte su propuesta (texto del 18/10) en formato de construcción, con lo que ya existe en el juego y las cuatro decisiones que tomó.

## 0. Decisiones de Oriol (18/10) y lo que implican

| # | Decisión | Cómo se aplica |
|---|---|---|
| 1 | **Se juega por primera vez el día 2**, y el día 2 se alarga para dar amplitud a la exploración | El motor tiene 3 franjas por día (`kFranjasPorDia = 3`, C++). Sin tocarlo: **dentro de la aldea el tiempo corre con su propio contador** (`variable aldea_tiempo`, 0..8: cada investigación —taberna, apuestas, prostíbulo, almacenes, herrero— suma 1 y los PNJ cambian de sitio/estado con él); entrar cuesta la tarde y salir te deja en la noche del día 2. Así una visita «vale» media tarde de juego sin romper AV17-02. Si Oriol prefiere 4 franjas reales el día 2, es un cambio de C++ pequeño y se pide aparte |
| 2 | **Gorran quiere vender la espada porque pierde dinero en las apuestas** | Gorran sigue siendo el cap (todo lo escrito sobre él vale: redención, aliado del día 3, bandidos sin jefe). La tensión interna es su **deuda con el tafur profesional** («el Cojo»): la espada está apalabrada con un comprador que llega esa noche. Eso da la segunda parte del conflicto y la razón de que haya prisa |
| 3 | **Apuestas: la versión barata** | Solo JSON: tiradas de Engaño / Percepción / Juego de manos con oro en juego (`mesa_cartas` y `mesa_dados`), 3 apuestas por visita como máximo, y el tafur hace trampas (Percepción para pillarle → información o bronca). Sin minijuego de motor |
| 4 | **Niños en puntos concretos** («los criminales también tienen familia») | Dos: el **recadero** (aprendiz del herrero, PNJ que habla y lleva recados: es la ruta «ayudar a alguien») y una **niña en las barracas** con su madre, sin diálogo propio (ambiente). Ninguno en la taberna, las apuestas ni el prostíbulo |
| — | **Arte nuevo con PixelLab** (Oriol: «això t'ho soluciones amb PixelLab») | Lista en §6; se genera desde el navegador integrado con el token de Oriol, como en el lote 0 |
| — | **Todo lo hace Fable** (Oriol: «no confío en el reparto para cosas de este calibre») | Niveles, PNJ, trama, botín, arte y validación en esta cadena; sin Qwen ni Codex |

## 1. Qué hay hoy y qué se conserva

- `montana_gorran` 28×36 (AV19-03) y `cueva_gorran` 30×28 (AV19-04: cinco salas, 11 contenedores, economía recalibrada). **Se sustituyen** por los cuatro mapas, pero **los ids se conservan** para no romper nada: `montana_gorran` pasa a 48×48 y `cueva_gorran` pasa a ser la aldea de 64×64 (nombre «La Aldea Baja de Gorran»). Así `b2_cueva_llegada` (enter `cueva_gorran`), `obj2_espada`, los prefabs `botin_cueva_*`, `lugares.json` y el manifiesto siguen valiendo. Nuevos: `galerias_gorran` (38×38) y `camara_gorran` (38×38).
- Trama existente que se reutiliza tal cual: `b2_cueva_llegada` (pagar 5 / hablar / colarse), la charla con Gorran (`b2_gorran*`), `b2_gorran_sigilo_casi/torpe`, `b2_gorran_combate` (sin matar), `b2_gorran_robo_limpio`, `b2_gorran_victoria` (perdonar / matar), `b2_gorran_derrota/huida/reintento`, `b2_cueva_sin_encargo`; día 3: `b3_gorran` (30 de oro si le perdonaste) y `b3_gorran_sin_jefe` (bandidos sin jefe si le mataste).
- Las cinco rutas de Oriol sobre lo que hay: **a las malas** = «Pelear por la espada»; **infiltrándose** = «Colarte por detrás [Sigilo]»; **como cliente o mercenario**, **ayudando a alguien del poblado** y **ruta alternativa (galerías)** son nuevas (§5).
- Objetos que ya existen y sirven: hoguera, brasero, cajas, cajas_almacen, sacos, sacos_monedas, barril(es), barriles_rotos, barriles_pila, fardos, rollo_cuerda, jaula, carromato_lona, carro, tendedero_ini/mig/fi, mesa_taberna, mesa_cartas, taburete, banco, cama_pobre_cap/peus, armero_espadas/lanzas, estandarte, antorcha, candelabro, velas_derretidas, huesos, totem_huesos, cofre_tesoro, barra_taberna_*, estanteria_botellas, yunque, paret_pedra_fosca_* (tabiques), tiles tierra_canon / barro / losa_oscura / agua_somera / roca.

## 2. MAPA 1 — `montana_gorran` · «El sendero del riesgo» · 48×48 exterior

Subida en zigzag con **cuatro alturas** (el motor ya las tiene, AV19-02; el generador `av19_niveles.montana()` ya hace terrazas y rampas). Entrada al sur (desde las afueras), boca al norte. Nada simétrico: el camino cruza el mapa dos veces.

| Zona | Casillas (x, y) | Altura | Qué hay | Habitantes |
|---|---|---|---|---|
| 1. Pie de montaña | (16..30, 40..47) | 0 | `salida_montana_gorran` ×5 en (18..22, 47) → afueras (20, 29); camino de tierra; 2 árboles torcidos (`arbol_seco`), matas, una **fita de piedra** (`columna_rota`), restos de sendero antiguo (`losa_oscura` sueltas) | — |
| 2. Sendero roto | zigzag: (16..19, 30..40) sube al oeste, luego (6..30, 26..30) cruza al este | 1 | camino de 3 de ancho, **rocas grandes** que estrechan a 1 en dos puntos (curvas ciegas), tramo erosionado (`tierra_piedras`), **pasarela de tablas** de 3 casillas sobre una grieta (`puente_madera`, arte nuevo si no hay) | 1 bandido apostado tras la roca de la segunda curva (`bandido_gorran`, enemigo al contacto) |
| 3. Cornisa del precipicio | (36..44, 12..26), franja de 1–2 de ancho con vacío (roca sin suelo) al este | 2 | postes de madera / valla rota (`empalizada_o1`), viento (sin objetos) | **emboscada**: 2 bandidos salen de detrás de la roca al pisar (38, 18) (beat `auto` con `startBattle`) |
| 4. Mirador de los vigilantes | plataforma lateral (6..14, 8..16) | 3 | cajas `x` ×3, hoguera, banco de piedra, **perro de guardia** (`perro_gorran`, arte nuevo), lonas | 1 vigilante `E` quieto mirando al sendero; 1 **patrulla `Ep`** que baja y sube por la terraza 2 (rutina) |
| 5. Entrada escondida | (20..28, 3..8) | 3 | boca medio tapada: **telas colgadas** y maleza (`tela_colgada`, arte nuevo; `mata`), carromato volcado, huesos; `boca_cueva_gorran` ×3 en (23..25, 4) → `cueva_gorran` (31, 60) | — |
| Secreta | grieta al este de la cornisa, (45, 22) | 2 | `grieta_gorran` (Ps): solo se ve con Percepción (beat al pisar (43, 22)); lleva a `galerias_gorran` (2, 35) | — |
| Botín | un cofre en el mirador (`botin_afueras_cofre`), un escondite bajo la pasarela | | `repartir_botin.py` con región afueras | |

Rampas siempre de 2 casillas y de un nivel; el resto del borde entre terrazas es acantilado. Guardas del generador: la subida abierta de (20, 46) nivel 0 a la boca nivel 3 **con** los objetos puestos; la grieta alcanzable; las rondas sobre suelo llano.

Sensación: subida, tensión, clandestinidad. Pistas de uso: huellas de carro, hoguera apagada, botellas.

## 3. MAPA 2 — `cueva_gorran` · «La Aldea Baja de Gorran» · 64×64 interior principal

Una gran cavidad irregular con extensiones; roca por fuera, **barro y tierra** en lo común, **losa** en los locales, **charcos** en los rincones. Luz: antorchas y hogueras (el motor tiene fuego/luz). Techo: ninguno (isométrica).

| Zona | Casillas | Qué hay | Habitantes (E enemigo · N neutral/habla · m animal) |
|---|---|---|---|
| A. Vestíbulo | (26..36, 52..62); boca en (29..33, 62..63) → montaña (24, 6) | control de acceso: 2 cajas, antorchas, barricada de tablas (`barricada_tablas`, arte nuevo) con hueco de 2 | 2 guardias `E` (son los de `b2_cueva_llegada`: pagar 5 / hablar / colarse); el **recadero** pasa corriendo al entrar (cinemática `aldea_llegada`) |
| B. Plaza central | (20..44, 32..50), irregular | hoguera grande en (32, 41), 4 mesas con taburetes, un **músico borracho** sentado, tendederos entre barracas, basura, un pozo cegado | 3 N hablando (`maleante_charla` ×2, `vieja_del_poblado`), 1 tafur callejero, **2 perros**, ratas, gallinas sueltas (arte nuevo) |
| C. Taberna «El Tajo» | (6..20, 36..50), local con tabiques de `paret_pedra_fosca` y barra | barra torcida (`barra_taberna_*`), mesas rotas (`mesa_rota`, arte nuevo), barriles, estantería de botellas, humo (partículas de fuego existentes) | **tabernera cansada** N (información: «la Furia la tienen arriba, pero no por mucho»), 2 bebedores N, 2 maleantes jugando a cartas (`mesa_cartas`), 1 buscabroncas `E` opcional (beat: te provoca si vas de «cliente» y fallas Engaño) |
| D. Apuestas | trastienda de la taberna (6..14, 28..36) | mesa grande (`mesa_dados`, arte nuevo), monedas, gente mirando | **el Cojo**, tafur profesional N (acreedor de Gorran): 3 apuestas por visita con tiradas; pillarle haciendo trampas (Percepción) da la pista del comprador |
| E. Compañía | (44..58, 36..50), telas colgadas y luz cálida | 3 **cubículos** con camas pobres, cortinas (`tela_colgada`), espejo (`espejo`), cofres pequeños, perfumes (`frascos`) | **la encargada** N («Doña Serra», sabe dónde guardan la espada y quién viene a comprarla), 3 trabajadoras N (dos hablan, una no), 1 cliente borracho N |
| F. Barracas | (4..22, 8..26) | 6 **barracas** (`barraca`, arte nuevo, 2×2), tendederos, cajas, bancos, un fuego pequeño | mujer con **niña** (ambiente, sin diálogo), 2 ancianos N desconfiados, 1 herido tumbado (orden `tumba`), gente durmiendo |
| G. Almacén de contrabando | (40..60, 8..26) | sacos, cajas_almacen, fardos, armero, barriles; **mula** atada (arte nuevo); cofre | 2 guardas `E` de ronda (como hoy); **documento** (el trato de venta de la espada, `darObjeto`); paso a MAPA 3 en (60, 17) `paso_galerias` → `galerias_gorran` (1, 18) |
| H. Establos y jaulas | (24..38, 8..18) | jaulas con gallinas, una cabra robada (arte nuevo), paja, comedero, perro atado | 1 mozo N (habla poco) |
| I. Taller del herrero sucio | (24..34, 20..30) | yunque, brasero, armero, chatarra, **el recadero** (aprendiz) | **herrero sucio** N: «la pieza buena la han subido arriba; la quieren vender»; su aprendiz es el niño recadero → ruta «ayudar a alguien» |
| J. Subida al recinto del jefe | escalera/rampa en (32..34, 2..6) con 2 guardias; `paso_camara` → `camara_gorran` (19, 36) | antorchas, estandarte de Gorran | 2 `E` fuertes (`bandido_acorazado`) que no dejan pasar sin salvoconducto (Gorran te llama / Serra te cuela / el Cojo te vende el paso / pelea) |

Población: 10–12 enemigos (2 vestíbulo, 2 almacén, 2 subida, 1 buscabroncas, 3 de patrulla por la plaza y las barracas, 2 en reserva para la bronca de la taberna), 14 PNJ neutrales (los de arriba), 2 niños, animales (2 perros, 1 cabra, 1 mula, 4 gallinas, ratas como decoración animada si PixelLab da un sprite de 2 frames).

Tiempo propio (`aldea_tiempo`): cada investigación (taberna, apuestas, compañía, almacén, herrero) suma 1. A los 6, el **comprador llega** (`bnd2_comprador_llega`): si aún no has resuelto, la espada cambia de manos y la persecución sigue en el MAPA 3 (es el «segundo acto» de la decisión 2).

## 4. MAPA 3 — `galerias_gorran` · «Galerías de contrabando» · 38×38

Funcional, menos social: túneles estrechos (1–2 de ancho) semiexcavados, tres cámaras, una rampa a la cota baja y el río subterráneo que lleva fuera.

| Zona | Casillas | Qué hay | Habitantes |
|---|---|---|---|
| Entrada desde el almacén | (1..6, 16..20) | `paso_almacen` → `cueva_gorran` (59, 17); cajas | — |
| Cámara de mercancía | (8..18, 6..16) | fardos, sacos, carros, cofre `C`, **cajón con apuestas escondidas** (dinero del Cojo: `darObjeto`/`grantGold` 40) | 2 contrabandistas trabajando N (uno te vende el paso por 15), 1 `E` |
| Túnel bajo | (6..30, 22..30), rampa de 2 | charcos, murciélagos (ambiente), **puesto de vigilancia** con hoguera | 1 patrulla `Ep` de 2 |
| Cámara del comprador | (22..34, 6..16) | mesa, lámparas, **la salida al río** (balsa: `barca`) por (36, 10) → `montana_gorran` (45, 22) por la grieta; aquí termina la persecución si el comprador se llevó la espada | el **comprador** N/E (combate opcional, 2 escoltas) |
| Secreta | (33, 30) `Ps` grieta → `camara_gorran` (36, 20): entrada lateral al recinto del jefe (Percepción o la pista del herrero) | | |

Habitantes: 4–6 enemigos, 1 patrulla, 2 neutrales, animales pequeños. Botín: 2 cofres, 1 escondite.

## 5. MAPA 4 — `camara_gorran` · «La cámara del jefe» · 38×38

| Zona | Casillas | Qué hay | Habitantes |
|---|---|---|---|
| 1. Acceso controlado | (16..22, 32..37); `paso_aldea` → `cueva_gorran` (33, 7) | portón (`porta_reixa`, arte nuevo), 2 guardias | 2 `E` (los de J si pasaste a la fuerza, o nadie si vienes invitado) |
| 2. Antecámara | (10..28, 24..32) | bancos, cajas, estandartes, trofeos (`cabeza_animal_pared`, arte nuevo), armas colgadas | 1 guarda sentado (orden `tumba`/sentado cuando exista) |
| 3. Sala de Gorran | (8..30, 10..24) | mesa grande con mapa y papeles, botellas, botín, **la piedra plana como altar** (hoy existe), hoguera | **Gorran** de espaldas (`b2_gorran_sigilo` tal cual), 2 esbirros |
| 4. Cámara de la espada | (26..34, 4..12), puerta con cerrojo | **pedestal / relicario saqueado** (arte nuevo) con la Furia de Alaro; cofre reforzado con el oro de las apuestas | — |
| Lateral | (36, 20) grieta desde MAPA 3 | | |

Resolución (todas con la trama de hoy debajo): combate (`b2_gorran_combate`), negociación (`b2_gorran` con la opción nueva «Sé a quién se la vendes. Págale yo» → pagar la deuda del Cojo, 60), hurto (`b2_gorran_robo_limpio` por la grieta lateral), o llegar tarde (`bnd2_comprador_llega`) y seguir al MAPA 3.

## 6. Arte nuevo (PixelLab) — lo único que no sale de los datos

Se genera desde el navegador integrado del Mac contra `https://api.pixellab.ai/mcp` con el token de Oriol (como el lote 0; el token no se guarda). Entra por `tools/pixellab/importar_props.py` / `importar_actores.py` / `importar_mascotas.py` y la paleta maestra (`aplicar_paleta.py`).

| Pieza | Herramienta | Tamaño | Cantidad / coste estimado |
|---|---|---|---|
| Barraca (2 variantes) | `create_map_object` | 128×128 (ocupa 2×2) | 2 |
| Tela colgada / cortina, barricada de tablas, mesa rota, mesa de dados, pedestal-relicario, portón de reja, trofeo de pared, lámpara improvisada, paja, botellas por el suelo, pasarela de tablas | `create_map_object` | 64×64 | 11 |
| Perro de guardia, cabra, mula, gallina | `create_character` size 32–40 + `animate_character` (reposo + andar) | — | 4 + 4 animaciones |
| Rata (2 frames) | `create_image_pixen` | 32×32 | 1 |
| **Personas**: tabernera, el Cojo, Doña Serra, 3 trabajadoras, herrero sucio, recadero, niña, contrabandistas, comprador, maleantes | **no hace falta PixelLab**: `gen_pnj.py` con las piezas del jugador (prendas de oficio y ropa pobre que ya hay) | | 0 |

Total: unas **22 generaciones**. Se pide permiso a Oriol antes de la tanda (gastan créditos).

## 7. Trama nueva (JSON, `boundington_segundo_dia.json` y `boundington_ocaso.json`)

Unos 55 beats nuevos, con la regla AV17-01 (tres salidas) en cada tirada que abre vía:

- **Llegada**: `b2_cueva_llegada` se queda; se añade la cinemática `aldea_llegada` (el recadero cruza corriendo, los guardias se giran, `emocion`).
- **Ruta cliente/mercenario** (`bnd2_aldea_cliente`): Engaño en el vestíbulo o 10 de oro → te mueves libre; en la taberna, Interpretación para «ser de los nuestros»; fallo → el buscabroncas; segundo fallo → te echan a la plaza, pero Serra te recoge (ruta de ayuda).
- **Ruta ayudar a alguien** (`bnd2_aldea_favor`): el herrero (su aprendiz ha robado al Cojo y lo van a lisiar: devolver el dinero, convencer al Cojo o pagar) o Serra (sacar a una de las chicas con el comprador). Pago: el salvoconducto para J o la grieta de G.
- **Apuestas** (`b2_aldea_apuesta_*`): 3 por visita; ganar (Juego de manos / Percepción contra rival 2–3) da 10–30; pillar la trampa da la pista del comprador; perder 3 seguidas te deja sin la bolsa y el Cojo «se cobra» (combate o favor).
- **El tiempo** (`aldea_tiempo` 0..8): a 6, `b2_comprador_llega` (auto): si la espada sigue arriba, Gorran baja a cerrar el trato en la plaza (cinemática: Gorran, el comprador y dos escoltas cruzan a la sala del jefe); si ya la tienes, te buscan.
- **Gorran**: a las opciones de hoy se suma «Pagar su deuda (60)» → `bnd2_gorran_pagado` (te da la espada y queda en deuda: día 3 trae a **todos** sus hombres, `b3_gorran_todos`) y «Enseñarle el trato» (si tienes el documento: Gorran descubre que el Cojo y el comprador van a medias → vuelve a los bandidos contra el Cojo: combate con aliados).
- **Galerías**: `b2_galerias_*` (vigilante, contrabandista que vende el paso, el comprador en su cámara, la balsa).
- **Día 3 / Ocaso**: `b3_gorran_todos` (si pagaste), `b3_huida_galerias` (tercer final de huida: «los de Gorran te sacan por las galerías hasta el río»; exige `bnd2_gorran_perdonado` o `bnd2_gorran_pagado`).
- Ledger: cada beat que paga entra en `tools/recompensas_escenas.json` (`validar_todo` lo exige).

## 8. Economía y botín
- Línea nueva en `ECONOMIA.md` §4: región **aldea** (la actual «cueva» se renombra) con presupuesto de 3 mapas: cofres visibles 60 %, escondites 25 %, apuestas ±40 netos de media, misiones (favores) 20–60.
- `repartir_botin.py` con los prefabs `botin_cueva_*` (se conservan) sobre los tres mapas; luego `simular_economia.py` a 0 (los márgenes se recalibran **una vez**, al final).

## 9. Orden de trabajo (todo Fable; una sesión por fila, cada una deja `sesiones/` y su fila en el TABLERO)

| Sesión | Qué | Comprobación |
|---|---|---|
| S1 | MAPA 1 y MAPA 2 en `av19_niveles.py` (geometría, alturas, pasos, objetos existentes; los que piden arte, con un marcador del mismo id para que el nivel ya valide) · manifiesto, `lugares.json`, pasos | `validar_todo` 0 errores; Excel de los cuatro en «Otros» para que Oriol los revise |
| S2 | MAPA 3 y MAPA 4 · PNJ nuevos en la hoja de `gen_pnj.py` · rutinas (ronda, beber, dormir, trabajar) en `rutinas_pnj.json` | `validar_pnj`, `validar_rutinas` |
| S3 | Trama (§7) + cinemáticas (`aldea_llegada`, la llegada del comprador) + ledger | `analisis_trama` sin huérfanas; partida simulada por las cinco rutas |
| S4 | Arte PixelLab (§6) con el token de Oriol en el navegador; importar; paleta | el juego carga las hojas; capturas |
| S5 | Botín y economía (§8) | `simular_economia` a 0 |

Antes de S1 no hace falta más decisión. Lo que sí hará falta en S4: que Oriol pegue el token de PixelLab en la sesión (no se guarda).

## 10. Fuera de alcance
C++ (minijuego de apuestas, 4 franjas, sentarse), el lore de la Furia de Alaro (se usa el de `Onegai 2/` tal cual), y la ópera.
