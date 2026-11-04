# E-GUION-02 — Guion desde una partida guardada (aprobado por Oriol el 03/11: «Sí, fes-la»)

Base: `agente/claude/UNI-02-11@653ad9ff`. Motor: `MotorGraphico-main/MotorGraphico/` (= MG). Sigue a E-GUION-01 (`78827571`).

## Problema
Para capturar el combate con 5 enemigos, la persecución de los sectarios (día 1) y las campanas del Ocaso, con teclas habría que jugar todo el prólogo y el día 1. El juego ya sabe guardar y cargar (`GameSession::captureSaveGame`/`restoreSaveGame`, `writeSaveGameFile`/`readSaveGameFile` en `include/Game/SaveGame.h`, «Continuar» en `src/Engine/Application.cpp` ~822–900), y las demos gl-free ya llegan a esos momentos (`MG/examples/demo_boundington_partida.h`).

## Supuestos (paso 0)
- Una `GameSession` de las demos puede hacer `captureSaveGame` y el fichero resultante se carga en `juego` por el mismo camino que «Continuar».
- El guardado lleva el héroe (`SavedHero`), así que al cargar el jugador sale vestido.
- Existe en la historia un combate con 5 enemigos (o uno que se pueda arrancar desde un guardado justo antes). Si no existe, se usa el combate con más enemigos que haya y se dice cuál.

## Comportamiento esperado
1. `./juego --guardado ruta.json [--guion g.txt]` arranca cargando ese guardado (sin título), por el mismo camino que «Continuar». Un guardado ilegible: no arranca, código 2 y el motivo.
2. Una herramienta gl-free `fabricar_guardados` (en `MG/tools/` o `MG/examples/`, como encaje mejor con la build) juega con el arnés de las demos hasta tres momentos y escribe:
   - `guardados/sectarios.json`: día 1, justo antes de seguir a los sectarios [Sigilo] (`b1_sectarios_seguir`);
   - `guardados/ocaso.json`: Ocaso, justo antes de que suenen las campanas;
   - `guardados/combate5.json`: justo antes de entrar en el combate con más enemigos.
   Con héroe creado (raza humana, aspecto por defecto) para que salga vestido.
3. Tres guiones `MG/tools/guiones/{sectarios,ocaso,combate5}.txt` que, con su guardado, llegan a la escena y hacen 2–4 capturas cada uno (la mesa de combate con todas las placas; el seguimiento; el ocaso con las campanas).

## Alcance
Puede tocar: `examples/juego.cpp`, `src/Engine/Application.cpp` / `include/Engine/Application.h` (solo el arranque desde fichero, sin hacer crecer las líneas: va a `GuionCaptura` o a un fichero nuevo, porque `linea_base_core` vigila los techos), el fichero nuevo de `fabricar_guardados` con su registro en `cmake/`, `tools/guiones/*.txt`, y una prueba gl-free (`tests/Engine/` o junto a `fabricar_guardados`) que compruebe que los tres guardados se escriben y se vuelven a leer con `readSaveGameFile` + `restoreSaveGame`.
No toca: aventuras, reglas, HUD ni el formato del guardado.

## Criterios de aceptación
1. Prueba gl-free verde: los 3 guardados se fabrican y se restauran en una sesión limpia con el mapa y las flags esperadas. **Mutación:** fabricar `sectarios` un paso después (con el beat ya disparado) → la prueba falla.
2. Las capturas en el Mac enseñan las tres escenas, con el jugador vestido. Si alguna no se consigue, se dice por qué, con la captura de dónde se queda.
3. Suite gl-free sin fallos nuevos (291 o más) y `juego 3` igual.

Presupuesto: 75 min.
