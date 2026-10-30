# Registro de discrepancias: documento de visión contra motor real

> **Regla del protocolo** (BITÁCORA, regla 3): *no resuelvas discrepancias en silencio; lo que no puedas resolver, regístralo.*
> Este fichero es ese registro. Ninguna línea de aquí se ha «arreglado» en el código ni en el documento: todas están **abiertas y con dueño** (Oriol).
> **Autoridad** (BITÁCORA §2.1): `canon (Onegai 2) → cartas (dndWeebCC) → motor (MotorGraphico)`. El `DOCUMENTO_DE_ALCANCE_Y_VISION.md` es visión y dirección creativa: donde contradice al reglamento ya implementado, **la recomendación por defecto es corregir el documento**, no el reglamento.
> Evidencias verificadas el 28/09/2026 sobre el árbol de trabajo (rutas relativas a `MotorGraphico-main/MotorGraphico/`).

---

## A. El documento contradice el reglamento ya implementado

| #   | El documento dice                                                        | El motor / reglamento hace                                                                                                                      | Evidencia                                                                            | Decisión            | USUARIO DICE:                                                                                                                                                                    |
| --- | ------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ | ------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| A1  | §10.6.A.4: «se cuentan éxitos (caras 4, 5 y 6)»                          | Ed. 3: cada **6 = 1 éxito**, cada **5 = 0.5**; los 4 solo suman con ventaja                                                                     | `include/RPG/DicePoolEngine.h:22-33`                                                 | D-COMB-01           | Ed. 3: cada **6 = 1 éxito**, cada **5 = 0.5**; los 4 solo suman con ventaja                                                                                                      |
| A2  | §10.6.A.4: «doble 6 es crítico; doble 1 es pifia»                        | Crítico = **todos** los dados 6. Pifia = **ningún** 5/6 **y** mínimo dos 1es                                                                    | `DicePoolEngine.h:36-45`, `es_pifia()`                                               | D-COMB-02           | Crítico = La mitad del total de los dados  es 6. Pifia = **ningún** 5/6 **y** mínimo dos 1es.                                                                                    |
| A3  | §10.6.A.1: iniciativa `1d6 + Destreza`                                   | El turno es **deliberadamente sin iniciativa ni velocidad**: aliados uno a uno y una sola pasada enemiga                                        | `include/Game/BattleState.h:24-32`                                                   | D-COMB-03           | `1d6 + Destreza`                                                                                                                                                                 |
| A4  | §10.6.B: «no existe pantalla negra de Game Over»                         | `GameMode::GameOver` existe y es **terminal**: «nada lo saca de ahí»                                                                            | `include/Game/SessionTipos.h:25`, `GameSession.h:461`, `GameOverScreen.h`            | D-COMB-05           | GameOver` existe y es **terminal**: «nada lo saca de ahí» + cicatriz afecta a moral                                                                                              |
| A5  | §10.7: se gana **EXP** y se sube de **nivel** con puntos a atributos     | «El reglamento no va de XP: va de **hitos**»; `grantMilestones` → `tierUp`, `needed = tier + 1`                                                 | `include/RPG/CharacterSheet.h:72-77,225-232`                                         | D-EQU-01            | la xp la podemos usar en afinidad a una cosa para dar progresion al juego, como oficios, fe o dominio de una clase, en el juego de mesa unicamente se plantean tiers de momento. |
| A6  | §10.7: seis atributos (FUE, DES, CON, INT, SAB, CAR)                     | Cuatro: `enum class Stat { CON, DES, INT, CAR }`, rango 0…8                                                                                     | `include/RPG/Stat.h:11-15`                                                           | D-EQU-02            | { CON, DES, INT, CAR }`, rango 0…8 dependiendo el tier                                                                                                                           |
| A7  | §10.12.B: carga en **kg** contra `Fuerza × 5 kg`                         | Peso por **categoría** (`light/medium/heavy`) contra `maxArmorWeight` de la clase                                                               | `include/RPG/InventoryEngine.h:70-101`                                               | D-EQU-04 / D-MOV-03 | carga en **kg** contra `Fuerza × 5 kg`                                                                                                                                           |
| A8  | §10.3.A: hostil = precios **triplicados**; respetado = **−15 %**         | `repPriceModPct` lineal: `-0.75 × rep` → −100 = +100 %, +100 = −50 % («P0 simplificado»)                                                        | `src/RPG/CharacterSheet.cpp:145-152`                                                 | D-MOR-08            | vincular el aumento por afinidad con su grupo, raza, estado o afinidad politica, mas tu moral mas pureza                                                                         |
| A9  | §10.3.A: bandas −19…+19 neutral y +20…+59 respetado                      | `label()`: NEUTRAL hasta +39, JUSTO hasta +74, HÉROE el resto                                                                                   | `src/Game/Morality.cpp:28-45`                                                        | D-MOR-03            | bandas −19…+19 neutral y +20…+59 respetado +75 Heroe                                                                                                                             |
| A10 | §10.12.A: locomoción continua con inercia, derrape de 0.4 s y multitudes | Movimiento **por celdas**: `tryMovePlayer(dx, dy)` con orden de pulsación y paso pendiente                                                      | `GameSession.h:129`, `include/Game/PlayerControlsData.h`                             | D-MOV-01            | locomoción continua con inercia, derrape de 0.4 s y multitudes + mover con el raton A* simplificado                                                                              |
| A11 | §10.10 y §11.2: misiones con triggers y objetivos propios                | Ya existe `AdventureScript` (objetivos, beats, timers) y el diario es **una vista de las flags**: «aquí no hay una segunda fuente de verdad»    | `include/RPG/Narrative/AdventureScript.h:74-97`, `include/Game/DiarioMisiones.h:1-8` | D-MIS-01            | -                                                                                                                                                                                |
| A12 | Todo el documento usa tildes y ñ con normalidad                          | Los JSON de aventura **no admiten tildes ni ñ** en `lines`/`speaker`/`text` (limitación de la `BitmapFont`); regla del workspace en `AGENTS.md` | `AGENTS.md` regla 4; `include/Render/BitmapFont.h`                                   | D-MIS-08            | ya trabajaremos los caracters raros cuando todo lo demas este finalizado, no?                                                                                                    |
| A13 | §10.6.A.3: cartas con coste en PA **o** PM                               | El motor juega `SkillDefinition` con enfriamiento o extras; no hay dos recursos separados                                                       | `include/RPG/Definitions/SkillDefinition.h`, `BattleState.h`                         | D-COMB-06           | El motor juega `SkillDefinition` con enfriamiento o extras; no hay dos recursos separados                                                                                        |

