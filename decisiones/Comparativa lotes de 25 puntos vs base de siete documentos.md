# Comparativa de los tres conceptos para Boundington

29/10/2026 · @Oriol

Revisión editorial de Codex, 29/10: correcciones sobre B, instrucciones de Oriol y límites del inventario. Las afirmaciones sobre el documento C conservan atribución a la comparativa original: su texto fuente no se ha localizado en esta revisión. Véase «Alcance de la revisión» antes de la bibliografía.

## Resumen

Son tres estrategias distintas sobre el mismo encargo. **A** (los cuatro lotes de 25 puntos, Antigravity) produce primero y decide después: 45 misiones, 58 PNJ cruzados, 4 tiendas y una checklist final. **B** (la base de siete documentos en `Planificacion_Boundington`, commit `7ddbbc8`) fija el terreno primero: inventario de la rama H, cronología, ficha de 30 apartados, cartera sin doble cómputo y dos pilotos antes de escalar. **C** (`claude/plan_contenido_boundington_29-10.md`, Fable) está entre los dos: inventario medido de la rama av19, una ficha de 25 campos atada a `quests.json`, `actos.json` y `facciones.json`, una misión escrita entera, la familia como cadena de siete eslabones y una cartera por regiones; no escribe las 45 hasta cerrar dos decisiones.

Veredicto: el esqueleto sale de B y C, que coinciden en el orden (decisiones, inventario, pilotos, lotes) y se complementan: B tiene las reglas de cronología, consecuencias y doble cómputo; C tiene la ficha atada al motor, la familia como cadena y el orden de trabajo con agentes y jornadas. A es la cantera. Nada se implementa aún: los tres asignan distinto las puertas sur, A, B y C proponen tres composiciones familiares distintas, B y C miden el juego en dos ramas diferentes, y A cuenta como secundarias escenas de la trama principal.

Fuentes: los cuatro lotes adjuntos (Plan Maestro no incluido), `00_LEEME` a `06_ESTANDAR_Y_PROMPT.md` de la carpeta compartida, `claude/plan_contenido_boundington_29-10.md` del proyecto y `ciudad_centro.xlsx` del proyecto.

## Comparación por ejes

Los tres recogen la secuencia general del calendario y en que las tiendas repetidas pasan a minería, música, mascotas y mensajería; divergen en casi todo lo demás.

| Eje | A · Lotes de 25 puntos | B · Base de siete documentos | C · Plan de contenido |
| --- | --- | --- | --- |
| Estrategia | Escribirlo todo ya; la decisión llega con la checklist final | Reglas e inventario; dos pilotos antes de escalar | Dos decisiones, inventario, una ficha de ejemplo; el resto después |
| Volumen entregado | 45 entradas (3 fichas numeradas completas; contenido por verificar), 4 tiendas, 58 PNJ cruzados | 0 fichas completas; cartera de 30 + 15 reservas | 1 misión completa, cadena familiar de 7 diseñada, cartera 30 + 15 por regiones |
| Anclaje al juego | Coordenadas «verificadas» en `.tmx` y `.json`, sin rama citada | Commit `102dc36`, rama integración H: 81 niveles, 654 filas | Rama `agente/qwen/integracion-av19-28-10`: `inventario.py` + 2 CSV en `Software/out/fable_inventario_29-10/` |
| Cronología | Día 1, Día 2, Gorran, «Día 3 completo», Ocaso | B1, B2, G, R, O; duración de G sin fijar; citas calculadas desde la fecha | Día 1, 2, Gorran el día 3 (un día), día 4 entero, Ocaso; avisa de que el 22/10 tenía Gorran el día 2 |
| Mira | Propone `(2,59)` en Ciudad Oeste | Observada en `(2,60)` y `(22,14)` en la rama H | Observada en `(7,35)` en av19; propone `(10,60)` junto al paso `(12..16,62)` |
| Familia | Dalor, Kaelen, Tarek, Sora, Jiro, Lina, tío Bael; los hermanos son ganchos a tiendas | Damián, Elvira, Inés, Mauro, Celia, Tomás y Bruno: nombres y parentescos propuestos, pendientes de fijar | Ansel, Dorotea, Bruno, Remei, yaya Ció, Teo, Nil; tres generaciones; cada uno un eslabón; cuenta como una cadena |
| Casa y Verdaderos | Casa en Surysal `(15..16, 25..26)`; ataque delante `(14..18, 28)` | Surysal por instrucción de Oriol; reconciliar ataque y acogida con el canon previo, coordenadas pendientes | Casa en Surysal `(8..11, 17..19)` junto al roble, fuera de la empalizada; los Verdaderos llegan desde Pico Dragón por `(16,1)` |
| Puertas sur | mercado → música, baños → mascotas, posada → mensajería | baños → música, mercado → mascotas, posada → mensajería | mercado → música, posada → mascotas, baños → mensajería |
| Gorran 15 | Trama principal de GORRAN-V2 troceada | 4 del 22/10 + 11 reservas nuevas | 4 del 22/10 + fechorías, Cal Serra, herido, herrero, apuestas + pagaré de Bruno, recado de Nil, mina vieja, barracas, músico |
| Consecuencias | 7 facciones inventadas, deltas ±10 | 4 capas; deltas solo tras cotejar `wf-04` | Mide: 3 facciones y solo mueven precios; propone 4 franjas y `factionRep` (C++ pequeño); deltas ±1, ±2 |
| Economía | Cifras fijadas sin fuente (60 po, 18 po, 6/12/20) | Sin cifras; exige fuente y límite antirrepetición | Ledger `recompensas_escenas.json`; 0,2 nivel por misión; precios según `ECONOMIA.md` |
| Oficios | Asume 8 oficios nuevos | Comprobar catálogo: minería existe, música no | `mineria` y `actuacion_*` existen; nuevo solo el de recadero |
| Validación | Comandos `validar_*` y recorrido de 10 min | Validador a construir tras pilotos | Validadores + explorador 300 partidas + esfuerzo en jornadas en cada ficha |
| Reutilización | Plantilla declarada estándar ya | Estándar definitivo tras probarlo | Ficha + página «estado del mundo» + hoja Huecos = prompt gigante |
| Orden de trabajo | Implementar la Fase A ya | 8 pasos | 7 pasos con agente y jornadas (Fable, Qwen/Codex) |

