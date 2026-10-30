# Plan derivado del Documento de Alcance y Vision

Generado por `planificacion_alcance_y_vision/build.py` desde `dades.py` y `workflows_juego/`. **No se edita a mano**: se regenera, y asi el plan, el Gantt y la pagina no pueden divergir.

Fuente de diseno: `DOCUMENTO_DE_ALCANCE_Y_VISION.md` (28/09/2026). Autoridad: `canon (Onegai 2) -> cartas (dndWeebCC) -> motor` (BITACORA 2.1); donde la vision contradice al reglamento ya implementado, se corrige la vision (ver `workflows_juego/DISCREPANCIAS_Y_DECISIONES.md`).

## 1. El numero

| Fase | Titulo | Tareas | Jornadas | Semanas a ritmo real | Origen |
| --- | --- | ---: | ---: | ---: | --- |
| AV0 | Contratos de datos y decisiones de diseno previas | 7 | 9.5 | 6 | NUEVA |
| AV1 | HUD, legibilidad y ergonomia | 7 | 10.5 | 7 | doc |
| AV2 | Unificacion del editor (ImGui) y retirada de level_editor | 5 | 9.5 | 6 | doc |
| AV3 | Reactividad fisica, moral y reputacion territorial | 8 | 19 | 13 | doc |
| AV4 | Vida social y economia: comercio, propiedades, oficios, romance, gremio | 16 | 34 | 23 | doc |
| AV5 | Combate tactico: cartas, dados d6 y caida sin game over | 11 | 22.5 | 15 | doc |
| AV6 | Editor universal de autoria y matriz de privilegios | 17 | 37 | 25 | doc |
| AV7 | Locomocion, carga y fisicas del jugador | 8 | 16 | 11 | NUEVA |
| AV8 | Sigilo, crimen, guardias, prision e identidad | 10 | 20.5 | 14 | NUEVA |
| AV9 | IA viva: rutinas 24 h, patrullas, clima y escenografia | 8 | 19.5 | 13 | NUEVA |
| AV10 | Misiones organicas, investigacion y bibliotecas | 6 | 14.5 | 10 | NUEVA |
| AV11 | Red: MMO open world (solo expansion) e instancias privadas | 5 | 15 | 10 | doc |
| AV12 | Aceptacion, equilibrio y deuda de contenido | 7 | 13.5 | 9 | NUEVA |
| AV13 | Combate tactico a lo Baldur's Gate 3: tablero, alcance, areas e IA | 36 | 55 | 37 | NUEVA |
| AV14 | Gamefeel y pulido de lo jugado: golpes, sonido, andar, poses y menus | 24 | 43.5 | 29 | NUEVA |
| AV15 | Usabilidad y la ciudad: moverse, orientarse, menus coherentes y accesibles | 7 | 13.5 | 9 | NUEVA |
| AV16 | Pulido de lo jugado el 15/10: creacion, capas, salidas y puertas, suelo, combate | 16 | 20.2 | 13 | NUEVA |
| AV17 | La historia con teatro: escenas con personajes, segundas oportunidades, el dia 3 y el ocaso en llamas | 12 | 19.5 | 13 | NUEVA |
| AV18 | Botin, economia y oficios: cofres, tesoros, tablas, precios y la hoja Trabajar | 7 | 13.5 | 9 | NUEVA |
| AV19 | Las afueras de Boundington y la cueva de Gorran (con altura) | 6 | 15 | 10 | NUEVA |
| AV20 | Temor y respeto por la apariencia (PO-38) | 1 | 1 | 1 | NUEVA |
| **Total** | | **224** | **422.2** | **281** | |

Una jornada (j) son ~6 h de trabajo enfocado. A ritmo real de ratos libres (~1.5 j por semana), las 422.2 jornadas son unas 281 semanas naturales. **Es una estimacion de alcance completo, no de trabajo restante ni una fecha de entrega**: las fechas del Gantt son una escala de esfuerzo con dependencias.

## 2. Los flujos de diseno (la capa que faltaba)

Cada flujo dice que tiene que pasar, con que numeros, quien se entera, que existe YA en el motor (con fichero y linea) y que decision de Oriol lo bloquea. Plantilla de 13 apartados en `workflows_juego/INDICE.md`.

| WF | Flujo | Documento | Base real en el motor | Tareas | Decisiones |
| --- | --- | --- | --- | ---: | ---: |
| WF-01 | Combate tactico: cartas, pool de d6 y caida sin game over | §6, §10.6, §10.7 | solida: BattleState, DicePoolEngine Ed.3, EnemyBrain, ConditionEngine, botin H2-05 | 59 | 7 |
| WF-02 | Movimiento: locomocion, estamina, ruido y carga | §10.12, §10.1.B | parcial: movimiento por celdas y controles (CONTROLES-01/02 en vuelo); nada de estamina, ruido ni kg | 14 | 5 |
| WF-03 | Compras, ventas, regateo y hurto en mostrador | §5.4, §10.14.C, §11.7 | parcial y duplicada: openShop/buyOffer funciona (E-04); ShopEngine paralelo sin usar; precios H2-08 sin aplicar | 8 | 8 |
| WF-04 | Moral, reputacion territorial y mutacion fisica | §3, §10.3, §10.5 | a medias: Morality y factionRep existen pero casi nada los llama; sin sistema de apariencia | 10 | 9 |
| WF-05 | Negocios, propiedades, rentas e inversion urbana | §5, §10.8, §10.9, §11.4 | funcionando: comprar negocio, alquiler 50-200% y collectRent() con pruebas (H3-05); el reloj cuenta pasos | 11 | 9 |
| WF-06 | Equipo, ficha de personaje y progresion por hitos | §10.7, §6.1, §3.1 | solida: ficha, tiers, hitos H2-04, checks de equipar con motivo; sin eleccion de carta ni kg | 8 | 9 |
| WF-07 | Misiones organicas, investigacion y narrativa | §7, §10.10, §10.11, §11.2 | solida: aventuras, beats, flags, timers y diario; faltan rumores, bibliotecas, gremio y calendario | 11 | 9 |

Flujos secundarios (ficha corta en `CATALOGO_FLUJOS_SECUNDARIOS.md`):

| FS | Flujo | Documento | Prioridad del primer corte | Tareas |
| --- | --- | --- | --- | ---: |
| FS-01 | Crimen, deteccion y niveles de alerta | §10.1 A-C | alta | 6 |
| FS-02 | Prision profunda, trabajos forzados y fuga | §10.1 C | media | 3 |
| FS-03 | Identidad, disfraz y subculturas | §10.2 | baja | 2 |
| FS-04 | Vinculos, cortejo, romance, familia y corrupcion | §10.4, §10.5 | media | 4 |
| FS-05 | Oficios, crafteo y jornada laboral | §10.8 | media | 3 |
| FS-06 | IA viva: rutinas 24 h, patrullas y fauna | §10.13 | alta | 7 |
| FS-07 | Escenografia viva: distritos, clima y objetos | §10.14 | media | 5 |
| FS-08 | Hogar, decoracion y vida domestica | §10.9, §10.4.5 | baja | 3 |

## 3. Que significa «hecho» en cada fase

### AV0 — Contratos de datos y decisiones de diseno previas

Documento: §1, §2, §10, §11.5 · Origen: NUEVA · 7 tareas · 9.5 j

**AV0-01 — Glosario unico de magnitudes: moral multifactor, reputacion por afinidad, estamina, peso en kg, ruido, sospecha, prosperidad**  
`1.5 j` · depende de: ninguna · flujo: WF-03, WF-04, WF-05, WF-06  
Documento: §1, §3, §10.1, §10.12, §10.14  
Ya existe: nada: cada documento usa estos terminos por su cuenta  
Cierre: Una tabla con cada magnitud, su rango, su formula y la capa que manda (canon/cartas/motor, BITACORA 2.1). Con las decisiones de Oriol del 28/09: UNA sola moral multifactor (B1/D-MOR-02/D-COM-02), 4 stats CON/DES/INT/CAR con rango 0..8 segun tier (A6), peso en kg donde CON hace de Fuerza (A7), estamina para correr/nadar/volar/saltar (D-MOV-02) y ruido por tirada + ruido del sitio + iluminacion (D-MOV-04). Ningun termino queda sin dueño.

**AV0-03 — Mapa de solapamientos con los paquetes vivos (Fase H, EST, corte B, fase E): que esta ya escrito y de quien es**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: §9, §12  
Ya existe: JUGABILIDAD_FASE_H.md, TABLERO_ESTABILIZACION.md, ALCANCE_REAL_2026-09-17.md, medir_alcance.py  
Cierre: Cada tarea de este plan dice si parte de cero, si continua un paquete vivo (con su ID) o si lo duplica; los duplicados se retiran del plan.

**AV0-02 — Contratos JSON de los catalogos nuevos (facciones, reputacion, propiedades, rentas, rutinas, clima, delitos, cartas)**  
`2 j` · depende de: AV0-01 · flujo: transversal  
Documento: §3.2, §5, §10.1, §10.13, §10.14  
Ya existe: D-01 ya fija como se declaran los campos de un catalogo RPG (20 loaders, 308 campos, carga todo-o-nada)  
Cierre: Cada catalogo nuevo tiene su schema con campos obligatorios y una prueba de carga que nombre campo, alias y entrada. Sin catch(...) mudo.

**AV0-04 — Autoridad cliente/servidor por sistema y validacion del contenido de expansion**  
`1 j` · depende de: AV0-01 · flujo: transversal  
Documento: §2  
Ya existe: nada de red en el motor  
Cierre: Una tabla por sistema: quien manda (cliente o servidor) y como se valida que un objeto es de expansion. Registrada como decision D00xx.

**AV0-05 — Fichas de decision pendientes de Oriol que bloquean fases**  
`1 j` · depende de: AV0-03 · flujo: transversal  
Documento: §10.3, §10.4, §12  
Ya existe: en «Esperando a Oriol» ya viven 3 numeraciones de fases sin resolver y el contrato de catalogos D0003  
Cierre: Cada bloqueo tiene su ficha en decisiones/ con Estado: proposed y la fase que bloquea escrita dentro.

**AV0-06 — Criterio de cierre «que sea divertido»: que se prueba en cada fase antes de darla por hecha**  
`1 j` · depende de: AV0-03 · flujo: transversal  
Documento: §1  
Ya existe: regla de cierre por alcance propuesta por Claude el 17/09 (medir_alcance.py), sin decidir  
Cierre: Ningun paquete se cierra si su producto no lo usa ningun ejecutable; el criterio queda escrito y se aplica en AV12.

**AV0-07 — Afinidad y maestria (oficio, fe, dominio de clase) como progresion paralela al tier**  
`1.5 j` · depende de: AV0-01 · flujo: WF-06  
Documento: §10.7  
Ya existe: nada: el motor solo tiene hitos y tiers (CharacterSheet.h:72, «el reglamento no va de XP»)  
Cierre: Un contrato con las pistas de afinidad (oficio, fe, dominio de clase), como suben y que desbloquean cada una. El tier sigue siendo la progresion del juego de mesa y la XP del documento de vision vive aqui, no como nivel (A5).

### AV1 — HUD, legibilidad y ergonomia

Documento: §9.1 / §12 Fase 1 · Origen: doc · 7 tareas · 10.5 j

**AV1-03 — Tipografia UTF-8 limpia y wrapWidth automatico**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: §9.1  
Ya existe: la fuente dibuja acentos desde el 17/08; la veda de tildes en los JSON de aventura sigue en «Esperando a Oriol»  
Cierre: Un texto largo con acentos y eñes se parte solo dentro de su caja en las 4 pantallas con mas texto.

**AV1-01 — Barras PV/PM/Moral sin solapamiento y con la moral como magnitud visible**  
`1.5 j` · depende de: AV0-01 · flujo: WF-04  
Documento: §9.1, §1  
Ya existe: el HUD actual solapa barras (AUDITORIA_UI_ETIQUETAS); la moral no existe como magnitud  
Cierre: En 1080p y en ventana pequeña las tres barras se leen enteras y la moral cambia de aspecto segun su valor.

**AV1-04 — Widget de tirada de dados d6 en el HUD: pool, exitos 4-6, critico y pifia**  
`2 j` · depende de: AV0-01 · flujo: WF-01  
Documento: §6.3, §10.6.A4  
Ya existe: H2-01: DicePoolEngine::roll_pool y opposed_degree ya resuelven de verdad; falta toda la presentacion  
Cierre: Al hablar, atacar o usar una habilidad se ven los dados y el grado; la pifia se distingue a simple vista.

**AV1-07 — Contrato de lectura de estamina, ruido y carga hacia el HUD**  
`1 j` · depende de: AV0-01 · flujo: WF-02  
Documento: §10.12.A, §10.12.B  
Ya existe: nada  
Cierre: Las tres magnitudes tienen su canal de publicacion definido y un widget provisional que las pinta.

**AV1-02 — Barra contextual fija de controles [ESC] [I] [E] [M] por modo**  
`1 j` · depende de: AV1-01 · flujo: transversal  
Documento: §9.1  
Ya existe: PlayerControls/PlayerControlsData existen; H1-03 cableo ESC→Opciones y H1-05 el idioma  
Cierre: La barra dice que hace cada tecla en exploracion, combate, dialogo, tienda y decoracion, y no tapa el mundo.

**AV1-05 — Iconos de condiciones y estados con rondas restantes**  
`2 j` · depende de: AV1-04 · flujo: WF-01  
Documento: §10.6.A1  
Ya existe: H2-03: ConditionEngine con 3 modos de stacking y damage = damagePerRound x stacks; CharacterSheet::conditions  
Cierre: Un enemigo envenenado muestra icono y contador, y ambos bajan por ronda.

**AV1-06 — Reloj de mundo, calendario y clima en el HUD**  
`1.5 j` · depende de: AV0-02 · flujo: transversal  
Documento: §10.13.A, §10.14.B  
Ya existe: nada: no hay reloj de mundo; F1 de zcode ya congela el dt activo en dialogo/combate  
Cierre: Se ve hora, dia y tiempo; el reloj avanza con el descanso, el viaje y la prision.

### AV2 — Unificacion del editor (ImGui) y retirada de level_editor

Documento: §9.2 / §12 Fase 2 · Origen: doc · 5 tareas · 9.5 j

**AV2-01 — Propiedades de instancia dentro de motor_editor (ImGui)**  
`3 j` · depende de: AV0-03 · flujo: transversal  
Documento: §9.2, §11  
Ya existe: EditorState::setObject* y el panel de capas E-12 ya estan; H4 cuenta 14 acciones de autor  
Cierre: Editar las propiedades de un objeto del mapa sin salir de motor_editor, con deshacer de una sola operacion.

**AV2-02 — Gestor de proyectos en motor_editor**  
`2 j` · depende de: AV2-01 · flujo: transversal  
Documento: §9.2  
Ya existe: Project/ProjectIndex separados del editor (EST-24, opencode 24/09)  
Cierre: Abrir, crear y cambiar de proyecto desde el editor; el indice se refresca sin reiniciar.

**AV2-04 — Herramientas clicables completas y marcadores en rombo**  
`2 j` · depende de: AV2-01 · flujo: transversal  
Documento: §9.2  
Ya existe: hitbox de herramienta incompleta (revision de zcode 03/09)  
Cierre: Cada herramienta tiene su zona clicable y los marcadores se ven como rombos sobre el mapa.

**AV2-03 — Retirar level_editor de CMake y dejar motor_editor como unico editor**  
`1 j` · depende de: AV2-02 · flujo: transversal  
Documento: §9.2  
Ya existe: los dos conviven; level_editor tiene cruces con E-15/E-16  
Cierre: Un solo ejecutable de editor; build y CI no mencionan level_editor.

**AV2-05 — Panel de problemas y referencias integrado en el flujo de autor**  
`1.5 j` · depende de: AV2-02 · flujo: transversal  
Documento: §11  
Ya existe: E-15 (qwen): ProblemsPanel con 3 pestañas y busqueda con acentos plegados, sin integrar del todo  
Cierre: Una referencia rota aparece en Problemas y un doble clic lleva al objeto en el mapa.

### AV3 — Reactividad fisica, moral y reputacion territorial

Documento: §3, §10.3, §10.5 / §12 Fase 3 · Origen: doc · 8 tareas · 19 j

**AV3-01 — Modelo de apariencia por capas: cicatrices, venas de mana, musculatura, tatuajes, ropa**  
`3 j` · depende de: AV0-02 · flujo: WF-04  
Documento: §3.1  
Ya existe: AnimatedEntity y Render/Player.h; ninguna apariencia reactiva  
Cierre: Cada mutacion es una capa con su disparador declarado en datos y se ve sobre el personaje.

**AV3-03 — Reputacion por faccion (-100..+100) y tabla de efectos por banda**  
`3 j` · depende de: AV0-02 · flujo: WF-04  
Documento: §3.2, §10.3.A  
Ya existe: nada; locations.json ya tiene las 19 naciones canonicas  
Cierre: Cada faccion tiene su valor por personaje y la banda decide precios, accesos y trato de los NPC.

**AV3-02 — Disparadores de mutacion: abuso de magia, cero magia, derrotas, prision**  
`2 j` · depende de: AV3-01 · flujo: WF-01, WF-04  
Documento: §3.1  
Ya existe: H3-08/G-11 hablan de recuperacion de derrota, sin cicatriz; el tatuaje carcelario no existe  
Cierre: Caer en combate añade una cicatriz permanente y queda guardada; el contador de venas sube con el uso de magia.

**AV3-04 — Afinidad cruzada: el mismo acto suma y resta en facciones distintas**  
`2 j` · depende de: AV3-03 · flujo: WF-04  
Documento: §10.3.B  
Ya existe: nada  
Cierre: Un acto definido en datos mueve varias facciones a la vez; el caso Ecla/Drow del documento se reproduce.

**AV3-05 — Reaccion social del NPC segun banda: aplaudir, huir, escupir, rendir pleitesia**  
`2 j` · depende de: AV3-03 · flujo: WF-04  
Documento: §3.2, §10.3.A  
Ya existe: NarrativeEngine con beats y skillCheck diferido (H3-03), sin entrada de reputacion  
Cierre: El mismo NPC reacciona distinto con -80 y con +80, y la reaccion sale de datos, no de codigo.

**AV3-08 — Moral multifactor: un unico valor complejo con sus factores, y reputacion por afinidad de grupo, raza, estado y politica**  
`3 j` · depende de: AV0-02, AV3-03 · flujo: WF-04  
Documento: §3.2, §10.3  
Ya existe: Morality (eje unico con motivo e historial), CharacterSheet::morality y ShopEngine::moral (float 0..1): tres morales conviven; factionRep existe sin cablear  
Cierre: UNA moral con sus factores declarados (actos, pureza, afinidad) y sin duplicados: Morality manda, CharacterSheet::morality serializa y ShopEngine::moral desaparece. Los precios y el trato responden a la afinidad con grupo, raza, estado o afinidad politica (A8/B1/D-MOR-02).

**AV3-06 — Fronteras vivas: salvoconductos, acceso a espacios cerrados y escaramuzas en la raya**  
`2 j` · depende de: AV3-03, AV3-05 · flujo: WF-04  
Documento: §3.2, §10.3.C  
Ya existe: LevelTransition existe pero nadie conecta el atlas con ella (H3-01)  
Cierre: Una faccion hostil cierra el paso y una aliada lo abre; cruzar la frontera cambia quien responde.

**AV3-07 — Corrupcion y redencion moral de NPC: la brujula de los allegados se mueve**  
`2 j` · depende de: AV3-05 · flujo: WF-04, FS-04  
Documento: §10.5  
Ya existe: nada  
Cierre: Los dos casos del documento (sacerdote corrompido, mercenaria redimida) se pueden ejecutar desde datos de dialogo.

### AV4 — Vida social y economia: comercio, propiedades, oficios, romance, gremio

Documento: §4, §5, §10.4, §10.8, §10.9, §10.11 / §12 Fase 4 · Origen: doc · 16 tareas · 34 j

**AV4-01 — Pantalla de compra/venta con stock, oro y precios del catalogo real**  
`2 j` · depende de: AV0-02 · flujo: WF-03  
Documento: §5.4, §10.14.C  
Ya existe: H3-04 (claude 26/09) y ShopEngine (H1-02); H2-08 precios bloqueado: EquipmentDefinition::price (413) no lo lee nadie  
Cierre: Entrar en la tienda, ver stock y precios reales, comprar, vender a 0.5x del valor en tienda, y que el oro y el inventario cuadren. Unificado sobre el modelo ShopItem (B2) y con precio base + dificultad de comercio/escasez por tienda (B4).

**AV4-02 — Regateo activo por carisma y reputacion mercantil**  
`1.5 j` · depende de: AV4-01 · flujo: WF-03  
Documento: §5.4  
Ya existe: opposed_degree del reglamento Ed. 3 resuelve tiradas enfrentadas, sin uso comercial  
Cierre: Regatear cambia el precio segun el grado de la tirada y la fama local; fallar sube el precio o cierra el trato.

**AV4-03 — Compra de inmuebles y escrituras (viviendas, tiendas, talleres)**  
`2 j` · depende de: AV4-01, AV3-03 · flujo: WF-05  
Documento: §5.1, §10.9  
Ya existe: nada; el editor ya puede marcar una puerta como inmueble comprable (§11.4.2, por hacer en AV6)  
Cierre: Acercarse al cartel de «Se Vende», pagar la tasacion y que la propiedad quede registrada a nombre del personaje.

**AV4-07 — Oficio de herreria: mineral, forja, minijuego de yunque, temple y calidades**  
`2.5 j` · depende de: AV4-01 · flujo: WF-05, FS-05  
Documento: §10.8  
Ya existe: nada; el catalogo de equipo ya tiene 413 piezas y rarezas (rarityRank)  
Cierre: Forjar un arma con las 4 calidades y que la calidad salga del minijuego, con ranura de runa en la obra maestra.

**AV4-08 — Oficio de alquimia: recoleccion, laboratorio, destilacion y recetas**  
`2.5 j` · depende de: AV4-01 · flujo: WF-05, FS-05  
Documento: §10.8  
Ya existe: consumables.json existe y el juego no lo lee (H2-08)  
Cierre: Recolectar en el bosque, destilar y obtener pociones con efecto real en combate y en el descanso.

**AV4-15 — Bolsa de oro por NPC, precio base de tienda y dificultad de comercio o escasez de recursos**  
`2 j` · depende de: AV4-01 · flujo: WF-03  
Documento: §5.4, §10.14.C  
Ya existe: ShopItem tiene price y stock; ningun NPC tiene limite de dinero  
Cierre: Ningun vendedor puede pagar cifras exorbitantes: cada NPC tiene una bolsa predeterminada que se agota y se repone, salvo quien de verdad pueda pagarla. Venta a 0.5x del valor en tienda y cada tienda aplica su precio base mas dificultad de comercio o escasez (B4/C).

**AV4-04 — Decoracion de interiores en cuadricula isometrica 2.5D**  
`2 j` · depende de: AV4-03 · flujo: WF-05, FS-08  
Documento: §5.3, §10.9  
Ya existe: capas de baldosas y objetos del mapa (E-12), pero no una cuadricula de mobiliario del jugador  
Cierre: Colocar camas, baules, mesas y estanterias en la casa comprada, con colision y guardado por parcela.

