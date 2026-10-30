# E-CAN3 — Reglas de aparición dudosas y PNJ mudos (agente: opencode · MiMo v2.6, esfuerzo bajo)

Parte de la cola de decisiones de FRONT-01: buscar en la vault la respuesta a preguntas del informe de la fábrica del 29-10 antes de pasárselas a Oriol.

## Preguntas (un paso por pregunta; actualiza el relevo después de cada una)
1. **C3-07.** `perdido_devoto`, `cultista_carcelero` y `criado_fiesta` tienen reglas de aparición dudosas. Para cada uno:
   - según la vault, **dónde y cuándo** tiene que estar (día, franja, lugar, condición);
   - cómo está en el juego: `git -c core.checkStat=minimal grep -n "<id>" agente/claude/F-C3 -- MotorGraphico-main | head -20`.

   Los nombres de la vault no serán los ids: busca por concepto (`devoto`, `carcelero`, `criado` + `fiesta`).
2. **C3-02.** En la fiesta hay 3 músicos y 3 nobles con quienes no se puede hablar. **¿La vault les da diálogo o papel?** Busca `músic`, `noble` y `fiesta` en `Boundington los diálogos.md` y `Capítols/`. Si tienen líneas, cópialas literalmente en el relevo (máximo 10 líneas por personaje) para que otro agente las ponga en el juego.

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
