# Auditoria de beats auto pegajosos (solo lectura)

Worktree `/Users/admin/onegai-ramas/UNI-02-11` @ `170178e8`, motor `MotorGraphico-main/MotorGraphico/`. Script: `/private/tmp/claude-501/-Users-admin-Documents-Documentos---Oriol-Os--2--Software/86912f52-7409-4be5-96ed-c6f982d913c3/scratchpad/auditoria_auto.py` (+ `genera_md.py`), ejecutados con `python3 -I`.

## 1. Que hace el motor (lo que se ha leido)

- `NarrativeEngine::disparaEn` (src/RPG/NarrativeEngine.cpp:36-130) recorre `beats` EN ORDEN del fichero y dispara el PRIMERO con el mismo `trigger.type` (y mismo target si no es auto), que no sea `once` ya disparado y cuyo `requires` encaje. Aplica efectos y devuelve: un disparo, un beat. `markBeatFired` se llama siempre; solo `once` lo consulta (y `Game/Codice.cpp`).
- Requisitos (`NarrativeCondition::isSatisfiedBy`, src/RPG/Narrative/NarrativeState.cpp:192): nivelMinimo/tierMinimo, moral, `lugar` (se ignora si el estado no tiene lugar), allFlags (AND), notFlags (NOR), anyFlags (OR), `variables` (rango [min,max], valor exacto si es un numero; 0 si nadie la toco), reputacion. En el JSON actual solo se usan allFlags, anyFlags, notFlags, lugar y variables.
- Efectos que cambian lo que miran los requisitos: setFlag, clearFlag, setVariable, addVariable (con clamp min/max), pasarFranja (variables `dia`/`franja`), skillCheck (enciende UNA de 4 flags, en el mismo applyNarrative), startBattle (flag de victoria/huida/derrota al cerrar el combate), tablaAleatoria (una flag de N). El resto (log, grantGold, grantLevel, acto, descanso...) no toca flags/variables.
- Opciones (`opciones`): obligatorias (la eleccion pendiente impide cerrar el dialogo), sus `effects` se aplican al elegir; JSON de opciones usa `requires`/`effects` (no `efectos`).
- Cuantas veces se dispara auto: `tick()` = una llamada a `fire(Auto)` = un solo beat, pero se llama desde varios sitios: `closeInteraction` al cerrar CADA dialogo (GameSessionInteraccion.cpp:207 en `closeInteraction`; tambien :243 y :455), `avanzaNarrativa` (cinematicas, GameSessionNarrativa.cpp:474), tras `tickTimers` si el reloj enciende una flag (:462) y tras cada evaluacion de eventos del mundo (EventosDeLaSesion.cpp:51). No hay mas mecanismo de «visto» que `once`/`beatFired`. Consecuencia: un auto pegajoso no solo tapa a los posteriores: al cerrar su dialogo, `closeInteraction` vuelve a llamar a `tick`, el mismo beat vuelve a encajar y reabre el dialogo: bucle sin salida (soft-lock) mientras se cumpla su `requires` y el jugador este en Exploration/Dialogue.
- Solo hay UNA aventura activa a la vez (prologo -> dia1 -> gorran -> dia2 -> ocaso, Application.cpp:405-420), pero el `NarrativeState` (flags) es compartido entre ficheros.
- JSON con `beats` que carga el motor: solo `assets/adventures/*.json` (10 ficheros; 5 se cargan en Application.cpp; silbido_medianoche, precuela_taberna, plaza_de_las_razas, este_norte_senal_ceniza y on_camino_del_norte no los carga la campana pero se auditan igual). `tests/fixtures/esquemas/aventura-v1.json` es un esquema de test, no una aventura. `assets/eventos/boundington.json` y `cinematicas.json` referencian beats por id pero no son beats.

## 2. Metodo

