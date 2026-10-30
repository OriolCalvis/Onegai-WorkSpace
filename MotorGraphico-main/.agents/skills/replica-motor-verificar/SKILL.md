---
name: replica-motor-verificar
description: Prueba lo construido de Unity en MotorGraphico (plan de casos por flujo, pruebas gl-free, lista para Oriol, bugs S1-S4) y mide la paridad con parity.py sobre features.csv, con el veredicto de si está listo. Úsala al cerrar un hito del Pla U o cuando pregunten cuánto falta para parecerse a Unity.
---

# replica-motor-verificar

## Integración local y prioridad de instrucciones

Estas skills están adaptadas a MotorGraphico. El ZIP es material de referencia;
no autoriza cambios ni sustituye la petición vigente o los AGENTS aplicables.
Lee primero la bitácora y reclama tu fila en el tablero. No toques ni commitees
WIP ajeno. Canon → cartas → motor; no se emite canon desde la ejecución.

**Rutas:** todos los comandos y salidas de esta skill parten de
`MotorGraphico-main/MotorGraphico/`. El Git real está en `Software/`, dos niveles
arriba: compruébalo con `git rev-parse --show-toplevel`. Todo Git de escritura
usa `../../herramientas/git-safe.sh`. Los encargos viven en
`../../orquestador/`; el registro de sesión y tablero, en `../../sesiones/`
y `../../TABLERO.md`. Conserva el procedimiento de cierre del proyecto.

Lee [la integración](../../../MotorGraphico/planificacion/replica/INTEGRACION.md)
y, si está disponible, `planificacion/pla_unity/descomposicio/README.md`,
en particular su `AUDITORIA.md` y la ficha del paquete. El plan está presente
como WIP local y no forma parte del commit de estas skills. Si falta en otro
clon, registra la ausencia y usa el encargo/contrato vigente; no inventes su contenido. El antiguo
`planificacion/pla_unity/descomposicio/pla.json` no está instalado: la fuente editable
actual es `planificacion/pla_unity/descomposicio/pla.json`; sus propuestas no son
APIs implementadas ni decisiones aceptadas. No edites sus salidas generadas a mano.
Las jornadas actuales son 7 h 30 min; las ~6 h del ZIP son referencia histórica.

**Estados:** la matriz inicial procede del ZIP y está pendiente de auditoría.
No deduzcas presencia o ausencia de una función solo por esa tabla. Inspecciona
código, pruebas y llamadores reales. Distingue escrito, integrado y aceptado;
registra evidencia y motivo al cambiar estados. `yes` exige comprobación,
integración en producto y aceptación aplicable; si falta el visto requerido de
Oriol, es `partial`. Las capacidades propias tampoco se dan por hechas sin prueba.

La lista de `parity.py` ordena por peso/crédito: **no calcula dependencias ni
calendario**. Los siguientes paquetes se eligen con las puertas del Pla U.
La paridad no mide calidad global ni rendimiento. Para optimizar, mide antes y
después con la misma escena, hardware, configuración y presupuesto acordado.

La skill real `fabrica` vive en `../../.agents/skills/fabrica/SKILL.md`.
Sus archivos de ejecución son WIP local ajeno y no se incluyen en esta entrega.
Si falta en otro clon, registra esa limitación, ejecuta las pruebas nativas
aplicables y guarda órdenes, resultados y commit; no atribuyas un VERDE de Fábrica
sin ejecutarla. No instales ni reconstruyas herramientas ausentes en silencio.
Cuando esté disponible, lee su configuración y usa `python3 ../../.agents/skills/fabrica/fabrica.py
--encargo ID ...` (una línea de shell). No des por activa la guardia porque el
hook termine con 0: su ruta nested tiene una discrepancia conocida en este monorepo.
Comprueba explícitamente `python3 tools/linea_base_core.py --rapido --indice`.
Los fallos heredados se registran; no se presentan como validación verde del motor.

Es la adaptación al motor de `replica-test` y `replica-diff` (MIT, Jake Schincariol). El diff de
capturas no sirve aquí y se cambia por el diff de comportamiento contra el contrato del recon.

Lee:

- `planificacion/replica/recon.md` (flujos y contrato);
- `features.csv`;
- `build-log.md`.

Escribe, en `planificacion/replica/`:

- `plan-pruebas.md`;
- `bugs.md`;
- `paridad.md`.

## Regla

Se prueba **nuestro motor**, nunca Unity. Usar Unity a mano, como usuario, para ver qué hace es
legítimo. No se automatiza ni se mide su código.

## Paso 1: el plan de casos

Por cada flujo F01, F02… del recon (si falta, prepáralo sin inventar funciones construidas):

