# Capturas del juego (03/11/2026, `agente/claude/UNI-02-11@e43db51d`, preset `macos-debug`)

Cómo: `./juego [--nivel X] N` (modo humo: vuelca el último fotograma a `juego_output.ppm`), convertido a PNG con `sips`. Ventana real de 1280×720 en el Mac, OpenGL real.

| Fichero | Qué se ve | Qué demuestra |
|---|---|---|
| `00_titol_f60.png` | Pantalla de título: pergamino con Continuar/Nueva partida/Opciones/Salir sobre la ciudad | El arranque normal llega al título y lo dibuja bien. |
| `01_ciudad_centro_f90.png`, `02_ciudad_centro_f100.png` | Casco Antiguo: HUD (PV, oro, objetivo), minimapa, PNJ, río, edificio del jugador translúcido | Cámara U4-01 centrada en el jugador; oclusión de edificios. |
| `03_ciudad_centro_ocaso_f300.png` | Boundington al ocaso (oeste y casco) | El nivel carga, pero **se ve de día**. |
| `04_afueras_ocaso_f300.png` | Las Afueras al ocaso | El nivel carga, también **de día** y muy vacío (encaja con la decoración que falta, F-REST.1). |
| `05_prado_del_rio_f120.png` | Prado del río (inicio del prólogo) | El nivel inicial carga. |

## Qué NO se ha podido capturar todavía
La mesa de combate con 5 enemigos, la persecución de los sectarios, las campanas del Ocaso, el polvo de los pasos y la animación de PNJ en movimiento necesitan **pulsar teclas** dentro del juego. Desde aquí no se puede:
- enviar teclas con `osascript`/System Events pide el permiso de Accesibilidad de macOS (la petición se quedó colgada, sin respuesta), y ese permiso solo lo puede dar Oriol;
- el juego no tiene repetición de entradas ni guion de captura.

## Hallazgos (pendientes de confirmar jugando)
1. Con `--nivel` se ve el objetivo del **prólogo** («Acércate a la orilla del río») en cualquier mapa: el nivel no carga la aventura de su día. Es normal en el modo de prueba, pero significa que el Ocaso no se puede comprobar así.
2. El jugador sale con el **cuerpo base** (sin pelo ni ropa) porque se salta el creador. Falta comprobar si en una partida nueva normal sale vestido.
3. Los niveles `*_ocaso` **no tienen tinte de atardecer** en el modo de prueba. Puede depender del estado del día (que aquí no se carga): por confirmar.

---

# E-MATRIZ-04 (04/11) — capturas nuevas o rehechas. Todas miradas con Read, una a una (algunas en hoja de contacto).

Guardados: `out/fabrica/UNI-02-11/guardados_ocaso/` (`fabricar_guardados` base, `--matriz-ocaso`, `--capturas-ocaso-m1`, `--capturas-dia3`). Los nombres de los PNG son los `captura` de los guiones de `MotorGraphico/tools/guiones/`. Lo que **no** se ve se dice aquí.