1. Se cargan los 10 JSON. Para cada beat `auto` sin `once` se construye un modelo abstracto del estado: flags (conjunto), variables (dict, 0 por defecto, `dia`/`franja` con la logica de FranjasDelDia).
2. Estados iniciales: los que cumplen su `requires` (allFlags encendidas; notFlags apagadas; para anyFlags, cada flag sola y todas a la vez; para variables, los extremos del rango y el 0 si cabe).
3. Se simula el disparo repetido (hasta K=25). Los efectos del beat son «azar» (skillCheck/startBattle/tabla = una rama por cada resultado posible; basta UNA rama que siga cumpliendo para contar como pegajoso). Las `opciones` son decision del jugador: el beat solo es pegajoso si TODA opcion disponible lo deja cumplido (si existe una salida que apaga el beat, el bucle lo corta el jugador: no es un bug del motor). Disponibilidad de opcion: se evalua con las flags/variables que el propio beat toca; las que no toca se suponen «no limitan».
4. Resultado: `pegajoso` (sigue cumpliendose tras 25 disparos), `limitado(n)` (se repite n veces y se apaga solo: paginas de charla), `una` (se apaga a la primera: lo normal).
5. Tapados: para cada pegajoso S y cada beat auto B posterior en el MISMO fichero: `compatible(S,B)` (sin flags contradictorias entre allFlags de uno y notFlags del otro, anyFlags satisfacibles, lugar con interseccion, variables con interseccion) y `implica(B,S)` (allFlags(S) subconjunto de allFlags(B), notFlags(S) subconjunto de notFlags(B), lugar(B) dentro de lugar(S), rangos de variables dentro). Clases:
   - **tapa seguro**: B implica S (cuando B encaja, S tambien y gana siempre).
   - **tapa de facto**: compatible, no implicado, S no tiene ningun «apagador» automatico alcanzable (nadie anterior a S en el fichero pone sus notFlags ni quita sus allFlags), B no necesita (cierre transitivo de prerrequisitos por flags) nada que S tenga en notFlags, y B necesita (directa o transitivamente) todo lo que enciende a S, es decir, solo puede ocurrir DESPUES de que S se encienda: B queda bloqueado para siempre.
   - **puede tapar**: compatible, pero o bien S tiene apagador automatico anterior a el, o bien B podria cumplirse ANTES de que S se encienda (ventana de orden), o el modelo no puede probar que B venga despues de S (flags puestas por cinematicas/eventos).
   - **no**: contradictorios, o B depende de una flag que apaga a S (cuando B pueda encajar, S ya esta apagado).
6. Criticidad: B es CRITICO si pone una flag que otros beats piden en allFlags, nadie mas la pone (ningun otro beat vivo ni mencion en eventos/cinematicas/otros assets) y B esta tapado. Se hace el cierre transitivo: lo que depende de flags perdidas tambien queda bloqueado.

## 3. Validacion con el Ocaso

Modelo con el estado actual (`once` en los 3): pegajosos del Ocaso = **`b3_dia_pasa`, `b3_xila_avisa`** (los tres b3_campanas, b3_campanas_solas, b3_norte_pasado NO aparecen). Pegajosos de todo el repo: 2.

Mutacion en memoria (quitando `once` a los 3): pegajosos del Ocaso = **`b3_dia_pasa`, `b3_campanas_solas`, `b3_campanas`, `b3_xila_avisa`, `b3_norte_pasado`**. Los tres se detectan.

Tapados en la mutacion (cuenta de beats posteriores clasificados): `b3_campanas_solas` -> 47 (puede tapar); `b3_campanas` -> 46 (puede tapar); `b3_norte_pasado` -> 84 (puede tapar).

Los beats que decia el commit 45e66555 (`b3_aviso_en_base`, las `b3_venides_arenga_*`) salen «puede tapar» bajo `b3_campanas` y `b3_campanas_solas` en la mutacion (no «tapa de facto»: sus requisitos dependen de `bnd3_esc04_*`, que pone `cinematicas.json`, y el modelo no puede derivar que vengan despues de `bnd3_esc04_campanas`). El metodo los detecta como tapados, con la clase mas prudente. `b3_afueras` NO sale bajo ninguno de los tres (discrepancia con el texto del commit): b3_norte_pasado pide notFlags `bnd3_en_afueras` y afueras la pide en allFlags (contradiccion), y b3_campanas/b3_campanas_solas tienen lugar `interior_cuartel` frente a `afueras_boundington_ocaso` (disjuntos). Tras el `once`, afueras sigue «puede tapar» por `b3_dia_pasa`.

Limite del modelo, honesto: no sale ningun «tapa seguro» (implicacion pura) en todo el repo, ni en la mutacion, porque los posteriores siempre dependen de flags extra (cinematicas/eventos) que el modelo no ve como implicacion.

## 4. Resultado: beats pegajosos en el repo (estado actual)

