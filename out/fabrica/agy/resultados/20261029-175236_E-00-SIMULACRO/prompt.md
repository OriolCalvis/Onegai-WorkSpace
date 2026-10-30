Eres Antigravity trabajando de noche, sin nadie delante, en una sola tarea (una «E») del proyecto Onegai: el juego, el motor C++ MotorGraphico y su lore. Nadie va a contestar preguntas hasta mañana: decide con prudencia y deja constancia.

## Dónde estás
- Tu directorio actual es una copia aislada del repositorio (un worktree de git) solo para esta E: /Users/admin/onegai-ramas/agy-E-00-SIMULACRO
- Rama: agente/antigravity/E-00-SIMULACRO, creada desde claude/unificacion-28-10.
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
# Relevo E-00-SIMULACRO (Antigravity, noche)
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
Base: claude/unificacion-28-10
Origen: simulacro del 29-10 (Oriol presente) para probar el circuito nocturno completo. No es una E del proyecto.
---
# E-00-SIMULACRO — Prueba del circuito nocturno (agente: Antigravity)

Objetivo: comprobar que el circuito funciona de punta a punta. La tarea es mínima a propósito.

## Pasos
1. Lee solo las primeras 15 líneas de `AGENTS.md` y resume en 3 líneas qué exige a los agentes.
2. Crea el fichero `out_simulacro/SIMULACRO.txt` con una sola línea: `Simulacro Antigravity 29-10 OK`. Sirve para probar que puedes escribir y que el script hace el commit.
3. Escribe el relevo `.agy_relevo.md`, y ponlo también como respuesta final.

## Criterios de aceptación
- El resumen de AGENTS.md es fiel (cita la línea).
- Existe `out_simulacro/SIMULACRO.txt` con esa línea exacta y ningún otro fichero cambiado.
- Hay relevo, en el fichero o en la respuesta, y dice si pudiste escribir ficheros.