| PNG | Guion | Qué se ve de verdad |
|---|---|---|
| `ocaso_cuartel_antes.png` | `ocaso.txt` f3 | Casa de Guardia con la tropa formada, HUD «Día 4 · Mañana», oro 280, «Nuevo objetivo: Sal de Boundington…». Aún no han sonado las campanas (es el estado inicial del guardado). **El HUD dice Mañana, no ocaso**: el reloj de la partida, no el tinte. |
| `ocaso_fundido_negro.png` | `ocaso.txt` f80 | Pantalla negra con el HUD encima: es el fundido de `base_campanas` (f28-136). Solo documenta que existe el fundido. |
| `ocaso_campanas_explorador.png` | f178 | **El aviso de las campanas**: diálogo «EXPLORADOR: ¡Los Perdidos estan en la ciudad! ¡Vienen por nosotros!» (`b3_campanas`) sobre la sala; sin HUD. ⚠ Las campanas son sonido: **no se ve nada que suene**, solo el aviso. |
| `ocaso_arenga_1.png`, `_2.png`, `_3.png` | f198/218/238 | **La arenga**, páginas 1/3, 2/3, 3/3 de `b3_venides_arenga_20`: «(con la mirada dura… delante de veinte hombres formados…)», «¡Aventurero! Tenemos que salir de aquí. Si caemos, todo el reino cae con nosotros.», «Preparaos para lo peor. Esto es solo el principio.» (VENIDES). Reemplazan a `s_ocaso_arenga/cuartel/fundido_negro` (borradas: no enseñaban nada de esto). |
| `m1_unirse_plaza_hud.png`, `m1_dudas_plaza_hud.png` | `m1_unirse.txt` / `m1_dudas.txt` f5 | Plaza del Casco Antiguo, «Día 3 · Noche», objetivo del Ocaso. **Oro real: 160 (unirse) y 120 (dudas)**: es la diferencia de oro del día 1. |
| `m1_unirse_sermon.png`, `m1_dudas_sermon.png` | f42 | Sermón de Sandes (plaza); la dudas muestra «Una mujer: Yo abro las cortinas» — pantallas distintas porque las páginas del sermón no avanzan igual. |
| `m1_unirse_menu_sermon.png` | f182 | **La diferencia de camino**: menú del sermón con **4 opciones** (Escuchar entre la gente, **Mirar a Verina**, Hablar de otra cosa, Correr a avisar a Venides). |
| `m1_dudas_menu_sermon.png` | f162 | Mismo momento (narración «La voz de Sandes no grita…», 2/2) con **3 opciones**: sin «Mirar a Verina» (sembraste dudas en ella). Las seis `m1_*` tienen md5 distintos (verificado con `md5`). |
| `s_sectarios_narracion_se_van.png` | `sectarios.txt` f120 | **Renombrada** (antes `seguimiento`): solo la narración «A la segunda esquina, el mayor se para y se vuelve…» (1/3). ⚠ **La persecución no se ve**: en capturas densas hasta f500 los sectarios no aparecen andando; queda sin demostrar. |
| `s_sectarios_sigilo.png`, `s_sectarios_tirada.png` | f70 / f90 | Narración «Dejas veinte pasos…» y «(Tirada de Sigilo.)» con la caja de tirada. |
| `m3_fiesta_sala_tras_eleccion.png` | `m3_fiesta.txt` f130 | **Renombrada** (antes `preguntas`): la sala de la mansión con el HUD, **sin diálogo**. ⚠ No se ve la elección «Prefiero hacerle preguntas» ni su respuesta. La elección sí está en `m3_fiesta_noble.png` («Tres copas de mi cuenta (6 de oro)» / «Prefiero hacerle preguntas», NOBLE: «La copa esta vacia otra vez.»). |
| `m4_oeste_todos.png`, `m4_solos_delatada.png`, `m4_caido.png` | `m4_*.txt` f60 | ⚠ **Los tres son casi la misma imagen**: el guardado final cargado en `huida_boundington` (héroe junto a Venides, «Hablar con Venides»). Solo cambian el **oro del HUD (510 / 420 / 170)** y la hora (Tarde / Mañana / Mañana). **No enseñan el final** (ni el epílogo ni la huida con gitanos): el estado final es el del arnés, que fotografía en el nivel de la huida. Para un final de verdad habría que capturar el epílogo (`b3_epilogo`) tras cargar. |
| `m2_tregua_menu.png`, `m2_tregua_pactos.png`, `m2_tregua_cerrada.png` | `m2_tregua.txt` | Descritas en `MATRIZ_GORRAN.md` Fase C (corregida). `cerrada` es solo la página 2/4. |

---

# E-MISIONES-01 fase C (04/11) — capturas `q1_*` de las misiones del día 1
Guardados `fabricar_guardados --capturas-misiones1 guardados_q1` (cama pagada y desayuno hecho, de pie junto al PNJ) y guiones `tools/guiones/q1_<mision>.txt` (`toca E` y captura del diálogo). Solo se guardan las de diálogo (9 de 25 misiones; las otras 16 se quedan con la prueba y el log de mutación, sin captura). Todas miradas con Read.

| Fichero | Qué se ve de verdad |
|---|---|
| `q1_ventana_dialogo.png` | Taberna Humilde de noche, narración «La bordadora aparta la labor para hacerte sitio…» (abre la misión de los vecinos de la ventana). HUD sin oro ni objetivo (aún no es de día). |
| `q1_jornaleros_dialogo.png` | Tomás: «¿Vienes a darme la razón o a quitármela?» (abre el rincón de los jornaleros); el héroe junto a la mesa de los tres. |
| `q1_venides_noche_dialogo.png` | Venides con el menú de 4 opciones (¿Qué te trae a Boundington? / ¿Qué escribes? / Hablar de otra cosa / Dejarle con su cuaderno). |
| `q1_encapuchado_dialogo.png` | Narración del hombre de la capucha («Bebe solo, de espaldas a la pared…»), héroe junto a la puerta. |
| `q1_aigren_dialogo.png` | Herrería Lingote de Oro: **sale la narración de entrada al nivel (1/4), no la charla de Aigren**; el guion llega al PNJ pero la charla empieza un `ENTER` después. |
| `q1_luisarda_dialogo.png` | La Boutique de Madame: igual, narración de entrada (1/4), no la charla. |
| `q1_xila_dialogo.png` | Pico Dragón / Taberna del Gremio de Ladrones: narración de entrada (1/4). Se ve el edificio del jugador translúcido. |
| `q1_familia_dialogo.png` | Pico Dragón, narración de entrada (1/3); el héroe en el borde izquierdo, la niña del gato a su lado. |
| `q1_almacen_dialogo.png` | **Sin diálogo**: dependencias de la secta con el HUD del día 1 todavía en «Preséntate al tendero» (1/8) porque el guardado se fabrica saltándose el tutorial; el `E` no abrió el almacén (el héroe no está pegado al PNJ en este recinto). ⚠ Captura no válida como prueba del almacén. |

Lo que NO demuestran: ninguna es el cierre de su misión (eso lo prueban las escenas de `MisionesDia1Tests` con su log en `logs_misiones1/`); enseñan el gancho. Pendiente: guardados «justo antes del cierre» para las otras 16.


