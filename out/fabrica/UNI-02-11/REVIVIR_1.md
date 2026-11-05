# E-REVIVIR-01 — Inventario de consecuencias de D0027 (morir reinicia el combate)

Lista para que Oriol decida. NO se ha reescrito historia ni tocado ninguna aventura, mision ni dialogo. Cifras y lineas medidas con
`assets/adventures/*.json` y `assets/catalogs/quests.json` en el commit de la rama (generadas con un recorrido de los JSON; los numeros de linea
son del fichero tal cual esta).

## Lo que pasa ahora (regla)
- Morir en cualquier combate (historia o mapa, ineludible o no) reinicia el combate: ni `flagDefeat`, ni `GameOver`.
- `flagDefeat` solo se enciende (a) huyendo, si el combate NO tiene `flagHuida` (como antes de D0027), o (b) perdiendo una pelea `sinMatar` (D0027 §6).
- Si el combate SI tiene `flagHuida` (y no es `sinMatar`), `flagDefeat` ya no se enciende de ninguna forma; solo `flagHuida`.
- En el contenido actual NO hay ningun combate `ineludible` (0 apariciones de `"ineludible"` en `assets/adventures/*.json`): no hay caso «no se puede huir».
  Si en el futuro hubiera uno, sus `flagDefeat`/`flagHuida` serian inalcanzables y la unica salida seria ganar.

## 1. `flagDefeat` que ya no se alcanzan de ninguna forma (el combate tiene `flagHuida` distinta y no es `sinMatar`)
(fichero:linea = la linea `"flagDefeat": "..."` del combate)

| flagDefeat | combates (beat: linea) | ¿Quien la lee? |
|---|---|---|
| `bnd1_guarida_perdida` | boundington_primer_dia.json: b1_cripta_descubierto 14165, b1_escape_parcial 14711, b1_herrero_pifia 15228, b1_dep_llegada_colarse 15446, b1_colarse_parcial 15568, b1_colarse_pifia 15621, b1_novicio_pifia 15751, b1_santuario_intruso 20472, b1_camara_sandes_herrero 20619, b1_camara_sandes_sospecha 20767, b1_camara_sandes_tunica 20909, b1_camara_sandes 21060, b1_guarida_por_aigren 22721, b1_edul 23929 | beat `b1_guarida_perdida` (primer_dia:26043; sigue viva por `bnd1_guarida_huida`); **decision del diario** quests.json:782 «Les plantaste cara en la cripta, y perdiste.» (flag ya inalcanzable: entrada de diario muerta; `bnd1_guarida_huida` no tiene entrada de diario) |
| `bnd1_secta_ganan` | primer_dia: b1_secta_burla 19428 | beat `b1_secta_ganan` (19488; viva por `bnd1_secta_huido`) |
| `bnd1_verdaderos_ganan` | primer_dia: b1_verdaderos_forastero 11548, b1_verdaderos 11669, b1_verdaderos_pelea 19972 | beats `b1_verdaderos_ganan` (primer_dia:20036) y `b3_cabecilla_pan` (ocaso:1532), ambos vivos por `bnd1_verdaderos_huido`; **diario** quests.json:556 «Les plantaste cara a los cinco, y perdiste.» (muerta; la de huir esta en quests.json:569) |
| `bnd2_saqueadores_ganan` | segundo_dia: b2_saq_combate_emboscada 9668, b2_saq_combate_tres 9724, b2_saq_combate 9771 | beat `b2_saqueadores_ganan` (9909; viva por `bnd2_saqueadores_huido`); **diario** quests.json:1079 «...y no bastó.» (muerta) |
| `bnd2_sectarios_mina_gana` | dia_gorran: b2_v_sectario 21419, b2_v_sectario_pifia 21523 | **beat `b2_v_sectarios_gana` (dia_gorran:21565): RAMA MUERTA** (solo requiere esta flag; texto «Despiertas en el comedor de los mineros...») |
| `bnd3_bandidos_ganan` | ocaso: b3_gorran_sin_jefe 4796 | beat `b3_bandidos_otros` (4835; viva por `bnd3_bandidos_huido`) |
| `bnd3_barril_gana` | ocaso: b3_barricada_gris_echado 4538, b3_barricada_gris 4575 | **ningun beat la lee** (no hay beat) |
| `bnd_tut_saqueador_gana` | primer_dia: tut_emboscada 13459 | **beat `tut_derrota` (primer_dia:17923): RAMA MUERTA** («Te levantas del polvo con la bolsa vacia...»); la tutorial solo se cierra por victoria o por `tut_huida` (17967) |

Resumen: 8 flags inalcanzables; 2 beats muertos (`b2_v_sectarios_gana`, `tut_derrota`), 1 flag sin lector (`bnd3_barril_gana`), 3 entradas de diario muertas
(quests.json:556, 782, 1079). El resto (`b1_guarida_perdida`, `b1_secta_ganan`, `b1_verdaderos_ganan`, `b2_saqueadores_ganan`, `b3_bandidos_otros`, `b3_cabecilla_pan`) sigue
vivo porque tambien se alcanza por `flagHuida`.
Estos combates ya no se simulan «perdiendo» en el arnes: «perder» = huir (flagHuida).

