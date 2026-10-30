---
name: "replica-motor-verificar"
description: "Prueba lo construido de Unity en MotorGraphico (plan de casos por flujo, pruebas gl-free, lista para Oriol, bugs S1-S4) y mide la paridad con parity.py sobre features.csv, con el veredicto de si está listo. Úsala al cerrar un hito del Pla U o cuando pregunten cuánto falta para parecerse a Unity."
---

# replica-motor-verificar

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

Por cada flujo F01, F02… del recon:

- **Camino feliz:** los pasos y lo que se ve al final.
- **Bordes:** recorre esta lista cada vez.
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
  - la orden en Mac y en Windows.
- **Negativos:**
  - JSON roto (debe dar error con el nombre del fichero, no un cuelgue);
  - un campo que falta;
  - un tipo equivocado.

Numera los casos así: F01-H1 (camino feliz), F01-E3 (borde), F01-N2 (negativo).

## Paso 2: automatiza lo que se pueda

- **Pruebas gl-free** con `ctest -L gl-free`:
  - una por caso que no necesite ver nada;
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
- **Mejor que Unity para Onegai:** lo anterior y además menos pasos en los flujos del juego. Es el
  objetivo: no hace falta un Unity entero, hace falta el que necesita este juego.

Da las cifras sin adornar: un 34 % es un 34 %.

## Salida

En el chat:

- `paridad.md`;
- los bugs por severidad;
- los casos ejecutados, superados y fallados;
- las cinco cosas siguientes que construir, en el orden de la matriz.

Después, `replica-motor-construir` para los huecos.