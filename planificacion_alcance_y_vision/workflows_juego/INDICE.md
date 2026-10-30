# Workflows de juego — índice y matriz de cruces

> **Qué es esto**: la capa de diseño que falta entre `DOCUMENTO_DE_ALCANCE_Y_VISION.md` (la visión) y el Gantt `AV0…AV12` (las tareas).
> Cada flujo responde a la misma pregunta: **qué tiene que pasar, con qué números, quién se entera, qué existe ya en el motor y qué tiene que decidir Oriol antes de programar.**
> **Estado**: DISEÑO. Ningún fichero de esta carpeta describe código existente salvo donde se cita el fichero y la línea.
> **Generado**: 28/09/2026 por qwen. Las rutas de evidencia son relativas a `MotorGraphico-main/MotorGraphico/`.

---

## 1. Los siete flujos nucleares

Todo el juego pasa por estos siete. Están escritos con la plantilla completa de 13 apartados.

| WF | Flujo | Fuente del doc | Tareas del Gantt | Base en el motor |
|---|---|---|---|---|
| [WF-01](wf-01-combate-tactico.md) | Combate táctico: cartas, pool de d6, caída sin Game Over | §6, §10.6, §10.7 | AV5-01…AV5-10, AV1-04, AV1-05 | **sólida**: `BattleState`, `DicePoolEngine`, `EnemyBrain`, `ConditionEngine`, botín |
| [WF-02](wf-02-movimiento-y-locomocion.md) | Movimiento: locomoción, estamina, ruido y carga | §10.12 | AV7-01…AV7-07, AV1-07 | **parcial**: movimiento por celdas y controles; nada de estamina, ruido ni peso en kg |
| [WF-03](wf-03-compras-y-comercio.md) | Compras, ventas, regateo y hurto | §5.4, §10.14.C, §11.7 | AV4-01, AV4-02, AV9-08, AV6-12 | **parcial y duplicada**: `openShop/buyOffer` funciona; `ShopEngine` paralelo sin usar |
| [WF-04](wf-04-moral-reputacion-y-mutacion.md) | Moral, reputación territorial y mutación física | §3, §10.3, §10.5 | AV3-01…AV3-07, AV1-01 | **a medias**: `Morality` y `factionRep` existen pero casi nada los llama |
| [WF-05](wf-05-negocios-propiedades-y-rentas.md) | Negocios, propiedades, rentas e inversión urbana | §5, §10.8, §10.9, §11.4 | AV4-03…AV4-09, AV6-10, AV10-02 | **funcionando**: comprar negocio, alquiler 50-200 %, `collectRent()` con pruebas |
| [WF-06](wf-06-equipo-ficha-y-progresion.md) | Equipo, ficha y progresión por hitos | §10.7, §6.1, §3.1 | AV5-09, AV5-10, AV7-04, AV12-04 | **sólida**: ficha, tiers, hitos, checks de equipar con motivos |
| [WF-07](wf-07-misiones-investigacion-y-narrativa.md) | Misiones orgánicas, investigación y narrativa | §7, §10.10, §10.11, §11.2 | AV10-01…AV10-06, AV4-13, AV4-14 | **sólida**: aventuras, beats, flags, timers y diario; faltan rumores, bibliotecas y gremio |

## 2. Los ocho flujos secundarios

Ficha corta en [CATALOGO_FLUJOS_SECUNDARIOS.md](CATALOGO_FLUJOS_SECUNDARIOS.md): FS-01 crimen y alerta · FS-02 prisión · FS-03 identidad y disfraz · FS-04 vínculos y romance · FS-05 oficios · FS-06 IA viva y patrullas · FS-07 escenografía y clima · FS-08 hogar y decoración.

## 3. La plantilla de cada flujo

Los 13 apartados son siempre los mismos, para poder comparar flujos de un vistazo:

1. **Qué tiene que sentir el jugador** — la promesa, en una frase sin tecnicismos.
2. **Actores y estado** — qué entidades intervienen, dónde viven hoy y **quién manda** (canon / cartas / motor, BITÁCORA §2.1).
3. **Magnitudes** — rango, fórmula, quién escribe y quién lee. Es la materia prima de AV0-01 (glosario único).
4. **Disparadores** — qué inicia el flujo.
5. **Flujo paso a paso** — diagrama Mermaid + tabla de pasos con su regla dura y su modo de fallo.
6. **Máquina de estados** — diagrama Mermaid de estados y transiciones.
7. **Reglas y números** — con su procedencia (sección del doc, reglamento o línea de código).
8. **Consecuencias cruzadas** — qué otros flujos se enteran de lo que ha pasado aquí.
9. **Feedback al jugador** — HUD, animación y audio.
10. **Casos límite, abusos y antipatrones** — lo que rompe el juego si no se decide ahora.
11. **Qué YA existe en el motor y qué falta** — con fichero y línea, para no planificar dos veces lo mismo (AV0-03).
12. **Decisiones pendientes de Oriol** — con opciones, recomendación y qué bloquea.
13. **Criterio de cierre y guion de prueba jugable** — lo que se ejecuta en AV12-02.

