# 2026-10-15 · claude (por encargo de Oriol, cubriendo a Z.AI) — AV18-08 recompensas y alternativas de escenas

**Tarea:** la pieza «Recompensas de escenas (Z.AI)» pendiente de `ECONOMIA.md` §7. Z.AI se quedó sin créditos del día; Oriol pidió terminarla. No había ficha: el alcance lo he fijado desde `ECONOMIA.md`.

**Hecho (solo JSON de aventuras + herramientas Python + documentos; sin C++, sin catálogos):**
- 3 aventuras de Boundington: 34 beats/opciones con premio narrativo + 1 beat nuevo (`rec_fuego_regalo`).
- `tools/recompensas_escenas.json` (libro de 61 premios), `tools/auditar_recompensas_escenas.py`, `tools/test_auditar_recompensas_escenas.py`; enganchado en `validar_todo.py`.
- `FORMATO_AVENTURAS.md` (apartado nuevo) y `planificacion_alcance_y_vision/RECOMPENSAS_ESCENAS.md`.

**Resultado:** misiones D1 259/280, D2 498/480, D3 421/400 (±15 %). `validar_todo.py` OK, 0 errores (22 avisos, los de antes). `medir_objetivos.py` 31/31. Prueba nueva 4/4.

**No comprobado:** `recorrer_recompensas` (C++) — la VM no tiene cmake. Prueba nueva sin registrar en CMake.

**Avisos:** ver RECOMPENSAS_ESCENAS.md (Ocaso paga tarde; Furia de Alaro vendible por 400; `rewardGold` de quests a 0).
