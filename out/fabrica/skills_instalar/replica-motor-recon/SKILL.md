---
name: "replica-motor-recon"
description: "Mapa de reconocimiento de lo que hace Unity (sistemas, API pública, flujos del editor, formatos de datos) para rehacerlo en MotorGraphico en C++, en sala limpia. Escribe planificacion/replica/recon.md y features.csv. Úsala al decidir qué copiar de Unity o al añadir una función nueva a la matriz."
---

# replica-motor-recon

Es la adaptación al motor de `replica-recon` (MIT, Jake Schincariol). La original mira pantallas de
apps web; esta mira **sistemas, API pública y flujos del editor** de Unity, y el destino es
MotorGraphico (C++ con datos JSON).

Todo lo demás se construye a partir de lo que escribe esta skill:

```
planificacion/replica/recon.md       el mapa
planificacion/replica/features.csv   la matriz: feature, area, priority, original, clone, notes
```

Si `features.csv` ya existe (sale del Pla U), **se amplía, no se rehace**. Cada fila lleva en `notes`
el paquete del Gantt (`dades_unity.py`) que la construye.

## Sala limpia: las reglas antes que nada

Se rehace **lo que hace** Unity, nunca **lo que es suyo**.

- **Fuentes permitidas** (cada una con su URL en la tabla de fuentes):
  - el Unity Manual y la Scripting API públicos;
  - Unity Learn;
  - las notas de versión;
  - charlas y vídeos públicos;
  - el uso del editor como usuario, con la licencia propia de Oriol.
- **Prohibido:**
  - leer o copiar `UnityCsReference`. Unity lo publica con licencia de solo consulta: no se puede
    reutilizar ni adaptar. Ni siquiera se abre para «ver cómo lo hacen»;
  - descompilar las DLL del editor o del runtime;
  - copiar shaders, assets, iconos o textos de Unity.
- **Nombres:** se puede citar el nombre de una función de Unity como referencia en `features.csv`,
  por ejemplo «(Animator Controller)». Las clases y los ficheros del motor llevan nombres propios.
- **Si la documentación no explica un comportamiento,** se prueba en el editor como usuario y se
  describe con palabras propias. No se busca en el código.

## Paso 1: alcance

Propón respuestas a estas tres preguntas y que Oriol diga sí:

1. **Qué sistema.** «Todo Unity» no es un proyecto. «Timeline: pistas, clips y editor» sí lo es.
2. **Para qué en Onegai.** El caso de uso concreto del juego. Por ejemplo: «la entrada de Gorran
   con tres actores a la vez».
3. **Qué no.** 3D, red, plataformas: se marcan `skip` con el motivo.

## Paso 2: fuentes

Una tabla con una URL en cada fila. Por orden de valor:

- Manual (el concepto y los ajustes);
- Scripting API (el contrato: qué recibe y qué devuelve cada cosa);
- notas de versión (qué importó añadir);
- Unity Learn (el flujo real, paso a paso);
- el editor como usuario.

## Paso 3: inventario de sistemas

Una fila por sistema, con ids estables SIS-01, SIS-02…:

`ID | sistema | piezas públicas (componentes, assets, ventanas) | qué resuelve | estados y modos`

Los modos importan tanto como los estados vacíos en una app:

- edición frente a Play Mode;
- un asset frente a su instancia;
- un prefab frente a su override;
- pausa;
- un error de datos.

## Paso 4: flujos del editor y del juego

Ids F01, F02… Cada flujo es un objetivo con sus pasos, como lo ve el usuario:

```
F03 Crear una cinemática con dos actores
    abrir Timeline -> pista de animación A -> pista B -> clip de cámara -> Play
    pasos en Unity: 9
    borde: clips solapados, actor borrado, saltar a mitad
```

El número de pasos en Unity es **la cifra a batir** en el editor propio.

## Paso 5: contrato de la API pública

Por sistema, lo que un usuario programa o configura:

- mensajes y callbacks (`OnTriggerEnter`/`Stay`/`Exit`), cuándo se llaman y en qué orden;
- parámetros y valores por defecto.

Descríbelo con palabras propias. Esto es lo que después mira `replica-motor-verificar`.

## Paso 6: modelo de datos inferido

Assets y formatos: qué guarda cada asset, cómo se referencia (GUID, ruta…) y qué pasa al
renombrarlo. Cada uno con su evidencia (URL) y su confianza: alta, media o «suposición». Se traduce
a JSON del motor en la fase de arquitectura, nunca a YAML de Unity.

## Paso 7: la matriz

Añade o actualiza filas en `features.csv`:

- `priority`: must, should o could. Must es lo que necesita el juego de Oriol, no lo que Unity
  considera central.
- `original`: `yes` si Unity lo tiene. `no` si es nuestro, por ejemplo las páginas de evento al
  estilo RPG Maker: entonces no puntúa y sale aparte.
- `clone`: `no` al principio.
- `notes`: el paquete del Gantt, o «sin paquete: propuesta».

Lo que no se puede o no se debe rehacer va con `skip` y el motivo:

- la Asset Store;
- servicios en la nube;
- SDK de consolas;
- render 3D en un motor isométrico 2D;
- el runtime de C#.

## Paso 8: tamaño

Sistemas, flujos y piezas difíciles (serialización, recarga en caliente, deshacer en el editor,
determinismo). Expresa el tamaño en jornadas del Gantt (1 jornada = unas 6 h de agente). Si no cabe
en una fase del Pla U, propón paquetes nuevos para `dades_unity.py`, marcados como «propuesta».

## Salida

- `recon.md` y `features.csv` actualizados.
- `python3 planificacion/replica/parity.py planificacion/replica/features.csv` para la cifra de
  partida.
- Cinco líneas en el chat: el sistema, los must nuevos, las tres partes más difíciles, lo que queda
  fuera, y el siguiente paso, `replica-motor-construir`.