**AV4-05 — Rentas e inquilinos: alquiler bajo prospera, alquiler usurero genera odio**  
`1.5 j` · depende de: AV4-03 · flujo: WF-05  
Documento: §5.1, §10.9  
Ya existe: H3-05 (zcode 28/09): GameSession::collectRent existe y nadie de fuera la llama  
Cierre: Comprar un negocio, fijar la renta, cobrarla con el calendario y ver el efecto en prosperidad y en reputacion.

**AV4-09 — Jornada laboral, mostrador propio, diezmos e impuestos locales**  
`1.5 j` · depende de: AV4-07, AV4-08 · flujo: WF-05, FS-05  
Documento: §10.8  
Ya existe: nada  
Cierre: Vender lo fabricado en tu tienda, cerrar el dia, pagar impuestos y ganar maestria profesional.

**AV4-10 — Los tres medidores del NPC: afecto, respeto y confianza**  
`3 j` · depende de: AV3-05 · flujo: FS-04  
Documento: §10.4.A  
Ya existe: nada  
Cierre: Cada NPC guarda los tres valores y las opciones de dialogo se abren o se cierran segun ellos.

**AV4-16 — Durabilidad y degradacion del equipo: se estropea con el uso y se repara en el taller**  
`2 j` · depende de: AV4-07 · flujo: WF-06  
Documento: §10.8, §10.7  
Ya existe: nada: el equipo no se degrada (EquipmentDefinition no tiene durabilidad)  
Cierre: Una pieza pierde durabilidad con el uso y pierde bonos al degradarse; el herrero la repara por un precio. Oriol respondio SI a D-EQU-09 el 28/09, en contra de la recomendacion: entra en el plan con su coste.

**AV4-06 — Inversion publica municipal: reparar infraestructuras y transformar el pueblo**  
`2 j` · depende de: AV4-05, AV3-03 · flujo: WF-05  
Documento: §5.2, §10.9  
Ya existe: nada  
Cierre: Donar oro al concejo cambia el aspecto del pueblo, baja la delincuencia y levanta una placa conmemorativa.

**AV4-11 — Cortejo en 5 fases: primera impresion, dialéctica, regalos, intimidad, enlace**  
`3 j` · depende de: AV4-10 · flujo: FS-04  
Documento: §10.4.B  
Ya existe: NarrativeEngine con beats y skillCheck (H3-03); nada de romance  
Cierre: Las 5 fases son jugables con consentimiento explicito y cada una deja estado guardado.

**AV4-13 — Gremio de aventureros: mercenarios con moralidad y tarifa diaria**  
`2.5 j` · depende de: AV4-10 · flujo: WF-07  
Documento: §4.4, §10.11  
Ya existe: nada  
Cierre: Contratar un mercenario, que un templario se niegue a un encargo infame y que el contratado cubra al jugador en combate.

**AV4-12 — Matrimonio, convivencia, descendencia y beneficios de hogar**  
`2 j` · depende de: AV4-11, AV4-03 · flujo: FS-04, FS-08  
Documento: §10.4.B5  
Ya existe: nada  
Cierre: Casarse, mudarse a una casa propia, que los hijos crezcan con el calendario y que el hogar de bonos al descansar.

**AV4-14 — Publicar encargos propios en el tablon y recogerlos dias despues**  
`2 j` · depende de: AV4-13, AV10-02 · flujo: WF-07  
Documento: §4.4, §10.11  
Ya existe: nada  
Cierre: Depositar una recompensa, que pase el tiempo de mundo y que otro grupo traiga el encargo.

### AV5 — Combate tactico: cartas, dados d6 y caida sin game over

Documento: §6, §10.6, §10.7 / §12 Fase 5 · Origen: doc · 11 tareas · 22.5 j

**AV5-01 — Mazo de habilidades y mano visible con limite por INT/SAB**  
`2.5 j` · depende de: AV0-02 · flujo: WF-01, WF-06  
Documento: §6.1, §10.6.A2  
Ya existe: skills.json y spells.json cargan en pruebas; el juego no lee spells (H2-06) y 202 de 210 hechizos estan huerfanos  
Cierre: En combate se ve la mano, se roba hasta el limite y las cartas vienen del mazo del personaje.

**AV5-02 — Enfriamiento de carta por reposo corto o largo (sin PA ni PM) y rango**  
`2 j` · depende de: AV5-01 · flujo: WF-01  
Documento: §6.1, §10.6.A3  
Ya existe: SkillEffect y BattleState resuelven efectos; los costes no se muestran ni se validan en pantalla  
Cierre: Una carta en enfriamiento no se puede jugar y se ve por que; el reposo la recupera. SIN recurso PA/PM (A13/D-COMB-06): solo cooldown, ligado a reposo corto o largo, y por defecto LARGO si la habilidad no lo indica.

**AV5-05 — Cartas de Creadores (legendarias) con efectos que alteran el campo**  
`2 j` · depende de: AV5-01 · flujo: WF-01  
Documento: §6.2, §10.6.A3  
Ya existe: passives.json y summons.json existen; las cartas de Run'Vel/Voluntades Gemelas/TML no  
Cierre: Una carta de Creador cambia el clima o la fisica del campo durante el combate y queda registrada en datos.

**AV5-09 — Progresion por hitos: tier, atributos y eleccion de carta entre 3**  
`2 j` · depende de: AV5-01 · flujo: WF-01, WF-06  
Documento: §10.7  
Ya existe: H2-04 (zcode 28/09): CharacterSheet::tierUp implementado y grantMilestoneOnEnd parseado; nadie lo llamaba  
Cierre: Terminar una aventura con hito sube el tier, cambia mano y limites, y deja elegir 1 carta entre 3.

**AV5-11 — Iniciativa 1d6 + Destreza y orden de turno**  
`1.5 j` · depende de: AV5-01 · flujo: WF-01  
Documento: §10.6.A1  
Ya existe: BattleState evita la iniciativa a proposito (BattleState.h:24-32: aliados uno a uno y una pasada enemiga)  
Cierre: El orden de turno sale de 1d6+DES y se ve en el HUD. Oriol decidio el 28/09 (A3) en contra del diseño actual del motor: el cambio se registra en el reglamento Ed. 4 antes de tocar BattleState, y las pruebas de turno se reescriben con el orden nuevo.

**AV5-03 — Resolucion por pool de d6: exitos Ed.3 (6=1, 5=0.5), critico si la mitad son 6, pifia sin 5/6 y dos 1es**  
`2 j` · depende de: AV5-02, AV1-04 · flujo: WF-01  
Documento: §6.3, §10.6.A4  
Ya existe: DicePoolEngine::roll_pool y opposed_degree (reglamento Ed. 3, sin CA ni CD)  
Cierre: El daño y la salvacion salen del pool pintado en pantalla. Reglas de Oriol 28/09 (A1/A2): exitos segun Ed. 3, CRITICO = seises >= mitad del pool redondeado hacia arriba (1d6 con un 6 es critico), pifia = ningun 5/6 y minimo dos 1es. Cambia DicePoolEngine y sus pruebas: registrar en el reglamento Ed. 4 ANTES de tocar codigo (el flujo es canon -> cartas -> motor).

**AV5-08 — Interaccion del combate con el decorado: cobertura, objetos arrojadizos, incendios**  
`2 j` · depende de: AV5-02, AV9-05 · flujo: WF-01  
Documento: §10.14.C  
Ya existe: nada; los objetos de la ciudad son inertes (H3-07)  
Cierre: Volcar una mesa da cobertura, una jarra desarma y un proyectil de fuego propaga un incendio que hace huir a los clientes.

**AV5-10 — Personalizacion del mazo entre combates y EXP por vias no violentas**  
`1.5 j` · depende de: AV5-09 · flujo: WF-01, WF-06  
Documento: §10.7  
Ya existe: nada  
Cierre: Retirar cartas obsoletas, reequilibrar ataque/defensa/magia y que descubrir una biblioteca o ganar un debate de progreso.

**AV5-04 — Seleccion de objetivos, alcance y area con cancelacion sin gasto**  
`2.5 j` · depende de: AV5-03 · flujo: WF-01  
Documento: §10.6.A3  
Ya existe: H2-02: selectTargetsFor con SELF/SINGLE_ENEMY/ALL_ENEMIES/SINGLE_ALLY/ALL_ALLIES/ZONE_RADIUS  
Cierre: Elegir un hechizo de area marca a los afectados y cancelar no gasta PM.

**AV5-06 — IA tactica enemiga: agresivo, cobarde, emboscador, con su propia mano**  
`2.5 j` · depende de: AV5-03 · flujo: WF-01  
Documento: §10.6.A5, §11.3.3  
Ya existe: EnemyBrain (constructor simple pos/patMin/patMax/maxHealth/stepInterval) y el menu de combate hardcodeado  
Cierre: Tres comportamientos distintos jugables contra el mismo grupo y el enemigo juega cartas, no ataques fijos.

**AV5-07 — Caida sin game over: noqueo y robo, rescate del eremita, cicatriz permanente**  
`2 j` · depende de: AV5-03, AV3-02 · flujo: WF-01  
Documento: §10.6.B  
Ya existe: H3-08: G-11 menciona slots y recuperacion de derrota, sin decidir donde reapareces ni que pierdes  
Cierre: Perder no cierra el juego: despiertas donde toca, falta algo en la bolsa y el personaje gana una cicatriz.

### AV6 — Editor universal de autoria y matriz de privilegios

Documento: §8, §11 / §12 Fase 6 · Origen: doc · 17 tareas · 37 j

**AV6-01 — Catalogo de NPC: personalidad, voz, complexion y sprite base**  
`2.5 j` · depende de: AV0-02 · flujo: transversal  
Documento: §11.1.1  
Ya existe: npcs.json existe y se genera desde el canon (BITACORA 2.6); sin personalidad ni rutina  
Cierre: Crear un NPC nuevo desde el editor con sus datos basicos y que aparezca en el mundo.

**AV6-06 — Creador de monstruos: tier y formulas automaticas de CA, PV y bono de ataque**  
`2 j` · depende de: AV0-02 · flujo: transversal  
Documento: §11.3.1  
Ya existe: monsters.json con 432 fichas; populate_from_monster_catalog corrige via recompute_derived (H2-07)  
Cierre: Elegir tier y que el editor calcule las cifras con el reglamento de Onegai, no a mano.

**AV6-09 — Pincel de zonas de ley y jurisdiccion: orden imperial, mercado neutro, tierra de nadie**  
`2 j` · depende de: AV2-01 · flujo: transversal  
Documento: §11.4.1  
Ya existe: capas de baldosas (E-12); ninguna capa de jurisdiccion  
Cierre: Pintar la zona y que el delito en zona roja quede impune y en zona azul traiga guardias.

**AV6-13 — Entorno y atmosfera de region: ciclo de luz, generador de lluvia y audio 3D espacial**  
`2.5 j` · depende de: AV2-01 · flujo: FS-07  
Documento: §11.8  
Ya existe: shaders de fog y lightmap en el motor; ningun editor de atmosfera  
Cierre: Fijar curvas de amanecer/mediodia/atardecer/noche, intensidad de lluvia y tres emisores de audio por plaza.

**AV6-14 — Matriz de privilegios Admin MMO / Game Master / Jugador con bloqueo de escritura sobre catalogos oficiales**  
`3 j` · depende de: AV0-04 · flujo: transversal  
Documento: §8, §11.5  
Ya existe: nada  
Cierre: Un Master no puede escribir sobre el catalogo del MMO aunque lo intente; el bloqueo se prueba.

**AV6-02 — Editor de rutina diaria 24 h por franjas y modo (comercio, social, dormir)**  
`2 j` · depende de: AV6-01 · flujo: FS-06  
Documento: §11.1.2  
Ya existe: nada  
Cierre: Pintar el horario de Luisarda y que el NPC lo cumpla: mostrador, taberna, casa con cerrojo.

**AV6-03 — Editor de parametros morales: faccion, romance habilitado, gustos y tolerancia al crimen**  
`2 j` · depende de: AV6-01, AV3-03 · flujo: transversal  
Documento: §11.1.3  
Ya existe: nada  
Cierre: Fijar faccion, gustos y que hacer cuando presencia un delito, sin tocar codigo.

**AV6-08 — Generador de botin con porcentajes de drop**  
`1.5 j` · depende de: AV6-06 · flujo: transversal  
Documento: §11.3.4  
Ya existe: RPG::LootResolver y loot_tables.json; H2-05 ya cablea el botin al ganar (verificado 28/09, sin commit)  
Cierre: Arrastrar objetos con su porcentaje y que el botin salga de esa tabla al ganar.

**AV6-10 — Marcar edificio como inmueble comprable con precio, renta base y capacidad de muebles**  
`1.5 j` · depende de: AV6-09 · flujo: WF-05, FS-08  
Documento: §11.4.2  
Ya existe: nada  
Cierre: Clic en la puerta, casilla de inmueble y sus tres parametros; el juego los lee para AV4-03.

**AV6-11 — Rutas de patrulla: waypoints con espera, orientacion de cabeza y animacion**  
`2 j` · depende de: AV6-09 · flujo: FS-06  
Documento: §11.6  
Ya existe: propiedades de instancia de patrulla (MapEntityDocument, patMin/patMax de EnemyBrain)  
Cierre: Trazar W1→W2→W3 con 10 s de espera y orientacion, y que la guardia la recorra de noche con antorcha.

**AV6-15 — Barra de control en vivo del Master: pausar mundo, inyectar monstruo, tirada secreta**  
`2 j` · depende de: AV6-14 · flujo: transversal  
Documento: §11.5  
Ya existe: nada  
Cierre: Con una partida abierta, el Master pausa, suelta una criatura en una celda y tira dados ocultos.