## Plantilla: 25 puntos frente a 30 apartados

Hay tres fichas, no dos: los 25 de A y los 25 de C son listas distintas. Se parecen más de lo que sugieren los números; la diferencia está en qué cubre cada una, a qué ficheros del juego se ata y cuántas misiones llegan a usarla entera.

Los 25 de A se agrupan en 7 bloques: identidad y contexto (1–5), entidades (6–8), breakpoints (9–11), caminos, tiradas y combates (12–16), diálogos y cinemáticas (17–19), facciones, moral y recompensas (20–23), interconexión y validación (24–25). Los 30 de B cubren lo mismo y añaden apartados que A no tiene como punto propio:

- Función de la misión (qué experiencia ofrece, qué sistema ejercita).
- Rutinas del PNJ por fase y hora, transición entre mapas, reencuentro.
- Texto del objetivo de diario: progreso, cierre parcial, rechazo, cancelación.
- Justificación de cada etapa obligatoria frente a la principal.
- Exploración: qué observa el jugador y cómo descubre una alternativa.
- Ruta accesible para jugador débil, pobre o sin compañero.
- Tiempo: qué acciones avanzan el reloj y cuánto.
- Presentación y persistencia: guardado, cambio de mapa, prevención de duplicados.

A enumera explícitamente los cuatro grados (crítico, éxito, roce, pifia); B pide grados, costes y efectos por fallo/pifia en el apartado 17, sin enumerar los cuatro. A concreta además comandos y un recorrido de 10 minutos; B exige rutas y tiempos medidos en el apartado 30, pero todavía no aporta comandos ni evidencia de ejecución.

Los 25 de C se agrupan en identidad (1–6), estructura (7–11), gente y bichos (12–15), pago (16–20), después (21–23) y producción (24–25). Es la única de las tres que nombra el fichero del motor en cada campo: objetivos opcionales con `doneFlag` en `quests.json`, actos en `actos.json`, facciones en `facciones.json`, pagos en el ledger de `recompensas_escenas.json`. C explicita recompensa por estilo, qué hay que construir y esfuerzo en jornadas. Hay solapamiento con B: caducidad en el apartado 22, recuperación tras fallo en el 23, cierre parcial en el 12 y testigos/propagación/lector en el 25. Conviene fusionar por cobertura verificable y no atribuir exclusividad a esos conceptos. Le faltan, respecto a B: rutinas por fase y hora (solo en el campo 2), texto del objetivo de diario, serie de combates y presentación o persistencia.

### Profundidad real entregada

| Concepto | Contenido | Formato | Fichas con todos los campos |
| --- | --- | --- | --- |
| A | Cadena 01+02 Mira y familia | 25 puntos numerados, 3 tablas, diálogos literales | 1 |
| A | Cadena 04 Xila | 25 puntos numerados | 1 |
| A | Cadena 09 Banco y oro nuevo | 25 puntos numerados | 1 |
| A | 4 tiendas nuevas | Tabla de 7 filas («1–3», «4–5», «20–25») | 0 |
| A | Cadenas 03, 10, 11, 12 | Tabla de 5 filas | 0 |
| A | Misiones 13–30 y G-01 a G-15 | 4–5 viñetas cada una | 0 |
| B | Cartera B01–B30, G01–G15 | Una línea por misión | 0 |
| C | El gato del roble | 25 campos, banderas y recompensas ya existentes | 1 |
| C | Familia (7 eslabones), 4 tiendas | Tabla de 5–6 columnas por fila | 0 |

