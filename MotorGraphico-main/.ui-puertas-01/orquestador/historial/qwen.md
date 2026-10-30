# Historial de qwen

## Hasta el 15/10 (pasada a historial al poner la cola del 15/10)

# qwen — tarea actual
**Asignada:** 14/10 por Claude · **Repo:** `MotorGraphico-main` · **Tipo:** continúa WIP propio (regla 3)

## H6-01 — AudioDirector (mitad GL-free) — terminarla
Tu fila viva desde el 05/10. `AudioDirector.h/.cpp` ya existen en el árbol. Zona exacta en la fila: solo tus rutas nuevas + la fuente en `motor_level` y el `include()` de `CMakeLists.txt`. **No toques** `Audio.h/.cpp`, `Application.*`, `OptionsState.*`, `SaveSlotState`, `DiarioMisiones.*`.

**Hecho cuando:** `test_audio_director` verde; música exclusiva por `GameMode` sin reiniciar la que suena; efectos que no la cortan; puente volúmenes 0..10 de `OptionsValues` → 0..1 por bus; un asset que falta va a `problems()` y no rompe. Commit aislado por pathspec. Fila a «Hecho».

---
## SIGUIENTE (solo si Oriol te lo dice al pegar el mensaje) — AV14-04 + AV14-06 — El banco de sonidos y la música de combate
Plan: `planificacion_alcance_y_vision/` (AV14, carril paralelo) y `COMBATE_BG3.md` §11. Va después de tu AudioDirector.
- **Zona (solo esto):** NUEVOS `assets/audio/combate/`, `assets/audio/ui/`, `assets/audio/pasos/`, `assets/audio/musica/`, `assets/audio/sonidos.json` (id → fichero, bus, volumen) y `assets/audio/LICENCIAS.md`.
- **Qué:** 3 variantes de cada uno, en ogg, normalizados:
  - golpe en carne, cuero y metal;
  - fallo (silbido), roce, crítico;
  - dados rodando, carta jugada;
  - botón encima/pulsado (papel);
  - pasos en piedra, madera, tierra y hierba;
  - caer al suelo.

  Y un bucle de música de combate de energía constante, sin solista (Elemechs, min 16:18), con golpe de entrada y cierre. Solo CC0 o con licencia que permita usarlo en el juego, apuntada fichero a fichero.
- **Hecho cuando:** está todo con su licencia y su id; nada sin licencia clara. Fila en el TABLERO.

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:** `agente/...` · hashes
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
