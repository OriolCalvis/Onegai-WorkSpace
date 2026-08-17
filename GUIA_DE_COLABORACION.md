# Cómo colaborar con Onegai-WorkSpace

Esta guía sirve cuando se trabaja con una IA o colaborador que no tiene acceso
directo al workspace. Onegai se divide en canon, cartas, motor y puente; por
eso compartir la pieza correcta evita perder tiempo y evita que una capa
contradiga a otra.

La dirección de los datos siempre es:

    Onegai 2 (canon) -> dndWeebCC (cartas y mapa) -> MotorGraphico (ejecución)

Antes de cambiar contenido compartido, hay que leer AGENTS.md y
BITACORA_DEL_PROYECTO.md. La bitácora es el punto común entre agentes.

## Opción 1: subir archivos

La forma más cómoda es arrastrar al chat un archivo o un ZIP pequeño de la
parte que se quiere revisar. Así el agente puede analizarlo y proponer cambios
concretos.

Buenos candidatos:

- Un JSON de localizaciones, niveles o puertas.
- Un script de Onegai-Core.
- Una aventura o catálogo de dndWeebCC.
- Un CMakeLists.txt, preset CMake o error de compilación del motor.
- Un fragmento de canon de Onegai 2 que deba llegar al mapa o al juego.

No hace falta subir builds, node_modules, target, out ni directorios .git.

## Opción 2: flujo modular de copiar y pegar

Describe primero el objetivo y pega después el código o el error relevante.
Por ejemplo:

    Necesito comprobar que las 191 puertas del motor coincidan con el mapa.

El agente debe devolver una propuesta acotada: qué archivo cambia, qué capa es
dueña del dato, el código corregido y el comando para verificarlo. Si falta
información canónica, no debe inventarla: se registra como pendiente.

## Opción 3: auditoría de estructura

Comparte la estructura del proyecto para que el agente pueda orientarse sin
clonar el repositorio:

    tree -I 'node_modules|.git|dist|build|out|target' -L 3

Con esa salida se puede decidir dónde vive cada script, JSON, nivel o documento
y detectar si una herramienta está escribiendo contra la capa equivocada.

## Opción 4: pair programming asíncrono

Formula una tarea concreta con su contexto. Ejemplos:

- La IA generó localizaciones del Este Sur y necesito el JSON que carga C++.
- Hay coordenadas que no coinciden entre Onegai 2 y dndWeebCC.
- Un nivel nuevo del motor no enlaza correctamente con el mapamundi.
- Maven, CMake o una demo fallan y tengo la salida de consola.

El agente puede responder con un plan, parches por archivo, comandos de
validación y una nota para la bitácora. Así varias IAs pueden avanzar sin
sobrescribir el trabajo de otra.

## Para empezar

Comparte uno de estos elementos:

1. El árbol de directorios.
2. El primer archivo, JSON, script o error que quieras revisar.
3. Un objetivo concreto del Este Sur o de cualquier otra región.

Con eso se puede entrar en modo depuración y seguir expandiendo Egaroth de
forma compatible con el canon, el mapa y el motor.
