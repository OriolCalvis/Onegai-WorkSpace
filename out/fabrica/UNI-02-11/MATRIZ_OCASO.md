# MATRIZ_DIA — ocaso

Generado por `MotorGraphico/tools/matriz_decisiones.py --dia ocaso` (no editar a mano: se regenera). 
Decisiones importantes: **8** beats con elecciones que leen despues del dia (de 31 beats con elecciones). Las `top 8` por peso llevan guardado y prueba (fase B).

Peso = beats/eventos/cinematicas posteriores distintos que leen (+0,5 por los de un salto) + moral (menor 2, medio 5, mayor 12) + |oro|/10 + 3 si da nivel.

## Decisiones importantes (por peso)

| # | beat:linea | peso | opcion (texto:linea) | pone | se lee despues en | consecuencia |
|---|---|---|---|---|---|---|
| 1 | b3_barricada_rota:4926 | 27.0 | Derribarla a golpes:4953 | bnd3_barricada_resuelta; acto:atrapados_rescatados(medio+); oro+40 | (moral/oro/nivel) | moral atrapados_rescatados rep {"aegroum": 5} |
|  |  |  | Romperla a distancia:4983 | bnd3_barricada_resuelta; acto:atrapados_rescatados(medio+); oro+40 | (moral/oro/nivel) | moral atrapados_rescatados rep {"aegroum": 5} |
|  |  |  | Buscar dónde no hay frascos:5008 | bnd3_barricada_resuelta; acto:atrapados_rescatados(medio+); oro+40 | (moral/oro/nivel) | moral atrapados_rescatados rep {"aegroum": 5} |
| 2 | b3_sandes_verina:3816 | 25.0 | Entregársela:3844 | bnd3_verina_entregada; bnd3_aliado_sandes; bnd3_venides_huye; bnd3_casco_resuelto; acto:verina_entregada(mayor-); oro+80; quita bnd3_verina_con_grupo, bnd3_con_venides | (moral/oro/nivel) | moral verina_entregada rep {"perdidos": 40} |
|  |  |  | No.:3892 | bnd3_sandes_pelea; acto:sandes_enfrentado(medio+); combate sandes x1+perdido_devoto x2 [victory>bnd3_sandes_huye_casco, defeat>bnd3_sandes_gana_casco] | (moral/oro/nivel) | moral sandes_enfrentado rep {"perdidos": -40, "aegroum": 15} |
| 3 | b3_verina_salvada_dudas:3115 (y b3_verina_salvada:3211, b3_verina_salvada_sola:3305) | 24.0 | Vamos a por el.:3163 | bnd3_verina_con_grupo; bnd3_promesa_edul; acto:verina_liberada(mayor+) | (moral/oro/nivel) | moral verina_liberada rep {"perdidos": -15, "aegroum": 5} |
|  |  |  | No podemos. No hay tiempo.:3188 | bnd3_verina_sola; acto:verina_liberada(mayor+) | (moral/oro/nivel) | moral verina_liberada rep {"perdidos": -15, "aegroum": 5} |
| 4 | b3_venides_rodillas:5760 | 17.0 | Intentar acabar con él:5817 | bnd3_venides_huye; acto:venides_atacado(mayor-) | (moral/oro/nivel) | moral venides_atacado rep {"aegroum": -40} |
|  |  |  | Dejarle:5833 | bnd3_venides_dejado; acto:venides_abandonado(medio-) | (moral/oro/nivel) | moral venides_abandonado |
| 5 | b3_sandes_oferta:3941 | 13.0 | Acepto.:3964 | bnd3_sandes_decidido; bnd3_aliado_sandes; bnd3_venides_huye; bnd3_casco_resuelto; acto:aliarse_con_sandes(menor0); oro+80; quita bnd3_con_venides | (moral/oro/nivel) |  |
|  |  |  | No.:4025 | bnd3_sandes_decidido; bnd3_sandes_pelea; acto:sandes_enfrentado(medio+); combate sandes x1+perdido_devoto x2 [victory>bnd3_sandes_huye_casco, defeat>bnd3_sandes_gana_casco] | (moral/oro/nivel) | moral sandes_enfrentado rep {"perdidos": -40, "aegroum": 15} |
| 6 | b3_edul_fiesta:5611 | 12.0 | Es verdad.:5636 | bnd3_edul_cerrado; bnd3_edul_salvado; bnd3_edul_con_grupo; acto:edul_liberado(mayor+) | (moral/oro/nivel) | moral edul_liberado |
| 7 | b3_verina_cautiva:2807 (y b3_verina_reconocida:2912, b3_verina_no_reconocida:3001) | 5.0 | Seguir tu camino:2890 | bnd3_verina_cerrada; bnd3_verina_abandonada; acto:verina_abandonada(medio-) | (moral/oro/nivel) | moral verina_abandonada |
| 8 | b3_sermon_po54:396 (y b3_sermon:529, b3_sermon_s0_a:656, b3_sermon_s1_a:779, b3_sermon_s2_a:895, b3_sermon_s3_a_miraba:1012, b3_sermon_s3_a_desafiado:1137, b3_sermon_s3_a:1258, b3_sermon_p0_a:1378) | 0.5 | Correr a avisar a Venides:500 | bnd3_a_la_base | (via b3_aviso_en_base) bnd3_avisado: eventos/boundington.json:13 (comienza_ocaso) | evento comienza_ocaso |

