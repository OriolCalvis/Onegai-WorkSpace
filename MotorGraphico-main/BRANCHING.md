# Ramas, versionado y CI/CD — Motor Gráfico Isométrico

## 1. Modelo de ramas

Git Flow simplificado, pensado para equipo pequeño/unipersonal.

- **`main`** — siempre estable. Solo recibe merges desde `develop` o `hotfix/*`. Sobre ella se etiquetan las versiones.
- **`develop`** — rama de integración. Los `feature/*` se fusionan aquí. Debe pasar el CI completo antes de cualquier merge.

| Tipo    | Nombre                   | Desde     | Hacia                              | Propósito                                                                        |
| ------- | ------------------------ | --------- | ----------------------------------- | --------------------------------------------------------------------------------- |
| Feature | `feature/<nombre-corto>` | `develop` | `develop`                           | Nueva funcionalidad (ej. `feature/resource-manager`, `feature/isometric-grid`)    |
| Bugfix  | `bugfix/<descripcion>`   | `develop` | `develop` (y `main` si es crítico)  | Corrección de errores en desarrollo                                               |
| Hotfix  | `hotfix/<version>`       | `main`    | `main` y `develop`                  | Arreglo urgente en producción, incrementa PARCHE                                  |
| Release | `release/<version>`      | `develop` | `main` y `develop`                  | Congelación de features, solo ajustes finales                                     |

**Ejemplo (Fase 1 del Gantt):** `feature/glfw-opengl-window` → PR a `develop` → `feature/textured-quad`, `feature/ortho-camera`, `feature/iso-grid-test` → `release/v0.1.0` → merge a `main` con tag `v0.1.0`.

Commits siguiendo el formato `<tipo>: <descripcion>` (`feat:`, `fix:`, `docs:`, `refactor:`, `chore:`...).

## 2. Versionado (SemVer) y etiquetas

```
v<MAYOR>.<MENOR>.<PARCHE>[-alpha.N|-beta.N|-rc.N]
```

- **MAYOR:** cambios incompatibles (reestructuración de la API, cambio de librería base).
- **MENOR:** nuevas funcionalidades que no rompen retrocompatibilidad.
- **PARCHE:** corrección de errores críticos.

Las etiquetas se anotan sobre commits de `main` (`git tag -a v0.1.0-alpha.1 -m "..."`). Al empujar un tag `v*` se dispara `release.yml` (ver sección 3).

Mapeo con las fases planificadas en `motor_grafico_gantt.puml`:

| Hito                                     | Tag              |
| ----------------------------------------- | ---------------- |
| Fin Fase 1 (grid isométrico funcional)   | `v0.1.0-alpha.1` |
| Fin Fase 2 (mapas y tiles)               | `v0.2.0-alpha.1` |
| Fin Fase 3 (entidades y animación)       | `v0.3.0-beta`    |
| Primera versión jugable                  | `v0.3.0`         |
| Fin Fase 4 (pulido visual)               | `v0.4.0`         |

Tags ligeros para hitos internos que no son releases públicos (no disparan `release.yml`): `dev-phase1-complete`, `integration/sprite-batching`.

## 3. CI/CD (`.github/workflows/`)

El código real del proyecto vive en el subdirectorio [`MotorGraphico/`](MotorGraphico/); ambos workflows operan con `working-directory: MotorGraphico`.

- **`ci.yml`** — en cada push/PR a `develop`/`main`, diariamente contra `develop` (cron) y manualmente (`workflow_dispatch`):
  - `build-test`: matriz Debug/Release × g++/clang++. Instala `xorg-dev libgl1-mesa-dev xvfb` (GLFW/motor_core los necesitan), compila con `CMAKE_EXPORT_COMPILE_COMMANDS=ON` y ejecuta `demo_resource_manager`, `demo_camera`, `demo_tilemap` (sin GL real, no necesita Xvfb), `demo_textured_quad` y `demo_isometric_renderer` (estos dos últimos bajo `xvfb-run` + `LIBGL_ALWAYS_SOFTWARE=1`: abren una ventana/contexto OpenGL real sobre una pantalla virtual, corren un número fijo de frames — ver `MotorGraphico/README.md` — y vuelcan el framebuffer). En Debug+g++ añade una pasada con ASan+UBSan (con `ASAN_OPTIONS=detect_leaks=0` solo para `demo_textured_quad`/`demo_isometric_renderer`: LeakSanitizer marca como fuga la cache interna de Mesa/llvmpipe, no código del proyecto).
  - `static-analysis`: `clang-format --dry-run --Werror` (bloqueante), `clang-tidy` **solo sobre los ficheros propios en `compile_commands.json`** (excluye `build/_deps/` y `third_party/`, dependencias de terceros que no queremos lintar), y `cppcheck` (sin `-I` a `third_party/`, por el mismo motivo: no es código nuestro).
- **`release.yml`** — al empujar un tag `v*`: compila Release en Linux y Windows, empaqueta (`motor-iso-<tag>-linux.tar.gz` / `...-win64.zip`) y publica el artefacto en la sección Releases de GitHub. Marca `prerelease` automáticamente si el tag contiene `-alpha`, `-beta` o `-rc`.

### Reglas de merge

- PR a `develop` o `main`: se ejecuta todo el pipeline; el merge queda bloqueado si falla `build-test` o `static-analysis`.
- Push directo a `develop` (permitido en equipo pequeño): mismo pipeline, notificación inmediata si falla.

## 4. Herramientas de estilo y análisis estático

- **`.clang-format`** (raíz) — estilo del proyecto (basado en Google, indentación de 4 espacios, línea de 100 columnas). `clang-format -i` para reformatear.
- **`.clang-tidy`** (raíz) — `bugprone-*`, `performance-*`, `modernize-*`, `readability-*`, con las reglas más ruidosas para este estilo de código desactivadas (`nodiscard` masivo, `magic-numbers`, etc.). No bloquea el pipeline por sí solo (se ejecuta sin `--warnings-as-errors`); sirve como señal, no como bureaucracia.
- **`cppcheck`** — sí bloquea el pipeline (`--error-exitcode=1`). Los falsos positivos puntuales se documentan inline con `// cppcheck-suppress <check>` y un comentario explicando por qué es seguro (ver `MotorGraphico/include/Core/Resources/ResourceManager.h`).

## 5. Pre-commit local (opcional, recomendado)

`.pre-commit-config.yaml` (raíz) instala:

- `clang-format` automático sobre los ficheros modificados de `MotorGraphico/{include,src,examples}`.
- Higiene básica (espacios en blanco al final de línea, fin de fichero, conflictos de merge sin resolver).
- Un build rápido (`Debug`, solo el target `demo_resource_manager`) como hook de `pre-push`, para detectar errores de compilación antes de llegar al CI.

Instalación:

```bash
pip install pre-commit
pre-commit install --hook-type pre-commit --hook-type pre-push
```

## 6. GLAD y stb_image

`MotorGraphico/third_party/{glad,stb}` están vendorizados en el repositorio (no son un paso manual de cada dev): en cuanto `CMakeLists.txt` los detecta, activa el target `motor_core` (Texture/Shader/Window/SpriteBatch) automáticamente. Se generaron una vez con `pip install glad && python -m glad --profile core --api "gl=3.3" --generator c --out-path third_party/glad` y descargando `stb_image.h`; ver `MotorGraphico/README.md` para el detalle si hay que regenerarlos.

`release.yml` todavía necesita actualizarse para empaquetar el binario real de la aplicación (hoy solo empaqueta `demo_resource_manager`) en cuanto exista un ejecutable jugable, más allá de las demos de verificación.
