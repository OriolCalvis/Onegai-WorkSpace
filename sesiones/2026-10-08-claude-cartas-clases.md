# 2026-10-08 · Claude (Cowork) · CARTAS-CLASES-01 — las clases del motor suben al creador de cartas: mazo físico en PDF y reglamento Ed. 4

Encargo directo de Oriol (08/10, en catalán): integrar en el creador web de cartas (`dndWeebCC`)
todo lo hecho con las clases en el motor; poder **descargar las cartas de habilidad para jugar las
clases en el rol de mesa**; actualizar el reglamento para unificarlo; y decirle si las métricas del
motor están más pulidas o si aún es pronto para decidirlo. Decisión escrita:
[`decisiones/D0012`](../decisiones/D0012-clases-y-metricas-del-motor-al-creador-de-cartas.md).

- **Commits:** ninguno; todo queda sin commitear en el árbol de trabajo de dndWeebCC-master (§6).
- **Deja esperando a Oriol:** `./mvnw test` y arrancar la web en su Mac; las 6 preguntas en verde del GDD §23, sin prisa; el sentido del flujo de las clases (D0012 §4).

| Ruta (`dndWeebCC-master/`) | Qué |
|---|---|
| `scripts/importar_clases_motor.py` | NUEVO — sube del motor (solo lee `MotorGraphico/assets/catalogs`) las 127 clases con sus habilidades, hechizos, pasivas, invocaciones, equipo inicial, set de Tier 2 y condiciones. Upsert por id: no borra, conserva el orden de las claves y los campos que solo tiene el creador; idempotente. |
| `data/cartas/{clases,habilidades,hechizos,pasivas,invocaciones,armas,condiciones}/` | Su salida: **1.223 cartas nuevas y 442 actualizadas**; una segunda pasada da 1.665 iguales. |
| `src/main/java/cat/dnd/cc/eines/mazo/` | NUEVOS — `CartaFisica` (una carta tal como se imprime), `MazoDeClase` (qué cartas lleva el mazo de una clase y cómo se leen en Ed. 4; sin Spring), `PdfCartas` (el PDF escrito a mano con las fuentes base del formato: **sin dependencias nuevas** en el `pom.xml`) y `MetricasPdf` (anchos de letra). |
| `src/main/java/cat/dnd/cc/service/MazoClaseService.java` | NUEVO — busca las cartas en los repositorios; PDF de una clase, de varias o ZIP. |
| `controller/TierClassController.java` | `GET /cartas/clases/{id}/mazo.pdf`, `/cartas/clases/mazos.pdf?familia=` y `/cartas/clases/mazos.zip?familia=`; familia y mazo en la ficha y en el álbum. |
| `model/CartaConExtras.java` + los 7 modelos `Tier*` + los 7 `Tier*Controller` | Los modelos guardan los campos que no conocen (`@JsonAnyGetter`/`@JsonAnySetter`) y, al editar desde el formulario, se copian de la carta guardada: **el editor web ya no borra** ganchos, sets, rarezas... |
| `eines/CalculadoraVida.java`, `service/PersonatgeService.java` | Vida Ed. 4: base + CON×3 + CAR + bono de tier + equipo, con techo por tier (18 en Tier 1). |
| `templates/cartas/clases/{detalle,lista}.html`, `static/css/{tokens,mazo-fisico}.css` | Panel «Mazo físico para jugar en la mesa» en la ficha; filtro y descarga por familia en el álbum. |
| `data/reglas/referencia_mesa.json` | NUEVO — cinco cartas de reglas de mesa que cierran cada mazo. |
| `docs/Sistema_Cartas_Tiers.md` | GDD **Ed. 4**: §3-§7, §10 y §11 reescritas; §21 (encuentros), §22 (las clases y su mazo) y §23 (lo que sigue en verde) nuevas. |
| `scripts/README.md`, `data/README.md` | Filas del script y de `data/reglas`. |
| `src/test/java/...` | `CalculadoraVidaTest` reescrito, `PersonatgeServiceTest` (vida Ed. 4), NUEVOS `MazoDeClaseTest`, `PdfCartasTest` y `CartaConExtrasTest`. |

Fuera de `dndWeebCC-master/`: `decisiones/D0012-...`, esta sesión, la fila del TABLERO e `INDICE.md`.

---

## 1. Qué ve Oriol

- **Ficha de una clase** (`/cartas/clases/{id}`): panel «Mazo físico para jugar en la mesa» con la
  lista de cartas tal como se imprimen (tipo, título, tier, la línea de mecánica y el aviso si la
  carta aún no está cerrada), **Descargar el mazo (PDF)**, «Ver el PDF» y las opciones: añadir el
  set de Tier 2, sin equipo, sin cartas de Creador, sin cartas de reglas.
