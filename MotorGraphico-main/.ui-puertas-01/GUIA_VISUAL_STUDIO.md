# Trabajo visual de Onegai

## Un solo punto de entrada

Abre Onegai-WorkSpace.code-workspace con Visual Studio Code. El explorador
muestra las cinco capas del proyecto y conserva la jerarquía canon, cartas,
motor y puente.

Al abrirlo, acepta las extensiones recomendadas. Las importantes son CMake
Tools y C/C++ Extension Pack para el motor; Extension Pack for Java y Spring
Boot para las cartas; Python para validadores y generadores.

## Motor gráfico: CMake

En macOS, selecciona el preset macos-debug desde la barra de estado de CMake
Tools. Después puedes usar los botones Configure, Build, Run y Debug de la
extensión o Terminal > Run Task.

Las tareas más habituales son:

- CMake · Motor: compilar todo (Debug)
- CMake · Motor: ejecutar juego (Debug)
- CMake · Motor: compilar un target...
- CMake · Motor: compilar todo (Release)

El último comando pide el nombre de cualquier ejecutable declarado por CMake,
por ejemplo juego, demo_mundo, demo_campana_boundington, demo_este_sur o
demo_senal_ceniza. Los binarios nuevos quedan fuera del código en
MotorGraphico-main/MotorGraphico/out/build/.

F5 inicia Motor · juego (macOS Debug). En Windows, abre directamente
MotorGraphico-main/MotorGraphico/CMakeLists.txt con Visual Studio 2022 y
selecciona uno de los presets windows-vs2022. Visual Studio detectará el mismo
CMakePresets.json y mostrará todos los targets CMake.

## Cartas: Maven y Spring Boot

El estado actual del proyecto de cartas declara Java 25. Selecciona JDK 25 en
el selector de Java de VS Code antes de importar Maven; no fijamos una ruta
para que el workspace funcione en otros equipos.

En Terminal > Run Task están:

- Maven · Cartas: compilar
- Maven · Cartas: test limpio (Java 25)
- Maven · Cartas: JAR (sin repackage)
- Maven · Cartas: iniciar Spring Boot

Se usa siempre ./mvnw, por lo que no hace falta instalar Maven globalmente.
La tarea de test limpia target antes de ejecutar y activa el modo experimental
de Byte Buddy para Java 25.

En el estado actual hay cuatro tests que siguen fallando por contenido y
dependencias ya existentes: Spring Boot no procesa todavía class files Java 25
y tres pruebas de combate buscan datos que han cambiado. Por eso el JAR sin
repackage es el artefacto que se puede generar hoy; el test limpio queda como
diagnóstico visible. El paquete ejecutable Spring Boot requerirá actualizar
Spring Boot o volver el proyecto a una versión Java compatible.
Visual Studio 2022 no es un IDE Java/Maven completo: para esa parte usa VS Code
con las extensiones recomendadas.

## Atajos

- Cmd+Shift+B: compila el motor en Debug.
- F5: compila y depura juego en macOS.
- Terminal > Run Task: abre el menú de todas las acciones.

Si CMake no enseña los presets, ejecuta Developer: Reload Window y vuelve a
elegir macos-debug. Si Maven detecta el JDK incorrecto, usa Java: Configure
Java Runtime y marca el JDK 25 como predeterminado.
