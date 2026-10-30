# Sesión 2026-10-15 — claude oficios_datos

- **Carril:** `oficios_datos` (lo lanza e integra Claude Cowork). Decisión: D0017.
- **Hecho:** catálogo `oficios.json` (15 oficios), tabla de pesca `oficios_tablas.json`, validador y pruebas, propuestas de fichas
  (peces y sitios que faltan), informe `planificacion_alcance_y_vision/OFICIOS.md`.
- **Paquete:** `MotorGraphico/out/claude_oficios_datos01/` (`aplicar.sh`, `nuevos.txt`, `oficios_datos01.tgz`, `propuesta/`).
- **Probado (en el contenedor, sobre una copia con lo traído del Mac):** aplicar.sh aplica 4 ficheros nuevos y se niega la segunda vez;
  validador 0 errores y 2 avisos (sitios que faltan); 29 pruebas en verde. No se ha compilado nada ni tocado la shell del Mac.
- **No tocado:** niveles, catálogos de objetos, `ObjectCatalog.h`, `Actuacion.*`.
- **Pendiente para Claude Cowork:** aplicar con `bash`; decidir las dudas de OFICIOS.md; el carril del motor lee `oficios.json`.
