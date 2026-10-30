# RELEVO — protocolo para agentes con poco contexto (FRONT-01, 29-10)

Este fichero **sustituye**, solo para las E de `orquestador/front01/`, la lectura de `BITACORA_DEL_PROYECTO.md` y `TABLERO.md`, que son demasiado grandes. **No los leas.** Lo demás de `AGENTS.md` sigue valiendo.

## 1. Qué lees (y nada más)
1. Este fichero.
2. Tu E: `orquestador/front01/<E>.md`.
3. Si ya existe, su relevo: `orquestador/front01/relevos/<E>.md`. **Si existe, continúas desde su «SIGUIENTE ACCIÓN»**, sin rehacer los pasos marcados como hechos.

Reglas de lectura:
- Nunca leas un fichero entero de más de 200 líneas. Usa `grep -n` para encontrar el sitio y lee solo ese trozo (`sed -n 'A,Bp'`).
- Limita las búsquedas: `| head -20`.
- Git siempre con `git -c core.checkStat=minimal …` (el repo está en iCloud).
- No compiles ni lances `ctest` salvo que tu E lo diga: la salida llena el contexto.

## 2. El relevo se escribe mientras trabajas, no al final
Ningún agente sabe con seguridad cuántos tokens le quedan, así que no se espera al final para escribir el informe.

- **Después de cada paso** de tu E, actualiza `relevos/<E>.md` con la plantilla del punto 4. Si se te corta la sesión, como mucho se pierde un paso.
- Los hallazgos van **con evidencia**: la ruta del fichero, la línea o el encabezado, y el commit o la rama. Sin evidencia, se escribe «sin verificar».

## 3. Cuándo parar y pasar el relevo
Para en cuanto pase **una** de estas cosas:
- el harness avisa de que queda poco contexto o de que se acaba el límite, o ha compactado la conversación;
- llevas unas **25 llamadas a herramientas** en esta sesión;
- has fallado dos veces en lo mismo, o te das cuenta de que repites algo que ya hiciste;
- la E está hecha.

Al parar: deja `relevos/<E>.md` al día, con la «SIGUIENTE ACCIÓN» escrita como una orden que otro agente pueda ejecutar sin pensar (comando exacto o fichero y línea), y **termina**. No empieces nada nuevo.

## 4. Plantilla de `relevos/<E>.md` (máximo 40 líneas)
```
# Relevo <E>
Agente y sesión: <agente> · <fecha hora> · sesión nº <n>
Sesión externa: <ninguna | codex <id> | opencode <id> | trae <trajectory.json>>
Estado: en curso | hecho | bloqueado (por qué)
Pasos: 1 hecho · 2 hecho · 3 en curso · 4 pendiente

## Hallazgos (con evidencia)
- <qué> — <fichero:línea o nota/encabezado> @ <rama/commit>

## SIGUIENTE ACCIÓN
<una orden concreta: qué comando o qué fichero y qué buscar>

## Dudas para Oriol
- <pregunta> — lo que dice la evidencia — mi propuesta
```

## 4b. Si te lanzan desde Claude Code (Codex, OpenCode o Trae)
- Quien lanza la E apunta en «Sesión externa» el id que devuelve `/codex:result` u `/opencode:result`, o la ruta del `--trajectory-file` de Trae.
- Para relevar, **primero se intenta reanudar esa sesión** (`/codex:rescue --resume`, `/opencode:rescue --resume`). Solo si no se puede, se empieza otra con el mensaje del §6.
- Las revisiones del harness (review gate) van **apagadas**.

## 5. Qué puedes tocar
- **Solo** `orquestador/front01/relevos/<E>.md`, salvo que tu E diga otra cosa.
- No escribes en la vault (`Onegai 2/`), ni en el código, ni en `TABLERO.md`, ni haces commits, push o merge, salvo que tu E lo diga.
- Si no se puede hacer, no cambies de tarea: lo escribes en el relevo como «bloqueado» y paras.

## 6. Mensaje para lanzar o relevar a un agente
> Eres <AGENTE>. Lee `Software/orquestador/front01/RELEVO.md` y después `Software/orquestador/front01/<E>.md`. Si existe `Software/orquestador/front01/relevos/<E>.md`, continúa desde su SIGUIENTE ACCIÓN. Sigue el protocolo de relevo.

Con el mismo mensaje, cualquier otro agente recoge una E a medias.
