# MATRIZ_DIA — segundo_dia

Generado por `MotorGraphico/tools/matriz_decisiones.py --dia segundo_dia` (no editar a mano: se regenera). 
Decisiones importantes: **28** beats con elecciones que leen despues del dia (de 122 beats con elecciones). Las `top 10` por peso llevan guardado y prueba (fase B).

Peso = beats/eventos/cinematicas posteriores distintos que leen (+0,5 por los de un salto) + moral (menor 2, medio 5, mayor 12) + |oro|/10 + 3 si da nivel.

## Decisiones importantes (por peso)

| # | beat:linea | peso | opcion (texto:linea) | pone | se lee despues en | consecuencia |
|---|---|---|---|---|---|---|
| 1 | b2_skilla_espada_devuelta_vista:713 (y b2_skilla_espada_devuelta:887) | 25.0 | Devolvérsela:736 | bnd2_skilla_pagada; bnd2_aliado_skilla; bnd2_tiene_aliados; confianza_rosita+1; acto:furia_de_alaro_devuelta(menor+); oro+50; nivel+0.29 | **bnd2_aliado_skilla**: adv/ocaso.json:3456 (b3_rosita_calle); **confianza_rosita**: adv/ocaso.json:6167 (b3_compania); (via b2_rosita_revela) bnd2_rosita_revelada: adv/ocaso.json:3502 (b3_rosita_calle/Gracias, Furia.) | beat b3_compania, beat b3_rosita_calle, moral furia_de_alaro_devuelta, opcion b3_rosita_calle/Gracias, Furia. |
|  |  |  | Devolvérsela y contarle todo:786 | bnd2_skilla_pagada; bnd2_aliado_skilla; bnd2_tiene_aliados; bnd2_rosita_lo_sabe; confianza_rosita+1; confianza_rosita+1; afecto_venides+1; acto:furia_de_alaro_devuelta(menor+); oro+50; nivel+0.29 | **bnd2_aliado_skilla**: adv/ocaso.json:3456 (b3_rosita_calle); **confianza_rosita**: adv/ocaso.json:6167 (b3_compania); (via b2_rosita_revela) bnd2_rosita_revelada: adv/ocaso.json:3502 (b3_rosita_calle/Gracias, Furia.) | beat b3_compania, beat b3_rosita_calle, moral furia_de_alaro_devuelta, opcion b3_rosita_calle/Gracias, Furia. |
|  |  |  | Decirle que no la encontraste:855 | bnd2_skilla_pagada; bnd2_rosita_desconfia; acto:furia_de_alaro_robada(medio-); oro-1 | **bnd2_rosita_desconfia**: adv/ocaso.json:3462 (b3_rosita_calle); adv/ocaso.json:3533 (b3_taberna_cerrada); (via b2_rosita_revela) bnd2_rosita_revelada: adv/ocaso.json:3502 (b3_rosita_calle/Gracias, Furia.) | beat b3_rosita_calle, beat b3_taberna_cerrada, moral furia_de_alaro_robada, opcion b3_rosita_calle/Gracias, Furia. |
| 2 | b2_noche_rosita_informa:13669 (y b2_venides_noche_rosita:14262, b2_venides_noche_todo:14459, b2_venides_noche_rosita2:14654, b2_venides_noche:14856) | 24.0 | Contarselo todo:13694 | bnd2_informe_dado; bnd2_venides_leal; afecto_venides+1; oro+50 | (via b2_venides_hombres) bnd_venides_10: adv/ocaso.json:1840 (b3_venides_arenga_10); adv/ocaso.json:2292 (b3_saqueadores_10) | beat b3_saqueadores_10, beat b3_venides_arenga_10 |
|  |  |  | Contarselo todo y ensenarle las pruebas:13739 | bnd2_informe_dado; bnd2_venides_leal; afecto_venides+1; oro+50 | (via b2_venides_hombres) bnd_venides_10: adv/ocaso.json:1840 (b3_venides_arenga_10); adv/ocaso.json:2292 (b3_saqueadores_10) | beat b3_saqueadores_10, beat b3_venides_arenga_10 |
|  |  |  | Contarle el ataque, pero no lo de Edul y Verina:13788 | bnd2_informe_dado; bnd2_venides_enganado; acto:venides_enganado(menor-); oro+25 | (via b2_venides_hombres) bnd_venides_10: adv/ocaso.json:1840 (b3_venides_arenga_10); adv/ocaso.json:2292 (b3_saqueadores_10) | beat b3_saqueadores_10, beat b3_venides_arenga_10, moral venides_enganado |
|  |  |  | No contarle nada:13833 | bnd2_informe_callado; bnd2_venides_sin_aviso; acto:venides_enganado(menor-) | **bnd2_venides_sin_aviso**: adv/ocaso.json:1738 (b3_aviso_en_base); adv/ocaso.json:1766 (b3_venides_retenido); eventos/boundington.json:27 (xila_avisa_a_venides); +2 mas | beat b3_aviso_en_base, beat b3_venides_retenido, evento venides_retenido, evento venides_sale_con_su_tropa, evento xila_avisa_a_venides, moral venides_enganado |
| 3 | b2_noche_rosita_informa_confianza:13530 | 18.0 | Contarselo todo:13555 | bnd2_informe_dado; bnd2_venides_leal; afecto_venides+1; oro+50 | (via b2_venides_hombres) bnd_venides_10: adv/ocaso.json:1840 (b3_venides_arenga_10); adv/ocaso.json:2292 (b3_saqueadores_10) | beat b3_saqueadores_10, beat b3_venides_arenga_10 |
|  |  |  | Contarle el ataque, pero no lo de Edul y Verina:13595 | bnd2_informe_dado; bnd2_venides_enganado; acto:venides_enganado(menor-); oro+25 | (via b2_venides_hombres) bnd_venides_10: adv/ocaso.json:1840 (b3_venides_arenga_10); adv/ocaso.json:2292 (b3_saqueadores_10) | beat b3_saqueadores_10, beat b3_venides_arenga_10, moral venides_enganado |
|  |  |  | No contarle nada:13640 | bnd2_informe_callado; bnd2_venides_sin_aviso; acto:venides_enganado(menor-) | **bnd2_venides_sin_aviso**: adv/ocaso.json:1738 (b3_aviso_en_base); adv/ocaso.json:1766 (b3_venides_retenido); eventos/boundington.json:27 (xila_avisa_a_venides); +2 mas | beat b3_aviso_en_base, beat b3_venides_retenido, evento venides_retenido, evento venides_sale_con_su_tropa, evento xila_avisa_a_venides, moral venides_enganado |
| 4 | b2_sonsaca_exito:13350 | 11.0 | Senalarselo a Venides:13389 | bnd2_noble_senalado; acto:noble_senalado(menor+) | **bnd2_noble_senalado**: adv/ocaso.json:129 (b3_noble_copas); adv/ocaso.json:1071 (b3_sermon_s3_a_miraba/Buscar al noble del brindis); adv/ocaso.json:1192 (b3_sermon_s3_a_desafiado/Buscar al noble del brindis); +1 mas | beat b3_noble_copas, moral noble_senalado rep {"perdidos": -5, "aegroum": 5}, opcion b3_sermon_s3_a/Buscar al noble del brindis, opcion b3_sermon_s3_a_desafiado/Buscar al noble del brindis, opcion b3_sermon_s3_a_miraba/Buscar al noble del brindis |
|  |  |  | Chantajearle:13405 | bnd2_noble_chantajeado; acto:noble_chantajeado(menor-); oro+30 | (moral/oro/nivel) | moral noble_chantajeado |
| 5 | b2_castillo_prueba:4176 | 9.0 | Ensenarle una prueba de los Perdidos:4197 | bnd2_guardia_preparada; bnd2_tiene_aliados; acto:guardia_preparada(menor+) | **bnd2_guardia_preparada**: adv/ocaso.json:1918 (b3_capitan_ronda); adv/ocaso.json:2344 (b3_saqueadores_guardia); catalogs/quests.json:1183 (bnd2_op_capitan_prueba) | beat b3_capitan_ronda, beat b3_saqueadores_guardia, diario bnd2_op_capitan_prueba, moral guardia_preparada rep {"aegroum": 5} |
|  |  |  | Venides hablaria por mi:4240 | bnd2_guardia_preparada; bnd2_tiene_aliados; acto:guardia_preparada(menor+) | **bnd2_guardia_preparada**: adv/ocaso.json:1918 (b3_capitan_ronda); adv/ocaso.json:2344 (b3_saqueadores_guardia); catalogs/quests.json:1183 (bnd2_op_capitan_prueba) | beat b3_capitan_ronda, beat b3_saqueadores_guardia, diario bnd2_op_capitan_prueba, moral guardia_preparada rep {"aegroum": 5} |
| 6 | b2_sectarios:10214 | 7.0 | La Barriada se arma: empalizada y guerreros:10234 | bnd2_barriada_delatada; acto:barriada_delatada(medio-) | **bnd2_barriada_delatada**: adv/ocaso.json:4625 (b3_zarela); adv/ocaso.json:6408 (b3_huida_gitanos) | beat b3_huida_gitanos, beat b3_zarela, moral barriada_delatada rep {"perdidos": 15, "barriada_surysal": -15} |
| 7 | b2_invitados:13028 | 6.0 | Sonsacar al noble que brinda [Engano]:13059 | tirada engano [botch>bnd2_sonsaca_pifia, partial>bnd2_sonsaca_parcial, success>bnd2_sonsaca_exito, critical>bnd2_sonsaca_critico] | (via b2_sonsaca_exito) bnd2_conspiradores: adv/ocaso.json:190 (b3_plaza_vacia) | beat b3_plaza_vacia |
|  |  |  | Sonsacarle, con el traje roto [Engano]:13085 | tirada engano [botch>bnd2_sonsaca_pifia, partial>bnd2_sonsaca_parcial, success>bnd2_sonsaca_exito, critical>bnd2_sonsaca_critico] | (via b2_sonsaca_exito) bnd2_conspiradores: adv/ocaso.json:190 (b3_plaza_vacia) | beat b3_plaza_vacia |
|  |  |  | Robar en el guardarropa [Sigilo]:13111 | acto:robo_en_la_fiesta(medio-); tirada sigilo [botch>bnd2_robo_pifia, partial>bnd2_robo_parcial, success>bnd2_robo_exito, critical>bnd2_robo_critico] | (moral/oro/nivel) | moral robo_en_la_fiesta |
| 8 | b2_saq_llegan:9221 | 5.0 | «Zarela conoce a vuestras madres.» [Persuasión]:9261 | bnd2_saq_decidido; tirada persuasion [botch>bnd2_saq_persu_pifia, partial>bnd2_saq_persu_parcial, success>bnd2_saq_persu_exito, critical>bnd2_saq_persu_critico] | (via b2_saq_persu_exito) bnd2_guerreros_gitanos: adv/ocaso.json:4620 (b3_zarela); (via b2_saq_persu_exito) bnd2_aliado_gitanos: adv/ocaso.json:6403 (b3_huida_gitanos) | beat b3_huida_gitanos, beat b3_zarela |
|  |  |  | Desenvainar la Furia de Alaro [Intimidación]:9286 | bnd2_saq_decidido; bnd2_furia_vista_saqueadores; tirada intimidacion [botch>bnd2_saq_furia_pifia, partial>bnd2_saq_furia_parcial, success>bnd2_saq_furia_exito, critical>bnd2_saq_furia_critico] | (via b2_saq_furia_exito) bnd2_guerreros_gitanos: adv/ocaso.json:4620 (b3_zarela); (via b2_saq_furia_exito) bnd2_aliado_gitanos: adv/ocaso.json:6403 (b3_huida_gitanos) | beat b3_huida_gitanos, beat b3_zarela |
|  |  |  | Comprarles la noche (30 de oro):9324 | bnd2_saq_decidido; bnd2_saq_pagados; acto:saqueadores_pagados(menor0); oro-30 | (moral/oro/nivel) |  |
| 9 | b2_edul_fiesta:10822 | 4.0 | Hablarle de Venides:10846 | bnd2_edul_fiesta_hecho; bnd2_edul_sabe_venides; bnd2_con_edul; bnd2_fiesta_dentro; bnd2_fiesta_intentada; bnd2_vigilante_resuelto; acto:dudas_en_edul(menor+) | **bnd2_edul_sabe_venides**: adv/ocaso.json:5618 (b3_edul_fiesta); (via b2_dentro) bnd2_fiesta_nobles: adv/ocaso.json:186 (b3_plaza_vacia) | beat b3_edul_fiesta, beat b3_plaza_vacia, moral dudas_en_edul |
|  |  |  | ¿Quien es el anfitrion?:10885 | bnd2_edul_fiesta_hecho; bnd2_con_edul; bnd2_fiesta_dentro; bnd2_fiesta_intentada; bnd2_vigilante_resuelto | (via b2_dentro) bnd2_fiesta_nobles: adv/ocaso.json:186 (b3_plaza_vacia) | beat b3_plaza_vacia |
| 10 | b2_luisarda_disfraz:16381 | 4.0 | Comprar el traje de gala (40):16414 | bnd2_disfraz; bnd2_disfraz_musico; bnd2_disfraz_resuelto; oro-40 | (moral/oro/nivel) |  |
| 11 | b2_boutique_robo_mal:16513 | 4.0 | Pagar (40):16541 | bnd2_disfraz; bnd2_disfraz_musico; bnd2_disfraz_resuelto; oro-40 | (moral/oro/nivel) |  |
| 12 | b2_xila_resaca:16012 | 3.0 | Despertarle y hablarle claro [Persuasión]:16040 | tirada persuasion [botch>bnd2_resaca_pifia, partial>bnd2_resaca_parcial, success>bnd2_resaca_exito, critical>bnd2_resaca_critico] | (via b2_xila_resaca_bien) bnd1_aliado_xila: catalogs/quests.json:214 (bnd1_op_xila); (via b2_xila_resaca_bien) bnd1_xila_honor: adv/ocaso.json:2037 (b3_xila_vendas); adv/ocaso.json:6864 (b3_epi_xila); +1 mas | beat b3_epi_xila, beat b3_xila_vendas, diario bnd1_op_xila, evento xila_avisa_a_venides |
|  |  |  | Pagarle un desayuno caliente (6):16060 | bnd2_resaca_exito; oro-6 | (via b2_xila_resaca_bien) bnd1_aliado_xila: catalogs/quests.json:214 (bnd1_op_xila); (via b2_xila_resaca_bien) bnd1_xila_honor: adv/ocaso.json:2037 (b3_xila_vendas); adv/ocaso.json:6864 (b3_epi_xila); +1 mas | beat b3_epi_xila, beat b3_xila_vendas, diario bnd1_op_xila, evento xila_avisa_a_venides |
| 13 | b2_rosita_revela:1057 | 2.0 | ¿Qué fue de los otros tres?:1094 | bnd2_rosita_semilla | **bnd2_rosita_semilla**: adv/ocaso.json:6159 (b3_compania) | beat b3_compania |
|  |  |  | No preguntar más:1112 | bnd2_rosita_semilla | **bnd2_rosita_semilla**: adv/ocaso.json:6159 (b3_compania) | beat b3_compania |
| 14 | b2_capitan:3940 | 2.0 | Pedirle que prepare a la guardia [Persuasion]:3964 | bnd2_capitan_hablado; tirada persuasion [botch>bnd2_capitan_pifia, partial>bnd2_capitan_parcial, success>bnd2_capitan_exito, critical>bnd2_capitan_critico] | (via b2_capitan_orden) bnd2_orden_capitan: catalogs/quests.json:238 (bnd1_op_aigren_orden); (via b2_capitan_convencido) bnd2_guardia_preparada: adv/ocaso.json:1918 (b3_capitan_ronda); adv/ocaso.json:2344 (b3_saqueadores_guardia); +1 mas | beat b3_capitan_ronda, beat b3_saqueadores_guardia, diario bnd1_op_aigren_orden, diario bnd2_op_capitan_prueba |
|  |  |  | Soy el que te trajo al camorrista anoche.:3994 | bnd2_capitan_hablado; bnd2_capitan_exito | (via b2_capitan_orden) bnd2_orden_capitan: catalogs/quests.json:238 (bnd1_op_aigren_orden); (via b2_capitan_convencido) bnd2_guardia_preparada: adv/ocaso.json:1918 (b3_capitan_ronda); adv/ocaso.json:2344 (b3_saqueadores_guardia); +1 mas | beat b3_capitan_ronda, beat b3_saqueadores_guardia, diario bnd1_op_aigren_orden, diario bnd2_op_capitan_prueba |
|  |  |  | No molestarle:4040 | bnd2_capitan_hablado | (via b2_capitan_orden) bnd2_orden_capitan: catalogs/quests.json:238 (bnd1_op_aigren_orden) | diario bnd1_op_aigren_orden |
| 15 | b2_cobrador:10611 | 2.0 | Pagar (20):10633 | bnd2_cobrador_resuelto; bnd2_en_la_mansion; bnd2_fiesta_intentada; bnd2_vigilante_resuelto; oro-20 | (moral/oro/nivel) |  |
| 16 | b2_fiesta_nobles:11495 (y b2_fiesta_nobles_s0:11588, b2_fiesta_nobles_p0:11681) | 2.0 | Como va la noche?:11522 | bnd2_fiesta_nobles_t_fiesta; bnd2_fiesta_nobles | **bnd2_fiesta_nobles**: adv/ocaso.json:186 (b3_plaza_vacia); (via b2_cierre_todo_xila_vela) bnd2_dia_cerrado: adv/ocaso.json:178 (b3_plaza_vacia); adv/ocaso.json:235 (b3_plaza); +1 mas | beat b3_plaza, beat b3_plaza_vacia, codigo codigo |
| 17 | b2_gitanos_puerta_cerrada:16655 | 2.0 | Volver con un saco de grano (10) y cargar tablas:16686 | bnd2_zarela_confia; bnd2_aliado_gitanos; bnd2_tiene_aliados; bnd2_gitanos_resuelto; oro-10 | **bnd2_aliado_gitanos**: adv/ocaso.json:6403 (b3_huida_gitanos) | beat b3_huida_gitanos |
| 18 | b2_skilla_hora:1203 (y b2_posadero_hora:1289) | 1.5 | Subir a dormir:1261 | bnd2_a_dormir | (via b2_cierre_todo_xila_vela) bnd2_dia_cerrado: adv/ocaso.json:178 (b3_plaza_vacia); adv/ocaso.json:235 (b3_plaza); +1 mas | beat b3_plaza, beat b3_plaza_vacia, codigo codigo |
| 19 | b2_fiesta_copas:12964 | 1.5 | Tres copas de mi cuenta (6 de oro):12986 | bnd2_noble_borracho; bnd2_sonsaca_exito; bnd2_noble_copas_ofrecidas; acto:noble_emborrachado(menor0); oro-6 | **bnd2_noble_borracho**: adv/ocaso.json:126 (b3_noble_copas); (via b2_sonsaca_exito) bnd2_conspiradores: adv/ocaso.json:190 (b3_plaza_vacia) | beat b3_noble_copas, beat b3_plaza_vacia |
| 20 | b2_sonsaca_fallo:13433 | 1.5 | Invitarle a tres copas (6 de oro):13460 | bnd2_noble_borracho; bnd2_sonsaca_exito; acto:noble_emborrachado(menor0); oro-6 | **bnd2_noble_borracho**: adv/ocaso.json:126 (b3_noble_copas); (via b2_sonsaca_exito) bnd2_conspiradores: adv/ocaso.json:190 (b3_plaza_vacia) | beat b3_noble_copas, beat b3_plaza_vacia |
| 21 | b2_xila_resaca_mal:16134 | 1.5 | Pagarle el desayuno de todas formas (6):16162 | bnd2_resaca_exito; oro-6 | (via b2_xila_resaca_bien) bnd1_aliado_xila: catalogs/quests.json:214 (bnd1_op_xila); (via b2_xila_resaca_bien) bnd1_xila_honor: adv/ocaso.json:2037 (b3_xila_vendas); adv/ocaso.json:6864 (b3_epi_xila); +1 mas | beat b3_epi_xila, beat b3_xila_vendas, diario bnd1_op_xila, evento xila_avisa_a_venides |
| 22 | b2_edul_herreria_s1:1750 | 1.0 | ¿Que hay en esos frascos?:1803 | bnd2_edul_herreria_t_frascos; bnd2_edul_frascos; bnd1_edul_mezcla | **bnd2_edul_frascos**: adv/ocaso.json:5012 (b3_barricada_rota/Buscar dónde no hay frascos); **bnd1_edul_mezcla**: adv/ocaso.json:5011 (b3_barricada_rota/Buscar dónde no hay frascos) | opcion b3_barricada_rota/Buscar dónde no hay frascos |
| 23 | b2_capitan_muerto:3848 | 1.0 | Pedirle que prepare a la guardia [Persuasion]:3872 | bnd2_capitan_hablado; tirada persuasion [botch>bnd2_capitan_pifia, partial>bnd2_capitan_parcial, success>bnd2_capitan_exito, critical>bnd2_capitan_critico] | (via b2_capitan_orden) bnd2_orden_capitan: catalogs/quests.json:238 (bnd1_op_aigren_orden); (via b2_capitan_convencido) bnd2_guardia_preparada: adv/ocaso.json:1918 (b3_capitan_ronda); adv/ocaso.json:2344 (b3_saqueadores_guardia); +1 mas | beat b3_capitan_ronda, beat b3_saqueadores_guardia, diario bnd1_op_aigren_orden, diario bnd2_op_capitan_prueba |
|  |  |  | No molestarle:3926 | bnd2_capitan_hablado | (via b2_capitan_orden) bnd2_orden_capitan: catalogs/quests.json:238 (bnd1_op_aigren_orden) | diario bnd1_op_aigren_orden |
| 24 | b2_mansion:11072 | 1.0 | Entrar por la puerta, con disfraz [Interpretacion]:11094 | bnd2_mansion_vista; tirada interpretacion [botch>bnd2_interpreta_pifia, partial>bnd2_interpreta_parcial, success>bnd2_interpreta_exito, critical>bnd2_interpreta_critico] | (via b2_disfraz_sospechas) bnd2_fiesta_nobles: adv/ocaso.json:186 (b3_plaza_vacia) | beat b3_plaza_vacia |
|  |  |  | Escuchar desde el jardin [Percepcion]:11123 | bnd2_mansion_vista; tirada percepcion [botch>bnd2_escucha_pifia, partial>bnd2_escucha_parcial, success>bnd2_escucha_exito, critical>bnd2_escucha_critico] | (via b2_escucha_rumores) bnd2_fiesta_rumores: adv/ocaso.json:185 (b3_plaza_vacia); (via b2_escucha_rumores) bnd2_fiesta_nobles: adv/ocaso.json:186 (b3_plaza_vacia) | beat b3_plaza_vacia |
| 25 | b2_noche_papeles:13862 | 1.0 | Dejarle los papeles sin hablar:13889 | bnd2_informe_dado; bnd2_papeles_mostrados | (via b2_venides_hombres) bnd_venides_10: adv/ocaso.json:1840 (b3_venides_arenga_10); adv/ocaso.json:2292 (b3_saqueadores_10) | beat b3_saqueadores_10, beat b3_venides_arenga_10 |
| 26 | b2_vecina_ventana:13917 (y b2_vecina_ventana_s0:14032, b2_vecina_ventana_p0:14147) | 1.0 | Que has visto?:13944 | bnd2_vecina_ventana_t_cortinas; bnd2_pista_cortinas | **bnd2_pista_cortinas**: adv/ocaso.json:191 (b3_plaza_vacia) | beat b3_plaza_vacia |
| 27 | b2_gitanos_recelo:16574 | 1.0 | Cargar agua del río hasta llenar los cubos:16604 | bnd2_zarela_confia; bnd2_aliado_gitanos; bnd2_tiene_aliados; bnd2_gitanos_resuelto; nivel+0.25 | **bnd2_aliado_gitanos**: adv/ocaso.json:6403 (b3_huida_gitanos) | beat b3_huida_gitanos |
| 28 | b2_fiesta_cocina:10298 | 0.5 | Entrar por la cocina:10320 | bnd2_fiesta_segunda_usada; bnd2_fiesta_dentro; quita bnd2_fiesta_resuelta, bnd2_convence_pifia, bnd2_convence_parcial, bnd2_echado, bnd2_arriba_pifia, bnd2_arriba_parcial, bnd2_robo_pifia, bnd2_robo_parcial, bnd2_despacho_intentado | (via b2_dentro) bnd2_fiesta_nobles: adv/ocaso.json:186 (b3_plaza_vacia) | beat b3_plaza_vacia |