De 45 misiones anunciadas a 25 puntos en A, 3 los tienen; las otras 42 son resúmenes de entre 5 y 15 líneas con coordenadas. Es el caso que B advierte en su regla de uso: «una misión no está desarrollada por tener 25 filas con una frase». B no ha aplicado su ficha a ninguna misión. C la ha aplicado a una, y lo hace reutilizando lo que ya existe (`b1_gato_no`, `b1_familia_mesa_manana`, `rec_fuego_regalo`, la regla PO-38 de que caer no toca la moral) en vez de inventar banderas nuevas, que es lo que hace A en sus tres fichas.

## Cruce de los cuatro lotes con la base de siete documentos

Nueve diferencias que deben resolverse según el lote. Las coordenadas y la asignación de locales bloquean su implementación; el recuento de cartera y la reputación no impiden avanzar en un piloto independiente que no dependa de ellas.

### 1. Las cuatro puertas sur tienen tres asignaciones y dos trazados

| Puerta | A (lotes, sin commit identificado) | B (base, rama H) | C (plan, rama av19) | `ciudad_centro.xlsx` del proyecto |
| --- | --- | --- | --- | --- |
| `puerta_banco_sur` | (6,39) → Minería | (6,40) → Minería | (6,38) → Minería | (6,38) → interior\_banco |
| `puerta_mercado_sur` | (19,39) → Música | (19,40) → Mascotas | (16,36) → Música | (16,36) → interior\_mercado |
| `puerta_banos_sur` | (14,39) → Mascotas | (14,40) → Música | (56,38) → Mensajería | (56,38) → interior\_banos |
| `puerta_posada_sur` | (48,39) → Mensajería | (48,40) → Mensajería | (26,36) → Mascotas | (26,36) → interior\_posada |
| `puerta_ropa_sur` | (56,58) → pesca y caza | (56,59) → pesca y caza | (56,56) → pesca y caza, cartel «Sastrería» | (56,56) → interior\_sastreria |

Hay dos trazados del centro: B identifica el commit de H; A enlaza archivos locales sin fijar commit. Sus coordenadas difieren en una casilla de `y`, pero eso no prueba que A describa H ni distingue puerta, fachada o punto de interacción. C y el xlsx describen otro trazado según la comparativa original. Solo `banco_sur → minería` coincide en los tres conceptos; música, mascotas y mensajería rotan entre las otras tres puertas según quién lo escribe. El rótulo «Sastrería» que viste lo explica C: la puerta ya entra en pesca y caza, pero el cartel sigue siendo `cartel_ropa_sur`; según esa propuesta, se corregiría el cartel. Antes de fijar el cambio hay que comprobar su instancia, definición de catálogo y texto en la build seleccionada; el nombre del ID por sí solo no demuestra el rótulo dibujado. Hay que fijar una sola fuente: el `ciudad_centro.json` de la build que se vaya a tocar, y una sola rama.

### 2. Las dos listas de Gorran son casi disjuntas

A trocea la trama ya escrita de GORRAN-V2 en 15 misiones: Victor y Asdrugal (G-03), los dados del Cojo (G-04), Joel en Escape Room 8 (G-05), la farsa del comprador (G-07), la huida del Cojo (G-13), el juicio de la Furia (G-14, que el propio lote llama «Hito 1 de la Campaña») y el río subterráneo (G-15). Son escenas de la misión principal, no secundarias. B lo prohíbe de forma explícita y parte de las cuatro ya diseñadas el 22/10 (El precio de Iris, La Veta Honda, Cera gris, La puerta de abajo), que A no menciona, más once reservas nuevas. C está en medio: las cuatro del 22/10, cinco que también están en A (las fechorías, Cal Serra, el herido, el herrero, las apuestas) y seis propias, dos de ellas nacidas de la familia (el pagaré de Bruno, el recado de Nil). Las únicas de A que funcionan como secundarias propias son G-02, G-08, G-09, G-10, G-11 y G-12.

### 3. En Boundington, A cuenta escenas existentes como misiones nuevas