**AV6-16 — Aislamiento en disco de la campana del Master (partidas/mi_campana/*.json)**  
`1.5 j` · depende de: AV6-14 · flujo: transversal  
Documento: §11.5  
Ya existe: ProjectIndex escanea proyectos; no hay separacion por rol  
Cierre: Todo lo que crea el Master cae en su carpeta privada y nada de eso contamina el mundo abierto.

**AV6-04 — Editor de arboles de dialogo por nodos con tiradas y bifurcaciones**  
`3 j` · depende de: AV6-03 · flujo: WF-07  
Documento: §11.1.4  
Ya existe: NarrativeEngine con beats y skillCheck diferido (4 grados → 4 flags); el editor no los dibuja  
Cierre: Dibujar un dialogo con una tirada de Carisma DC 13 que bifurca en exito y fallo, y jugarlo.

**AV6-12 — Tiendas fisicas: props comerciales y ranuras de mercancia con DC de hurto y reposicion**  
`2.5 j` · depende de: AV6-10 · flujo: WF-03  
Documento: §11.7  
Ya existe: DEC-05 aislo los catalogos de tienda; la mercancia no existe como objeto colocado en el mundo  
Cierre: Poner una pocion sobre el mostrador con precio 25, DC 14 y reposicion cada 24 h de juego.

**AV6-05 — Editor de misiones: objetivos dinamicos, triggers de celda y condiciones compuestas**  
`3 j` · depende de: AV6-04 · flujo: WF-07  
Documento: §11.2  
Ya existe: questIds en los documentos y QuestDefinition NO existe (AUDITORIA_CONTRATOS); fase E tiene interruptores locales  
Cierre: Pintar 4 baldosas de trigger con condicion de hora y sigilo, y que al pisarlas salte la escena. Sobre AdventureScript como UNICA fuente, no un QuestDefinition paralelo (D-MIS-01), y declarando el tipo de mision: principal, evento, secundaria, de profesion, de raza o de evento unico.

**AV6-07 — Asignacion de baraja y de comportamiento tactico al monstruo**  
`2 j` · depende de: AV6-06, AV5-06 · flujo: transversal  
Documento: §11.3.2, §11.3.3  
Ya existe: nada  
Cierre: Darle 3x Zarpazo, 2x Grito, 1x Salto y comportamiento emboscador, y que lo use en combate.

**AV6-17 — Segregacion de misiones por tipo en el editor y en el diario: principales, eventos, secundarias, profesion, raza y evento unico**  
`2 j` · depende de: AV6-05, AV10-01 · flujo: WF-07  
Documento: §7, §10.10, §11.2  
Ya existe: DiarioMisiones construye una lista plana desde las flags del NarrativeState  
Cierre: El diario agrupa por tipo y el editor declara el tipo al crear la aventura; los eventos unicos (casarse, tener un hijo) tienen su propia pista. Un solo contrato sobre AdventureScript (D-MIS-01).

### AV7 — Locomocion, carga y fisicas del jugador

Documento: §10.12 · Origen: NUEVA · 8 tareas · 16 j

**AV7-01 — Modos de locomocion con su velocidad, ruido y gasto de estamina**  
`3 j` · depende de: AV0-01 · flujo: WF-02  
Documento: §10.12.A  
Ya existe: Player.h, PlayerMotionData.h y CONTROLES-02 de Codex en vuelo (fase al caminar, reposo orientado)  
Cierre: Los 8 modos del documento (paseo, caminar, jog, sprint, agachado, esquiva, salto, natacion) con sus cifras, sobre MOVIMIENTO POR CELDAS con interpolacion visual: no se pasa a continuo (D-MOV-01/A10, confirmado el 28/09).

**AV7-02 — Inercia y derrape: frenada en seco, giro de 180º e impacto en multitudes**  
`1.5 j` · depende de: AV7-01 · flujo: WF-02  
Documento: §10.12.A4  
Ya existe: rebump/bump de Player en vuelo por otra sesion, sin inercia de sprint  
Cierre: Soltar el sprint a 2,6x derrapa 0,4 s levantando polvo y chocar con un civil le tira la cesta y le enfada.

**AV7-03 — Publicar estamina, ruido y carga al HUD por el canal acordado**  
`1 j` · depende de: AV7-01, AV1-07 · flujo: WF-02  
Documento: §10.12  
Ya existe: nada  
Cierre: Sprintear baja la barra visible, el ruido se indica y el icono de movilidad cambia con el peso.

**AV7-04 — Carga y peso: cuatro bandas con su efecto en movilidad, esquiva, ruido y daño lumbar**  
`2 j` · depende de: AV7-01 · flujo: WF-02, WF-06  
Documento: §10.12.B  
Ya existe: InventoryEngine con rarityRank y equipo por tier (BITACORA 2.13); ningun peso por objeto  
Cierre: Ligera/media/pesada/sobrecarga se comportan como dice el documento, incluido el Fat Roll y el daño al 150%.

**AV7-05 — Biomecanica por superficie: adoquin, barro con huellas 45 s, hielo y aceite, madera crujiente**  
`2.5 j` · depende de: AV7-01 · flujo: WF-02  
Documento: §10.12.C  
Ya existe: nada; las baldosas no llevan material  
Cierre: El barro deja huellas seguibles, el hielo pide una salvacion de Destreza DC 11 y la tabla mala suena en sigilo.

**AV7-08 — Pathfinding A* sobre celdas y movimiento con clic de raton con interpolacion visual**  
`3 j` · depende de: AV7-01 · flujo: WF-02  
Documento: §10.12.A  
Ya existe: tryMovePlayer(dx,dy) por celdas, RuntimeTile::hasCollision y CONTROLES-02 (fase al caminar, reposo orientado)  
Cierre: Clic en el suelo: el personaje traza la ruta A* sobre el grafo de celdas, la recorre con interpolacion suave (inercia y derrape visuales) y se detiene al llegar o al chocar. Las celdas siguen siendo la verdad para colision, IA, patrullas y editor (D-MOV-01/A10, confirmado el 28/09).

**AV7-06 — Vadeo y natacion con peso: chapoteo, velocidad al 70% y hundirse con placas**  
`1.5 j` · depende de: AV7-04 · flujo: WF-02  
Documento: §10.12.A8  
Ya existe: nada  
Cierre: Cruzar un arroyo suena y frena; nadar con 30 kg de placas hunde y asfixia salvo magia de respiracion.

**AV7-07 — Radio sonoro por calzado y armadura como dato consultable por la IA**  
`1.5 j` · depende de: AV7-04 · flujo: WF-02  
Documento: §10.1.B, §10.12.B  
Ya existe: nada  
Cierre: Placas corriendo = 8 m, botas de cuero agachado = 0 m, y la percepcion enemiga lee ese radio.

### AV8 — Sigilo, crimen, guardias, prision e identidad

Documento: §10.1, §10.2 · Origen: NUEVA · 10 tareas · 20.5 j

**AV8-03 — Catalogo de delitos por gravedad (menores, medios, mayores) y quien los presencia**  
`2 j` · depende de: AV0-02 · flujo: FS-01  
Documento: §10.1.A  
Ya existe: nada  
Cierre: Cada delito tiene su gravedad en datos y su lista de testigos posibles; allanar de dia y de noche no puntua igual.

**AV8-04 — Mecanica del cadaver: entidad fisica, arrastre, escondite, descubrimiento y panico**  
`2 j` · depende de: AV8-03 · flujo: FS-01  
Documento: §10.1.B  
Ya existe: nada  
Cierre: Un cuerpo se puede arrastrar a un callejon o al rio; si un civil lo ve, grita y corre a la campana.

**AV8-09 — Passing Score del disfraz: ropa, cosmetica, anatomia y magia de ilusion**  
`2 j` · depende de: AV3-01 · flujo: FS-03  
Documento: §10.2.A  
Ya existe: nada  
Cierre: El exito del disfraz sale de combinar las cuatro entradas y se reevalua a menos de 3 m.

**AV8-01 — Cono de vision de 120º con zona central 6 m, periferica 12 m y espalda ciega**  
`2.5 j` · depende de: AV7-07 · flujo: WF-02, FS-01  
Documento: §10.1.B  
Ya existe: nada; los NPC no perciben  
Cierre: El cono se ve en el editor y en debug; entrar en la espalda sin ruido no detecta.

**AV8-05 — Niveles de alerta 1-5: interrogatorio, detencion, campana y rastrillos, cazarrecompensas, proscripcion**  
`3 j` · depende de: AV8-04 · flujo: FS-01  
Documento: §10.1.C  
Ya existe: nada  
Cierre: Subir de nivel cambia la respuesta del mundo, hasta el cartel de «se busca vivo o muerto» y las patrullas hostiles.

**AV8-10 — Repercusiones culturales del disfraz por region e infiltracion en zonas segregadas**  
`1.5 j` · depende de: AV8-09, AV3-03 · flujo: FS-03  
Documento: §10.2.B  
Ya existe: nada  
Cierre: El mismo atuendo abre puertas en Boundington y provoca expulsion en un feudo ultraconservador.

**AV8-02 — Barra de sospecha progresiva en la zona periferica y deteccion instantanea en la central**  
`1.5 j` · depende de: AV8-01 · flujo: FS-01  
Documento: §10.1.B  
Ya existe: nada  
Cierre: Quedarse en la franja de 6-12 m llena la barra y acaba en alerta; a menos de 6 m es inmediato.

**AV8-06 — Intercepcion y tres salidas: rendirse, sobornar/engañar, resistirse armado**  
`2 j` · depende de: AV8-05 · flujo: FS-01  
Documento: §10.1.C  
Ya existe: opposed_degree resuelve la tirada enfrentada de carisma contra honestidad  
Cierre: Las tres ramas existen; resistirse y matar guardias sube a nivel 5.

**AV8-07 — Prision jugable: confiscacion, salto temporal, rutina carcelaria, trabajos forzados y tatuaje**  
`2.5 j` · depende de: AV8-06, AV3-02 · flujo: FS-02  
Documento: §10.1.C  
Ya existe: nada; el salto temporal necesita el reloj de mundo (AV1-06)  
Cierre: Entrar preso pierde el equipo en el cofre del alcaide, salta dias de calendario y deja marca corporal.

**AV8-08 — Fuga carcelaria: ganzua del carcelero borracho, pozo ciego, cloacas**  
`1.5 j` · depende de: AV8-07 · flujo: FS-02  
Documento: §10.1.C  
Ya existe: nada  
Cierre: Las dos vias de salida (condena cumplida y fuga) son jugables y la fuga deja estado de buscado.

### AV9 — IA viva: rutinas 24 h, patrullas, clima y escenografia

Documento: §10.13, §10.14 · Origen: NUEVA · 8 tareas · 19.5 j

**AV9-01 — Arboles de comportamiento y horario circadiano del ciudadano (06:00→23:00)**  
`3 j` · depende de: AV0-02 · flujo: FS-06  
Documento: §10.13.A  
Ya existe: EnemyBrain y patrullas en propiedades de instancia; ningun NPC civil con rutina  
Cierre: Un ciudadano se levanta, trabaja, almuerza, va a la taberna y se encierra a dormir, con el cono desactivado.

**AV9-05 — Objetos del mundo que hacen algo: puertas con destino, cofres con botin, mesas volcables, fuego**  
`3 j` · depende de: AV2-01 · flujo: WF-01, FS-07  
Documento: §10.14.C  
Ya existe: H3-07: MapEntityDocument y propiedades de instancia ya existen; el runtime no las ejecuta y los objetos son inertes  
Cierre: Una puerta lleva donde dice, un cofre da su botin y una mesa volcada sirve de cobertura.

**AV9-02 — Dialogo ambiental dinamico con memoria de sucesos recientes**  
`2 j` · depende de: AV9-01 · flujo: FS-06  
Documento: §10.13.A  
Ya existe: NarrativeEngine con beats; sin memoria de sucesos del mundo  
Cierre: Dos vecinos comentan el ahorcamiento de ayer porque ayer hubo un ahorcamiento.

**AV9-04 — Fauna urbana y pilluelos: perros que delatan, gatos, ratas y carteristas**  
`1.5 j` · depende de: AV9-01 · flujo: FS-06  
Documento: §10.13.C  
Ya existe: nada  
Cierre: Oler a carne fresca hace que un perro te siga ladrando y un pilluelo intenta las 5 monedas con su tirada pasiva.

**AV9-06 — Distritos con estetica y seguridad propias: noble, mercado, arrabales, muelles**  
`2 j` · depende de: AV9-05 · flujo: FS-07  
Documento: §10.14.A  
Ya existe: el generador de ciudad produce barrios (BITACORA Parte 3: los callejones derivan y se comen la muralla)  
Cierre: Cuatro barrios distinguibles por suelo, props, guardia y sonido, pintados desde datos de distrito.

**AV9-07 — Clima dinamico con su efecto en visibilidad, reflejos, charcos, sonido y reaccion social**  
`2.5 j` · depende de: AV1-06 · flujo: FS-07  
Documento: §10.14.B  
Ya existe: shaders de fog y lightmap; nada de clima  
Cierre: La lluvia abrillanta el adoquin y hace recoger los toldos; la niebla corta los conos de vision a la mitad.

**AV9-03 — Estados de alerta de la patrulla: verde, amarillo, naranja con enjambre y rojo con acoso coordinado**  
`3 j` · depende de: AV9-01, AV8-05 · flujo: FS-06  
Documento: §10.13.B  
Ya existe: nada  
Cierre: El silbato trae a las patrullas en 60 m en menos de 10 s y en rojo dos guardias cortan el paso por la calle paralela.

**AV9-08 — Tiendas con mercancia fisica en mostrador y hurto en tiempo real**  
`2.5 j` · depende de: AV9-05, AV6-12 · flujo: WF-03, FS-07  
Documento: §10.14.C  
Ya existe: ShopEngine (H1-02) y H3-04 resuelven la compra por menu, no por objeto colocado  
Cierre: El frasco esta sobre la madera; agarrarlo en sigilo lo quita del mundo y si la tendera se gira, grita.

### AV10 — Misiones organicas, investigacion y bibliotecas

Documento: §7, §10.10 · Origen: NUEVA · 6 tareas · 14.5 j

**AV10-01 — Contrato QuestDefinition, cargador y diario de misiones**  
`3 j` · depende de: AV0-02 · flujo: WF-07  
Documento: §7.2, §10.10  
Ya existe: H3-02: questIds en los documentos y QuestDefinition NO existe; cuelga al vacio  
Cierre: Abrir el diario muestra misiones activas, objetivos y estado, y los questIds de los niveles resuelven.

**AV10-02 — Tiempo de mundo REAL: reloj que avanza aunque el jugador este quieto, con fases dia/noche**  
`2 j` · depende de: AV1-06 · flujo: WF-03, WF-05, WF-07, FS-02, FS-06  
Documento: §10.9, §10.11  
Ya existe: updateNarrativeTimers(dtActivo) de F1 (zcode) congela en dialogo/combate/AFK; no hay calendario  
Cierre: El reloj avanza en tiempo real incluso con el jugador quieto (B6): fases dia/noche visibles, y los patrones de NPC, enemigos y los eventos mas peligrosos por zona y hora cambian con el. Rentas, cosechas y encargos del gremio reaccionan al mismo reloj, que sustituye al contador de pasos.

**AV10-04 — Investigacion en bibliotecas: estanterias interactivas, tiradas de Investigacion/Historia, paginas sueltas**  
`2.5 j` · depende de: AV10-01 · flujo: WF-07  
Documento: §7.1, §10.10  
Ya existe: nada  
Cierre: Registrar una estanteria da un diario censurado con una pista util (la contraseña de la bodega).

**AV10-03 — Rumores en la taberna y misiones sin iconos flotantes**  
`2 j` · depende de: AV10-01, AV9-02 · flujo: WF-07  
Documento: §7.2, §10.10  
Ya existe: nada  
Cierre: Una mision nace de escuchar y preguntar, no de un marcador en el mapa.

**AV10-05 — Cadenas de mision con repercusion regional y botin unico**  
`2 j` · depende de: AV10-04, AV3-03 · flujo: WF-07  
Documento: §7.2  
Ya existe: nada  
Cierre: Resolver la cadena cambia la reputacion de la region y entrega un objeto que no esta en otra parte.

**AV10-06 — Viajar entre naciones conectando el atlas con LevelTransition**  
`3 j` · depende de: AV10-02 · flujo: WF-07  
Documento: §10.3.C  
Ya existe: H3-01: 8 TMX de mapamundi y locations.json con 19 naciones; nadie los conecta y 13 desembarcos son inalcanzables  
Cierre: Desde Boundington se llega al mapamundi y a otra nacion, y se vuelve; conectividad.py en verde.

### AV11 — Red: MMO open world (solo expansion) e instancias privadas

Documento: §2 / §12 Fase 7 · Origen: doc · 5 tareas · 15 j

**AV11-01 — Protocolo de red y autoridad: que viaja, que se valida en servidor, como se reconcilia**  
`3 j` · depende de: AV0-04 · flujo: transversal  
Documento: §2  
Ya existe: nada de red en el motor  
Cierre: Un documento de protocolo con los mensajes por sistema y su autoridad, mas un eco cliente/servidor de prueba.

**AV11-02 — Servidor de mundo abierto con filtro de contenido de expansion**  
`4 j` · depende de: AV11-01 · flujo: transversal  
Documento: §2  
Ya existe: nada  
Cierre: Un cliente con un objeto custom desbalanceado no entra; el filtro rechaza y lo registra.

**AV11-03 — Servidor de salas privadas con Master y sus herramientas en vivo**  
`3 j` · depende de: AV11-01, AV6-15 · flujo: transversal  
Documento: §2, §11.5  
Ya existe: nada  
Cierre: Cuatro amigos en una sala con Master: el Master pausa, inyecta y tira en secreto, y los demas lo ven.

**AV11-05 — Antitrampas basico: validacion de tiradas y de movimientos en servidor**  
`2 j` · depende de: AV11-02 · flujo: transversal  
Documento: §2  
Ya existe: nada  
Cierre: Una tirada y un desplazamiento imposibles se detectan y se registran sin tirar la partida.

**AV11-04 — Economia unificada del MMO y persistencia de propiedades, rentas e impuestos**  
`3 j` · depende de: AV11-02, AV4-05 · flujo: transversal  
Documento: §2, §5  
Ya existe: nada  
Cierre: Comprar una tienda en el mundo abierto sigue siendo tuya al reconectar y la renta se cobra con el calendario.

### AV12 — Aceptacion, equilibrio y deuda de contenido

Documento: §1, §12 · Origen: NUEVA · 7 tareas · 13.5 j

**AV12-05 — Deuda de contenido aguas arriba: 202 hechizos huerfanos, 432 monstruos con vida 0, 59 clases sin hechizos**  
`3 j` · depende de: AV0-03 · flujo: transversal  
Documento: §12  
Ya existe: BITACORA 4.1 y 4.3; va por canon → cartas → motor, no se inventa en el motor  
Cierre: El contenido que el motor necesita existe aguas arriba o esta registrado como bug de contenido con dueño. Metodo fijado por Oriol el 28/09 (seccion C del registro de discrepancias): dar valores a los hechizos huerfanos usando las 6 clases referenciadas (Monje del Despertar, Senor de las Armas, Bardo de las 4 estaciones, Apostata, Furia de los Creadores, Mago chasqueador); rellenar la vida de los 432 monstruos y medir su dificultad con el harness 1 contra 1..n; adaptar las 59 clases sin hechizos al tono de las referenciadas multiplicando maneras de jugar; e incorporar los 28 lootIds a la economia. HECHO 14/10: medida en DEUDA_DE_CONTENIDO.md (claude_deuda01, Sonnet).

**AV12-01 — Pasar medir_alcance.py por cada fase: nada se cierra si no llega al jugador**  
`1.5 j` · depende de: AV0-06 · flujo: transversal  
Documento: §1, §12  
Ya existe: tools/medir_alcance.py mide por uso desde cada main(); la regla de cierre esta propuesta y sin decidir  
Cierre: Cada fase cerrada trae su medicion: el producto lo usa un ejecutable y la prueba define lo que prueba.

**AV12-02 — Partida de prueba guionizada que recorre los 7 flujos de workflows_juego/**  
`2 j` · depende de: AV12-01 · flujo: transversal  
Documento: §10  
Ya existe: nada  
Cierre: Un guion jugable de combate, movimiento, compra, moral, negocio, equipo y mision, con su resultado escrito.

**AV12-04 — Equilibrado de combate: pools de d6, coste de cartas, tiers y healthCap**  
`2 j` · depende de: AV5-09 · flujo: WF-01, WF-06  
Documento: §6, §10.6  
Ya existe: en «Esperando a Oriol»: la vida doblada por tier contra el healthCap actual (tier 5 = 128/192 contra tope 125)  
Cierre: Los cabos sueltos del reglamento Ed. 4 decididos y las cifras de tier coherentes con VIDA FIJA + equipo para ampliarla (D-COMB-07): el tope base no se dobla por tier. Incluye el critico por mitad de seises (A2) y la iniciativa 1d6+DES (A3).

**AV12-06 — Actualizar BITACORA, TABLERO y el documento de alcance con el estado real**  
`1 j` · depende de: AV12-02 · flujo: transversal  
Documento: §12  
Ya existe: el protocolo ya obliga a dejar huella por sesion  
Cierre: El documento de alcance y este plan dicen lo mismo que el codigo; las discrepancias estan registradas, no calladas. Incluye REESCRIBIR los apartados del documento de vision que Oriol ya resolvio el 28/09 (§10.6.A.4 exitos y critico, §10.6.A.1 iniciativa, §10.6.B caida, §10.7 EXP y atributos, §10.12.B peso y locomocion, §10.11 cobro del encargo, §11.4 renta semanal) con la regla decidida, y retirar las notas de revision que las acompañan.

**AV12-03 — Equilibrado economico: precios, rentas, impuestos y oro en circulacion**  
`2 j` · depende de: AV4-05 · flujo: WF-03, WF-05  
Documento: §5  
Ya existe: mercado real medido: pocion 30, eter 45, pan 8; 413 precios generados que nadie leia (H2-08)  
Cierre: Una tabla con el oro que entra y sale por semana de juego y ninguna via de enriquecimiento degenerada.

**AV12-07 — Harness de balanceo: dano maximo por clase en grupos de 1 contra 1..n enemigos (n hasta 10)**  
`2 j` · depende de: AV12-04 · flujo: WF-01  
Documento: §6, §12  
Ya existe: nada: 432 monstruos con vida 0 y 59 clases sin hechizos  
Cierre: Una herramienta que mide el dano maximo de cada clase referenciada contra 1..10 enemigos y publica la tabla. Con ella se rellena la vida de los monstruos y se decide su dificultad, como pidio Oriol en la seccion C del registro de discrepancias.

### AV13 — Combate tactico a lo Baldur's Gate 3: tablero, alcance, areas e IA

Documento: COMBATE_BG3.md, D0014 · Origen: NUEVA · 36 tareas · 55 j

**AV13-01 — Reglamento: el empate roza (1), la ayuda de dados en Tier 0-1 y los esbirros a DES 1-2**  
`0.5 j` · depende de: ninguna · flujo: WF-01  
Documento: D0014 §1-3  
Ya existe: DANO-02 (06/10) dice «empate o pierde -> 0»; el reglamento manda sobre el motor (canon -> cartas -> motor)  
Carril: **principal** (Claude) · zona: dndWeebCC-master/docs/Sistema_Cartas_Tiers.md  
Cierre: Sistema_Cartas_Tiers.md dice el roce, la ayuda de dados y el papel de los esbirros, con la fecha y la decision D0014. HECHO 14/10: Sistema_Cartas_Tiers.md §7.4.

**AV13-04 — Probabilidad exacta de una tirada enfrentada: ganar, roce y fallar, con ventaja, critico de dos 6 y pifia**  
`1.5 j` · depende de: ninguna · flujo: WF-01  
Documento: COMBATE_BG3.md §6  
Ya existe: DicePoolEngine::roll_pool tira; nadie calcula la probabilidad (por convolucion, sin simular)  
Carril: **paralelo** (opencode) · zona: NUEVOS include/RPG/ProbabilidadPool.h, src/RPG/ProbabilidadPool.cpp, tests/RPG/ProbabilidadPoolTests.cpp, cmake/ProbabilidadPool.cmake (+ su include en CMakeLists.txt)  
Cierre: Para pools 0..8 contra 0..8, con y sin ventaja/desventaja, la exacta coincide con 200 000 tiradas de roll_pool a menos de 0,5 puntos; sin GL; pruebas de los bordes (pool 0, un dado, dos 6). HECHO 14/10: RPG::ProbabilidadPool y el critico de dos 6 (claude_dados01, Opus).

**AV13-06 — Tablero de combate sin GL: recuadro de 15x15 casillas del nivel, transitables, ocupadas, distancias y linea de vision**  
`2 j` · depende de: ninguna · flujo: WF-01  
Documento: COMBATE_BG3.md §3  
Ya existe: BattleState no tiene posiciones (BattleState.h: turno Dragon Quest); el nivel ya sabe que casilla bloquea (RuntimeTile::hasCollision)  
Carril: **principal** (Claude) · zona: NUEVOS include/Game/TableroCombate.h, src/Game/TableroCombate.cpp + pruebas  
Cierre: Dado un nivel y quienes pelean, el tablero dice que casillas cuentan, quien esta en cada una, la distancia y si se ven; pruebas sin ventana. HECHO 14/10: Game::TableroCombate, 11x11, test_tablero_combate.

**AV13-07 — Ruta A* y casillas alcanzables con presupuesto, sin GL (sirve al combate y al clic de raton de AV7-08)**  
`1.5 j` · depende de: ninguna · flujo: WF-01  
Documento: COMBATE_BG3.md §3, §10.12.A  
Ya existe: nada: tryMovePlayer va casilla a casilla; AV7-08 pide el mismo A*  
Carril: **principal** (Claude (14/10: la necesitaba el movimiento en combate; trae pasa a AV14-20)) · zona: NUEVOS include/Game/RutaCasillas.h, src/Game/RutaCasillas.cpp, tests/Game/RutaCasillasTests.cpp, cmake/RutaCasillas.cmake (+ su include en CMakeLists.txt)  
Cierre: ruta(desde, hasta) y alcanzables(desde, presupuesto) con callbacks de transitable/coste/ocupada, 4 vecinos; pruebas de muro, rodeo, sin camino y presupuesto justo. HECHO 14/10: Game::RutaCasillas (Dijkstra en el recuadro), test_ruta_casillas 4/4.

**AV13-08 — Plantillas de area sobre casillas, sin GL: circulo, cono de 90 grados, linea y cuadrado**  
`1 j` · depende de: ninguna · flujo: WF-01  
Documento: COMBATE_BG3.md §5  
Ya existe: SkillTarget tiene ZONE_RADIUS sin geometria; ALL_ENEMIES alcanza a todos sin mirar donde estan  
Carril: **paralelo** (codex) · zona: NUEVOS include/Game/PlantillaArea.h, src/Game/PlantillaArea.cpp, tests/Game/PlantillaAreaTests.cpp, cmake/PlantillaArea.cmake (+ su include en CMakeLists.txt)  
Cierre: Cada plantilla devuelve sus casillas desde un origen y una direccion, con corte opcional por linea de vision (callback); pruebas con dibujos ASCII de cada forma. HECHO 14/10 (ChatGPT): PlantillaArea con 20 pruebas; integrada por Claude en claude_dialogos04.

**AV13-09 — Alcance y area de cada carta, aguas arriba: cuerpo a cuerpo, alcance N, a distancia, sobre si; y su plantilla**  
`2 j` · depende de: ninguna · flujo: WF-01  
Documento: COMBATE_BG3.md §5  
Ya existe: las cartas no dicen su alcance; el motor tendria que deducirlo de SkillTarget y las etiquetas  
Carril: **paralelo** (gemini) · zona: dndWeebCC-master (las cartas de las clases jugables); NO el motor  
Cierre: Las cartas de las clases jugables (las 4 de CLASES-03, el Senor de las armas, las SP) llevan alcance y area en sus datos, con un validador que avisa de las que faltan; propuesta de esquema revisada antes de rellenar.

**AV13-21 — Tipos de dano, resistencias y vulnerabilidades en los datos: cartas de las clases jugables y monstruos de Boundington**  
`1.5 j` · depende de: ninguna · flujo: WF-01  
Documento: COMBATE_BG3.md §8  
Ya existe: las cartas llevan etiquetas de elemento (Fuego, Sagrado...) que usan las particulas; no hay resistencias  
Carril: **paralelo** (gemini) · zona: dndWeebCC-master (cartas y fichas de monstruos); NO el motor  
Cierre: Cada carta de dano dice su tipo y los monstruos de Boundington sus resistencias y vulnerabilidades, con validador.

**AV13-30 — El critico atraviesa el bloqueo salvo bloqueo critico; la pifia del atacante da un ataque de oportunidad fisico**  
`1 j` · depende de: ninguna · flujo: WF-01  
Documento: D0015 §4 y §6  
Ya existe: El bloqueo para el golpe antes de tirar; la pifia del atacante solo hace 0  
Carril: **principal** (Claude) · zona: CartasTiradas (reaccionAntesDelGolpe, golpeDeArma), DanoEnfrentado  
Cierre: Con un bloqueo preparado el atacante tira: si saca critico, el que bloquea tira y solo para con critico; si el atacante pifia, el defensor le da un golpe de arma gratis (nunca un hechizo); pruebas de los cuatro casos. HECHO 14/10: CartasBloqueo.cpp; 3 pruebas.

**AV13-31 — La pifia en una salvacion: te entra el dano y el efecto, duplicado**  
`0.5 j` · depende de: ninguna · flujo: WF-01  
Documento: D0015 §10  
Ya existe: La pifia del objetivo da un dado mas de la carta  
Carril: **principal** (Claude) · zona: CartasTiradas (danoDeCarta, estados), DanoEnfrentado  
Cierre: Si el objetivo pifia la salvacion recibe el dano entero y el estado de la carta con el doble de rondas (o de intensidad); prueba. HECHO 14/10: RPG::dano::efectoDoble; la pifia ya no da un dado mas.

**AV13-33 — Morir: escena de noqueo o rescate, cicatriz y moral, Game Over y vuelta al ultimo guardado con una cicatriz mas**  
`1.5 j` · depende de: ninguna · flujo: WF-01  
Documento: D0015 §7 (sustituye a D-COMB-05)  
Ya existe: GameOver con Cargar y Titulo; la cicatriz no se guarda al cargar  
Carril: **principal** (Claude) · zona: GameSession (derrota), GameOverScreen, SaveGame (cicatrices), Application  
Cierre: Al caer se ve la escena, sale el Game Over y Cargar vuelve al ultimo guardado con la cicatriz anadida (y su moral); prueba de que la cicatriz sobrevive a la carga. HECHO 14/10: Game::EscenaNoqueo (noqueo o rescate, ENTER pasa la frase) antes del papiro; cicatriz fuera de la partida (Game::Cicatrices); si cae el heroe se pierde. Falta la moral (PO-38).

**AV13-34 — La compania de Rosita llega cuando Sandes, a 1/4 de vida o menos, te da un golpe mortal; scripteado para que pase siempre**  
`1 j` · depende de: ninguna · flujo: WF-01  
Documento: D0015 §13, canon §13.4  
Ya existe: ROSITA-01: llega cuando Sandes o tu bajais a un cuarto  
Carril: **principal** (Claude) · zona: aventuras de Boundington (Ocaso), Refuerzos, BattleState  
Cierre: Con Sandes a 1/4 o menos, su siguiente golpe te deja a las puertas y entra la compania; si no pasa solo, Sandes lo fuerza; la demo del Ocaso lo juega. HECHO 14/10: Refuerzo::golpeMortal, BattleState::ponGolpeMortal; ocaso regenerado.

**AV13-35 — Borrar el combate y las pruebas antiguas (sin mesa de cartas), midiendo antes que nadie los usa**  
`1.5 j` · depende de: ninguna · flujo: WF-01  
Documento: D0015 §11  
Ya existe: El camino sin mesa (etiquetas Exit/Fracas) sigue vivo por la prueba de nivel del editor y varias pruebas  
Carril: **principal** (Claude) · zona: BattleState (camino sin mesa), demos y pruebas viejas, tools/buscar_sobrante.py  
Cierre: La medida dice quien usa cada pieza; lo que no usa nadie se borra; la prueba de nivel del editor usa la mesa; ctest sigue verde. HECHO 14/10: medida en MEDIDA_COMBATE_VIEJO.md; siempre hay mesa (sin ficha, las basicas); borrados el camino sin mesa de BattleState, BattleMenuActions/BattleMenuState/TargetSelection, demo_battle y sus pruebas.

**AV13-36 — Las 950 cartas de ejemplo y las clases escondidas, convertidas en habilidades y clases de monstruos y enemigos**  
`2 j` · depende de: ninguna · flujo: WF-01  
Documento: D0015 §17  
Ya existe: 950 cartas de ejemplo (Llerba/Tomba) sin uso; clases escondidas  
Carril: **paralelo** (gemini) · zona: dndWeebCC-master (material de ejemplo Llerba/Tomba, clases escondidas); NO el motor  
Cierre: Cada carta util pasa al catalogo de monstruos con su rol; las clases escondidas son clases de enemigo; lista de lo que se descarta.

**AV13-02 — Roce en el motor: un golpe fisico empatado quita 1 y el registro dice «Roce»**  
`0.5 j` · depende de: AV13-01 · flujo: WF-01  
Documento: D0014 §1  
Ya existe: RPG::dano::dadosFisicos devuelve 0 con el empate; GolpesEnPantalla ya pinta «EMPATE»  
Carril: **principal** (Claude) · zona: include/RPG/DanoEnfrentado.h, tests de DANO-02, CartasTiradas.cpp (el registro)  
Cierre: Empatar quita 1 PV; las pruebas de DANO-02 fijan el roce y fallan con la regla vieja; el numero dice «Roce». HECHO 14/10: RPG::dano::roza (sesion 2026-10-14-claude-combate-bg3).

**AV13-10 — Colocar a todos al empezar: heroe y enemigos donde estaban, acompanantes y refuerzos en la libre mas cercana; el borde en tinta**  
`1 j` · depende de: AV13-06 · flujo: WF-01  
Documento: COMBATE_BG3.md §3  
Ya existe: la camara ya se acerca a los que pelean (COMBATE-01); los refuerzos se suman al bando sin sitio  
Carril: **principal** (Claude) · zona: GameSession (inicio del combate), ApplicationCinematica::combate, Refuerzos  
Cierre: Al empezar un combate cada uno tiene casilla, se ve el borde del tablero y un refuerzo entra andando por el. HECHO 14/10 (sin el refuerzo que entra andando, que espera a AV13-12): GameSession::montaTablero y el borde en tinta (Pergamino::SueloCombate).

**AV13-22 — Resistencia mitad, vulnerabilidad doble, inmunidad cero en el motor; el numero y el sonido salen del tipo**  
`1 j` · depende de: AV13-21 · flujo: WF-01  
Documento: COMBATE_BG3.md §8  
Ya existe: el dano no tiene tipo en el motor  
Carril: **principal** (Claude) · zona: CartasTiradas (dano), catalogo de monstruos  
Cierre: Un esqueleto vulnerable al contundente recibe el doble y el registro lo dice.

**AV13-03 — Esbirros a DES 1-2 y re-medir: te pegan el 30-35 % y el 1 contra 3 sigue en el 40 % (Oriol, 06/10)**  
`1 j` · depende de: AV13-02 · flujo: WF-01  
Documento: D0014 §3  
Ya existe: simulador_dificultad (GDD §21, Tier 1); el saqueador acierta el 14 % (analisis del 14/10)  
Carril: **principal** (Claude) · zona: assets/objects/boundington_enemigos.json, examples/simulador_dificultad.cpp  
Cierre: simulador_dificultad esbirros da el 30-35 % de aciertos y el ~40 % de caidas en el 1 contra 3; la tabla queda en la sesion. HECHO 14/10: saqueador 5 PV y DES 2 (D0014 §3).

**AV13-05 — Ayuda de dados: al apuntar, % de ganar, roce y fallar, dano previsto y vida que quedaria; nota la primera vez; opcion**  
`1.5 j` · depende de: AV13-02, AV13-04 · flujo: WF-01  
Documento: D0014 §2, COMBATE_BG3.md §6  
Ya existe: la barra de cartas (UI-BG3-01) ensena la carta senalada y la placa del enemigo; no hay prevision  
Carril: **principal** (Claude) · zona: HUD de combate (MesaCombateHud, BarraCombate), OptionsValues  
Cierre: Al senalar un objetivo se ven los tres % y el dano previsto; la primera vez sale la nota; en Tier 0-1 viene encendida y se apaga en Opciones.

**AV13-11 — Turno por iniciativa 1d6+DES, uno a uno y enemigos incluidos, con los retratos arriba (absorbe AV5-11)**  
`2 j` · depende de: AV13-10 · flujo: WF-01  
Documento: D-COMB-03, COMBATE_BG3.md §3  
Ya existe: AV5-11 lo pedia sin tablero; BattleState resuelve a todos los enemigos de golpe  
Carril: **principal** (Claude) · zona: BattleState, CartasEnCombate, EnemyBrain, BarraCombate (orden de turno)  
Cierre: Cada combatiente juega su turno en el orden de su tirada; la barra de arriba lo ensena y avanza; las pruebas de turno reescritas. HECHO 14/10 (los acompanantes aun actuan con el heroe): Game::OrdenDeTurno, BattleState::tiraIniciativa/pasaAlSiguiente, los retratos en ese orden con su tirada.

**AV13-12 — Moverse por casillas en tu turno: 6 casillas, las alcanzables resaltadas, la ruta con su coste y el aviso de ataque de oportunidad**  
`2.5 j` · depende de: AV13-07, AV13-11 · flujo: WF-01  
Documento: COMBATE_BG3.md §3, D0014 §7.1  
Ya existe: fuera de combate se anda casilla a casilla; en combate no se anda  
Carril: **principal** (Claude) · zona: GameSession (modo combate), Application (raton/teclado en combate), render del suelo  
Cierre: En tu turno ves hasta donde llegas, la ruta y lo que gasta; andas con clic o con las flechas; el movimiento que queda se ve y se puede gastar antes y despues de actuar. A MEDIAS 14/10: alcance y ruta en el suelo, clic y el heroe anda (SueloCombate, PasosEnRuta, mueveEnCombate); los rivales se acercan, se les ve andar y pegan al llegar. Faltan las flechas, ver lo que gasta y andar despues de actuar (pregunta B7).

**AV13-32 — La ayuda de dados dura hasta 4 dados en la caracteristica de ataque principal, y si bajas no vuelve**  
`1.5 j` · depende de: AV13-04, AV13-05 · flujo: WF-01  
Documento: D0015 §2  
Ya existe: AV13-05 la planeaba para Tier 0-1  
Carril: **principal** (Claude) · zona: AyudaDeDados (nuevo), SessionData (bandera), Opciones  
Cierre: Se apaga para siempre (bandera guardada) la primera vez que la caracteristica de ataque principal llega a 4 dados; siempre en el mismo punto; prueba con una ficha que sube y baja.

**AV13-13 — Alcance en juego: solo se apunta a quien esta a tiro; los validos resaltados; deducido mientras falten los datos de AV13-09 (absorbe AV5-04)**  
`1.5 j` · depende de: AV13-12 · flujo: WF-01  
Documento: COMBATE_BG3.md §5  
Ya existe: AV5-04 (seleccion de objetivos con cancelacion sin gasto); hoy cualquier carta llega a cualquiera  
Carril: **principal** (Claude) · zona: CartasEnCombate, BarraCombate, render de objetivos  
Cierre: Una carta cuerpo a cuerpo no se juega a 3 casillas y dice por que; cancelar no gasta; con los datos de AV13-09, manda el dato. HECHO 14/10: Game/AlcanceCartas (range -> 0/1/4/8/12 casillas, viendole mas alla de 1); solo se apunta a quien esta a tiro; la que no llega a nadie, bloqueada con su motivo; la carta grande dice el alcance. Las cifras, propuesta para Oriol.

**AV13-15 — Economia del turno a la vista: movimiento, accion, movimiento-carta, canalizacion, reaccion, «Terminar turno» y Correr/Esquivar/Retirarse/Ayudar**  
`2 j` · depende de: AV13-12 · flujo: WF-01  
Documento: COMBATE_BG3.md §4  
Ya existe: TurnoHeroe (GAMEMACHINE B7) lleva la cuenta; la accion principal cierra el turno  
Carril: **principal** (Claude) · zona: CartasEnCombate (TurnoHeroe), BarraCombate, MesaCombateHud  
Cierre: Lo que queda del turno se ve con iconos; el boton Terminar turno; las cuatro acciones basicas funcionan. Si Oriol confirma la propuesta de §4, la accion principal ya no cierra el turno. A MEDIAS 14/10: los pasos que quedan, en rombos, en la tira del turno; Correr/Esquivar/Retirarse/Ayudar esperan a la pregunta de §4. Oriol confirmo §4 el 14/10 (D0014 §8): la accion principal ya no cierra el turno.

**AV13-14 — Areas visibles en el suelo al apuntar: pillan a todos los de dentro, aliados incluidos**  
`1.5 j` · depende de: AV13-08, AV13-13 · flujo: WF-01  
Documento: COMBATE_BG3.md §5, D0014 §7.1  
Ya existe: ALL_ENEMIES alcanza a todos los enemigos y a ningun aliado  
Carril: **principal** (Claude) · zona: CartasJugar (objetivos de area), render del suelo  
Cierre: Una carta de area ensena su forma en el suelo, resalta a los que pilla y les afecta a todos al confirmar. A MEDIAS 14/10: el alcance de la carta senalada, pintado en el suelo (SueloCombate::tiro); faltan las plantillas de area (AV13-08) HECHO 14/10: con las plantillas de ChatGPT (AV13-08); Game::AreaDeCarta, CartasEnCombate::enElArea (aliados incluidos), el area en el suelo y en la tira. La forma se deduce hasta AV13-09.

**AV13-16 — Ataques de oportunidad al salir del alcance (salvo Retirarse) y la parada como reaccion**  
`1.5 j` · depende de: AV13-15 · flujo: WF-01  
Documento: COMBATE_BG3.md §7  
Ya existe: las reacciones preparadas saltan solas (H2-06), sin posiciones  
Carril: **principal** (Claude) · zona: CartasTiradas (reacciones), RutaCasillas (al salir del alcance)  
Cierre: Salir del lado de un enemigo le da un golpe (y se avisa antes en la ruta); Retirarse lo evita; la parada bloquea uno por ronda.

**AV13-17 — La posicion como dados: flanqueo da ventaja, la cobertura quita un dado, disparar con un enemigo al lado da desventaja**  
`1.5 j` · depende de: AV13-13 · flujo: WF-01  
Documento: COMBATE_BG3.md §7  
Ya existe: ventaja/desventaja ya existen en el pool y en los estados de las cartas  
Carril: **principal** (Claude) · zona: CartasTiradas (ventajas), TableroCombate  
Cierre: Las tres situaciones cambian el pool, el % de AV13-05 lo refleja y el registro dice por que.

**AV13-20 — Caido, no muerto: tiradas por turno, Ayudar levanta, tres fallos = fuera de combate; el heroe va a la consecuencia sin Game Over**  
`1.5 j` · depende de: AV13-15 · flujo: WF-01  
Documento: COMBATE_BG3.md §8, D-COMB-05  
Ya existe: AV5-07 (caida sin game over); hoy un aliado a 0 PV desaparece del combate  
Carril: **principal** (Claude) · zona: BattleState (bajas), CartasEnCombate (Ayudar), GameSession (caida del heroe)  
Cierre: Un aliado a 0 se queda caido y otro lo levanta; el heroe que cae del todo despierta en la consecuencia (AV5-07).

**AV13-23 — Canalizacion = concentracion: un sostenido a la vez y tirada de CON al recibir dano**  
`1 j` · depende de: AV13-15 · flujo: WF-01  
Documento: COMBATE_BG3.md §8  
Ya existe: la canalizacion es una de las acciones del turno (B7) y no se rompe  
Carril: **principal** (Claude) · zona: CartasEnCombate (canalizacion)  
Cierre: Recibir dano mientras canalizas pide CON contra el dano; si falla, el efecto se acaba y se ve.

**AV13-24 — Las cartas que esperaban posiciones: requiere_posiciones y la velocidad de movimiento**  
`1 j` · depende de: AV13-12, AV13-13 · flujo: WF-01  
Documento: COMBATE_BG3.md §2  
Ya existe: skills.json: 1 requiere_posiciones y 2 «pendiente: la velocidad de movimiento (sin posiciones en el combate)»  
Carril: **principal** (Claude) · zona: GanchosCarta, CartasJugar  
Cierre: Esas tres cartas hacen lo que dicen sobre el tablero y salen de la lista de pendientes.

**AV13-28 — La accion principal ya no cierra el turno: «Terminar turno» y el turno se acaba solo cuando no queda nada que gastar**  
`1.5 j` · depende de: AV13-15 · flujo: WF-01  
Documento: D0014 §8, COMBATE_BG3.md §4  
Ya existe: GAMEMACHINE B7: la accion principal cierra el turno  
Carril: **principal** (Claude) · zona: CartasEnCombate (TurnoHeroe), Application (confirmBattleCommand, pasaTurno), BarraCombate, MesaCombateHud  
Cierre: Atacas y despues andas; F o el boton «Terminar turno» lo cierra; sin nada que gastar se cierra solo; pruebas de la economia reescritas. HECHO 14/10: CartasEnCombate::ponTurnoBG3 (con tablero); se cierra con Terminar o sin nada que gastar.

**AV13-18 — Empujar: tirada enfrentada que mueve 1-2 casillas; si choca, roza**  
`1 j` · depende de: AV13-15, AV13-17 · flujo: WF-01  
Documento: COMBATE_BG3.md §4  
Ya existe: nada  
Carril: **principal** (Claude) · zona: CartasJugar (accion basica), TableroCombate  
Cierre: Empujar a un enemigo contra una pared le quita 1 y le deja al lado; contra el borde del tablero, no se sale.

**AV13-19 — IA enemiga por puntuacion sobre el tablero (casilla x carta x % de ganar), con roles: bruto, tirador, flanqueador y cobarde (amplia AV5-06)**  
`3 j` · depende de: AV13-04, AV13-12, AV13-13, AV13-17 · flujo: WF-01  
Documento: COMBATE_BG3.md §10  
Ya existe: EnemyBrain: agresivo/cobarde/emboscador sin tablero (AV5-06); Elemechs: «falta coordinacion»  
Carril: **principal** (Claude) · zona: EnemyBrain (la parte de combate), CartasEnCombate (mano del enemigo)  
Cierre: Los cuatro roles se distinguen jugando contra el mismo grupo; se reparten los objetivos; el cobarde huye al 25 %; pruebas sin ventana de que eligen lo esperado.

**AV13-25 — Superficies que se combinan (fuego, agua, grasa, hielo, humo) y barriles (amplia AV5-08)**  
`3 j` · depende de: AV13-14, AV13-22 · flujo: WF-01  
Documento: COMBATE_BG3.md §9  
Ya existe: AV5-08 y AV9-05 (objetos del mundo que hacen algo)  
Carril: **principal** (Claude) · zona: TableroCombate (superficies), render del suelo, particulas  
Cierre: Una carta de fuego sobre grasa la incendia; el agua moja y el rayo hace mas a los mojados; se ve y suena.

**AV13-29 — Los acompanantes con su propio turno en la iniciativa, y los juega el jugador (salvo los que su ficha diga que van solos)**  
`3 j` · depende de: AV13-11, AV13-28 · flujo: WF-01  
Documento: D0015 §1  
Ya existe: Venides, Xila y los demas actuan con IA justo despues del heroe (cerrarTurno)  
Carril: **principal** (Claude) · zona: OrdenDeTurno, BattleState, CartasEnCombate (acompanantes), BarraCombate (mano del que juega), ApplicationCinematica  
Cierre: Cada acompanante tira 1d6+DES, tiene su retrato y su turno; en su turno la barra ensena sus cartas y anda con el raton; la camara va con el; los de IA siguen solos. HECHO 14/10: OrdenDeTurno con todos los aliados; BattleState::aliadoEnTurno/loJuegaElJugador; la barra del acompanante (BarraAcompanante); su objeto "acompanante_<id>" en el World con su dibujo, andando; su puesto en la tira. Faltan: los de IA no andan.

**AV13-26 — Encuentros por presupuesto segun la dificultad, refuerzos con tope y jefes que se preparan (butchery)**  
`2 j` · depende de: AV13-19 · flujo: WF-01  
Documento: COMBATE_BG3.md §9  
Ya existe: ROSITA-01: refuerzos por vida; los encuentros son fijos en la aventura  
Carril: **principal** (Claude) · zona: aventuras (startBattle), Refuerzos  
Cierre: Un combate declara su presupuesto y la dificultad cambia cuantos y quienes; los refuerzos paran en su tope; Sandes bebe antes de pelear.

**AV13-27 — Escena de demo y simulador con tablero: el combate de Boundington jugado de punta a punta con posiciones**  
`1.5 j` · depende de: AV13-19 · flujo: WF-01  
Documento: COMBATE_BG3.md, AV12-07  
Ya existe: demo_boundington juega 58 escenas sin tablero; el simulador mide sin posiciones  
Carril: **principal** (Claude) · zona: examples/simulador_dificultad.cpp, demo_boundington  
Cierre: Una escena juega un combate con movimiento, area y reaccion; el simulador vuelve a dar los numeros de D0014 §3 con posiciones.

### AV14 — Gamefeel y pulido de lo jugado: golpes, sonido, andar, poses y menus

Documento: COMBATE_BG3.md §11-13, D0014 · Origen: NUEVA · 24 tareas · 43.5 j

**AV14-01 — Un solo evento de golpe (quien, a quien, cuanto, tipo, grado) del que cuelgan numero, particulas, sonido y marco**  
`1 j` · depende de: ninguna · flujo: WF-01  
Documento: COMBATE_BG3.md §11  
Ya existe: ApplicationCinematica::combate deduce los golpes comparando vidas entre fotogramas (COMBATE-01)  
Carril: **principal** (Claude) · zona: NUEVOS include/Game/GolpesCombate.h + src; BattleState y CartasTiradas lo emiten  
Cierre: La resolucion emite el golpe con su grado (fallo, roce, acierto, critico, pifia); los numeros de COMBATE-01 salen de el y no de comparar vidas. HECHO 14/10: Game::GolpeCombate y CampoDeBatalla; BattleState apunta cada golpe con el grado de su tirada (BattleStateGolpes.cpp); la pantalla los lee de ahi.

**AV14-04 — Banco de sonidos de combate, interfaz y pasos (CC0 o propios, con licencia): golpes por material, fallo, roce, critico, dados, cartas, botones, papel**  
`2 j` · depende de: ninguna · flujo: transversal  
Documento: COMBATE_BG3.md §11-12  
Ya existe: AudioDirector (H6-01) sin efectos de combate; ningun sonido de golpe en assets  
Carril: **paralelo** (qwen) · zona: NUEVOS assets/audio/combate/, assets/audio/ui/, assets/audio/pasos/, assets/audio/sonidos.json, assets/audio/LICENCIAS.md  
Cierre: Cada sonido de la lista de COMBATE_BG3.md §11 con 3 variantes, normalizado, en ogg, con su licencia y su id en sonidos.json; ninguno sin licencia clara.

**AV14-09 — Andar fluido con los frames de hoy: sin paron entre casillas, aceleracion y frenada, camara suave, pasos y polvo**  
`1.5 j` · depende de: ninguna · flujo: WF-02  
Documento: D0014 §7.3, COMBATE_BG3.md §12  
Ya existe: Player interpola 140 ms por casilla; Oriol: «la animacion de andar es horrible», «no puede ser que el movimiento no se vea fluido»  
Carril: **principal** (Claude) · zona: Render/Player, PlayerMotionData, camara, AudioDirector (pasos)  
Cierre: Manteniendo la tecla se anda sin tirones (medido: sin fotograma quieto entre casillas); la camara no salta; los pasos suenan y levantan polvo. A MEDIAS 14/10: sin paron entre casillas y camara a la par (sonda andar: 0 tirones, antes 83 de 117); faltan los pasos que suenan (AV14-05) y el polvo.

**AV14-10 — Pixel: ciclo de andar de 6 frames en 4 direcciones para los cuerpos base y los personajes de la historia**  
`3 j` · depende de: ninguna · flujo: WF-02  
Documento: COMBATE_BG3.md §13  
Ya existe: ciclo de 3 frames (paso, quieto, paso) por direccion, 64 px (razas_andando.json)  
Carril: **paralelo** (pixel) · zona: assets/textures/pixellab/lote_andar/ (PNG de PixelLab; los importa Claude)  
Cierre: Los PNG en el formato de orquestador/pixel.md: 64 px, pies en la fila 60, 6 frames por direccion.

**AV14-13 — Pixel: poses dignas (sentado en 4 direcciones, brindar, escribir, llamar, apoyado en la barra, dormir) para los personajes de la historia**  
`3 j` · depende de: ninguna · flujo: transversal  
Documento: D0014 §7.2, COMBATE_BG3.md §13  
Ya existe: los PNJ solo tienen 12 frames de andar  
Carril: **paralelo** (pixel) · zona: assets/textures/pixellab/lote_poses/  
Cierre: Las poses en el formato de orquestador/pixel.md para Venides, Verina, Edul, Sandes, Sho-Noco, la tabernera y los parroquianos de la Taberna Humilde.

**AV14-15 — Recorrido automatico de todos los botones de todos los menus, con informe de los que no responden o no se ven**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: D0014 §6  
Ya existe: CAPTURAS-UI (zcode) saca las pantallas; nadie pulsa sus botones  
Carril: **paralelo** (zcode) · zona: NUEVOS examples/recorrido_botones.cpp, cmake/RecorridoBotones.cmake, capturas_ui/botones/ (informe y capturas); sin tocar Application.*  
Cierre: Un ejecutable recorre titulo, creacion, pausa, opciones, inventario, tienda, dialogo y combate; pulsa cada boton; el informe lista lo que falla con su captura.

**AV14-18 — Pixel: iconos de los objetos de las tiendas de Boundington, de la economia del turno y de los tipos de dano**  
`2 j` · depende de: ninguna · flujo: transversal  
Documento: COMBATE_BG3.md §13  
Ya existe: iconos.json tiene los de estados y algunos objetos  
Carril: **paralelo** (pixel) · zona: assets/textures/pixellab/lote_iconos/  
Cierre: Un icono de 32 px por objeto a la venta en Boundington, por recurso del turno y por tipo de dano.

**AV14-19 — El lag: compilar en Release en el Mac, medir arranque y creador, y hacer los retratos del creador por partes**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: Oriol 14/10: «se lagea al principio y en el creador»  
Ya existe: el build del Mac es -O0 (CMAKE_BUILD_TYPE vacio); en la VM: arranque 180 ms, primer fotograma del creador ~300 ms  
Carril: **principal** (Claude) · zona: Personajes/creador (retratos), CMakePresets  
Cierre: Con Release, ninguna pantalla tarda mas de 100 ms en su primer fotograma en el Mac.

**AV14-20 — Validador de puertas: cada puerta lleva a un nivel que existe, y la casilla a la que llega se pisa y tiene puerta de vuelta**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: MAPA-01 (el rumbo va por las puertas), D0014 §6 (que todo vaya bien)  
Ya existe: validar_todo valida catalogos y aventuras; nadie mira que las puertas lleven a un sitio pisable  
Carril: **paralelo** (trae) · zona: NUEVOS tools/validar_puertas.py y tools/test_validar_puertas.py; una llamada en tools/validar_todo.py  
Cierre: El validador recorre assets/levels/*.json y lista cada puerta rota, cada llegada en pared y cada nivel sin vuelta; pruebas con niveles de mentira; enganchado a validar_todo como aviso. HECHO 14/10: tools/validar_puertas.py (claude_puertas02, Haiku).

**AV14-21 — Prologo: los tres amigos con tunica y sin capucha; despues se la ponen y salen andando de la habitacion**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: D0015 §12  
Ya existe: Los tres amigos de 757 llevan capucha morada desde el principio  
Carril: **principal** (Claude) · zona: assets/cinematicas/cinematicas.json, aventura del prologo, sprites de los amigos  
Cierre: La cinematica los ensena sin capucha, el gesto de ponerla y la salida andando; si falta el sprite, se pide a Pixel.

**AV14-22 — El diario de misiones completo: apartados, decisiones tomadas, sus consecuencias y lo que se te pide (H3-02 + MISIONES-01 juntos)**  
`3 j` · depende de: ninguna · flujo: transversal  
Documento: D0015 §16  
Ya existe: Dos mitades: la ficha quests.json (H3-02) y la vista de las flags (MISIONES-01)  
Carril: **principal** (Claude) · zona: DiarioMisiones, QuestDefinition (H3-02), MISIONES-01, PaginaCuaderno  
Cierre: Una sola fuente; cada mision ensena sus apartados, las decisiones que tomaste y su consecuencia, y lo que falta; pruebas con Boundington. HECHO 14/10: claude_diario01 (Opus 2).

**AV14-23 — D-MOR-09 y D-MIS-03: la reputacion por faccion en el comercio y el HUD; el rumor es un beat que salta al hablar**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: D0015 §15  
Ya existe: factionRep existe y nadie lo lee; los rumores como texto suelto  
Carril: **principal** (Claude) · zona: Comercio, HUD (reputacion), aventuras (rumores)  
Cierre: El precio depende de la reputacion con su faccion y el HUD la ensena; los rumores son beats con TriggerType::Talk. HECHO 14/10: claude_reputacion01 (Opus 2).

**AV14-24 — Plan de integracion de lo terminado que nadie usa (las 13 piezas de H8-06), en el Gantt**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: D0015 §18  
Ya existe: 13 piezas, 7.645 lineas, sin consumidor  
Carril: **principal** (Claude) · zona: planificacion (Gantt), tools/buscar_sobrante.py  
Cierre: Cada pieza con su tarea (enchufar, rehacer o borrar) y su fase; ninguna sin destino. HECHO 14/10: INVENTARIO_SOBRANTE.md (Haiku): 11 piezas.

**AV14-02 — El golpe integrado en la hoja: el marco del pergamino se tine y tiembla, numero en tinta con Roce/Critico/Fallo, parpadeo, retroceso y hit-stop**  
`2 j` · depende de: AV14-01 · flujo: WF-01  
Documento: D0014 §5, COMBATE_BG3.md §11  
Ya existe: COMBATE-01: numeros y bandas rojas superpuestas; Oriol: «el recuadro rojo podria ser como el marco de la hoja»  
Carril: **principal** (Claude) · zona: GolpesEnPantalla, PergaminoGL (marco), Player/Enemy (parpadeo y retroceso)  
Cierre: Cuando te pegan se tine y tiembla el marco de la hoja (ni una banda superpuesta); cada grado tiene su numero; el cuerpo parpadea y retrocede; el critico para 70 ms. HECHO 14/10: Game::EfectosDeGolpe (parpadeo, retroceso, esquiva, pausa de 70 ms); la hoja tenida y temblando con la barra fantasma (HudRetales); sin bandas.

**AV14-03 — Particulas de impacto por tipo de dano (astillas, chispas, brasas, sangre) con las familias de CANON_PARTICULAS**  
`1.5 j` · depende de: AV14-01 · flujo: WF-01  
Documento: COMBATE_BG3.md §11  
Ya existe: PART-02d: las particulas de la carta jugada; ninguna en el impacto  
Carril: **principal** (Claude) · zona: ParticulasDeCartas / EfectosDeCartas, particulas_cartas.json  
Cierre: Cada golpe que entra salpica las de su tipo sobre quien lo recibe; el fallo, ninguna; el critico, el doble. A MEDIAS 14/10: por grado (polvo, chispas, chispas y sangre); faltan las de material (astillas, brasas).

**AV14-05 — Que suene: cada golpe, fallo, roce y critico, los dados, las cartas, los pasos y los botones, con variaciones**  
`1.5 j` · depende de: AV14-01, AV14-04 · flujo: WF-01  
Documento: COMBATE_BG3.md §11  
Ya existe: AudioDirector (qwen) reparte musica y efectos por bus  
Carril: **principal** (Claude) · zona: ApplicationCinematica / Application (audio), AudioDirector  
Cierre: Un combate de prueba suena entero sin repetir el mismo golpe dos veces seguidas; el volumen de Opciones manda.

**AV14-06 — Musica de combate en bucle de energia constante, sin solista (Elemechs 16:18), con golpe de entrada y salida**  
`1 j` · depende de: AV14-04 · flujo: transversal  
Documento: COMBATE_BG3.md §11  
Ya existe: nada de musica de combate  
Carril: **paralelo** (qwen) · zona: NUEVOS assets/audio/musica/ (+ su entrada en sonidos.json y LICENCIAS.md)  
Cierre: Un bucle de 60-120 s que empalma sin corte, el golpe de entrada y el cierre, con licencia.

**AV14-07 — Los dados del pool ruedan en el centro: los 6 brillan, los 5 a medias, contra los del defensor (absorbe AV1-04)**  
`2 j` · depende de: AV14-01 · flujo: WF-01  
Documento: COMBATE_BG3.md §11, AV1-04  
Ya existe: AV1-04 lo pedia; DiceRollWidget existe con pruebas y no sale en el combate en pergamino  
Carril: **principal** (Claude) · zona: DiceRollWidget, HUD de combate  
Cierre: Cada tirada enfrentada ensena los dos pools rodando y el resultado leido de un vistazo; se puede acelerar.

**AV14-08 — La camara sigue al que actua; velocidad del combate x1/x1,5/x2 y saltar animaciones (accesibilidad)**  
`1.5 j` · depende de: AV14-01 · flujo: WF-01  
Documento: COMBATE_BG3.md §11  
Ya existe: COMBATE-01: zoom a la pelea al empezar  
Carril: **principal** (Claude) · zona: ApplicationCinematica::combate (camara), OptionsValues  
Cierre: La camara va al que tiene el turno; la opcion acelera todo a la vez (animacion, sonido y dano). A MEDIAS 14/10: la camara va con quien juega (60 % hacia el); falta la velocidad x1/x1,5/x2 en Opciones.

**AV14-11 — Integrar el ciclo de 6 frames: hojas, generador y animacion (con el paso de 3 frames para lo que aun no tenga 6)**  
`1.5 j` · depende de: AV14-10, AV14-09 · flujo: WF-02  
Documento: COMBATE_BG3.md §12  
Ya existe: HojaPersonaje::animar reparte 3 frames por direccion  
Carril: **principal** (Claude) · zona: HojaPersonaje, tools/gen_personajes.py, tools/pixellab/importar_actores.py  
Cierre: Los cuerpos base y los personajes de la historia andan con 6 frames; lo demas sigue con 3 sin romperse.

**AV14-14 — Sentarse y poses en el motor: punto de asiento en sillas y bancos, ordenes «sienta» y «pose» en las cinematicas, PNJ sentados en la taberna**  
`2 j` · depende de: AV14-13 · flujo: transversal  
Documento: D0014 §7.2  
Ya existe: las cinematicas mueven y orientan; nadie se sienta (Oriol 14/10: «haz sentarse a los personajes»)  
Carril: **principal** (Claude) · zona: Cinematica (ordenes), niveles (punto de asiento), HojaPersonaje (poses)  
Cierre: En el brindis de 756 los cuatro estan sentados y Venides brinda; en la Taberna Humilde los parroquianos se sientan y beben.

**AV14-17 — Tienda rehecha con lo que diga Oriol, con los iconos de cada objeto**  
`2 j` · depende de: AV14-18 · flujo: transversal  
Documento: Oriol 14/10: «los HUDs de comprar tienda aun estan fatal»  
Ya existe: TIENDA-02 y TIENDA-04 (sin folio); falta saber que es lo que mas molesta  
Carril: **principal** (Claude) · zona: TiendaHoja, PaginaCuaderno  
Cierre: Oriol da la tienda por buena.

**AV14-12 — Pixel: las prendas con el ciclo de 6 frames, por lotes**  
`4 j` · depende de: AV14-11 · flujo: WF-02  
Documento: COMBATE_BG3.md §12  
Ya existe: las prendas tienen el ciclo de 3  
Carril: **paralelo** (pixel) · zona: assets/textures/pixellab/lote_andar_prendas/  
Cierre: Cada lote entregado y importado; el heroe vestido anda con 6 frames.

**AV14-16 — Arreglar los menus: estado encima y pulsado en cada boton, sonido de papel, transiciones sin tirones**  
`2 j` · depende de: AV14-15, AV14-05 · flujo: transversal  
Documento: D0014 §6  
Ya existe: TITULO-01 (14/10) avisa al elegir; muchos botones no cambian al pasar por encima  
Carril: **principal** (Claude) · zona: HUD Pergamino (botones), Application (menus)  
Cierre: El recorrido de AV14-15 sale limpio y cada boton responde a la vista y al oido.

### AV15 — Usabilidad y la ciudad: moverse, orientarse, menus coherentes y accesibles

Documento: USABILIDAD_UOC.md, D0015 · Origen: NUEVA · 7 tareas · 13.5 j

**AV15-01 — Evaluacion heuristica (Nielsen + Norman) y recorrido cognitivo de las cuatro tareas del jugador nuevo**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: USABILIDAD_UOC.md §3  
Ya existe: Nadie ha evaluado el juego con metodo  
Carril: **principal** (Claude) · zona: planificacion (informe), capturas  
Cierre: Informe por pantalla con captura, principio incumplido, gravedad y propuesta; las cuatro tareas recorridas paso a paso.

**AV15-02 — Los sitios por su nombre: carteles con nombre y emblema sobre la puerta («El lingote de oro» donde esta Luisarda), el mismo en minimapa, tienda y dialogo**  
`2 j` · depende de: ninguna · flujo: transversal  
Documento: D0015 §14, USABILIDAD_UOC.md §2  
Ya existe: Los sitios se llaman por su funcion  
Carril: **principal** (Claude) · zona: assets/levels (ciudad), assets/objects (carteles), aventuras (nombres), minimapa  
Cierre: Cada tienda y servicio tiene nombre propio y emblema reconocible en 5 s, igual en todos los sitios; lista de nombres revisada por Oriol. HECHO 14/10: Game::Lugares lee lugares.json (ChatGPT, PO-39/40/41); cartel con emblema, nombre y funcion sobre cada puerta cercana, nombre en el aviso, en el minimapa (emblemas y Cerca:), rotulo al entrar y cabecera de la tienda. Faltan emblemas propios para 11 de 17 sitios (arte).

**AV15-06 — La guia de estilo de Onegai: paleta, letras, reticula de 8 px, botones con sus 5 estados, iconos y voz**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: USABILIDAD_UOC.md §2  
Ya existe: Tema.h tiene los colores; nada escrito para el equipo  
Carril: **principal** (Claude) · zona: NUEVO planificacion_alcance_y_vision/GUIA_DE_ESTILO.md, HUD/Pergamino/Tema.h  
Cierre: La guia escrita a partir de Tema.h y lo que hay; cada pantalla nueva se revisa contra ella.

**AV15-03 — Quitar ruido de la ciudad: calles principales continuas, fuera los objetos que tapan o cortan el paso, lo que tapa al jugador translucido**  
`3 j` · depende de: AV15-01 · flujo: transversal  
Documento: D0015 §14, USABILIDAD_UOC.md §2  
Ya existe: Muchos objetos tapan la vista e impiden ir recto (Oriol, 14/10)  
Carril: **principal** (Claude) · zona: assets/levels (ciudad), BloquesNivel (transparencia)  
Cierre: Inventario de obstaculos; de la plaza a cada tienda se va recto o casi (desvio medido < 25 %); ningun choque con algo que parecia andable. HECHO 14-15/10: claude_ciudad01 y claude_ciudad02 (297 -> 190 objetos); sigue en AV16-09.

**AV15-04 — El minimapa como mapa: lugares con nombre e icono, hitos y filtros (tiendas, servicios, casas)**  
`1.5 j` · depende de: AV15-02 · flujo: transversal  
Documento: USABILIDAD_UOC.md §2  
Ya existe: El minimapa marca el objetivo y la zona  
Carril: **principal** (Claude) · zona: MinimapaTinta, HudRetales  
Cierre: Al abrir el mapa se filtra por tipo; los nombres salen; el jugador se reubica en menos de 10 s. HECHO 15/10: mapa grande con M, chinchetas, mision seguida y flecha (claude_mapa01).

**AV15-05 — Los menus viejos al pergamino: comprar casas, las termas y el resto, con confirmacion en lo caro e irreversible**  
`3 j` · depende de: AV15-06 · flujo: transversal  
Documento: D0015 §14, USABILIDAD_UOC.md §2  
Ya existe: Varios menus con la estetica antigua  
Carril: **principal** (Claude) · zona: Negocio (casas), termas, y cada pantalla con la estetica vieja  
Cierre: Inventario de pantallas; ninguna fuera de la guia de estilo; comprar una casa pide resumen y confirmacion; lo comprado por error se revende.

**AV15-07 — Legibilidad y accesibilidad: contraste 4,5:1 sobre el pergamino, lineas de 45-75, nada solo por color, texto escalable, menos temblor si se pide**  
`1.5 j` · depende de: AV15-06 · flujo: transversal  
Documento: USABILIDAD_UOC.md §2  
Ya existe: Contraste sin medir; el color solo distingue algunas marcas  
Carril: **principal** (Claude) · zona: DialogoHoja, Opciones, Tema  
Cierre: Contraste medido en cada texto; trama ademas de color en el suelo; tamano de texto en Opciones; animaciones reducidas apaga el temblor nuevo.

### AV16 — Pulido de lo jugado el 15/10: creacion, capas, salidas y puertas, suelo, combate

Documento: PLAN_15-10.md §2, D0018 · Origen: NUEVA · 16 tareas · 20.2 j

**AV16-00 — Commitear el arbol de hoy (los 17 paquetes del 14 y 15/10 aplicados) para que las otras IAs partan de el**  
`0.2 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §0, PO-04  
Ya existe: Todo lo aplicado esta sin commitear: una rama nueva saldria del 08/10  
Carril: **principal** (Oriol) · zona: MotorGraphico-main (git)  
Cierre: Un commit con todo; git status limpio salvo build-release/ y out/.

**AV16-01 — Creacion: el Espectro (y cualquier raza no jugable) no sale en la lista ni en la busqueda**  
`0.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.1  
Ya existe: Oriol lo ha pedido varias veces; la lista ensena las 43 razas, Espectros incluidos  
Carril: **principal** (Claude) · zona: Game/CreacionPersonaje*, PaginasCreacion, catalogo de razas (jugable)  
Cierre: Las razas no jugables no se pueden elegir; una prueba lo fija y falla si vuelve.

**AV16-02 — Creacion: los atributos empiezan todos a 0; ni la raza ni el kit suman; solo se destacan los principales de la clase**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.2  
Ya existe: La hoja ensena «raza +2» y el kit «+1 INT»; Oriol: «han de estar tots en 0, no sumis coses»  
Carril: **principal** (Claude) · zona: Game/CreacionPersonaje*, PaginasCreacion (AL EMPEZAR, KIT DEL COFRE)  
Cierre: AL EMPEZAR ensena 0 en todo con los principales de la clase resaltados; el kit no dice que sume; la ficha en partida igual; pruebas.

**AV16-03 — Ropa que no cuadra con el cuerpo: auditoria automatica de cada prenda x cuerpo x talla x rumbo y arreglo (o fuera de la lista)**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.3  
Ya existe: La tunica de mago (PixelLab) con talla Normal no encaja (captura 15/10 10:24)  
Carril: **principal** (Claude) · zona: Game/Personajes (MontajePersonajes), tools/gen_personajes.py, assets/textures/personajes  
Cierre: Informe con cada combinacion; ninguna prenda ofrecida se sale del cuerpo; las que no tienen arreglo no se ofrecen y van a Pixel.

**AV16-04 — Orden de dibujo: lo plano del suelo (alfombras, felpudos) siempre debajo de los personajes; el barquero delante de su barca**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.4  
Ya existe: La alfombra de la taberna y del banco se pinta encima del jugador; Sho-noco queda detras de la barca  
Carril: **principal** (Claude) · zona: Render (orden de dibujo), catalogo de objetos (capa)  
Cierre: Regla de capas escrita (suelo, plano, de pie, encima); cada objeto con su capa; capturas de las tres escenas; pruebas de orden.

**AV16-05 — El jugador anda en las cinematicas tan fluido como Sho-noco**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.5  
Ya existe: En cinematicas el jugador va a tirones; Sho-noco se mueve fluido  
Carril: **principal** (Claude) · zona: ApplicationCinematica, Render/Player  
Cierre: El jugador usa la misma interpolacion y el mismo ciclo que Sho-noco; tira de capturas o video.

**AV16-06 — Salidas que se ven: la salida a otro mapa marcada en el suelo con su cartel «Hacia X»; dentro, «Salir a la calle»; nombres de todas las puertas revisados**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.6, PO-47  
Ya existe: Las salidas del borde no se ven; dentro de los edificios el aviso dice «Entrar en Salida»  
Carril: **principal** (Claude) · zona: HUD (avisos, CartelesPuertas, minimapa), lugares.json, niveles (nombres de puertas)  
Cierre: Cada salida tiene marca, cartel y nombre del destino; ninguna puerta se llama «Salida»; tool que lista los nombres.

**AV16-07 — Puertas que se ven: al acercarte, la puerta se resalta aunque la tape un tejado, y el cartel apunta a ella**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.7  
Ya existe: La puerta del Lingote de Oro no se ve desde la calle  
Carril: **principal** (Claude) · zona: Render (contorno), HUD  
Cierre: A 6 casillas la puerta tiene contorno; si la tapa algo, se ve a traves; capturas del Lingote de Oro y la Caja Firme.

**AV16-10 — Suelo mas sencillo y con transiciones: menos variantes y sin rayado; bordes suaves entre materiales (dual-grid)**  
`2 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.10  
Ya existe: El suelo tiene mas detalle que las casas; cambios de material en linea recta  
Carril: **principal** (Claude) · zona: Render (terreno), assets/textures/terreno*, tools de tiles  
Cierre: Capturas antes/despues de la plaza, el puente y el prado; mismo rendimiento.

**AV16-11 — Combate: el turno del rival se entiende (se esconde tu mano, la camara va al que actua, cartel «Turno de X») y la tira de iniciativa se lee mejor**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.11  
Ya existe: Oriol: la tira y el turno del rival «a mejorar»  
Carril: **principal** (Claude) · zona: HUD de combate (MesaCombateHud, BarraCombate), camara de combate  
Cierre: Capturas del turno propio y del rival; nadie duda de a quien le toca.

**AV16-12 — Combate: menos pasos por turno (6 -> 4 casillas, DES 3+ suma 1) y re-medir**  
`0.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.12  
Ya existe: Oriol: «demasiados pasos en general»  
Carril: **principal** (Claude) · zona: Game/TableroCombate (movimiento), simulador_dificultad  
Cierre: 4 casillas de base; simulador re-medido; pruebas.

**AV16-13 — El fundido no molesta: solo se funde lo que tapa al jugador, no lo que tiene al lado**  
`0.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.13  
Ya existe: El 40 % de fundido molesta cuando el objeto te corta el paso  
Carril: **principal** (Claude) · zona: Render/OcclusionRules  
Cierre: Regla y prueba; capturas.

**AV16-14 — Estados nuevos (PO-37): quemado, congelado, aturdido y silenciado, y dados extra**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.14, D0018  
Ya existe: 16 estados; faltan estos cuatro  
Carril: **principal** (Claude) · zona: RPG (estados), CartasEnCombate, iconos  
Cierre: Cada estado con su efecto, su duracion por potencia, su icono y su prueba.

**AV16-15 — Los mapas en Excel: cada nivel como una hoja de casillas con colores y codigos que Oriol edita, y vuelta al juego**  
`2 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §2.15 (Oriol, 15/10 10:38)  
Ya existe: Para hablar de un mapa solo hay capturas  
Carril: **principal** (Claude) · zona: NUEVOS tools/nivel_a_excel.py, tools/excel_a_nivel.py, planificacion (MAPAS_EN_EXCEL.md)  
Cierre: ciudad_centro ida y vuelta sin perder nada; Oriol mueve una casa o un cofre en el Excel y sale en el juego.

**AV16-08 — Cada puerta, un sitio distinto: los segundos (Boutique de Madame, Taberna Rica, Caja Firme, mercado) con nombre e interior propios**  
`1 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §2.8  
Ya existe: Hay dos Boutique de Madame, dos Taberna Rica y dos Caja Firme  
Carril: **paralelo** (ChatGPT) · zona: assets/catalogs/lugares.json, interiores _sur nuevos (copias), BANDEJA (nombres)  
Cierre: Ningun nombre repetido en lugares.json; nombres nuevos aprobados por Oriol en la BANDEJA; validador que lo comprueba.

**AV16-09 — Boundington con amplitud: reubicar los edificios del centro (calles anchas, puertas a la calle, paso plaza-iglesia) y regenerar**  
`3 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §2.9, PO-44  
Ya existe: La ciudad sigue apretada; puertas escondidas; la iglesia a 44 % de desvio  
Carril: **paralelo** (Qwen) · zona: tools/gen_ciudad.py, tools/gen_ciudad_deco.py, tools/despejar_calles.py, assets/levels/ciudad_centro.json, assets/maps/ciudad_centro.tmx  
Cierre: medir_calles --comprobar en verde, ninguna puerta escondida (cada puerta mira a una calle de 2 casillas o mas), desvio < 25 % en todas; capturas antes/despues.

### AV17 — La historia con teatro: escenas con personajes, segundas oportunidades, el dia 3 y el ocaso en llamas

Documento: PLAN_15-10.md §3, D0018 · Origen: NUEVA · 12 tareas · 19.5 j

**AV17-09 — Escribir las 10 deidades que faltan (PO-45)**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §3.9, DEUDA_DE_CONTENIDO.md  
Ya existe: 100 hechizos cuelgan de 10 deidades que no existen  
Carril: **principal** (Oriol) · zona: assets/catalogs (deidades)  
Cierre: Las 10 fichas escritas por Oriol.

**AV17-10 — Ordenes nuevas de cinematica para Z.AI: trepar y bajar, arder, tumbado, patrulla, emocion (bocadillo)**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §3.10  
Ya existe: Las cinematicas mueven, giran y hablan  
Carril: **principal** (Claude) · zona: Game/Cinematica, FORMATO_AVENTURAS.md  
Cierre: Ordenes documentadas con ejemplo y prueba.

**AV17-11 — Fuego y humo en el mundo: casas en llamas y celdas que queman**  
`1.5 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §3.11  
Ya existe: Hay particulas de golpe y braseros  
Carril: **principal** (Claude) · zona: Render (particulas), objetos (fuego)  
Cierre: Un objeto «fuego» con luz, humo y dano al pisar; capturas.

**AV17-01 — Segundas oportunidades: ninguna tirada fallada cierra una via; el PNJ ofrece otra (un favor) y, si vuelves a fallar, una de explorar**  
`2 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §3.1  
Ya existe: Fallar con Luisarda o con Aigren cierra la via; invitar a cenar no tiene alternativa  
Carril: **paralelo** (Z.AI) · zona: assets/adventures/boundington_*.json  
Cierre: Tabla de cada tirada de Boundington con sus tres salidas; ninguna mision queda imposible; validador de aventuras en verde.

**AV17-02 — Pasar el tiempo: esperar a mediodia, tarde o noche en tabernas y posada; el dia 3 empieza por la manana**  
`1.5 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §3.2  
Ya existe: Solo se puede esperar a la noche; el dia 3 empieza ya de noche  
Carril: **paralelo** (Z.AI) · zona: assets/adventures/boundington_*.json (beats de esperar)  
Cierre: Las cuatro franjas elegibles; el dia 3 deja acabar lo pendiente antes del ocaso.

**AV17-03 — La cripta bajo la iglesia jugable: sectarios, Edul y Verina, enganar o unirse, escuchar, escapar**  
`3 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §3.3  
Ya existe: Es un texto sin personajes  
Carril: **paralelo** (Z.AI) · zona: assets/adventures/boundington_primer_dia.json, assets/levels/cripta_*.json (poblar)  
Cierre: La escena tiene personajes que se mueven y al menos 4 tiradas con salidas distintas; unirse abre una rama.

**AV17-04 — El castillo con personajes y cinematica**  
`2 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §3.4  
Ya existe: La historia del castillo es estatica  
Carril: **paralelo** (Z.AI) · zona: assets/adventures (castillo), assets/levels (castillo, poblar)  
Cierre: El capitan y la guardia estan en la sala; entrada y salida con movimiento.

**AV17-05 — El gato en el arbol: cinematica de trepar a un arbol cercano y bajar con el gato**  
`1 j` · depende de: AV17-10 · flujo: transversal  
Documento: PLAN_15-10.md §3.5  
Ya existe: Solo un texto  
Carril: **paralelo** (Z.AI) · zona: assets/adventures (el gato)  
Cierre: El heroe anda al arbol, sube, baja con el gato; con la pose de sustitucion hasta que Pixel dibuje trepar.

**AV17-08 — Teatralidad: guia de escena (entrar, girarse, acercarse, pausas, bocadillos) aplicada a las 10 escenas principales**  
`1 j` · depende de: AV17-10 · flujo: transversal  
Documento: PLAN_15-10.md §3.8  
Ya existe: Muchas escenas son solo texto  
Carril: **paralelo** (Z.AI) · zona: FORMATO_AVENTURAS.md (guia de escena), assets/adventures  
Cierre: Ninguna escena principal sin movimiento de actores; lista en el informe.

**AV17-12 — Las plegarias de deidad en el juego (prayerSkillIds)**  
`0.5 j` · depende de: AV17-09 · flujo: transversal  
Documento: PLAN_15-10.md §3.9  
Ya existe: Se carga y nadie lo lee  
Carril: **principal** (Claude) · zona: Game (plegarias), CatalogoCartas  
Cierre: Elegir deidad da sus plegarias; prueba.

**AV17-06 — La fiesta del Barrio Alto jugable: Xila por la manana, la tienda abandonada (colarse, investigar, los trajes) y la fiesta (moverse, hablar, espiar, salir)**  
`3 j` · depende de: AV19-05, AV17-01 · flujo: transversal  
Documento: PLAN_15-10.md §3.6  
Ya existe: Toda la infiltracion es una conversacion  
Carril: **paralelo** (Z.AI) · zona: assets/adventures/boundington_segundo_dia.json, niveles de AV19-05 (poblar)  
Cierre: Tres partes jugables con tiradas y consecuencias; se puede fallar y seguir.

**AV17-07 — El ocaso en llamas: Perdidos patrullando, rutas no deseadas en llamas, casas ardiendo, muertos; contraste con el dia**  
`2 j` · depende de: AV19-06, AV17-11, AV17-02 · flujo: transversal  
Documento: PLAN_15-10.md §3.7  
Ya existe: El ocaso es estatico  
Carril: **paralelo** (Z.AI) · zona: assets/adventures/boundington_ocaso.json  
Cierre: La huida se juega por la ciudad en llamas con patrullas; capturas.

### AV18 — Botin, economia y oficios: cofres, tesoros, tablas, precios y la hoja Trabajar

Documento: PLAN_15-10.md §4, D0017, D0018 · Origen: NUEVA · 7 tareas · 13.5 j

**AV18-07 — Oficios, tanda 1: servir, estudiar y pescar, con la hoja Trabajar y los dados rodando**  
`3 j` · depende de: ninguna · flujo: transversal  
Documento: D0017, OFICIOS.md, PLAN_15-10.md §4.7  
Ya existe: Catalogo y arte hechos (oficios_datos01, arte01, dado01)  
Carril: **principal** (Claude) · zona: NUEVOS Game/Oficios.*, hoja Trabajar, DiceRollWidget  
Cierre: Los tres oficios se juegan; dados de oficio por rango; dados rodando; pruebas.

**AV18-01 — La economia cerrada de los tres dias de Boundington: presupuesto por dia y mapa, precios por rareza, venta al 25 %, tres perfiles**  
`1 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §4.1 (analisis de Oriol)  
Ya existe: Precios sueltos (pan 8, daga 60, espada 120); sin presupuesto  
Carril: **paralelo** (ChatGPT) · zona: NUEVO planificacion_alcance_y_vision/ECONOMIA.md  
Cierre: ECONOMIA.md con tablas y la propuesta a Oriol en la BANDEJA.

**AV18-06 — Enemigos con mas acciones (PO-46): dano en los 900 ataques y 2-3 cartas por monstruo de Boundington**  
`1.5 j` · depende de: AV16-14 · flujo: transversal  
Documento: PLAN_15-10.md §4.6  
Ya existe: Los ataques no tienen cifra; los monstruos no tienen cartas  
Carril: **paralelo** (ChatGPT) · zona: assets/catalogs/monsters.json (ataques), cartas de monstruo  
Cierre: Validador en verde; los de Boundington con cartas propias.

**AV18-02 — Contenedores en el motor: cofres, barriles, sacos, armarios y tesoros ocultos con tabla, presupuesto, garantizados y requisitos; se tiran una vez y se guardan**  
`3 j` · depende de: AV18-01 · flujo: transversal  
Documento: PLAN_15-10.md §4.2  
Ya existe: Hay un papiro de cofre (m_cofre) para objetos sueltos  
Carril: **paralelo** (ChatGPT) · zona: NUEVOS Game/Contenedores.*, catalogo de objetos (bloque contenedor), por ancla: GameSessionInteraccion, SaveGame  
Cierre: Abrir, cerrar con llave, ocultos con Percepcion, guardado; pruebas; captura.

**AV18-03 — Tablas de botin y objetos: 40-60 reutilizables y 10-15 unicos con efecto tactico**  
`2 j` · depende de: AV18-01, AV16-14 · flujo: transversal  
Documento: PLAN_15-10.md §4.3  
Ya existe: Hay 28 ids de botin con precio  
Carril: **paralelo** (ChatGPT) · zona: assets/catalogs (objetos de botin, tablas), tools/validar_botin.py  
Cierre: Catalogo y tablas por region y contenedor; validador en verde.

**AV18-04 — Simulador de economia por dia con tres perfiles (explorador, historia, optimizador)**  
`1 j` · depende de: AV18-03 · flujo: transversal  
Documento: PLAN_15-10.md §4.4  
Ya existe: Nada  
Carril: **paralelo** (ChatGPT) · zona: NUEVO tools/simular_economia.py  
Cierre: Saldo por dia y perfil dentro de los margenes de ECONOMIA.md.

**AV18-05 — Repartir el botin por Boundington, el prologo y las afueras con el presupuesto por mapa**  
`2 j` · depende de: AV18-02, AV18-03, AV16-09 · flujo: transversal  
Documento: PLAN_15-10.md §4.5  
Ya existe: Casi no hay nada que registrar  
Carril: **paralelo** (Qwen) · zona: assets/levels (Boundington, prologo, afueras): objetos contenedor  
Cierre: Recuento por mapa dentro del presupuesto; capturas de 3 escondites.

### AV19 — Las afueras de Boundington y la cueva de Gorran (con altura)

Documento: PLAN_15-10.md §5, D0018 · Origen: NUEVA · 6 tareas · 15 j

**AV19-02 — Altura en el motor para niveles naturales: escalones y rampas, dibujo por altura, colision**  
`3 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §5.2  
Ya existe: Todo es plano  
Carril: **principal** (Claude) · zona: Render (alturas), Level (alturas), GameSession (colision por altura)  
Cierre: Un nivel de prueba con dos alturas y una rampa; pruebas.

**AV19-01 — Las afueras: senda desde el vado a un rio, un bosquecillo, granjas con huerto y un establo**  
`3 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §5.1  
Ya existe: Nada: la cueva de Gorran no tiene puerta  
Carril: **paralelo** (Qwen) · zona: NUEVOS assets/levels/afueras_*.json y su TMX, puertas desde los campamentos del vado  
Cierre: Nivel conectado ida y vuelta; validar_puertas y conectividad en verde; sitios de huerto y establo.

**AV19-05 — Niveles para la fiesta: la tienda abandonada y la mansion del Barrio Alto (salon, jardin, piso de arriba)**  
`2 j` · depende de: AV16-00 · flujo: transversal  
Documento: PLAN_15-10.md §5.5  
Ya existe: Nada  
Carril: **paralelo** (Qwen) · zona: NUEVOS assets/levels/tienda_abandonada.json, mansion_barrio_alto*.json  
Cierre: Niveles conectados; listos para que Z.AI los pueble.

**AV19-03 — La montana pequena con altura: el sendero que sube a la cueva**  
`2 j` · depende de: AV19-01, AV19-02 · flujo: transversal  
Documento: PLAN_15-10.md §5.3  
Ya existe: Nada  
Carril: **paralelo** (Qwen) · zona: NUEVO assets/levels/montana_gorran*.json  
Cierre: Se sube andando; capturas.

**AV19-06 — Boundington en el ocaso: fuego, escombros, rutas cortadas por las llamas, cadaveres y Perdidos patrullando**  
`2 j` · depende de: AV16-09, AV17-11 · flujo: transversal  
Documento: PLAN_15-10.md §5.6  
Ya existe: El ocaso usa la ciudad de dia  
Carril: **paralelo** (Qwen) · zona: NUEVO assets/levels/ciudad_centro_ocaso.json  
Cierre: Variante cargable; la ruta buena se ve; capturas.

**AV19-04 — La cueva de Gorran: 3-5 salas, patrullas, cofre protegido y la guarida con la Furia de Alaro**  
`3 j` · depende de: AV19-03, AV18-02 · flujo: transversal  
Documento: PLAN_15-10.md §5.4  
Ya existe: La mision apunta a un nivel de ejemplo  
Carril: **paralelo** (Qwen) · zona: NUEVOS assets/levels/cueva_gorran_*.json, enemigos que patrullan, botin  
Cierre: Se juega de punta a punta; el objetivo de la espada apunta aqui.

### AV20 — Temor y respeto por la apariencia (PO-38)

Documento: PLAN_15-10.md §6, D0018 · Origen: NUEVA · 1 tareas · 1 j

**AV20-01 — Diseno: temor y respeto por la apariencia (cicatrices, raza, mutaciones, ropa) cruzado con moral y reputacion**  
`1 j` · depende de: ninguna · flujo: transversal  
Documento: PLAN_15-10.md §6, PO-38  
Ya existe: La moral no se toca al caer; la apariencia no cuenta  
Carril: **principal** (Claude) · zona: planificacion (diseno), Moral, Reputacion  
Cierre: Propuesta en la BANDEJA con numeros y ejemplos.

## 4. Cruces de fase declarados

El orden de presentacion es el del documento; el orden real lo da el grafo. Estas dependencias apuntan a una fase posterior a proposito:

- AV4-14 (AV4) depende de AV10-02 (AV10): cruce de fase hacia delante
- AV5-08 (AV5) depende de AV9-05 (AV9): cruce de fase hacia delante
- AV6-17 (AV6) depende de AV10-01 (AV10): cruce de fase hacia delante
- AV17-06 (AV17) depende de AV19-05 (AV19): cruce de fase hacia delante
- AV17-07 (AV17) depende de AV19-06 (AV19): cruce de fase hacia delante

## 5. Tareas transversales (no pertenecen a un flujo del jugador)

Contratos, editor, red y aceptacion: habilitan flujos pero no son un flujo. Son 94 de 224.

- AV0-03 — Mapa de solapamientos con los paquetes vivos (Fase H, EST, corte B, fase E): que esta ya escrito y de quien es
- AV1-03 — Tipografia UTF-8 limpia y wrapWidth automatico
- AV14-04 — Banco de sonidos de combate, interfaz y pasos (CC0 o propios, con licencia): golpes por material, fallo, roce, critico, dados, cartas, botones, papel
- AV14-13 — Pixel: poses dignas (sentado en 4 direcciones, brindar, escribir, llamar, apoyado en la barra, dormir) para los personajes de la historia
- AV14-15 — Recorrido automatico de todos los botones de todos los menus, con informe de los que no responden o no se ven
- AV14-18 — Pixel: iconos de los objetos de las tiendas de Boundington, de la economia del turno y de los tipos de dano
- AV14-19 — El lag: compilar en Release en el Mac, medir arranque y creador, y hacer los retratos del creador por partes
- AV14-20 — Validador de puertas: cada puerta lleva a un nivel que existe, y la casilla a la que llega se pisa y tiene puerta de vuelta
- AV14-21 — Prologo: los tres amigos con tunica y sin capucha; despues se la ponen y salen andando de la habitacion
- AV14-22 — El diario de misiones completo: apartados, decisiones tomadas, sus consecuencias y lo que se te pide (H3-02 + MISIONES-01 juntos)
- AV14-23 — D-MOR-09 y D-MIS-03: la reputacion por faccion en el comercio y el HUD; el rumor es un beat que salta al hablar
- AV14-24 — Plan de integracion de lo terminado que nadie usa (las 13 piezas de H8-06), en el Gantt
- AV15-01 — Evaluacion heuristica (Nielsen + Norman) y recorrido cognitivo de las cuatro tareas del jugador nuevo
- AV15-02 — Los sitios por su nombre: carteles con nombre y emblema sobre la puerta («El lingote de oro» donde esta Luisarda), el mismo en minimapa, tienda y dialogo
- AV15-06 — La guia de estilo de Onegai: paleta, letras, reticula de 8 px, botones con sus 5 estados, iconos y voz
- AV16-00 — Commitear el arbol de hoy (los 17 paquetes del 14 y 15/10 aplicados) para que las otras IAs partan de el
- AV16-01 — Creacion: el Espectro (y cualquier raza no jugable) no sale en la lista ni en la busqueda
- AV16-02 — Creacion: los atributos empiezan todos a 0; ni la raza ni el kit suman; solo se destacan los principales de la clase
- AV16-03 — Ropa que no cuadra con el cuerpo: auditoria automatica de cada prenda x cuerpo x talla x rumbo y arreglo (o fuera de la lista)
- AV16-04 — Orden de dibujo: lo plano del suelo (alfombras, felpudos) siempre debajo de los personajes; el barquero delante de su barca
- AV16-05 — El jugador anda en las cinematicas tan fluido como Sho-noco
- AV16-06 — Salidas que se ven: la salida a otro mapa marcada en el suelo con su cartel «Hacia X»; dentro, «Salir a la calle»; nombres de todas las puertas revisados
- AV16-07 — Puertas que se ven: al acercarte, la puerta se resalta aunque la tape un tejado, y el cartel apunta a ella
- AV16-10 — Suelo mas sencillo y con transiciones: menos variantes y sin rayado; bordes suaves entre materiales (dual-grid)
- AV16-11 — Combate: el turno del rival se entiende (se esconde tu mano, la camara va al que actua, cartel «Turno de X») y la tira de iniciativa se lee mejor
- AV16-12 — Combate: menos pasos por turno (6 -> 4 casillas, DES 3+ suma 1) y re-medir
- AV16-13 — El fundido no molesta: solo se funde lo que tapa al jugador, no lo que tiene al lado
- AV16-14 — Estados nuevos (PO-37): quemado, congelado, aturdido y silenciado, y dados extra
- AV16-15 — Los mapas en Excel: cada nivel como una hoja de casillas con colores y codigos que Oriol edita, y vuelta al juego
- AV17-09 — Escribir las 10 deidades que faltan (PO-45)
- AV17-10 — Ordenes nuevas de cinematica para Z.AI: trepar y bajar, arder, tumbado, patrulla, emocion (bocadillo)
- AV17-11 — Fuego y humo en el mundo: casas en llamas y celdas que queman
- AV18-07 — Oficios, tanda 1: servir, estudiar y pescar, con la hoja Trabajar y los dados rodando
- AV19-02 — Altura en el motor para niveles naturales: escalones y rampas, dibujo por altura, colision
- AV20-01 — Diseno: temor y respeto por la apariencia (cicatrices, raza, mutaciones, ropa) cruzado con moral y reputacion
- AV0-02 — Contratos JSON de los catalogos nuevos (facciones, reputacion, propiedades, rentas, rutinas, clima, delitos, cartas)
- AV0-04 — Autoridad cliente/servidor por sistema y validacion del contenido de expansion
- AV0-05 — Fichas de decision pendientes de Oriol que bloquean fases
- AV0-06 — Criterio de cierre «que sea divertido»: que se prueba en cada fase antes de darla por hecha
- AV12-05 — Deuda de contenido aguas arriba: 202 hechizos huerfanos, 432 monstruos con vida 0, 59 clases sin hechizos
- AV14-06 — Musica de combate en bucle de energia constante, sin solista (Elemechs 16:18), con golpe de entrada y salida
- AV14-14 — Sentarse y poses en el motor: punto de asiento en sillas y bancos, ordenes «sienta» y «pose» en las cinematicas, PNJ sentados en la taberna
- AV14-17 — Tienda rehecha con lo que diga Oriol, con los iconos de cada objeto
- AV15-03 — Quitar ruido de la ciudad: calles principales continuas, fuera los objetos que tapan o cortan el paso, lo que tapa al jugador translucido
- AV15-04 — El minimapa como mapa: lugares con nombre e icono, hitos y filtros (tiendas, servicios, casas)
- AV15-05 — Los menus viejos al pergamino: comprar casas, las termas y el resto, con confirmacion en lo caro e irreversible
- AV15-07 — Legibilidad y accesibilidad: contraste 4,5:1 sobre el pergamino, lineas de 45-75, nada solo por color, texto escalable, menos temblor si se pide
- AV16-08 — Cada puerta, un sitio distinto: los segundos (Boutique de Madame, Taberna Rica, Caja Firme, mercado) con nombre e interior propios
- AV16-09 — Boundington con amplitud: reubicar los edificios del centro (calles anchas, puertas a la calle, paso plaza-iglesia) y regenerar
- AV17-01 — Segundas oportunidades: ninguna tirada fallada cierra una via; el PNJ ofrece otra (un favor) y, si vuelves a fallar, una de explorar
- AV17-02 — Pasar el tiempo: esperar a mediodia, tarde o noche en tabernas y posada; el dia 3 empieza por la manana
- AV17-03 — La cripta bajo la iglesia jugable: sectarios, Edul y Verina, enganar o unirse, escuchar, escapar
- AV17-04 — El castillo con personajes y cinematica
- AV17-05 — El gato en el arbol: cinematica de trepar a un arbol cercano y bajar con el gato
- AV17-08 — Teatralidad: guia de escena (entrar, girarse, acercarse, pausas, bocadillos) aplicada a las 10 escenas principales
- AV17-12 — Las plegarias de deidad en el juego (prayerSkillIds)
- AV18-01 — La economia cerrada de los tres dias de Boundington: presupuesto por dia y mapa, precios por rareza, venta al 25 %, tres perfiles
- AV18-06 — Enemigos con mas acciones (PO-46): dano en los 900 ataques y 2-3 cartas por monstruo de Boundington
- AV19-01 — Las afueras: senda desde el vado a un rio, un bosquecillo, granjas con huerto y un establo
- AV19-05 — Niveles para la fiesta: la tienda abandonada y la mansion del Barrio Alto (salon, jardin, piso de arriba)
- AV2-01 — Propiedades de instancia dentro de motor_editor (ImGui)
- AV1-02 — Barra contextual fija de controles [ESC] [I] [E] [M] por modo
- AV1-06 — Reloj de mundo, calendario y clima en el HUD
- AV11-01 — Protocolo de red y autoridad: que viaja, que se valida en servidor, como se reconcilia
- AV12-01 — Pasar medir_alcance.py por cada fase: nada se cierra si no llega al jugador
- AV14-16 — Arreglar los menus: estado encima y pulsado en cada boton, sonido de papel, transiciones sin tirones
- AV17-06 — La fiesta del Barrio Alto jugable: Xila por la manana, la tienda abandonada (colarse, investigar, los trajes) y la fiesta (moverse, hablar, espiar, salir)
- AV18-02 — Contenedores en el motor: cofres, barriles, sacos, armarios y tesoros ocultos con tabla, presupuesto, garantizados y requisitos; se tiran una vez y se guardan
- AV18-03 — Tablas de botin y objetos: 40-60 reutilizables y 10-15 unicos con efecto tactico
- AV19-03 — La montana pequena con altura: el sendero que sube a la cueva
- AV19-06 — Boundington en el ocaso: fuego, escombros, rutas cortadas por las llamas, cadaveres y Perdidos patrullando
- AV2-02 — Gestor de proyectos en motor_editor
- AV2-04 — Herramientas clicables completas y marcadores en rombo
- AV6-01 — Catalogo de NPC: personalidad, voz, complexion y sprite base
- AV6-06 — Creador de monstruos: tier y formulas automaticas de CA, PV y bono de ataque
- AV6-09 — Pincel de zonas de ley y jurisdiccion: orden imperial, mercado neutro, tierra de nadie
- AV6-14 — Matriz de privilegios Admin MMO / Game Master / Jugador con bloqueo de escritura sobre catalogos oficiales
- AV11-02 — Servidor de mundo abierto con filtro de contenido de expansion
- AV12-02 — Partida de prueba guionizada que recorre los 7 flujos de workflows_juego/
- AV17-07 — El ocaso en llamas: Perdidos patrullando, rutas no deseadas en llamas, casas ardiendo, muertos; contraste con el dia
- AV18-04 — Simulador de economia por dia con tres perfiles (explorador, historia, optimizador)
- AV18-05 — Repartir el botin por Boundington, el prologo y las afueras con el presupuesto por mapa
- AV19-04 — La cueva de Gorran: 3-5 salas, patrullas, cofre protegido y la guarida con la Furia de Alaro
- AV2-03 — Retirar level_editor de CMake y dejar motor_editor como unico editor
- AV2-05 — Panel de problemas y referencias integrado en el flujo de autor
- AV6-03 — Editor de parametros morales: faccion, romance habilitado, gustos y tolerancia al crimen
- AV6-08 — Generador de botin con porcentajes de drop
- AV6-15 — Barra de control en vivo del Master: pausar mundo, inyectar monstruo, tirada secreta
- AV6-16 — Aislamiento en disco de la campana del Master (partidas/mi_campana/*.json)
- AV11-03 — Servidor de salas privadas con Master y sus herramientas en vivo
- AV11-05 — Antitrampas basico: validacion de tiradas y de movimientos en servidor
- AV12-06 — Actualizar BITACORA, TABLERO y el documento de alcance con el estado real
- AV11-04 — Economia unificada del MMO y persistencia de propiedades, rentas e impuestos
- AV6-07 — Asignacion de baraja y de comportamiento tactico al monstruo

## 6. Orden de ejecucion

Orden topologico estable (por ID dentro de cada nivel de dependencias). Una tarea no se empieza con dependencias abiertas.

```
AV0-01    AV0-03    AV1-03    AV13-01   AV13-04   AV13-06 
AV13-07   AV13-08   AV13-09   AV13-21   AV13-30   AV13-31 
AV13-33   AV13-34   AV13-35   AV13-36   AV14-01   AV14-04 
AV14-09   AV14-10   AV14-13   AV14-15   AV14-18   AV14-19 
AV14-20   AV14-21   AV14-22   AV14-23   AV14-24   AV15-01 
AV15-02   AV15-06   AV16-00   AV16-01   AV16-02   AV16-03 
AV16-04   AV16-05   AV16-06   AV16-07   AV16-10   AV16-11 
AV16-12   AV16-13   AV16-14   AV16-15   AV17-09   AV17-10 
AV17-11   AV18-07   AV19-02   AV20-01   AV0-02    AV0-04  
AV0-05    AV0-06    AV0-07    AV1-01    AV1-04    AV1-07  
AV12-05   AV13-02   AV13-10   AV13-22   AV14-02   AV14-03 
AV14-05   AV14-06   AV14-07   AV14-08   AV14-11   AV14-14 
AV14-17   AV15-03   AV15-04   AV15-05   AV15-07   AV16-08 
AV16-09   AV17-01   AV17-02   AV17-03   AV17-04   AV17-05 
AV17-08   AV17-12   AV18-01   AV18-06   AV19-01   AV19-05 
AV2-01    AV7-01    AV1-02    AV1-05    AV1-06    AV10-01 
AV11-01   AV12-01   AV13-03   AV13-05   AV13-11   AV14-12 
AV14-16   AV17-06   AV18-02   AV18-03   AV19-03   AV19-06 
AV2-02    AV2-04    AV3-01    AV3-03    AV4-01    AV5-01  
AV6-01    AV6-06    AV6-09    AV6-13    AV6-14    AV7-02  
AV7-03    AV7-04    AV7-05    AV7-08    AV8-03    AV9-01  
AV9-05    AV10-02   AV10-04   AV11-02   AV12-02   AV13-12 
AV13-32   AV17-07   AV18-04   AV18-05   AV19-04   AV2-03  
AV2-05    AV3-02    AV3-04    AV3-05    AV3-08    AV4-02  
AV4-03    AV4-07    AV4-08    AV4-15    AV5-02    AV5-05  
AV5-09    AV5-11    AV6-02    AV6-03    AV6-08    AV6-10  
AV6-11    AV6-15    AV6-16    AV7-06    AV7-07    AV8-04  
AV8-09    AV9-02    AV9-04    AV9-06    AV9-07    AV10-03 
AV10-05   AV10-06   AV11-03   AV11-05   AV12-04   AV12-06 
AV13-13   AV13-15   AV3-06    AV3-07    AV4-04    AV4-05  
AV4-09    AV4-10    AV4-16    AV5-03    AV5-08    AV5-10  
AV6-04    AV6-12    AV8-01    AV8-05    AV8-10    AV11-04 
AV12-03   AV12-07   AV13-14   AV13-16   AV13-17   AV13-20 
AV13-23   AV13-24   AV13-28   AV4-06    AV4-11    AV4-13  
AV5-04    AV5-06    AV5-07    AV6-05    AV8-02    AV8-06  
AV9-03    AV9-08    AV13-18   AV13-19   AV13-25   AV13-29 
AV4-12    AV4-14    AV6-07    AV6-17    AV8-07    AV13-26 
AV13-27   AV8-08  
```

## 7. Estado de las decisiones (Oriol respondio el 28/09)

De las 56 decisiones: **51 cerradas**, **2 asumidas** (Oriol dejo la pregunta en blanco y se aplica la recomendacion diciendolo en voz alta) y **3 abiertas**. La respuesta literal, la consecuencia y las tareas que la recogen estan en `DECISIONES_RESUELTAS.md` (generado).

| ID | Estado | Consecuencia en el plan | Tareas |
| --- | --- | --- | --- |
| D-COMB-01 | cerrada | Gana el reglamento Ed. 3: 6 = 1 exito, 5 = 0.5, los 4 solo con ventaja. Se corrige §10.6.A.4 del documento de vision. | AV5-03 AV1-04 AV12-04 |
| D-COMB-02 | asumida | CRITICO = la mitad o mas de los dados son 6. El redondeo hacia arriba (1d6 con un 6 = critico) es la recomendacion aplicada porque Oriol dejo esa pregunta en blanco. Pifia sin cambio. Cambia DicePoolEngine y sus pruebas: primero el reglamento Ed. 4. | AV5-03 AV12-04 AV12-07 |
| D-COMB-03 | cerrada | Iniciativa SI: 1d6 + Destreza. Contradice el turno simple por bandos de BattleState, asi que se registra en el reglamento antes de tocar codigo y se reescriben las pruebas de turno. | AV5-11 |
| D-COMB-04 | cerrada | La carta ES la habilidad referenciada por las clases (SkillDefinition), no una entidad nueva. Las 6 clases canonicas pegadas en el registro son la referencia de contenido. | AV5-01 |
| D-COMB-05 | asumida | GameOver se queda como modo pero como ESTADO DE CONSECUENCIA (escena de noqueo o rescate, cicatriz y moral aplicadas, y se continua). Oriol escribio que es terminal; la compatibilidad con §10.6.B es la recomendacion aplicada, pendiente de confirmacion. | AV5-07 AV3-02 |
| D-COMB-06 | cerrada | Sin PA ni PM: solo cooldown ligado a reposo corto o largo. Si la habilidad no lo indica, cuenta como LARGO. | AV5-02 |
| D-COMB-07 | cerrada | Vida FIJA + equipo para ampliarla: el tope base no se dobla por tier. Recalcular healthCap y los tiers con ese criterio. | AV12-04 AV5-09 |
| D-MOV-01 | cerrada | CELDAS + A* + interpolacion visual y clic de raton. Oriol habia pedido continuo; al ver el coste (rehacer colision, IA, editor y pruebas) confirmo la opcion recomendada. Nueva tarea AV7-08. | AV7-01 AV7-08 AV8-01 |
| D-MOV-02 | cerrada | Estamina nueva para correr, nadar, volar y saltar. Nace en el motor y se declara en el glosario. | AV0-01 AV7-01 AV1-07 |
| D-MOV-03 | cerrada | Peso en KG. Que CON haga de Fuerza es la recomendacion aplicada (Oriol dejo esa pregunta en blanco): bandas sobre CON x 5 kg. | AV7-04 AV0-01 |
| D-MOV-04 | cerrada | Ruido por calculo de tirada + ruido del sitio + si esta iluminado o sombreado. La iluminacion entra como factor de deteccion, no solo como estetica. | AV7-07 AV8-01 AV8-02 |
| D-MOV-05 | abierta | Sin respuesta. Se planifica con la recomendacion: bonificador a la defensa en vez de i-frames, que encaja en la enfrentada Ed. 3. | AV7-01 |
| D-COM-01 | cerrada | Unificar sobre ShopItem{price,stock,moralImpact,vendible,requiredLevel} y conectarlo con el nuevo sistema de moral. GameSession deja de mantener su ShopOffer minimo. | AV4-01 AV3-08 |
| D-COM-02 | cerrada | Moral unificada en multiples valores a tener en cuenta: un valor complejo, no un int suelto ni un float 0..1. | AV0-01 AV3-08 |
| D-COM-03 | cerrada | «Depende»: el precio de venta no es un 50% fijo universal; se concreta por tienda con precio base + dificultad de comercio/escasez. | AV4-15 AV12-03 |
| D-COM-04 | cerrada | Venta a 0.5x del valor en tienda, precio base por tienda + dificultad/escasez, y unificacion de ShopItem en el precio. H2-08 sigue siendo el prerrequisito. | AV4-01 AV4-15 |
| D-COM-05 | cerrada | Regateo = enfrentada de Carisma, reutilizando DicePoolEngine y visible en el HUD. | AV4-02 |
| D-COM-06 | cerrada | Ambos pueden ser: hurto sobre la mercancía fisica Y sobre el stock. La ranura fisica es el caso visible; el stock, el general. | AV9-08 AV6-12 |
| D-COM-07 | cerrada | Calendario global: ver D-NEG-01. Restock e impuestos al mismo reloj. | AV10-02 |
| D-COM-08 | cerrada | «Delante»: se exige proximidad fisica. La apertura remota de E-04 queda solo para depuracion. | AV4-01 |
| D-MOR-01 | cerrada | Dos magnitudes: la moral personal explica tu cuerpo y tus dialogos; la reputacion explica el trato de cada territorio. | AV3-03 AV3-08 |
| D-MOR-02 | cerrada | Unificar la moral en un valor complejo de mas de un factor y darle profundidad. Oriol pide librerias que trabajen esos patrones complejos. | AV3-08 AV0-01 |
| D-MOR-03 | cerrada | Bandas del documento: -19..+19 neutral, +20..+59 respetado, +75 heroe. Cabo suelto: que pasa entre +60 y +74 (el doc dice Salvador); se propone mantener Salvador ahi. | AV3-03 AV1-01 |
| D-MOR-04 | cerrada | «Como capas si»: la mutacion fisica se implementa por capas de sprite. AV3-01 pasa de solo-datos a capas. | AV3-01 AV3-02 |
| D-MOR-05 | cerrada | Poco a poco: de momento la faccion de los humanos, e ir ampliando. Ni 3 ni 19 de golpe. | AV3-03 AV0-02 |
| D-MOR-06 | cerrada | Se degrada con los SIGLOS sin interactuar: decae, pero en escala de siglos, no de dias. | AV3-03 |
| D-MOR-07 | cerrada | Solo las facciones que el jugador sabe: nada de tabla completa. | AV1-01 |
| D-MOR-08 | cerrada | El aumento de precio se vincula a la afinidad con su grupo, raza, estado o afinidad politica, mas tu moral y tu pureza. Multifactor: ni la formula lineal del motor ni las bandas planas del doc. | AV3-08 AV4-15 |
| D-MOR-09 | abierta | Sin respuesta. Se planifica con la recomendacion: cablear ya la lectura de factionRep en comercio y HUD, porque una magnitud que no se lee no se puede balancear. | AV3-08 AV4-01 |
| D-NEG-01 | cerrada | Reloj de mundo en TIEMPO REAL que avanza aunque el jugador este quieto, con fases dia/noche que cambian patrones de NPC, enemigos y eventos peligrosos por zona. El contador de pasos se retira. | AV10-02 AV4-05 |
| D-NEG-02 | cerrada | Renta DIARIA (no semanal como dice §11.4 del documento). | AV4-05 AV10-02 |
| D-NEG-03 | cerrada | Propiedad = todo lo que sea comprable y toda cosa construida que sea de alguien. | AV4-03 AV6-10 |
| D-NEG-04 | cerrada | Inquilino como registro de eventos indirectos y directos del jugador hacia su beneficio de vida, no como NPC con IA propia de entrada. | AV4-05 |
| D-NEG-05 | cerrada | La inversion publica mejora o degrada la calidad de la infraestructura: cambio real y medible, no solo fama. | AV4-06 |
| D-NEG-06 | cerrada | Minijuego a ritmo (no tirada) para los oficios. | AV4-07 AV4-08 |
| D-NEG-07 | cerrada | Los impuestos financian la prosperidad del pueblo y la amplian al superar ciertas cuotas: no son un sumidero. | AV4-09 AV12-03 |
| D-NEG-08 | cerrada | Si: se puede vender un negocio propio. | AV4-03 |
| D-NEG-09 | cerrada | Todo cuenta: los muebles suben el valor de la propiedad y permiten reformarla. No solo el alquiler mueve la moral. | AV4-04 AV4-05 AV3-08 |
| D-EQU-01 | cerrada | Tiers para el juego de mesa; la XP del documento se reutiliza como AFINIDAD (oficio, fe, dominio de clase). Progresion doble, no niveles. | AV0-07 AV5-09 |
| D-EQU-02 | cerrada | Cuatro stats {CON, DES, INT, CAR} con rango 0..8 dependiente del tier. Las cartas canonicas que dicen «Fue» se mapean a CON. | AV0-01 |
| D-EQU-03 | cerrada | Si: las 3 opciones de carta al subir salen de learnableSkills de la clase. | AV5-09 |
| D-EQU-04 | cerrada | kg (con CON haciendo de Fuerza, ver D-MOV-03). | AV7-04 AV0-01 |
| D-EQU-05 | cerrada | Si: la ropa tiene efecto social Y efectos de aumento de caracteristicas. | AV3-01 |
| D-EQU-06 | cerrada | Si: al subir de tier los PV se rellenan al tope nuevo. | AV5-09 |
| D-EQU-07 | cerrada | Si: se expone la multiclass al jugador, en contra de la recomendacion. Habra que equilibrar dos mazos y el handLimit multiclass. | AV5-10 |
| D-EQU-08 | cerrada | Entre combates es gratis; DENTRO del combate cuesta un turno para desaprender y otro para aprender. | AV5-10 |
| D-EQU-09 | cerrada | Si: existe degradacion del equipo. En contra de la recomendacion, entra con su coste: nueva tarea AV4-16. | AV4-16 |
| D-MIS-01 | cerrada | Unificar y segregar por tipo: principales, eventos, secundarias, de profesion, de raza y de evento unico (casarse, tener un hijo). AdventureScript sigue siendo la unica fuente. | AV10-01 AV6-17 |
| D-MIS-02 | cerrada | Solo iconos de cosas en el mapa, de momento: nada de marcadores de mision flotantes. | AV10-03 AV1-06 |
| D-MIS-03 | abierta | Sin respuesta. Se planifica con la recomendacion: el rumor es un beat con TriggerType::Talk y condiciones. | AV10-03 |
| D-MIS-04 | cerrada | Depende de la prueba: investigacion sola = Inteligencia; conocimiento arcano = Carisma; etc. Hace falta una tabla prueba -> stat en el glosario. | AV10-04 AV0-01 |
| D-MIS-05 | cerrada | Se decide mision a mision que da cada una: no hay recompensa estandar. | AV10-01 AV10-05 |
| D-MIS-06 | cerrada | Depende del mercenario: podria atacarte, irse, no querer hablarte nunca mas o acobardarse. Cuatro salidas, no dos. | AV4-13 |
| D-MIS-07 | cerrada | El oro se cobra DESPUES de completar el encargo, no por adelantado. Corrige §10.11 del documento. | AV4-14 |
| D-MIS-08 | cerrada | Se aplaza: los caracteres raros se trabajaran cuando todo lo demas este finalizado. Sigue la veda de tildes/ñ en los JSON de aventura. | AV1-03 |
| D-MIS-09 | cerrada | Depende de la naturaleza del rumor: los urgentes caducan (un secuestro), los atemporales no (una espada milenaria en una cueva). Hace falta un campo de caducidad por mision. | AV10-01 AV10-03 |

## 8. Dos carriles: lo de Claude y lo que pueden coger las otras IAs

Desde el 14/10 (D0014) las tareas nuevas dicen su carril. **Principal**: toca el juego (Application, GameSession, el HUD de combate, las aventuras) y lo lleva Claude. **Paralelo**: ficheros NUEVOS con su linea de CMake, o datos aguas arriba; lo puede coger otra IA sin pisar lo principal. Quien la tiene esta en `orquestador/<agente>.md` y se reclama con su fila en el TABLERO, con la zona de esta tabla.

| Tarea | Carril | Quien | Jornadas | Depende de | Zona |
| --- | --- | --- | ---: | --- | --- |
| AV13-01 — Reglamento: el empate roza (1), la ayuda de dados en Tier 0-1 y los esbirros a DES 1-2 | principal | Claude | 0.5 | ninguna | dndWeebCC-master/docs/Sistema_Cartas_Tiers.md |
| AV13-04 — Probabilidad exacta de una tirada enfrentada: ganar, roce y fallar, con ventaja, critico de dos 6 y pifia | paralelo | opencode | 1.5 | ninguna | NUEVOS include/RPG/ProbabilidadPool.h, src/RPG/ProbabilidadPool.cpp, tests/RPG/ProbabilidadPoolTests.cpp, cmake/ProbabilidadPool.cmake (+ su include en CMakeLists.txt) |
| AV13-06 — Tablero de combate sin GL: recuadro de 15x15 casillas del nivel, transitables, ocupadas, distancias y linea de vision | principal | Claude | 2 | ninguna | NUEVOS include/Game/TableroCombate.h, src/Game/TableroCombate.cpp + pruebas |
| AV13-07 — Ruta A* y casillas alcanzables con presupuesto, sin GL (sirve al combate y al clic de raton de AV7-08) | principal | Claude (14/10: la necesitaba el movimiento en combate; trae pasa a AV14-20) | 1.5 | ninguna | NUEVOS include/Game/RutaCasillas.h, src/Game/RutaCasillas.cpp, tests/Game/RutaCasillasTests.cpp, cmake/RutaCasillas.cmake (+ su include en CMakeLists.txt) |
| AV13-08 — Plantillas de area sobre casillas, sin GL: circulo, cono de 90 grados, linea y cuadrado | paralelo | codex | 1 | ninguna | NUEVOS include/Game/PlantillaArea.h, src/Game/PlantillaArea.cpp, tests/Game/PlantillaAreaTests.cpp, cmake/PlantillaArea.cmake (+ su include en CMakeLists.txt) |
| AV13-09 — Alcance y area de cada carta, aguas arriba: cuerpo a cuerpo, alcance N, a distancia, sobre si; y su plantilla | paralelo | gemini | 2 | ninguna | dndWeebCC-master (las cartas de las clases jugables); NO el motor |
| AV13-21 — Tipos de dano, resistencias y vulnerabilidades en los datos: cartas de las clases jugables y monstruos de Boundington | paralelo | gemini | 1.5 | ninguna | dndWeebCC-master (cartas y fichas de monstruos); NO el motor |
| AV13-30 — El critico atraviesa el bloqueo salvo bloqueo critico; la pifia del atacante da un ataque de oportunidad fisico | principal | Claude | 1 | ninguna | CartasTiradas (reaccionAntesDelGolpe, golpeDeArma), DanoEnfrentado |
| AV13-31 — La pifia en una salvacion: te entra el dano y el efecto, duplicado | principal | Claude | 0.5 | ninguna | CartasTiradas (danoDeCarta, estados), DanoEnfrentado |
| AV13-33 — Morir: escena de noqueo o rescate, cicatriz y moral, Game Over y vuelta al ultimo guardado con una cicatriz mas | principal | Claude | 1.5 | ninguna | GameSession (derrota), GameOverScreen, SaveGame (cicatrices), Application |
| AV13-34 — La compania de Rosita llega cuando Sandes, a 1/4 de vida o menos, te da un golpe mortal; scripteado para que pase siempre | principal | Claude | 1 | ninguna | aventuras de Boundington (Ocaso), Refuerzos, BattleState |
| AV13-35 — Borrar el combate y las pruebas antiguas (sin mesa de cartas), midiendo antes que nadie los usa | principal | Claude | 1.5 | ninguna | BattleState (camino sin mesa), demos y pruebas viejas, tools/buscar_sobrante.py |
| AV13-36 — Las 950 cartas de ejemplo y las clases escondidas, convertidas en habilidades y clases de monstruos y enemigos | paralelo | gemini | 2 | ninguna | dndWeebCC-master (material de ejemplo Llerba/Tomba, clases escondidas); NO el motor |
| AV14-01 — Un solo evento de golpe (quien, a quien, cuanto, tipo, grado) del que cuelgan numero, particulas, sonido y marco | principal | Claude | 1 | ninguna | NUEVOS include/Game/GolpesCombate.h + src; BattleState y CartasTiradas lo emiten |
| AV14-04 — Banco de sonidos de combate, interfaz y pasos (CC0 o propios, con licencia): golpes por material, fallo, roce, critico, dados, cartas, botones, papel | paralelo | qwen | 2 | ninguna | NUEVOS assets/audio/combate/, assets/audio/ui/, assets/audio/pasos/, assets/audio/sonidos.json, assets/audio/LICENCIAS.md |
| AV14-09 — Andar fluido con los frames de hoy: sin paron entre casillas, aceleracion y frenada, camara suave, pasos y polvo | principal | Claude | 1.5 | ninguna | Render/Player, PlayerMotionData, camara, AudioDirector (pasos) |
| AV14-10 — Pixel: ciclo de andar de 6 frames en 4 direcciones para los cuerpos base y los personajes de la historia | paralelo | pixel | 3 | ninguna | assets/textures/pixellab/lote_andar/ (PNG de PixelLab; los importa Claude) |
| AV14-13 — Pixel: poses dignas (sentado en 4 direcciones, brindar, escribir, llamar, apoyado en la barra, dormir) para los personajes de la historia | paralelo | pixel | 3 | ninguna | assets/textures/pixellab/lote_poses/ |
| AV14-15 — Recorrido automatico de todos los botones de todos los menus, con informe de los que no responden o no se ven | paralelo | zcode | 1.5 | ninguna | NUEVOS examples/recorrido_botones.cpp, cmake/RecorridoBotones.cmake, capturas_ui/botones/ (informe y capturas); sin tocar Application.* |
| AV14-18 — Pixel: iconos de los objetos de las tiendas de Boundington, de la economia del turno y de los tipos de dano | paralelo | pixel | 2 | ninguna | assets/textures/pixellab/lote_iconos/ |
| AV14-19 — El lag: compilar en Release en el Mac, medir arranque y creador, y hacer los retratos del creador por partes | principal | Claude | 1 | ninguna | Personajes/creador (retratos), CMakePresets |
| AV14-20 — Validador de puertas: cada puerta lleva a un nivel que existe, y la casilla a la que llega se pisa y tiene puerta de vuelta | paralelo | trae | 1.5 | ninguna | NUEVOS tools/validar_puertas.py y tools/test_validar_puertas.py; una llamada en tools/validar_todo.py |
| AV14-21 — Prologo: los tres amigos con tunica y sin capucha; despues se la ponen y salen andando de la habitacion | principal | Claude | 1 | ninguna | assets/cinematicas/cinematicas.json, aventura del prologo, sprites de los amigos |
| AV14-22 — El diario de misiones completo: apartados, decisiones tomadas, sus consecuencias y lo que se te pide (H3-02 + MISIONES-01 juntos) | principal | Claude | 3 | ninguna | DiarioMisiones, QuestDefinition (H3-02), MISIONES-01, PaginaCuaderno |
| AV14-23 — D-MOR-09 y D-MIS-03: la reputacion por faccion en el comercio y el HUD; el rumor es un beat que salta al hablar | principal | Claude | 1 | ninguna | Comercio, HUD (reputacion), aventuras (rumores) |
| AV14-24 — Plan de integracion de lo terminado que nadie usa (las 13 piezas de H8-06), en el Gantt | principal | Claude | 1 | ninguna | planificacion (Gantt), tools/buscar_sobrante.py |
| AV15-01 — Evaluacion heuristica (Nielsen + Norman) y recorrido cognitivo de las cuatro tareas del jugador nuevo | principal | Claude | 1.5 | ninguna | planificacion (informe), capturas |
| AV15-02 — Los sitios por su nombre: carteles con nombre y emblema sobre la puerta («El lingote de oro» donde esta Luisarda), el mismo en minimapa, tienda y dialogo | principal | Claude | 2 | ninguna | assets/levels (ciudad), assets/objects (carteles), aventuras (nombres), minimapa |
| AV15-06 — La guia de estilo de Onegai: paleta, letras, reticula de 8 px, botones con sus 5 estados, iconos y voz | principal | Claude | 1 | ninguna | NUEVO planificacion_alcance_y_vision/GUIA_DE_ESTILO.md, HUD/Pergamino/Tema.h |
| AV16-00 — Commitear el arbol de hoy (los 17 paquetes del 14 y 15/10 aplicados) para que las otras IAs partan de el | principal | Oriol | 0.2 | ninguna | MotorGraphico-main (git) |
| AV16-01 — Creacion: el Espectro (y cualquier raza no jugable) no sale en la lista ni en la busqueda | principal | Claude | 0.5 | ninguna | Game/CreacionPersonaje*, PaginasCreacion, catalogo de razas (jugable) |
| AV16-02 — Creacion: los atributos empiezan todos a 0; ni la raza ni el kit suman; solo se destacan los principales de la clase | principal | Claude | 1 | ninguna | Game/CreacionPersonaje*, PaginasCreacion (AL EMPEZAR, KIT DEL COFRE) |
| AV16-03 — Ropa que no cuadra con el cuerpo: auditoria automatica de cada prenda x cuerpo x talla x rumbo y arreglo (o fuera de la lista) | principal | Claude | 1.5 | ninguna | Game/Personajes (MontajePersonajes), tools/gen_personajes.py, assets/textures/personajes |
| AV16-04 — Orden de dibujo: lo plano del suelo (alfombras, felpudos) siempre debajo de los personajes; el barquero delante de su barca | principal | Claude | 1.5 | ninguna | Render (orden de dibujo), catalogo de objetos (capa) |
| AV16-05 — El jugador anda en las cinematicas tan fluido como Sho-noco | principal | Claude | 1 | ninguna | ApplicationCinematica, Render/Player |
| AV16-06 — Salidas que se ven: la salida a otro mapa marcada en el suelo con su cartel «Hacia X»; dentro, «Salir a la calle»; nombres de todas las puertas revisados | principal | Claude | 1.5 | ninguna | HUD (avisos, CartelesPuertas, minimapa), lugares.json, niveles (nombres de puertas) |
| AV16-07 — Puertas que se ven: al acercarte, la puerta se resalta aunque la tape un tejado, y el cartel apunta a ella | principal | Claude | 1 | ninguna | Render (contorno), HUD |
| AV16-10 — Suelo mas sencillo y con transiciones: menos variantes y sin rayado; bordes suaves entre materiales (dual-grid) | principal | Claude | 2 | ninguna | Render (terreno), assets/textures/terreno*, tools de tiles |
| AV16-11 — Combate: el turno del rival se entiende (se esconde tu mano, la camara va al que actua, cartel «Turno de X») y la tira de iniciativa se lee mejor | principal | Claude | 1.5 | ninguna | HUD de combate (MesaCombateHud, BarraCombate), camara de combate |
| AV16-12 — Combate: menos pasos por turno (6 -> 4 casillas, DES 3+ suma 1) y re-medir | principal | Claude | 0.5 | ninguna | Game/TableroCombate (movimiento), simulador_dificultad |
| AV16-13 — El fundido no molesta: solo se funde lo que tapa al jugador, no lo que tiene al lado | principal | Claude | 0.5 | ninguna | Render/OcclusionRules |
| AV16-14 — Estados nuevos (PO-37): quemado, congelado, aturdido y silenciado, y dados extra | principal | Claude | 1.5 | ninguna | RPG (estados), CartasEnCombate, iconos |
| AV16-15 — Los mapas en Excel: cada nivel como una hoja de casillas con colores y codigos que Oriol edita, y vuelta al juego | principal | Claude | 2 | ninguna | NUEVOS tools/nivel_a_excel.py, tools/excel_a_nivel.py, planificacion (MAPAS_EN_EXCEL.md) |
| AV17-09 — Escribir las 10 deidades que faltan (PO-45) | principal | Oriol | 1 | ninguna | assets/catalogs (deidades) |
| AV17-10 — Ordenes nuevas de cinematica para Z.AI: trepar y bajar, arder, tumbado, patrulla, emocion (bocadillo) | principal | Claude | 1 | ninguna | Game/Cinematica, FORMATO_AVENTURAS.md |
| AV17-11 — Fuego y humo en el mundo: casas en llamas y celdas que queman | principal | Claude | 1.5 | ninguna | Render (particulas), objetos (fuego) |
| AV18-07 — Oficios, tanda 1: servir, estudiar y pescar, con la hoja Trabajar y los dados rodando | principal | Claude | 3 | ninguna | NUEVOS Game/Oficios.*, hoja Trabajar, DiceRollWidget |
| AV19-02 — Altura en el motor para niveles naturales: escalones y rampas, dibujo por altura, colision | principal | Claude | 3 | ninguna | Render (alturas), Level (alturas), GameSession (colision por altura) |
| AV20-01 — Diseno: temor y respeto por la apariencia (cicatrices, raza, mutaciones, ropa) cruzado con moral y reputacion | principal | Claude | 1 | ninguna | planificacion (diseno), Moral, Reputacion |
| AV13-02 — Roce en el motor: un golpe fisico empatado quita 1 y el registro dice «Roce» | principal | Claude | 0.5 | AV13-01 | include/RPG/DanoEnfrentado.h, tests de DANO-02, CartasTiradas.cpp (el registro) |
| AV13-10 — Colocar a todos al empezar: heroe y enemigos donde estaban, acompanantes y refuerzos en la libre mas cercana; el borde en tinta | principal | Claude | 1 | AV13-06 | GameSession (inicio del combate), ApplicationCinematica::combate, Refuerzos |
| AV13-22 — Resistencia mitad, vulnerabilidad doble, inmunidad cero en el motor; el numero y el sonido salen del tipo | principal | Claude | 1 | AV13-21 | CartasTiradas (dano), catalogo de monstruos |
| AV14-02 — El golpe integrado en la hoja: el marco del pergamino se tine y tiembla, numero en tinta con Roce/Critico/Fallo, parpadeo, retroceso y hit-stop | principal | Claude | 2 | AV14-01 | GolpesEnPantalla, PergaminoGL (marco), Player/Enemy (parpadeo y retroceso) |
| AV14-03 — Particulas de impacto por tipo de dano (astillas, chispas, brasas, sangre) con las familias de CANON_PARTICULAS | principal | Claude | 1.5 | AV14-01 | ParticulasDeCartas / EfectosDeCartas, particulas_cartas.json |
| AV14-05 — Que suene: cada golpe, fallo, roce y critico, los dados, las cartas, los pasos y los botones, con variaciones | principal | Claude | 1.5 | AV14-01, AV14-04 | ApplicationCinematica / Application (audio), AudioDirector |
| AV14-06 — Musica de combate en bucle de energia constante, sin solista (Elemechs 16:18), con golpe de entrada y salida | paralelo | qwen | 1 | AV14-04 | NUEVOS assets/audio/musica/ (+ su entrada en sonidos.json y LICENCIAS.md) |
| AV14-07 — Los dados del pool ruedan en el centro: los 6 brillan, los 5 a medias, contra los del defensor (absorbe AV1-04) | principal | Claude | 2 | AV14-01 | DiceRollWidget, HUD de combate |
| AV14-08 — La camara sigue al que actua; velocidad del combate x1/x1,5/x2 y saltar animaciones (accesibilidad) | principal | Claude | 1.5 | AV14-01 | ApplicationCinematica::combate (camara), OptionsValues |
| AV14-11 — Integrar el ciclo de 6 frames: hojas, generador y animacion (con el paso de 3 frames para lo que aun no tenga 6) | principal | Claude | 1.5 | AV14-10, AV14-09 | HojaPersonaje, tools/gen_personajes.py, tools/pixellab/importar_actores.py |
| AV14-14 — Sentarse y poses en el motor: punto de asiento en sillas y bancos, ordenes «sienta» y «pose» en las cinematicas, PNJ sentados en la taberna | principal | Claude | 2 | AV14-13 | Cinematica (ordenes), niveles (punto de asiento), HojaPersonaje (poses) |
| AV14-17 — Tienda rehecha con lo que diga Oriol, con los iconos de cada objeto | principal | Claude | 2 | AV14-18 | TiendaHoja, PaginaCuaderno |
| AV15-03 — Quitar ruido de la ciudad: calles principales continuas, fuera los objetos que tapan o cortan el paso, lo que tapa al jugador translucido | principal | Claude | 3 | AV15-01 | assets/levels (ciudad), BloquesNivel (transparencia) |
| AV15-04 — El minimapa como mapa: lugares con nombre e icono, hitos y filtros (tiendas, servicios, casas) | principal | Claude | 1.5 | AV15-02 | MinimapaTinta, HudRetales |
| AV15-05 — Los menus viejos al pergamino: comprar casas, las termas y el resto, con confirmacion en lo caro e irreversible | principal | Claude | 3 | AV15-06 | Negocio (casas), termas, y cada pantalla con la estetica vieja |
| AV15-07 — Legibilidad y accesibilidad: contraste 4,5:1 sobre el pergamino, lineas de 45-75, nada solo por color, texto escalable, menos temblor si se pide | principal | Claude | 1.5 | AV15-06 | DialogoHoja, Opciones, Tema |
| AV16-08 — Cada puerta, un sitio distinto: los segundos (Boutique de Madame, Taberna Rica, Caja Firme, mercado) con nombre e interior propios | paralelo | ChatGPT | 1 | AV16-00 | assets/catalogs/lugares.json, interiores _sur nuevos (copias), BANDEJA (nombres) |
| AV16-09 — Boundington con amplitud: reubicar los edificios del centro (calles anchas, puertas a la calle, paso plaza-iglesia) y regenerar | paralelo | Qwen | 3 | AV16-00 | tools/gen_ciudad.py, tools/gen_ciudad_deco.py, tools/despejar_calles.py, assets/levels/ciudad_centro.json, assets/maps/ciudad_centro.tmx |
| AV17-01 — Segundas oportunidades: ninguna tirada fallada cierra una via; el PNJ ofrece otra (un favor) y, si vuelves a fallar, una de explorar | paralelo | Z.AI | 2 | AV16-00 | assets/adventures/boundington_*.json |
| AV17-02 — Pasar el tiempo: esperar a mediodia, tarde o noche en tabernas y posada; el dia 3 empieza por la manana | paralelo | Z.AI | 1.5 | AV16-00 | assets/adventures/boundington_*.json (beats de esperar) |
| AV17-03 — La cripta bajo la iglesia jugable: sectarios, Edul y Verina, enganar o unirse, escuchar, escapar | paralelo | Z.AI | 3 | AV16-00 | assets/adventures/boundington_primer_dia.json, assets/levels/cripta_*.json (poblar) |
| AV17-04 — El castillo con personajes y cinematica | paralelo | Z.AI | 2 | AV16-00 | assets/adventures (castillo), assets/levels (castillo, poblar) |
| AV17-05 — El gato en el arbol: cinematica de trepar a un arbol cercano y bajar con el gato | paralelo | Z.AI | 1 | AV17-10 | assets/adventures (el gato) |
| AV17-08 — Teatralidad: guia de escena (entrar, girarse, acercarse, pausas, bocadillos) aplicada a las 10 escenas principales | paralelo | Z.AI | 1 | AV17-10 | FORMATO_AVENTURAS.md (guia de escena), assets/adventures |
| AV17-12 — Las plegarias de deidad en el juego (prayerSkillIds) | principal | Claude | 0.5 | AV17-09 | Game (plegarias), CatalogoCartas |
| AV18-01 — La economia cerrada de los tres dias de Boundington: presupuesto por dia y mapa, precios por rareza, venta al 25 %, tres perfiles | paralelo | ChatGPT | 1 | AV16-00 | NUEVO planificacion_alcance_y_vision/ECONOMIA.md |
| AV18-06 — Enemigos con mas acciones (PO-46): dano en los 900 ataques y 2-3 cartas por monstruo de Boundington | paralelo | ChatGPT | 1.5 | AV16-14 | assets/catalogs/monsters.json (ataques), cartas de monstruo |
| AV19-01 — Las afueras: senda desde el vado a un rio, un bosquecillo, granjas con huerto y un establo | paralelo | Qwen | 3 | AV16-00 | NUEVOS assets/levels/afueras_*.json y su TMX, puertas desde los campamentos del vado |
| AV19-05 — Niveles para la fiesta: la tienda abandonada y la mansion del Barrio Alto (salon, jardin, piso de arriba) | paralelo | Qwen | 2 | AV16-00 | NUEVOS assets/levels/tienda_abandonada.json, mansion_barrio_alto*.json |
| AV13-03 — Esbirros a DES 1-2 y re-medir: te pegan el 30-35 % y el 1 contra 3 sigue en el 40 % (Oriol, 06/10) | principal | Claude | 1 | AV13-02 | assets/objects/boundington_enemigos.json, examples/simulador_dificultad.cpp |
| AV13-05 — Ayuda de dados: al apuntar, % de ganar, roce y fallar, dano previsto y vida que quedaria; nota la primera vez; opcion | principal | Claude | 1.5 | AV13-02, AV13-04 | HUD de combate (MesaCombateHud, BarraCombate), OptionsValues |
| AV13-11 — Turno por iniciativa 1d6+DES, uno a uno y enemigos incluidos, con los retratos arriba (absorbe AV5-11) | principal | Claude | 2 | AV13-10 | BattleState, CartasEnCombate, EnemyBrain, BarraCombate (orden de turno) |
| AV14-12 — Pixel: las prendas con el ciclo de 6 frames, por lotes | paralelo | pixel | 4 | AV14-11 | assets/textures/pixellab/lote_andar_prendas/ |
| AV14-16 — Arreglar los menus: estado encima y pulsado en cada boton, sonido de papel, transiciones sin tirones | principal | Claude | 2 | AV14-15, AV14-05 | HUD Pergamino (botones), Application (menus) |
| AV17-06 — La fiesta del Barrio Alto jugable: Xila por la manana, la tienda abandonada (colarse, investigar, los trajes) y la fiesta (moverse, hablar, espiar, salir) | paralelo | Z.AI | 3 | AV19-05, AV17-01 | assets/adventures/boundington_segundo_dia.json, niveles de AV19-05 (poblar) |
| AV18-02 — Contenedores en el motor: cofres, barriles, sacos, armarios y tesoros ocultos con tabla, presupuesto, garantizados y requisitos; se tiran una vez y se guardan | paralelo | ChatGPT | 3 | AV18-01 | NUEVOS Game/Contenedores.*, catalogo de objetos (bloque contenedor), por ancla: GameSessionInteraccion, SaveGame |
| AV18-03 — Tablas de botin y objetos: 40-60 reutilizables y 10-15 unicos con efecto tactico | paralelo | ChatGPT | 2 | AV18-01, AV16-14 | assets/catalogs (objetos de botin, tablas), tools/validar_botin.py |
| AV19-03 — La montana pequena con altura: el sendero que sube a la cueva | paralelo | Qwen | 2 | AV19-01, AV19-02 | NUEVO assets/levels/montana_gorran*.json |
| AV19-06 — Boundington en el ocaso: fuego, escombros, rutas cortadas por las llamas, cadaveres y Perdidos patrullando | paralelo | Qwen | 2 | AV16-09, AV17-11 | NUEVO assets/levels/ciudad_centro_ocaso.json |
| AV13-12 — Moverse por casillas en tu turno: 6 casillas, las alcanzables resaltadas, la ruta con su coste y el aviso de ataque de oportunidad | principal | Claude | 2.5 | AV13-07, AV13-11 | GameSession (modo combate), Application (raton/teclado en combate), render del suelo |
| AV13-32 — La ayuda de dados dura hasta 4 dados en la caracteristica de ataque principal, y si bajas no vuelve | principal | Claude | 1.5 | AV13-04, AV13-05 | AyudaDeDados (nuevo), SessionData (bandera), Opciones |
| AV17-07 — El ocaso en llamas: Perdidos patrullando, rutas no deseadas en llamas, casas ardiendo, muertos; contraste con el dia | paralelo | Z.AI | 2 | AV19-06, AV17-11, AV17-02 | assets/adventures/boundington_ocaso.json |
| AV18-04 — Simulador de economia por dia con tres perfiles (explorador, historia, optimizador) | paralelo | ChatGPT | 1 | AV18-03 | NUEVO tools/simular_economia.py |
| AV18-05 — Repartir el botin por Boundington, el prologo y las afueras con el presupuesto por mapa | paralelo | Qwen | 2 | AV18-02, AV18-03, AV16-09 | assets/levels (Boundington, prologo, afueras): objetos contenedor |
| AV19-04 — La cueva de Gorran: 3-5 salas, patrullas, cofre protegido y la guarida con la Furia de Alaro | paralelo | Qwen | 3 | AV19-03, AV18-02 | NUEVOS assets/levels/cueva_gorran_*.json, enemigos que patrullan, botin |
| AV13-13 — Alcance en juego: solo se apunta a quien esta a tiro; los validos resaltados; deducido mientras falten los datos de AV13-09 (absorbe AV5-04) | principal | Claude | 1.5 | AV13-12 | CartasEnCombate, BarraCombate, render de objetivos |
| AV13-15 — Economia del turno a la vista: movimiento, accion, movimiento-carta, canalizacion, reaccion, «Terminar turno» y Correr/Esquivar/Retirarse/Ayudar | principal | Claude | 2 | AV13-12 | CartasEnCombate (TurnoHeroe), BarraCombate, MesaCombateHud |
| AV13-14 — Areas visibles en el suelo al apuntar: pillan a todos los de dentro, aliados incluidos | principal | Claude | 1.5 | AV13-08, AV13-13 | CartasJugar (objetivos de area), render del suelo |
| AV13-16 — Ataques de oportunidad al salir del alcance (salvo Retirarse) y la parada como reaccion | principal | Claude | 1.5 | AV13-15 | CartasTiradas (reacciones), RutaCasillas (al salir del alcance) |
| AV13-17 — La posicion como dados: flanqueo da ventaja, la cobertura quita un dado, disparar con un enemigo al lado da desventaja | principal | Claude | 1.5 | AV13-13 | CartasTiradas (ventajas), TableroCombate |
| AV13-20 — Caido, no muerto: tiradas por turno, Ayudar levanta, tres fallos = fuera de combate; el heroe va a la consecuencia sin Game Over | principal | Claude | 1.5 | AV13-15 | BattleState (bajas), CartasEnCombate (Ayudar), GameSession (caida del heroe) |
| AV13-23 — Canalizacion = concentracion: un sostenido a la vez y tirada de CON al recibir dano | principal | Claude | 1 | AV13-15 | CartasEnCombate (canalizacion) |
| AV13-24 — Las cartas que esperaban posiciones: requiere_posiciones y la velocidad de movimiento | principal | Claude | 1 | AV13-12, AV13-13 | GanchosCarta, CartasJugar |
| AV13-28 — La accion principal ya no cierra el turno: «Terminar turno» y el turno se acaba solo cuando no queda nada que gastar | principal | Claude | 1.5 | AV13-15 | CartasEnCombate (TurnoHeroe), Application (confirmBattleCommand, pasaTurno), BarraCombate, MesaCombateHud |
| AV13-18 — Empujar: tirada enfrentada que mueve 1-2 casillas; si choca, roza | principal | Claude | 1 | AV13-15, AV13-17 | CartasJugar (accion basica), TableroCombate |
| AV13-19 — IA enemiga por puntuacion sobre el tablero (casilla x carta x % de ganar), con roles: bruto, tirador, flanqueador y cobarde (amplia AV5-06) | principal | Claude | 3 | AV13-04, AV13-12, AV13-13, AV13-17 | EnemyBrain (la parte de combate), CartasEnCombate (mano del enemigo) |
| AV13-25 — Superficies que se combinan (fuego, agua, grasa, hielo, humo) y barriles (amplia AV5-08) | principal | Claude | 3 | AV13-14, AV13-22 | TableroCombate (superficies), render del suelo, particulas |
| AV13-29 — Los acompanantes con su propio turno en la iniciativa, y los juega el jugador (salvo los que su ficha diga que van solos) | principal | Claude | 3 | AV13-11, AV13-28 | OrdenDeTurno, BattleState, CartasEnCombate (acompanantes), BarraCombate (mano del que juega), ApplicationCinematica |
| AV13-26 — Encuentros por presupuesto segun la dificultad, refuerzos con tope y jefes que se preparan (butchery) | principal | Claude | 2 | AV13-19 | aventuras (startBattle), Refuerzos |
| AV13-27 — Escena de demo y simulador con tablero: el combate de Boundington jugado de punta a punta con posiciones | principal | Claude | 1.5 | AV13-19 | examples/simulador_dificultad.cpp, demo_boundington |

## 9. Reglas de trabajo

- **Ninguna tarea se empieza con sus dependencias abiertas**; el orden de la seccion 6 ya las respeta.
- **Ningun flujo se implementa sin su decision resuelta** cuando la decision bloquea esa tarea (columna «Bloquea» de la seccion 7).
- **Una tarea se cierra cuando llega al jugador**, no cuando compila: `tools/medir_alcance.py` por uso desde cada `main()` (AV12-01).
- **No se planifica dos veces lo mismo**: la columna «Ya existe» de cada tarea y el apartado 11 de cada flujo son la defensa contra reescribir el motor.
- **Las discrepancias se registran, no se resuelven en silencio** (BITACORA, regla 3): `workflows_juego/DISCREPANCIAS_Y_DECISIONES.md`.
- **Si una tarea supera el doble de su estimacion, se para y se replantea**, en vez de anadirle jornadas.
- **El flujo es en una sola direccion**: `canon -> cartas -> motor`. La deuda de contenido (202 hechizos huerfanos, 432 monstruos con vida 0, 59 clases sin hechizos) se reclama aguas arriba, no se inventa en el motor (AV12-05).
