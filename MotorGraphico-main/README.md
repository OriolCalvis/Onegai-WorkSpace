# MotorGraphico

Motor gráfico 2D isométrico, estética pixel art, pensado como base
reutilizable para juegos roguelike en C++17 / OpenGL 3.3.

- Código, CMake y documentación técnica del motor: [`MotorGraphico/`](MotorGraphico/README.md)
- Modelo de ramas, versionado (SemVer) y CI/CD: [`BRANCHING.md`](BRANCHING.md)
- Diagramas de diseño (clases, flujo, Gantt del motor y Gantt del RPG) y análisis DAFO: junto al código en [`MotorGraphico/`](MotorGraphico/)

## CI

Los workflows de GitHub Actions viven en [`.github/workflows/`](.github/workflows/):

- `ci.yml` — build (Debug/Release × g++/clang++), smoke test, sanitizers, formato (`clang-format`) y análisis estático (`clang-tidy`, `cppcheck`) en cada push/PR a `develop`/`main`.
- `release.yml` — empaqueta y publica un release en GitHub al crear un tag `v*`.

Ver [`BRANCHING.md`](BRANCHING.md) para el detalle completo.