De las 18 misiones 13–30 de A, al menos siete son beats ya implementados o diseñados: 13 Bastián (AV19-08 / CRIPTA-25), 15 Pulga (D1-13), 16 camorrista (T-02), 17 instructor gris y 26 carros (ESCENAS-07), 18 Zarela (D2-04), 19 y 20 fiesta (ESCENAS-03). A les añade tres vías y un lector, lo cual es útil, pero no son contenido nuevo. Las nuevas de verdad son 14, 21, 22, 23, 24, 25, 27, 28, 29 y 30. B hace la distinción en su propia tabla (B17, B24 y B26 marcadas como «existente a profundizar») y por eso aún no certifica 30.

### 4. Reputación: A inventa la escala que B pide no inventar

A fija siete facciones (`vecinos_casco`, `gremios_comercial`, `guardia_corona`, `nobleza_barrio_alto`, `bajos_fondos_pico`, `barriada_surysal`, `culto_perdidos`) con deltas de ±5 a ±15 en cada misión. B exige cotejar con `wf-04-moral-reputacion-y-mutacion.md` antes de fijar un número. C lo ha medido: `facciones.json` tiene tres (`aegroum`, `perdidos`, `barriada_surysal`), la reputación solo cambia precios (±25 y ±50 %) y ningún diálogo ni puerta la lee todavía; propone cuatro franjas y exponer `factionRep` como variable de aventura, con deltas de ±1 y ±2. Los IDs propuestos por A requieren definición o correspondencia con las facciones existentes. Sus deltas no están calibrados; no basta comparar ±10 con ±1 para demostrar incompatibilidad sin conocer rango, umbrales, frecuencia y efectos. Los deltas de C siguen siendo una propuesta, no una escala aprobada. Los tres coinciden en separar moral (`actos.json`) de reputación; B añade confianza personal y reconocimiento comunitario como capas aparte, y avisa de que «una familia no se convierte automáticamente en una facción».

### 5. Cronología: secuencia aprobada y duración pendiente

Los tres tienen la misma secuencia con etiquetas distintas: A cuenta el regreso como «Día 3», C como día 4 (Gorran es el día 3), B usa letras. B también contrasta explícitamente la cronología del 22/10 en B1, «Cronología y esperas». La instrucción posterior de Oriol ya fija dos días iniciales, salida a la aldea y un día entero al regresar, con Ocaso esa noche; no se vuelve a ofrecer Gorran en el día 2 como decisión abierta. A y C asumen que la estancia en Gorran dura un día; B la deja sin fijar. Para el «espéranos X días», A conmuta por `dia_actual`, B pide calcularlo desde la fecha del mundo, y C plantea el ejemplo con Gorran el día 3. Ese ejemplo no resuelve aceptaciones tardías: cada cita debe calcular lo que falta desde la fase/fecha real, con resultado definido si el plazo ya pasó.

### 6. Mira, tres propuestas familiares y la casa de la puerta azul

Mira está en tres sitios: A propone `(2,59)`; B la observó en `(2,60)` en la rama H; C la observó en `(7,35)` junto al árbol torcido en la rama av19 y propone `(10,60)` porque `(2,59)` queda a diez casillas del paso. A y C proponen familias diferentes: A da un padre, cinco hermanos y un tío (Dalor, Kaelen 21, Tarek 19, Sora 17, Jiro y Lina 14, Bael 48), cada hermano como gancho a una tienda; C da tres generaciones (Ansel, Dorotea, el tío Bruno exminero que debe al Cojo, la tía Remei pastelera, la yaya Ció con el violín, Teo 16 en la cola del pan, Nil 12 recadero), cada uno con una misión propia, y la cadena entera cuenta como una de las diez. C es la que se ata al canon escrito (el hacha, la cuñada de los pasteles de miel, Martín y Rufo). Sobre la casa, A y C la mueven a Surysal pero a casillas distintas (`(15..16, 25..26)` y `(8..11, 17..19)`); B adopta Surysal por instrucción de Oriol y deja por desarrollar coordenadas, momento del ataque y acogida. Además propone una tercera familia en B4: Damián, Elvira, Inés, Mauro, Celia, Tomás y Bruno, con conexiones a trabajo, música, mensajería y animales. Bruno coincide como nombre con C, pero tiene otro parentesco/oficio; no se deben fusionar por nombre. C propone resolver el ataque haciendo que los Verdaderos lleguen desde Pico Dragón por `(16,1)` y lo marca como decisión 2.

### 7. Oficios y economía sin cotejar

A crea o asume ocho oficios y fija precios y jornales. B ya comprobó que minería existe en `oficios.json` y música no, y que hay objetos de actuaciones con contrato propio. Ninguna cifra de A cita `ECONOMIA.md`.

### 8. Hallazgos útiles que solo están en un lado

