# MISIONES_DIA1 — inventario de misiones del día 1 (E-MISIONES-01)

Generado por `MotorGraphico/tools/matriz_misiones.py --dia primer_dia` (no editar a mano: se regenera). Fuente: `assets/adventures/boundington_primer_dia.json` (393 beats, todos asignados a una misión: 0 sin misión, 0 en dos).

**25 misiones**, **25** se pueden cerrar (alcanzabilidad estática: el beat que enciende el cierre tiene requisitos alcanzables, PNJ en un nivel y no escondido por `ocultoSi`). Cobertura del explorador (../../out/fabrica/UNI-02-11/explorar_20_post.log, 20 partidas): ver columna «vistos».

Una misión = grupo de beats con un comienzo y un cierre. `mision` tiene cierre propio; `charla` es ambiente de un PNJ (su cierre es despedirse; la flag es la de «visto»).

## Tabla

| # | misión | tipo | empieza (PNJ / nivel) | se abre con | se cierra con | recompensa | diario | vistos | ¿cierra? |
|---|---|---|---|---|---|---|---|---|---|
| 1 | **La primera noche: cama y rumores en la Taberna Humilde** `llegada_cama` | mision | b0_puerta_cerrada, skilla / interior_taberna | `b0_llegada:68` enter `interior_taberna.json` | `bnd0_dormido` (b0_subir_oido:265, b0_subir:314) | oro +0/-105 | obj_posada, obj_preguntar | 20/22 | sí |
| 2 | **Los vecinos de la ventana** `ventana` | charla | bordadora, carretero_jubilado / cueva_gorran | `b0_bordadora:2499` talk `bordadora` | `bnd0_ventana_visto` (b0_ventana_v0:2953, b0_ventana:3070) | — | obj_preguntar | 9/9 | sí |
| 3 | **El rincon de los jornaleros (Tomas, Berto y el tercero)** `jornaleros` | mision | tomas_jornalero, berto_jornalero / interior_taberna | `b0_tomas:2577` talk `tomas_jornalero` | `bnd0_rincon_hecho`, `bnd_rumor_parroquiano` (b0_rincon:3534, b1_rincon_amigos:5014, beat_parroquiano:5047) | moral: reirse_de_tomas(menor-) | obj_preguntar | 11/13 | sí |
| 4 | **El hombre de la capucha** `encapuchado` | charla | encapuchado / interior_taberna | `b0_capucha:2825` talk `encapuchado` | `bnd0_capucha_vista` (b0_capucha:2825) | — | — | 1/3 | sí |
| 5 | **El caballero que escribe (Venides, de noche)** `venides_noche` | charla | venides / interior_taberna | `b0_venides:1941` talk `venides` | `bnd0_venides_visto` (b0_venides_v0:1703, b0_venides_v1:1822, b0_venides:1941) | — | obj_preguntar | 7/9 | sí |
| 6 | **El desayuno con Venides y su encargo** `desayuno` | mision | b0_puerta_cerrada, venides / interior_taberna | `b1_desayuno:4152` auto `` | `bnd1_desayuno_hecho` (b1_desayuno_v0:3996, b1_desayuno:4152, b1_desayuno_s0_a:4307) | — | obj_preguntar | 14/14 | sí |
| 7 | **Presentarte a los tenderos y la emboscada de la plaza** `presentaciones` | mision | tendero_mercado, joyero / ciudad_centro | `beat_llegada:4725` enter `ciudad_centro.json` | `bnd_tut_cerrado` (tut_victoria:17882, tut_derrota:17923, tut_huida:17967) | oro +15/-15; nivel +1.8 | obj_tut_tendero, obj_tut_joyero, obj_tut_sastre (+1) | 9/10 | sí |
| 8 | **Recorrer la plaza y escuchar al vecindario** `plaza` | mision | anciana, vecino_de_plaza / ciudad_centro | `tut_plaza:13830` enter `ciudad_centro.json` | `bnd1_plaza_explorada` (tut_plaza:13830, tut_plaza_ya:13859) | — | obj_preguntar, obj_plaza | 7/8 | sí |
| 9 | **El carterista de la plaza (la pulga)** `carterista` | mision | carterista / ciudad_centro | `b1_carterista:13784` enter `ciudad_centro.json` | `bnd1_carterista_resuelto` (b1_carterista_pillado:26071, b1_carterista_robo:26666) | oro +10/-10; moral: carterista_perdonado(menor+), robar_al_ladron(menor-) | obj_preguntar | 7/10 | sí |
| 10 | **Aigren, el herrero (por su hijo o por los Perdidos)** `aigren` | mision | aigren / interior_herreria | `b1_aigren_charla:5478` talk `aigren` | `bnd1_aigren_resuelto` (b1_aigren_envio:6130, b1_aigren_perdidos:18107, b1_aigren_redimido:18197) | moral: aigren_presionado(medio-), aigren_redimido(medio+), aigren_saca_a_su_hijo(menor+) | obj_preguntar, bnd1_op_aigren_segunda | 23/25 | sí |
| 11 | **Luisarda, la sastra: cenar con Rosita o sonsacarla** `luisarda` | mision | skilla, luisarda / interior_sastreria | `b1_luisarda_tienda:6511` talk `luisarda` | `bnd1_luisarda_con_rosita`, `bnd1_luisarda_vista`, `bnd1_luisarda_dejada` (b1_luisarda_recado:7088, b1_luisarda:7209, b1_luisarda_pide:19003) | objetos: pocion x2, pocion x2, pocion x2; moral: luisarda_malos_modos(menor-), luisarda_silencio(menor+), luisarda_animada(menor+), luisarda_con_rosita(menor+) | obj_preguntar, bnd1_op_luisarda, bnd1_op_luisarda_segunda | 22/28 | sí |
| 12 | **Los sectarios de la plaza (la bolsa y la invitacion)** `sectarios` | mision | sectario_perdido / ciudad_centro | `b1_secta:7643` talk `sectario_perdido` | `bnd1_sectarios_resueltos` (b1_secta_s3:8089, b1_secta_burla:19384, b1_sectarios_masacre:19515) | oro +15/0 | obj_preguntar | 15/17 | sí |
| 13 | **Xila, el antiguo mercader de Pico Dragon** `xila` | mision | xila / ciudad_oeste | `b1_xila_desconocido:8693` talk `xila` | `bnd1_xila_resuelto` (b1_xila_habla_sobrio:19679, b1_xila_habla:19739) | oro +0/-4; objetos: vendas x2; moral: xila_escuchado(menor+), xila_maltratado(menor-), xila_emborrachado(menor-), xila_sin_alcohol(menor+) | obj_preguntar, obj_xila, bnd1_op_xila | 24/27 | sí |
| 14 | **La familia de la puerta azul (el gato, los Verdaderos y la cena)** `familia` | mision | nina_del_gato / ciudad_oeste | `b1_mira:10985` talk `nina_del_gato` | `bnd1_familia_resuelta` (b1_verdaderos:11558, b1_verdaderos_ganan:20038, b1_fuego_s2_a:28431) | oro +50/0; nivel +0.5; objetos: pastel x2; moral: verdaderos_enfrentados(medio+), sumarse_verdaderos(medio-), gato_bajado(menor+), verdaderos_calmados(menor+) (+2) | obj_preguntar, bnd1_op_familia | 26/29 | sí |
| 15 | **Las pruebas de la Iglesia de la Luz y el sacerdote** `iglesia` | mision | sacerdote / interior_iglesia | `beat_iglesia:12625` enter `interior_iglesia.json` | `bnd_pruebas_suficientes` (beat_iglesia:12625) | nivel +0.7 | obj_pruebas | 6/8 | sí |
| 16 | **La ronda de noche: el cuartel y el vigilante** `ronda_noche` | mision | — / interior_cuartel | `b1_cuartel_noche:13955` enter `interior_cuartel.json` | `bnd1_vigilante_visto`, `bnd1_cuartel_visto` (b1_cuartel_noche:13955, b1_vigilante:13984) | — | — | 6/6 | sí |
| 17 | **Entrar en la guarida: el herrero, las armas y la puerta** `entrada_guarida` | mision | — / interior_iglesia_s1 | `b1_guarida_puerta:14957` enter `interior_iglesia_s1.json` | `bnd1_guarida_dentro`, `bnd1_cripta_colado` (b1_guarida_puerta:14957, b1_herrero_exito:15062, b1_herrero_parcial:15105) | oro +40/0; nivel +1.0; objetos: pocion x1; moral: bajar_sin_armas(menor0), verina_sacada_de_la_guarida(medio+), unirse_perdidos(menor0) | — | 5/15 | sí |
| 18 | **Descubierto en la cripta: calmar, correr, rendirse o pelear** `descubierto` | mision | — / — | `b1_cripta_descubierto:14037` auto `` | `bnd1_capturado_resuelto`, `bnd1_cripta_huida`, `bnd1_calma_cerrado` (b1_calma_exito:14184, b1_calma_parcial:14219, b1_calma_pifia:14253) | nivel +0.5; heridas 2 | — | 1/13 | sí |
| 19 | **La ceremonia de los Perdidos y el idolo** `ceremonia` | mision | — / santuario_subterraneo | `b1_guarida_ceremonia:20283` enter `santuario_subterraneo.json` | `bnd1_ceremonia_acabada`, `bnd1_ceremonia_visto` (b1_ceremonia_acaba_sereno:14890, b1_ceremonia_acaba:14927, b1_ceremonia:27531) | nivel +0.9; moral: ver_ceremonia_gurkazaal(menor0) | obj_pruebas | 8/14 | sí |
| 20 | **El archivo y las dependencias: copistas, literas, tunica y sabotaje** `archivo` | mision | archivo_perdidos, archivero_cripta / dependencias_secta | `b1_archivo_aviso:15792` talk `archivo_perdidos` | `bnd1_estudio_visto`, `bnd1_archivo_hecho`, `bnd1_sabotaje_hecho` (b1_estudio_vuelta:17764, b1_estudio:17790) | nivel +0.8 | obj_pruebas, bnd1_op_luisarda_segunda | 19/32 | sí |
| 21 | **La camara honda: Sandes, el prisionero y el pozo** `camara_sereno` | mision | prisionero_cripta, pozo_cripta / camara_profunda | `b1_camara_sandes:20949` enter `camara_profunda.json` | `bnd1_sereno_libre`, `bnd1_sereno_llevado`, `bnd1_salir_pozo` (b1_sereno_s2:21671, b1_sereno_s3:21804, b1_sereno_s4:21968) | nivel +1.8; moral: unirse_perdidos(menor0), luisarda_delatada(medio-); heridas 1 | — | 20/27 | sí |
| 22 | **Verina, Edul y la guarida: sacar, sonsacar, unirte o acabar** `verina_edul` | mision | edul_cripta, verina_cripta / dependencias_secta | `b1_edul:23793` auto `` | `bnd1_guarida_resuelta` (b1_edul_rescate:14757, b1_edul:23793, b1_edul_cripta:23999) | oro +120/0; nivel +0.3; objetos: pocion x1; moral: unirse_perdidos(menor0), plantar_cara_guarida(menor0), dudas_en_verina(menor+), verina_sacada_de_la_guarida(medio+) (+1) | — | 17/32 | sí |
| 23 | **El informe a Venides: las pruebas y el aviso antes del alba** `informe_venides` | mision | venides / interior_taberna | `b1_borracho:11969` talk `venides` | `bnd_aviso_dado`, `bnd1_venides_informe_hecho` (b1_venides_informe_bastan:28878, b1_venides_informe:28991) | oro +60/0; nivel +0.28 | obj_pruebas, obj_venides | 10/11 | sí |
| 24 | **La pelea de la taberna y el camorrista** `pelea_taberna` | mision | — / — | `b1_pelea:26905` auto `` | `bnd1_camorrista_visto`, `bnd1_pelea_lanzada` (b1_pelea_oido:26855, b1_pelea:26905, b1_pelea_apaleado:27045) | oro +25/0; moral: camorrista_a_la_guardia(menor+), camorrista_ejecutado(mayor-) | — | 6/6 | sí |
| 25 | **Subir al cuarto y cerrar el dia (la espera de los hombres de Venides)** `cierre_dia` | mision | — / interior_taberna_p1 | `b1_sube_al_cuarto:27375` enter `interior_taberna_p1.json` | `bnd_dia_cerrado` (cierre_creadores:27402, cierre_espera:27445, cierre_silencio:27488) | nivel +1.8 | — | 5/5 | sí |

