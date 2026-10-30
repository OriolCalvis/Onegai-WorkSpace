# Relevo E-L0
Agente y sesión: Claude (Opus, sesión de claude.ai enlazada al Mac) · 29-10 16:50 · sesión nº 1
Estado: hecho
Pasos: 1 hecho · 2 hecho · 3 hecho · 4 hecho · 5 hecho · 6 hecho (commit 8c5263b)

## Resultado
- **Rama `agente/claude/FRONT-L0` @ 8c5263b**, sobre `claude/unificacion-28-10` (9d52ae8). Un solo commit con el trailer `Fabrica-E: E-L0`. Creado con objetos git y un índice temporal: **la copia de trabajo de Oriol (en `agente/qwen/integracion-av19-28-10`) no se ha tocado**. Sin push ni merge.
- Ficheros:
  - `.agents/skills/fabrica/fabrica.py` (1.1 → 1.2): sección de reanudación y subórdenes `reservar`, `paso`, `latido`, `reanudar`, `liberar` y `trailer`. Del código anterior solo cambian la versión y la tabla de órdenes;
  - `test_fabrica_reanudar.py` (nuevo);
  - `SKILL.md`: apartado «Reanudar una E».
- Pruebas: `python3 .agents/skills/fabrica/test_fabrica_reanudar.py -v` → **20 de 20 OK**, también desde una extracción limpia de la rama. Mutaciones: **13 de 13 detectadas**.

## Revisión independiente (un subagente que no vio cómo se escribió)
Encontró, y reprodujo, 3 problemas altos y 5 medios. Todos están corregidos y cada uno tiene su prueba:
1. **Alta:** con la CLI cada orden es un proceso nuevo, así que el latido no funcionaba y se podía robar una E. → La reserva es del **agente** (`--agente` o `FABRICA_AGENTE`); `paso` y `latido` la renuevan.
2. **Alta:** había una carrera al liberar una reserva caducada. → Ahora se aparta con un rename atómico, se compara el token y, si no era la misma, se devuelve con `os.link`.
3. **Alta:** si git fallaba, se repetía el commit. → `commits_de` lanza un error y `reanudar` no cambia nada.
4. **Media:** la reanudación buscaba solo en HEAD. → Busca en la rama de la E.
5. **Media:** la huella de la validación se tomaba tarde. → `validar en_curso` guarda la huella al empezar, y `hecho` falla si el árbol ha cambiado entretanto.
6. **Media:** se podía tocar la reserva de otro agente. → `paso`, `latido` y `liberar` exigen ser el dueño; `liberar --forzar` queda para Oriol.
7. **Media, Windows:** se reintenta `os.replace` y la lectura si dan PermissionError, y `OpenProcess` con acceso denegado cuenta como proceso vivo. Esto **no está probado en Windows**.
8. **Media:** el coste de la huella en el repo grande. → Está documentado, pero **no se ha medido en el Mac** (ver dudas).
- Bajas también corregidas: los ficheros ajenos en `reservas/` se ignoran, el trailer no puede partirse en dos líneas, las referencias git se validan y se escapan, y los errores salen como mensaje con código 2 o 4, sin traza.

## Desviaciones de la E
- La huella de `validar` es el árbol git (con un índice temporal), no el texto de `git diff`: el diff cambiaba de forma al commitear y hacía revalidar.
- Las pruebas están junto a la herramienta (`.agents/skills/fabrica/`), porque `tests/tools/` no existe en la raíz de `Software/`.
- El protocolo de reserva cambió respecto a la E por el hallazgo 1: ya no se basa en el PID con la CLI, sino en el agente y su latido.

## SIGUIENTE ACCIÓN
Nada en esta E. Para integrarla: Oriol revisa `git show agente/claude/FRONT-L0` y la junta cuando quiera.

## Dudas para Oriol
- **Medir la huella en el repo real** antes de usarla en la fábrica de noche: `git add -A` sobre un índice temporal recorre los ~12.000 ficheros seguidos, puede descargar de iCloud lo que no esté en local y guarda en `.git` los ficheros nuevos no ignorados. Si es lento, se limita a las rutas de la zona del encargo.
- `liberar --forzar`: ¿solo tú, o también el coordinador?