A detectó nueve sótanos y plantas altas amueblados sin misión (`banco_s1`, `mercado_p1`, `biblioteca_p1`, `universidad_p1`, `opera_p1`, `coliseo_s1`, `posada_p1`, `banos_p1`, `cuartel_p1`) y los conecta todos. B detectó que el manifiesto aún dice «Prólogo + tres días», que el diario solo tiene 9 objetivos explícitos y que las 654 filas de PNJ incluyen instancias repetidas. Los dos hallazgos son complementarios.

### 9. Tres inventarios sobre dos ramas

A cuenta 58 PNJ con nombre en `boundington_npcs.json`; B cuenta 654 filas de PNJ e interacciones en 81 niveles de la rama H; C reporta 241 nombres: 38 con talk detectado y colocación, 39 colocados sin talk detectado y 164 sin colocar, según los filtros de su script. No se contradicen: miden cosas distintas en código distinto. Pero cualquier cartera que diga «este PNJ no tiene misión» depende de cuál de los tres se use. C es el único que entrega el script (`inventario.py`) y los CSV; B entrega el resultado en `02_INVENTARIO_OBSERVADO.md`. C señala 12 de 18 locales como posibles huecos. Al leer su script se observa que cuenta PNJ colocados con triggers `talk` por `objectId`; no recorre todos los eventos `auto`/`enter`, condiciones, rutinas ni lógica de código. Por tanto, la conclusión verificable es «sin PNJ con talk directo detectado por este script», no «sin un beat» ni «sin contenido». Además escanea catálogos globales y filtra niveles por nombre, sin restringirse al manifiesto: los 241 nombres no certifican 241 personajes de esta campaña.

## Lo que vale de cada concepto

A aporta material, B aporta reglas y C aporta el enganche con el motor. Conviene no tirar ninguno de los tres.

**De A se aprovecha:**

- Las tres fichas completas (Mira, Xila, Banco), con diálogos literales, grados de tirada y recorrido de prueba.
- Los siete familiares con nombre, edad, oficio y un gancho cada uno hacia una tienda o hacia Gorran.
- La tabla cruzada de 58 PNJ: quién es, dónde está, qué misiones activa y qué eventos lee.
- Los nueve pisos y sótanos sin uso y las ideas para cada uno.
- Diez secundarias nuevas de Boundington (14, 21–25, 27–30) y seis de Gorran (G-02, G-08 a G-12).
- Los comandos de validación y la checklist A1–C4.

**De B se aprovecha:**

- El inventario con commit citado y la separación canon / adaptación aprobada / borrador / implementado.
- La cronología B1–B2–G–R–O con citas calculadas desde la fecha del mundo.
- La ficha de 30 apartados y sus cinco tablas mínimas, en especial ruta accesible, tiempo y fallo recuperable.
- La cartera sin doble cómputo y la regla de que un episodio no cuenta como misión.
- La matriz local → PNJ → sistema → evento que lee el resultado.
- Las cuatro capas de consecuencia y la prohibición de fijar deltas sin cotejar `wf-04` y `ECONOMIA.md`.
- El orden de trabajo en ocho pasos, con los pilotos antes de escalar.

**De C se aprovecha:**

- La ficha de 25 campos atada a `quests.json`, `actos.json`, `facciones.json` y el ledger, con segunda oportunidad, cierre «mal» y recompensa por estilo.
- «El gato del roble» escrita entera sobre banderas y recompensas que ya existen.
- La familia como cadena de siete eslabones atada al canon (hacha, pasteles de miel, Martín y Rufo), con el tío Bruno como puente a Gorran y Nil como puente a la mensajería.
- La medición de que hay tres facciones y solo mueven precios, y la propuesta mínima de franjas y `factionRep`.
- El inventario con script y CSV, y la lista de 12 locales sin PNJ con talk directo detectado, como candidatos a revisión de contenido.
- La explicación del cartel «Sastrería» como cambio de `objectId`.
- La cartera por regiones con lo existente marcado, y el orden de trabajo con agente y jornadas.

## Método combinado para secundarias, oficios y campañas

B y C como esqueleto, A como cantera, en este orden:

