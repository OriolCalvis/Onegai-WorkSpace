---
name: "replica-motor-construir"
description: "Diseña en C++ y construye pieza a pieza en MotorGraphico las funciones de Unity de features.csv, en sala limpia, con encargos ODD, aprobación de C++ de Oriol y el ciclo de la fábrica. Úsala tras replica-motor-recon o cuando toque implementar un paquete del Pla U."
---

# replica-motor-construir

Es la adaptación al motor de `replica-architect` y `replica-build` (MIT, Jake Schincariol), juntas en
dos fases.

Lee:

- `planificacion/replica/recon.md` y `features.csv`;
- `planificacion/pla_unity/dades_unity.py`;
- `AGENTS.md`.

Escribe:

- `planificacion/replica/arquitectura.md`;
- los encargos en `orquestador/encargos/`;
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
  - nada de C++ sin aprobación de Oriol escrita en el encargo;
  - rama propia y nada de push ni merge;
  - ningún fichero de más de 600 líneas;
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
   - handles (`HandlePool`), nunca punteros guardados entre fotogramas;
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

   Si hace falta un paquete nuevo, se propone para `dades_unity.py` con duración y dependencias,
   marcado «propuesta». El Gantt lo cambia Oriol.

## Fase B: construir pieza a pieza

Por cada hito, un **encargo** con `orquestador/PLANTILLA_ENCARGO.md`. Se abre con el Resultado (qué
podrá hacer o ver Oriol en 10 minutos), la zona de ficheros, las decisiones y la prueba jugable. El
encargo PLAU-01 es un buen modelo.

Después, por cada encargo:

1. **Plan:**
   - lee el código real y rellena «Plan»;
   - si los nombres del encargo no coinciden con el código, dilo;
   - **para hasta que Oriol lo apruebe.**
2. **Línea base:** `fabrica.py inicio <ENCARGO>` antes del primer cambio (skill `fabrica-onegai`).
3. **Construye** en pasos pequeños:
   - la prueba gl-free primero: falla, la arreglas y pasa;
   - `fabrica.py vigilar --max 20` o `check` mientras trabajas;
   - con un ROJO abierto no se sigue: `fabrica.py depurar <check>`.
4. **La matriz:**
   - pon las filas tocadas a `yes`, o a `partial` con una nota de lo que falta;
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