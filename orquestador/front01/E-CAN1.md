# E-CAN1 — Venides: ojos y aspecto físico (agente: opencode · Nemotron light 3.5, esfuerzo bajo)

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