## Avisos automaticos (marcar ⚠ tras revisar a mano)

- `flag-sin-lector` b2_skilla_espada_devuelta_vista:736 — bnd2_tiene_aliados (Devolvérsela)
- `flag-sin-lector` b2_skilla_espada_devuelta_vista:786 — bnd2_tiene_aliados (Devolvérsela y contarle todo)
- `flag-sin-lector` b2_skilla_espada_devuelta:907 — bnd2_tiene_aliados (Devolvérsela)
- `flag-sin-lector` b2_skilla_espada_devuelta:957 — bnd2_tiene_aliados (Devolvérsela y contarle todo)
- `flag-sin-lector` b2_capitan_muerto:3902 — bnd2_pista_guardia (¿Cuantos hombres tienes?)
- `flag-sin-lector` b2_capitan:4016 — bnd2_pista_guardia (¿Cuantos hombres tienes?)
- `flag-sin-lector` b2_castillo_prueba:4197 — bnd2_tiene_aliados (Ensenarle una prueba de los Perdidos)
- `flag-sin-lector` b2_castillo_prueba:4240 — bnd2_tiene_aliados (Venides hablaria por mi)
- `flag-sin-lector` b2_disfraz_hallado_xila:4503 — bnd2_disfraz_noble (De noble excéntrico de Ascaria)
- `flag-sin-lector` b2_disfraz_hallado_xila:4522 — bnd2_disfraz_actor (De actor)
- `flag-sin-lector` b2_disfraz_hallado:4594 — bnd2_disfraz_noble (De noble excéntrico de Ascaria)
- `flag-sin-lector` b2_disfraz_hallado:4613 — bnd2_disfraz_actor (De actor)
- `flag-sin-lector` b2_xila_escalon:4710 — bnd2_xila_te_lleva (¿Que sabes hoy?)
- `flag-sin-lector` b2_xila_escalon_s0:4816 — bnd2_xila_te_lleva (¿Que sabes hoy?)
- `flag-sin-lector` b2_xila_escalon_p0:4923 — bnd2_xila_te_lleva (¿Que sabes hoy?)
- `flag-sin-lector` b2_forjador:5082 — bnd2_pista_surysal_prisa (¿Para qué tanta prisa?)
- `flag-sin-lector` b2_forjador:5108 — bnd1_pista_oro_nuevo (¿Quién os paga?)
- `flag-sin-lector` b2_forjador:5137 — bnd2_pista_surysal_instructor (¿Quién os enseña a pelear?)
- `flag-sin-lector` b2_forjador_s0:5207 — bnd2_pista_surysal_prisa (¿Para qué tanta prisa?)
- `flag-sin-lector` b2_forjador_s0:5233 — bnd1_pista_oro_nuevo (¿Quién os paga?)
- `flag-sin-lector` b2_forjador_p0:5306 — bnd2_pista_surysal_prisa (¿Para qué tanta prisa?)
- `flag-sin-lector` b2_forjador_p0:5332 — bnd1_pista_oro_nuevo (¿Quién os paga?)
- `flag-sin-lector` b2_vecina_horca:5406 — bnd2_pista_surysal_horcas (¿Una horca como una lanza?)
- `flag-sin-lector` b2_vecina_horca_s0:5514 — bnd2_pista_surysal_horcas (¿Una horca como una lanza?)
- `flag-sin-lector` b2_vecina_horca_s1:5646 — bnd2_pista_surysal_campanas (¿Qué día es «el día»?)
- `flag-sin-lector` b2_vecina_horca_p0:5734 — bnd2_pista_surysal_horcas (¿Una horca como una lanza?)
- `flag-sin-lector` b2_guardian_charla:5867 — bnd2_pista_puente_vigilado (¿Qué pasa en el puente?)
- `flag-sin-lector` b2_guardian_charla_s0:5973 — bnd2_pista_puente_vigilado (¿Qué pasa en el puente?)
- `flag-sin-lector` b2_guardian_charla_p0:6185 — bnd2_pista_puente_vigilado (¿Qué pasa en el puente?)
- `flag-sin-lector` b2_padre_barriada:7090 — bnd2_pista_padre_arde (¿Volveréis a la ciudad?)
- `flag-sin-lector` b2_padre_barriada_s0:7204 — bnd2_pista_padre_arde (¿Volveréis a la ciudad?)
- `flag-sin-lector` b2_padre_barriada_p0:7321 — bnd2_pista_padre_arde (¿Volveréis a la ciudad?)
- `flag-sin-lector` b2_guardian_segunda:7396 — bnd2_guardian_favor (¿Qué favor?)
- `flag-sin-lector` b2_instructor_gris:7653 — bnd2_instructor_puente_preguntado (Quien vigila el puente?)
- `flag-sin-lector` b2_instructor_gris_s0:7787 — bnd2_instructor_puente_preguntado (Quien vigila el puente?)
- `flag-sin-lector` b2_instructor_gris_p0:7922 — bnd2_instructor_puente_preguntado (Quien vigila el puente?)
- `flag-sin-lector` b2_instructor_fallo:8068 — bnd2_pista_puente (Preguntar por el puente)
- `flag-sin-lector` b2_zarela_s2_a:8589 — bnd2_pista_perdidos_cuentan (¿Han venido los Perdidos por aquí?)
- `flag-sin-lector` b2_saqueadores_ganan:9943 — rec_amuleto_velar (Quedarte a velar la noche con ellos)
- `flag-sin-lector` b2_fiesta_cocina:10320 — bnd2_fiesta_segunda_usada (Entrar por la cocina)
- `flag-sin-lector` b2_fiesta_puerta:10414 — bnd2_fiesta_intentada (Esquivar al vigilante [Sigilo])
- `flag-sin-lector` b2_vigilante_charla:10579 — bnd2_vigilante_amigo (¿Muchos anos de vigilante?)
- `flag-sin-lector` b2_cobrador:10696 — bnd2_cobrador_amenazado (Amenazarle)
- `flag-sin-lector` b2_cobrador:10730 — bnd2_cobrador_hablado (¿Quien no paga en este barrio? [Persuasion])
- `flag-sin-lector` b2_edul_fiesta:10846 — bnd2_con_edul (Hablarle de Venides)
- `flag-sin-lector` b2_edul_fiesta:10885 — bnd2_con_edul (¿Quien es el anfitrion?)
- `flag-sin-lector` b2_vigilante_te_para:10940 — bnd2_vigilante_hablado (Convencerle de que te deje pasar [Persuasion])
- `flag-sin-lector` b2_vigilante_te_para:10964 — bnd2_vigilante_hablado (Buscar otra ruta por los jardines)
- `flag-sin-lector` b2_vigilante_te_para:10964 — bnd2_por_los_jardines (Buscar otra ruta por los jardines)
- `flag-sin-lector` b2_vigilante_te_para:10989 — bnd2_vigilante_hablado (Irte)
- `flag-sin-lector` b2_mansion:11146 — bnd2_escalada_pedida (Saltar la verja y entrar por una ventana [Atletismo])
- `flag-sin-lector` b2_fiesta_mercaderes:11801 — bnd2_fiesta_mercaderes (Como va la noche?)
- `flag-sin-lector` b2_fiesta_mercaderes_s0:11893 — bnd2_fiesta_mercaderes (Como va la noche?)
- `flag-sin-lector` b2_fiesta_mercaderes_p0:11986 — bnd2_fiesta_mercaderes (Como va la noche?)
- `flag-sin-lector` b2_fiesta_musicos:12080 — bnd2_cuarteto_pista (Como va la noche?)
- `flag-sin-lector` b2_fiesta_musicos_s0:12172 — bnd2_cuarteto_pista (Como va la noche?)
- `flag-sin-lector` b2_fiesta_musicos_p0:12265 — bnd2_cuarteto_pista (Como va la noche?)
- `flag-sin-lector` b2_fiesta_criados:12359 — bnd2_criado_pista (Como va la noche?)
- `flag-sin-lector` b2_fiesta_criados_s0:12451 — bnd2_criado_pista (Como va la noche?)
- `flag-sin-lector` b2_fiesta_criados_p0:12544 — bnd2_criado_pista (Como va la noche?)
- `flag-sin-lector` b2_fiesta_anfitriona:12638 — bnd2_traje_notado (Como va la noche?)
- `flag-sin-lector` b2_fiesta_anfitriona_s0:12730 — bnd2_traje_notado (Como va la noche?)
- `flag-sin-lector` b2_fiesta_anfitriona_p0:12823 — bnd2_traje_notado (Como va la noche?)
- `flag-sin-lector` b2_dentro:12930 — bnd2_despacho_buscado (Subir al despacho)
- `flag-sin-lector` b2_fiesta_copas:12986 — bnd2_noble_copas_ofrecidas (Tres copas de mi cuenta (6 de oro))
- `flag-sin-lector` b2_fiesta_copas:13012 — bnd2_noble_copas_ofrecidas (Prefiero hacerle preguntas)
- `flag-sin-lector` b2_sonsaca_exito:13405 — bnd2_noble_chantajeado (Chantajearle)
- `flag-sin-lector` b2_noche_rosita_informa_confianza:13595 — bnd2_venides_enganado (Contarle el ataque, pero no lo de Edul y Verina)
- `flag-sin-lector` b2_noche_rosita_informa:13788 — bnd2_venides_enganado (Contarle el ataque, pero no lo de Edul y Verina)
- `flag-sin-lector` b2_noche_papeles:13889 — bnd2_papeles_mostrados (Dejarle los papeles sin hablar)
- `flag-sin-lector` b2_noche_papeles:13905 — bnd2_papeles_prefiere_hablar (Prefiero contartelo yo)
- `flag-sin-lector` b2_venides_noche_rosita:14385 — bnd2_venides_enganado (Contarle el ataque, pero no lo de Edul y Verina)
- `flag-sin-lector` b2_venides_noche_todo:14580 — bnd2_venides_enganado (Contarle el ataque, pero no lo de Edul y Verina)
- `flag-sin-lector` b2_venides_noche_rosita2:14782 — bnd2_venides_enganado (Contarle el ataque, pero no lo de Edul y Verina)
- `flag-sin-lector` b2_venides_noche:14982 — bnd2_venides_enganado (Contarle el ataque, pero no lo de Edul y Verina)
- `flag-sin-lector` b2_gitanos_recelo:16604 — bnd2_tiene_aliados (Cargar agua del río hasta llenar los cubos)
- `flag-sin-lector` b2_gitanos_puerta_cerrada:16686 — bnd2_tiene_aliados (Volver con un saco de grano (10) y cargar tablas)
- `flag-sin-lector` b2_fiesta_vuelta:16764 — bnd2_fiesta_vuelta_usada (Volver y pagar al portero (10))
- `flag-sin-lector` b2_fiesta_vuelta:16804 — bnd2_fiesta_vuelta_usada (Dejarlo estar)
- `opciones-iguales` b2_rosita_revela:1094 — ¿Qué fue de los otros tres? | No preguntar más (bnd2_rosita_semilla)
- `opciones-iguales` b2_castillo_prueba:4197 — Ensenarle una prueba de los Perdidos | Venides hablaria por mi (bnd2_guardia_preparada,bnd2_tiene_aliados)
- `opciones-iguales` b2_vigilante_charla:10503 — Convencerle de que te deje pasar [Persuasion] | Vengo a tocar en la fiesta. [Persuasion] (bnd2_convence_critico,bnd2_convence_exito,bnd2_convence_parcial,bnd2_convence_pifia,bnd2_fiesta_intentada,bnd2_vigilante_charlado,bnd2_vigilante_hablado)
- `opciones-iguales` b2_invitados:13059 — Sonsacar al noble que brinda [Engano] | Sonsacarle, con el traje roto [Engano] (bnd2_sonsaca_critico,bnd2_sonsaca_exito,bnd2_sonsaca_parcial,bnd2_sonsaca_pifia)
- `opciones-iguales` b2_noche_rosita_informa:13694 — Contarselo todo | Contarselo todo y ensenarle las pruebas (bnd2_informe_dado,bnd2_venides_leal)
- `opciones-iguales` b2_venides_noche_rosita:14291 — Contarselo todo | Contarselo todo y ensenarle las pruebas (bnd2_informe_dado,bnd2_venides_leal)
- `opciones-iguales` b2_venides_noche_todo:14486 — Contarselo todo | Contarselo todo y ensenarle las pruebas (bnd2_informe_dado,bnd2_venides_leal)
- `opciones-iguales` b2_venides_noche_rosita2:14688 — Contarselo todo | Contarselo todo y ensenarle las pruebas (bnd2_informe_dado,bnd2_venides_leal)
- `opciones-iguales` b2_venides_noche:14888 — Contarselo todo | Contarselo todo y ensenarle las pruebas (bnd2_informe_dado,bnd2_venides_leal)

