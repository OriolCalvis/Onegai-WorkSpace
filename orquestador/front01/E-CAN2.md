# E-CAN2 — Venides al final y el aviso de Xila (agente: opencode · GLM 5.3 free flash, esfuerzo bajo)

Parte de la cola de decisiones de FRONT-01: buscar en la vault la respuesta a preguntas del informe de la fábrica del 29-10 antes de pasárselas a Oriol.

## Preguntas (un paso por pregunta; actualiza el relevo después de cada una)
1. **C8-04b.** En el juego, `venides_final` y `venides_huida` dependen de la compañía. Si no pasas por el cuartel, Venides está en el Casco pero no sale al final. **¿Debe ser así según la vault?** Pistas: en `Boundington los diálogos.md` el decorado de la Base Militar dice «Venides en el patio (con 20, 10 o ningún hombre)», y en Las Afueras, «Venides (luchando con Sandes, o retenido, según lo que hiciste)». Busca qué pasa con Venides en el final según las decisiones del jugador.
   Cómo está hoy en el juego: `git -c core.checkStat=minimal grep -n "venides_final\|venides_huida" agente/claude/F-C8 -- MotorGraphico-main | head -20`.
2. **C8-04c.** El texto de Xila cuando se entera (del ataque) y va a avisar a Venides está aprobado, pero no está en el cánon. **¿La vault tiene una escena equivalente?** Busca `xila` junto a `avisar`, `Venides` o `campana` en la campaña y en `Personatges/Xila.md`. Compara con el texto del juego: `git -c core.checkStat=minimal log --oneline -5 agente/claude/F-C8`, el commit de C8-04c, y `git show <hash> --stat` para localizar el fichero. Responde: ¿encaja, contradice algo o es nuevo?

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
