# Revisión del reglamento Ed. 4 (combate, tiers, moral, menús)

**Autor:** Claude (Cowork) · **Fecha:** 17/09/2026 · **Fuente:** mensaje de Oriol del 17/09 + sus
respuestas a cuatro preguntas de esa misma sesión.

Esto **no es el reglamento**: es la revisión del texto de Oriol contra lo que hay hoy en el motor.
El reglamento canónico (GDD Ed. 4 + `decisiones/D0007`) lo está escribiendo **qwen**, que tiene fila
propia en el TABLERO («Combate Ed. 4»). Este documento existe para que esa fila y quien venga
después tengan en un sitio: (1) lo que Oriol decidió, (2) las contradicciones que quedan sin
resolver, y (3) el coste real en código de cada regla.

---

## 1. Decisiones tomadas por Oriol (17/09, en sesión)

| # | Tema | Decisión |
|---|---|---|
| 1 | **Crítico** | **La mitad del pool en seises** (4d6 → 2 seises; 6d6 → 3). Sustituye a «todos los dados seis» de Ed. 3. |
| 2 | **Hechizos** | **Tirada enfrentada**, igual que el ataque físico. Se descarta el umbral fijo «mínimo N éxitos = tu característica» que él mismo había descrito. |
| 3 | **Salvación de hechizo** | Fallarla = **daño entero + efecto de impacto**. Superarla = **mitad de daño y sin efecto**. El «doble» de su texto era el entero visto desde la mitad. |
| 4 | **Daño físico** | Se tiran tantos d6 como diga el margen de éxitos y **el daño es la suma** de esos dados (1 dado con un 5 = 5 de daño; 2 dados que suman 9 = 9 de daño). |

---

## 2. El reglamento tal como queda

### 2.1 Pool (sin cambios respecto a Ed. 3 — ya programado)
- Se tiran N d6, N = la característica que aplique.
- **6 = 1 éxito · 5 = ½ éxito · 1-4 = nada.**
- **Ventaja:** el 4 también suma ½. **Desventaja:** sólo cuentan los 6.
- **Pifia:** ningún 5 ni 6 **y** al menos dos 1.

### 2.2 Ataque físico
1. El **arma decide la característica**: la que permite usar la fuerza tira CON, la de precisión (daga)
   tira DES. *Hoy no existe ese campo en el catálogo de objetos: hay que añadirlo.*
2. El defensor tira su pool (DES/CON). *Hoy `SkillExecutor` usa el **mayor** de DES/CON; si el arma
   del atacante elige característica, conviene decidir si el defensor también elige o sigue con el mayor.*
3. **Margen = éxitos del atacante − éxitos del defensor.** Margen ≤ 0 → **daño 0**: el bloqueo entero
   que pide Oriol sale solo de la fórmula, no hace falta una regla aparte.
4. **Dados de daño = margen redondeado**: **0,5 sube a 1** (única excepción); cualquier otro X,5 baja
   (1,5 → 1; 2,5 → 2).
5. **Daño = suma de esos d6.**
6. **Crítico:** dobla los dados de daño (2 → 4).
7. **Pifia del atacante:** el defensor **no tira** y puede **contraatacar**.

Comprobación con los ejemplos de Oriol:

| Ataque | Defensa | Margen | Dados | ¿Cuadra? |
|---|---|---|---|---|
| 5,5,6 (2) | 5,5 (1) | 1 | 1 | ✅ (él dijo 1) |
| 6,6,6,5 (3,5) | 6,5 (1,5) | 2 | 2, crítico → 4 | ✅ |
| 5,6,6 (2,5) | 6 (1) | 1,5 | 1 (baja) | ✅ |
| 6,5 (1,5) | 6 (1) | 0,5 | 1 (sube) | ✅ |
| 6,5 (1,5) | — (0) | 1,5 | 1 (baja) | ❌ **él dijo 2** |

### 2.3 Hechizos
- **Enfrentada**, sin umbral: lanzador contra defensor.
- Defensor pierde → **daño entero de la carta** (2d6 = la suma) **+ efecto de impacto**.
- Defensor gana **o empata** → **mitad de daño, sin efecto**.
- *Hoy el motor hace otra cosa:* si gana el defensor, `opposed_degree` devuelve `BOTCH` = **0 daño y
  sin debuff**; el empate da mitad. Con Ed. 4, ganar la salvación ya no anula el daño: lo parte.

### 2.4 Inmunidades
Jefes que ignoran estados alterados; enemigos normales inmunes a un tipo de daño o a un estado.
*Hoy `ConditionEngine` no consulta inmunidades y el catálogo de enemigos no las trae.*

### 2.5 Tiers y vida
- Tier nuevo al llegar a **nivel 20, 40, 60, 80 y 100**. Cada tier da más ataques y mejoras, y sube la
  dificultad de los encuentros.