Los diagramas Mermaid se ven directamente en Obsidian y en GitHub.

## 4. Matriz de cruces: quién escribe y quién lee cada magnitud

Una magnitud con dos escritores y ninguna tabla de autoridad es un bug de diseño. Esta matriz es el insumo de **AV0-01 (glosario único)**.

| Magnitud | Escribe | Lee | Estado en el motor |
|---|---|---|---|
| Oro | WF-03, WF-05, WF-07 | WF-03, WF-05, FS-01 (fianza), FS-02 | existe (`gold()`, `SaveGame`) |
| Moral | WF-04 (hoy solo vía WF-05 alquiler) | WF-03, WF-04, FS-01, FS-04 | **tres implementaciones** en conflicto |
| Reputación por facción | WF-04, WF-07 | WF-03, FS-01, FS-03, FS-06 | existe en la ficha, **nadie la llama** |
| Estamina | WF-02 | WF-01, WF-02, FS-02, FS-05 | **no existe** |
| Ruido | WF-02, WF-01 | FS-01, FS-06 | **no existe** |
| Peso / carga | WF-06 | WF-02, WF-03, FS-01 | existe como categoría, no como kg |
| PV y vida tope | WF-01, WF-06 | HUD, FS-02 | existe (`healthCap`, tope 125) |
| Mano y mazo | WF-06 | WF-01 | existe en datos (`handLimit`, `maxSkillCards`) |
| Hitos y tier | WF-06, WF-07 | WF-01, WF-06 | existe (`grantMilestones`, `tierUp`) |
| Flags y objetivos | WF-07 | diario, contenido, FS-04 | existe (`NarrativeState`) |
| Tiempo de mundo | calendario (AV10-02) | WF-05, FS-02, FS-05, FS-06, WF-03 (restock) | **no existe**: el reloj de rentas cuenta pasos |
| Apariencia | WF-04 | FS-03, FS-04, FS-06 | **no existe** |
| Sospecha / alerta | FS-01 | FS-06, WF-03, WF-01 | **no existe** |
| Prosperidad de zona | WF-05 | FS-01, FS-07, WF-03 | **no existe** |

## 5. Los cuellos de botella del diseño

Tres cosas bloquean más flujos que ninguna otra. Si se deciden primero, el resto se ordena solo:

1. **El calendario de mundo** (AV10-02). Lo necesitan WF-03 (restock), WF-05 (rentas semanales, cosechas), FS-02 (condenas), FS-05 (jornadas), FS-06 (rutinas 24 h). Hoy el único reloj cuenta **pasos del jugador** (`kStepsPerRentCycle = 40`).
2. **El glosario único de magnitudes** (AV0-01). Hoy «moral» significa tres cosas distintas en el código y el doc pide una cuarta (territorial). Sin esto, cada sistema mueve su propio número.
3. **La autoridad del reglamento sobre el doc de visión** (BITÁCORA §2.1). Cuatro apartados del documento contradicen el reglamento ya implementado: dados, crítico/pifia, progresión por EXP y seis atributos. Ver [DISCREPANCIAS_Y_DECISIONES.md](DISCREPANCIAS_Y_DECISIONES.md).

## 6. Cómo se usa esto antes de programar

1. Oriol lee las **decisiones pendientes** (apartado 12 de cada flujo + registro central) y decide. Son **56 decisiones numeradas** (7 de combate, 5 de movimiento, 8 de comercio, 9 de moral, 9 de negocios, 9 de equipo y 9 de misiones); ninguna requiere código para resolverse, y el índice completo se regenera en `PLAN_ALCANCE_Y_VISION.md` §7.
2. Lo decidido se aplica al `DOCUMENTO_DE_ALCANCE_Y_VISION.md` (la visión se corrige donde contradice al reglamento) y a `dades.py` (las tareas cambian de alcance).
3. `python3 build.py` regenera el plan, el Gantt y la página: los ficheros generados no se editan a mano.
4. Cada flujo tiene un **guion de prueba jugable** (apartado 13): los siete juntos son la partida guionizada de AV12-02.
5. Cuando un flujo se implementa, su apartado 11 se actualiza con la evidencia nueva y la sesión correspondiente en `sesiones/` lo cita.

## 7. Ficheros de esta carpeta

| Fichero | Contenido |
|---|---|
| `INDICE.md` | este fichero |
| `wf-01`…`wf-07` | los siete flujos nucleares |
| `CATALOGO_FLUJOS_SECUNDARIOS.md` | los ocho secundarios en ficha corta |
| `DISCREPANCIAS_Y_DECISIONES.md` | registro central: doc contra motor, y la bandeja de decisiones |
