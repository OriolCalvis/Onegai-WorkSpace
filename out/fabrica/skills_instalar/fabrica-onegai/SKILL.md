---
name: "fabrica-onegai"
description: "Ciclo de validadores automáticos, registro y entrega con evidencia para encargos de código o datos en Software/ (MotorGraphico): línea base, checks continuos, depurar y entregar desde clon limpio."
---

# Fábrica de Software/ (Onegai / MotorGraphico)

La herramienta vive en el repo: `.agents/skills/fabrica/fabrica.py`, con `fabrica.json` y `SKILL.md` al lado. Es la misma para todos los agentes. Lee ese `SKILL.md` antes de empezar: manda sobre este resumen.

## Cuándo

En todo encargo que cambie código o datos del repo: al reclamarlo, mientras trabajas y al cerrar.

## Cómo, desde esta sesión

- El repo está en el ordenador de Oriol. Ejecuta con la shell del ordenador enlazado (device_bash), desde la raíz de `Software/`. Si no hay carpeta conectada, pide acceso a la carpeta `Software`.
- Si solo tienes una copia en el contenedor, la fábrica funciona igual, pero la compilación y `ctest` necesitan su cmake. Dilo en la evidencia.

## El ciclo

1. `python3 .agents/skills/fabrica/fabrica.py inicio <ENCARGO>`, ANTES de tocar ningún fichero. Toma la línea base: lo que ya falla queda como heredado y no bloquea.
2. Mientras trabajas:
   - `fabrica.py vigilar --max 20` (siempre con `--max`) o `fabrica.py check` / `check --fase completa`.
   - No sigas escribiendo con un ROJO abierto.
3. Si sale ROJO: `fabrica.py depurar <check>`.
   - Lee el primer error de los fallos NUEVOS y crúzalo con tus cambios.
   - Si no es de tu zona de ficheros, no lo arregles: apúntalo en el encargo.
4. Al cerrar:
   - commit con `herramientas/git-safe.sh`;
   - después `fabrica.py entregar --encargo-md orquestador/encargos/<ENCARGO>.md`, que prueba desde un clon limpio, pega la evidencia en el encargo y genera el zip con logs y huellas sha256.
   - Con ROJO no se entrega.
   - Rellena a mano las tres líneas de la prueba jugable.
5. Pasa a Oriol el `EVIDENCIA.md` y el `entrega_<ENCARGO>.zip`, y la línea para `RESULTADOS.md` que imprime la fábrica (estado producido, nunca visto).

## Reglas

- Nunca push ni merge.
- La fábrica no sustituye el «visto» de Oriol.
- Si un fallo sale como «código N» en vez de con nombre, la regex de `fabrica.json` no casa con la salida real: ajústala ahí y dilo.