---

# E-MISIONES-01b (04/11) — capturas de CIERRE de las 25 misiones del dia 1: `q1_<mision>_cierre.png`
Cada una es el dialogo o resultado del cierre de su mision, jugado en el juego real desde un guardado «justo antes» (`fabricar_guardados --capturas-cierre guardados_q1c`, guardados `q1c_<mision>.json`) con el guion `tools/guiones/q1_<mision>_cierre.txt` (la cabecera lleva las pulsaciones). Todas miradas con Read. Los guiones se generaron con `logs_misiones1b/{gen_guion_cierre.py,mkguion.py,final.sh,final_gen.sh}`.

Lo que el juego hace al cargar (importa para entender los guardados): un beat `auto` NO salta al cargar, salta al CERRAR un dialogo (tick del cierre), y uno `enter`, al pisar una puerta (E sobre la puerta). Por eso los cierres automaticos acaban tras una charla con un PNJ de al lado y los de entrada se guardan encima de la puerta. Donde el cierre depende de una tirada o un combate, el guardado ya lleva la flag del resultado (S:) y el juego lo cuenta al cerrar el dialogo; no se ha forzado ninguna tirada en el juego.

| Mision | Que se ve (pagina) |
|---|---|
| llegada_cama | Narracion «La cama huele a lavanda...» en el cuarto de arriba (b0_subir, enciende bnd0_dormido) |
| ventana | Bordadora: «Uy, uno nuevo...» con el menu de 4 (b0_ventana; el cierre es `visto`, que se enciende con este beat) |
| jornaleros | Tomas: «¡Eso! En la iglesia vieja...» (1/2, tras «Darle la razon»; rincon_hecho) |
| encapuchado | «Hay mesas libres.» (3/3, b0_capucha) |
| venides_noche | Venides: «Lo que pasa. Quien estaba, que dijo...» tras «¿Que escribes?» (el cierre es `visto`) |
| desayuno | Narracion 5/5: Venides se va y se abren los sitios del dia (tras «Ponerse en marcha») |
| presentaciones | 4/4 de tut_victoria: la bolsa del saqueador, tutorial cerrado (tras la charla de la vieja, que el juego abre antes) |
| plaza | tut_plaza_ya: «Recorres la plaza entre puestos y corrillos...» |
| carterista | Chaval: «¡No te robaba!...» (2/2, b1_carterista_pillado) |
| aigren | Aigren 3/3: «No les entregare las ultimas...» (b1_aigren_redimido) |
| luisarda | Luisarda 5/5: «Voy. Pero si me matan, sera culpa tuya.» (con_rosita) |
| sectarios | Sectario mayor 2/2 con «Cuenta conmigo / Me lo pensare» (b1_sectarios_masacre) |
| xila | Xila 1/4: «Los Perdidos llevan lunas preparandose...» (b1_xila_habla) |
| familia | El padre 4/4 tras «Os llevo a la Barriada de Surysal» (familia_resuelta) |
| iglesia | 3/3 de beat_iglesia: el nombre Gurkazaal en el circulo de cera |
| ronda_noche | 2/2 de b1_cuartel_noche: «Nada fuera de lo normal...» |
| entrada_guarida | «Sueltas el arma en el cesto...» (guarida_dentro) |
| descubierto | Sandes 2/2: «Ya has visto bastante. Sube...» (te rindes y esperas: capturado_resuelto) |
| ceremonia | «Nadie te mira todavia. ¿Que haces?» con el menu de 4 (b1_ceremonia; el cierre es `ceremonia_visto`) |
| archivo | Edul 4/4 en la sala de estudio (la zona cripta_estudio enciende estudio_visto) |
| camara_sereno | Sandes 2/2: «Y sube el primero...» (sandes_se_va, tras «Me he perdido») |
| verina_edul | Verina 3/3 tras «Sembrar dudas en Verina» (guarida_resuelta) |
| informe_venides | Venides 5/5: «Por el tiempo perdido...» (venides_informe_hecho) |
| pelea_taberna | Rosita 3/3 tras «Proteger al borracho dormido» (pelea_lanzada) |
| cierre_dia | Narracion 4/4 de cierre_silencio: «y manana nadie estara mirando hacia alli.» (dia_cerrado) |

Limites honestos: (1) en ventana, venides_noche, encapuchado y ceremonia el cierre es una flag de «visto» que enciende el primer beat, asi que la captura enseña ese dialogo, no un final distinto. (2) En archivo se ve la escena de la sala de estudio (el flag cierra al pisar la zona), no un dialogo propio del archivo. (3) Las tiradas no se forzaron en el juego: se sembro la flag del resultado (aigren, carterista, sectarios, familia, presentaciones). (4) `edul_hablado` se siembra a mano en la escena `edul_rescate` de la prueba (no en las capturas). Borradas por no ensenar el dialogo: `q1_almacen_dialogo.png` (sin dialogo) y `q1_{aigren,luisarda,xila,familia}_dialogo.png` (solo la narracion de entrada); se quedan `q1_{ventana,jornaleros,venides_noche,encapuchado}_dialogo.png`.
