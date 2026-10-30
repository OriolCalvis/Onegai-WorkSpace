# E-L0 — Registro de reanudación de la fábrica (agente: Claude; antes DeepSeek v4.1 con su harness)

Lote L0 de FRONT-01 (el encargo completo está en el proyecto de claude.ai, `claude/encargo_fabrica_fronteras_29-10.md`; aquí va lo necesario). Es **la única E de esta tanda que cambia código.** Es larga: el relevo es imprescindible.

## Dónde
- Fichero: `.agents/skills/fabrica/fabrica.py`, en la raíz de `Software/`. Solo usa la biblioteca estándar de Python y tiene que seguir así.
- Trabajo en una rama propia, fuera de la carpeta de iCloud. El worktree se crea **una sola vez**: si el relevo dice que ya existe, se reutiliza.
  ```
  git -c core.checkStat=minimal worktree add ~/onegai-ramas/claude-L0 -b agente/claude/FRONT-L0 claude/unificacion-28-10
  ```
- Commits con `herramientas/git-safe.sh`. Nada de push ni merge.
- Antes de editar, **no leas `fabrica.py` entero**. Primero: `grep -n "^def \|^    def \|argparse\|add_parser" .agents/skills/fabrica/fabrica.py | head -60`. Después, lee solo las funciones que vas a tocar.

## Qué hay que construir
1. **Estado por E:** `out/fabrica/<ENCARGO>/estado_E/<E>.json`. Se escribe de forma atómica (fichero temporal + `os.replace`). Guarda:
   - `e`, `reservada_por` (el agente, el PID y la hora), `commit_base` y `rama`;
   - `pasos`: `implementar`, `validar`, `corregir_N`, `commit` y `evidencia`, cada uno con su estado (`pendiente`, `hecho` o `fallido`) y lo que ha producido;
   - `diff_validado`: el hash del diff respecto a `commit_base`;
   - `bloqueada_por`: opcional.
2. **Reserva atómica:** crear `out/fabrica/<ENCARGO>/reservas/<E>.lock` con `os.open(..., O_CREAT|O_EXCL)`. Guarda el agente, el PID y la hora, y un latido que se actualiza en cada paso.
   - Si el lock ya existe, la E no se toca.
   - Un lock caducado no se roba: se apunta como «reserva caducada» y se libera en el siguiente arranque, solo si el PID ya no existe.
3. **Reanudar con git como fuente de verdad:**
   - Cada commit de una E lleva el trailer `Fabrica-E: <E>`.
   - Al reanudar, se busca un commit con ese trailer en `commit_base..HEAD`. Si existe y el estado dice `commit: pendiente`, el paso se marca como `hecho` con su hash y **no se vuelve a hacer el commit**.
   - `validar` se salta si el hash del diff actual es igual a `diff_validado`.
4. **Subórdenes nuevas** en `fabrica.py`:
   ```
   reservar <E> · paso <E> <paso> <estado> [--producto X] · reanudar <E> · liberar <E>
   ```

## Pruebas: `tests/tools/test_fabrica_reanudar.py` (unittest, sin red, en un repo git temporal)
1. Se mata el proceso entre `validar` y `commit` → al reanudar hay un solo commit y no se vuelve a validar.
2. Se mata **después del commit y antes de escribir el estado** → al reanudar, el commit se reconoce por el trailer y no se crea un segundo.
3. Dos procesos reservan la misma E a la vez (con `multiprocessing`) → solo uno lo consigue.
4. Hay un lock caducado de un PID que no existe → se libera en el arranque. Si el PID existe, no se libera.

## Pasos (actualiza el relevo después de cada uno)
1. Crear el worktree y localizar en `fabrica.py` dónde va cada cosa (con grep). Anota en el relevo las funciones y las líneas.
2. Estado atómico, con su prueba.
3. Reserva y lock, con las pruebas 3 y 4.
4. Trailer y reanudación, con las pruebas 1 y 2.
5. Subórdenes y una línea en la skill (`SKILL.md` de la fábrica).
6. `python3 -m unittest tests/tools/test_fabrica_reanudar.py -v`, entero en verde. Commit.

Si un paso es demasiado largo para una sesión, pártelo en el relevo («4a hecho, 4b pendiente»). El commit de cada paso terminado se apunta en el relevo.

## Resultado
- La rama `agente/claude/FRONT-L0` con un commit por paso.
- La salida de unittest (solo el resumen final) en el relevo.
- En «Dudas para Oriol», cualquier decisión de diseño que hayas tenido que tomar.