## A0 — Los dos ocultos que dejó E-GORRAN-TREGUA (resueltos en el arnés; ahora cualquier oculto hace fallar)

1. **`seguidor_brinda` (mansión, día 3).** Un jugador real entra en la fiesta llegando de noche (franja 2) a `ciudad_este` con `bnd2_encargo` y SIN haberse acostado: `b2_fiesta_puerta` (J:10381, `enter` ciudad_este, `notFlags bnd2_a_dormir`) salta y ofrece las entradas (esquivar al vigilante, hablar con él, pagar al cobrador, Edul, la cocina) que acaban en `bnd2_fiesta_dentro`. El fallo era del arnés: tras la Barriada (cinemática `barriada_anochece_dentro`) ya es de noche, `anochece()` hablaba igualmente con Rosita y, como «Esperar a que anochezca» ya no se ofrece, cogía «Subir a dormir» → `bnd2_a_dormir` → la puerta no salta. Arreglo: `Jugador::anochece()` no hace nada si en el día 3 ya es franja 2. Además, con mala suerte (el vigilante, la puerta) NO se entra en la fiesta y el noble no está: `demo_campana_boundington` solo le habla si `seVe("seguidor_brinda")`.
2. **`venides_base` (cuartel del Ocaso).** Con `bnd2_venides_sin_aviso` (callaste el informe: «No contarle nada») Venides NO está en la base (cinematicas.json:612-615 `si bnd2_dia_cerrado / sin bnd2_venides_sin_aviso`) y el Ocaso lo cuenta él mismo (`b3_venides_retenido` O:1766: «El caballero salió solo esta mañana…»). El jugador no le habla: `ocasoBase()` solo lo hace si `seVe("venides_base")`; las campanas suenan solas al ponerse el sol.
3. `Jugador::habla` ya no tiene lista de pendientes: hablar con quien está oculto hace `require(false)` (con `BND_OCULTO_AVISA=1` solo lista).