1. **Una revisión de integración identificada.** Localizar la build que Oriol quiere ampliar y registrar commit, assets y mapas. Contrastar H y av19 para conservar cambios necesarios; no tratar las dos ramas como opciones excluyentes ni descartar coordenadas sin migración. Fijar después las cinco puertas y validar ida/vuelta y navegación. Esfuerzo pendiente de inspección.
2. **Una ficha fusionada por cobertura.** Cruzar los campos de A, B y C con la petición de Oriol: al menos 25 apartados sustanciales por submisión, con ubicaciones, diálogos, rutas, combate, recompensas y consecuencias. Incorporar los contratos concretos del motor de C sin perder rutinas, diario, oleadas y persistencia de B. No decidir 29 por sumar listas: registrar equivalencias y validar los pilotos. «No aplica» justificado es válido; un pendiente esencial impide declarar la ficha lista para implementar.
3. **Dos pilotos.** «El gato del roble» ya está en C con su ficha; Xila/vestuario parte de la ficha de A y se pasa a la fusionada. Si la ficha no aguanta dos pilotos, se corrige antes de escribir 43 más.
4. **Una familia coherente.** Comparar las tres propuestas A/B/C por parentesco, pasado, función y conexiones, conservando ocho miembros contando a Mira y la ausencia de la madre mientras no se revise. Se pueden combinar aportaciones si se resuelven identidades y contradicciones explícitamente; no hay obligación de aceptar una lista completa. Fijar el reparto antes de desarrollar la cena.
5. **Una sola cartera.** Volcar las tres carteras (45 de A, 45 de B, 45 de C) en una tabla con ID único y estado: nuevo, existente a profundizar, episodio, escena de la principal. Objetivo: 30 dossiers de Boundington con 10 cadenas y un bloque de Gorran, sin contar nada dos veces.
6. **Gorran.** La trama principal no cuenta. Base: las cuatro del 22/10 (B, C), las cinco que A y C comparten (fechorías, Cal Serra, herido, herrero, apuestas), las dos que nacen de la familia (C) y las reservas de B que no dupliquen.
7. **Consecuencias.** Partir de lo medido por C (tres facciones, solo precios); aplicar las cuatro capas de B como regla; verificar en esa revisión rangos, umbrales, escritores y lectores de reputación. Calibrar después deltas y recompensas con las reglas vigentes y `ECONOMIA.md`. Los IDs/deltas de A y las franjas/deltas de C se mantienen como propuestas hasta ese contraste; no se aprueba C por ser una cifra menor.
8. **Oficios y tiendas.** Catálogo según C (`mineria` y `actuacion_*` existen, recadero nuevo). Cada local convertido con interior, ida y vuelta, rótulo, vendedor, horario, existencias, oficio y una consecuencia que otro evento lea (regla de B; fichas de A y C como punto de partida).
9. **Validar antes de escalar.** Confirmar comandos y contratos existentes; comprobar referencias, etapas inaccesibles, ciclos, recompensas repetibles y consecuencias sin lector. Añadir rutas dirigidas para éxito, fallo, llegada tardía, baja habilidad, falta de dinero/aliado y guardado/carga. Las 300 partidas propuestas por C son muestreo adicional: indicar cobertura y semilla, sin confundirlas con prueba exhaustiva o validación jugable.
10. **Solo entonces, el estándar.** La ficha probada más la página «estado del mundo» y la hoja Huecos (C) se convierten en el prompt para otras campañas.

El método que sale de aquí para cualquier campaña: una rama y un commit → inventario con script y hoja de huecos → cronología y citas → página del estado del mundo → cartera con estados → dos pilotos con la ficha → validación → lotes por región → plantilla definitiva.

## Decisiones que quedan para Oriol

**Ya fijado por Oriol:** dos días iniciales, salida a la aldea, un día completo al volver y Ocaso esa noche; familia en Surysal con ocho miembros contando a Mira; cuatro nuevos tipos de tienda; planificación progresiva antes del estándar definitivo; mínimo 25 apartados sustanciales por submisión. No se solicita aprobar otra vez estos puntos.

**Por concretar durante el trabajo:**

- Revisión de integración y mapas de la build de referencia; verificar antes de pedir elegir una rama.
- Asignación de los cuatro negocios a edificios concretos y sus interiores.
- Composición y nombres definitivos de la familia entre las tres propuestas; posición exacta de casa, árbol y rutas.
- Momento del ataque de los Verdaderos y continuidad de la acogida de Zarela.
- Duración de la estancia en Gorran, solo si se necesitan fechas absolutas; la secuencia ya está fijada.
- Si las 15 de Gorran son el total de secundarias de esa zona o 15 nuevas además de las existentes. La principal no se trocea para completar el número.
- Cobertura final de la ficha a partir de dos pilotos; escala y lectores de reputación tras auditoría del motor.

## Alcance de la revisión · Codex, 29/10

Esta revisión corrige la representación de B y las decisiones ya expresadas por Oriol; se consultan directamente B4 y B6, los documentos B de esta sesión, el script `Software/out/fable_inventario_29-10/inventario.py` y fragmentos del lote A1. C no se ha localizado en el workspace ni en la búsqueda local de documentos. Sus cifras y propuestas siguen atribuidas a la comparativa original, pendientes de cotejo directo; no se certifican aquí como implementación.

