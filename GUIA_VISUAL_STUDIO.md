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


# GUIA_VISUAL_STUDIO.md — Comandos de Compilación y Pruebas

Basado en la estructura del proyecto MotorGraphico-main, aquí tienes los comandos necesarios para compilar y probar el motor gráfico. El proyecto usa **CMake** con generador Makefile (Linux) y compiladores clang++/g++.

## 1. Preparar el entorno de compilación

```bash
# Desde la raíz del repositorio MotorGraphico-main
cd /ruta/a/MotorGraphico-main/MotorGraphico

# Crear directorio de build y configurar CMake
mkdir -p build && cd build
cmake -S .. -B build -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Alternativa Release:
# cmake -S .. -B build -DCMAKE_BUILD_TYPE=Release
```

## 2. Compilar el motor

```bash
# Compilar todo el proyecto (modo Debug)
cmake --build build -j --config Debug

# Solo compilar el motor principal (librerías)
cmake --build build -j --target libmotor_core -c Debug

# Compilar solo los ejecutables de demo
cmake --build build -j --config Debug -- target demo_textured_quad demo_isometric_renderer
```

## 3. Compilar y correr tests unitarios

```bash
# Compilar todos los tests
cmake --build build -j --config Debug

# Correr tests gl-free (la mayoría, ~80%)
ctest --output-on-failure --test-dir build -L "gl-free"

# Correr tests específicos por nombre
./build/test_bitmap_font
./build/test_hud_minimap
./build/test_save_game
./build/test_game_session
./build/test_battle_state
./build/test_catalog_content
./build/test_condition_engine
./build/test_inventory_engine

# Corner todos los tests y ver resumen
ctest --output-on-failure --test-dir build 2>&1 | tail -20
```

## 4. Compilar con AddressSanitizer (para detectar memory leaks)

```bash
# Compilar con ASan
cmake -S .. -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"

cmake --build build-asan -j

# Correr tests con ASan
ASAN_OPTIONS=detect_leaks=0 xvfb-run -a ctest --test-dir build-asan --output-on-failure --no-tests=error -R "^test_(application_shutdown|post_processing.*)$"
ASAN_OPTIONS=detect_leaks=0 xvfb-run -a ./build-asan/demo_resource_manager
ASAN_OPTIONS=detect_leaks=0 xvfb-run -a ./build-asan/demo_camera
ASAN_OPTIONS=detect_leaks=0 xvfb-run -a ./build-asan/demo_tilemap
ASAN_OPTIONS=detect_leaks=0 xvfb-run -a ./build-asan/juego 3
ASAN_OPTIONS=detect_leaks=0 xvfb-run -a ./build-asan/demo_textured_quad 5
ASAN_OPTIONS=detect_leaks=0 xvfb-run -a ./build-asan/demo_isometric_renderer 5
```

## 5. Ejecutar demos sin OpenGL (GL-free, sin ventana)

```bash
# Demos GL-free que no necesitan ventana gráfica
./build/demo_resource_manager
./build/demo_camera
./build/demo_tilemap
./build/demo_campana_boundington
./build/demo_mundo
```

## 5. Ejecutar demos con OpenGL (requiere Xvfb)

```bash
# Para demos que abren ventana real (necesitan Xvfb)
xvfb-run -a ./build/demo_textured_quad 5
xvfb-run -a ./build/demo_isometric_renderer 5
xvfb-run -a ctest --test-dir build --output-on-failure --no-tests=error -R "^test_(application_shutdown|post_processing.*)$"
```

## 5. Flujo de trabajo típico

```bash
# 1. Configurar
mkdir -p build && cd build
cmake -S .. -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++

# 2. Compilar
cmake --build build -j

# 3. Probar (gl-free)
ctest --output-on-failure --test-dir build -L "gl-free" 2>&1 | grep -E "Passed|FAILED"

# 4. Probar todos
ctest --output-on-failure --test-dir build 2>&1 | tail -5

# 5. Limpiar y recompilar desde cero
cd /MotorGraphico-main/MotorGraphico
rm -rf build build-asan
```

## 6. Comandos útiles adicionales

```bash
# Generar compile_commands.json para clang-tidy
cmake -S .. -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Ver qué targets están disponibles
cmake --build build -- target_help

# Compilar solo un archivo fuente (usando compile_commands.json)
clang-tidy -p build include/RPG/RpgCatalogs.h

# Ver cambios pending en el repositorio
git status
git diff

# Correr el validador de contenido (ahora en CI)
python3 tools/validar_todo.py

# Verificar controles (test_verificar_controles.py + verificar_controles.py)
python3 tools/test_verificar_controles.py
python3 tools/verificar_controles.py
```

## 7. Estructura de carpetas importantes

```
/MotorGraphico-main/MotorGraphico/
├── build/              # Directorio de compilación (generado por CMake)
├── build-asan/         # Build con AddressSanitizer
├── src/                # Código fuente C++
├── include/            # Headers del motor
├── tests/              # Tests unitarios
├── tools/              # Herramientas de validación (validar_todo.py, etc.)
├── planificacion/      # Gantt charts, MAPA_DEL_CODIGO.md, seguimiento/
├── assets/             # Recursos y catálogos del juego
├── examples/           # Demos y ejemplos jugables
├── editor/             # Editor visual del motor
├── cmake/              # Scripts de CMake personalizados
└── .github/workflows/ci.yml  # CI configuration
```

## Notas importantes

- **Compiladores**: El proyecto funciona mejor con `clang++` en Linux. En macOS usa el compilador del sistema (clang). Windows requiere MSVC (experimental en CI).
- **Dependencias**: Se necesitan `libgl1-mesa-dev xorg-dev xvfb` para demos con GL en Linux (se instalan con `sudo apt-get install -y xorg-dev libgl1-mesa-dev xvfb`).
- **Tests gl-free**: ~81/88 tests pasan sin necesidad de ventana gráfica. Los ~7 failures restantes son effects de post-procesado que requieren GL.
- **Builds concurrentes**: Puedes tener `build/` y `build-asan/` simultáneos para comparar.
- **Archivo compile_commands.json**: Generado por CMake, usado por clang-format, clang-tidy, cppcheck.

---
*Estos comandos están diseñados para el entorno Linux (el principal de desarrollo). En macOS sustituye `clang++` por el compilador del sistema y omite la instalación de packages de X11. En Windows requiere MSVC y la configuración experimental del CI.*