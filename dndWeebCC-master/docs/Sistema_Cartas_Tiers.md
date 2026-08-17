# Sistema de Cartas y Tiers — Documento de Diseño

*Readaptación de una estructura de rol de fantasía heroica inspirada en la arquitectura clásica de atributos, clases, hechizos y equipo, rediseñada para progresión por tiers y personaje-como-mazo-de-cartas. No reproduce texto, tablas ni reglas propietarias de ningún sistema comercial; toda la terminología, fórmulas y contenido mecánico son originales de este documento.*

> **Edición 2 — notas de la revisión.** Las secciones 1-13 (reglas base) reflejan la edición 2 del sistema:
> stats CON/DES/INT/CAR (sustituye a Sabiduría por Carisma, con responsabilidades repartidas para que
> ningún stat "lo haga todo"), sistema de recursos por **pilas de cartas** (Activa / Descanso Corto /
> Descanso Largo) en vez de puntos de energía/maná, cartas de Invocación independientes, cartas
> evolutivas, fórmula de Multiclase, símbolos de compatibilidad de equipo, y resolución de tiradas
> por **pool de d6 con éxitos y medios éxitos** (sección 7) en vez de `1d20 + modificador`. Las
> secciones 14-18 (clases, razas, meses de nacimiento, cartas de ejemplo y personajes de ejemplo)
> están regeneradas para la edición 2: usan CAR, el sistema de pilas (`recovery`), los símbolos de
> compatibilidad y la fórmula de vida unificada. Todo el documento es contenido jugable.

---

## Índice