Una ficha con 25 números no equivale a ficha completa, implementable ni probada. Las tablas de profundidad anteriores cuentan cobertura formal declarada, no una certificación de calidad. Para cerrar un piloto deben resolverse sus ubicaciones, estados, alternativas, recompensas y lectores concretos.

Las diferencias de branch, sistema de coordenadas y unidad de recuento se registran antes de comparar. «Sin talk detectado», «sin colocar» y «sin misión» son estados distintos. La falta de una vía detectada por el inventario no prueba que no exista otra.


## Bibliografía

Cada entrada indica el documento, los apartados citados y en qué sección de esta comparativa se usan. Los cuatro lotes de A están en `~/.gemini/antigravity/brain/50be5445-…/`; los siete de B en `Software/Onegai 2/Planificacion_Boundington/` (commit `7ddbbc8`); C es un doc del proyecto.

### A · Especificación Quirúrgica, Estándar de 25 Puntos (Antigravity, 29/10/2026)

| Ref. | Documento | Apartados citados | Usado en |
| --- | --- | --- | --- |
| A0 | `plan_maestro_boundington_y_estandar_25.md` | No leído; solo se conoce por los enlaces de los lotes | Resumen |
| A1 | `especificacion_25_puntos_lote1_surysal_xila_tiendas.md` | Parte I, Cadena 01+02, Bloques I–VII (puntos 1–25): tabla de PNJ (punto 6), enemigos (7), breakpoints (9–11), vías A–C (12–14), grados de tirada (16), diálogos (17–19), reputación y moral (21–22), validación (25) | Ejes; Plantilla; Cruce §1, §4, §6 |
| A1 | ídem | Parte II, Cadena 04 Xila (puntos 1–25) | Plantilla (profundidad); Método paso 3 |
| A1 | ídem | Parte III, las 4 tiendas nuevas (tablas «1–3» a «20–25») y «Resumen de siguientes pasos operativos», Fase A | Cruce §1, §7; Decisiones |
| A2 | `especificacion_25_puntos_lote2_locales_y_gorran.md` | Nota inicial («Hallazgo clave»: 9 sótanos y plantas altas); Parte I, Cadenas 09–12; Parte II, locales restantes; Parte III, matriz G-01 a G-15; «¿Cómo seguimos ahora?» | Plantilla; Cruce §2, §8; Lo que vale |
| A3 | `especificacion_25_puntos_lote3_secundarias_13_a_30_y_pnjs.md` | Parte I, Cadena 03; Parte II, Misiones 13–30; Parte III, reinventario de 58 PNJ | Cruce §3, §9; Lo que vale |
| A4 | `especificacion_25_puntos_lote4_gorran_15_y_checklist.md` | Parte I, Bloques A–C (G-01 a G-15), en especial G-14 «Hito 1 de la Campaña»; Parte II, Checklist A1–A5, B1–B5, C1–C4 | Cruce §1, §2, §4, §5; Decisiones |

### B · Boundington: base de ampliación de campaña (Planificación v0.1, 29/10/2026)

| Ref. | Documento | Apartados citados | Usado en |
| --- | --- | --- | --- |
| B0 | `00_LEEME.md` | «Qué manda»; «Evidencia y límites» (commit `102dc36`, rama `prueba/claude/integracion-H-29-10`; fuentes de diseño) | Resumen; Ejes |
| B1 | `01_PLAN_Y_HALLAZGOS.md` | «Cronología y esperas»; «Hallazgos del inventario»; «Edificios repetidos: propuesta de reutilización»; «Matriz editorial inicial de cobertura»; «Contexto y reputación»; «Orden de trabajo» | Ejes; Cruce §1, §4, §5, §6, §8, §9; Lo que vale; Método |
| B2 | `02_INVENTARIO_OBSERVADO.md` | Inventario de 81 niveles y 654 filas (no leído entero; cifras tomadas de B1) | Cruce §9 |
| B3 | `03_VOCES_Y_CONDICIONES.md` | No citado | — |
| B4 | [`04_FAMILIA_Y_CADENAS.md`](../Onegai%202/Planificacion_Boundington/04_FAMILIA_Y_CADENAS.md) | Tabla de ocho miembros; «Entrada de Mira»; «Cadena entre Boundington, Rosita y Gorran»; «Piloto Xila»; lectura directa en revisión | Ejes; Cruce §6; Método paso 4 |
| B5 | `05_CARTERA.md` | «Boundington: 30 espacios de contenido, 10 cadenas mínimas» (B01–B32, nota sobre B11 y B25); «Gorran: bloque separado de 15 espacios» (G01–G15, nota final) | Ejes; Cruce §2, §3; Método pasos 5–6 |
| B6 | `06_ESTANDAR_Y_PROMPT.md` | «Regla de uso»; «Treinta apartados obligatorios»; «Tablas mínimas»; «Prompt reutilizable de trabajo»; «Herramientas de trabajo» | Plantilla; Cruce §4, §7; Método pasos 2, 8, 9 |