## Detalle por misión

### 1. La primera noche: cama y rumores en la Taberna Humilde (`llegada_cama`, mision)

- **Se abre**: `b0_llegada` (línea 68), trigger `enter` assets/levels/interior_taberna.json. Requisitos: no: bnd0_noche.
- **PNJ / niveles**: b0_puerta_cerrada, skilla, posadero / interior_taberna.json, ciudad_centro.json, interior_taberna_p1.json, recuerdo_taberna.json, interior_posada.json.
- **Se cierra** (flags `bnd0_dormido`): `b0_subir_oido:265` pone bnd0_dormido; `b0_subir:314` pone bnd0_dormido.
- **Recompensa**: oro neto -105 (cobros 0, pagos -105).
- **Diario**: `obj_posada` (doneFlag `bnd_skilla`, assets/adventures/boundington_primer_dia.json:39); `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45).
- **Explorador**: 20/22 beats vistos. Sin ver: b0_rosita_n_perdidos, b0_rosita_n_furia
  - Opciones que no ha elegido nadie (19): b0_rosita_n_cama#0 «¿Luisarda es amiga tuya?», b0_rosita_n_perdidos#0 «¿Qué se dice de los Perdidos?», b0_rosita_n_perdidos#1 «Bajar la voz y preguntar por los Perdidos», b0_rosita_n_perdidos#2 «Dejarlo ahí», b0_rosita_n_ella#0 «¿Cómo te llamas?», b0_rosita_n_ella#1 «Presentarte: me llamo {nombre}» (+13)
- **¿Se puede cerrar?** sí: `b0_subir_oido:265`, `b0_subir:314`

### 2. Los vecinos de la ventana (`ventana`, charla)

- **Se abre**: `b0_bordadora` (línea 2499), trigger `talk` bordadora. Requisitos: todas: bnd0_noche; no: bnd0_dormido, bnd0_ventana_abierta.
- **PNJ / niveles**: bordadora, carretero_jubilado, marido_bordadora / cueva_gorran.json, interior_taberna.json, recuerdo_taberna.json.
- **Se cierra** (flags `bnd0_ventana_visto`): `b0_ventana_v0:2953` pone bnd0_ventana_visto; `b0_ventana:3070` pone bnd0_ventana_visto.
- **Recompensa**: oro neto +0 (cobros 0, pagos 0).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45).
- **Explorador**: 9/9 beats vistos.
  - Opciones que no ha elegido nadie (4): b0_ventana_v0#3 «Volver a tu mesa», b0_ventana#2 «Hablar de otra cosa», b0_ventana_p0_a#0 «Contestar de dónde vienes», b0_ventana_p0_a#3 «Volver a tu mesa»
- **¿Se puede cerrar?** sí: `b0_ventana_v0:2953`, `b0_ventana:3070`

### 3. El rincon de los jornaleros (Tomas, Berto y el tercero) (`jornaleros`, mision)

- **Se abre**: `b0_tomas` (línea 2577), trigger `talk` tomas_jornalero. Requisitos: todas: bnd0_noche; no: bnd0_dormido, bnd0_rincon_hecho, bnd0_rincon_abierto.
- **PNJ / niveles**: tomas_jornalero, berto_jornalero, jornalero_callado, parroquiano_humilde / interior_taberna.json, recuerdo_taberna.json, ciudad_centro.json, ciudad_oeste.json, ciudad_surysal.json, cueva_gorran.json.
- **Se cierra** (flags `bnd0_rincon_hecho`, `bnd_rumor_parroquiano`): `b0_rincon:3534` pone bnd0_rincon_hecho; `b1_rincon_amigos:5014` pone bnd_rumor_parroquiano; `beat_parroquiano:5047` pone bnd_rumor_parroquiano.
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), actos reirse_de_tomas (menor-).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45).
- **Explorador**: 11/13 beats vistos. Sin ver: b0_tomas_amigo, b0_berto_ya
- **¿Se puede cerrar?** sí: `b0_rincon:3534`, `b1_rincon_amigos:5014`, `beat_parroquiano:5047`

### 4. El hombre de la capucha (`encapuchado`, charla)

- **Se abre**: `b0_capucha` (línea 2825), trigger `talk` encapuchado. Requisitos: todas: bnd0_noche; no: bnd0_dormido, bnd0_capucha_vista.
- **PNJ / niveles**: encapuchado / interior_taberna.json, recuerdo_taberna.json.
- **Se cierra** (flags `bnd0_capucha_vista`): `b0_capucha:2825` pone bnd0_capucha_vista.
- **Recompensa**: oro neto +0 (cobros 0, pagos 0).
- **Diario**: sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión.
- **Explorador**: 1/3 beats vistos. Sin ver: b0_capucha_te_mira, b0_capucha_otra
- **¿Se puede cerrar?** sí: `b0_capucha:2825`

### 5. El caballero que escribe (Venides, de noche) (`venides_noche`, charla)

- **Se abre**: `b0_venides` (línea 1941), trigger `talk` venides. Requisitos: todas: bnd0_noche; no: bnd0_dormido, bnd0_venides_visto.
- **PNJ / niveles**: venides / interior_taberna.json, recuerdo_taberna.json.
- **Se cierra** (flags `bnd0_venides_visto`): `b0_venides_v0:1703` pone bnd0_venides_visto; `b0_venides_v1:1822` pone bnd0_venides_visto; `b0_venides:1941` pone bnd0_venides_visto.
- **Recompensa**: oro neto +0 (cobros 0, pagos 0).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45).
- **Explorador**: 7/9 beats vistos (1 solo del héroe creado). Sin ver: b0_venides_v0, b0_venides_p0
  - Opciones que no ha elegido nadie (18): b0_venides_n_escribes#1 «Dejarlo ahí», b0_venides_n_el#1 «¿De dónde eres?», b0_venides_n_el#2 «Pareces preocupado.», b0_venides_n_el#3 «Dejarlo ahí», b0_venides_v0#0 «¿Qué te trae a Boundington?», b0_venides_v0#1 «¿Qué escribes?» (+12)
- **¿Se puede cerrar?** sí: `b0_venides_v0:1703`, `b0_venides_v1:1822`, `b0_venides:1941`

### 6. El desayuno con Venides y su encargo (`desayuno`, mision)

- **Se abre**: `b1_desayuno` (línea 4152), trigger `auto` (auto). Requisitos: todas: bnd0_dormido, bnd1_desayuno_llamado; no: bnd1_desayuno_hecho, bnd1_desayuno_visto.
- **PNJ / niveles**: b0_puerta_cerrada, venides / interior_taberna.json, recuerdo_taberna.json.
- **Se cierra** (flags `bnd1_desayuno_hecho`): `b1_desayuno_v0:3996` pone bnd1_desayuno_hecho; `b1_desayuno:4152` pone bnd1_desayuno_hecho; `b1_desayuno_s0_a:4307` pone bnd1_desayuno_hecho; `b1_desayuno_s1_a:4451` pone bnd1_desayuno_hecho; `b1_desayuno_s2_a:4593` pone bnd1_desayuno_hecho.
- **Recompensa**: oro neto +0 (cobros 0, pagos 0).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45).
- **Explorador**: 14/14 beats vistos.
  - Opciones que no ha elegido nadie (3): b1_desayuno_n_rosita_a#1 «Dejarlo ahí», b1_desayuno#2 «Hablar de otra cosa», b1_desayuno#3 «Ponerse en marcha»
- **¿Se puede cerrar?** sí: `b1_desayuno_v0:3996`, `b1_desayuno:4152`, `b1_desayuno_s0_a:4307`, `b1_desayuno_s1_a:4451` (+1)

### 7. Presentarte a los tenderos y la emboscada de la plaza (`presentaciones`, mision)

- **Se abre**: `beat_llegada` (línea 4725), trigger `enter` assets/levels/ciudad_centro.json. Requisitos: no: bnd_llegada.
- **PNJ / niveles**: tendero_mercado, joyero, sastre / ciudad_centro.json, interior_mercado.json, interior_joyeria.json, interior_sastreria.json.
- **Se cierra** (flags `bnd_tut_cerrado`): `tut_victoria:17882` pone bnd_tut_cerrado; `tut_derrota:17923` pone bnd_tut_cerrado; `tut_huida:17967` pone bnd_tut_cerrado.
- **Salidas** (tiradas y combates, 1): combate `tut_emboscada` → bnd_tut_saqueador_vencido/bnd_tut_saqueador_gana/bnd_tut_saqueador_huido
- **Recompensa**: oro neto +0 (cobros 15, pagos -15), nivel +1.8.
- **Diario**: `obj_tut_tendero` (doneFlag `bnd_tut_tendero`, assets/adventures/boundington_primer_dia.json:15); `obj_tut_joyero` (doneFlag `bnd_tut_joyero`, assets/adventures/boundington_primer_dia.json:21); `obj_tut_sastre` (doneFlag `bnd_tut_sastre`, assets/adventures/boundington_primer_dia.json:27); `obj_tut_emboscada` (doneFlag `bnd_tut_emboscada_resuelta`, assets/adventures/boundington_primer_dia.json:33).
- **Explorador**: 9/10 beats vistos. Sin ver: b1_reloj
- **¿Se puede cerrar?** sí: `tut_victoria:17882`, `tut_derrota:17923`, `tut_huida:17967`

### 8. Recorrer la plaza y escuchar al vecindario (`plaza`, mision)

- **Se abre**: `tut_plaza` (línea 13830), trigger `enter` assets/levels/ciudad_centro.json. Requisitos: todas: bnd_tut_cerrado; no: bnd1_plaza_explorada.
- **PNJ / niveles**: anciana, vecino_de_plaza / ciudad_centro.json, ciudad_oeste.json, ciudad_surysal.json.
- **Se cierra** (flags `bnd1_plaza_explorada`): `tut_plaza:13830` pone bnd1_plaza_explorada; `tut_plaza_ya:13859` pone bnd1_plaza_explorada.
- **Recompensa**: oro neto +0 (cobros 0, pagos 0).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45); `obj_plaza` (doneFlag `bnd1_plaza_explorada`, assets/catalogs/quests.json:181).
- **Explorador**: 7/8 beats vistos. Sin ver: tut_plaza
- **¿Se puede cerrar?** sí: `tut_plaza:13830`, `tut_plaza_ya:13859`

### 9. El carterista de la plaza (la pulga) (`carterista`, mision)

- **Se abre**: `b1_carterista` (línea 13784), trigger `enter` assets/levels/ciudad_centro.json. Requisitos: todas: bnd_llegada, bnd_tut_cerrado, bnd1_desayuno_hecho; no: bnd1_carterista_visto; vars: {"franja": {"max": 1}}.
- **PNJ / niveles**: carterista / ciudad_centro.json.
- **Se cierra** (flags `bnd1_carterista_resuelto`): `b1_carterista_pillado:26071` pone bnd1_carterista_resuelto; `b1_carterista_robo:26666` pone bnd1_carterista_resuelto.
- **Salidas** (tiradas y combates, 1): tirada `b1_carterista` [percepcion] → bnd1_carterista_pifia/bnd1_carterista_parcial/bnd1_carterista_exito/bnd1_carterista_critico
- **Recompensa**: oro neto +0 (cobros 10, pagos -10), actos carterista_perdonado (menor+), robar_al_ladron (menor-).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45).
- **Explorador**: 7/10 beats vistos (1 solo del héroe creado). Sin ver: b1_carterista_rapido, b1_pulga_p0_a, b1_carterista_robo
  - Opciones que no ha elegido nadie (4): b1_pulga_p0_a#0 «¿Cómo te llamas?», b1_pulga_p0_a#1 «¿Por qué robas?», b1_pulga_p0_a#2 «Hablar de otra cosa», b1_pulga_p0_a#3 «Dejarle ir»
- **¿Se puede cerrar?** sí: `b1_carterista_pillado:26071`, `b1_carterista_robo:26666`

### 10. Aigren, el herrero (por su hijo o por los Perdidos) (`aigren`, mision)

- **Se abre**: `b1_aigren_charla` (línea 5478), trigger `talk` aigren. Requisitos: no: bnd1_aigren_visto, bnd1_aigren_charla_visto.
- **PNJ / niveles**: aigren / interior_herreria.json.
- **Se cierra** (flags `bnd1_aigren_resuelto`): `b1_aigren_envio:6130` pone bnd1_aigren_resuelto; `b1_aigren_perdidos:18107` pone bnd1_aigren_resuelto; `b1_aigren_redimido:18197` pone bnd1_aigren_resuelto; `b1_aigren_se_va:18250` pone bnd1_aigren_resuelto; `b1_aigren_asedio:18304` pone bnd1_aigren_resuelto; `b1_aigren_discurso:18420` pone bnd1_aigren_resuelto.
- **Salidas** (tiradas y combates, 3): tirada `b1_aigren_hijo` [persuasion] → bnd1_aigren_a_pifia/bnd1_aigren_a_parcial/bnd1_aigren_a_exito/bnd1_aigren_a_critico; tirada `b1_aigren_hijo` [intimidacion] → bnd1_aigren_b_pifia/bnd1_aigren_b_parcial/bnd1_aigren_b_exito/bnd1_aigren_b_critico; tirada `b1_aigren_perdidos` [persuasion] → bnd1_aigren_c_pifia/bnd1_aigren_c_parcial/bnd1_aigren_c_exito/bnd1_aigren_c_critico
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), actos aigren_presionado (medio-), aigren_redimido (medio+), aigren_saca_a_su_hijo (menor+).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45); `bnd1_op_aigren_segunda` (doneFlag `bnd1_aigren_enterado`, assets/catalogs/quests.json:227).
- **Explorador**: 23/25 beats vistos. Sin ver: b1_aigren_discurso, b1_aigren_nada
  - Opciones que no ha elegido nadie (1): b1_aigren_charla_n_ceaseton#0 «¿Por qué te fuiste?»
- **¿Se puede cerrar?** sí: `b1_aigren_envio:6130`, `b1_aigren_perdidos:18107`, `b1_aigren_redimido:18197`, `b1_aigren_se_va:18250` (+4)

### 11. Luisarda, la sastra: cenar con Rosita o sonsacarla (`luisarda`, mision)

- **Se abre**: `b1_luisarda_tienda` (línea 6511), trigger `talk` luisarda. Requisitos: no: bnd1_luisarda_vista, bnd1_luisarda_con_rosita, bnd1_luisarda_tienda_visto.
- **PNJ / niveles**: skilla, luisarda, sastre / interior_sastreria.json, interior_taberna.json, interior_taberna_p1.json, recuerdo_taberna.json.
- **Se cierra** (flags `bnd1_luisarda_con_rosita`, `bnd1_luisarda_vista`, `bnd1_luisarda_dejada`): `b1_luisarda_recado:7088` pone bnd1_luisarda_dejada, bnd1_luisarda_vista; `b1_luisarda:7209` pone bnd1_luisarda_dejada, bnd1_luisarda_vista; `b1_luisarda_pide:19003` pone bnd1_luisarda_con_rosita; `b1_luisarda_noche_hablada:19163` pone bnd1_luisarda_con_rosita.
- **Salidas** (tiradas y combates, 6): tirada `b1_luisarda_recado` [intimidacion] → bnd1_luisarda_pifia/bnd1_luisarda_parcial/bnd1_luisarda_exito/bnd1_luisarda_critico; tirada `b1_luisarda_recado` [engano] → bnd1_luisarda_pifia/bnd1_luisarda_parcial/bnd1_luisarda_exito/bnd1_luisarda_critico; tirada `b1_luisarda_recado` [persuasion] → bnd1_luisarda_c_pifia/bnd1_luisarda_c_parcial/bnd1_luisarda_c_exito/bnd1_luisarda_c_critico; tirada `b1_luisarda` [intimidacion] → bnd1_luisarda_pifia/bnd1_luisarda_parcial/bnd1_luisarda_exito/bnd1_luisarda_critico; tirada `b1_luisarda` [engano] → bnd1_luisarda_pifia/bnd1_luisarda_parcial/bnd1_luisarda_exito/bnd1_luisarda_critico; tirada `b1_luisarda` [persuasion] → bnd1_luisarda_c_pifia/bnd1_luisarda_c_parcial/bnd1_luisarda_c_exito/bnd1_luisarda_c_critico
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), objetos pocion x2 (b1_rosita_luisarda), pocion x2 (b1_luisarda_pociones_miedo), pocion x2 (b1_luisarda_pociones), actos luisarda_malos_modos (menor-), luisarda_silencio (menor+), luisarda_animada (menor+), luisarda_con_rosita (menor+).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45); `bnd1_op_luisarda` (doneFlag `bnd1_luisarda_con_rosita`, assets/catalogs/quests.json:195); `bnd1_op_luisarda_segunda` (doneFlag `bnd_pista_luisarda`, assets/catalogs/quests.json:219).
- **Explorador**: 22/28 beats vistos. Sin ver: b1_luisarda_recado, b1_luisarda_pociones, b1_sala_p0_a, b1_luisarda_favor_cumplido, b1_luisarda_noche, b1_luisarda_calla
  - Opciones que no ha elegido nadie (12): b1_luisarda_tienda_n_rosita#0 «Que allí tienes un plato caliente.», b1_luisarda_tienda_s0#1 «Rosita dice que no vas a cenar.», b1_luisarda_tienda_p0#1 «Rosita dice que no vas a cenar.», b1_luisarda_recado#0 «Insistir, de malos modos [Intimidación]», b1_luisarda_recado#1 «Hacerle creer que ya lo sabes todo [Engaño]», b1_luisarda_recado#2 «Convencerla, con lo de Rosita [Persuasión]» (+6)
- **¿Se puede cerrar?** sí: `b1_luisarda_recado:7088`, `b1_luisarda:7209`, `b1_luisarda_pide:19003`, `b1_luisarda_noche_hablada:19163`

### 12. Los sectarios de la plaza (la bolsa y la invitacion) (`sectarios`, mision)

- **Se abre**: `b1_secta` (línea 7643), trigger `talk` sectario_perdido. Requisitos: no: bnd1_sectarios_vistos, bnd1_secta_visto.
- **PNJ / niveles**: sectario_perdido / ciudad_centro.json.
- **Se cierra** (flags `bnd1_sectarios_resueltos`): `b1_secta_s3:8089` pone bnd1_sectarios_resueltos; `b1_secta_burla:19384` pone bnd1_sectarios_resueltos; `b1_sectarios_masacre:19515` pone bnd1_sectarios_resueltos; `b1_sectarios_nada:19582` pone bnd1_sectarios_resueltos.
- **Salidas** (tiradas y combates, 3): tirada `b1_secta_s2` [persuasion] → bnd1_sectarios_pifia/bnd1_sectarios_parcial/bnd1_sectarios_exito/bnd1_sectarios_critico; tirada `b1_sectarios_seguir` [sigilo] → bnd1_seguir_pifia/bnd1_seguir_parcial/bnd1_seguir_exito/bnd1_seguir_critico; combate `b1_secta_burla` → bnd1_secta_vencidos/bnd1_secta_ganan/bnd1_secta_huido
- **Recompensa**: oro neto +15 (cobros 15, pagos 0).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45).
- **Explorador**: 15/17 beats vistos. Sin ver: b1_secta_ganan, b1_sectarios_seguidos
  - Opciones que no ha elegido nadie (5): b1_secta_v0#0 «Sí. Mi bolsa.», b1_secta_v0#1 «¿Quiénes sois?», b1_secta_v0#2 «Hablar de otra cosa», b1_secta_s2#0 «¿Qué luz?», b1_secta_burla#1 «Dejarlo ahí»
- **¿Se puede cerrar?** sí: `b1_secta_s3:8089`, `b1_secta_burla:19384`, `b1_sectarios_masacre:19515`, `b1_sectarios_nada:19582`

### 13. Xila, el antiguo mercader de Pico Dragon (`xila`, mision)

- **Se abre**: `b1_xila_desconocido` (línea 8693), trigger `talk` xila. Requisitos: no: bnd1_xila_conocido, bnd1_xila_resuelto; vars: {"franja": {"max": 1}}.
- **PNJ / niveles**: xila / ciudad_oeste.json.
- **Se cierra** (flags `bnd1_xila_resuelto`): `b1_xila_habla_sobrio:19679` pone bnd1_xila_resuelto; `b1_xila_habla:19739` pone bnd1_xila_resuelto.
- **Salidas** (tiradas y combates, 2): tirada `b1_xila_s3_a` [persuasion] → bnd1_xila_pifia/bnd1_xila_parcial/bnd1_xila_exito/bnd1_xila_critico; tirada `b1_xila_s3` [persuasion] → bnd1_xila_pifia/bnd1_xila_parcial/bnd1_xila_exito/bnd1_xila_critico
- **Recompensa**: oro neto -4 (cobros 0, pagos -4), objetos vendas x2 (b1_xila_habla_sobrio), actos xila_escuchado (menor+), xila_maltratado (menor-), xila_emborrachado (menor-), xila_sin_alcohol (menor+).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45); `obj_xila` (doneFlag `bnd1_xila_conocido`, assets/catalogs/quests.json:188); `bnd1_op_xila` (doneFlag `bnd1_aliado_xila`, assets/catalogs/quests.json:211).
- **Explorador**: 24/27 beats vistos (2 solo del héroe creado). Sin ver: b1_xila_desconocido_caster, b1_xila_desconocido_tank, b1_xila_s4_a
  - Opciones que no ha elegido nadie (24): b1_xila_desconocido_caster#0 «Sentarte con él, sin decir nada», b1_xila_desconocido_caster#1 «¿Estás bien?», b1_xila_desconocido_caster#2 «Tratarle con desprecio», b1_xila_desconocido_caster#3 «Dejarle en paz», b1_xila_desconocido_tank#0 «Sentarte con él, sin decir nada», b1_xila_desconocido_tank#1 «¿Estás bien?» (+18)
- **¿Se puede cerrar?** sí: `b1_xila_habla_sobrio:19679`, `b1_xila_habla:19739`

### 14. La familia de la puerta azul (el gato, los Verdaderos y la cena) (`familia`, mision)

- **Se abre**: `b1_mira` (línea 10985), trigger `talk` nina_del_gato. Requisitos: no: bnd1_gato_intentado, bnd1_familia_resuelta, bnd1_mira_visto; vars: {"franja": 0}.
- **PNJ / niveles**: nina_del_gato / ciudad_oeste.json, ciudad_surysal.json.
- **Se cierra** (flags `bnd1_familia_resuelta`): `b1_verdaderos:11558` pone bnd1_familia_resuelta; `b1_verdaderos_ganan:20038` pone bnd1_familia_resuelta; `b1_fuego_s2_a:28431` pone bnd1_familia_resuelta; `b1_fuego_s3_a:28574` pone bnd1_familia_resuelta.
- **Salidas** (tiradas y combates, 6): tirada `b1_mira_s1` [atletismo] → bnd1_gato_pifia/bnd1_gato_parcial/bnd1_gato_exito/bnd1_gato_critico; tirada `b1_verdaderos_forastero` [persuasion] → bnd1_verdaderos_pifia/bnd1_verdaderos_parcial/bnd1_verdaderos_exito/bnd1_verdaderos_critico; combate `b1_verdaderos_forastero` → bnd1_verdaderos_vencidos/bnd1_verdaderos_ganan/bnd1_verdaderos_huido; tirada `b1_verdaderos` [persuasion] → bnd1_verdaderos_pifia/bnd1_verdaderos_parcial/bnd1_verdaderos_exito/bnd1_verdaderos_critico; combate `b1_verdaderos` → bnd1_verdaderos_vencidos/bnd1_verdaderos_ganan/bnd1_verdaderos_huido; combate `b1_verdaderos_pelea` → bnd1_verdaderos_vencidos/bnd1_verdaderos_ganan/bnd1_verdaderos_huido
- **Recompensa**: oro neto +50 (cobros 50, pagos 0), nivel +0.5, objetos pastel x2 (rec_fuego_regalo), actos verdaderos_enfrentados (medio+), sumarse_verdaderos (medio-), gato_bajado (menor+), verdaderos_calmados (menor+), familia_a_la_barriada (menor+), cobrar_al_padre (menor-).
- **Diario**: `obj_preguntar` (doneFlag `bnd_rumor`, assets/adventures/boundington_primer_dia.json:45); `bnd1_op_familia` (doneFlag `bnd1_familia_resuelta`, assets/catalogs/quests.json:203).
- **Explorador**: 26/29 beats vistos. Sin ver: b1_familia_despues_cobrada, b1_fuego_s3_a, b1_fuego_p0_a
  - Opciones que no ha elegido nadie (13): b1_mira_v0#1 «¿Y tú cómo te llamas?», b1_mira_v0#2 «Hablar de otra cosa», b1_mira_v0#3 «Ahora no puedo», b1_fuego_s0_a#0 «¿De dónde sois?», b1_fuego_s0_a#3 «Dar las gracias e irte», b1_fuego_s2_a#0 «¿Os molestan mucho?» (+7)
- **¿Se puede cerrar?** sí: `b1_verdaderos:11558`, `b1_verdaderos_ganan:20038`, `b1_fuego_s2_a:28431`, `b1_fuego_s3_a:28574`

### 15. Las pruebas de la Iglesia de la Luz y el sacerdote (`iglesia`, mision)

- **Se abre**: `beat_iglesia` (línea 12625), trigger `enter` assets/levels/interior_iglesia.json. Requisitos: alguna: bnd_pista_xila, bnd0_pista_iglesia, bnd1_pista_iglesia_noche; no: bnd_iglesia_vista.
- **PNJ / niveles**: sacerdote / interior_iglesia.json, interior_castillo.json.
- **Se cierra** (flags `bnd_pruebas_suficientes`): `beat_iglesia:12625` pone bnd_pruebas_suficientes.
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), nivel +0.7.
- **Diario**: `obj_pruebas` (doneFlag `bnd_pruebas_suficientes`, assets/adventures/boundington_primer_dia.json:51).
- **Explorador**: 6/8 beats vistos (2 solo del héroe creado). Sin ver: b1_sacerdote_v0, b1_sacerdote_s2
  - Opciones que no ha elegido nadie (7): b1_sacerdote_v0#0 «¿Qué pasa en esta iglesia?», b1_sacerdote_v0#1 «¿Quién baja ahí?», b1_sacerdote_v0#2 «Hablar de otra cosa», b1_sacerdote_v0#3 «Irte», b1_sacerdote_s2#0 «Rezar con él», b1_sacerdote_s2#1 «Hablar de otra cosa» (+1)
- **¿Se puede cerrar?** sí: `beat_iglesia:12625`

### 16. La ronda de noche: el cuartel y el vigilante (`ronda_noche`, mision)

- **Se abre**: `b1_cuartel_noche` (línea 13955), trigger `enter` assets/levels/interior_cuartel.json. Requisitos: no: bnd1_cuartel_visto, bnd1_noche_cortada, bnd1_a_dormir, bnd_noche; vars: {"franja": 2}.
- **PNJ / niveles**: — / interior_cuartel.json, ciudad_este.json.
- **Se cierra** (flags `bnd1_vigilante_visto`, `bnd1_cuartel_visto`): `b1_cuartel_noche:13955` pone bnd1_cuartel_visto; `b1_vigilante:13984` pone bnd1_vigilante_visto.
- **Salidas** (tiradas y combates, 2): tirada `b1_vigilante` [sigilo] → bnd1_sigilo_pifia/bnd1_sigilo_parcial/bnd1_sigilo_exito/bnd1_sigilo_critico; tirada `b1_vigilante_te_para` [persuasion] → bnd1_vigilante_pifia/bnd1_vigilante_parcial/bnd1_vigilante_exito/bnd1_vigilante_critico
- **Recompensa**: oro neto +0 (cobros 0, pagos 0).
- **Diario**: sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión.
- **Explorador**: 6/6 beats vistos.
- **¿Se puede cerrar?** sí: `b1_cuartel_noche:13955`, `b1_vigilante:13984`

### 17. Entrar en la guarida: el herrero, las armas y la puerta (`entrada_guarida`, mision)

- **Se abre**: `b1_guarida_puerta` (línea 14957), trigger `enter` assets/levels/interior_iglesia_s1.json. Requisitos: no: bnd1_guarida_dentro, bnd1_guarida_resuelta.
- **PNJ / niveles**: — / interior_iglesia_s1.json, dependencias_secta.json.
- **Se cierra** (flags `bnd1_guarida_dentro`, `bnd1_cripta_colado`): `b1_guarida_puerta:14957` pone bnd1_guarida_dentro; `b1_herrero_exito:15062` pone bnd1_guarida_dentro; `b1_herrero_parcial:15105` pone bnd1_guarida_dentro; `b1_herrero_pifia:15153` pone bnd1_guarida_dentro; `b1_dep_llegada_osario:15311` pone bnd1_cripta_colado; `b1_colarse_exito:15462` pone bnd1_cripta_colado.
- **Salidas** (tiradas y combates, 10): tirada `b1_guarida_puerta` [engano] → bnd1_herrero_pifia/bnd1_herrero_parcial/bnd1_herrero_exito/bnd1_herrero_critico; combate `b1_herrero_pifia` → bnd1_puerta_ganada/bnd1_guarida_perdida/bnd1_guarida_huida; tirada `b1_dep_llegada_colarse` [sigilo] → bnd1_colarse_pifia/bnd1_colarse_parcial/bnd1_colarse_exito/bnd1_colarse_critico; tirada `b1_dep_llegada_colarse` [engano] → bnd1_novicio_pifia/bnd1_novicio_parcial/bnd1_novicio_exito/bnd1_novicio_critico; combate `b1_dep_llegada_colarse` → bnd1_cripta_ganada/bnd1_guarida_perdida/bnd1_guarida_huida; tirada `b1_colarse_parcial` [engano] → bnd1_novicio_pifia/bnd1_novicio_parcial/bnd1_novicio_exito/bnd1_novicio_critico (+4 más)
- **Recompensa**: oro neto +40 (cobros 40, pagos 0), nivel +1.0, objetos pocion x1 (b1_guarida_por_aigren), actos bajar_sin_armas (menor0), verina_sacada_de_la_guarida (medio+), unirse_perdidos (menor0).
- **Diario**: sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión.
- **Explorador**: 5/15 beats vistos. Sin ver: b1_guarida_puerta, b1_herrero_exito, b1_herrero_parcial, b1_herrero_pifia, b1_puerta_ganada, b1_dep_llegada_dentro, b1_colarse_pifia, b1_novicio_parcial, b1_novicio_pifia, b1_guarida_por_aigren
  - Opciones que no ha elegido nadie (12): b1_guarida_puerta#0 «Dejar las armas», b1_guarida_puerta#1 «Vengo de parte del herrero.», b1_guarida_puerta#2 ««Vengo de parte del herrero.» [Engaño]», b1_guarida_puerta#3 «Darte la vuelta», b1_herrero_pifia#0 «Dejar las armas y bajar», b1_herrero_pifia#1 «Apartarlos a las bravas» (+6)
- **¿Se puede cerrar?** sí: `b1_guarida_puerta:14957`, `b1_herrero_exito:15062`, `b1_herrero_parcial:15105`, `b1_herrero_pifia:15153` (+4)

### 18. Descubierto en la cripta: calmar, correr, rendirse o pelear (`descubierto`, mision)

- **Se abre**: `b1_cripta_descubierto` (línea 14037), trigger `auto` (auto). Requisitos: no: bnd1_cripta_hostil, bnd1_cripta_descubierta, bnd1_cripta_capturado, bnd1_unido_al_culto, bnd1_edul_escoltado, bnd1_pozo_usado, bnd1_cripta_echado, bnd1_cripta_huida; vars: {"cripta_sospecha": {"min": 3}}.
- **PNJ / niveles**: — / —.
- **Se cierra** (flags `bnd1_capturado_resuelto`, `bnd1_cripta_huida`, `bnd1_calma_cerrado`): `b1_calma_exito:14184` pone bnd1_calma_cerrado; `b1_calma_parcial:14219` pone bnd1_calma_cerrado; `b1_calma_pifia:14253` pone bnd1_calma_cerrado; `b1_carrera_exito:14298` pone bnd1_cripta_huida; `b1_carrera_parcial:14340` pone bnd1_cripta_huida; `b1_capturado_solo:14431` pone bnd1_capturado_resuelto.
- **Salidas** (tiradas y combates, 6): tirada `b1_cripta_descubierto` [engano] → bnd1_calma_pifia/bnd1_calma_parcial/bnd1_calma_exito/bnd1_calma_critico; tirada `b1_cripta_descubierto` [atletismo] → bnd1_carrera_pifia/bnd1_carrera_parcial/bnd1_carrera_exito/bnd1_carrera_critico; combate `b1_cripta_descubierto` → bnd1_cripta_ganada/bnd1_guarida_perdida/bnd1_guarida_huida; tirada `b1_capturado_solo` [atletismo] → bnd1_escape_pifia/bnd1_escape_parcial/bnd1_escape_exito/bnd1_escape_critico; tirada `b1_capturado` [atletismo] → bnd1_escape_pifia/bnd1_escape_parcial/bnd1_escape_exito/bnd1_escape_critico; combate `b1_escape_parcial` → bnd1_camara_ganada/bnd1_guarida_perdida/bnd1_guarida_huida
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), nivel +0.5, heridas el carcelero de la cripta (4) (b1_carrera_parcial), las cadenas de la cripta (4) (b1_escape_pifia).
- **Diario**: sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión.
- **Explorador**: 1/13 beats vistos. Sin ver: b1_cripta_descubierto, b1_calma_exito, b1_calma_parcial, b1_calma_pifia, b1_carrera_exito, b1_carrera_parcial, b1_carrera_pifia, b1_capturado_solo, b1_capturado, b1_escape_exito (+2)
  - Opciones que no ha elegido nadie (10): b1_cripta_descubierto#0 ««Sandes me ha mandado a mí.» [Engaño]», b1_cripta_descubierto#1 «Rendirte», b1_cripta_descubierto#2 «Correr hacia la salida [Atletismo]», b1_cripta_descubierto#3 «Pelear», b1_capturado_solo#0 «Romper las cadenas [Atletismo]», b1_capturado_solo#1 «Esperar a Edul» (+4)
- **¿Se puede cerrar?** sí: `b1_calma_exito:14184`, `b1_calma_parcial:14219`, `b1_calma_pifia:14253`, `b1_carrera_exito:14298` (+5)

### 19. La ceremonia de los Perdidos y el idolo (`ceremonia`, mision)

- **Se abre**: `b1_guarida_ceremonia` (línea 20283), trigger `enter` assets/levels/santuario_subterraneo.json. Requisitos: alguna: bnd1_guarida_dentro, bnd1_cripta_colado; no: bnd1_guarida_vista, bnd1_cripta_hostil, bnd1_ceremonia_acabada.
- **PNJ / niveles**: — / santuario_subterraneo.json.
- **Se cierra** (flags `bnd1_ceremonia_acabada`, `bnd1_ceremonia_visto`): `b1_ceremonia_acaba_sereno:14890` pone bnd1_ceremonia_acabada; `b1_ceremonia_acaba:14927` pone bnd1_ceremonia_acabada; `b1_ceremonia:27531` pone bnd1_ceremonia_visto.
- **Salidas** (tiradas y combates, 6): tirada `b1_santuario_intruso` [engano] → bnd1_novicio_pifia/bnd1_novicio_parcial/bnd1_novicio_exito/bnd1_novicio_critico; combate `b1_santuario_intruso` → bnd1_cripta_ganada/bnd1_guarida_perdida/bnd1_guarida_huida; tirada `b1_ceremonia` [conocimiento_arcano] → bnd1_idolo_pifia/bnd1_idolo_parcial/bnd1_idolo_exito/bnd1_idolo_critico; tirada `b1_ceremonia_s0_a` [conocimiento_arcano] → bnd1_idolo_pifia/bnd1_idolo_parcial/bnd1_idolo_exito/bnd1_idolo_critico; tirada `b1_ceremonia_s1_a` [persuasion] → bnd1_charla_pifia/bnd1_charla_parcial/bnd1_charla_exito/bnd1_charla_critico; tirada `b1_ceremonia_p0_a` [conocimiento_arcano] → bnd1_idolo_pifia/bnd1_idolo_parcial/bnd1_idolo_exito/bnd1_idolo_critico
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), nivel +0.9, actos ver_ceremonia_gurkazaal (menor0).
- **Diario**: `obj_pruebas` (doneFlag `bnd_pruebas_suficientes`, assets/adventures/boundington_primer_dia.json:51).
- **Explorador**: 8/14 beats vistos. Sin ver: b1_idolo_gurkazaal, b1_idolo_misterio, b1_ceremonia, b1_ceremonia_s0_a, b1_ceremonia_s1_a, b1_ceremonia_p0_a
  - Opciones que no ha elegido nadie (17): b1_santuario_intruso#0 «Sentarte con los novicios, con tu túnica», b1_ceremonia#0 «Mirar la estatua [Conocimiento arcano]», b1_ceremonia#1 «Escuchar el canto», b1_ceremonia#2 «Hablar de otra cosa», b1_ceremonia#3 «Volver a subir», b1_ceremonia_s0_a#0 «Mirar la estatua [Conocimiento arcano]» (+11)
- **¿Se puede cerrar?** sí: `b1_ceremonia_acaba_sereno:14890`, `b1_ceremonia_acaba:14927`, `b1_ceremonia:27531`

### 20. El archivo y las dependencias: copistas, literas, tunica y sabotaje (`archivo`, mision)

- **Se abre**: `b1_archivo_aviso` (línea 15792), trigger `talk` archivo_perdidos. Requisitos: todas: bnd1_dep_novicio_aviso; no: bnd1_archivero_fuera, bnd1_novicios_fuera, bnd1_cripta_hostil, bnd1_archivo_leido.
- **PNJ / niveles**: archivo_perdidos, archivero_cripta, literas_novicios, almacen_perdidos, copista_cripta / dependencias_secta.json.
- **Se cierra** (flags `bnd1_estudio_visto`, `bnd1_archivo_hecho`, `bnd1_sabotaje_hecho`): `b1_estudio_vuelta:17764` pone bnd1_estudio_visto; `b1_estudio:17790` pone bnd1_estudio_visto.
- **Salidas** (tiradas y combates, 8): tirada `b1_archivo_aviso` [investigacion] → bnd1_archivo_pifia/bnd1_archivo_parcial/bnd1_archivo_exito/bnd1_archivo_critico; tirada `b1_archivo_vigilado` [investigacion] → bnd1_archivo_pifia/bnd1_archivo_parcial/bnd1_archivo_exito/bnd1_archivo_critico; tirada `b1_archivero` [engano] → bnd1_archivero_eng_pifia/bnd1_archivero_eng_parcial/bnd1_archivero_eng_exito/bnd1_archivero_eng_critico; tirada `b1_archivero` [persuasion] → bnd1_archivero_per_pifia/bnd1_archivero_per_parcial/bnd1_archivero_per_exito/bnd1_archivero_per_critico; tirada `b1_archivero` [intimidacion] → bnd1_archivero_int_pifia/bnd1_archivero_int_parcial/bnd1_archivero_int_exito/bnd1_archivero_int_critico; tirada `b1_literas` [sigilo] → bnd1_tunica_pifia/bnd1_tunica_parcial/bnd1_tunica_exito/bnd1_tunica_critico (+2 más)
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), nivel +0.8.
- **Diario**: `obj_pruebas` (doneFlag `bnd_pruebas_suficientes`, assets/adventures/boundington_primer_dia.json:51); `bnd1_op_luisarda_segunda` (doneFlag `bnd_pista_luisarda`, assets/catalogs/quests.json:219).
- **Explorador**: 19/32 beats vistos (1 solo del héroe creado). Sin ver: b1_archivo_aviso, b1_archivo_parcial, b1_archivo_pifia, b1_archivero_eng_pifia, b1_archivero_per_exito, b1_archivero_per_parcial, b1_archivero_per_pifia, b1_archivero_int_parcial, b1_archivero_int_pifia, b1_tunica_exito (+3)
  - Opciones que no ha elegido nadie (12): b1_archivo_aviso#0 «Leer de reojo [Investigación]», b1_archivo_aviso#1 «Leer las Escrituras», b1_archivo_aviso#2 «Volver luego», b1_archivo_vigilado#1 «Leer las Escrituras», b1_archivo_libre#1 «Leer las Escrituras», b1_archivero#1 ««Quiero aprender a leerlas.» [Persuasión]» (+6)
- **¿Se puede cerrar?** sí: `b1_estudio_vuelta:17764`, `b1_estudio:17790`

### 21. La camara honda: Sandes, el prisionero y el pozo (`camara_sereno`, mision)

- **Se abre**: `b1_camara_sandes` (línea 20949), trigger `enter` assets/levels/camara_profunda.json. Requisitos: no: bnd1_cripta_hostil, bnd1_camara_vista, bnd1_sereno_llevado, bnd1_cripta_capturado.
- **PNJ / niveles**: prisionero_cripta, pozo_cripta / camara_profunda.json.
- **Se cierra** (flags `bnd1_sereno_libre`, `bnd1_sereno_llevado`, `bnd1_salir_pozo`): `b1_sereno_s2:21671` pone bnd1_sereno_libre; `b1_sereno_s3:21804` pone bnd1_sereno_libre; `b1_sereno_s4:21968` pone bnd1_sereno_libre; `b1_cadenas_exito:22190` pone bnd1_sereno_libre; `b1_cadenas_parcial:22228` pone bnd1_sereno_libre; `b1_pozo_salida:22451` pone bnd1_salir_pozo.
- **Salidas** (tiradas y combates, 8): tirada `b1_camara_sandes_herrero` [engano] → bnd1_sandes_eng_pifia/bnd1_sandes_eng_parcial/bnd1_sandes_eng_exito/bnd1_sandes_eng_critico; combate `b1_camara_sandes_herrero` → bnd1_camara_ganada/bnd1_guarida_perdida/bnd1_guarida_huida; tirada `b1_camara_sandes_sospecha` [engano] → bnd1_sandes_eng_pifia/bnd1_sandes_eng_parcial/bnd1_sandes_eng_exito/bnd1_sandes_eng_critico; combate `b1_camara_sandes_sospecha` → bnd1_camara_ganada/bnd1_guarida_perdida/bnd1_guarida_huida; combate `b1_camara_sandes_tunica` → bnd1_camara_ganada/bnd1_guarida_perdida/bnd1_guarida_huida; combate `b1_camara_sandes` → bnd1_camara_ganada/bnd1_guarida_perdida/bnd1_guarida_huida (+2 más)
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), nivel +1.8, heridas el borde del pozo (4) (b1_pozo_parcial), actos unirse_perdidos (menor0), luisarda_delatada (medio-).
- **Diario**: sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión.
- **Explorador**: 20/27 beats vistos (1 solo del héroe creado). Sin ver: b1_camara_sandes_herrero, b1_sandes_eng_exito, b1_sandes_eng_parcial, b1_sandes_eng_pifia, b1_sereno_s2, b1_charla_sandes, b1_sandes_killville
  - Opciones que no ha elegido nadie (23): b1_camara_sandes_herrero#0 ««Me he perdido.» [Engaño]», b1_camara_sandes_herrero#1 ««Vengo a unirme de verdad.»», b1_camara_sandes_herrero#2 «Soltarlo ahora mismo», b1_camara_sandes_herrero#3 «Volver escaleras arriba», b1_camara_sandes_sospecha#0 ««Me he perdido.» [Engaño]», b1_camara_sandes_sospecha#2 «Soltarlo ahora mismo» (+17)
- **¿Se puede cerrar?** sí: `b1_sereno_s2:21671`, `b1_sereno_s3:21804`, `b1_sereno_s4:21968`, `b1_cadenas_exito:22190` (+2)

### 22. Verina, Edul y la guarida: sacar, sonsacar, unirte o acabar (`verina_edul`, mision)

- **Se abre**: `b1_edul` (línea 23793), trigger `auto` (auto). Requisitos: todas: bnd1_con_edul; no: bnd1_edul_hablado.
- **PNJ / niveles**: edul_cripta, verina_cripta / dependencias_secta.json.
- **Se cierra** (flags `bnd1_guarida_resuelta`): `b1_edul_rescate:14757` pone bnd1_guarida_resuelta; `b1_edul:23793` pone bnd1_guarida_resuelta; `b1_edul_cripta:23999` pone bnd1_guarida_resuelta; `b1_edul_cripta_s0:24139` pone bnd1_guarida_resuelta; `b1_edul_cripta_p0:24767` pone bnd1_guarida_resuelta; `b1_verina_cripta_n_ven:25005` pone bnd1_guarida_resuelta.
- **Salidas** (tiradas y combates, 2): combate `b1_edul` → bnd1_guarida_ganada/bnd1_guarida_perdida/bnd1_guarida_huida; tirada `b1_edul_cripta_s3` [persuasion] → bnd1_llave_pifia/bnd1_llave_parcial/bnd1_llave_exito/bnd1_llave_critico
- **Recompensa**: oro neto +120 (cobros 120, pagos 0), nivel +0.3, objetos pocion x1 (b1_edul), actos unirse_perdidos (menor0), plantar_cara_guarida (menor0), dudas_en_verina (menor+), verina_sacada_de_la_guarida (medio+), guarida_exterminada (menor0).
- **Diario**: sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión.
- **Explorador**: 17/32 beats vistos. Sin ver: b1_edul_rescate, b1_charla_verina, b1_banco, b1_banco_s0_a, b1_banco_p0_a, b1_charla_edul, b1_frascos, b1_frascos_s0_a, b1_frascos_p0_a, b1_llave_exito (+5)
  - Opciones que no ha elegido nadie (40): b1_edul_rescate#0 ««Y el sereno?»», b1_edul_rescate#1 «Subir con él», b1_banco#0 «¿Qué lees?», b1_banco#1 «¿De dónde eres?», b1_banco#2 «¿Por qué estás aquí?», b1_banco#3 «Fingir que crees» (+34)
- **¿Se puede cerrar?** sí: `b1_edul_rescate:14757`, `b1_edul:23793`, `b1_edul_cripta:23999`, `b1_edul_cripta_s0:24139` (+5)

### 23. El informe a Venides: las pruebas y el aviso antes del alba (`informe_venides`, mision)

- **Se abre**: `b1_borracho` (línea 11969), trigger `talk` venides. Requisitos: no: bnd1_venides_informe_hecho, bnd1_borracho_visto; vars: {"franja": 2}.
- **PNJ / niveles**: venides / interior_taberna.json, recuerdo_taberna.json.
- **Se cierra** (flags `bnd_aviso_dado`, `bnd1_venides_informe_hecho`): `b1_venides_informe_bastan:28878` pone bnd1_venides_informe_hecho, bnd_aviso_dado; `b1_venides_informe:28991` pone bnd1_venides_informe_hecho, bnd_aviso_dado.
- **Recompensa**: oro neto +60 (cobros 60, pagos 0), nivel +0.28.
- **Diario**: `obj_pruebas` (doneFlag `bnd_pruebas_suficientes`, assets/adventures/boundington_primer_dia.json:51); `obj_venides` (doneFlag `bnd_aviso_dado`, assets/adventures/boundington_primer_dia.json:57).
- **Explorador**: 10/11 beats vistos. Sin ver: b1_borracho_p0
  - Opciones que no ha elegido nadie (7): b1_borracho_n_montanas#1 «Insistir otra vez», b1_borracho_s0#3 «Hablar de lo que has averiguado», b1_borracho_p0#0 «¿Quién es Edul?», b1_borracho_p0#1 «¿Qué pasó en las Santas Montañas?», b1_borracho_p0#2 «Hablar de otra cosa», b1_borracho_p0#3 «Hablar de lo que has averiguado» (+1)
- **¿Se puede cerrar?** sí: `b1_venides_informe_bastan:28878`, `b1_venides_informe:28991`

### 24. La pelea de la taberna y el camorrista (`pelea_taberna`, mision)

- **Se abre**: `b1_pelea` (línea 26905), trigger `auto` (auto). Requisitos: todas: bnd1_a_dormir; no: bnd1_pelea_lanzada.
- **PNJ / niveles**: — / —.
- **Se cierra** (flags `bnd1_camorrista_visto`, `bnd1_pelea_lanzada`): `b1_pelea_oido:26855` pone bnd1_pelea_lanzada; `b1_pelea:26905` pone bnd1_pelea_lanzada; `b1_pelea_apaleado:27045` pone bnd1_camorrista_visto.
- **Salidas** (tiradas y combates, 2): combate `b1_pelea_oido` → bnd1_pelea_ganada/bnd1_pelea_perdida/bnd1_pelea_huida; combate `b1_pelea` → bnd1_pelea_ganada/bnd1_pelea_perdida/bnd1_pelea_huida
- **Recompensa**: oro neto +25 (cobros 25, pagos 0), actos camorrista_a_la_guardia (menor+), camorrista_ejecutado (mayor-).
- **Diario**: sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión.
- **Explorador**: 6/6 beats vistos.
  - Opciones que no ha elegido nadie (1): b1_pelea_rosita#1 «Soltarlo»
- **¿Se puede cerrar?** sí: `b1_pelea_oido:26855`, `b1_pelea:26905`, `b1_pelea_apaleado:27045`

### 25. Subir al cuarto y cerrar el dia (la espera de los hombres de Venides) (`cierre_dia`, mision)

- **Se abre**: `b1_sube_al_cuarto` (línea 27375), trigger `enter` assets/levels/interior_taberna_p1.json. Requisitos: todas: bnd_noche; no: bnd_dia_cerrado, bnd1_en_cuarto.
- **PNJ / niveles**: — / interior_taberna_p1.json.
- **Se cierra** (flags `bnd_dia_cerrado`): `cierre_creadores:27402` pone bnd_dia_cerrado; `cierre_espera:27445` pone bnd_dia_cerrado; `cierre_silencio:27488` pone bnd_dia_cerrado.
- **Recompensa**: oro neto +0 (cobros 0, pagos 0), nivel +1.8.
- **Diario**: sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión.
- **Explorador**: 5/5 beats vistos.
- **¿Se puede cerrar?** sí: `cierre_creadores:27402`, `cierre_espera:27445`, `cierre_silencio:27488`

## Hallazgos ⚠ del análisis estático

(ninguno)

## Hallazgos ⚠ de las pruebas (fase B, `tests/Engine/MisionesDia1Tests.cpp`) y del explorador

El análisis estático da las 25 misiones por cerrables; lo que sigue sale de jugarlas con el arnés (que respeta `ocultoSi`) y del log del explorador (20 partidas).

1. ⚠ **`b1_aigren_discurso` es inalcanzable** (`boundington_primer_dia.json`, beat `b1_aigren_discurso`). Pide `bnd1_aigren_c_parcial|c_pifia` y no `bnd1_aigren_orden_pedida`, pero `b1_aigren_papel` (justo encima, mismas flags más `bnd1_aigren_visto`) salta antes y enciende `orden_pedida`. Probado (`aigren_discurso`: con tirada regular salta `b1_aigren_papel` y no `b1_aigren_discurso`). Arreglo posible (no tocado): quitar el beat o darle una condición que papel no cubra.
2. ⚠ **`tut_plaza` (enter) nunca salta**: `tut_plaza_ya` (auto, mismo efecto `bnd1_plaza_explorada`) salta en el mismo cierre del tutorial y lo tapa. Duplicado inofensivo; el explorador lo da por «no visto» siempre.
3. ⚠ **La misión «Descubierto en la cripta» (12 beats: `b1_cripta_descubierto`, calma/carrera/capturado/escape) no la ha visto ninguna de las 20 partidas del explorador** (1/13). Las escenas `descubierto_*` la juegan entera (calmar con éxito/pifia, rendirse, correr éxito/parcial/pifia, pelear) sembrando `cripta_sospecha=3`: el camino real a esa sospecha (pifias en la entrada) no lo toma el explorador.
4. ⚠ **La entrada a la guarida sin invitación (`b1_guarida_puerta`, `b1_herrero_*`, `b1_puerta_ganada`) tampoco la ve el explorador** (5/15): siempre entra invitado. Las escenas `guarida_herrero_*` y `entrada_guarida` la cubren (éxito, parcial, pifia y bajar, pifia y apartarlos a las bravas).
5. ⚠ **Lo que el explorador no ha elegido nunca** y ahora tiene escena: las pifias de Percepción del carterista (`b1_carterista_robo`), la derrota y la huida de la emboscada (`tut_derrota`, `tut_huida`), reírse de Tomás y su secuela (`b0_tomas_ofendido`), Berto (`b0_berto_ya`), «Rosita y los Perdidos / la Furia» (`b0_rosita_n_perdidos`, `b0_rosita_n_furia`, `b0_subiendo_oido`) y el encapuchado que te mira (`b0_capucha_te_mira`).
6. ⚠ **Sin escena todavía (siguen sin ver)**: `b1_luisarda_recado` y `b1_luisarda_pociones` (silencio con Luisarda), `b1_secta_ganan` / `b1_sectarios_seguidos` (la pelea y el sigilo con los sectarios), `b1_xila_desconocido_caster|tank` (el explorador no los ve; el arnés del dia 1 sí pasa por `_tank`), `b1_archivo_*` y `b1_archivero_*` con sus tiradas, `b1_literas`, `b1_tunica_*`, `b1_banco`, `b1_frascos`, `b1_llave_*`, `b1_edul_rescate`, `b1_guarida_ganada|exterminada|perdida`, `b1_idolo_misterio`. Están en la tabla de cada misión, columna «Sin ver».
7. Nota de método: los cierres de varias misiones son beats `auto` que se vuelven a disparar mientras la flag falta, así que una mutación que quite la flag de cierre hace entrar al arnés en el bucle de 150 pasos de `drena()` en lugar de dejar la escena en rojo (pasaba con `plaza`, `carterista`, `aigren`, `sectarios`, `archivo`, `cierre_dia`, `llegada_cama`). Las mutaciones de esas misiones quitan, por eso, la salida de la tirada o la flag que arranca el cierre, no el cierre mismo.