Solo **2** de los 728 beats auto sin `once` del repo son pegajosos, ambos en `boundington_ocaso.json`. Los otros 9 ficheros: 0.

| Fichero | Beat pegajoso:linea | Requisitos | Efectos | Tapa a | Clase | Critico | Arreglo minimo |
|---|---|---|---|---|---|---|---|
| boundington_ocaso | `b3_dia_pasa`:275 | all=['bnd3_campanas'] not=['bnd3_espera', 'bnd3_avisado'] | log:EV-1981-BOUNDINGTON. La Matanza ha empezado. | tapa de facto: `b3_campanas`, `b3_aviso_en_base`, `b3_venides_retenido`, `b3_venides_arenga_20`, `b3_venides_arenga_10`, `b3_venides_arenga_solo`, `b3_fanaticos_escolta`; puede tapar: 118 | permanente de facto | **SI** (soft-lock; criticos: `b3_venides_arenga_20`, `b3_venides_arenga_10`) | `"once": true` (como los 3 ya arreglados; no hay flag propia que apagar: solo hace log). |
| boundington_ocaso | `b3_xila_avisa`:1667 | all=['evt_xila_avisa_a_venides'] not=['bnd3_arenga'] lugar=['assets/levels/base_militar.json', 'assets/levels/interior_cuartel.json'] | (ninguno) | puede tapar: 32 | tiene apagador anterior (arengas) | **SI** (soft-lock; sin criticos probados (condicional)) | `"once": true` (el beat no tiene efectos; la guarda notFlags con flag propia exigiria una flag nueva). |

Detalle:

- **`b3_dia_pasa`** (boundington_ocaso.json:275): requiere `bnd3_campanas` y que no esten `bnd3_espera` ni `bnd3_avisado`; solo hace `log`. `bnd3_campanas` la enciende el evento `comienza_ocaso` (eventos/boundington.json) tambien por `puesta_sol` (cruzar dia 4 franja 2) sin que el jugador haya avisado ni esperado. Ruta: el jugador no avisa a Venides ni «espera» con Rosita y llega la puesta de sol -> `b3_dia_pasa` encaja, se lee, se cierra el dialogo, `closeInteraction` hace `tick` y vuelve a encajar. Sus apagadores son: `b3_aviso_en_base` (auto, `bnd3_avisado`; esta en la linea 1420, DESPUES de S, asi que lo tapa el propio S), `b3_avisar_venides` (talk, jugador, lin. ~1255) y `b3_rosita_ocaso` (talk, jugador): en bucle el jugador no puede alcanzar a nadie. Es el cuarto hermano de los tres del commit 45e66555 y esta ANTES de ellos en el fichero.
- **`b3_xila_avisa`** (boundington_ocaso.json:1667): requiere `evt_xila_avisa_a_venides` (la pone el evento del mundo `xila_avisa_a_venides` al cruzar `ocaso_tarde`) y `notFlags bnd3_arenga`, con lugar base_militar/interior_cuartel; NO tiene `effects`. Mientras el heroe este en la base/cuartel y ningun auto anterior encaje (b3_venides_retenido, b3_venides_arenga_20/10/solo, que son los que ponen `bnd3_arenga`, estan ANTES), se repite en bucle. Es un caso condicional (depende de que las arengas anteriores no encajen), pero el beat es tambien una escena de ese evento (escenas: `{"beat":"b3_xila_avisa"}`), asi que puede dispararse dos veces (por el evento y por el tick).

### CRITICOS: lo que se bloquea

**b3_dia_pasa**: 7 beats bloqueados para siempre (tapa seguro/de facto); 1 flags quedan sin otra fuente; 2 beats criticos (ponen flags que otros piden y nadie mas pone):

| Beat tapado:linea | clase | flags que solo pone el | beats que las piden (nº) |
|---|---|---|---|
| `b3_venides_arenga_20`:1486 | tapa de facto | bnd3_escolta | 1 |
| `b3_venides_arenga_10`:1527 | tapa de facto | bnd3_escolta | 1 |

**b3_xila_avisa**: 0 beats bloqueados para siempre (tapa seguro/de facto); 0 flags quedan sin otra fuente; 0 beats criticos (ponen flags que otros piden y nadie mas pone):

| Beat tapado:linea | clase | flags que solo pone el | beats que las piden (nº) |
|---|---|---|---|
(ninguno: todos los demas son solo «puede tapar»)


