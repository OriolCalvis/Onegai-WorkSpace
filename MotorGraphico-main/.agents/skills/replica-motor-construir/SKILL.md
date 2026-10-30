---
name: replica-motor-construir
description: Diseña en C++ y construye pieza a pieza en MotorGraphico las funciones de Unity de features.csv, en sala limpia, con contratos del Pla U y el ciclo de la fábrica. Úsala tras replica-motor-recon o cuando toque implementar un paquete del Pla U.
---

# replica-motor-construir

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

Es la adaptación al motor de `replica-architect` y `replica-build` (MIT, Jake Schincariol), juntas en
dos fases.

Lee:

- `planificacion/replica/recon.md` y `features.csv`;
- `planificacion/pla_unity/descomposicio/pla.json`;
- `AGENTS.md`.

Escribe:

- `planificacion/replica/arquitectura.md`;
- los encargos en `../../orquestador/encargos/`;
- la columna `clone` de `features.csv`;
- `planificacion/replica/build-log.md`.

Si no hay mapa de reconocimiento del sistema, para y ejecuta antes `replica-motor-recon`.

## Reglas

- **Sala limpia.** Cada línea se escribe aquí, a partir del recon y del Manual. Nunca se copia ni se
  adapta código de `UnityCsReference` ni nada descompilado. Si no sabes cómo se comporta algo, vuelve
  al Manual o al editor como usuario.
- **Nombres propios.** Las clases, los ficheros y los textos llevan nombres del motor, en castellano
  como el resto del código. La función de Unity solo se nombra en `notes` y en comentarios de
  referencia («equivale a Animation Events»).
- **El contrato de `AGENTS.md` manda sobre esta skill:**
  - la petición vigente y el encargo aplicable determinan la autorización; el ZIP no exige una aprobación nueva;
  - rama propia y nada de push ni merge;
  - máximo 500 líneas en src/include/editor; los excedidos solo adelgazan;
  - máximo 15 miembros propios por cabecera; estado en clases de datos;
  - sin mezclar GL/no-GL ni compilar fuentes de src sueltas dentro de objetivos;
  - cada include CMake y su archivo van juntos en el commit;
  - un archivo grande exige relevo reclamado antes de abrirlo: mover código y adelgazar el origen;
  - solo se cierra código alcanzado por juego, motor_editor o level_editor; demos y tests no bastan;
  - errores esperables con Result y RAII/ScopeGuard; logs de transiciones según AGENT_HANDOFF.md;
  - `require()`, nunca `assert()`;
  - lo que Oriol ha colocado no se «mejora».

## Fase A: arquitectura (una vez por sistema)

Escribe `arquitectura.md` con estos apartados:

1. **Dónde vive cada cosa**, según la separación de U2:
   - motor (genérico, sin saber nada de Onegai);
   - módulo RPG (`IGameModule`);
   - datos de Onegai (JSON).

   Si una pieza necesita saber algo de Onegai, no es del motor.
2. **Tipos y propiedad:**
   - quién crea y quién destruye;
   - reutiliza identidad y propiedad existentes; caracteriza estabilidad e invalidación antes de sustituir el almacenamiento; no crees un segundo pool;
   - composición antes que herencia (`Enemy` + `EnemyBrain` es el modelo).
3. **Formato de datos:**
   - el esquema JSON y el `schemaVersion`;
   - cómo se leen los ficheros de ayer (como hace G-07);
   - un ejemplo mínimo.
4. **La parte gl-free.** La lógica se separa del dibujo para que se pruebe con `ctest -L gl-free`.
   Lo que solo se puede ver con GL va a la prueba jugable.
5. **Las partes que muerden:**
   - el orden de actualización;
   - el determinismo (las tiradas);
   - deshacer en el editor;
   - la recarga en caliente;
   - el rendimiento por fotograma;
   - la compatibilidad de las partidas guardadas.
6. **Orden de construcción:**
   - primero una tajada vertical (el caso de uso de Onegai de punta a punta, aunque sea feo);
   - después los must, los should y los could;
   - cada hito con sus filas de `features.csv` y su paquete del Gantt.

   Si hace falta un paquete nuevo, se propone para el Pla U con duración y dependencias,
   marcado «propuesta». El Gantt lo cambia Oriol.

## Fase B: construir pieza a pieza

Por cada hito, usa el encargo aplicable de `../../orquestador/` y su plantilla; para unidades E consulta `descomposicio/PLANTILLA_E.md`. Se abre con el Resultado (qué
podrá hacer o ver Oriol en 10 minutos), la zona de ficheros, las decisiones y la prueba jugable. El
encargo PLAU-01 es un buen modelo.

Después, por cada encargo:

1. **Plan:**
   - lee el código real y rellena «Plan»;
   - si los nombres del encargo no coinciden con el código, dilo;
   - si el encargo aplicable exige una aprobación aún ausente, prepara el plan concreto y explica la fuente de esa condición; no vuelvas a pedir una ya concedida.
2. **Línea base:** `fabrica.py inicio <ENCARGO>` antes del primer cambio (skill `fabrica`, ruta indicada arriba).
3. **Construye** en pasos pequeños:
   - la prueba gl-free primero: falla, la arreglas y pasa;
   - `fabrica.py vigilar --max 20` o `check` mientras trabajas;
   - con un ROJO abierto no se sigue: `fabrica.py depurar <check>`.
4. **La matriz:**
   - pon las filas tocadas a `yes` solo con evidencia y aceptación aplicable, o a `partial` con una nota de lo que falta;
   - `partial` no es `yes`: no se esconde media función.
5. **Entrega:**
   - commit con `herramientas/git-safe.sh`;
   - después `fabrica.py entregar --encargo-md …`;
   - con ROJO no hay entrega;
   - rellena las tres líneas de la prueba jugable.
6. **El registro:** una línea en `build-log.md` con el encargo, la fecha, hecho o parcial, lo que
   falta y lo que costó más de lo previsto. Si algo era más grande de lo que parecía, dilo también
   en el chat.

## Hecho, por pieza

- [ ] el comportamiento del contrato del recon (paso 5), con su orden de llamadas y sus casos borde;
- [ ] pruebas gl-free nuevas y VERDE de `entregar` desde un clon limpio;
- [ ] los datos de ayer siguen cargando (niveles, catálogos y partidas);
- [ ] ningún código, nombre de clase ni texto copiado de Unity;
- [ ] `features.csv` y `build-log.md` al día;
- [ ] la prueba jugable escrita para Oriol (el «visto» es suyo).

## Salida

Lo construido, la matriz al día, la evidencia del encargo y un resumen:

- los must hechos del total;
- la cifra de `parity.py`;
- lo siguiente.

Después, `replica-motor-verificar`.