## Avisos automaticos (marcar ⚠ tras revisar a mano)

- `flag-sin-lector` b3_sermon_po54:428 — bnd3_pista_campanas (Escuchar entre la gente)
- `flag-sin-lector` b3_sermon:556 — bnd3_pista_campanas (Escuchar entre la gente)
- `flag-sin-lector` b3_sermon_s0_a:678 — bnd3_pista_campanas (Escuchar entre la gente)
- `flag-sin-lector` b3_sermon_p0_a:1402 — bnd3_pista_campanas (Escuchar entre la gente)
- `flag-sin-lector` b3_rosita_ocaso:1639 — bnd3_sin_aviso_hoy (Esperar a que anochezca)
- `flag-sin-lector` b3_verina_salvada_dudas:3163 — bnd3_promesa_edul (Vamos a por el.)
- `flag-sin-lector` b3_verina_salvada_dudas:3188 — bnd3_verina_sola (No podemos. No hay tiempo.)
- `flag-sin-lector` b3_verina_salvada:3257 — bnd3_promesa_edul (Vamos a por el.)
- `flag-sin-lector` b3_verina_salvada:3282 — bnd3_verina_sola (No podemos. No hay tiempo.)
- `flag-sin-lector` b3_verina_salvada_sola:3348 — bnd3_promesa_edul (Vamos a por el.)
- `flag-sin-lector` b3_verina_salvada_sola:3373 — bnd3_verina_sola (No podemos. No hay tiempo.)
- `flag-sin-lector` b3_pulga:3583 — bnd3_esc05_pulga_guia (Seguir a Pulga)
- `flag-sin-lector` b3_sandes_verina:3892 — bnd3_sandes_pelea (No.)
- `flag-sin-lector` b3_sandes_oferta:4007 — bnd3_sandes_gana_pregunta (¿Y tú qué ganas?)
- `flag-sin-lector` b3_sandes_oferta:4025 — bnd3_sandes_pelea (No.)
- `flag-sin-lector` b3_edul_casco:5185 — bnd3_edul_pelea (Liberarle)
- `flag-sin-lector` b3_edul_killville:5352 — bnd3_edul_pelea (Liberarle)
- `flag-sin-lector` b3_edul_killville:5388 — bnd3_edul_abandonado (Dejarle)
- `flag-sin-lector` b3_edul:5438 — bnd3_edul_pelea (Liberarle)
- `flag-sin-lector` b3_edul:5474 — bnd3_edul_abandonado (Dejarle)
- `flag-sin-lector` b3_edul_ofrece_compania:5524 — bnd3_con_edul (Ven con nosotros)
- `flag-sin-lector` b3_edul_ofrece_compania:5538 — bnd3_edul_a_salvo (Busca un lugar seguro)
- `flag-sin-lector` b3_edul_fiesta:5636 — bnd3_edul_con_grupo (Es verdad.)
- `flag-sin-lector` b3_venides_rodillas:5789 — bnd3_venides_perdido (Ayudarle a escapar)
- `flag-sin-lector` b3_venides_rodillas:5833 — bnd3_venides_dejado (Dejarle)
- `flag-sin-lector` b3_barca_s1_a:7271 — bnd3_asentado (¿Lo que hice ha cambiado algo?)
- `opciones-iguales` b3_barricada_rota:4953 — Derribarla a golpes | Romperla a distancia | Buscar dónde no hay frascos (bnd3_barricada_resuelta)
- `lectura-sin-escritor` assets/eventos/boundington.json:11 — bnd3_campanas 
- `lectura-sin-escritor` assets/eventos/boundington.json:14 — bnd3_xila_avisa 

