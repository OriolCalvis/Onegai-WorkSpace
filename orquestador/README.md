# ORQUESTADOR — una persona coordina, varias IAs trabajan

Coordinador: **Claude** (proyecto Software-Onegai en claude.ai). Dueño: **Oriol**.
Creado el 14/10/2026.

## Cómo funciona
1. Cada agente tiene **su fichero** aquí (`codex.md`, `qwen.md`, …) con UNA tarea concreta.
2. Oriol abre el agente y pega:
   > Eres <AGENTE>. Lee `Software/orquestador/README.md` y después `Software/orquestador/<agente>.md`, y haz la tarea. Al terminar rellena la sección INFORME de tu fichero.
3. Cada noche Claude lee los informes, `TABLERO.md` y `sesiones/`, escribe **`REVISAR.md`**
   (qué tiene que mirar y probar Oriol) y pone la siguiente tarea en cada fichero.
   Los domingos deja además **`SEMANA.md`** con el resumen de la semana.

## PROTOCOLO — obligatorio para todos los agentes
Todo lo de `AGENTS.md` y la BITÁCORA sigue valiendo (fila en `TABLERO.md` antes de empezar, fichero en
`sesiones/`, `herramientas/git-safe.sh` para escribir en git). Además:

1. **Haz SOLO la tarea de tu fichero.** Si no se puede hacer o está bloqueada, no la cambies por otra:
   escribe en el INFORME por qué y para.
2. **Trabajo nuevo = rama propia, fuera de la carpeta sincronizada.** Desde la raíz del repo:
   ```
   git worktree add ~/onegai-ramas/<agente>-<TAREA> -b agente/<agente>/<TAREA>
   ```
   Trabaja, compila, prueba y commitea **allí** (con `git-safe.sh`). Nunca en la rama principal,
   nunca `push`, nunca `merge`. Oriol junta las ramas tras revisar.
3. **Si tu tarea continúa WIP que ya está en el árbol principal** (lo dice tu fichero), no lo muevas:
   commitea solo tus ficheros por pathspec con `git-safe.sh`, comprobando antes
   `git diff --cached --name-status` (si salen borrados que no son tuyos, es el índice minado: `reset`, no `commit`).
4. **No toques zonas de filas ajenas vivas** del TABLERO. Nada de `Application.*` salvo que tu fichero lo diga.
5. **El INFORME es obligatorio** y es lo que lee Oriol. Sin informe, la sesión no cuenta.
6. **SIGUIENTE (desde el 14/10).** Algunos ficheros traen, debajo de la tarea actual, una sección
   «SIGUIENTE». Solo se empieza si Oriol lo dice al pegar el mensaje («…y después la SIGUIENTE»),
   y con su propia fila en el TABLERO. Salen del **carril paralelo** del plan
   (`planificacion_alcance_y_vision/PLAN_ALCANCE_Y_VISION.md` §8): ficheros nuevos con su línea
   de CMake, o datos aguas arriba. Nunca tocan lo que lleva Claude (el combate, `GameSession`,
   `Application`, el HUD de combate, las aventuras).

7. **Las preguntas para Oriol (desde el 14/10) van a [`BANDEJA_PRODUCT_OWNER.md`](../BANDEJA_PRODUCT_OWNER.md)**,
   con su número PO-xx, por qué importa, tu recomendación y una línea «Tu respuesta:». No se pregunta lo
   que ya está en su §0 («Resuelto»). Si un punto está contestado, pásalo a su decisión y muévelo a §0.

## Agentes
| Agente | Fichero | Cómo se usa |
|---|---|---|
| Codex / ChatGPT | `codex.md` | app ChatGPT/Codex |
| opencode | `opencode.md` | app/terminal |
| Qwen Code | `qwen.md` | terminal (`qwen`) |
| Gemini CLI | `gemini.md` | terminal (`gemini`) |
| ZCode | `zcode.md` | app |
| Trae | `trae.md` | app |
| Claude Opus (2.ª sesión) | `claude_opus2.md` | Claude Code, modelo Opus, esfuerzo alto (desde el 14/10) |
| Claude Sonnet | `claude_sonnet.md` | Claude Code, modelo Sonnet, esfuerzo medio (desde el 14/10) |
| Claude Haiku | `claude_haiku.md` | Claude Code, modelo Haiku, esfuerzo medio; medidas y herramientas nuevas en `tools/` (desde el 14/10) |
| Pixel (PixelLab) | `pixel.md` | Oriol con PixelLab: sprites y animaciones (desde el 14/10) |
| Hayase | `hayase.md` | **pendiente**: Oriol tiene que decir qué es |
| StoryCraftr, AuthorAgent | — | herramientas dentro de `Onegai 2/`; funcionan con clave de API, sin plan propio. Pendiente de decidir uso |
