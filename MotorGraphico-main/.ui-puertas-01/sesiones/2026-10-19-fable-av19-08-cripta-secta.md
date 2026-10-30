# 19/10/2026 — Fable — AV19-08: la cripta de los Perdidos en cinco mapas (trama y cinemáticas)

Encargo de Oriol (19/10): el Excel `boundington_secta_iglesia_5_mapas.xlsx` (iglesia, osario, dependencias, santuario, cámara) y, encima, «quan entrem al sòtan de l'església i ens trobem a Edul i Verina, la cinemàtica ha de saltar quan entrem a la sala d'estudi dels novicis; ampliar la història: combats, engany o tirades per infiltrar-se, o que t'hagin convidat; la cinemàtica només fora de combat; i després Edul sempre s'ha d'oferir a acompanyar-te a salvo».

## Mapas (tools/importar_secta.py)
- Tres niveles nuevos leídos del Excel tal cual: `dependencias_secta` 32×32, `santuario_subterraneo` 30×30, `camara_profunda` 28×28. El suelo por color (roca, losa oscura/clara, piedra antigua, losas cálidas, alfombra, altar, agua somera), los objetos por las etiquetas de Oriol (copista, archivero, guardias, ritual, pozo, cofres).
- Pasos: osario (1,8) y galería (11,8) de `interior_iglesia_s1` → dependencias (1,27) y (4,31); conducto (24,26) → santuario (24,24); (27,4) → cámara (14,27). Manifiesto y `lugares.json` al día. PNJ horneados.

## Trama (tools/parche_secta.py, idempotente) — `boundington_primer_dia.json`, +25 beats
Cómo se baja (la trampilla de `b1_guarida_puerta` sigue igual: dejar las armas / «vengo de parte del herrero» / darte la vuelta):
- **Invitado o sin armas** (`bnd1_guarida_dentro`): al entrar en las dependencias, un novicio te manda a esperar a la sala de estudio (`b1_dep_llegada_dentro`).
- **Colarse** (sin `bnd1_guarida_dentro`, `b1_dep_llegada_colarse`): Sigilo (éxito: pasas; parcial: un novicio te para → Engaño o taparle la boca; pifia: pelea), Engaño como novicio (éxito: túnica y candil; parcial: pasas pero «Sandes querrá verte»; pifia: pelea), a las bravas (pelea: 2 devotos + 1 carcelero) o volver arriba. Quien llega al santuario sin permiso (`b1_santuario_intruso`): Engaño o pelea.
- **Hostil** (`bnd1_cripta_hostil`): los enemigos de la cripta (`perdido_devoto`, `cultista_carcelero`) solo se ven con esa flag (o el día 3, `bnd2_dia_cerrado`); el santuario queda vacío de canto (`b1_santuario_hostil`); en la cámara, Sandes ya no está. Derrota y huida: `b1_guarida_perdida` (ya existía).
- **La sala de estudio** (cinemática `cripta_estudio`, zona losa clara 8..14 × 15..21 de las dependencias, nunca en combate ni tras `bnd1_guarida_pelea`): Verina lee en el banco, Edul entra por el pasillo con su frasco → `b1_estudio` → `bnd1_con_edul` → **`b1_edul` (el de siempre: salir con él / unirte / plantarles cara / sembrar dudas)**. Si Edul ya te habló (viniste por la ceremonia), `b1_estudio_vuelta`.
- **Edul siempre se ofrece**: «Salir con él» sale por la galería del osario a la puerta de la iglesia (cinemática `edul_escolta`, o `edul_escolta_fuera` desde otro mapa de la cripta, a (56,53) de `ciudad_centro`). Si eliges otra cosa, lo dice (`b1_edul_oferta`) y cada vez que le hablas vuelve a ofrecerse (`b1_edul_escolta`, trigger talk `edul_cripta`).
- **La ceremonia** (`b1_guarida_ceremonia` y su árbol `b1_ceremonia_*`, `b1_charla_*`, `b1_frascos`) ahora salta al entrar en `santuario_subterraneo` (invitado o colado); el texto de llegada va por el conducto.
- **La cámara profunda**: Sandes con un sereno encadenado (`b1_camara_sandes`: «me he perdido» / «¿quién es?» / soltarlo → 2 carceleros, Sandes huye «lo tengo todo previsto»). El sereno (`b1_sereno`, talk) da la pista de las barricadas (`bnd1_sabe_barricadas`, +1 pista) y, si la cripta es hostil, se le puede soltar (`bnd1_sereno_libre`).
- Niveles concedidos: colarse 0.2, pasar por novicio 0.2, abrirte paso 0.3, plantar cara a Sandes 0.3, liberar al sereno 0.3. Ningún oro nuevo (el ledger de recompensas no cambia).

## Gente
- `edul_cripta` (10,19) y `verina_cripta` (12,20) en la sala de estudio; `sandes_cripta` (14,8) y `prisionero_cripta` (21,8) en la cámara (sustituyen a los `edul`/`verina_joven` provisionales del Excel). Composiciones en `tools/actores_pnj.py` (capucha de la cripta; el sereno con su casaca azul). Apariciones: se esconden al pelear, al salir escoltado, al salvar a Verina, al liberar al sereno y al día siguiente.

## Validación
`validar_todo` 0 errores (22 avisos viejos), `validar_cinematicas` 20/0, `analisis_trama` sin huecos nuevos (flags-gancho sin lector: `bnd1_cripta_sospecha`, `bnd1_cripta_tunica`, `bnd1_sabe_sereno`, `bnd1_dep_novicio_aviso` — para el día 2/3 si Oriol quiere que Sandes te reconozca o que la torre sepa lo del sereno).

## Pendiente / ganchos
- Que `bnd1_sereno_libre` cuente en la torre (día 2) y `bnd1_cripta_sospecha` en el sermón de Sandes (día 3): sin escribir.
- Cuando hay pelea en la cripta, la guardia de arriba no se entera (no se pone `bnd1_guardias_avisados`): decisión de Oriol.
- Duplicados « 2.*» de Finder en `src/`, `include/`, `cmake/`, `tests/` movidos a `_to_delete/finder_dupes_19-10/` (no estaban en git).