### C · Boundington y Gorran, plan de contenido (Fable, 29/10/2026)

| Ref. | Documento | Apartados citados | Usado en |
| --- | --- | --- | --- |
| C0 | `claude/plan_contenido_boundington_29-10.md` | Cabecera (rama `agente/qwen/integracion-av19-28-10`; `Software/out/fable_inventario_29-10/`) | Ejes; Cruce §9 |
| C§0 | ídem | §0 «El calendario, como lo he entendido hoy» (tabla de 4 días; decisión 1) | Ejes; Cruce §5; Decisiones |
| C§1 | ídem | §1 «Surysal no es la Barriada» (Mira en `(7,35)`; casa en `(8..11, 17..19)`; Verdaderos por `(16,1)`; decisión 2) | Ejes; Cruce §6; Decisiones |
| C§2 | ídem | §2 «La ficha de misión secundaria, 25 puntos» (campos 1–25 en seis grupos) | Plantilla; Método paso 2 |
| C§3 | ídem | §3 «Ejemplo completo: El gato del roble» (campos 2, 4, 8–11, 16–21, 24–25) | Plantilla (profundidad); Lo que vale; Método paso 3 |
| C§4 | ídem | §4 «La familia de la puerta azul: siete y Mira» (tabla de 7 eslabones; la cadena en el calendario) | Ejes; Cruce §2, §6; Método paso 4 |
| C§5 | ídem | §5 «Tiendas repetidas → cuatro locales nuevos» (coordenadas, nombres, `cartel_ropa_sur`) | Cruce §1, §7; Lo que vale |
| C§6 | ídem | §6 «Inventario: dónde estamos flojos» (241 PNJ, 38 con trama; 12 de 18 locales sin beat; 3 facciones) | Ejes; Cruce §4, §9 |
| C§7 | ídem | §7 «Presupuesto: 30 + 15 secundarias» (reparto por regiones; lista de Gorran) | Cruce §2; Método pasos 5–6 |
| C§8 | ídem | §8 «El mundo en el que pasa todo» (franjas de reputación; `factionRep`) | Cruce §4; Método paso 7 |
| C§9–10 | ídem | §9 «Decisiones para Oriol» (1–7 con respuesta por defecto); §10 «Orden de trabajo propuesto» | Decisiones; Método |

### Otras fuentes consultadas o citadas por los tres

| Ref. | Fuente | Qué aporta | Usado en |
| --- | --- | --- | --- |
| X1 | `ciudad_centro.xlsx` (archivo del proyecto), hoja Objetos | Coordenadas de `puerta_banco_sur`, `puerta_mercado_sur`, `puerta_posada_sur`, `puerta_banos_sur`, `puerta_ropa_sur` y sus interiores | Cruce §1 |
| X2 | `planificacion_alcance_y_vision/workflows_juego/wf-04-moral-reputacion-y-mutacion.md` | Diseño de moral y reputación; citado por B1 como DISEÑO, no auditoría | Cruce §4 |
| X3 | `planificacion_alcance_y_vision/ECONOMIA.md` | Precios y recompensas; citado por B6 y C§2 | Ejes; Método paso 7 |
| X4 | `claude/gorran_dia2_cuatro_misiones.md` y `claude/revision_gorran_6_puntos.md` (22/10) | Diseño previo de Gorran: calendario con Gorran el día 2 y las cuatro misiones Iris, Veta Honda, Cera gris, Puerta de abajo; citados por B1, B5 y C§0 | Cruce §2, §5 |
| X5 | `claude/plantilla_mision_infiltracion.md` | Plantilla hermana de la ficha de C§2 | Plantilla |
| X6 | `MotorGraphico/out/claude_gorran_trama/DISENO_DIA2_GORRAN.md` | Fuente de diseño citada por B0 | Ejes |

No leídos en la comparativa original: A0, B3, B4 y el cuerpo de B2. La revisión de Codex sí lee B4 directamente. El documento C y el xlsx no se han cotejado directamente en esta revisión; se conserva la atribución de la comparación original.

Fuente adicional leída en revisión: [`inventario.py`](../out/fable_inventario_29-10/inventario.py), selección de catálogos/niveles y bucle `trigger.type == talk`; sustenta la limitación metodológica de Cruce §9. Instrucción primaria: mensaje de Oriol en este chat que fija B1 → B2 → salida a la aldea → regreso con un día entero → Ocaso nocturno, y la familia en Surysal; sustenta Cronología y Decisiones.