## Qué es «importante» en el último día
Aquí no hay día siguiente: importa lo que cambia **el final** (`bnd3_final_oeste` / `_solos` / `_caido`), el **epílogo** (`b3_epi_*`: Venides contigo/lejos, Rosita, Verina viva/sola/muerta, Xila, Sandes, compañía, solo), **quién sobrevive** (Verina, Edul, Venides, Xila, Sandes), el **oro / moral / nivel finales** y el **diario**. El script solo ve lo que se lee *después* en el mismo día (8 decisiones); las pruebas de la fase B miden lo demás jugando.

## Decisiones y finales del Ocaso (comprobados jugando, fase B)
Finales (flag, `b3_*`): **oeste con los gitanos** (`b3_huida_gitanos`: `bnd2_aliado_gitanos` y no `bnd2_barriada_delatada`, +50 oro), **solos** (`b3_huida_sola`, mapa de la región; `b3_aliado_fuera` «con los Perdidos» está tapado, ⚠ 1) y **caído** (`b3_caida`: perder uno de los combates de la salida). Rutas de salida (zona de `ciudad_oeste`): oeste, norte (pelea en las calles; se salta Verina y Sandes) y quedarse (oleada, la base cae, `b3_base_cae` pone `bnd3_ruta_oeste`).

## Fase B — Un guardado y una prueba por camino (9 caminos, máx. 10)
`./fabricar_guardados --matriz-ocaso <carpeta>` → `guardados_ocaso/ocaso_<camino>.json` (al acabar el epílogo, en `huida_boundington`) y las ranuras. Prueba: `MG/tests/Engine/MatrizOcasoTests.cpp` (`test_matriz_ocaso`, 5 casos): cada camino juega días 1, Gorran, 3 y Ocaso de verdad con el arnés (que falla con cualquier personaje oculto), restaura en sesión limpia y comprueba el final (y solo ese), quién vive, oro/moral/nivel/mochila y el **diario** (`Game::construirDiario`). Mutación **con log** por camino: `logs_ocaso/<camino>_mut.log` (original verde / mutada ROJA con `[FAIL]` / restaurada verde; la mutación es una copia temporal cargada por la prueba, el JSON no se toca: md5 antes y después en el log).

| camino | desde (día 3) | decisión / final | quién vive | oro·moral·nivel | mutación (en rojo) |
|---|---|---|---|---|---|
| `oeste_todos` | `xila_corre` | corre a avisar a Venides; Xila avisa igual; oeste con gitanos | Venides, Xila, Verina, Edul | 510·42·8 | `b3_edul_con_verina` sin `bnd3_edul_salvado` |
| `oeste_xila_avisa` | `xila_avisa` (espera la tarde) | `b3_xila_avisa` SÍ se ve, luego campanas y arenga | ídem | 510·42·8 | `b3_xila_avisa` sin su `evt_` |
| `oeste_xila_sin_aviso` | `xila_sin_aviso` | campanas solas, sin aviso de Xila ni Venides | Xila, Edul (Verina y Venides no) | 460·27·8 | `b3_xila_vendas` sin `bnd3_con_xila` |
| `oeste_sin_venides` | `informe_nada` | Venides sin aviso, sin Xila | Edul solo | 280·22·7 | `b3_venides_rodillas`/«Dejarle» sin `bnd3_venides_dejado` |
| `solos_delatada` | `barriada_delata` | final solos (`b3_huida_sola`) | Venides, Verina, Edul | 420·36·7 | `b3_huida_sola` sin `bnd3_final_solos` |
| `sandes_aliado` | `fiesta_dentro`, quedarse + «Acepto.» | aliado de Sandes; sale por los gitanos | nadie de los tuyos (Verina muere esperando) | 340·3·7 | «Acepto.» sin `bnd3_aliado_sandes` |
| `oeste_norte` | `fiesta_dentro`, norte | final oeste sin pasar por la ciudad | Venides, Edul | 330·25·7 | `b3_esc04_camino_norte` sin su flag |
| `caido` | `fiesta_dentro`, pierde saqueadores | caído | Venides (que sale solo) | 170·11·5 | `b3_caida` sin `bnd3_final_caido` |
| `sacrificios` | `informe_nada`: Verina sí, Edul no, ataca a Venides | oeste | Verina | 280·13·7 | `b3_edul`/«Dejarle» sin `bnd3_edul_abandonado` |