- **Álbum de clases** (`/cartas/clases`): filtro por familia (3.ª iteración, propias, SP por clase
  base, 2.ª iteración) y dos botones: todos los mazos de la familia en un PDF, o un ZIP con un PDF
  por clase.
- **El PDF:** A4, 9 cartas de 63,5 × 88,9 mm (tamaño de carta de juego) con marcas de corte; marco
  de color por tipo, gema de tier, elemento con su color del canon, fichas (objetivo, alcance,
  duración, pila), la línea de tirada («Tira 1 + DES contra CON o DES (la mayor) · +2d6 si gana»),
  el texto de la carta y, en rojo, lo que la carta todavía no cierra. Orden: clase, pasiva(s), las
  7 estándar, hechizos, las 3 de Creador (Tier 2), invocaciones, equipo inicial, [set], 5 de reglas.

## 2. Verificación

Hecha:
- Copia del repo en el contenedor de la sesión. Lo nuevo y lo modificado compila con
  `javac --release 17` contra stubs escritos a mano de las APIs de Spring/Jackson (los jar no
  están: Maven Central está bloqueado en el contenedor y el acceso a `~/.m2` del Mac fue
  **denegado**). Las 15 pruebas que no necesitan Spring pasan (`CalculadoraVidaTest` 5,
  `MazoDeClaseTest` 6, `PdfCartasTest` 4).
- Los mazos con los datos de verdad: 127 clases → 2.089 cartas; el PDF de todas, 270 hojas,
  ~1,4 MB, `qpdf --check` sin errores; hojas rasterizadas y revisadas a ojo (Bardo, Apóstata,
  Ancla del Vacío, Ángel vengador, Cazasoles). El CSS, con una maqueta estática en Chromium.
- En el Mac: los 24 originales comprobados por md5 antes de sobrescribirlos (nadie los había
  tocado); los 36 ficheros escritos y comprobados por md5; el importador ejecutado aquí
  (1.223/442; segunda pasada, 1.665 iguales) y `data/cartas` idéntico, carpeta a carpeta, al que
  se probó en el contenedor.

Sin hacer (**queda para Oriol**):
- `./mvnw test` (corre además `PersonatgeServiceTest` y `CartaConExtrasTest`, que necesitan
  Mockito y Jackson), arrancar la web y abrir una ficha y el álbum: las plantillas Thymeleaf se
  han revisado a mano, no ejecutado.

## 3. Hallazgos

- El GDD de la web (Ed. 2) contradecía la Ed. 3 (CA y CD) y ninguna de sus cifras estaba simulada;
  el motor lo resuelve todo enfrentado y su balance sale del simulador. Por eso el reglamento
  adopta el motor (D0012 §3).
- **El editor web borraba lo que no conocía**: guardar una carta desde el formulario perdía los
  ganchos, la rareza, el set... Arreglado con `CartaConExtras` (y su prueba).
- 142 cartas impresas llevan aviso en rojo: las 57 cartas de clase de la 2.ª iteración («sus
  cartas aún no se han adaptado a la Ed. 4») y 85 de las clases nuevas, las que el motor marca con
  un gancho `pendiente:` (19 habilidades y 2 pasivas) o `pendiente_parcial:` (44 y 20). El GDD §22
  decía «20 bloqueadas»: medido sobre los ganchos son 19, y así queda corregido.
- Las **defensas** de la ficha de personaje (CA, defensa mental, resistencia física) siguen siendo
  las de la Ed. 2; solo la vida pasa a Ed. 4.
- Las razas y las deidades del motor no se han subido (fuera del encargo).

## 4. La opinión que pidió Oriol

Las métricas del motor están más pulidas y se adoptan (D0012 §3): probadas, simuladas, coherentes
con la Ed. 3 y con lo que Oriol pide del juego. Le marqué 15 puntos en verde; él cerró cuatro
el mismo día (§8, `D0013`) y los que quedan esperan sin prisa (GDD §23).

## 5. Siguientes pasos

1. Oriol: `./mvnw test` y arrancar la web (§6). Si algo falla, es de esta sesión.
2. Oriol: D0012 §4 y los puntos del GDD §23.
3. Cada vez que cambien las clases del motor: `python3 scripts/importar_clases_motor.py`
   (antes, `--dry-run`).
4. Pendientes fuera del encargo: defensas Ed. 4 en la ficha de personaje; subir razas y deidades;
   marcar `MotorGraphico-main/GDD_MG.md` como histórico (zona del motor).