## Hallazgos ⚠ (revisados a mano sobre los avisos y comprobados jugando; las pruebas `hallazgo_*` los fijan)

1. ⚠ **`b2_xila_resaca` (el desayuno a Xila borracho) no se alcanza jugando.** Es `talk` con `xila` (S:16012, exige `bnd2_encargo` y `bnd1_xila_borracho`), pero cinematicas.json:398 esconde `xila` en cuanto hay `bnd2_encargo` (salvo `bnd_ocaso_empezado`). Las tres opciones (`Despertarle [Persuasión]`, `Pagarle un desayuno (6)`, `Dejarle dormir`) y las variantes `b2_xila_resaca_bien/_mal` (que darían `bnd1_aliado_xila` y `bnd1_xila_honor` a un Xila borracho) son letra muerta; Xila borracho no se recupera el día 3 (`hallazgo_xila_resaca_no_se_alcanza_porque_xila_esta_oculto_el_dia_3`). No se toca la historia.
2. ⚠ **Verina arriba se alcanza, pero solo con una combinación y sin consecuencia.** `b2_rosita_verina_arriba` (S:1172) exige `bnd1_verina_salvada_guarida` (solo «Sacar a Verina de aquí», ruta de Aigren) y `confianza_rosita >= 3`. Las fuentes son +1 `b1_pelea` («Proteger al borracho dormido») / `b1_rosita_luisarda`, +1 «Devolvérsela» y +2 «Devolvérsela y contarle todo»: **sí** llega a 3 con Verina salvada + proteger al borracho + devolver y contarlo todo (camino `verina_arriba`), y con ninguna otra de las ya probadas (devolver y contarlo todo da 2). Y `bnd2_rosita_verina` solo la lee el diario (quests.json:746): ningún beat del Ocaso cambia (`hallazgo_verina_arriba_solo_la_lee_el_diario`).
3. ⚠ **El aviso de Xila es frágil.** `xila_avisa_a_venides` (eventos/boundington.json:25) exige `bnd1_xila_honor` (solo el día 1, sobrio; el día 3 no se puede, hallazgo 1), no `bnd3_campanas` y no `bnd2_venides_sin_aviso`, y salta al cruzar la tarde. Si el jugador corre a avisar a Venides (`b3_avisar_venides`, O:1562) las campanas suenan antes y Xila ya no avisa; si calló el informe, tampoco. Solo avisa quien espera a la tarde sin avisar (`xila_avisa`, con «Esperar a la tarde»); sin Venides (`xila_sin_aviso`) no sale `b3_xila_avisa` (O:1973).
4. ⚠ **«No contarle nada» a Venides** (`bnd2_venides_sin_aviso`): quita sus hombres (`bnd_venides_10/20`, `b3_venides_arenga_solo`), la base sin Venides (`b3_venides_retenido`, campanas solas) y el aviso de Xila; el heroe sale solo. Es la decisión con más eco del día y no avisa de ello.
5. ⚠ **No ir a la fiesta cambia el principio del Ocaso**: la plaza sale vacía (`b3_plaza_vacia`, O:172: ni documentos, ni `bnd2_fiesta_nobles`, ni `conspiradores`) y no hay noble que buscar en el sermón. Con Gorran y la tregua no hay `bnd2_documentos`, así que solo la fiesta rellena la plaza.
6. ⚠ **Decisiones sin eco más allá de oro y moral**: `bnd2_noble_chantajeado` (+30, moral a la baja), `bnd2_venides_enganado` («Contarle el ataque, pero no lo de Edul y Verina»), `bnd2_tiene_aliados` (solo el diario) y las pistas `bnd2_pista_*` de la Barriada y los forjadores (88 avisos `flag-sin-lector` en total). En el informe hay opciones que dan lo mismo (`Contarselo todo` / `...y ensenarle las pruebas`).
7. ⚠ **Mentir a Rosita sobre la espada** (`Decirle que no la encontraste`): `bnd2_rosita_desconfia` quita a Rosita del Ocaso (`b3_taberna_cerrada` en vez de `b3_rosita_calle`) y, además, el informe ya no sale por `b2_noche_rosita_informa` sino por `b2_noche_papeles` (S:13862), cuyo «Prefiero contártelo yo» exige hablar otra vez con Venides (el arnés lo hace; un jugador que no, cierra el día sin informe).
8. ⚠ **Las ocultas que quedaban (A0) eran del arnés, no de la historia**: ver arriba.

