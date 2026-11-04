Base: agente/claude/UNI-02-11
Origen: orquestador/front01/E-CAN1.md (antes asignada a Nemotron; reasignada a Antigravity el 29-10 porque Nemotron no sirve para lore propio)
---
# E-CAN1 — Venides: ojos y aspecto físico (agente: Antigravity, noche)

Parte de la cola de decisiones de FRONT-01: buscar en la vault la respuesta a preguntas del informe de la fábrica del 29-10 antes de pasárselas a Oriol.

## Preguntas (un paso por pregunta; actualiza el relevo después de cada una)
1. **¿De qué color son los ojos de Venides, azules o verdes?** (pregunta 1 de C3-04c-dis). Una búsqueda rápida no lo encontró, y no hay `Personatges/Venides.md`. Busca `venides` junto a `ojos`, `mirada` o `azul`/`verde` en la campaña, en los índices y en `$V/OnegaiTimeLine/TML/` (eventos de Ceaseton 1982 b.f. y Boundington 1981 b.f.).
2. **Su aspecto físico en 800 d.e.** (edad, pelo, cicatrices, ropa, armadura). Recoge todo lo que diga la vault, con citas.
3. **Borrador de ficha.** Con lo encontrado, escribe en el relevo una propuesta de `Personatges/Venides.md` con el mismo formato que `Personatges/Xila.md` (mira solo sus encabezados: `grep -n '^#' …/Xila.md`). **No la crees en la vault**: la crea Oriol si le parece bien.

## Dónde buscar (vault de Obsidian: cánon, solo lectura)
Desde `Software/`:
- `V="Onegai 2/Onegai"`
- Campaña del juego: `$V/OnegaiTimeLine/Campanyes/Canon/Aegroum/Campanya 1 Boundinghton/`. Lo principal es `Boundington los diálogos.md` (más de 1.000 líneas: **solo con grep**). También están `Campanya 1 Boundington - Estructura.md`, `Capítols/`, `Personatges/`, `Llocs/` y `Subquests/`.
- Índices: `$V/Referència/Índexs/indice_personajes.md` e `indice_lugares.md`.

Busca con `grep -rn -i "<palabra>" "<carpeta>" --include=*.md | head -20`. Después lee solo el trozo: `sed -n 'A,Bp' "<fichero>"`.

Orden de autoridad:
1. La campaña.
2. Los índices. Si un índice marca una **decisión del autor** (⚠️), esa manda.
3. Los JSON del juego **no** son cánon.

## Resultado (en el relevo), por pregunta
- **Veredicto:**
  - *lo dice la vault*, con la cita literal corta, el fichero y el encabezado o la línea;
  - *no lo dice*, con qué has buscado;
  - *se contradice*, con las dos citas.
- **Cómo está hoy en el juego**, si te lo pide la pregunta, con el fichero y la línea en la rama indicada.
- **Propuesta** para Oriol, en una línea, marcada como propuesta.

No escribas nada en la vault ni en el juego.


## Corrección pedida (revisión 2026-11-02)
No es una devolución: la E no llegó a ejecutarse (01-11 y 02-11 BLOQUEADO creando el worktree, `fatal: referencia inválida: claude/unificacion-28-10`; esa rama se perdió en la desaparición del repo del 29-10 y no se recuperó). Claude cambia la base a `prueba/claude/integracion-I-30-10` (tiene `Onegai 2/Onegai/` y `orquestador/front01/`).

**Importante para Antigravity:** esta E es solo de lectura. **No intentes escribir ningún fichero, tampoco `.agy_relevo.md`**: en el modo nocturno escribir está denegado y la ejecución se corta sin respuesta (pasó el 29-10 y el 31-10). Pon el relevo completo, con el formato pedido, **solo en tu respuesta final**, e indica en «Estado» que no se escribió ningún fichero a propósito.

## Corrección pedida (revisión 2026-11-03)
No es una devolución (cuenta 0/2): el 03-11 el worktree se creó bien (HEAD b41f8a43), pero la ejecución se cortó a los 10 s con la respuesta vacía porque Antigravity intentó **ejecutar un comando de shell** (`denied_actions: RunCommand`; stderr: «a tool required the "command" permission that headless mode cannot prompt for, so it was auto-denied»). En el modo nocturno cualquier herramienta que pida permiso corta la ejecución entera.

**Importante para Antigravity, por encima de lo que digan las instrucciones de arriba:**
- **No ejecutes ningún comando de terminal** (ni `grep`, ni `sed`, ni `ls`): están denegados. Donde la E dice `grep`/`sed`, usa solo tus herramientas internas de lectura y búsqueda de ficheros (buscar texto en ficheros, buscar por nombre, listar carpeta, ver un fichero por rango de líneas).
- **No escribas ningún fichero**, tampoco `.agy_relevo.md` (escribir también está denegado).
- Pon el relevo completo **solo en tu respuesta final**, e indica en «Estado» que no se ejecutó ningún comando ni se escribió ningún fichero a propósito.
- Si hasta las herramientas de lectura te piden permiso, termina enseguida con un relevo «bloqueado» que lo diga.


## Actualizacion 03-11 (Claude)
La pregunta 1 ya esta contestada por Oriol (D0022 6.1): los ojos de Venides son **azul grisaceo**. No la busques. Quedan las preguntas 2 y 3. Base nueva: `agente/claude/UNI-02-11` (D0022 1.1).