## 6. Sin commit: por qué y cómo hacerlo

En el sandbox no se puede borrar, así que un commit exige `commit-tree`, escribir la rama a mano y
renombrar locks (BITACORA Parte 3). El control de permisos de esta sesión no lo permitió sobre el
repo compartido y no lo he forzado. Desde el Mac, después de `./mvnw test` y de mirar
`git status --short` (debe salir `pom.xml` más lo de esta sesión):

    cd "$HOME/Documents/Documentos - Oriol Os (2)/Software/dndWeebCC-master"
    git add -- data/README.md data/reglas docs/Sistema_Cartas_Tiers.md scripts/README.md \
      scripts/importar_clases_motor.py src/main/java/cat/dnd/cc src/main/resources/static/css \
      src/main/resources/templates/cartas/clases src/test/java/cat/dnd/cc
    git commit -m "feat(clases): mazo fisico en PDF por clase y reglamento Ed. 4 (CARTAS-CLASES-01, D0012)"
    git add -- data/cartas
    git commit -m "data: las 127 clases del motor con sus cartas (CARTAS-CLASES-01, D0012)"

`pom.xml` **no** va: es WIP ajeno sin commitear (`java.version` 25 → 17). Lo nuevo compila con
`--release 17`, así que vale con cualquiera de las dos versiones.

## 7. Concurrencia y restos

- Fila CARTAS-CLASES-01 reclamada antes del primer fichero. `MotorGraphico/` solo se ha leído;
  BND-ESCENAS-01 (otra sesión de Claude) trabaja en el motor y no toca `dndWeebCC-master/`.
- Restos: `dndWeebCC-master/target/claude_cartas_clases/` (74 MB: copias del repo y del motor para
  pasarlos al contenedor, y dos ficheros de una prueba de renombrado). Es desechable y está fuera
  de git; el sandbox no puede borrarlo: `./mvnw clean` (o borrar la carpeta a mano).

## 8. Respuesta de Oriol (08/10, tarde) → `D0013`

Oriol, molesto con razón: el crítico se le había preguntado muchas veces, también después de
decidirlo. Lo que dijo, y dónde ha quedado:

- **Crítico = al menos dos 6** (con un dado no hay; con dos, los dos; con tres o más, basta con dos):
  «ha de sortir 2 6 si tens més de 3 daus o més». Junto con su «sí» del 05/10 a que pida al menos dos
  dados.
- **Los topes de dados del equipo 2, 4, 6, 8, 8 y 8 son ley**, no propuesta: «No, són llei».
- **Atributo de las cartas:** las estándar alternan los dos atributos principales de la clase; las de
  Creador, casi cualquiera.
- **Tier 1 basta** para el estudio de dificultad.
- «Tot lo altre ja ho treballarem; el web no és tant important a dia d'avui.»

Hecho: [`decisiones/D0013`](../decisiones/D0013-reglas-cerradas-por-oriol-critico-topes-atributo-tier1.md)
(`accepted`, con sus palabras y lo que sustituye); el GDD (§3, §4, §7.3, §21 y §23, que ahora separa
las preguntas de Oriol del trabajo que no es una pregunta); las cartas de reglas de mesa; D0012; en
el TABLERO, las filas del 17/09 y del 28/09 de su bandeja anotadas como respondidas y la tarea
CRITICO-DOS-SEIS-01 en «Pendiente». El motor juega todavía «todos 6»: no se ha tocado (otra zona y
otra tarea). 584 de las 935 habilidades de las clases no dicen con qué atributo tiran (contenido
pendiente, no pregunta).

## 9. PRESION-01: la «presión» de Bleach (idea de Oriol, 08/10, noche)

Oriol, fuera del tema: le encanta la «força espiritual» de Bleach (el reiatsu: alguien con tanta
presencia que es como si la gravedad aumentara de golpe) y quiere usarla cuando aparezca un
personaje muy, muy fuerte. «Estudia-ho.»

Hecho: [`MotorGraphico-main/MotorGraphico/planificacion/IDEA_PRESION_ESPIRITUAL.md`](../MotorGraphico-main/MotorGraphico/planificacion/IDEA_PRESION_ESPIRITUAL.md) —qué es en Bleach (con fuentes), quién la tendría en Onegai, el rasgo
«Presión N» para la mesa (Ed. 4) y cómo se vería en el juego— y una línea en el GDD §21 que apunta
allí. Es una idea, no una regla: no hay nada que preguntarle hasta que aparezca el personaje.