## B. Duplicados y magnitudes huérfanas dentro del propio motor

No son contradicciones con el documento: son piezas escritas que no se hablan entre ellas. Son el riesgo más caro de la lista, porque **cada una ya tiene pruebas propias y parecen correctas por separado**.

| #   | Qué pasa                                                                                                                                                                                                                           | Evidencia                                                                                                              | Decisión                                                                      | USUARIO DICE:                                                                                                                                                                                                                                                                                |
| --- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| B1  | **Tres morales**: `Game::Morality` (int −100…+100 con motivo e historial), `CharacterSheet::morality` (int) y `ShopEngine::moral` (float 0…1 con `moralBase` 0.5)                                                                  | `include/Game/Morality.h`, `include/RPG/CharacterSheet.h:98`, `include/Game/ShopEngine.h`                              | D-MOR-02                                                                      | Unificar la moral y darle profundidad, aqui por ejemplo es donde digo que hay que haber librerias que trabajen ciertos patrones complejos                                                                                                                                                    |
| B2  | **Dos motores de comercio**: `GameSession::openShop/buyOffer` con `ShopOffer{objectId,name,price}` (llega al jugador) y `Game::ShopEngine` con `ShopItem{price,stock,moralImpact,vendible,requiredLevel}` (solo lo usan sus tests) | `include/Game/SessionTipos.h:75-79` contra `include/Game/ShopEngine.h`; `grep ShopEngine src include` → solo su `.cpp` | D-COM-01                                                                      | Unificar con el nuevo sistema de moral: `ShopItem{price,stock,moralImpact,vendible,requiredLevel}` (solo lo usan sus tests                                                                                                                                                                   |
| B3  | **Reputación por facción sin cablear**: `factionRep`, `getRep`, `adjustRep` y `repPriceModPct` existen y nadie los llama fuera de `CharacterSheet`; el catálogo de mundo sí carga `factionRepDefault`                              | `src/RPG/CharacterSheet.cpp:130-152`, `include/RPG/Definitions/DefinicionesMundo.h:106`                                | D-MOR-09                                                                      |                                                                                                                                                                                                                                                                                              |
| B4  | **413 precios de equipo que nadie leía**: `EquipmentDefinition::price` existe, 282 piezas son alcanzables y ninguna tiene `pickup.price`, así que el botín no se puede vender                                                      | parche H2-08 escrito y **sin aplicar** (28/09); `planificacion/seguimiento/H2-08-PRECIOS.md`                           | D-COM-04                                                                      | poner un precio *0.5 su valor en tienda y cada tienda tenga precio base + dificultad de comercio/escasez de recursos y luego aplicar la unificacion de ShopItem en el precio                                                                                                                 |
| B5  | **Reglas inyectables que nadie inyectaba**: `setInventoryRules` no se llamaba en ningún sitio del motor hasta el arreglo del 28/09 en `ApplicationNivel.cpp`; sin ellas `equipmentFor()` devolvía `nullptr`                        | TABLERO fila H2-08 (opencode, 28/09)                                                                                   | lección: toda regla inyectable necesita una prueba que falle si no se inyecta | Hay que conectar todos los elementos y antes de crear revisar si ya existe algun elmento similar y preguntar como afrontar si no es obviamente una modificacion del proceso o un añadido, plantear herencias?                                                                                |
| B6  | **Reloj por pasos, no por tiempo**: `kStepsPerRentCycle = 40` pasos del jugador; el documento habla de semanas, cosechas, días de condena y jornadas                                                                               | `include/Game/GameSession.h:186,404-407`                                                                               | D-NEG-01 / D-COM-07                                                           | El horario del mundo se debe actualizar aunque el jugador este quieto dando fases de dia/noche, donde se puede aprovechar para cambiar los patrones conductuales de los npc's y de los enemigos o mobs, incluso eventos algo mas peligrosos depende de que zonas de una ciudad se frequenten |
| B7  | **`Morality::label()` devuelve «HÉROE» con tilde** mientras el comentario de la clase dice que las etiquetas son sin acentos porque la `BitmapFont` solo tiene mayúsculas                                                          | `include/Game/Morality.h:44-47`, `src/Game/Morality.cpp:45`                                                            | verificar en AV1-01/AV1-03                                                    | Sin accentos si                                                                                                                                                                                                                                                                              |

## C. Deuda de contenido aguas arriba (no es del motor)

El flujo es `canon → cartas → motor`: esto no se inventa en el motor, se registra y se reclama (AV12-05).

| Deuda                       | Cifra                 | Efecto en los flujos                     | Solucion                                                                                                                                                                                                                                                                           |
| --------------------------- | --------------------- | ---------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Hechizos huérfanos          | 202                   | WF-01: cartas que no se pueden jugar     | Darles valores utilizando las clases que te voy a referenziar:                                                                                                                                                                                                                     |
| Monstruos con vida 0        | 432                   | WF-01: enemigos que no pueden combatirse | Poner valores y testear su dificultad por el daño maximo que podria hacer todas las clases referenciadas en grupos de 1 contra 1*n(siendo n=0 y cada bucle n++;) hasta un total de 10 enemigos, asi sabremos el daño maximo de cada clase y podremos trabajar mejor en el balanceo |
| Clases sin hechizos         | 59                    | WF-06: mazos vacíos                      | Habra que referenciarlas y modificar las habilidades para que encajen con el tono de las clases referenciadas, pero con variedad de opciones multiplicando las maneras de jugar                                                                                                    |
| `lootIds` reales            | 28                    | WF-01/WF-06: botín casi nulo             | habra que mirar como incorporarlo en la economia del juego                                                                                                                                                                                                                         |
| Precios de equipo generados | 413 (282 alcanzables) | WF-03: nada se puede vender              | se debe poder vender, aunque cada npc tiene que tener una cantidad de dinero predeterminaa impidiendo que un vendedor pueda pagar cifraz exhorbitadas excepto quien si pueda pagarlas                                                                                              |
Clases referenciadas:
Clase Monje del Despertar

**Passiva**

> Si aciertas un golpe básico acumulas Ki.  
> Puedes gastar Ki para recuperar habilidades.  
> Pierdes un [?] de Ki o aplicas un segundo impacto.

Habilidades:

**Purificación corporal**

Abdomen de Marfil

Reguvenecion de Jade

Rafaga elemental

3 golpes de puro dolor

Golpe sanguineo

Puño ensordecedor

Habilidad de creador:

Estampida

Sentidos de alerta

Cancion de Dios

**Señor de las Armas**

Passiva:

Habilidades:

**Escudo Divino**Bloqueas un ataque y ciegas al enemigo.

**Ráfagas del Arsenal**Lanzas hasta tres armas que atacan a uno o varios enemigos.

**Arsenal del Rey**Invocas un arma que ataca por ti causando 1d6.

**Arsenal del Rey**Invocas un arma que ataca por ti causando 1d6.

**Ráfagas del Arsenal**Lanzas hasta tres armas que atacan a uno o varios enemigos.

**Cadena Real**El enemigo encadenado tira con desventaja 3 turnos.

**Corte Real**Cortas en un radio de 8 pies. Haces 2d6 de daño en área.

**Ráfagas del Arsenal**Lanzas hasta tres armas que atacan a uno o varios enemigos.

Habilidad de creador:

**Confianza Extrema:** Atacas con un aliado. Tu ataque se convierte en crítico. Tu aliado no aplica dados.

**Moral Inconmensurable:** Proporcionas +1 de CA a todos tus aliados.

**Destructor:** Ataca y te atacan con ventaja.

**Bardo de las 4 estaciones**

### Passiva

**Bardo de las 4 estaciones**

> Eliges un beneficio al día.

- **Primavera:** Tus curaciones curan +2.
- **Verano:** Ventaja en ataque.
- **Otoño:** Ventaja en defensa.
- **Invierno:** CA +2, ataques físicos reflejan 1 turno.

Habilidades:

**Canción del Otoño**Lanzas una melodía que causa desventaja a todos los enemigos que la oigan durante 2 turnos.

**Ola de Elementos**Puedes devolver una canción a mano.

**Bendición de las Estrellas**Bloquea un ataque.

**Canción de la Primavera**Curas 1d6 a los aliados cercanos.

**Invocación elemental**Un aliado obtiene 2 PA al final de su turno durante 2 turnos. Obtiene también ventaja en sus tiradas de ataque durante 2 turnos.

**Canción del Invierno**Congela a los enemigos cercanos. Los enemigos alcanzados quedan fuera de combate 2 turnos. Sus tiradas de defensa tienen desventaja.

**Canción del Verano**Lanza una bola de fuego que aplica 3d6 en un radio de 5 pies.

Habilidad de creador:

**Elevación del Ser**Permite a un jugador tener un turno adicional.

**Égida divina**Añade +2d6 a tirada de un aliado.

**Último Aliento**Si un aliado muere, volvéis a la vida con 1 PV.

**Apóstata**

### Passiva

**Apóstata**

> Cada vez que utilices Desgarro Sangriento para un contador, se reinicia cada descanso.

**Desgarro Sangriento:**

> Pierdes 1 punto cada vez que lanzas un hechizo o habilidad desde tu zona de reposo.

Habilidades:

**Escudo de Sombras**Acumulas 10 puntos de vida adicional como tope. Cada golpe te da 2 puntos.

**Absorción de Alma**Canaliza tus artes mágicas y absorbes 1d6 de la vida del oponente por turno. 3 turnos.

**Destructor del Escudo de Sombras**Explosión en un área de 4 pies. Causa tanto daño como vida acumula el escudo.

**Maldición de la debilidad**Aplica desventaja a un enemigo. 10 turnos.

**Invocación Oscura**Invocas a un aliado oscuro durante 10 turnos en combate.

**Marca Profana**Maldices a un enemigo haciendo que ataques con ventaja contra él durante 3 turnos. Le haces +1d6 de daño adicional.

**Estruendo desolador**Mediante un grito, dejas fuera de combate a 1 enemigo durante 1 turno. Sus tiradas de defensa tienen desventaja.

Habilidad de creador:

**Paso ilusorio**Haces un dash a spirit y creas una copia tuya en el lugar de origen. Cuesta el doble de recuperar con Desgarro Sangriento.

**Susurro Demoníaco**Tus ataques hacen 1d? a los enemigos adyacentes a tu objetivo durante 1 turno. Cuesta el doble de recuperar con Desgarro Sangriento.

**Ataque Maldito**Al golpear te hace X cantidad de daño y lo suma al daño realizado. Cuesta el doble de recuperar con Desgarro Sangriento.

Invocaciones:

Espectro Menor Fue 4 Des 2 Int 0 Car 5 6Vida

Cuervo Descompuesto Fue 1 Des 3 Int 0 Car 3 4Vida

**Furia de los Creadores**

### Passiva

**EN FURIA: Furia del creador**

> Cuando realizas un ataque puedes usar Magias Antiguas.

**Magias Antiguas:**

> Utiliza una carta de magia al azar.

Habilidades:

**Castigo Divino**Aplicas 1d6 de daño de luz durante 3 turnos.?**Ataque del Rayo**Paralizas a un enemigo y aplicas 1d6 de daño de rayo. 2 turnos.

**Protección del creador**Contrarresta todos los ataques 1 turno.

**Furia: Fe**Hace un ataque adicional, te cura el daño realizado.

**Furia Justiciera**Puedes reaccionar cuando atacan a un aliado y defenderlo. 1 turno.

**Ira Imparable**Tu ira se materializa y realiza un ataque que atrae al enemigo objetivo. 1 turno.

**Puño Volcánico**Tu arma inflige 2d6 de daño de fuego adicional. 2 turnos.

Habilidad de creador:

**Fuerzas internas**Golpea una ráfaga de 3 golpes consecutivos. Triplicando el daño realizado.

**A las armas**Realiza un ataque adicional.

**Desarme**Emite un grito que desarma a los enemigos.

**Mago chasqueador**

### Passiva

**Cascada letal**

> Por cada tirada puedes volver a lanzar 1 dado.

Després hi ha una condició que llegeixo aproximadament com:

> Si lanzas una tirada ≥ 20, gastas hasta 2 dados.

Habilidades:

**Bola de foc**

**Paso instantáneo**

**Sombra cegadora**

**Raig electrizant**

**Ocupa mi lugar**

**Espiral de Espinas**

**Recuperación predigitadora**

Habilidad de creador:

**Sin tregua**

**Repliegue mágico**

**Sin sentido**
## D. Bandeja de decisiones

Las **56 decisiones numeradas** viven en el apartado 12 de cada flujo y se vuelcan automáticamente en `PLAN_ALCANCE_Y_VISION.md` §7 y en la página HTML. Convención de identificadores:

| Prefijo | Flujo | Cuántas |
|---|---|---|
| `D-COMB-nn` | WF-01 combate | 7 |
| `D-MOV-nn` | WF-02 movimiento | 5 |
| `D-COM-nn` | WF-03 comercio | 8 |
| `D-MOR-nn` | WF-04 moral y reputación | 9 |
| `D-NEG-nn` | WF-05 negocios y rentas | 9 |
| `D-EQU-nn` | WF-06 equipo y progresión | 9 |
| `D-MIS-nn` | WF-07 misiones | 9 |

**Cómo cerrarlas**: cada decisión se convierte en una entrada `D00xx` de `decisiones/` con formato MADR cuando Oriol la responde. Mientras estén abiertas, la recomendación escrita en el flujo es la que se asume al planificar, y se dice en voz alta que es una suposición.

**Las cinco que desbloquean más trabajo** (por número de flujos que dependen de ellas):

1. **D-NEG-01 / D-COM-07** — calendario de mundo contra reloj por pasos. Bloquea WF-03, WF-05, FS-02, FS-05, FS-06.
2. **D-MOR-02** — una sola moral. Bloquea AV0-01, WF-03, WF-04, WF-05.
3. **D-COMB-01 y D-COMB-02** — el reglamento de dados en el documento. Bloquea AV5-03, AV1-04, AV12-04 y cualquier prueba de balance.
4. **D-MOV-01** — celdas contra continuo. Bloquea toda la fase AV7 y el diseño de FS-01.
5. **D-MIS-01** — no duplicar el contrato de misiones. Bloquea AV10-01 antes de que alguien escriba un segundo diario.