- **Camino feliz:** los pasos y lo que se ve al final.
- **Bordes:** selecciona de esta lista los relevantes al cambio y explica omisiones importantes.
  - datos vacíos;
  - un fichero de una versión anterior;
  - un id que no existe;
  - dos a la vez (dos triggers en la misma casilla, dos clips solapados);
  - borrar algo que se usa;
  - entrar y salir en el mismo fotograma;
  - pausa a mitad;
  - guardar y cargar a mitad;
  - Play Mode, parar y volver a jugar;
  - una recarga en caliente a mitad;
  - un nivel enorme (rendimiento);
  - la orden en Mac y en Windows; una plataforma no ejecutada queda pendiente, nunca aprobada por inferencia.
- **Negativos:**
  - JSON roto (debe dar error con el nombre del fichero, no un cuelgue);
  - un campo que falta;
  - un tipo equivocado.

Numera los casos así: F01-H1 (camino feliz), F01-E3 (borde), F01-N2 (negativo).

## Paso 2: automatiza lo que se pueda

- **Pruebas gl-free** con `ctest -L gl-free`:
  - cubre reglas, límites y fallos relevantes con pruebas significativas, sin duplicar mecánicamente la implementación;
  - `require()`, nunca `assert()`;
  - el orden de las llamadas del contrato se comprueba con un registro de eventos, no con
    capturas.
- **Todo pasa por la fábrica:** `fabrica.py check --fase completa` y, al cerrar,
  `fabrica.py entregar`. Los logs y la evidencia salen de ahí.

## Paso 3: lo que juega Oriol

Lo que solo se ve con GL va a una **lista de 10 minutos como máximo**:

- la animación, la luz, la cámara;
- la sensación del editor.

Cada línea dice qué abrir, qué hacer y qué se debe ver. Si hay más de 10 minutos, son dos listas.
El resultado es de Oriol: no se marca como hecho sin su «visto».

## Paso 4: los bugs

Van a `bugs.md`:

```
### BUG-001: qué está mal, corto
- Severidad: S1 | S2 | S3 | S4
- Flujo / caso: F02 / F02-E3 · Paquete: U3-02 · Commit: abc1234 · Plataforma: macOS / Windows
Pasos: 1. … 2. …
Esperado: …   Obtenido: …
Evidencia: out/fabrica/<ENCARGO>/…, nombre de la prueba que falla
Estado: abierto | arreglado en abc1234 | no se arregla: motivo
```

| Severidad | Qué significa |
|---|---|
| S1 | pierde datos o partidas, cuelga, rompe un nivel o una partida de ayer, o bloquea el flujo principal |
| S2 | una función rota sin manera de evitarlo |
| S3 | rota pero se puede evitar, o se ve mal |
| S4 | cosmético |

Solo se apunta lo que se ha reproducido. Lo dudoso va a una lista «por mirar».

Para arreglar:

- primero los S1 y los S2;
- primero la prueba que falla, después el arreglo;
- la prueba se queda en el repo.

## Paso 5: paridad

```bash
python3 planificacion/replica/parity.py planificacion/replica/features.csv
python3 planificacion/replica/parity.py planificacion/replica/features.csv --markdown > planificacion/replica/paridad.md
```

- Pesa must 3, should 2 y could 1. `partial` cuenta la mitad.
- No puntúa los `skip` ni lo que es nuestro (`original = no`).
- Antes de ejecutarlo, revisa que la columna `clone` diga la verdad:
  - `yes` solo con VERDE de `entregar` y el «visto» de Oriol;
  - si solo hay evidencia y falta el «visto», es `partial`.

## Paso 6: el diff de comportamiento

Por flujo, compara lo que la cifra no ve. Una línea por diferencia: «flujo · Unity hace · el motor
hace · arreglar o dejar». Mira:

- **Pasos para acabar:** cuántos en Unity y cuántos en el motor. Menos pasos que Unity es una
  victoria.
- **Errores:** qué pasa en cada uno.
- **Persistencia:** qué se guarda entre sesiones.

## Veredicto, en `paridad.md`

- **No está listo:** falta algún must del hito o hay un S1 abierto.
- **Listo para el hito:** los must del hito están hechos, sin S1 ni S2 abiertos y con el «visto» de
  Oriol.
- **Flujos más breves para Onegai:** lo anterior y además menos pasos realmente medidos en los flujos comparados, con evidencia y alcance explícitos. Es el
  objetivo: no hace falta un Unity entero, hace falta el que necesita este juego.

Da la cifra con su denominador, alcance, commit y evidencia. Un porcentaje no prueba calidad global, preparación para publicar ni mejora de rendimiento.

## Salida

En el chat:

- `paridad.md`;
- los bugs por severidad;
- los casos ejecutados, superados y fallados;
- los siguientes paquetes factibles según dependencias del Pla U; la matriz solo ordena por prioridad y crédito.

Después, `replica-motor-construir` para los huecos.