## Fase B — Guardados y pruebas (10 caminos desde 2 finales de Gorran)

`./fabricar_guardados --matriz-dia3 guardados` (`dia3_<camino>.json`, un guardado del cierre del día 3 por camino y las ranuras de «Continuar»; la última que se corre manda). Prueba: `MG/tests/Engine/MatrizDia3Tests.cpp` (`test_matriz_dia3`): cada camino juega el día 1, Gorran y el día 3 de verdad con el arnés, fotografía el cierre, restaura en una sesión limpia, **empieza el Ocaso** (`ocasoBase`, o «Esperar a la tarde» en los de Xila) y comprueba flags, oro, moral, nivel, mochila, lectores del Ocaso (se ven o no) y los beats que el Ocaso cuenta. Mutación por camino (copia temporal del JSON, el original no se toca): todas en rojo; más una mutación de oro y otra de moral por camino.

| camino | desde | decisión del día 3 | consecuencia comprobada (Ocaso) | mutación |
|---|---|---|---|---|
| `fiesta_dentro` | tregua | entrar en la fiesta y señalar el noble a Venides | `bnd2_fiesta_nobles`, `b3_plaza` (no `b3_plaza_vacia`), buscar al noble del brindis, 10 hombres | `b2_sonsaca_exito` sin `bnd2_noble_senalado` |
| `sin_fiesta` | tregua | no subir al Barrio Alto | `b3_plaza_vacia`, sin noble | `b3_plaza_vacia` sin `bnd3_esc04_sermon` |
| `verina_arriba` | pelea (4 líos) + Verina salvada + proteger al borracho | devolver la espada y contarlo todo | `confianza_rosita=3`, `bnd2_rosita_verina` | `b2_rosita_verina_arriba` sin `bnd1_verina_salvada_guarida` |
| `espada_mentira` | pelea | «Decirle que no la encontraste» | `bnd2_rosita_desconfia`, `b3_taberna_cerrada`, sin `b3_rosita_calle` | sin `bnd2_rosita_desconfia` |
| `informe_nada` | tregua | «No contarle nada» | `b3_venides_retenido`, campanas solas, sin arenga | sin `bnd2_venides_sin_aviso` |
| `xila_avisa` | pelea + Xila sobrio (día 1) | informe dado, esperar a la tarde | `b3_xila_avisa` → `b3_aviso_en_base`, 20 hombres | informe que pone `bnd2_venides_sin_aviso` |
| `xila_sin_aviso` | pelea + Xila sobrio | callar el informe | no hay `b3_xila_avisa` | sin `bnd2_venides_sin_aviso` |
| `noble_chantaje` | tregua | chantajear al noble | +30 oro, moral 7, sin noble que buscar | sin `bnd2_noble_chantajeado` |
| `barriada_delata` | pelea | delatar la Barriada | sin `b3_zarela` ni `b3_huida_gitanos` | sin `bnd2_barriada_delatada` |
| `castillo_retirada` | tregua | retirarse ante el capitán | sin `b3_capitan_ronda` | sin `b2_castillo_prueba_cerrada` |

