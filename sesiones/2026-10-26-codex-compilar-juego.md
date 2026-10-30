# Compilación del juego integrado — 26/10/2026

Autor: Codex. Encargo de Oriol: compilar el juego para revisar las mejoras integradas.

- Leídos protocolo, histórico, propiedad y trampas de la bitácora, tablero y handoff.
- Repo real: Software; HEAD inicial bfc5a22. Cambios ajenos preexistentes en fuentes de aura, Application, ShaderManager, pruebas, documentación, tablero y sesiones: preservados sin incluirlos en el commit.
- Hooks configurados mediante git-safe: MotorGraphico-main/MotorGraphico/tools/hooks (relativo a Software).
- cmake --preset macos-release y cmake --build --preset macos-release --target juego -j4: éxito.
- Binario: MotorGraphico-main/MotorGraphico/out/build/macos-release/juego; recursos actualizados por copiar_assets.
- Avisos: captura de structured binding como extensión C++20 en AcompanantesAventura.cpp:41; bibliotecas duplicadas en el enlace.
- ./juego 3 y ./juego --nivel assets/levels/ciudad_centro.json 3: salida 0, captura 1280x720 y cierre normal.
- La primera prueba gráfica en sandbox quedó bloqueada por Cocoa; terminada y repetida correctamente con acceso nativo.
- Otra compilación preexistente en build detectada, sin intervenir. No se ha ejecutado la suite completa ni se certifican todas las mecánicas.
- TABLERO ya contenía cambios ajenos: se añade únicamente la fila propia y se deja fuera del commit para preservarlos.