1. [Visión general y principios de diseño](#1-visión-general-y-principios-de-diseño)
2. [Estructura del personaje](#2-estructura-del-personaje)
3. [Sistema de tiers](#3-sistema-de-tiers)
4. [Stats](#4-stats)
5. [Cálculo de vida](#5-cálculo-de-vida)
6. [Recursos](#6-recursos)
7. [Acciones de combate](#7-acciones-de-combate)
8. [Tags mecánicos](#8-tags-mecánicos)
9. [Sistema de cartas — plantillas y estructura](#9-sistema-de-cartas)
10. [Restricciones de balance](#10-restricciones-de-balance)
11. [Progresión de personaje](#11-progresión-de-personaje)
12. [Guía para diseñar cartas nuevas](#12-guía-para-diseñar-cartas-nuevas)
13. [Estructura de archivos / base de datos](#13-estructura-de-archivos)
14. [Clases iniciales](#14-clases-iniciales)
15. [Razas iniciales](#15-razas-iniciales)
16. [Trasfondos iniciales: los 12 meses de nacimiento](#16-trasfondos-iniciales-los-12-meses-de-nacimiento)
17. [Cartas de ejemplo](#17-cartas-de-ejemplo)
18. [Personajes de ejemplo](#18-personajes-de-ejemplo)
19. [Presentación visual: plantilla de carta y mesa de personaje](#19-presentación-visual-plantilla-de-carta-y-mesa-de-personaje)
20. [Diseño de aventuras: mazos de historia por actos](#20-diseño-de-aventuras-mazos-de-historia-por-actos)

---

## 1. Visión general y principios de diseño

Este sistema mantiene la fantasía heroica, la exploración y el combate táctico de los juegos de rol de mesa clásicos, pero sustituye la progresión por niveles (1-20) por una progresión por **tiers**, y representa cada elemento jugable del personaje —clase, raza, trasfondo, pasivas, habilidades, hechizos y equipo— como una **carta** física o digital con reglas propias.

### Principios rectores

**Sin niveles tradicionales.** La progresión ocurre en 6 escalones (Tier 0 a Tier 5), no en 20 pasos incrementales. Cada tier es un salto de poder y complejidad perceptible, no un ajuste marginal.

**Todo elemento importante es una carta.** Si algo afecta las reglas del personaje, existe como carta: se puede leer, intercambiar, coleccionar y mostrar sobre la mesa. La ficha de personaje es, en la práctica, un tablero donde se colocan cartas activas.

**Personalización modular.** Raza, clase y trasfondo no son paquetes cerrados; son puntos de partida que aportan cartas y aperturas hacia otras cartas, siempre dentro de límites de balance explícitos.

**El balance no depende de los dados.** Los dados resuelven acciones puntuales (ataques, pruebas, salvaciones), pero el equilibrio estructural del juego viene de stats base, equipo, tier, coste de habilidades, número de cartas permitidas y restricciones por rol y clase.

**Cuatro stats, no seis.** CON, DES, INT y CAR cubren todo lo que en otros sistemas se reparte entre seis u ocho atributos. Menos números que trackear, más peso específico por punto invertido. Ninguno de los cuatro es "el stat correcto": cada uno domina una parcela distinta del juego (ver sección 4) para que no exista una build objetivamente óptima.

**La vida es determinista.** Ningún dado decide cuánta vida gana un personaje al avanzar de tier. La vida sale de una fórmula fija: clase + CON + CAR + tier + equipo (ver sección 5).

**El equipo es una palanca de balance, no decoración.** Los objetos aportan stats, habilidades y penalizaciones tácticas; son la principal herramienta para ajustar a un personaje sin tocar sus cartas de clase.

**Cada clase es jugable desde el primer minuto.** Toda clase inicial trae consigo una carta de clase, una pasiva única, un equipo básico y un número fijo de habilidades — nadie empieza con una hoja en blanco.

**Una clase es un paquete de mecánicas, no una lista de bonificadores.** Dos clases con las mismas stats deben *jugarse* de forma distinta gracias a su pasiva y a cómo gestionan sus cartas de habilidad (ver "Filosofía de clase" en la sección 10), no solo tener números diferentes en la misma fórmula.

**Los recursos son cartas, no barras de puntos.** Una habilidad usada no "cuesta maná": se mueve a una pila de recuperación (Descanso Corto o Descanso Largo) y vuelve a la mano cuando corresponde. El estado de tus recursos se lee mirando dónde están tus cartas, no un número (ver sección 6).

---

## 2. Estructura del personaje

Un personaje **es** una combinación de cartas, una por cada módulo. Cada módulo es independiente y
se puede ampliar (nuevas clases, razas, invocaciones...) sin reescribir el reglamento de los demás.
Sus componentes:

1. **Carta de Clase** — 1, obligatoria (2 si el personaje hace Multiclase, ver sección 11).
2. **Carta de Raza** — 1, obligatoria.
3. **Carta de Personalidad/Trasfondo** — 1, obligatoria. No da stats: da reglas sociales (virtud,
   defecto, objetivo) que generan inspiración, puntos heroicos o favores narrativos.
4. **Carta Pasiva de Clase** — 1, otorgada por la clase, única e intransferible. Es la identidad
   mecánica de la clase: nunca se elimina y nunca hay una segunda pasiva de clase activa a la vez
   (ver sección 9.4).
5. **Cartas de Habilidad** — el núcleo del juego. Se reparten entre tres pilas (Activa, Descanso
   Corto, Descanso Largo — ver sección 6), no en "slots" fijos.
6. **Cartas de Equipo** — una por slot ocupado. Cada pieza aporta bonos de stat y, opcionalmente,
   una habilidad propia vinculada (`linkedSkill`).
7. **Cartas de Objeto Consumible** — limitadas por un slot de inventario de consumibles.
8. **Cartas de Invocación** — si el personaje tiene acceso a invocar criaturas, cada una es una
   carta independiente con su propia vida, ataques, movimiento y pasiva (ver sección 9.10).
9. **Carta de Deidad/Afinidad** *(opcional)* — vínculo narrativo y mecánico con un poder superior,
   relevante sobre todo para CAR (ver sección 9.11).
10. **Cartas de Rasgo Especial** — otorgadas por raza, trasfondo o hitos narrativos.
11. **Carta de Estadísticas** — donde se suman todos los modificadores de CON/DES/INT/CAR aportados
    por raza, equipo, pasivas, objetos, maldiciones y bendiciones. Los stats base de un personaje
    **no se suben directamente**: solo cambian a través de estas fuentes (ver sección 4).
12. **Tier actual** — de 0 a 5.
13. **Vida y defensas calculadas** — resultado de las fórmulas de la sección 5 (vida, clase de
    armadura, defensa mental, resistencia física, precisión, iniciativa).

La ficha de personaje física recomendada es una superficie de juego (playmat) con zonas marcadas:
clase/raza/trasfondo, pasiva, las tres pilas de habilidad (Activa / Descanso Corto / Descanso
Largo), equipo por slot, consumibles, invocaciones activas y un contador de vida y defensas.

---

## 3. Sistema de tiers

El tier reemplaza al nivel como medida de progresión. No es solo "más poder": cada tier desbloquea complejidad mecánica nueva (slots, tipos de carta, recursos).

| Tier | Nombre | Descripción | Límite de mano | Rareza de equipo típica |
|---|---|---|---:|---|
| 0 | Civil / Aprendiz | Personaje sin entrenamiento formal. Prólogos, supervivencia, PNJ relevantes. | 6 | Común |
| 1 | Aventurero Inicial | Inicio estándar de campaña. Set de clase completo. | 10 | Común / poco común |
| 2 | Aventurero Competente | Primeras especializaciones y mejoras de equipo relevantes. Multiclase disponible (baja a 12). | 15 | Rara |
| 3 | Héroe Consolidado | Combinaciones potentes, cartas avanzadas de clase. Multiclase disponible (baja a 16). | 20 | Épica |
| 4 | Campeón / Maestro | Alto impacto narrativo y táctico, cartas únicas. | 24 | Épica / legendaria |
| 5 | Mítico *(opcional)* | Reservado para campañas épicas; cartas extremadamente poderosas y muy restringidas. | 28 | Legendaria |

Notas:

- **"Límite de mano"** cuenta solo cartas de habilidad (ver sección 11.1). Raza, pasiva de clase y equipo están siempre activos y no ocupan hueco de mano.
- El equipo siempre usa los mismos **6 slots fijos** (sección 9.7) en cualquier tier; lo que cambia con el tier es la rareza/poder de lo que se puede encontrar o equipar, no el número de slots.
- Regla de oro de objetos mágicos: un personaje solo puede llevar **un objeto de rareza épica o legendaria a la vez**, salvo que su clase o pasiva lo permita explícitamente — evita acumular poder desmedido.
- Cada carta declara un **tier mínimo**; nunca se puede jugar por debajo de ese mínimo, aunque sobren slots.
- Subir de tier no es automático por "puntos de experiencia acumulados": lo marca el narrador/director de juego al cierre de un arco narrativo, sesión hito o logro relevante, de forma similar a un sistema de nivel por hitos.
- El Tier 5 es opcional y debe anunciarse desde el inicio de campaña; su contenido está pensado para ser raro incluso dentro de una campaña que lo incluya.
- Al subir de tier, un personaje puede además optar por hacer **Multiclase** en vez de una mejora normal de su clase actual (ver sección 11.1): no da cartas extra, reparte el mismo total entre dos clases.

---

## 4. Stats

Cuatro atributos, puntuados normalmente entre 1 y 6 en personajes de Tier 0-1, pudiendo llegar a
10+ en Tier 5. **Ningún stat "lo hace todo".** Cada uno es responsable de una parcela del juego
distinta y ninguno interviene en la fórmula de otro, para que no exista un stat objetivamente mejor
que los demás (ver "Las fórmulas" al final de esta sección).

### CON — Constitución
Resistencia física, salud, aguante.
Gobierna: **Vida** (junto con CAR), **Resistencia física** (venenos, enfermedades, sangrado, fatiga),
uso de armadura pesada, aguantar heridas graves.

### DES — Destreza
Agilidad, reflejos, coordinación.
Gobierna: **Clase de Armadura**, **Iniciativa**, movimiento, esquiva, precisión con armas ligeras
o a distancia.

### INT — Inteligencia
Conocimiento, lógica, técnica, magia arcana.
Gobierna: **Precisión mágica**, número máximo de invocaciones activas simultáneas, conocimiento,
rituales, uso de objetos mágicos.

### CAR — Carisma
Voluntad, determinación, presencia, conexión con lo divino.
Gobierna: **Defensa mental** (resistencia a miedo, control, encantamiento), afinidad divina,
liderazgo, persuasión, control sobre las criaturas invocadas propias, y aporta a la **Vida** junto
con CON (representa moral y determinación: un líder carismático también es más difícil de abatir).

### Las fórmulas

Este es el punto más delicado del sistema: si un stat domina las fórmulas de los demás, todo el
mundo acaba optimizando hacia el mismo personaje. Por eso cada fórmula de la sección 5 usa un stat
distinto como responsable único, y ningún stat aparece en más de dos fórmulas:

| Stat | Aparece en |
|---|---|
| CON | Vida (junto a CAR), Resistencia física |
| DES | Clase de Armadura, Iniciativa |
| INT | Precisión mágica, nº de invocaciones |
| CAR | Vida (junto a CON), Defensa mental |

---

## 5. Fórmulas: vida y defensas

Todas las fórmulas de esta sección son 100% deterministas. Ningún dado decide cuánta vida o
defensa gana un personaje al avanzar de tier o equiparse. Cada fórmula depende de un único stat
"responsable" (más el equipo y el tier, que son transversales) — así ningún stat compite con otro
por ser "el bueno" (ver sección 4).

### Vida

```
Vida = min(
  Vida Base de Clase + (CON × 3) + (CAR × 1) + Bono de Tier + Bono de Equipo,
  Techo de vida por tier
)
```

- **Vida Base de Clase**: valor fijo definido en la carta de clase; los roles con más presencia en
  primera línea (tanque, equilibrado) tienen una base más alta que los roles frágiles (mago,
  invocador), pero el multiplicador de CON y CAR es el **mismo número fijo para todas las clases**
  — el diferenciador de rol es la vida base y el equipo, no una fórmula distinta por clase.
- **CON × 3**: la resistencia física siempre pesa más que la determinación.
- **CAR × 1**: un líder con presencia también aguanta más — representa moral y determinación, no
  solo dureza física.
- **Bono de Tier**: ver tabla de bonos de tier (sin cambios respecto a la edición 1).
- **Bono de Equipo**: suma de bonos de vida directos otorgados por objetos equipados. No todo el
  equipo aporta vida.
- **Techo de vida por tier**: límite duro. Por mucho que un personaje apile CON, CAR, equipo y
  bonos, su vida no puede superar el techo de su tier. Un jugador no puede "comprar" más vida más
  allá de lo que su tier permite — es la regla fundamental contra el power creep.

> **⚠️ Deuda técnica (PENDIENTE de implementar).** Esta fórmula con techo es la especificación
> canónica de la edición 2. Hoy `CalculadoraVida.java` aún usa la fórmula legacy por clase
> (`vidaBase + multiplicadorCon × CON + bonoTier`, sin CAR ni techo). Migrar el código Java y
> recalcular `healthScaling`/`baseHealth` de las clases a `data/cartas/clases/` es la tarea que
> cierra esta sección. Hasta entonces, los personajes generados por la app quedan fuera del
> esquema de techo descrito aquí.

### Techo de vida por tier

| Tier | Techo de vida |
|---:|---:|
| 0 | 27 (o −25% sobre el techo de Tier 1) |
| 1 | 35 |
| 2 | 50 |
| 3 | 70 |
| 4 | 95 |
| 5 | 125 |

El techo es el verdadero garante del balance de vida: por muchas fuentes de bono que se acumulen,
la vida se satura en el valor del tier. El diferenciador entre un tanque y un mago del mismo tier
es cuánto se acercan a ese techo (el tanque vive en él, el mago suele quedar bastante por debajo),
no si pueden pasarlo.

### Bono de vida por tier

| Tier | Bono de vida acumulado |
|---:|---:|
| 0 | +0 (o −25% sobre el resultado de Tier 1, ver Tier 0 abajo) |
| 1 | +0 (línea base) |
| 2 | +4 |
| 3 | +9 |
| 4 | +16 |
| 5 | +25 |

El Tier 0 no tiene fórmula propia: se calcula la vida de Tier 1 y se reduce un 25% (redondeo hacia
abajo), reflejando que el personaje aún no tiene set de clase completo.

### Clase de Armadura (CA)

```
Clase de Armadura = 10 + DES + Bono de Armadura
```

La Constitución no interviene aquí — la CA es pura agilidad más lo que lleves puesto. `Bono de
Armadura` sale de `statBonuses` de la carta de equipo (sección 9.7); una armadura pesada puede
limitar cuánto DES cuenta (ver `penalties` en la misma carta) en vez de restar directamente a la CA.

### Defensa mental

```
Defensa mental = 10 + CAR + Bonos raciales o de pasiva
```

Resiste miedo, encantamiento, dominación y presión social/sobrenatural. Es el equivalente mental
a la CA: mismo "10 + stat" que el resto de defensas, para que las cuatro fórmulas se lean igual de
un vistazo.

### Resistencia física

```
Resistencia física = 10 + CON
```

Resiste venenos, enfermedades, sangrado y fatiga. Independiente de la vida: un personaje puede
tener mucha vida y poca resistencia física si su CON es baja y su vida viene sobre todo de CAR y
equipo.

### Precisión con armas

```
Precisión con armas = Atributo indicado por el arma (normalmente DES o CON, según el tipo de arma)
```

Cada carta de equipo de tipo arma declara qué stat usa para precisión (ver `precisionStat` en la
sección 9.7): armas ligeras y a distancia suelen usar DES, armas pesadas a dos manos suelen usar
CON. Esto es a propósito distinto de la CA (que siempre es DES): dos personajes con la misma CA
pueden golpear de forma muy distinta según su arma.

### Precisión mágica

```
Precisión mágica = 10 + INT
```

Se aplica a hechizos y habilidades con `castingStat: INT`. Las cartas con `castingStat: CAR`
(magia divina/afinidad, ver sección 9.6) usan **Defensa mental** como referencia de poder en vez
de esta fórmula — no hay una "precisión CAR" separada, reutiliza la misma fórmula que ya
existe para no multiplicar sistemas de precisión.

### Iniciativa

```
Iniciativa = DES + Modificadores (raza, pasiva, equipo)
```

### Ejemplo de cálculo

Guardián de Hierro (Tanque, vida base 16), CON 4, CAR 2, Tier 3, escudo reforzado (+2 vida):

```
Vida = 16 + (4 × 3) + (2 × 1) + 9 + 2 = 16 + 12 + 2 + 9 + 2 = 41
```

Con DES 2 y sin armadura, su Clase de Armadura sería `10 + 2 + 0 = 12`; con CAR 2, su Defensa
mental sería `10 + 2 = 12`. Las cuatro fórmulas se calculan de forma independiente: cambiar el
equipo del Guardián sube su CA sin tocar su vida ni su defensa mental.

---

## 6. Recursos: el sistema de pilas

No hay barras de puntos (maná, energía...) que trackear turno a turno. El recurso de un personaje
**es literalmente dónde están sus cartas de habilidad**. Cada carta de habilidad indica en su campo
`recovery` a qué pila va cuando se usa:

| Pila | Qué contiene | Cuándo vuelven las cartas |
|---|---|---|
| **Activa** | Cartas de habilidad disponibles para jugar ahora mismo. | — (es el punto de partida) |
| **Descanso Corto** | Cartas usadas con `recovery: descanso_corto`. | Al hacer un descanso corto: toda la pila vuelve a la mano/Activa. |
| **Descanso Largo** | Cartas usadas con `recovery: descanso_largo`, y también cualquier carta de `recovery: descanso_corto` que no se haya podido recuperar todavía. | Solo al hacer un descanso largo. |

Reglas de la pila:

1. Al jugar una carta de habilidad, esta **se mueve** de la pila Activa a la pila que indique su
   campo `recovery`. No se "gasta": sigue existiendo, solo no está disponible hasta el descanso
   correspondiente.
2. `recovery: ninguno` significa que la carta no se mueve de sitio (habilidades pasivas o de uso
   libre, normalmente de bajo impacto).
3. Un vistazo a la mesa de juego dice todo lo que dice una barra de recursos: cuántas cartas te
   quedan en Activa, y qué tienes esperando en cada pila de descanso.
4. Esto sustituye por completo al antiguo sistema de energía/maná/aguante/foco por puntos: ninguna
   carta de la edición 2 tiene coste en puntos de recurso, solo `recovery`.

---

## 7. Resolución de acciones y combate

### 7.1 La tirada de dados: pool de d6

Toda acción con posibilidad de fallo (atacar, esquivar, convencer, investigar) se resuelve igual:
tira tantos **d6** como indique el valor de la característica implicada en la acción.

**Ejemplo:** Destreza 5 → tiras 5d6.

Cada dado aporta una cantidad de éxitos:

| Resultado | Valor |
|---|---:|
| 6 | +1 éxito |
| 5 | +0,5 éxitos |
| 1-4 | 0 éxitos |

Se suman todos los éxitos obtenidos. **Ejemplo:** 4d6 → 6, 5, 2, 1 = 1 (del 6) + 0,5 (del 5) + 0 +
0 = **1,5 éxitos**.

El resultado debe igualar o superar una **Dificultad (CD)**:

| Dificultad | Descripción |
|---:|---|
| 0,5 | Fácil |
| 1 | Normal / Desafiante |
| 1,5 | Difícil |
| 2 | Muy difícil |
| 2,5+ | Extraordinaria |

- CD de una habilidad de enemigo o de una acción narrativa (investigar, escalar...): la fija el
  director de juego con la tabla anterior.
- Las cuatro fórmulas de defensa de la sección 5 (CA, Defensa mental, Resistencia física, Precisión
  mágica) siguen siendo el número fijo `10 + stat (+ bonos)` y se usan tal cual cuando la acción
  enfrenta una defensa fija (p. ej. atacar la CA de un enemigo). La tabla de dificultades
  (0,5-2,5+) es para pruebas sin un enfrentamiento numérico ya definido (habilidad narrativa,
  prueba de característica, salvación).

**Tiradas enfrentadas.** Cuando dos personajes compiten directamente (forcejeo, sigilo contra
percepción, persuasión contra voluntad), ambos tiran con la característica correspondiente y gana
quien obtenga más éxitos. Empate: ninguno se impone, ambos logran un éxito parcial, o lo resuelve
el director de juego según la escena.

**Éxito crítico:** si todos los dados de la tirada muestran 6, el efecto es máximo o se obtiene un
beneficio extra a discreción del director de juego. **Fracaso crítico (pifia):** si la tirada
obtiene 0 éxitos, el director de juego puede añadir una complicación (caer, romper el arma, recibir
daño, sufrir un contraataque).

El daño de un ataque que impacta se calcula **con esta misma tirada**, sin dados adicionales
(ver sección 7.6).

### 7.2 Ventaja y desventaja

Ventaja y desventaja no cambian el número de dados tirados: cambian qué caras cuentan como éxito.

**Ventaja** — el 4 también vale medio éxito:

| Resultado | Valor |
|---|---:|
| 6 | +1 éxito |
| 5 | +0,5 éxitos |
| 4 | +0,5 éxitos |
| 1-3 | 0 éxitos |

**Desventaja** — solo el 6 cuenta:

| Resultado | Valor |
|---|---:|
| 6 | +1 éxito |
| 1-5 | 0 éxitos |

Ventaja y desventaja **no se acumulan**: si hay varias fuentes de ventaja y una de desventaja, se
cancelan entre sí y se tira con la tabla normal (nunca se combinan varias ventajas para ampliar el
rango de éxito más allá del 4, ni varias desventajas para reducirlo por debajo del 6).

Fuente típica de desventaja: usar un arma o armadura fuera de la compatibilidad de la clase (ver
sección 9.7) — △ Ligera nunca penaliza, ○ Media dentro de una compatibilidad más ligera da
desventaja en tiradas de ataque, □ Pesada fuera de compatibilidad da desventaja **y** reduce la
velocidad de movimiento a la mitad.

---

### 7.3 Estructura del turno

Al empezar un combate, el orden de turno se ordena por **Iniciativa** (fórmula determinista de la
sección 5: `DES + Modificadores`, sin tirada) de mayor a menor; se mantiene toda la escena salvo que
una carta diga lo contrario. En caso de empate en Iniciativa, decide quien tenga mayor DES base, y
si persiste, lo decide el director de juego. Cada turno, un personaje dispone de:

- **1 Acción principal** (⚔) — atacar, jugar una carta de habilidad, usar un objeto.
- **1 Acción de Movimiento** (👣) — moverte tu velocidad base, o jugar una carta de movimiento.
- **1 Reacción** (🛡) por ronda — se juega fuera de tu turno, en respuesta a un disparador (p. ej.
  recibir un ataque).
- **1 Canalización** (⏳) — sustituye a la acción principal y de movimiento del turno: la habilidad
  tarda el turno entero en completarse.
- **Acciones libres**, ilimitadas por defecto salvo que una carta las restrinja (hablar, soltar un
  objeto, un gesto).

Cada carta jugable declara qué tipo de acción consume, usando uno de estos valores de campo `actionType`:

`accion`, `accion_menor`, `reaccion`, `pasiva`, `movimiento`, `preparacion`, `canalizacion`, `consumible`.

`canalizacion` implica que la habilidad permanece activa mientras el personaje mantenga concentración (equivalente conceptual a "concentración"), y se pierde si el personaje recibe daño y falla una prueba de mantenimiento (CAR o CON, a definir por la carta).

### 7.4 Condiciones

Las condiciones modifican temporalmente las capacidades de un personaje. La carta o efecto que las
provoca especifica su duración; si no la especifica, dura hasta el final de la escena.

| Condición | Efecto |
|---|---|
| Sangrado | Al empezar tu turno, recibes 1d4 de daño (o el que indique la fuente). Se cura con un descanso corto o una habilidad de curación. |
| Caído | Desventaja en ataques. Los ataques cuerpo a cuerpo contra ti tienen ventaja. Levantarte cuesta tu Acción de Movimiento. |
| Agarrado | Tu velocidad baja a 0 y no puedes esquivar. Para soltarte, supera una tirada de CON contra la CD de quien te agarra. |
| Cegado | Desventaja en ataques; las habilidades que requieren ver al objetivo fallan automáticamente. |
| Encantado | No puedes atacar a quien te ha encantado; sus interacciones sociales contigo tienen ventaja. |
| Asustado | Desventaja en ataques y tiradas de habilidad mientras veas la fuente del miedo; no puedes acercarte a ella voluntariamente. |
| Inconsciente | Caes (condición Caído). No puedes actuar ni moverte. Los ataques contra ti tienen ventaja y son críticos si aciertan. |
| Paralizado | No puedes moverte. Desventaja en salvaciones de DES y CON. Los ataques cuerpo a cuerpo contra ti tienen ventaja. |
| Envenenado | Desventaja en tiradas de ataque y de habilidad. Puede evolucionar a daño continuo si no se trata. |
| Fatiga | Cada nivel de Fatiga resta 1 dado a todas las tiradas de característica (mínimo 1 dado mientras la característica sea mayor que 0). Al llegar a nivel 6, el personaje muere. Se recupera con un descanso largo (2 niveles por descanso). |

### 7.5 Muerte y agonía

Llegar a 0 de Vida no es muerte instantánea salvo que el efecto lo indique explícitamente.

1. **Moribundo.** A 0 de Vida, el personaje cae Moribundo: no puede hacer acciones principales ni
   de movimiento, solo hablar con esfuerzo y una acción libre.
2. **Salvación por muerte.** Al empezar tu turno, tira tantos d6 como tu CON contra CD 1 (Normal),
   sin más bonificadores salvo que una carta lo permita. Éxito (≥1 éxito): te estabilizas a 1 de
   Vida. Fracaso (0,5 éxitos): bajas 1 punto de Vida (a negativo). Fracaso crítico (0 éxitos): bajas
   2 puntos.
3. **Muerte instantánea.** Si la Vida baja de `−CON` (en negativo) o llega a −10, el personaje
   muere sin posibilidad de salvación.
4. **Carga del Destino** *(una vez por personaje)*. En vez de morir definitivamente, el jugador
   puede aceptar una cicatriz permanente (una limitación narrativa/mecánica concreta, p. ej. perder
   un ojo) y perder 2 puntos de un stat al azar, a cambio de sobrevivir.
5. **Recuperación.** Descanso corto (1h): gasta dados de golpe (1d8 por tier) para curarte y
   recupera la pila de Descanso Corto. Descanso largo (8h): recupera toda la Vida, todas las pilas,
   y reduce la Fatiga en 2 niveles.

### 7.6 Cálculo de daño

No existen dados de daño separados: **la misma tirada que impacta genera el daño**. Un solo
mecanismo de dados (el pool de d6 de 7.1) resuelve todo el combate.

```
Daño = Éxitos de la tirada de ataque × Multiplicador del arma + Bonos fijos de la carta
```

| Categoría de peso del arma | Multiplicador |
|---|---:|
| △ Ligera (y ataques desarmados con entrenamiento) | ×1 |
| ○ Media | ×1,5 |
| □ Pesada | ×2 |
| Improvisada / desarmado sin entrenamiento | ×0,5 |
| Hechizos y habilidades mágicas | ×1,5 (salvo que la carta declare otro) |

Reglas:

1. **Redondeo hacia abajo** del total final, mínimo 1 de daño si el ataque impactó.
2. **Bonos fijos después del multiplicador.** Los efectos de carta tipo "suma tu CON al daño" o
   "+2 contra objetivos Quemados" se añaden al resultado, no se multiplican.
3. **Éxito crítico (todos 6):** el daño se calcula como si cada dado hubiera aportado 1 éxito
   completo (éxitos = número de dados), en línea con el "efecto máximo" de 7.1.
4. **El arma no lleva dado propio.** Su contribución mecánica es su categoría de peso
   (multiplicador automático), su `precisionStat` y sus tags/habilidad vinculada. Cualquier
   dado heredado de material legacy (1d8, 1d10...) se conserva solo como texto de sabor y se
   mapea así: d4/d6 → △ · d8 → ○ · d10/d12 → □.
5. **Compatibilidad:** este multiplicador es independiente de la penalización por peso fuera de
   compatibilidad (7.2) — un arma □ fuera de compatibilidad sigue multiplicando ×2, pero se tira
   con desventaja y mueve a mitad de velocidad.

**Ejemplo:** Renn (DES 4) ataca con Dagas Gemelas (△ ×1) a un enemigo flanqueado usando *Golpe
Preciso* ("daño adicional igual a tu DES si el objetivo está flanqueado"). Tira 4d6: 6, 5, 5, 2 =
2 éxitos. Impacta y hace 2 × 1 + 4 = **6 de daño**. Con un mandoble (□ ×2) la misma tirada haría
2 × 2 + 4 = 8, pero Renn tiraría con desventaja por incompatibilidad de su clase (armas △).

---

## 8. Tags mecánicos

Los tags son metadatos que permiten combos, restricciones, filtrado y organización de colección. Cada carta puede llevar varios.

**Elementales / de daño:** `[Fisico]` `[Magico]` `[Fuego]` `[Hielo]` `[Veneno]` `[Sagrado]` `[Oscuro]`

**De origen mágico/marcial:** `[Marcial]` `[Arcano]` `[Divino]` `[Naturaleza]`

**Tácticos:** `[Bloqueo]` `[Evasion]` `[Curacion]` `[Control]` `[Invocacion]` `[Concentracion]` `[Area]` `[Proyectil]`

**De equipo:** `[Armadura Pesada]` `[Armadura Media]` `[Armadura Ligera]` `[Arma Ligera]` `[Arma Pesada]` `[Escudo]`

**Otros:** `[Reaccion]` `[Consumible]` `[Ritual]` `[Sigilo]`

Los tags se usan en tres formas dentro de una carta:

1. **`mechanicTags`** — describen qué es la carta (para búsqueda y combos positivos).
2. **`requiredTags`** — condiciones de equipo o estado necesarias para poder jugar la carta (p. ej. requiere `[Escudo]` equipado).
3. **`incompatibleTags`** — si el personaje tiene activa una carta con un tag de esta lista, no puede jugar/equipar la carta actual.

### Símbolos de compatibilidad

Para equipo y clases, en vez de repetir texto largo (`allowedEquipmentTags`), se usa un pequeño
set de símbolos legibles de un vistazo tanto en texto como en la carta impresa:

**Peso de equipo:** △ Ligero · ○ Medio · □ Pesado

**Afinidad:** ✦ Arcano · ☠ Maldito · ✝ Sagrado · ♞ Montado · ⚙ Mecánico

Una carta de clase declara su compatibilidad con estos símbolos (p. ej. "Compatible: △ Armas
ligeras, △ Armaduras ligeras"); una carta de equipo declara los suyos propios. Si no coinciden, el
equipo no es utilizable por esa clase sin una carta que lo habilite explícitamente. No se
combinan más de dos símbolos por carta — si hace falta más matiz, es una restricción textual en
`restrictions`, no un símbolo nuevo.

### Iconos de tipo de carta

Cada carta jugable de habilidad/hechizo lleva un icono según su `actionType`, para reconocer de un
vistazo qué hace sin leer el texto:

| Icono | Tipo |
|---|---|
| ⚔ | Acción (marcial) |
| ✨ | Encantamiento / hechizo |
| 🛡 | Reacción |
| 👣 | Movimiento |
| ❤ | Curación |
| 👻 | Invocación |
| ⏳ | Canalización |
| 🌑 | Ritual |

---

## 9. Sistema de cartas

Ver documento `01_clases.md`, `02_razas_trasfondos.md` y `03_cartas_ejemplo.md` para las plantillas completas por tipo de carta y ejemplos. La estructura de campos común a **toda** carta:

| Campo | Obligatorio | Descripción |
|---|---|---|
| `id` | Sí | Identificador único, `snake_case`, con sufijo de tier cuando aplica (`_t1`, `_t2`...) |
| `name` | Sí | Nombre mostrado |
| `type` | Sí | `class` \| `race` \| `background` \| `passive` \| `skill` \| `spell` \| `equipment` \| `consumable` \| `trait` \| `summon` \| `deity` |
| `tier` | Sí | Tier mínimo requerido (0-5) |
| `rarity` | No | `common` \| `uncommon` \| `rare` \| `epic` \| `mythic` |
| `classTags` | No | Clases compatibles (vacío = universal) |
| `roleTags` | No | Roles compatibles (`tank`, `striker`, `support`, `controller`...) |
| `mechanicTags` | No | Ver sección 8 |
| `requiredStats` | No | Mínimos de CON/DES/INT/CAR para poder jugar la carta |
| `requiredTags` | No | Tags de equipo/estado necesarios |
| `incompatibleTags` | No | Tags que bloquean el uso conjunto |
| `recovery` | Cuando aplica (cartas de habilidad/hechizo) | `activa` \| `descanso_corto` \| `descanso_largo` \| `ninguno` — a qué pila va la carta tras usarse (sustituye al antiguo `cost`/recurso por puntos, ver sección 6) |
| `actionType` | Cuando aplica | Ver sección 7 |
| `typeIcon` | Cuando aplica | Uno de los iconos de la sección 8 (⚔ ✨ 🛡 👣 ❤ 👻 ⏳ 🌑) |
| `range` | Cuando aplica | `melee` \| `short` \| `medium` \| `long` \| `self` |
| `duration` | Cuando aplica | `instant` \| `1_turn` \| `1_min` \| `concentration` \| `permanent` |
| `effect` | Sí (salvo cartas puramente narrativas) | `{ description, scaling }` |
| `limitations` | No | Lista de restricciones textuales |
| `evolvesInto` | No | ID de la versión evolucionada de esta carta (ver "Cartas evolutivas", sección 9.12); sustituye a la carta actual sin ocupar un hueco extra |
| `flavorText` | No | Texto narrativo/ambientación |

Cada tipo de carta añade campos propios; se detallan en las plantillas de la sección 13.

---

## 10. Restricciones de balance

### Filosofía de clase

Antes de las reglas numéricas: **una clase es un paquete de mecánicas, no una lista de
bonificadores.** Dos clases con las mismas stats deben jugarse de forma distinta porque gestionan
sus pilas de habilidad de forma distinta (una vive de vaciar la pila Activa rápido y descansar
seguido; otra guarda una carta cara en Descanso Largo toda la partida) y porque su pasiva cambia
una regla del juego, no un número. Al diseñar una clase nueva, la primera pregunta no es "¿qué
bonificador da?" sino "¿qué decisión distinta obliga a tomar cada turno?".

1. **Límite de cartas activas por tier** — tabla de la sección 3. Se cuenta en juego, no en colección: un personaje puede *poseer* más cartas de las que puede tener activas a la vez, y las cambia entre descansos largos o en el "taller"/tienda entre sesiones.
2. **Límite de cartas por tipo:**
   - 1 carta de clase (2 si hay Multiclase, ver sección 11.1, repartiendo el mismo total de cartas).
   - 1 carta de raza.
   - 1 carta de personalidad/trasfondo.
   - 1 pasiva de clase, siempre única — nunca hay una segunda pasiva de clase activa a la vez.
   - Habilidades activas según el total de cartas del tier (ver tabla sección 3 y fórmula de la sección 11.1), repartidas entre las tres pilas.
   - Equipo limitado por slots fijos (sección "Cartas de Equipo").
   - Consumibles: máx. 3 cartas simultáneas en la ranura de consumibles, sin importar tier.
   - Invocaciones activas simultáneas limitadas por INT (ver sección 4 y 9.10).
3. **Requisitos por stat** — `requiredStats` en la carta; si no se cumple, la carta no puede equiparse/aprenderse (no solo "jugarse con penalización").
4. **Requisitos por clase/rol** — `classTags`/`roleTags`; vacíos = disponible para cualquier clase.
5. **Requisitos por tier** — nunca se juega una carta por debajo de su tier mínimo, incluso con slots libres.
6. **Penalizaciones de equipo** — la armadura pesada por defecto añade el tag `[Armadura Pesada]`, que es incompatible con cartas marcadas `incompatibleTags: ["Armadura Pesada"]` (típicamente hechizos arcanos avanzados y habilidades de sigilo).
7. **Recuperación de cartas** — toda habilidad o hechizo con impacto de combate relevante debe tener `recovery: descanso_corto` o `descanso_largo` (nunca `ninguno`); solo las cartas de bajo impacto pueden ser de uso libre.
8. **Tags incompatibles** — ver sección 8, campo `incompatibleTags`.
9. **Ningún stat en más de dos fórmulas** — ver la tabla de la sección 4; si una carta nueva hace que un stat empiece a influir en una tercera fórmula, hay que replantear la carta antes de publicarla.
10. **Habilidades divinas: máximo 3 por personaje.** Las habilidades divinas (hechizos con
    `castingStat: CAR` otorgados por una deidad, ver 9.11) son **universales**: cualquier clase
    puede aprenderlas. Son la capa de personalización transversal del sistema — curas, mejoras de
    stats, ataques, evasiones y utilidades que complementan el kit de la clase, de modo que dos
    jugadores con la misma clase puedan jugarse de forma distinta. Límites duros: **3 habilidades
    divinas como máximo por personaje** (ocupan hueco de mano normal), todas de deidades cuyas
    obligaciones aceptes, y tu raza/mes debe ser compatible con la deidad o deberás ganarte su
    favor narrativamente. Elegirlas es parte de la construcción del personaje, no un añadido
    gratuito.

### Ejemplos de incompatibilidad ya fijados en el sistema base

- `[Armadura Pesada]` bloquea `[Conjuro Arcano Avanzado]` (tier ≥ 3) salvo carta especial que lo habilite explícitamente.
- `[Concentracion]` es incompatible entre sí: solo se puede mantener **una** carta con este tag activa a la vez.
- `[Escudo]` bloquea cartas con tag `[Arma Pesada]` que declaren uso a dos manos.

---

## 11. Progresión de personaje

Al subir de tier (decisión del director de juego, típicamente al cierre de un arco narrativo), el personaje recibe **una** de las siguientes recompensas, elegida por el jugador dentro de lo permitido por su clase:

- +1 a un stat (máximo +1 por stat por tier; tope de stat = tier actual + 3).
- Nueva carta de habilidad o hechizo (dentro del límite de cartas activas del nuevo tier).
- Nueva carta pasiva (si el nuevo tier lo permite; ver tabla sección 3).
- Mejora de una carta ya poseída (sigue su `upgradePath`).
- Acceso a un nuevo objeto de equipo de tier igual al nuevo tier.
- Un nuevo slot de carta (por ejemplo, segunda pasiva en Tier 3).
- Carta de especialización de clase (Tier 2 en adelante, ver sección 14).
- Carta única ligada a la historia de campaña (otorgada por el director de juego, no elegida libremente).

### Resumen por tier

| Tier | Novedades típicas |
|---|---|
| 1 | Clase, raza y trasfondo elegidos. 1 pasiva. 3 habilidades. Equipo básico. |
| 2 | +1 habilidad/hechizo. +1 mejora de equipo. Acceso a especialización básica. |
| 3 | Mejora de una habilidad principal (`evolvesInto`). Cartas avanzadas. Multiclase disponible. |
| 4 | Carta de especialización mayor. Equipo raro o superior. Mejora de stat significativa. |
| 5 | Carta mítica. Habilidad definitiva. Fuerte impacto narrativo. |

### 11.1 Límite de mano y Multiclase

Cada tier fija el **límite de mano**: cuántas cartas de habilidad puede tener un personaje listas
para jugar (no cuenta raza, pasiva ni equipo, que están siempre activos sin ocupar hueco de mano):

| Tier | Límite de mano (una clase) | Límite de mano (Multiclase) |
|---:|---:|---:|
| 1 | 10 | — (Multiclase no disponible) |
| 2 | 15 | 12 |
| 3 | 20 | 16 |

Regla clave: **hacer Multiclase nunca da más cartas que ir de una sola clase — al contrario, reduce
el límite.** Un personaje de Tier 2 sin Multiclase lleva 15 cartas de su única clase; si decide
multiclasear, su límite baja a 12, repartidas entre las dos clases como quiera (p. ej. 6 y 6, o 9 y
3). Esto evita construir personajes "imposibles" que acumulen lo mejor de dos clases sin coste: el
precio de la versatilidad es siempre llevar menos cartas en total.

Requisitos para Multiclase: disponible desde Tier 2 en adelante, y requiere haber tenido contacto
narrativo con la clase secundaria (un maestro, un libro, un aliado — a criterio del director de
juego). La clase secundaria aporta acceso a sus cartas de habilidad y su compatibilidad de equipo,
pero **no** una segunda pasiva (la pasiva de clase sigue siendo única, ver sección 9.4).

---

## 12. Guía para diseñar cartas nuevas

Para mantener el balance al añadir contenido:

1. **Define el tier antes que el efecto.** El tier acota cuánto puede hacer la carta; no al revés.
2. **Toda carta con impacto de combate debe tener coste.** Si no consume recurso, debe tener límite de usos por descanso o requerir una condición no trivial (`limitations`).
3. **Compara contra una carta similar ya publicada.** Usa la tabla de bonos de vida por tier y las cartas de ejemplo (sección 17) como vara de medir: una carta de Tier 2 no debería superar en más de ~30% el efecto numérico de una equivalente de Tier 1.
4. **Usa `requiredStats` en vez de subir el efecto.** Si una carta parece fuerte para su tier, primero prueba exigir un mínimo de CON/DES/INT/CAR más alto antes de rebajar el efecto.
5. **Toda carta ofensiva fuerte necesita una debilidad explícita.** `incompatibleTags`, coste alto, `duration: concentration`, o un `range: melee` son las cuatro palancas preferidas.
6. **No dupliques nombres de mecánica.** Si dos cartas hacen lo mismo con distinto flavor, son la misma carta con re-skin, no dos cartas nuevas — evita inflar el catálogo sin aportar decisiones nuevas.
7. **Prueba la carta en una mesa antes de fijarla como "core".** Márcala `rarity: uncommon` o superior mientras esté en pruebas; solo pasa a `common` tras validarse en juego.
8. **Todo upgradePath debe ser estrictamente mejor, nunca lateral.** Si quieres una variante lateral (mismo tier, efecto distinto), es una carta nueva, no una mejora.

---

## 13. Estructura de archivos

Estructura de referencia para representar el sistema digitalmente (independiente de cualquier motor o framework concreto):

```
/cards
  /classes/{class_id}.json
  /races/{race_id}.json
  /backgrounds/{background_id}.json
  /skills/{skill_id}.json
  /spells/{spell_id}.json
  /passives/{passive_id}.json
  /equipment/{item_id}.json
  /consumables/{item_id}.json
  /traits/{trait_id}.json
  /summons/{summon_id}.json
  /deities/{deity_id}.json
/characters/{character_id}.json
/rules/tiers.json
/rules/piles.json
/rules/tags.json
/rules/compatibility_symbols.json
```

Cada carpeta bajo `/cards` contiene un archivo por carta, nombrado con su `id`. Un índice (`/cards/index.json`) puede listar todas las cartas con sus campos mínimos (`id`, `name`, `type`, `tier`) para carga rápida sin parsear cada archivo individual. Los personajes referencian cartas por `id`, nunca las embeben completas, de forma que actualizar una carta actualiza automáticamente a todos los personajes que la usan.

`/rules/piles.json` sustituye al antiguo `/rules/resources.json` de la edición 1: en vez de definir
recursos por puntos, describe las tres pilas (Activa / Descanso Corto / Descanso Largo) y sus
condiciones de recuperación (sección 6). `/rules/compatibility_symbols.json` es nuevo: define los
símbolos △ ○ □ ✦ ☠ ✝ ♞ ⚙ y qué combinaciones son válidas (sección 8).

---
### Plantillas de carta por tipo

Cada tipo de carta se presenta primero como **plantilla de texto** (para rellenar a mano o en un editor simple) y después como **esquema JSON** (para uso digital). Los campos marcados `(obl.)` son obligatorios; el resto son opcionales u opcionales-si-aplica.

#### 9.1 Carta de Clase

Además de sus stats, una carta de clase declara con qué puede combinarse: **compatibilidad de
armas y armaduras** (usando los símbolos de la sección 8), **lore** propio, y su **número máximo de
cartas de habilidad** (que sigue la fórmula de tier de la sección 11.1, salvo que la clase declare
una excepción explícita).

**Plantilla de texto**

```
NOMBRE:
ROL: [tank | balanced | agile | caster | support]
TIER INICIAL:
VIDA BASE:
COMPATIBLE:
  ARMAS: (símbolos, ej. △ Armas ligeras)
  ARMADURAS: (símbolos, ej. △ Armaduras ligeras)
MÁXIMO DE CARTAS DE HABILIDAD: (por tier, normalmente sigue la tabla de la sección 11.1)
EQUIPO INICIAL: (lista)
HABILIDADES INICIALES: (número + lista)
PASIVA DE CLASE: (única, ver sección 9.4)
TAGS DE EQUIPO RESTRINGIDO:
LORE:
DESCRIPCIÓN / IDENTIDAD:
ESPECIALIZACIONES (Tier 2+): (lista de 2-3 nombres)
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "class",
  "role": "tank | balanced | agile | caster | support (obl.)",
  "tier": "number (obl., tier mínimo para elegir la clase, normalmente 1)",
  "baseHealth": "number (obl.)",
  "compatibility": {
    "weapons": ["△", "..."],
    "armor": ["△", "..."]
  },
  "maxSkillCards": "number (obl.; por defecto sigue la tabla de tier de la sección 11.1)",
  "startingEquipment": ["item_id", "..."],
  "startingCards": {
    "passive": "passive_id (obl., única — ver 9.4)",
    "skills": ["skill_id", "..."]
  },
  "restrictedTags": ["tag", "..."],
  "specializations": ["specialization_id", "..."],
  "lore": "string",
  "description": "string"
}
```

---

#### 9.2 Carta de Raza

Una raza debe aportar siempre una **habilidad** con personalidad propia, nunca solo un +1 plano a
una stat — el bono de stat es un añadido menor, no el motivo para elegir la raza.

**Plantilla de texto**

```
NOMBRE:
IDIOMAS:
VELOCIDAD:
DEIDADES COMPATIBLES: (si aplica, ver 9.11)
BONOS DE STATS: (menor, ej. +1 CON — nunca el rasgo principal)
RASGO RACIAL: (habilidad con nombre propio, no un número suelto)
RASGO ACTIVO OPCIONAL:
AFINIDADES: (equipo, magia, entorno)
LIMITACIONES:
TAGS NARRATIVOS:
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "race",
  "tier": 1,
  "languages": ["string", "..."],
  "speed": "number (obl.)",
  "compatibleDeities": ["deity_id", "..."],
  "statBonuses": { "CON": 0, "DES": 0, "INT": 0, "CAR": 0 },
  "racialTrait": { "name": "string (obl.)", "description": "string (obl.)" },
  "activeTrait": { "name": "string", "description": "string" },
  "affinities": ["tag", "..."],
  "limitations": ["string", "..."],
  "narrativeTags": ["tag", "..."],
  "unlocks": ["card_id", "..."],
  "flavorText": "string"
}
```

---

#### 9.3 Carta de Personalidad (Trasfondo)

Esta carta **no da stats**. Da reglas sociales: una virtud, un defecto y un objetivo. Se usa para
otorgar inspiración, puntos heroicos o favores narrativos cuando el jugador interpreta su virtud,
sufre por su defecto, o avanza hacia su objetivo — el trasfondo es una herramienta de
interpretación con recompensa mecánica ligera, no una fuente de bonos numéricos.

**Plantilla de texto**

```
NOMBRE:
ORIGEN SOCIAL:
VIRTUD: (rasgo positivo que el jugador puede interpretar)
DEFECTO: (rasgo negativo/complicación que el jugador puede interpretar)
OBJETIVO: (meta narrativa concreta del personaje)
COMPETENCIAS NARRATIVAS:
CONTACTOS:
EQUIPO ADICIONAL:
RECOMPENSA POR INTERPRETAR: (inspiración | punto heroico | favor divino)
CARTAS ESPECIALES DESBLOQUEADAS:
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "background",
  "tier": 1,
  "virtue": "string (obl.)",
  "flaw": "string (obl.)",
  "objective": "string (obl.)",
  "narrativeSkills": ["string", "..."],
  "contacts": ["string", "..."],
  "bonusEquipment": ["item_id", "..."],
  "roleplayReward": "inspiracion | punto_heroico | favor_divino",
  "unlocks": ["card_id", "..."],
  "flavorText": "string"
}
```

---

#### 9.4 Carta Pasiva

La pasiva de clase **es la identidad de la clase**: modifica por completo cómo se juega, no es un
bonus más. Reglas fijas:

- Cada clase tiene **una única** pasiva de clase, ligada a ella desde Tier 1.
- **Nunca puede eliminarse** ni sustituirse mientras el personaje mantenga esa clase.
- Un personaje solo tiene **una pasiva de clase activa** a la vez, incluso haciendo Multiclase (la
  clase secundaria no aporta una segunda pasiva de clase, ver sección 11.1). Sí puede tener además
  rasgos pasivos de raza o trasfondo (secciones 9.2/9.3), que son independientes de esta carta.

**Plantilla de texto**

```
NOMBRE:
CLASE:
TIER MÍNIMO: (normalmente 1)
CONDICIÓN DE ACTIVACIÓN:
BENEFICIO CONSTANTE:
LIMITACIÓN:
SINERGIAS:
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "passive",
  "tier": "number (obl.)",
  "classTags": ["class_id (obl., normalmente una sola clase)"],
  "trigger": "string (obl., condición de activación)",
  "effect": { "description": "string (obl.)", "scaling": "CON | DES | INT | CAR | none" },
  "limitations": ["string"],
  "synergyTags": ["tag", "..."],
  "unique": true,
  "flavorText": "string"
}
```

---

#### 9.5 Carta de Habilidad

El núcleo del juego. No tiene coste en puntos de recurso: al jugarla, se mueve a la pila que
indique `recovery` (ver sección 6).

**Plantilla de texto**

```
NOMBRE:
TIER MÍNIMO:
ICONO: [⚔ Acción | ✨ Encantamiento | 🛡 Reacción | 👣 Movimiento | ❤ Curación | 👻 Invocación | ⏳ Canalización | 🌑 Ritual]
TIPO DE ACCIÓN: [accion | accion_menor | reaccion | movimiento | preparacion]
RECUPERACIÓN: [activa | descanso_corto | descanso_largo | ninguno]
STAT DE DEFENSA (a quién apunta): [CA | Defensa mental | Resistencia física]
ALCANCE:
EFECTO PRINCIPAL:
EFECTO ADICIONAL (opcional):
CONDICIONES:
EVOLUCIONA A: (id de la versión mejorada, si es una carta evolutiva — ver 9.12)
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "skill",
  "tier": "number (obl.)",
  "rarity": "common | uncommon | rare | epic | mythic",
  "typeIcon": "⚔ | ✨ | 🛡 | 👣 | ❤ | 👻 | ⏳ | 🌑",
  "classTags": ["class_id"],
  "roleTags": ["role"],
  "mechanicTags": ["tag", "..."],
  "requiredStats": { "CON": 0, "DES": 0, "INT": 0, "CAR": 0 },
  "requiredTags": ["tag"],
  "incompatibleTags": ["tag"],
  "recovery": "activa | descanso_corto | descanso_largo | ninguno (obl.)",
  "actionType": "accion | accion_menor | reaccion | movimiento | preparacion",
  "range": "melee | short | medium | long | self",
  "duration": "instant | 1_turn | 1_min | concentration | permanent",
  "defenseStat": "CA | defensa_mental | resistencia_fisica",
  "effect": { "description": "string (obl.)", "scaling": "CON | DES | INT | CAR | none" },
  "limitations": ["string"],
  "evolvesInto": "skill_id (opcional, ver 9.12)",
  "flavorText": "string"
}
```

---

#### 9.6 Carta de Hechizo

Los hechizos son cartas de habilidad especializadas: mismo sistema de pilas (`recovery`), pero
declaran una `castingStat` de dos posibles orígenes de poder — arcano (INT) o divino/afinidad (CAR).
Un hechizo de `castingStat: INT` usa la fórmula de **Precisión mágica** (sección 5); uno de
`castingStat: CAR` usa **Defensa mental** como referencia — no hay una tercera fórmula de precisión.

**Plantilla de texto**

```
NOMBRE:
ESCUELA MÁGICA:
TIER MÍNIMO:
STAT DE CONJURACIÓN: [INT (arcano) | CAR (divino/afinidad)]
RECUPERACIÓN: [activa | descanso_corto | descanso_largo]
ALCANCE:
ÁREA (opcional):
DURACIÓN:
EFECTO / DAÑO:
CONDICIONES DE MEJORA:
LIMITACIONES POR CLASE:
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "spell",
  "school": "string (obl., ej. evocacion, abjuracion, invocacion...)",
  "tier": "number (obl.)",
  "rarity": "common | uncommon | rare | epic | mythic",
  "classTags": ["class_id"],
  "castingStat": "INT | CAR (obl.)",
  "recovery": "activa | descanso_corto | descanso_largo (obl.)",
  "range": "melee | short | medium | long | self",
  "area": "string (opcional, ej. 'radio 10 pies')",
  "duration": "instant | 1_turn | 1_min | concentration | permanent",
  "mechanicTags": ["tag", "..."],
  "requiredTags": ["tag"],
  "incompatibleTags": ["tag"],
  "effect": { "description": "string (obl.)", "scaling": "INT | CAR" },
  "upgradeConditions": "string (opcional)",
  "limitations": ["string"],
  "evolvesInto": "spell_id (opcional, ver 9.12)",
  "flavorText": "string"
}
```

---

#### 9.7 Carta de Equipo

Cada pieza aporta bonos de stat directos y, opcionalmente, una **habilidad propia vinculada**
(`linkedSkill`) — así una pieza sigue siendo interesante aunque dé pocos puntos de stat. Ejemplo:
unas botas con solo +1 DES pero con una acción de "ignora terreno difícil durante un turno" son
una elección táctica, no solo un número.

**Plantilla de texto**

```
NOMBRE:
TIPO DE OBJETO:
SLOT:
BONOS DE STATS:
PENALIZACIONES (opcional):
STAT DE PRECISIÓN (solo armas): [CON | DES]
SÍMBOLOS DE COMPATIBILIDAD: (△ ○ □ / ✦ ☠ ✝ ♞ ⚙)
HABILIDAD PROPIA (opcional): (id de una carta de habilidad ligada a este objeto)
RESTRICCIONES:
RAREZA:
TIER MÍNIMO:
```

**Slots disponibles (6, ver también sección 3 del manual de referencia): un personaje solo puede
llevar un objeto por slot a la vez — cambiar de objeto en combate cuesta la Acción principal salvo
que la carta diga lo contrario.**

| Slot | Símbolo | Contiene |
|---|---|---|
| `cabeza` | ⛑ | Cascos, diademas, máscaras, coronas |
| `torso` | 👕 | Armaduras, túnicas, capas pesadas |
| `piernas` | 👖 | Grebas, faldas de batalla, pantalones reforzados |
| `pies` | 👢 | Botas, calzado ligero, calzado mágico |
| `arma_principal` | ⚔ | El arma en la mano dominante |
| `arma_secundaria` | 🛡 | Arma corta, daga, escudo o foco mágico |

Cada objeto declara además su **categoría de peso** con los símbolos de la sección 8 (△ Ligera, ○
Media, □ Pesada). Si la categoría supera la compatibilidad de la clase que lo lleva, se aplica una
penalización mecánica fija (ver sección 7.2): △ nunca penaliza; ○ fuera de compatibilidad da
desventaja en tiradas de ataque; □ fuera de compatibilidad da desventaja **y** reduce la velocidad
de movimiento a la mitad. Esta penalización es automática por categoría — no hace falta escribirla
carta a carta.

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "equipment",
  "slot": "cabeza | torso | piernas | pies | arma_principal | arma_secundaria (obl.)",
  "tier": "number (obl.)",
  "rarity": "common | uncommon | rare | epic | mythic",
  "statBonuses": { "CON": 0, "DES": 0, "INT": 0, "CAR": 0, "health": 0 },
  "penalties": { "CON": 0, "DES": 0, "INT": 0, "CAR": 0 },
  "precisionStat": "CON | DES (solo si slot es un arma, ver 'Precisión con armas' en la sección 5)",
  "compatibilitySymbols": ["△", "..."],
  "grantedTags": ["tag", "..."],
  "linkedSkill": "skill_id (opcional, la habilidad propia de esta pieza)",
  "requiredStats": { "CON": 0, "DES": 0, "INT": 0, "CAR": 0 },
  "restrictions": ["string"],
  "flavorText": "string"
}
```

---

#### 9.8 Carta de Objeto Consumible

**Plantilla de texto**

```
NOMBRE:
TIER MÍNIMO:
EFECTO AL USAR:
TIPO DE ACCIÓN PARA USAR:
LÍMITE DE USOS (por objeto/por descanso):
RAREZA:
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "consumable",
  "tier": "number (obl.)",
  "rarity": "common | uncommon | rare | epic | mythic",
  "actionType": "accion | accion_menor | reaccion",
  "effect": { "description": "string (obl.)" },
  "uses": "number (obl., normalmente 1 - se descarta tras usarse)",
  "flavorText": "string"
}
```

---

#### 9.9 Carta de Rasgo Especial

**Plantilla de texto**

```
NOMBRE:
ORIGEN: [raza | trasfondo | historia | tier]
TIER MÍNIMO:
EFECTO:
LIMITACIÓN:
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "trait",
  "origin": "race | background | story | tier (obl.)",
  "tier": "number (obl.)",
  "effect": { "description": "string (obl.)" },
  "limitations": ["string"],
  "flavorText": "string"
}
```

---

#### 9.10 Carta de Invocación

Una habilidad de tipo invocación (icono 👻) **no describe a la criatura**: solo la invoca. La
criatura es su propia carta independiente, con toda su información a mano sin tener que consultar
el manual. El número de invocaciones activas simultáneas que un personaje puede mantener está
gobernado por INT (sección 4).

**Plantilla de texto**

```
NOMBRE:
INVOCADA POR: (id de la carta de habilidad que la invoca)
TIER MÍNIMO:
VIDA:
ATAQUES: (lista, cada uno con su efecto)
MOVIMIENTO:
PASIVA:
DURACIÓN DE LA INVOCACIÓN:
CONTROL: (automática | controlada por el jugador | controlada por CAR del invocador)
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "summon",
  "summonedBy": "skill_id | spell_id (obl.)",
  "tier": "number (obl.)",
  "health": "number (obl.)",
  "attacks": [{ "name": "string", "effect": "string" }],
  "movement": "number (obl.)",
  "passive": { "name": "string", "description": "string" },
  "duration": "instant | 1_turn | 1_min | concentration | permanent",
  "control": "automatica | jugador | car_invocador",
  "flavorText": "string"
}
```

---

#### 9.11 Carta de Deidad / Afinidad *(opcional)*

Para campañas donde la fe o el pacto con un poder superior importa mecánicamente, no solo
narrativamente. Vincula con CAR (afinidad divina, sección 4) y con `compatibleDeities` de la carta
de raza (sección 9.2).

Cada deidad **otorga un repertorio de habilidades divinas** (hechizos universales con
`castingStat: CAR`, campo `deity` apuntando a ella y `grantedSpells` en la carta de deidad).
Cualquier clase puede aprenderlas, con el límite duro de **3 habilidades divinas por personaje**
(regla 10.10) — son la capa de personalización que diferencia a dos personajes de la misma clase.

**Plantilla de texto**

```
NOMBRE:
DOMINIO: (guerra, naturaleza, muerte, conocimiento...)
FAVOR OTORGADO: (efecto mecánico menor, ligado a CAR)
COMPATIBLE CON: (razas, trasfondos o clases afines)
OBLIGACIONES / TABÚES:
```

**Esquema JSON**

```json
{
  "id": "string (obl.)",
  "name": "string (obl.)",
  "type": "deity",
  "domain": "string (obl.)",
  "favor": { "description": "string (obl.)", "scaling": "CAR | none" },
  "compatibleWith": ["race_id | background_id | class_id", "..."],
  "obligations": ["string"],
  "flavorText": "string"
}
```

---

#### 9.12 Cartas evolutivas

Algunas cartas de habilidad/hechizo llevan una marca de evolución: en vez de ocupar un hueco nuevo
al mejorar, **sustituyen a la versión anterior**. Por ejemplo `Fletxa d'Ombra I → II → III`: al
cumplir la condición de mejora (normalmente subir de tier, o un hito de campaña), la carta I se
retira y la II ocupa exactamente su mismo sitio en la pila — el personaje nunca lleva las dos a la
vez ni gasta un slot extra por mejorar una carta que ya tenía.

Mecánica: el campo `evolvesInto` de la carta actual apunta al `id` de la siguiente versión. Una
carta con `evolvesInto` vacío es la versión final de su línea evolutiva. Esto es distinto de
`specializations` (sección 9.1, a nivel de clase): una evolución mejora una carta concreta que ya
tienes, una especialización abre una rama nueva de opciones de clase.
## 14. Clases iniciales

Cinco clases cubren los cinco roles base del sistema. Todas siguen la fórmula de vida de la
sección 5 (`Vida Base + CON×3 + CAR×1 + Bono de Tier + Bono de Equipo` — el multiplicador es el
mismo para todas: el diferenciador de rol es la vida base, la compatibilidad de equipo y el perfil
de recuperación de sus cartas), las tablas de tier de la sección 3 y el sistema de pilas de la
sección 6. Ninguna carta de esta sección tiene coste en puntos: cada habilidad o hechizo declara
su `recovery`.

---

### 14.1 Guardián de Hierro (Tanque)

**Rol:** Tanque. **Vida base:** 16. **Stat principal:** CON. **Stat secundaria:** CAR.
**Compatibilidad:** Armas △ ○ · Armaduras △ ○ □. **Equipo inicial:** Escudo Reforzado, Armadura de Malla, Espada Corta.
**Habilidades iniciales:** 3. **Hechizos iniciales:** 0. **Pasiva de clase:** Muro Viviente.

**Identidad:** primera línea defensiva. Protege aliados, bloquea pasos, resiste castigo y penaliza a quien lo ignora. Su CAR no es adorno: representa la presencia que obliga al enemigo a mirarle a él, y aporta a su vida.

**Perfil de pilas:** vive de ciclos cortos — sus dos cartas clave vuelven con cualquier descanso corto, así que puede gastarlas sin miedo en cada combate.

**Restricciones:** no puede usar armas a dos manos mientras lleve escudo equipado; la armadura pesada (□) le añade el tag `[Armadura Pesada]`, incompatible con cartas `[Sigilo]`; no accede a hechizos arcanos de tier ≥ 2 salvo carta especial.

**Pasiva de clase — Muro Viviente:** mientras lleves escudo equipado, reduces el primer daño físico recibido cada ronda en una cantidad igual a tu CON.

**Habilidades iniciales:**
1. **Golpe de Escudo** — ⚔ Acción · requiere `[Escudo]` · daño físico leve + empuje corto · Recuperación: **descanso corto**.
2. **Interponerse** — 🛡 Reacción · cuando un aliado a alcance corto recibe daño, absorbes la mitad en su lugar · Recuperación: **descanso corto**.
3. **Postura Defensiva** — ⚔ Acción menor · +CA hasta tu próximo turno, reduces tu movimiento a la mitad · Recuperación: **activa** (uso libre: su contrapartida de movimiento es el coste).

**Progresión:** Tier 2 añade *Provocar* (control de amenaza, descanso corto); Tier 3 añade la pasiva secundaria *Piel de Roble* (resistencia a un tipo de daño elegido al descansar) y Golpe de Escudo evoluciona a *Golpe de Escudo Mayor* (`evolvesInto`, sustituye a la carta original sin gastar hueco); Tier 4 desbloquea especialización mayor; Tier 5 habilidad definitiva *Bastión Inquebrantable*.

**Especializaciones (desde Tier 2):**
- **Bastión** — máxima defensa de zona, control de área.
- **Vengador Blindado** — tanque ofensivo, contraataques y daño por recibir golpes.
- **Custodio Sagrado** — defensa + curación vía CAR, acceso a cartas `[Sagrado]`.

**JSON de la carta de clase:**

```json
{
  "id": "class_guardian_iron",
  "name": "Guardián de Hierro",
  "type": "class",
  "role": "tank",
  "tier": 1,
  "baseHealth": 16,
  "compatibility": { "weapons": ["△", "○"], "armor": ["△", "○", "□"] },
  "maxSkillCards": 10,
  "startingEquipment": ["item_reinforced_shield", "item_chain_armor", "item_short_sword"],
  "startingCards": {
    "passive": "passive_living_wall",
    "skills": ["skill_shield_bash", "skill_intercept", "skill_defensive_stance"],
    "spells": []
  },
  "restrictedTags": ["Conjuro Arcano Avanzado", "Arma Pesada a Dos Manos"],
  "specializations": ["spec_bastion", "spec_armored_avenger", "spec_sacred_custodian"],
  "lore": "Los guardianes de hierro se forjan en las murallas: donde ellos se plantan, la línea no cae.",
  "description": "Clase defensiva de primera línea: protege aliados, bloquea caminos y resiste ataques."
}
```

---

### 14.2 Sable Errante (Guerrero equilibrado)

**Rol:** Combatiente equilibrado. **Vida base:** 12. **Stat principal:** DES. **Stat secundaria:** CON.
**Compatibilidad:** Armas △ ○ □ · Armaduras △ ○. **Equipo inicial:** Espada Larga, Armadura Media, Brazal de Parada.
**Habilidades iniciales:** 3. **Hechizos iniciales:** 0. **Pasiva de clase:** Instinto de Combate.

**Identidad:** versátil, capaz de atacar y aguantar con solvencia. No domina ningún extremo pero no tiene debilidades claras. Es la clase con la compatibilidad de armas más amplia del juego.

**Perfil de pilas:** mezcla equilibrada — un ataque fuerte que gasta pila y utilidades de uso libre para los turnos "vacíos".

**Restricciones:** sin acceso a hechizos salvo carta de especialización; la armadura media (○) fuera de contexto de sigilo no penaliza, pero es incompatible con cartas `[Sigilo]` avanzadas (tier ≥ 2).

**Pasiva de clase — Instinto de Combate:** una vez por combate, cuando caes por debajo de la mitad de tu vida máxima, tu próximo ataque tiene ventaja.

**Habilidades iniciales:**
1. **Tajo Doble** — ⚔ Acción · dos golpes ligeros consecutivos con el arma equipada · Recuperación: **descanso corto**.
2. **Paso de Flanqueo** — 👣 Movimiento · te desplazas y ganas +1 a tu próxima tirada de ataque si atacas desde un ángulo distinto al del último aliado que atacó · Recuperación: **activa**.
3. **Golpe de Interrupción** — 🛡 Reacción · cuando un enemigo cercano lanza una habilidad, puedes atacarlo; si impactas, reduces el efecto de su acción · Recuperación: **descanso corto**.

**Progresión:** Tier 2: Tajo Doble evoluciona a *Combo de Acero* (encadena un tercer golpe si el primero impacta); Tier 3 pasiva secundaria *Segundo Aliento* (recupera vida al iniciar combate); Tier 4 especialización mayor; Tier 5 habilidad definitiva *Danza de Filo Infinito*.

**Especializaciones (desde Tier 2):**
- **Duelista** — precisión, contragolpes, ventaja en combates 1 contra 1.
- **Rompedor de Líneas** — daño en área con armas pesadas (□), empujes.
- **Capitán de Escuadra** — buffs breves a aliados cercanos en combate, apoyado en CAR.

**JSON:**

```json
{
  "id": "class_wandering_blade",
  "name": "Sable Errante",
  "type": "class",
  "role": "balanced",
  "tier": 1,
  "baseHealth": 12,
  "compatibility": { "weapons": ["△", "○", "□"], "armor": ["△", "○"] },
  "maxSkillCards": 10,
  "startingEquipment": ["item_longsword", "item_medium_armor", "item_parry_bracer"],
  "startingCards": {
    "passive": "passive_combat_instinct",
    "skills": ["skill_double_slash", "skill_flanking_step", "skill_interrupt_strike"],
    "spells": []
  },
  "restrictedTags": ["Conjuro Arcano"],
  "specializations": ["spec_duelist", "spec_line_breaker", "spec_squad_captain"],
  "lore": "Sin señor, sin muralla y sin templo: el sable errante confía en su filo y en el camino.",
  "description": "Combatiente versátil, sólido en ataque y aguante, sin dependencias mágicas."
}
```

---

### 14.3 Sombra del Camino (Pícaro / Explorador)

**Rol:** Especialista ágil. **Vida base:** 10. **Stat principal:** DES. **Stat secundaria:** INT.
**Compatibilidad:** Armas △ · Armaduras △. **Equipo inicial:** Dagas Gemelas, Armadura Ligera, Kit de Herramientas.
**Habilidades iniciales:** 4. **Hechizos iniciales:** 0. **Pasiva de clase:** Paso Silencioso.

**Identidad:** movilidad, sigilo y daño de precisión. Débil si es alcanzado en combate directo prolongado. Lleva una carta más en mano inicial que el resto: su fuerza es tener siempre la herramienta adecuada, no golpear más fuerte.

**Perfil de pilas:** todas sus cartas de impacto van a descanso corto — la Sombra funciona a ráfagas y necesita descansar más a menudo que nadie. Elegir cuándo gastar la última carta de la pila Activa es su decisión central de juego.

**Restricciones:** sin acceso a armadura media ni pesada (○ □ fuera de compatibilidad: desventaja, ver sección 7.2) — además pierde su pasiva de clase mientras lleve una equipada; penalización a cartas `[Bloqueo]`.

**Pasiva de clase — Paso Silencioso:** tus pruebas de sigilo tienen ventaja si te has movido este turno sin atacar.

**Habilidades iniciales:**
1. **Golpe Preciso** — ⚔ Acción · daño adicional si el objetivo no te ha detectado o está flanqueado · Recuperación: **descanso corto**.
2. **Desaparecer entre Sombras** — 👣 Acción menor · ganas ventaja en tu próxima prueba de sigilo · Recuperación: **descanso corto**.
3. **Trampa Rápida** — ⏳ Preparación · colocas una trampa menor que impone desventaja al primer enemigo que la active · Recuperación: **descanso corto**.
4. **Esquiva Instintiva** — 🛡 Reacción · reduces el daño de un ataque que te impacte · Recuperación: **descanso corto**.

**Progresión:** Tier 2: Golpe Preciso evoluciona a *Ataque Furtivo Mejorado*; Tier 3 pasiva secundaria *Reflejos Felinos* (ventaja en una salvación de DES por combate); Tier 4 especialización mayor; Tier 5 habilidad definitiva *Mil Cortes*.

**Especializaciones (desde Tier 2):**
- **Asesino** — daño de sigilo explosivo, finalización de objetivos débiles.
- **Explorador** — utilidad de mapa, trampas, rastreo, ventaja en exploración.
- **Saboteador** — control con veneno y trampas de área.

**JSON:**

```json
{
  "id": "class_road_shadow",
  "name": "Sombra del Camino",
  "type": "class",
  "role": "agile",
  "tier": 1,
  "baseHealth": 10,
  "compatibility": { "weapons": ["△"], "armor": ["△"] },
  "maxSkillCards": 10,
  "startingEquipment": ["item_twin_daggers", "item_light_armor", "item_tool_kit"],
  "startingCards": {
    "passive": "passive_silent_step",
    "skills": ["skill_precise_strike", "skill_vanish", "skill_quick_trap", "skill_instinctive_dodge"],
    "spells": []
  },
  "restrictedTags": ["Armadura Pesada", "Escudo"],
  "specializations": ["spec_assassin", "spec_scout", "spec_saboteur"],
  "lore": "Nadie recuerda su cara; todos recuerdan que la puerta estaba abierta y el cofre vacío.",
  "description": "Movilidad, sigilo y precisión; frágil en combate directo prolongado."
}
```

---

### 14.4 Arcanista (Mago)

**Rol:** Mago / control. **Vida base:** 8. **Stat principal:** INT. **Stat secundaria:** DES.
**Compatibilidad:** Armas △ · Armaduras △ · Afinidad ✦. **Equipo inicial:** Foco Arcano, Ropa Ligera, Daga Simple.
**Habilidades iniciales:** 2. **Hechizos iniciales:** 3. **Pasiva de clase:** Canalizador Arcano.

**Identidad:** frágil pero versátil. Controla el campo de batalla, inflige daño elemental y resuelve problemas con conocimiento mágico. Su DES secundaria es supervivencia pura: sin armadura real, su CA es lo único que le separa de un golpe directo.

**Perfil de pilas:** un "truco" de uso libre (Lanza de Brasas) que nunca le deja sin nada que hacer, más hechizos de pila que definen cada combate. Su pasiva le permite estirar la pila Activa más que ningún otro conjurador.

**Restricciones:** lanzar hechizos con armadura media o pesada equipada (○ □, fuera de compatibilidad) impone desventaja en la tirada de conjuración; depende de INT para casi todas sus cartas; pocas cartas defensivas propias al inicio.

**Pasiva de clase — Canalizador Arcano:** una vez por descanso corto, tras resolver un hechizo con `recovery: descanso_corto`, puedes devolverlo a la pila Activa en vez de enviarlo a su pila de descanso.

**Habilidades iniciales (no mágicas):**
1. **Golpe con Foco** — ⚔ Acción · ataque menor con el foco arcano · Recuperación: **activa**.
2. **Paso Arcano** — 👣 Movimiento · pequeño desplazamiento instantáneo de corto alcance · Recuperación: **descanso corto**.

**Hechizos iniciales:**
1. **Lanza de Brasas** — ✨ Tier 1 · INT · alcance medio · daño de fuego; si el objetivo ya está `[Quemado]`, el daño aumenta · Recuperación: **activa** (su truco básico).
2. **Escudo Breve** — 🛡 Tier 1 · INT · reacción · reduce el daño de un ataque entrante · Recuperación: **descanso corto**.
3. **Mano Invisible** — ✨ Tier 1 · INT · utilidad · manipula objetos pequeños a distancia corta · Recuperación: **activa**.

**Progresión:** Tier 2 añade un hechizo adicional y *Sobrecarga Menor* (aumenta el daño del siguiente hechizo a cambio de vida, descanso largo); Tier 3 pasiva secundaria *Memoria Arcana* (al ganar un combate, devuelve 1 carta de tu pila Descanso Corto a la pila Activa); Tier 4 especialización mayor; Tier 5 habilidad definitiva *Convergencia Arcana*.

**Especializaciones (desde Tier 2):**
- **Piromante** — daño de fuego, quemaduras, explosiones.
- **Tejedor de Escudos** — barreras, protección mágica, control defensivo.
- **Cronomante Menor** — ralentizar enemigos, manipular iniciativa.

**JSON:**

```json
{
  "id": "class_arcanist",
  "name": "Arcanista",
  "type": "class",
  "role": "caster",
  "tier": 1,
  "baseHealth": 8,
  "compatibility": { "weapons": ["△"], "armor": ["△", "✦"] },
  "maxSkillCards": 10,
  "startingEquipment": ["item_arcane_focus", "item_light_robes", "item_simple_dagger"],
  "startingCards": {
    "passive": "passive_arcane_channeler",
    "skills": ["skill_focus_strike", "skill_arcane_step"],
    "spells": ["spell_ember_lance", "spell_brief_shield", "spell_unseen_hand"]
  },
  "restrictedTags": ["Armadura Media", "Armadura Pesada"],
  "specializations": ["spec_pyromancer", "spec_shield_weaver", "spec_minor_chronomancer"],
  "lore": "El poder arcano no perdona la carne débil: por eso el arcanista aprende antes a no estar donde cae el golpe.",
  "description": "Clase frágil y versátil: control de campo, daño elemental y utilidad arcana."
}
```

---

### 14.5 Voz del Alba (Sanador / Apoyo)

**Rol:** Sanador / apoyo. **Vida base:** 10. **Stat principal:** CAR. **Stat secundaria:** CON.
**Compatibilidad:** Armas △ ○ · Armaduras △ ○ · Afinidad ✝. **Equipo inicial:** Símbolo Sagrado, Equipo Ligero, Bastón Simple.
**Habilidades iniciales:** 2. **Hechizos iniciales:** 2. **Pasiva de clase:** Aliento Estable.

**Identidad:** sostiene al grupo, cura heridas y aplica bendiciones y penalizaciones tácticas menores. Su magia es divina: conjura con CAR (ver sección 9.6), la misma presencia que gobierna su Defensa mental y aporta a su vida — la Voz del Alba aguanta más de lo que su rol sugiere.

**Perfil de pilas:** sus curaciones van a descanso corto — decidir a quién curar *ahora* y a quién pedirle que aguante hasta el descanso es su decisión central de juego.

**Restricciones:** sus hechizos de curación no pueden apuntarse a sí misma salvo carta especial (fomenta el juego en equipo); penalización a cartas `[Sigilo]`.

**Pasiva de clase — Aliento Estable:** al inicio de cada uno de tus turnos, si un aliado a alcance corto está por debajo de la mitad de su vida, recupera 1 punto de vida.

**Habilidades iniciales:**
1. **Guía Táctica** — ✨ Acción menor · un aliado a alcance corto gana ventaja en su próxima prueba o ataque · Recuperación: **descanso corto**.
2. **Golpe de Bastón** — ⚔ Acción · ataque físico menor · Recuperación: **activa**.

**Hechizos iniciales:**
1. **Luz que Sana** — ❤ Tier 1 · CAR · alcance corto · restaura vida a un aliado · Recuperación: **descanso corto**.
2. **Bendición Menor** — ✨ Tier 1 · CAR · un aliado gana +1 a sus próximas tiradas de ataque o salvación durante 1 minuto · Recuperación: **descanso corto**.

**Progresión:** Tier 2 añade *Curación en Cadena* (cura a dos aliados adyacentes entre sí, descanso largo); Tier 3 pasiva secundaria *Vínculo Protector* (redirige parte del daño de un aliado marcado); Tier 4 especialización mayor; Tier 5 habilidad definitiva *Amanecer*.

**Especializaciones (desde Tier 2):**
- **Custodio de Vida** — curación pura, sostenibilidad de grupo.
- **Heraldo de Bendiciones** — buffs ofensivos y defensivos a aliados.
- **Purgador** — cartas `[Sagrado]` ofensivas contra enemigos `[Oscuro]`.

**JSON:**

```json
{
  "id": "class_dawn_voice",
  "name": "Voz del Alba",
  "type": "class",
  "role": "support",
  "tier": 1,
  "baseHealth": 10,
  "compatibility": { "weapons": ["△", "○"], "armor": ["△", "○", "✝"] },
  "maxSkillCards": 10,
  "startingEquipment": ["item_holy_symbol", "item_light_gear", "item_simple_staff"],
  "startingCards": {
    "passive": "passive_steady_breath",
    "skills": ["skill_tactical_guidance", "skill_staff_strike"],
    "spells": ["spell_healing_light", "spell_minor_blessing"]
  },
  "restrictedTags": ["Armadura Pesada"],
  "specializations": ["spec_life_custodian", "spec_blessing_herald", "spec_purger"],
  "lore": "Cuando la noche parece no acabar nunca, alguien tiene que recordarle al grupo que el alba existe.",
  "description": "Sostiene al grupo con curación divina (CAR), bendiciones y apoyo táctico."
}
```

---

### Tabla resumen de clases

| Clase | Rol | Vida base | Stats | Compatibilidad | Mano inicial (hab. + hech.) |
|---|---|---:|---|---|---:|
| Guardián de Hierro | Tanque | 16 | CON / CAR | Armas △ ○ · Armad. △ ○ □ | 3 + 0 |
| Sable Errante | Equilibrado | 12 | DES / CON | Armas △ ○ □ · Armad. △ ○ | 3 + 0 |
| Sombra del Camino | Ágil | 10 | DES / INT | Armas △ · Armad. △ | 4 + 0 |
| Arcanista | Mago | 8 | INT / DES | Armas △ · Armad. △ ✦ | 2 + 3 |
| Voz del Alba | Sanador/Apoyo | 10 | CAR / CON | Armas △ ○ · Armad. △ ○ ✝ | 2 + 2 |

Con el multiplicador de vida unificado (CON×3 + CAR×1), el rango de vida por rol sale de la vida
base y de dónde invierte stats cada clase: un Guardián típico de Tier 1 (CON 4, CAR 2) tiene
16+12+2 = 30 de vida; un Arcanista típico (CON 1, CAR 1) tiene 8+3+1 = 12 — una proporción de
~2,5× que crece con el equipo (los tanques llevan las piezas con más `health`).

---

## 15. Razas iniciales

Cinco razas, cada una con bono de stat menor, rasgo racial con nombre propio, rasgo activo
opcional y afinidades (el bono de stat nunca es el motivo para elegir la raza — ver sección 9.2).
Los bonos usan exclusivamente las cuatro stats de la edición 2: CON, DES, INT y CAR.

### 15.1 Humano de las Marcas

**Bono de stats:** +1 a una stat a elección. **Rasgo racial — Adaptable:** una vez por descanso largo, puedes tratar una carta de habilidad o hechizo como si tuviera 1 tier menos de requisito (mínimo tier 1). **Rasgo activo — Determinación:** una vez por combate, ignora los efectos de una condición negativa durante 1 turno. **Afinidad:** ninguna restricción de equipo. **Tags narrativos:** `[Versatil]` `[Comun]`.

```json
{ "id": "race_human_marches", "name": "Humano de las Marcas", "type": "race", "tier": 1,
  "languages": ["Común", "Uno regional a elección"], "speed": 30, "compatibleDeities": [],
  "statBonuses": { "CON": 0, "DES": 0, "INT": 0, "CAR": 0 },
  "racialTrait": { "name": "Adaptable", "description": "1/descanso largo: trata una carta como si exigiera 1 tier menos (mín. 1)." },
  "activeTrait": { "name": "Determinación", "description": "1/combate: ignora una condición negativa durante 1 turno." },
  "affinities": [], "limitations": ["El +1 de stat se elige al crear el personaje y no puede cambiarse después"],
  "narrativeTags": ["Versatil", "Comun"], "unlocks": [] }
```

### 15.2 Elfo del Dosel

**Bono de stats:** +1 DES. **Rasgo racial — Sentidos del Dosel:** ventaja en pruebas de percepción visual y nunca sufre desventaja por poca luz natural. **Rasgo activo — Paso Élfico:** una vez por descanso corto, mueve gratis sin provocar reacciones de oportunidad. **Afinidad:** armas ligeras (△) y arcos; la armadura pesada (□) le impone −1 DES efectivo mientras la lleve, además de la desventaja normal por incompatibilidad. **Tags narrativos:** `[Longevo]` `[Silvano]`.

```json
{ "id": "race_elf_canopy", "name": "Elfo del Dosel", "type": "race", "tier": 1,
  "languages": ["Común", "Élfico"], "speed": 30, "compatibleDeities": ["deity_wild_court"],
  "statBonuses": { "CON": 0, "DES": 1, "INT": 0, "CAR": 0 },
  "racialTrait": { "name": "Sentidos del Dosel", "description": "Ventaja en percepción visual; ignora desventaja por poca luz natural." },
  "activeTrait": { "name": "Paso Élfico", "description": "1/descanso corto: te mueves sin provocar reacciones de oportunidad." },
  "affinities": ["Arma Ligera", "Proyectil"], "limitations": ["-1 DES efectivo mientras lleve Armadura Pesada"],
  "narrativeTags": ["Longevo", "Silvano"], "unlocks": ["passive_canopy_stride_t3"] }
```

### 15.3 Enano de las Fraguas Profundas

**Bono de stats:** +1 CON. **Rasgo racial — Sangre de Forja:** resistencia a `[Veneno]` y ventaja en pruebas para resistir enfermedades. **Rasgo activo — Firmeza:** una vez por combate, ignora un efecto de empuje o derribo. **Afinidad:** armadura pesada (□) y armas marciales; puede desbloquear cartas de forja rúnica en tiers superiores. **Tags narrativos:** `[Subterraneo]` `[Artesano]`.

```json
{ "id": "race_dwarf_deepforge", "name": "Enano de las Fraguas Profundas", "type": "race", "tier": 1,
  "languages": ["Común", "Enano"], "speed": 25, "compatibleDeities": ["deity_forge_father"],
  "statBonuses": { "CON": 1, "DES": 0, "INT": 0, "CAR": 0 },
  "racialTrait": { "name": "Sangre de Forja", "description": "Resistencia a [Veneno]; ventaja al resistir enfermedades." },
  "activeTrait": { "name": "Firmeza", "description": "1/combate: ignora un empuje o derribo." },
  "affinities": ["Armadura Pesada", "Arma Pesada"], "limitations": [],
  "narrativeTags": ["Subterraneo", "Artesano"], "unlocks": ["skill_rune_forging_t2"] }
```

### 15.4 Orco de Gongorguma (Orco / Semiorco)

**Bono de stats:** +1 CON o +1 DES a elección. **Rasgo racial — Furia Contenida:** cuando caes al 25% o menos de tu vida máxima, tu próximo ataque cuerpo a cuerpo gana daño adicional igual a tu CON, una vez por combate. **Rasgo activo — Aguante Feroz:** una vez por descanso largo, al llegar a 0 de vida quedas en 1 punto de vida en su lugar. **Afinidad:** armas pesadas (□); penalización narrativa/social leve en entornos urbanos hostiles (ajustable por mesa). **Tags narrativos:** `[Tribal]` `[Resiliente]`.

```json
{ "id": "race_orc_gongorguma", "name": "Orco de Gongorguma", "type": "race", "tier": 1,
  "languages": ["Común", "Orco"], "speed": 30, "compatibleDeities": ["deity_wolf_king"],
  "statBonuses": { "CON": 1, "DES": 0, "INT": 0, "CAR": 0 },
  "racialTrait": { "name": "Furia Contenida", "description": "1/combate: al caer a ≤25% de vida, tu próximo golpe cuerpo a cuerpo suma daño igual a tu CON." },
  "activeTrait": { "name": "Aguante Feroz", "description": "1/descanso largo: si llegas a 0 de vida, quedas en 1 en su lugar." },
  "affinities": ["Arma Pesada"], "limitations": ["Complicaciones sociales opcionales en asentamientos hostiles a orcos", "El +1 puede asignarse a DES en vez de CON al crear el personaje"],
  "narrativeTags": ["Tribal", "Resiliente"], "unlocks": [] }
```

### 15.5 Sangre Feérica de Ascaria (Linaje mágico / feérico)

**Bono de stats:** +1 INT o +1 CAR a elección. **Rasgo racial — Eco del Velo:** una vez por descanso corto, puedes repetir una prueba de INT o CAR fallida. **Rasgo activo — Paso entre Velos:** una vez por descanso largo, te desplazas a un punto visible a corto alcance de forma instantánea. **Afinidad:** cartas `[Arcano]` (✦) y `[Naturaleza]`; vida base −1 (frágil por naturaleza feérica, se aplica como penalización fija tras calcular la fórmula). **Tags narrativos:** `[Feerico]` `[Extraplanar]`.

```json
{ "id": "race_ascaria_fey_blood", "name": "Sangre Feérica de Ascaria", "type": "race", "tier": 1,
  "languages": ["Común", "Feérico"], "speed": 30, "compatibleDeities": ["deity_veiled_queen"],
  "statBonuses": { "CON": 0, "DES": 0, "INT": 0, "CAR": 0 },
  "racialTrait": { "name": "Eco del Velo", "description": "1/descanso corto: repite una prueba fallida de INT o CAR." },
  "activeTrait": { "name": "Paso entre Velos", "description": "1/descanso largo: teletransporte instantáneo a punto visible de alcance corto." },
  "affinities": ["Arcano", "Naturaleza"], "limitations": ["-1 a la vida máxima final calculada", "El +1 se asigna a INT o CAR al crear el personaje"],
  "narrativeTags": ["Feerico", "Extraplanar"], "unlocks": ["spell_veil_step_t2"] }
```

---

## 16. Trasfondos iniciales: los 12 meses de nacimiento

El trasfondo de un personaje es su **mes de nacimiento**. Cada mes del calendario imprime un
temperamento: una energía dominante, una virtud, un defecto y un propósito vital. Sustituye a los
arquetipos sociales clásicos (soldado, erudito, criminal...): dos soldados nacidos en meses
distintos son personas distintas, y eso importa más que su oficio.

Siguiendo la sección 9.3, el mes **no da stats ni bonos numéricos**: da reglas sociales — virtud,
defecto y objetivo — y una pasiva narrativa ligera. La recompensa por interpretarlo (inspiración,
punto heroico o favor divino, según el mes) es su única mecánica. Los ids coinciden con los del
catálogo de la aplicación (`data/cartas/transfondos`).

| # | Mes | Energía | Recompensa por interpretar |
|--:|---|---|---|
| 1 | Mes de la Escarcha | Vegetación | Inspiración |
| 2 | Mes de la Siembra | Tierra | Inspiración |
| 3 | Mes del Agua | Agua | Inspiración |
| 4 | Mes del Oráculo | Rayo | Favor divino |
| 5 | Mes del Sol | Vegetación | Favor divino |
| 6 | Mes de la Cosecha | Tierra | Inspiración |
| 7 | Mes del Hierro | Aire | Punto heroico |
| 8 | Mes de la Agonía | Vegetación | Inspiración |
| 9 | Mes de la Tierra | Fuego | Inspiración |
| 10 | Mes de los Aullidos | Tierra | Favor divino |
| 11 | Mes de las Sombras | Rayo | Punto heroico |
| 12 | Mes de las Estrellas | Energía Suprema | Favor divino |

### 16.1 Mes de la Escarcha — El Ermitaño Armónico
Sabiduría introspectiva, potencia emocional contenida, conexión mística con la naturaleza. **Virtud:** reflexivo, sereno ante la adversidad, observador incisivo. **Defecto:** apático emocionalmente, desconfiado por naturaleza. **Objetivo:** dominar el conocimiento antiguo. **Competencias narrativas:** Naturaleza, Conocimiento Arcano, Trato con Animales, Persuasión, Interpretación.

```json
{ "id": "mes_de_lescarcha", "name": "Mes de la Escarcha", "type": "background", "tier": 1,
  "virtue": "Reflexivo y sereno ante la adversidad; observador incisivo.",
  "flaw": "Apático emocionalmente; desconfiado por naturaleza.",
  "objective": "Dominar el conocimiento antiguo.",
  "narrativeSkills": ["Naturaleza", "Conocimiento Arcano", "Trato con Animales", "Persuasión", "Interpretación"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "inspiracion", "unlocks": [],
  "flavorText": "Energía: Vegetación. Reservado pero intenso — El Ermitaño Armónico." }
```

### 16.2 Mes de la Siembra — El Custodio del Surco
Enfocado en crear, estabilizar, regenerar; fiel a sus raíces y a sus pactos. **Virtud:** paciente como el arado, protector de la comunidad, perseverancia firme. **Defecto:** testarudo incansable, incapaz de aceptar novedades. **Objetivo:** hacer crecer un pueblo desde la semilla. **Competencias:** Trato con Animales, Conocimiento Arcano, Supervivencia, Persuasión, Atletismo.

```json
{ "id": "mes_de_la_siembra", "name": "Mes de la Siembra", "type": "background", "tier": 1,
  "virtue": "Paciente como el arado; protector de la comunidad; perseverancia firme.",
  "flaw": "Testarudo incansable; incapaz de aceptar novedades.",
  "objective": "Hacer crecer un pueblo desde la semilla.",
  "narrativeSkills": ["Trato con Animales", "Conocimiento Arcano", "Supervivencia", "Persuasión", "Atletismo"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "inspiracion", "unlocks": [],
  "flavorText": "Energía: Tierra. Observador de ciclos — El Custodio del Surco." }
```

### 16.3 Mes del Agua — El Guardián de las Aguas Profundas
Reflexión, conocimiento que fluye, sabiduría ancestral; no busca imponer, sino transformar con tiempo y palabra. **Virtud:** flexible como la corriente, introspectivo y sabio, creativo inagotable. **Defecto:** desconexión emocional, miedo a enfrentarse a la realidad. **Objetivo:** comprender el paso del tiempo y los sueños. **Competencias:** Conocimiento Arcano, Supervivencia, Investigación, Juego de Manos, Persuasión.

```json
{ "id": "mes_de_laigua", "name": "Mes del Agua", "type": "background", "tier": 1,
  "virtue": "Flexible como la corriente; introspectivo y sabio; creativo inagotable.",
  "flaw": "Desconexión emocional; miedo a enfrentarse a la realidad.",
  "objective": "Comprender el paso del tiempo y los sueños.",
  "narrativeSkills": ["Conocimiento Arcano", "Supervivencia", "Investigación", "Juego de Manos", "Persuasión"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "inspiracion", "unlocks": [],
  "flavorText": "Energía: Agua. Soñador lúcido — El Guardián de las Aguas Profundas." }
```

### 16.4 Mes del Oráculo — El Profeta del Trueno
Luchador con voz de verdad; sueña para transformar y sacude los cimientos del mundo con ideas y acción. **Virtud:** inspirado por visiones, valeroso ante lo desconocido, tacto profético. **Defecto:** inestable emocionalmente, incapaz de mantener objetivos a largo plazo. **Objetivo:** traducir las profecías del Cielo Fragmentado. **Competencias:** Investigación, Religión, Interpretación, Intimidación, Engaño.

```json
{ "id": "mes_de_loracle", "name": "Mes del Oráculo", "type": "background", "tier": 1,
  "virtue": "Inspirado por visiones; valeroso ante lo desconocido; tacto profético.",
  "flaw": "Inestable emocionalmente; incapaz de mantener objetivos a largo plazo.",
  "objective": "Traducir las profecías del Cielo Fragmentado.",
  "narrativeSkills": ["Investigación", "Religión", "Interpretación", "Intimidación", "Engaño"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "favor_divino", "unlocks": [],
  "flavorText": "Energía: Rayo. Atraído por el misterio — El Profeta del Trueno." }
```

### 16.5 Mes del Sol — El Corazón de las Hojas
Late con la naturaleza, actúa con pasión y brilla como el sol entre las sombras; hace florecer lo que toca, pero quema si no se controla. **Virtud:** carismático y radiante, protector de la vida, inspirador natural. **Defecto:** vanidoso o narcisista, excesivamente protector. **Objetivo:** esparcir la belleza de la vida a través de la magia natural. **Competencias:** Engaño, Naturaleza, Interpretación, Trato con Animales, Acrobacias.

```json
{ "id": "mes_del_sol", "name": "Mes del Sol", "type": "background", "tier": 1,
  "virtue": "Carismático y radiante; protector de la vida; inspirador natural.",
  "flaw": "Vanidoso o narcisista; excesivamente protector.",
  "objective": "Esparcir la belleza de la vida a través de la magia natural.",
  "narrativeSkills": ["Engaño", "Naturaleza", "Interpretación", "Trato con Animales", "Acrobacias"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "favor_divino", "unlocks": [],
  "flavorText": "Energía: Vegetación. Irradia alegría sin esfuerzo — El Corazón de las Hojas." }
```

### 16.6 Mes de la Cosecha — El Cultivador de Equilibrios
Persona de cimientos sólidos; un puente entre mundos: entre el bosque y el mercado, entre el pueblo y la corona. **Virtud:** trabajador incansable, confiable y leal, generoso con su tiempo y recursos. **Defecto:** demasiado rígido con sus convicciones, obsesionado con el orden. **Objetivo:** cuidar los campos que alimentarán un reino. **Competencias:** Investigación, Naturaleza, Atletismo, Engaño, Medicina.

```json
{ "id": "mes_de_la_cosecha", "name": "Mes de la Cosecha", "type": "background", "tier": 1,
  "virtue": "Trabajador incansable; confiable y leal; generoso con su tiempo y recursos.",
  "flaw": "Demasiado rígido con sus convicciones; obsesionado con el orden.",
  "objective": "Cuidar los campos que alimentarán un reino.",
  "narrativeSkills": ["Investigación", "Naturaleza", "Atletismo", "Engaño", "Medicina"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "inspiracion", "unlocks": [],
  "flavorText": "Energía: Tierra. Pilar silencioso — El Cultivador de Equilibrios." }
```

### 16.7 Mes del Hierro — La Llama del Orden Impuesto
Voluntad encarnada: cuando entra en acción, el mundo se reestructura en torno a él. No busca la paz, sino el equilibrio a través del conflicto. **Virtud:** valiente sin miedo al combate, leal hasta la muerte, resolutivo bajo presión. **Defecto:** temperamento explosivo, dificultad para perdonar. **Objetivo:** derrocar una injusticia arraigada en el mundo. **Competencias:** Religión, Intimidación, Historia, Supervivencia, Conocimiento Arcano.

```json
{ "id": "mes_del_ferro", "name": "Mes del Hierro", "type": "background", "tier": 1,
  "virtue": "Valiente sin miedo al combate; leal hasta la muerte; resolutivo bajo presión.",
  "flaw": "Temperamento explosivo; dificultad para perdonar.",
  "objective": "Derrocar una injusticia arraigada en el mundo.",
  "narrativeSkills": ["Religión", "Intimidación", "Historia", "Supervivencia", "Conocimiento Arcano"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "punto_heroico", "unlocks": [],
  "flavorText": "Energía: Aire. Fuerte como una muralla — La Llama del Orden Impuesto." }
```

### 16.8 Mes de la Agonía — El Portador de la Sombra Dulce
Alma cargada que lleva dentro la belleza del sacrificio y la melancolía; puede abrir caminos donde nadie se atreve a mirar. **Virtud:** persistencia a pesar del dolor, introspectivo y profundo, comprensión del sufrimiento ajeno. **Defecto:** envidia de los caminos ajenos, proclive a la tristeza o la melancolía. **Objetivo:** crear una obra que inmortalice su dolor. **Competencias:** Interpretación, Percepción, Sigilo, Religión, Medicina.

```json
{ "id": "mes_de_lagonia", "name": "Mes de la Agonía", "type": "background", "tier": 1,
  "virtue": "Persistencia a pesar del dolor; introspectivo y profundo; comprende el sufrimiento ajeno.",
  "flaw": "Envidia de los caminos ajenos; proclive a la tristeza o la melancolía.",
  "objective": "Crear una obra que inmortalice su dolor.",
  "narrativeSkills": ["Interpretación", "Percepción", "Sigilo", "Religión", "Medicina"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "inspiracion", "unlocks": [],
  "flavorText": "Energía: Vegetación. Herido pero sabio — El Portador de la Sombra Dulce." }
```

### 16.9 Mes de la Tierra — El Corazón de la Tierra Silenciosa
La esencia de la estabilidad, de aquello que lo sostiene todo sin reclamar protagonismo; la memoria viva del mundo físico y espiritual. **Virtud:** conexión profunda con la naturaleza, arraigado y paciente, protector de los más débiles. **Defecto:** demasiado rígido ante el cambio, incapaz de perdonar una traición. **Objetivo:** hacer crecer una nueva comunidad en un territorio hostil. **Competencias:** Naturaleza, Interpretación, Medicina, Atletismo, Percepción.

```json
{ "id": "mes_de_la_terra", "name": "Mes de la Tierra", "type": "background", "tier": 1,
  "virtue": "Conexión profunda con la naturaleza; arraigado y paciente; protector de los más débiles.",
  "flaw": "Demasiado rígido ante el cambio; incapaz de perdonar una traición.",
  "objective": "Hacer crecer una nueva comunidad en un territorio hostil.",
  "narrativeSkills": ["Naturaleza", "Interpretación", "Medicina", "Atletismo", "Percepción"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "inspiracion", "unlocks": [],
  "flavorText": "Energía: Fuego. Rápido para defender, lento para atacar — El Corazón de la Tierra Silenciosa." }
```

### 16.10 Mes de los Aullidos — El Soñador de las Sombras
Lleva dentro los gritos de los que se fueron y la semilla de un futuro que aún no ha nacido; comprende que todo fin es también un comienzo. **Virtud:** introspección profunda, fiel a sus visiones e intuiciones, coraje ante la muerte y lo desconocido. **Defecto:** proclive al aislamiento emocional, tendencia a idealizar el pasado. **Objetivo:** acompañar almas perdidas hasta el reposo. **Competencias:** Religión, Conocimiento Arcano, Percepción, Historia, Investigación.

```json
{ "id": "mes_dels_aullits", "name": "Mes de los Aullidos", "type": "background", "tier": 1,
  "virtue": "Introspección profunda; fiel a sus visiones; coraje ante la muerte y lo desconocido.",
  "flaw": "Proclive al aislamiento emocional; tendencia a idealizar el pasado.",
  "objective": "Acompañar almas perdidas hasta el reposo.",
  "narrativeSkills": ["Religión", "Conocimiento Arcano", "Percepción", "Historia", "Investigación"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "favor_divino", "unlocks": [],
  "flavorText": "Energía: Tierra. Amante de la noche y del silencio — El Soñador de las Sombras." }
```

### 16.11 Mes de las Sombras — La Llama de la Tempestad Interior
Fuego que arde desde dentro: un alma marcada por el conflicto, con el poder de cambiar el orden establecido. **Virtud:** valentía ante la oscuridad interior, perspicacia para detectar intenciones ocultas, capacidad de adaptación y supervivencia. **Defecto:** tendencia a la obsesión, inestabilidad emocional. **Objetivo:** romper las cadenas que le impusieron desde el nacimiento. **Competencias:** Conocimiento Arcano, Percepción, Engaño, Historia, Religión.

```json
{ "id": "mes_de_les_ombres", "name": "Mes de las Sombras", "type": "background", "tier": 1,
  "virtue": "Valentía ante la oscuridad interior; perspicaz con las intenciones ocultas; superviviente adaptable.",
  "flaw": "Tendencia a la obsesión; inestabilidad emocional.",
  "objective": "Romper las cadenas que le impusieron desde el nacimiento.",
  "narrativeSkills": ["Conocimiento Arcano", "Percepción", "Engaño", "Historia", "Religión"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "punto_heroico", "unlocks": [],
  "flavorText": "Energía: Rayo. Vive en la frontera de la luz y la oscuridad — La Llama de la Tempestad Interior." }
```

### 16.12 Mes de las Estrellas — El Guardián de los Hilos del Destino
Nacido bajo la noche única de las tres lunas, lleva el eco del universo dentro; tiene la responsabilidad de ver y comprender más allá del tiempo. **Virtud:** sagacidad profética, compasión universal, visión clara del pasado, presente y futuro. **Defecto:** expectativas irreales o demasiado elevadas, peligro de aislamiento intelectual. **Objetivo:** escribir una profecía que altere el curso de la historia. **Competencias:** Conocimiento Arcano, Interpretación, Atletismo, Intimidación, Historia.

```json
{ "id": "mes_de_les_estrelles", "name": "Mes de las Estrellas", "type": "background", "tier": 1,
  "virtue": "Sagacidad profética; compasión universal; visión clara del pasado, presente y futuro.",
  "flaw": "Expectativas irreales o demasiado elevadas; peligro de aislamiento intelectual.",
  "objective": "Escribir una profecía que altere el curso de la historia.",
  "narrativeSkills": ["Conocimiento Arcano", "Interpretación", "Atletismo", "Intimidación", "Historia"],
  "contacts": [], "bonusEquipment": [], "roleplayReward": "favor_divino", "unlocks": [],
  "flavorText": "Energía: Energía Suprema. Parece haber nacido viejo — El Guardián de los Hilos del Destino." }
```

---

## 17. Cartas de ejemplo

Los IDs coinciden con los usados como equipo/habilidades iniciales en la sección de clases, de forma que el catálogo es coherente de punta a punta. Ninguna carta tiene coste en puntos: todas declaran `recovery` (sección 6). El resto de habilidades mencionadas en las clases pero no desarrolladas aquí (`skill_defensive_stance`, `skill_interrupt_strike`, `skill_arcane_step`, `skill_staff_strike`) siguen exactamente la misma plantilla y quedan como ejercicio directo de aplicación de las reglas.

### 17.1 Diez habilidades físicas

**Golpe de Escudo** (`skill_shield_bash`) — Tier 1 · ⚔ Acción · requiere `[Escudo]` · daño físico leve + empuje corto · escala con CON · Recuperación: descanso corto.
```json
{ "id": "skill_shield_bash", "name": "Golpe de Escudo", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "⚔", "classTags": ["class_guardian_iron"], "roleTags": ["tank"], "mechanicTags": ["Fisico", "Bloqueo"],
  "requiredTags": ["Escudo"], "recovery": "descanso_corto", "actionType": "accion",
  "range": "melee", "duration": "instant", "defenseStat": "CA",
  "effect": { "description": "Ataca con el escudo: daño físico leve y empuja al objetivo 5 pies.", "scaling": "CON" },
  "limitations": ["Requiere escudo equipado"], "evolvesInto": "skill_shield_bash_t3" }
```

**Interponerse** (`skill_intercept`) — Tier 1 · 🛡 Reacción · redirige la mitad del daño de un aliado cercano a ti · Recuperación: descanso corto.
```json
{ "id": "skill_intercept", "name": "Interponerse", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "🛡", "classTags": ["class_guardian_iron"], "roleTags": ["tank"], "mechanicTags": ["Fisico", "Control"],
  "recovery": "descanso_corto", "actionType": "reaccion", "range": "short", "duration": "instant",
  "effect": { "description": "Cuando un aliado a alcance corto recibe daño, absorbes la mitad del daño en su lugar.", "scaling": "CON" },
  "limitations": ["El aliado debe estar a alcance corto"] }
```

**Tajo Doble** (`skill_double_slash`) — Tier 1 · ⚔ Acción · dos golpes ligeros consecutivos · escala con DES · Recuperación: descanso corto.
```json
{ "id": "skill_double_slash", "name": "Tajo Doble", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "⚔", "classTags": ["class_wandering_blade"], "roleTags": ["balanced"], "mechanicTags": ["Fisico", "Marcial"],
  "recovery": "descanso_corto", "actionType": "accion", "range": "melee", "duration": "instant", "defenseStat": "CA",
  "effect": { "description": "Dos ataques ligeros consecutivos con el arma equipada.", "scaling": "DES" },
  "evolvesInto": "skill_steel_combo_t2" }
```

**Paso de Flanqueo** (`skill_flanking_step`) — Tier 1 · 👣 Movimiento · bonifica el ataque si flanqueas desde un ángulo nuevo · Recuperación: activa (uso libre).
```json
{ "id": "skill_flanking_step", "name": "Paso de Flanqueo", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "👣", "classTags": ["class_wandering_blade"], "roleTags": ["balanced"], "mechanicTags": ["Fisico", "Marcial"],
  "recovery": "activa", "actionType": "movimiento", "range": "self", "duration": "instant",
  "effect": { "description": "Te desplazas; si atacas desde un ángulo distinto al del último aliado que atacó, ganas +1 a la tirada de ataque.", "scaling": "none" } }
```

**Golpe Preciso** (`skill_precise_strike`) — Tier 1 · ⚔ Acción · daño adicional contra objetivos no alertados o flanqueados · escala con DES · Recuperación: descanso corto.
```json
{ "id": "skill_precise_strike", "name": "Golpe Preciso", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "⚔", "classTags": ["class_road_shadow"], "roleTags": ["agile"], "mechanicTags": ["Fisico", "Sigilo"],
  "recovery": "descanso_corto", "actionType": "accion", "range": "melee", "duration": "instant", "defenseStat": "CA",
  "effect": { "description": "Daño adicional si el objetivo no te ha detectado o está flanqueado.", "scaling": "DES" },
  "evolvesInto": "skill_precise_strike_t2" }
```

**Desaparecer entre Sombras** (`skill_vanish`) — Tier 1 · 👣 Acción menor · ventaja en tu próxima prueba de sigilo · Recuperación: descanso corto.
```json
{ "id": "skill_vanish", "name": "Desaparecer entre Sombras", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "👣", "classTags": ["class_road_shadow"], "roleTags": ["agile"], "mechanicTags": ["Sigilo"],
  "recovery": "descanso_corto", "actionType": "accion_menor", "range": "self", "duration": "1_turn",
  "effect": { "description": "Ganas ventaja en tu próxima prueba de sigilo antes de tu siguiente turno.", "scaling": "none" } }
```

**Trampa Rápida** (`skill_quick_trap`) — Tier 1 · ⏳ Preparación · impone desventaja al primer enemigo que la active · Recuperación: descanso corto.
```json
{ "id": "skill_quick_trap", "name": "Trampa Rápida", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "⏳", "classTags": ["class_road_shadow"], "roleTags": ["agile"], "mechanicTags": ["Control", "Fisico"],
  "recovery": "descanso_corto", "actionType": "preparacion", "range": "short", "duration": "permanent",
  "effect": { "description": "Colocas una trampa menor; el primer enemigo que la active sufre desventaja en su próxima tirada de ataque.", "scaling": "none" } }
```

**Esquiva Instintiva** (`skill_instinctive_dodge`) — Tier 1 · 🛡 Reacción · reduce el daño de un ataque recibido · escala con DES · Recuperación: descanso corto.
```json
{ "id": "skill_instinctive_dodge", "name": "Esquiva Instintiva", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "🛡", "classTags": ["class_road_shadow"], "roleTags": ["agile"], "mechanicTags": ["Evasion", "Fisico"],
  "recovery": "descanso_corto", "actionType": "reaccion", "range": "self", "duration": "instant",
  "effect": { "description": "Reduces el daño de un ataque que te impacte.", "scaling": "DES" } }
```

**Golpe con Foco** (`skill_focus_strike`) — Tier 1 · ⚔ Acción · ataque menor con el foco arcano · Recuperación: activa (uso libre).
```json
{ "id": "skill_focus_strike", "name": "Golpe con Foco", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "⚔", "classTags": ["class_arcanist"], "roleTags": ["caster"], "mechanicTags": ["Fisico"],
  "recovery": "activa", "actionType": "accion", "range": "melee", "duration": "instant", "defenseStat": "CA",
  "effect": { "description": "Golpe menor con el foco arcano equipado.", "scaling": "INT" } }
```

**Guía Táctica** (`skill_tactical_guidance`) — Tier 1 · ✨ Acción menor · un aliado cercano gana ventaja en su próxima prueba · escala con CAR · Recuperación: descanso corto.
```json
{ "id": "skill_tactical_guidance", "name": "Guía Táctica", "type": "skill", "tier": 1, "rarity": "common",
  "typeIcon": "✨", "classTags": ["class_dawn_voice"], "roleTags": ["support"], "mechanicTags": ["Control"],
  "recovery": "descanso_corto", "actionType": "accion_menor", "range": "short", "duration": "1_turn",
  "effect": { "description": "Un aliado a alcance corto gana ventaja en su próxima prueba o ataque.", "scaling": "CAR" } }
```

---

### 17.2 Diez hechizos

**Lanza de Brasas** (`spell_ember_lance`) — Tier 1 · INT · alcance medio · daño de fuego, aumentado si el objetivo está `[Quemado]` · Recuperación: activa (truco básico).
```json
{ "id": "spell_ember_lance", "name": "Lanza de Brasas", "type": "spell", "school": "evocacion", "tier": 1,
  "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "activa",
  "range": "medium", "duration": "instant", "mechanicTags": ["Fuego", "Magico"],
  "effect": { "description": "Daño de fuego a un objetivo; si ya está [Quemado], el daño aumenta.", "scaling": "INT" },
  "evolvesInto": "spell_ember_lance_t2" }
```

**Escudo Breve** (`spell_brief_shield`) — Tier 1 · INT · reacción · reduce el daño de un ataque entrante · Recuperación: descanso corto.
```json
{ "id": "spell_brief_shield", "name": "Escudo Breve", "type": "spell", "school": "abjuracion", "tier": 1,
  "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "descanso_corto",
  "range": "self", "duration": "instant", "mechanicTags": ["Magico", "Bloqueo"],
  "effect": { "description": "Reduces el daño del próximo ataque que te impacte este turno.", "scaling": "INT" } }
```

**Mano Invisible** (`spell_unseen_hand`) — Tier 1 · INT · utilidad · manipula objetos pequeños a distancia corta · Recuperación: activa.
```json
{ "id": "spell_unseen_hand", "name": "Mano Invisible", "type": "spell", "school": "invocacion", "tier": 1,
  "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "activa",
  "range": "short", "duration": "1_min", "mechanicTags": ["Magico", "Control"],
  "effect": { "description": "Manipulas objetos pequeños a distancia corta durante 1 minuto.", "scaling": "none" } }
```

**Luz que Sana** (`spell_healing_light`) — Tier 1 · CAR (divino) · alcance corto · restaura vida a un aliado · Recuperación: descanso corto.
```json
{ "id": "spell_healing_light", "name": "Luz que Sana", "type": "spell", "school": "restauracion", "tier": 1,
  "classTags": ["class_dawn_voice"], "castingStat": "CAR", "recovery": "descanso_corto",
  "range": "short", "duration": "instant", "mechanicTags": ["Sagrado", "Curacion"],
  "effect": { "description": "Restaura puntos de vida a un aliado a alcance corto.", "scaling": "CAR" },
  "limitations": ["No puede apuntarse a uno mismo salvo carta especial"], "evolvesInto": "spell_healing_light_t2" }
```

**Bendición Menor** (`spell_minor_blessing`) — Tier 1 · CAR (divino) · un aliado gana +1 a ataque o salvación durante 1 minuto · Recuperación: descanso corto.
```json
{ "id": "spell_minor_blessing", "name": "Bendición Menor", "type": "spell", "school": "abjuracion", "tier": 1,
  "classTags": ["class_dawn_voice"], "castingStat": "CAR", "recovery": "descanso_corto",
  "range": "short", "duration": "1_min", "mechanicTags": ["Sagrado", "Control"],
  "effect": { "description": "Un aliado gana +1 a sus próximas tiradas de ataque o salvación.", "scaling": "CAR" } }
```

**Esquirla de Escarcha** (`spell_frost_shard`) — Tier 1 · INT · alcance medio · daño de hielo y reduce el movimiento del objetivo · Recuperación: descanso corto.
```json
{ "id": "spell_frost_shard", "name": "Esquirla de Escarcha", "type": "spell", "school": "evocacion", "tier": 1,
  "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "descanso_corto",
  "range": "medium", "duration": "1_turn", "mechanicTags": ["Hielo", "Magico", "Control"],
  "effect": { "description": "Daño de hielo; el objetivo reduce su movimiento a la mitad hasta su próximo turno.", "scaling": "INT" } }
```

**Guardia Arcana** (`spell_arcane_ward`) — Tier 2 · INT · canalización · crea una barrera que absorbe daño · Recuperación: descanso largo.
```json
{ "id": "spell_arcane_ward", "name": "Guardia Arcana", "type": "spell", "school": "abjuracion", "tier": 2,
  "classTags": ["class_arcanist"], "castingStat": "INT", "recovery": "descanso_largo",
  "range": "self", "duration": "concentration", "mechanicTags": ["Magico", "Bloqueo", "Concentracion"],
  "effect": { "description": "Genera una barrera que absorbe daño igual a 2× tu INT antes de romperse.", "scaling": "INT" },
  "incompatibleTags": ["Concentracion"] }
```

**Zarpa de la Naturaleza** (`spell_natures_grasp`) — Tier 2 · CAR (afinidad natural) · alcance corto · inmoviliza brevemente al objetivo · Recuperación: descanso corto.
```json
{ "id": "spell_natures_grasp", "name": "Zarpa de la Naturaleza", "type": "spell", "school": "naturaleza", "tier": 2,
  "classTags": [], "castingStat": "CAR", "recovery": "descanso_corto",
  "range": "short", "duration": "1_turn", "mechanicTags": ["Naturaleza", "Control"],
  "effect": { "description": "Raíces o zarzas inmovilizan al objetivo hasta el final de su próximo turno (salvación de CON para resistir).", "scaling": "CAR" } }
```

**Pulso Radiante** (`spell_radiant_pulse`) — Tier 2 · CAR (divino) · área corta · daño a enemigos y cura leve a aliados en la zona · Recuperación: descanso largo.
```json
{ "id": "spell_radiant_pulse", "name": "Pulso Radiante", "type": "spell", "school": "restauracion", "tier": 2,
  "classTags": ["class_dawn_voice"], "castingStat": "CAR", "recovery": "descanso_largo",
  "range": "short", "area": "radio 10 pies", "duration": "instant", "mechanicTags": ["Sagrado", "Area", "Curacion"],
  "effect": { "description": "Daño [Sagrado] a enemigos y curación leve a aliados dentro del área.", "scaling": "CAR" } }
```

**Paso entre Velos** (`spell_veil_step_t2`) — Tier 2 · INT · utilidad · pequeño teletransporte instantáneo · Recuperación: descanso corto.
```json
{ "id": "spell_veil_step_t2", "name": "Paso entre Velos", "type": "spell", "school": "invocacion", "tier": 2,
  "classTags": [], "castingStat": "INT", "recovery": "descanso_corto",
  "range": "short", "duration": "instant", "mechanicTags": ["Magico", "Feerico"],
  "effect": { "description": "Te desplazas instantáneamente a un punto visible de alcance corto.", "scaling": "none" },
  "limitations": ["Desbloqueado por defecto para el linaje Sangre Feérica de Ascaria"] }
```

---

### 17.3 Diez pasivas

Las cinco primeras son pasivas de clase (`unique: true`, ver sección 9.4); las demás son pasivas raciales, de trasfondo o secundarias de tier, que conviven con la pasiva de clase sin sustituirla.

**Muro Viviente** (`passive_living_wall`) — Tier 1 · clase · mientras lleves escudo, reduces el primer daño físico recibido cada ronda en una cantidad igual a tu CON.
```json
{ "id": "passive_living_wall", "name": "Muro Viviente", "type": "passive", "tier": 1, "classTags": ["class_guardian_iron"],
  "trigger": "Escudo equipado, cada ronda", "effect": { "description": "Reduce el primer daño físico recibido cada ronda en una cantidad igual a tu CON.", "scaling": "CON" },
  "synergyTags": ["Escudo", "Bloqueo"], "unique": true }
```

**Instinto de Combate** (`passive_combat_instinct`) — Tier 1 · clase · una vez por combate, al caer bajo la mitad de tu vida, tu próximo ataque tiene ventaja.
```json
{ "id": "passive_combat_instinct", "name": "Instinto de Combate", "type": "passive", "tier": 1, "classTags": ["class_wandering_blade"],
  "trigger": "1/combate, al caer bajo el 50% de vida máxima", "effect": { "description": "Tu próximo ataque tiene ventaja.", "scaling": "none" },
  "synergyTags": ["Marcial"], "unique": true }
```

**Paso Silencioso** (`passive_silent_step`) — Tier 1 · clase · ventaja en sigilo si te moviste este turno sin atacar.
```json
{ "id": "passive_silent_step", "name": "Paso Silencioso", "type": "passive", "tier": 1, "classTags": ["class_road_shadow"],
  "trigger": "Te has movido este turno sin atacar", "effect": { "description": "Ventaja en tu próxima prueba de sigilo.", "scaling": "none" },
  "synergyTags": ["Sigilo", "Evasion"], "unique": true }
```

**Canalizador Arcano** (`passive_arcane_channeler`) — Tier 1 · clase · una vez por descanso corto, un hechizo de `recovery: descanso_corto` vuelve a la pila Activa tras resolverse.
```json
{ "id": "passive_arcane_channeler", "name": "Canalizador Arcano", "type": "passive", "tier": 1, "classTags": ["class_arcanist"],
  "trigger": "1/descanso corto, al resolver un hechizo con recovery descanso_corto", "effect": { "description": "El hechizo vuelve a la pila Activa en vez de ir a su pila de descanso.", "scaling": "none" },
  "synergyTags": ["Arcano"], "unique": true }
```

**Aliento Estable** (`passive_steady_breath`) — Tier 1 · clase · al inicio de tu turno, si un aliado cercano está bajo la mitad de su vida, recupera 1 punto.
```json
{ "id": "passive_steady_breath", "name": "Aliento Estable", "type": "passive", "tier": 1, "classTags": ["class_dawn_voice"],
  "trigger": "Inicio de turno, aliado a alcance corto bajo 50% de vida", "effect": { "description": "El aliado recupera 1 punto de vida.", "scaling": "none" },
  "synergyTags": ["Curacion", "Sagrado"], "unique": true }
```

**Paso del Dosel** (`passive_canopy_stride_t3`) — Tier 3 · racial (Elfo del Dosel) · ignoras terreno difícil natural (raíces, maleza, ramas bajas).
```json
{ "id": "passive_canopy_stride_t3", "name": "Paso del Dosel", "type": "passive", "tier": 3, "classTags": [],
  "trigger": "Pasiva constante", "effect": { "description": "Ignoras terreno difícil de origen natural.", "scaling": "none" },
  "synergyTags": ["Silvano"], "unique": false }
```

**Calma Devota** (`passive_devout_calm_t2`) — Tier 2 · desbloqueada por meses de energía divina (Oráculo, Sol, Aullidos, Estrellas) · ventaja en pruebas de CAR para resistir miedo o manipulación mientras portes un símbolo sagrado.
```json
{ "id": "passive_devout_calm_t2", "name": "Calma Devota", "type": "passive", "tier": 2, "classTags": [],
  "trigger": "Símbolo sagrado equipado", "effect": { "description": "Ventaja en pruebas de CAR para resistir miedo o manipulación.", "scaling": "none" },
  "synergyTags": ["Sagrado"], "unique": false }
```

**Piel de Roble** (`passive_oak_skin_t3`) — Tier 3 · secundaria del Guardián de Hierro · al descansar, elige un tipo de daño; ganas resistencia a él hasta tu próximo descanso.
```json
{ "id": "passive_oak_skin_t3", "name": "Piel de Roble", "type": "passive", "tier": 3, "classTags": ["class_guardian_iron"],
  "trigger": "Al completar un descanso corto o largo", "effect": { "description": "Eliges un tipo de daño y ganas resistencia a él hasta tu próximo descanso.", "scaling": "none" },
  "synergyTags": ["Bloqueo"], "unique": false }
```

**Segundo Aliento** (`passive_second_wind_t3`) — Tier 3 · secundaria del Sable Errante · al iniciar combate, recuperas vida igual a tu CON, una vez por descanso largo.
```json
{ "id": "passive_second_wind_t3", "name": "Segundo Aliento", "type": "passive", "tier": 3, "classTags": ["class_wandering_blade"],
  "trigger": "1/descanso largo, al iniciar combate", "effect": { "description": "Recuperas vida igual a tu CON.", "scaling": "CON" },
  "synergyTags": ["Marcial"], "unique": false }
```

**Reflejos Felinos** (`passive_feline_reflexes_t3`) — Tier 3 · secundaria de Sombra del Camino · una vez por combate, ventaja en una salvación de DES.
```json
{ "id": "passive_feline_reflexes_t3", "name": "Reflejos Felinos", "type": "passive", "tier": 3, "classTags": ["class_road_shadow"],
  "trigger": "1/combate", "effect": { "description": "Ventaja en una tirada de salvación de DES.", "scaling": "none" },
  "synergyTags": ["Evasion"], "unique": false }
```

---

### 17.4 Diez objetos de equipo

Todos usan los 6 slots fijos de la sección 9.7 y declaran su categoría de peso (△ ○ □) y, si aplica, su afinidad (✦ ☠ ✝ ♞ ⚙). Las armas declaran `precisionStat`.

**Escudo Reforzado** (`item_reinforced_shield`) — Slot: arma secundaria · ○ Medio · +1 CON · otorga `[Escudo]`.
```json
{ "id": "item_reinforced_shield", "name": "Escudo Reforzado", "type": "equipment", "slot": "arma_secundaria", "tier": 1, "rarity": "common",
  "statBonuses": { "CON": 1 }, "penalties": {}, "compatibilitySymbols": ["○"], "grantedTags": ["Escudo", "Bloqueo"],
  "restrictions": ["-1 a pruebas de sigilo mientras esté equipado"] }
```

**Armadura de Malla** (`item_chain_armor`) — Slot: torso · □ Pesado · +2 vida · −1 DES · otorga `[Armadura Pesada]`.
```json
{ "id": "item_chain_armor", "name": "Armadura de Malla", "type": "equipment", "slot": "torso", "tier": 1, "rarity": "common",
  "statBonuses": { "health": 2 }, "penalties": { "DES": 1 }, "compatibilitySymbols": ["□"], "grantedTags": ["Armadura Pesada"],
  "restrictions": ["Incompatible con hechizos arcanos de Tier 2+ salvo carta especial"] }
```

**Espada Corta** (`item_short_sword`) — Slot: arma principal · △ Ligero · precisión DES · `[Arma Ligera]`.
```json
{ "id": "item_short_sword", "name": "Espada Corta", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
  "statBonuses": {}, "precisionStat": "DES", "compatibilitySymbols": ["△"], "grantedTags": ["Arma Ligera", "Fisico"] }
```

**Espada Larga** (`item_longsword`) — Slot: arma principal · ○ Medio · precisión DES (a dos manos: CON y gana `[Arma Pesada]`, perdiendo el slot de arma secundaria).
```json
{ "id": "item_longsword", "name": "Espada Larga", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
  "statBonuses": {}, "precisionStat": "DES", "compatibilitySymbols": ["○"], "grantedTags": ["Arma Ligera"],
  "restrictions": ["Puede empuñarse a dos manos: pasa a precisión CON y tag [Arma Pesada], dejando inutilizable el slot de arma secundaria"] }
```

**Armadura Media** (`item_medium_armor`) — Slot: torso · ○ Medio · +1 vida · `[Armadura Media]`.
```json
{ "id": "item_medium_armor", "name": "Armadura Media", "type": "equipment", "slot": "torso", "tier": 1, "rarity": "common",
  "statBonuses": { "health": 1 }, "penalties": {}, "compatibilitySymbols": ["○"], "grantedTags": ["Armadura Media"],
  "restrictions": ["-1 a pruebas de sigilo"] }
```

**Dagas Gemelas** (`item_twin_daggers`) — Slot: arma principal · △ Ligero · +1 DES · precisión DES · `[Arma Ligera]` `[Sigilo]`.
```json
{ "id": "item_twin_daggers", "name": "Dagas Gemelas", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
  "statBonuses": { "DES": 1 }, "precisionStat": "DES", "compatibilitySymbols": ["△"], "grantedTags": ["Arma Ligera", "Sigilo"] }
```

**Armadura Ligera** (`item_light_armor`) — Slot: torso · △ Ligero · +1 DES · `[Armadura Ligera]`.
```json
{ "id": "item_light_armor", "name": "Armadura Ligera", "type": "equipment", "slot": "torso", "tier": 1, "rarity": "common",
  "statBonuses": { "DES": 1 }, "compatibilitySymbols": ["△"], "grantedTags": ["Armadura Ligera"] }
```

**Foco Arcano** (`item_arcane_focus`) — Slot: arma secundaria · △ Ligero · ✦ Arcano · +1 INT · habilidad propia vinculada.
```json
{ "id": "item_arcane_focus", "name": "Foco Arcano", "type": "equipment", "slot": "arma_secundaria", "tier": 1, "rarity": "common",
  "statBonuses": { "INT": 1 }, "compatibilitySymbols": ["△", "✦"], "grantedTags": ["Arcano"], "linkedSkill": "skill_focus_strike" }
```

**Símbolo Sagrado** (`item_holy_symbol`) — Slot: arma secundaria · △ Ligero · ✝ Sagrado · +1 CAR · requerido por hechizos divinos.
```json
{ "id": "item_holy_symbol", "name": "Símbolo Sagrado", "type": "equipment", "slot": "arma_secundaria", "tier": 1, "rarity": "common",
  "statBonuses": { "CAR": 1 }, "compatibilitySymbols": ["△", "✝"], "grantedTags": ["Sagrado"] }
```

**Bastón Simple** (`item_simple_staff`) — Slot: arma principal · △ Ligero · precisión CON · compatible con conjuración.
```json
{ "id": "item_simple_staff", "name": "Bastón Simple", "type": "equipment", "slot": "arma_principal", "tier": 1, "rarity": "common",
  "statBonuses": {}, "precisionStat": "CON", "compatibilitySymbols": ["△"], "grantedTags": ["Arma Ligera"], "linkedSkill": "skill_staff_strike" }
```

---

## 18. Personajes de ejemplo

Cinco personajes completos que muestran el sistema en juego: cómo se combinan raza, mes de
nacimiento, clase, cartas y equipo, cómo se calcula la vida paso a paso con la fórmula de la
sección 5 (`Vida Base + CON×3 + CAR×1 + Bono de Tier + Bono de Equipo`) y qué perfil de pilas
(sección 6) tiene cada uno en la mesa.

---

### 18.1 Borkun Piedra-Fría — El Tanque

**Raza:** Enano de las Fraguas Profundas · **Mes:** Mes del Hierro · **Clase:** Guardián de Hierro · **Tier:** 2

**Stats:** CON 5 · DES 1 · INT 1 · CAR 2 *(CON 3 base + 1 racial + 1 por progresión de Tier 2)*

**Vida:**
```
CON efectiva (5 + Escudo Reforzado +1) = 6 · CAR efectiva = 2
Vida = 16 (base clase) + (6 × 3) + (2 × 1) + 4 (bono Tier 2) + 2 (Armadura de Malla)
Vida = 16 + 18 + 2 + 4 + 2 = 42
```

**Mano (4 cartas de habilidad):** *Golpe de Escudo* (⚔ desc. corto), *Interponerse* (🛡 desc. corto), *Provocar* (desc. corto, nueva de Tier 2), *Postura Defensiva* (⚔ activa). Siempre activas, fuera de mano: pasiva de clase *Muro Viviente*, rasgo racial *Sangre de Forja*, y la pasiva narrativa de su mes, *La Llama del Orden Impuesto*.

**Equipo:** Escudo Reforzado (○), Armadura de Malla (□), Espada Corta (△).

**Perfil de juego:** primera línea del grupo. Absorbe golpes gracias a Muro Viviente, redirige daño con Interponerse y mantiene la amenaza enemiga sobre sí mismo. Tras cada combate le quedan casi todas las cartas en Descanso Corto: con un descanso de una hora vuelve a estar entero — el Guardián funciona a ciclos cortos, como manda su clase.

---

### 18.2 Liesel Vantorra — La Maga Frágil

**Raza:** Sangre Feérica de Ascaria · **Mes:** Mes de las Estrellas · **Clase:** Arcanista · **Tier:** 1

**Stats:** CON 1 · DES 2 · INT 4 · CAR 1 *(INT 3 base + 1 racial, asignado a INT)*

**Vida:**
```
CON efectiva = 1 · CAR efectiva = 1
Vida = 8 (base clase) + (1 × 3) + (1 × 1) + 0 (bono Tier 1) + 0 (equipo) − 1 (penalización racial fija)
Vida = 8 + 3 + 1 + 0 + 0 − 1 = 11
```

**Mano (5 cartas):** *Lanza de Brasas* (✨ activa), *Mano Invisible* (✨ activa), *Golpe con Foco* (⚔ activa), *Escudo Breve* (🛡 desc. corto), *Paso Arcano* (👣 desc. corto). Siempre activas: pasiva de clase *Canalizador Arcano*, rasgo racial *Eco del Velo*, pasiva del mes *El Guardián de los Hilos del Destino*.

**Equipo:** Foco Arcano (△ ✦, +1 INT → INT efectiva 5), Ropa Ligera (△), Daga Simple (△).

**Perfil de juego:** 11 puntos de vida la convierten en la pieza más frágil del grupo — cualquier golpe directo es peligroso. A cambio, su mano casi no se agota: tres de sus cinco cartas son de uso libre, y Canalizador Arcano le permite recuperar Escudo Breve una vez entre descansos. Depende del posicionamiento y de que el Tanque mantenga la amenaza lejos de ella.

---

### 18.3 Renn Doslunas — El Pícaro Ágil

**Raza:** Elfo del Dosel · **Mes:** Mes de las Sombras · **Clase:** Sombra del Camino · **Tier:** 2

**Stats:** CON 2 · DES 4 · INT 2 · CAR 1 *(DES 3 base + 1 racial; CON 1 base + 1 por progresión de Tier 2, priorizando sobrevivir a un golpe)*

**Vida:**
```
CON efectiva = 2 · CAR efectiva = 1
Vida = 10 (base clase) + (2 × 3) + (1 × 1) + 4 (bono Tier 2) + 0 (equipo)
Vida = 10 + 6 + 1 + 4 + 0 = 21
```

**Mano (5 cartas):** *Ataque Furtivo Mejorado* (⚔ desc. corto, evolución de Golpe Preciso en Tier 2), *Desaparecer entre Sombras* (👣 desc. corto), *Trampa Rápida* (⏳ desc. corto), *Esquiva Instintiva* (🛡 desc. corto), más una utilidad a elección. Siempre activas: pasiva de clase *Paso Silencioso*, rasgo activo racial *Paso Élfico*, pasiva del mes *La Llama de la Tempestad Interior*.

**Equipo:** Dagas Gemelas (△, +1 DES), Armadura Ligera (△, +1 DES) — DES efectiva 6, CA = 16 · Kit de Herramientas.

**Perfil de juego:** golpea fuerte desde el sigilo y se retira antes de que el enemigo pueda responder. Toda su mano va a Descanso Corto: dos combates seguidos sin descansar lo dejan sin cartas, así que decidir cuándo parar es parte de jugar bien a la Sombra. 21 de vida es su margen real de error.

---

### 18.4 Hermana Aine — La Sanadora

**Raza:** Humano de las Marcas · **Mes:** Mes del Sol · **Clase:** Voz del Alba · **Tier:** 1

**Stats:** CON 2 · DES 1 · INT 2 · CAR 3 *(CAR 2 base + 1 elegido por rasgo humano)*

**Vida:**
```
CON efectiva = 2 · CAR efectiva (3 + Símbolo Sagrado +1) = 4
Vida = 10 (base clase) + (2 × 3) + (4 × 1) + 0 (bono Tier 1) + 0 (equipo)
Vida = 10 + 6 + 4 + 0 + 0 = 20
```

**Mano (4 cartas):** *Luz que Sana* (❤ CAR, desc. corto), *Bendición Menor* (✨ CAR, desc. corto), *Guía Táctica* (✨ desc. corto), *Golpe de Bastón* (⚔ activa). Siempre activas: pasiva de clase *Aliento Estable*, rasgo humano *Adaptable*, pasiva del mes *El Corazón de las Hojas*.

**Equipo:** Símbolo Sagrado (△ ✝, +1 CAR), Equipo Ligero (△), Bastón Simple (△).

**Perfil de juego:** su CAR alta hace doble trabajo — potencia su magia divina (conjura con CAR, sección 9.6) y aporta a su vida y Defensa mental (14). Sus tres cartas de apoyo van a Descanso Corto: la decisión central de cada combate es a quién curar ahora y a quién pedirle que aguante hasta el descanso. Brilla en grupo, no en solitario.

---

### 18.5 Grosh "Puño Sereno" — El Híbrido (Tanque/Sanador)

**Raza:** Orco de Gongorguma · **Mes:** Mes de los Aullidos · **Clase:** Guardián de Hierro (especialización **Custodio Sagrado**, desde Tier 2) · **Tier:** 3

**Stats:** CON 5 · DES 1 · INT 1 · CAR 3 *(CON 3 base + 1 racial + 1 progresión Tier 2; CAR 2 base + 1 progresión Tier 3 — el Custodio cura vía CAR)*

**Vida:**
```
CON efectiva (5 + Escudo Reforzado +1) = 6 · CAR efectiva = 3
Vida = 16 (base clase) + (6 × 3) + (3 × 1) + 9 (bono Tier 3) + 2 (Armadura de Malla)
Vida = 16 + 18 + 3 + 9 + 2 = 48
```

**Mano (5 cartas):** *Golpe de Escudo Mayor* (⚔ desc. corto, evolución de Tier 3), *Interponerse* (🛡 desc. corto), *Provocar* (desc. corto), *Postura Defensiva* (⚔ activa), *Bendición del Custodio* (❤ desc. corto, habilidad de especialización: cura a un aliado adyacente al bloquear un ataque). Siempre activas: pasiva de clase *Muro Viviente*, pasiva secundaria *Piel de Roble* (Tier 3), rasgo racial *Furia Contenida*, pasiva del mes *El Soñador de las Sombras*.

**Equipo:** Escudo Reforzado (○), Armadura de Malla (□), Espada Corta (△).

**Perfil de juego:** ejemplo de hibridación **dentro de una misma clase** vía especialización, en vez de multiclase: sigue siendo mecánicamente un Guardián de Hierro (misma vida base, mismo perfil de pilas), pero su rama Custodio Sagrado reemplaza parte de su kit ofensivo por curación ligada a CAR — por eso su progresión de Tier 3 fue a CAR y no a CON. Con 48 de vida es el personaje más resistente del grupo y además sostiene al resto: el arquetipo de "tanque que también cura" resuelto sin tocar la fórmula base, solo cambiando qué cartas elige.

---

### Tabla comparativa

| Personaje | Clase | Tier | Vida final | Mano | Perfil de pilas |
|---|---|---:|---:|---:|---|
| Borkun Piedra-Fría | Guardián de Hierro | 2 | 42 | 4 | Ciclos cortos: 3 cartas a desc. corto |
| Liesel Vantorra | Arcanista | 1 | 11 | 5 | 3 de uso libre + 2 a desc. corto |
| Renn Doslunas | Sombra del Camino | 2 | 21 | 5 | Todo a desc. corto: juega a ráfagas |
| Hermana Aine | Voz del Alba | 1 | 20 | 4 | 3 a desc. corto + 1 de uso libre |
| Grosh "Puño Sereno" | Guardián de Hierro (Custodio Sagrado) | 3 | 48 | 5 | Ciclos cortos + curación |

La tabla ilustra el rango de vida pretendido por rol con el multiplicador unificado de la
edición 2: la maga frágil (11) frente al tanque (42-48) es una diferencia de ~4×, que sale de la
vida base de clase, de dónde invierte stats cada personaje (CON y CAR alimentan la vida; DES e
INT no) y del equipo — no de fórmulas distintas por clase. La columna de pilas sustituye a la
antigua columna de "recurso máximo": el recurso de un personaje de la edición 2 es dónde están
sus cartas.

---

## 19. Presentación visual: plantilla de carta y mesa de personaje

Un personaje no se lee como una hoja de personaje ni como un PDF de texto corrido: se lee como un conjunto de cartas colocadas físicamente delante del jugador (o, en la app, su equivalente digital). Esta sección fija dos cosas: la plantilla visual que comparten todas las cartas del juego, y la distribución oficial de "mesa" con la que se organiza un personaje completo. La versión digital actual (`personatges/{id}`) ya implementa la distribución de mesa descrita en 19.2.

### 19.1. Plantilla universal de carta (5 zonas)

Da igual si la carta es una habilidad, una espada o un rasgo racial: el jugador debe saber siempre dónde mirar sin leer toda la carta. Toda carta se divide en 5 zonas fijas, siempre en el mismo orden:

| Zona | Contenido |
|---|---|
| ① Cabecera | Nombre + iconos de tipo + tier |
| ② Ilustración | Arte principal de la carta *(pendiente: la app todavía no gestiona imágenes por carta)* |
| ③ Reglas | Texto mecánico (efecto, bonos, requisitos) |
| ④ Pie | Etiquetas · coste · rareza |
| ⑤ Identificador | ID · expansión · número de carta |

Con cientos de cartas, esta consistencia es lo que permite aprender el juego sin leer un manual: el coste siempre está abajo, el nombre siempre arriba, las reglas siempre en el centro. Las páginas `/cartas/*` actuales ya cubren las zonas ①③④⑤ (tier+nombre, descripción/reglas, tags/rareza, ID); la zona ② (ilustración) queda como mejora futura pendiente de un sistema de subida de imágenes.

### 19.2. Distribución de mesa de un personaje (5 zonas)

Si todos los personajes usan la misma distribución, cualquiera puede leer el estado de un personaje ajeno solo mirando la mesa, sin preguntar "¿cuánta vida tienes?". La ficha (`personatges/{id}/detall.html`) implementa estas 5 zonas en este orden:

1. **Identidad** — Pasiva, Clase, Raza, Trasfondo y Estadísticas (CON/DES/INT/CAR + Vida + CA) en una fila. Es la parte que casi no cambia durante la aventura.
2. **Equipamiento** — los 6 slots fijos (sección 9.7) colocados en forma de silueta: Cabeza arriba; Arma principal, Torso y Arma secundaria en la fila central; Piernas y Pies debajo.
3. **Combate** — todas las cartas de habilidad preparadas, en rejilla. Es la zona más grande, donde vive la toma de decisiones.
4. **Recuperación** — pilas de Descanso Corto, Descanso Largo y Exiliadas (sección 6). *Estado actual: la app dibuja las tres pilas pero no lleva la cuenta en vivo de qué carta está en cuál — eso requiere una sesión de combate persistente, que todavía no existe. Hasta entonces, toda habilidad preparada se considera disponible.*
5. **Recursos temporales** — Invocaciones, Estados, Bendiciones y Maldiciones. Ninguno de estos cuatro tipos de carta existe todavía en el catálogo (ver `docs/UML_Sistema_Cartas_Tiers.mermaid`, bloque "propuesto" — `Enemigo`/`CartaAventura` y familia). La zona queda reservada en el layout para cuando se implementen.

Si en el futuro se juega en mesa física además de en la app, la recomendación es un tablero de jugador (~A4) con estas mismas zonas impresas vacías, sin texto de reglas — solo los huecos marcados — para que cualquier jugador nuevo sepa automáticamente dónde va cada carta.

---

## 20. Diseño de aventuras: mazos de historia por actos

Igual que un personaje es un mazo de cartas (sección 3), una **aventura es tres mazos de cartas de historia** — Acto I, Acto II y Acto III — que se barajan, se roban y dejan rastro físico en la mesa. El sistema resuelve el problema central de toda aventura prediseñada: qué pasa cuando los jugadores ignoran una misión. Aquí, ignorar una carta no rompe la historia; la **poda**. Los hilos que los jugadores abandonan desaparecen del juego mediante un filtrado físico entre actos, y vuelven después como consecuencias, no como misiones pendientes. El principio: *los jugadores deciden qué ignorar, el filtro decide qué muere, y el mazo decide cuándo les explota en la cara.*

Esta sección extiende la plantilla universal de carta (19.1) a un nuevo tipo, la **Carta de Historia**, y define el flujo de mesa del director: preparación, resolución, filtrado y cierre.

### 20.1. Componentes físicos

| Componente | Función |
|---|---|
| **Cartas de Historia** | Una escena o evento por carta, con su **código narrativo** en la cabecera (`A1-03`, `B2-07`, `C3-12`: acto + número) |
| **Fichas de Estado** | 🟢 **Verde** = completada/éxito · 🔴 **Roja** = ignorada/fracaso. Se colocan sobre la carta al resolverla y no se retiran en toda la partida |
| **Sobre de "Historia Perdida"** | Aparta las cartas eliminadas por el filtrado. No se juegan nunca; los jugadores no saben que existían |

Las pilas resueltas de actos anteriores **se quedan sobre la mesa con sus fichas encima**: son la memoria de la partida y se consultan constantemente.

### 20.2. Anatomía de la Carta de Historia

Reutiliza las 5 zonas de la sección 19.1 con dos adaptaciones:

| Zona | Contenido específico de historia |
|---|---|
| ① Cabecera | Código narrativo (`A1-03`) + título + símbolo 🔗 si es carta de secuencia fija |
| ③ Reglas | Texto de la escena, con ramas condicionales internas ("si `A1-02` tiene ficha Verde… / si tiene Roja…") y la línea de gancho *"Si ignoran esto…"* |
| ④ Pie | **Etiqueta de Activación**: vacía = carta **Base** (siempre entra en el mazo) · `Requisito: A1-02 = Verde` o `= Roja` = solo entra si se cumple |

La Etiqueta de Activación es el equivalente narrativo de los requisitos de equipo (9.7): un metadato que decide si la carta participa, no cómo se juega.

### 20.3. Flujo de partida

**Acto I — barajado total.** Se barajan TODAS las cartas del Acto I (no hay nada que filtrar todavía) y se roban una a una. Los jugadores deciden en cada carta si la resuelven o pasan de largo; al terminarla, el director coloca ficha Verde o Roja sobre ella. **El acto termina cuando el mazo se agota**, con independencia de cuántas cartas se completaran: el paso de acto no depende del éxito sino del agotamiento de la baraja, así que la aventura nunca se atasca.

**Actos II y III — filtrado previo.** Antes de barajar, el director hace el **Filtrado de Mazo**: extiende las cartas del acto boca arriba, lee la Etiqueta de Activación de cada una y la coteja con las fichas de las pilas anteriores (el Acto III se filtra contra Acto I *y* Acto II):

- Requisito cumplido (pide Verde y hay Verde, o pide Roja y hay Roja) → **entra en el mazo**.
- Requisito incumplido → **al sobre de Historia Perdida**. Esa subtrama muere; la historia fluye como si ese camino nunca hubiera existido.
- Carta Base (sin requisito) → **entra siempre**.

Las supervivientes se barajan y ese es el mazo del acto. Si los jugadores preguntan por una opción que dependía de una carta descartada ("¿podemos usar la llave?"), la respuesta es que esa opción no existe en su partida: nunca conocieron al herrero que la forjaba.

**Cartas 🔗 de secuencia fija.** Una carta con símbolo 🔗 y numeración (`Orden: 1/3`, `2/3`…) **no se baraja nunca**: tras el filtrado se coloca boca abajo encima del mazo en su orden, y el resto de cartas barajadas van debajo. Sirve para tramos que exigen orden (una infiltración paso a paso, el descenso de piso en una mazmorra) dentro de un acto por lo demás caótico.

**Cartas Inyectadas (consecuencia en caliente).** Para que un hilo ignorado reaparezca *en mitad* de un acto posterior sin reordenar mazos, se diseñan cartas **Base** (entran siempre) cuyo texto interno consulta las fichas en el momento de robarse:

> *"Mientras cruzáis el bosque: si `A1-02` (El Pacto) tiene ficha ROJA, el demonio que ignorasteis aparece ahora mismo y os tiende una emboscada. Si tiene VERDE, encontráis un ciervo herido."*

La carta se adapta sola al estado de la partida en el instante exacto en que se lee. Es el mecanismo estándar para que lo ignorado no vuelva como misión nueva, sino como consecuencia explosiva en medio de lo que están haciendo.

**Carta de Balance Final.** Cada aventura cierra con una carta especial que nunca se filtra ni se baraja: se juega siempre la última. Su texto es una tabla de consecuencias que se lee contando las fichas de las tres pilas:

- *3 o más Rojas en Acto I → el reino arde en llamas.*
- *Más Verdes que Rojas en Acto II → los dioses os favorecen.*
- *`A1-01` en Roja → el asesino que ignorasteis os apuñala al recibir la recompensa.*

Da cierre explícito a todos los hilos, incluidos los podados.

### 20.4. Las 5 reglas de oro para escribir Cartas de Historia

1. **Autocontención.** El mazo se baraja: la carta 6 puede salir antes que la 2. Cada carta debe contener un mini-conflicto que se plantea y se resuelve en ella misma, aunque alimente la trama grande.
2. **Efecto mariposa obligatorio.** Toda carta que pueda acabar en Roja DEBE tener una consecuencia en el Acto II o III (una carta gemela filtrada, o una rama en una carta inyectada). Si ignorarla no repercute, la ficha roja sobra y la decisión del jugador no valía nada.
3. **Semáforo de carga.** Máximo 1 requisito duro por Etiqueta de Activación (2 en casos excepcionales). Una carta que exige tres fichas concretas a la vez casi nunca pasará el filtro: es trabajo de escritura desperdiciado.
4. **Válvula de escape.** Al menos el **40% de las cartas de cada acto deben ser Base**. Garantiza que, aunque los jugadores ignoren todo, los mazos de los Actos II y III siempre tengan 3-4 cartas jugables y la aventura no se quede en blanco.
5. **Gancho de cierre.** Cada carta termina con una línea *"Si ignoran esto…"* que el director lee en voz alta al colocar la ficha Roja. Los jugadores sienten que *ellos* decidieron omitirlo, y que el mundo sigue girando sin ellos.

### 20.5. Las 3 arquitecturas de aventura

Tres columnas vertebrales para estructurar una aventura sobre este sistema. Se pueden mezclar, pero conviene elegir una como base.

| Arquitectura | Ideal para | Los actos son… | Mecánica distintiva |
|---|---|---|---|
| **1. Misión Principal** ("espina de pescado") | Épica lineal, gran villano, viaje largo | I: preparativos y viaje · II: camino traicionero · III: enfrentamiento final | Cartas Base cuyo texto cambia según fichas ("el puente está reparado si `A1-02` es Verde; roto y defendido si es Roja"). Mismo final, dificultades y aliados variables |
| **2. Facciones** ("ramo de flores") | Ciudades abiertas, gremios, mercenarios | No hay misión principal: 3-4 facciones con 2 cartas cada una en Acto I | Dos Verdes de una facción activan su carta *Aliado* en Acto II; dos Rojas activan *Enemigo*. El Acto III se resuelve por recuento: la facción con más Verdes decide el final. Máxima rejugabilidad |
| **3. Crawler** (mazmorra/exploración) | Dungeon crawl, mansión, continente inexplorado | Los actos son **profundidad**, no tiempo: I superficie · II subterráneo · III guarida del jefe | Robar carta = abrir puerta. Las cartas 🔗 marcan el descenso de nivel. Verdes en exploración dan ventajas contra el jefe; Rojas hacen que los monstruos ignorados refuercen el Acto III (+1 dificultad) |

**Acto 0 (mazo de aprendizaje).** Si la aventura es compleja, las 3 primeras cartas del Acto I se marcan 🔗 como tutorial en orden forzoso: escenas sencillas que enseñan la mecánica Completar/Ignorar. A partir de la carta 4, barajado total.

### 20.6. La Matriz de Cruce (herramienta de escritura)

Antes de escribir el texto de las cartas, el diseñador rellena esta tabla: una fila por cada carta del Acto I que pueda ser ignorada, indicando qué activa cada color. Es la forma rápida de verificar las reglas 2 y 3 (todo tiene consecuencia; nada exige demasiado).

| Código Acto I | Nombre | Verde (éxito) activa… | Roja (ignorada) activa… |
|---|---|---|---|
| `A1-03` | El Mapa del Tesoro | `B2-07` (ruta segura) | `B2-11` (bandidos) |
| `A1-07` | La Cura del Veneno | `C3-02` (el rey está sano) | `C3-09` (el rey muere delante de ellos — carta inyectada) |
| `A1-10` | El Pacto con el Demonio | — (callejón sin salida) | `C3-15` (el demonio cobra la deuda) |

Un hilo argumental (una misión, una subtrama) es por tanto un **mini-árbol repartido entre actos**: presentación en Acto I, desarrollo en Acto II, desenlace en Acto III, unido por sus códigos en la matriz. Como los mazos se barajan, los hilos se entrelazan en orden imprevisible, pero la matriz permite al director reconstruir el contexto de cualquier carta robada.

### 20.7. Chuleta del director

Tarjeta resumen para tener en mesa:

| Momento | Acción |
|---|---|
| Resolver carta | Ficha 🟢 si la completan, 🔴 si la ignoran; leer el gancho *"Si ignoran esto…"* con la Roja |
| Fin de acto | El mazo se agota → siguiente acto (el éxito no importa para avanzar) |
| Preparar Acto II | Filtrar contra fichas de la pila A1; descartes al sobre de Historia Perdida |
| Preparar Acto III | Filtrar contra fichas de A1 **y** B2 |
| Carta 🔗 | No barajar: boca abajo encima del mazo, en su orden |
| Carta inyectada | Al robarla, consultar las fichas que cite su texto y narrar la rama que corresponda |
| Última carta | Balance Final: contar Verdes/Rojas de las tres pilas y leer el epílogo que aplique |

*Estado de implementación: este sistema está definido para mesa física. Su equivalente digital (entidades `adventure`/`event`/`quest` de `docs/Arquitectura_Datos_Onegai.md` y el bloque propuesto `CartaAventura` del UML) puede automatizar el filtrado y el recuento de fichas, pero no existe todavía en la app. Para generar aventuras de este sistema (a mano o con IA), usar los prompts de `docs/Plantilla_Prompt_Contenido.md §18`: esqueleto+matriz → cartas → validación.*
