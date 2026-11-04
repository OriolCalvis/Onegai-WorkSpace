# Reglas de ECC en este workspace — prioridad

Las reglas de `rules/ecc/` (C++ y Python) son una selección del paquete ECC 2.2.3 instalada
el 2026-11-04 para este workspace. Son orientación de estilo y seguridad, no normas del proyecto.

Si una regla de ECC contradice `AGENTS.md`, `BITACORA_DEL_PROYECTO.md` o una decisión de `decisiones/`,
**manda la bitácora**. En particular:

- Commits en castellano con contexto en el cuerpo, con `herramientas/git-safe.sh`; no se usan prefijos `feat:`/`fix:`.
- `require()` y nunca `assert()` en demos y pruebas; sin tildes/ñ en `lines`/`speaker`/`text` de JSONs; `ensure_ascii=False` en generadores Python.
- Flujo en una sola dirección `canon → cartas → motor`; ninguna skill o agente de ECC autoriza cambios de lore ni encargos nuevos.
- No hay objetivo de cobertura del 80 %: la aceptación la fijan los criterios de cada encargo.

El paquete ECC completo sigue instalado para Codex (`codex plugin list`) y Qwen Code (`qwen extensions list`);
en Claude Code el plugin global está desactivado y solo se carga esta selección.
