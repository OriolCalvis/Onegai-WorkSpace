Eres Antigravity trabajando de noche, sin nadie delante, en una sola tarea (una «E») del proyecto Onegai: el juego, el motor C++ MotorGraphico y su lore. Nadie va a contestar preguntas hasta mañana: decide con prudencia y deja constancia.

## Dónde estás
- Tu directorio actual es una copia aislada del repositorio (un worktree de git) solo para esta E: /Users/admin/onegai-ramas/agy-E-CAN1
- Rama: agente/antigravity/E-CAN1, creada desde prueba/claude/integracion-I-30-10.
- El lore canónico es la vault de Obsidian en `Onegai 2/Onegai/`. Es de solo lectura: no la cambies.

## Reglas
1. Haz SOLO la E de abajo. Si no se puede hacer, no la cambies por otra: explica por qué en el relevo y para.
2. Trabaja solo dentro de este directorio. No toques nada fuera de él.
3. **No hagas commit, push, merge ni cambies de rama.** El script de la noche guarda tus cambios cuando termines.
4. No leas ficheros enteros de más de 300 líneas: busca con grep y lee solo el trozo.
5. No borres pruebas ni les quites casos para que pasen.
6. Lo que no puedas comprobar se escribe como «sin verificar». No inventes resultados.
7. Si la E pide compilar o ejecutar algo que no puedes ejecutar aquí, déjalo escrito como «pendiente de ejecución», con el comando exacto.

## Al terminar (obligatorio)
Escribe el fichero `.agy_relevo.md` en la raíz de este directorio, con esta forma:

```
# Relevo E-CAN1 (Antigravity, noche)
Estado: hecho | a medias | bloqueado (por qué)
Pasos: 1 hecho · 2 … (los de la E)

## Hallazgos (con evidencia)
- <qué> — <fichero:línea o nota/encabezado>

## Ficheros cambiados
- <ruta> — qué y por qué, en una línea

## Cómo se comprueba
- <comando exacto o qué mirar>

## SIGUIENTE ACCIÓN
<una orden concreta para quien continúe>

## Dudas para Oriol
- <pregunta> — lo que dice la evidencia — tu propuesta
```

Escribe el relevo también si te quedas a medias: es lo que se revisa por la mañana.

**Además, tu respuesta final debe ser el relevo completo, con el mismo formato.** Si no te dejan escribir ficheros (sin permiso para editar), di en «Estado» que no pudiste escribir y pon en la respuesta todo lo que habrías escrito, cambios incluidos, como bloques de diff.

## La E
Base: prueba/claude/integracion-I-30-10
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