Quedan listadas sin guardado ni prueba: `b2_saq_llegan` (4 opciones), `b2_edul_fiesta`, `b2_cobrador`, `b2_luisarda_disfraz`, `b2_gitanos_*`, `b2_invitados` (sonsacar / robar), `b2_xila_resaca` (inalcanzable, hallazgo 1).

## Fase C — Capturas (`out/fabrica/UNI-02-11/capturas/m3_*.png`, 1280×720, OpenGL real, modo guion)

`./fabricar_guardados --capturas-dia3 guardados` y `./juego --guardado guardados/<m3_x>.json --guion ../../../tools/guiones/<m3_x>.txt` (`m3_fiesta.txt`, `m3_informe.txt`). Revisadas con la vista una a una: `m3_fiesta_noble` (la mansión, el noble que brinda: «Tres copas de mi cuenta (6 de oro)» / «Prefiero hacerle preguntas»), `m3_fiesta_preguntas` (de vuelta a la sala tras «Prefiero hacerle preguntas»), `m3_informe_menu` (Venides: «Siéntate. Hoy no bebo. Hoy cuentas tú.» con «Contarselo todo» / «No contarle nada»; **solo dos opciones** en este camino) y `m3_informe_callado` (tras «No contarle nada»: «Se levanta.», 2/3). `m3_fiesta_mansion` es la sala sin diálogo (el guardado). b2_fiesta_puerta (`enter` ciudad_este) no salta al cargar un guardado, por eso la captura de la fiesta es ya dentro. En el HUD del guardado de la tregua sale «Si traes la Furia de Alaro, devuélvesela a Rosita» (objetivo condicional, sin espada).