## 2. Ramas alcanzables HUYENDO cuyo texto cuenta una derrota fisica
(el jugador huyo, pero la rama dice que cayo / se desmayo / perdio; fichero:linea del beat y de la frase)

**Ocaso (dia 4), sin `flagHuida`: huir enciende `flagDefeat` y se lee como caida**
- `b3_caida` (boundington_ocaso.json:6657; texto 6674-6676: «La calle se te acaba antes que las fuerzas.», «...hoy sale menos.»). Se entra con huir de cualquiera de
  `b3_saqueadores*`, `b3_fanaticos*`, `b3_oleada`, `b3_norte` (flags `bnd3_saqueadores_ganan`, `bnd3_fanaticos_ganan`, `bnd3_oleada_gana`, `bnd3_norte_gana`). Pone `bnd3_fuera` y
  `bnd3_final_caido` (6685) y el log «FINAL: caido en Boundington.».
- `b3_rodeo_caido` (ocaso:5162; texto 5178 la misma frase). Pone `bnd3_final_caido` (5183).
- **El final caido en el diario**: quests.json:1446-1448, decision «Como acabo la noche»: «Caiste en Boundington.» / «La calle se te acabo antes que las fuerzas. Boundington ardio igual, con o sin ti.».
- **El objetivo que el diario deja a medias**: `obj3_salir` «Sal de Boundington con vida: el oeste arde menos.» (ocaso:111-115, `doneFlag` `bnd3_salio_vivo`): en el camino caido
  no se enciende, asi que el apartado «Salir de Boundington» (quests.json:1212-1226) queda **0/1** aunque el jugador huyo.
- Tambien por huir (sin flagHuida), no fisicas pero de «has perdido»: `b3_verina_perdida` (ocaso:3480 «No llegas...»), `b3_edul_perdido` (5783), `b3_sandes_gana_casco` (4304, texto 4320 «Las cadenas de Sandes te dejan en el suelo de la plaza. Cuando consigues levantarte...»),
  `b3_compania` (6233, texto 6260; con `bnd3_sandes_gana`) y `b3_sandes_sigue` (6397 «Te levantas como puedes.»).

**Dia Gorran (`flagDefeat == flagHuida`: huir cuenta como perder)**
- `bgd_perdida` (boundington_dia_gorran.json:3243; texto 3257 «Caes junto a la puerta de piedra. Lo ultimo que ves es una vela que se enciende.»).
- `bgb_detras_perdida` (dia_gorran:7343; texto 7357 «Despiertas junto al cabrestante. Tecla te ha subido a pulso.»).

**Dia 2**
- `b2_saqueadores_ganan` (segundo_dia:9909; texto 9927 «Caes. Lo ultimo que ves son las antorchas...»): ahora se llega solo por `bnd2_saqueadores_huido` (huir).

**Dia 1 (se entra por la flag de huir)**
- `b1_secta_ganan` (primer_dia:19488: «Te levantas del adoquin con la boca partida. Los tres ya no estan...»), `b1_verdaderos_ganan` (20036: «Los Verdaderos se van riendo...»; la entrada de diario de huir esta en quests.json:569).

## 3. Lo que sigue igual (no es hallazgo)
- Peleas `sinMatar` (10, nadie muere): siguen levantandose y encendiendo `flagDefeat` sin cicatriz: dia_gorran 4529, 4563, 10054, 18090, 18152, 18205, 18261, 19059; primer_dia 26895, 26943.
- Combates con `flagHuida` y texto de huida (p. ej. `tut_huida`, `b2_gorran_huida`): no hay cambio.

## 4. Pruebas / arnes cambiados (para que lo veas)
- Arnes (`demo_boundington_partida.h`), `tools/recorrer_recompensas.cpp`, `tools/explorar_aventura.py`: «perder» ahora es huir (flagHuida; o flagDefeat si no hay flagHuida); `sinMatar` sigue perdiendo con flagDefeat; de un ineludible no se huye (se gana).
- Caminos de matriz que forzaban perder hasta una rama que ya no existe se han **convertido** (misma entrada, ahora afirman D0027: sale la rama de huir y la de perder queda sin encender):
  `verdaderos_ganan` -> `verdaderos_perder` (fabricar_guardados.h) y `pan_cabecilla_ganan` -> `pan_cabecilla_perder` (MatrizDia1Tests.cpp);
  `emboscada_derrota` -> `emboscada_perder_es_huir`, `secta_ganan` -> `secta_perder_es_huir`, `guarida_perdida` -> `guarida_perder_es_huir` (MisionesDia1Tests.cpp).
  Eso las deja iguales a su camino «huir» en lo que se juega; la diferencia es lo que prohibe cada una (`noVe`: la flag de derrota).
- Matriz del Ocaso (`caido`, «pierde contra los saqueadores y cae») y matriz de Gorran no han necesitado cambio: sus combates no tienen flagHuida, huir enciende flagDefeat; su descripcion «pierde y cae» es ahora «huye y se lee como caida» (ver seccion 2).
- Explorador (`explorar_boundington`): cada combate de historia, 65 % gana, 15 % huye, 20 % muere y reintenta; a la tercera muerte huye (o, si es ineludible, sigue hasta ganar, con tope de seguridad de 50). Cuenta muertes en su informe.