**Aviso de Xila sí / no / tras correr**: `oeste_xila_avisa` (se ve la escena `b3_xila_avisa`) frente a `oeste_xila_sin_aviso` (no hay) y `oeste_todos` (`bnd3_xila_avisa` puesta pero **no** se ve la escena).
**No cubierto**: la compañía de Rosita (`b3_compania`: exige Verina+Edul+Xila+Venides, `bnd2_rosita_semilla`, `confianza_rosita>=2` y perder el combate final; necesita un día 1 con Aigren-Verina **y** Xila sobrio que ninguna ruta actual junta), derrota en `b3_fanaticos`/`b3_oleada`/`b3_norte` y «Entregársela» a Sandes.

## Hallazgos ⚠
1. ⚠ **`b3_aliado_fuera` («FINAL: con los Perdidos») nunca salta**: va tras `b3_huida_gitanos` y `b3_huida_sola` (posiciones 122, 123, 124), ninguna excluye `bnd3_aliado_sandes`, y gana el primero que coincide. Con Sandes aliado se sale «con los gitanos» o «sin ayuda» (`sandes_aliado` sale por gitanos). Además su texto promete «cincuenta monedas» y no hay `grantGold`. Fijado por `hallazgo_el_final_con_los_perdidos_queda_tapado…`. No se toca la historia.
2. ⚠ **El diario da por hecho «Sal de Boundington con vida» si caes** (`obj3_salir`, `doneFlag bnd3_fuera`, que `b3_caida` también pone). Fijado por `hallazgo_el_diario_da_por_hecha_la_salida…`.
3. ⚠ **El diario no registra el final** (oeste / solos / caído / compañía) en ninguna decisión; solo ruta, Verina, Sandes, Edul y Venides. Tampoco Verina muerta por quedarte esperando (`bnd3_verina_perdida_tarde`) ni Edul vencido por la furia de Sandes.
4. ⚠ **Quedarse en la base se ve en el diario como «Fuiste hacia el oeste»** (`b3_base_cae` pone `bnd3_ruta_oeste`, y la ficha muestra la primera ruta que coincide).
5. ⚠ **`b3_xila_avisa` no se ve en `xila_corre`** (conflicto que dejó E-DIA3-FIX): su requires incluye `notFlags bnd3_arenga` y dice «hasta las campanas»; si corres a avisar, la arenga ya se hizo. `bnd3_xila_avisa` sí queda puesta. Es hallazgo, no se toca.
6. ⚠ Con **Venides sin aviso** `obj3_avisar` queda sin hacer para siempre, aunque `obj3_salir` se cumpla (diario 2/3).
7. ⚠ En todos los caminos que se juegan por defecto, «No.» a Sandes gana siempre (`bnd3_sandes_huye_casco`): el héroe con suerte buena nunca pierde esa pelea; no hay camino en la tabla que la pierda.
8. ⚠ 29 avisos automáticos de abajo: 24 `flag-sin-lector` (decisiones que solo leen la ficha del diario o el epílogo por otra flag), `opciones-iguales` en `b3_barricada_rota` (tres opciones dan lo mismo: `bnd3_barricada_resuelta`, +40 oro y el mismo acto), `lectura-sin-escritor` de `bnd3_campanas` y `bnd3_xila_avisa` (las pone un evento, `eventos/boundington.json`, no una opción).

## Cómo reproducir
`python3 MotorGraphico/tools/matriz_decisiones.py --dia ocaso --top 8` · `./fabricar_guardados --matriz-ocaso <carpeta>` · `ctest -L gl-free` (`test_matriz_ocaso`) · mutación de un camino: `MATRIZ_SOLO=<id> MATRIZ_MUTAR=1 ./test_matriz_ocaso`.