Lectura: con `b3_dia_pasa` encendido el soft-lock es total (el dialogo se reabre sin parar), asi que en la practica NADA posterior avanza: ni las arengas, ni saqueadores/oleadas, ni Verina/Sandes/Edul, ni los epilogos `b3_epi_*` ni el final (`b3_barca` -> `bnd3_epilogo`). La tabla de arriba es la parte que el modelo puede PROBAR por flags; el resto (118 beats «puede tapar», lista abajo) queda bloqueado por el bucle, no por la regla de orden.

### Lista completa de beats tapados

**b3_dia_pasa** (125):
`b3_reloj`, `b3_sermon_cripta`, `b3_sermon_po54`, `b3_sermon`, `b3_sermon_s0_a`, `b3_sermon_s1_a`, `b3_sermon_s2_a`, `b3_sermon_s3_a`, `b3_sermon_p0_a`, `b3_campanas`, `b3_aviso_en_base`, `b3_venides_retenido`, `b3_venides_arenga_20`, `b3_venides_arenga_10`, `b3_venides_arenga_solo`, `b3_capitan_ronda`, `b3_aigren_carro`, `b3_xila_avisa`, `b3_xila_vendas`, `b3_xila_fiesta`, `b3_por_donde_solo`, `b3_por_donde`, `b3_esc04_camino_oeste`, `b3_esc04_camino_norte`, `b3_esc04_camino_quedarse`, `b3_saqueadores_20`, `b3_saqueadores_10`, `b3_saqueadores_guardia`, `b3_saqueadores`, `b3_fanaticos_escolta`, `b3_fanaticos`, `b3_oleada`, `b3_base_cae`, `b3_norte`, `b3_norte_pasado`, `b3_verina_tarde`, `b3_verina_cautiva_escolta`, `b3_verina_cautiva`, `b3_verina_reconocida`, `b3_verina_no_reconocida`, `b3_esc05_verina_libre`, `b3_verina_salvada_dudas`, `b3_verina_salvada`, `b3_verina_salvada_sola`, `b3_verina_perdida`, `b3_esc05_rosita_pasada`, `b3_rosita_calle`, `b3_taberna_cerrada`, `b3_pulga`, `b3_casco`, `b3_rev_viga`, `b3_rev_casa`, `b3_rev_teobaldo`, `b3_sandes_venides`, `b3_sandes_xila`, `b3_sandes_verina`, `b3_sandes_oferta`, `b3_verina_segunda`, `b3_esc05_iglesia`, `b3_sandes_huye_casco`, `b3_sandes_gana_casco`, `b3_afueras`, `b3_candela`, `b3_orfeu`, `b3_cojo_cajas`, `b3_barricada_floja`, `b3_barricada_gris`, `b3_morvan`, `b3_garsa`, `b3_cojo_afueras`, `b3_amigos_afueras`, `b3_zarela`, `b3_gorran`, `b3_gorran_sin_jefe`, `b3_bandidos_vencidos`, `b3_bandidos_otros`, `b3_barricada_vista`, `b3_barricada_zarela`, `b3_rodeo`, `b3_rodeo_hecho`, `b3_barricada_rota`, `b3_bastian`, `b3_rodeo_caido`, `b3_edul_furia_casco`, `b3_edul_casco`, `b3_edul_furia`, `b3_edul`, `b3_edul_ofrece_compania`, `b3_edul_con_verina`, `b3_edul_fiesta`, `b3_edul_sin_verina`, `b3_edul_perdido`, `b3_edul_vencido`, `b3_venides_rodillas`, `b3_venides_libre`, `b3_esc06_pacto_salida`, `b3_final_venides`, `b3_final`, `b3_compania`, `b3_compania_de_los_tuyos`, `b3_rev_compania_cubre`, `b3_sandes_vencido`, `b3_sandes_sigue`, `b3_esc06_afueras_listas`, `b3_huida_gitanos`, `b3_huida_sola`, `b3_aliado_fuera`, `b3_caida`, `b3_rev_hito`, `b3_epilogo_caido`, `b3_epilogo`, `b3_epi_venides_lejos`, `b3_epi_venides_contigo`, `b3_epi_rosita`, `b3_epi_verina_viva`, `b3_epi_verina_sola`, `b3_epi_verina_muerta`, `b3_epi_xila`, `b3_epi_sandes`, `b3_epi_compania`, `b3_epi_solo`, `b3_epi_iris`, `b3_barca`, `b3_barca_s0_a`, `b3_barca_s1_a`