- **Vida fija por clase y doblando por tier:** mago 8 → 16 → 32 → 64 → 128; tanque 12 → 24 → 48 → 96 → 192.
- *Hoy:* `assets/rules/tier_rules.json` ya trae seis tiers (0..5) pero **sin nivel mínimo**, y la vida
  sale de `VidaBaseClase + CON*3 + CAR*1` **acotada por `healthCap`** (27/35/50/70/95/125). Las bases
  por clase sí coinciden (mago 8, tanque 12).

### 2.6 Menú del jugador
Lista de misiones, mapamundi, mapas visitados y marcas de misiones/eventos.
*Hoy no existe nada de esto*, y además depende de que el juego llegue a ejecutar eventos de mapa
(pendiente en `ALCANCE_REAL_2026-09-17.md`).

### 2.7 Interfaz de combate
Hechizos como **cartas** con previsualización, selección de objetivo (un enemigo, varios, o aliados) e
**iconos de buffos/debuffos** propios y del enemigo.
*Hoy:* `BattleAction` sólo admite **un** objetivo (`targetIndex`) y el menú de batalla es la lista fija
{Atacar, Tajo, Cura, Pocion, Huir}; no hay HUD de estados.

### 2.8 Moral de dos ejes (modelo Fable II)
- **Bien ↔ Mal** (lo que haces a los demás) y **Puro ↔ Corrupto** (cuánto te sacrificas frente a
  cuánto explotas), cada uno de −1000 a +1000, y nueve arquetipos en la combinación.
- Fuentes: misiones y decisiones de historia, matar/robar/sacrificar, donaciones, **alquileres y
  precios de tienda**, comida y bebida, vida sexual y familiar.
- Efectos: aspecto del personaje, y percepción de los NPC separada en fama, miedo, atractivo y
  amor/odio.
- *Hoy:* `Game/Morality` es **un solo eje de −100 a +100** con cinco etiquetas, alimentado por el
  alquiler, y el guardado almacena ese entero → **hace falta migrar partidas** (propuesta: valor×10 al
  eje moral, pureza a 0) y el HUD usa `fraction()`, que pasa a necesitar dos barras.

---

## 3. Lo que queda sin decidir (bandeja de Oriol)

1. **El caso 1,5 − 0.** Con la regla de redondeo que él mismo dio, son 1 dado; en su último ejemplo
   dijo 2. Las dos lecturas posibles:
   - **(a) Margen de éxitos** (la del cuadro de arriba): 1,5 → 1 dado.
   - **(b) Resta por caras:** (seises del atacante − seises del defensor) + (cincos − cincos) = 1+1 = **2 dados**.
   La lectura (b) cuadra con ese ejemplo pero **rompe el tercero** (5,6,6 contra 6 daría 2 dados y él
   dijo 1). Hace falta que Oriol elija una; el resto del reglamento no cambia.
2. **¿El crítico atraviesa el bloqueo?** Si el defensor iguala o supera, el margen es 0. Con «se
   bloquea el daño entero» el crítico también se bloquea; si se quiere que un crítico siempre haga
   algo, hay que escribirlo.
3. **Redondeo del crítico en pools impares:** 3d6 y 5d6 — ¿dos y tres seises (hacia arriba) o uno y dos?
4. **Vida doblada contra `healthCap`.** El doblado de tier 5 (128 mago, 192 tanque) pasa del tope
   actual (125). Y si la vida dobla cada tier, el daño (media 3,5 por dado) tiene que crecer igual o
   los combates de tier alto duran el triple de turnos: ¿dados de daño por tier, o multiplicador del arma?
5. **Contraataque por pifia:** ¿es un ataque normal fuera de turno, una vez por pifia? ¿el que
   contraataca puede a su vez pifiar?

---

## 4. Coste en código (mapa rápido)

| Regla | Qué se toca | Estado |
|---|---|---|
| Pool, ventaja/desventaja, pifia | `RPG/DicePoolEngine.h` | **ya hecho** |
| Crítico = mitad de seises | `DicePoolEngine::roll_pool` (`critical`) | cambio de una condición |
| Margen → dados de daño → suma | `DicePoolEngine` (nuevo) + `BattleState`/`SkillExecutor` | **nuevo** |
| Salvación: mitad en vez de cero | `opposed_degree` + `SkillExecutor` | cambio de regla |
| Característica del arma (CON/DES) | catálogo de objetos + `BattleState` | **nuevo campo** |
| Inmunidades | catálogo de enemigos + `ConditionEngine` | **nuevo** |
| Nivel → tier | `assets/rules/tier_rules.json` + `TierRules` | campo nuevo |
| Vida fija doblada | `TierRules::compute_cap_health`, `CharacterSheet` | cambio de fórmula |
| Objetivos múltiples y aliados | `BattleAction`, `BattleState` | **nuevo** |
| Cartas de hechizo e iconos de estado | HUD de combate (no existe) | **nuevo** |
| Menú del jugador (misiones, mapamundi) | nada que reaprovechar | **nuevo**, depende de eventos de mapa |
| Moral de dos ejes | `Game/Morality.*`, `SaveGame` (migración), HUD | reescritura acotada |