## Cómo reproducir
`python3 MotorGraphico/tools/matriz_decisiones.py --dia segundo_dia --top 10` · `ctest -L gl-free` (`test_matriz_dia3`) · `./fabricar_guardados --matriz-dia3 guardados`.

## E-DIA3-FIX — Xila, Verina y el aviso (D0025, 04/11)

1. **Xila con resaca** (`cinematicas.json`): `xila` se oculta con `bnd2_encargo` salvo `bnd_ocaso_empezado` **o `bnd1_xila_borracho`**; una segunda regla (O) lo oculta de nuevo con `bnd2_xila_resaca_visto`. Borracho el dia 1 se ve el dia 3 hasta que se le habla; `b2_xila_resaca_bien` pone `bnd1_xila_honor`, quita `bnd1_xila_borracho` y se vuelve a esconder. Camino `xila_resaca`: borracho (dia 1) -> desayuno (dia 3) -> avisa en el Ocaso (`bnd3_xila_avisa`, `b3_xila_avisa`, `b3_aviso_en_base`).
2. **Verina arriba**: `b2_rosita_verina_arriba` pide `confianza_rosita >= 2` (antes 3). Camino `verina_arriba_2` (Verina salvada + devolver y contarlo todo, sin proteger al borracho; el `verina_arriba` de confianza 3 sigue).
3. **Aviso de Xila** (`eventos/boundington.json`): sin `bnd3_campanas` en las condiciones del evento; dos disparadores en O sobre `ocaso_tarde`: sin campanas (el de siempre) o con `bnd3_avisado` (el jugador corrio a avisar a Venides: las campanas ya suenan). Se mantienen `bnd1_xila_honor` (el recuperado lo tiene), sin `bnd1_xila_borracho` y sin `bnd2_venides_sin_aviso` (si Venides salio solo, no esta en la base). Camino `xila_corre`.
   - Mínimo coherente: la escena `b3_xila_avisa` (ocaso.json, fuera de abasto) narra el aviso "antes de las campanas" y exige no `bnd3_arenga`; en `xila_corre` la arenga ya se hizo, asi que solo se pone `bnd3_xila_avisa` (el evento salta, la escena no se ve). Si Oriol quiere la escena tras avisar, hace falta un beat nuevo en el Ocaso.
   - El tiempo en el arnes de `xila_corre`: tras `ocasoBase` se pone `franja=1` a mano (el juego avanza la franja con las acciones).
- Pruebas: `MatrizDia3Tests.cpp` (3 caminos nuevos, 2 pruebas unitarias de visibilidad y del evento) y mutaciones: cinematicas.json de HEAD -> `xila_resaca` aborta (`seVe("xila")`) y la prueba de visibilidad falla; eventos de HEAD -> `xila_corre` y la prueba del evento fallan (`xila_resaca` sigue pasando: el recuperado ya tenia honor y no borracho); `min` 3 -> `verina_arriba_2` falla.