**b3_xila_avisa** (32):
`b3_norte_pasado`, `b3_candela`, `b3_orfeu`, `b3_cojo_cajas`, `b3_barricada_floja`, `b3_barricada_gris`, `b3_morvan`, `b3_garsa`, `b3_cojo_afueras`, `b3_amigos_afueras`, `b3_gorran`, `b3_gorran_sin_jefe`, `b3_bandidos_vencidos`, `b3_bandidos_otros`, `b3_bastian`, `b3_caida`, `b3_rev_hito`, `b3_epilogo_caido`, `b3_epilogo`, `b3_epi_venides_lejos`, `b3_epi_venides_contigo`, `b3_epi_rosita`, `b3_epi_verina_viva`, `b3_epi_verina_sola`, `b3_epi_verina_muerta`, `b3_epi_xila`, `b3_epi_sandes`, `b3_epi_solo`, `b3_epi_iris`, `b3_barca`, `b3_barca_s0_a`, `b3_barca_s1_a`

## 5. Pegajosos en triggers talk / enter (fuera de alcance, informativo)

60 beats talk/enter sin `once` no se apagan solos: es lo normal (respuesta repetible de un NPC al final de su lista) y NO tapan nada a menos que un beat posterior del mismo target tenga requisitos que impliquen los suyos. Solo hay 1 caso:

- `b1_sectarios_despues`:8332 (talk `sectario_perdido`, requiere solo `bnd1_sectarios_resueltos`, sin efectos) TAPA SEGURO a `b1_sectarios_seguir`:19556 (mismo target, requisitos mas estrictos que implican los suyos). `b1_sectarios_seguir` es la persecucion con [Sigilo] que pone `bnd_pista_sectarios` (consumida mas abajo, lin. ~28998) y nunca se puede disparar. Arreglo minimo: mover `b1_sectarios_seguir` ANTES de `b1_sectarios_despues` en el JSON (viene de `tools/parche_c_trama.py:205`, que lo anade al final), o poner en `despues` `notFlags: [bnd1_sectarios_seguir_visto, bnd_pista_sectarios]`. (`once` no vale: `despues` es la respuesta repetible.)

Beats auto `limitado(n)` (paginas de charla con `sigue`, se repiten n veces dentro de su dialogo y se apagan al elegir salida): 0, distribuidos {}. No son un problema.

## 6. Falsos positivos / limites posibles

- El modelo es por flags/variables; no ve efectos de `cinematicas.json` (`flag`/`beat` steps), `eventos/boundington.json` (efectos del evento, banderas `evt_*`), `Companeros`, cofres, refuerzos ni el sistema de timers. Una flag que solo esas fuentes apaguen puede hacer que un «pegajoso» se apague en juego (falso positivo) o que un auto parezca no pegajoso cuando otro agente lo enciende de nuevo (falso negativo). Para b3_dia_pasa: `bnd3_avisado` y `bnd3_campanas` aparecen en eventos/boundington.json (como `senal` y condicion; no como efecto que las apague) y `bnd3_campanas` tambien en cinematicas.json.
- «Tapa de facto» es una estimacion: asume que S esta encendido cuando B querria dispararse. Si B se dispara ANTES de que S se encienda (ventana de orden de eventos) se ve. Las listas de «puede tapar» son un MAXIMO (incluyen beats que ocurren antes de que S se encienda o en rutas que no coexisten con «no avisar y no esperar»); lo fiable en gravedad es el soft-lock del propio pegajoso.
- Opciones: la disponibilidad solo mira flags que el propio beat toca; una opcion que dependa de una flag ajena se supone disponible. Si en juego esa opcion nunca esta disponible y era la unica salida, habria pegajosos que el modelo da por salvados (falso negativo).
- Rutas inalcanzables: no se comprueba que `requires` de S sea alcanzable (p. ej. si `evt_xila_avisa_a_venides` nunca se enciende, `b3_xila_avisa` no molesta).
- `lugar` se trata como interseccion de conjuntos; el motor lo ignora si el estado no tiene lugar (pruebas sin niveles).
- No se audita el orden entre S y beats auto de OTRO fichero (solo hay una aventura activa a la vez).
- No se ha buscado el patron «re-armable» (auto que se apaga con una flag que otro beat limpia con clearFlag y vuelve a disparar): fuera del alcance pedido.
