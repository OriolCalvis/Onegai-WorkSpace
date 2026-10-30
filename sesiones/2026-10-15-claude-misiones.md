# 2026-10-15 · Claude · carril `misiones` — MISIONES-02 (objetivos claros)

Ronda del 15/10. Pedido de Oriol: «es bastante difícil incluso para mí saber qué se tiene que hacer». El «seguir misión», la flecha y el mapa grande (`claude_mapa01`) solo guían si cada objetivo trae `donde`.

Paquete: `MotorGraphico/out/claude_misiones01/` (por ancla, formato de `claude_reputacion01`). Informe con la tabla antes/después: `planificacion_alcance_y_vision/OBJETIVOS_CLAROS.md`.

## Qué hay

1. **`tools/medir_objetivos.py`** (+ `tools/test_medir_objetivos.py`, 12 pruebas): lista los objetivos de las aventuras y de `quests.json` y dice si cada uno tiene `donde` y si se sigue (el nivel existe, el objectId está en un nivel, hay camino de puertas desde el Casco Antiguo). Resumen corto; `--detalle` una ficha por objetivo; `--solo-boundington --exigir` sale con 1 si falla alguno.
2. **`donde` en 31 objetivos** (antes: 18 de Boundington y 12 de fuera no tenían). Después: 0 sin destino y 0 que no se resuelven en Boundington; 0 y 0 fuera. `mision_llave` y `carta_del_santuario` se dejan.
3. **28 textos reescritos**: frase corta, imperativa, con el sitio («Investigar las máscaras de los Perdidos» → «Habla con Luisarda, en La Boutique de Madame, y sigue las máscaras.»). Ids, flags y beats no cambian.

## Cómo lo resuelve el juego (comprobado en el código)

`MinimapaTinta.cpp` + `Game::RumboNiveles`: una entrada de `donde` con «/» o «.json» es un nivel y se compara por nombre de fichero; el resto es un objectId que se marca solo en el nivel donde estás. Para una persona de otro nivel: `["assets/levels/X.json", "id"]`. `AdventureScriptLoader.cpp` admite cadena o lista.

## Cómo se ha hecho (y lo que no)

- `device_bash` cayó a mitad de sesión (varios fallos seguidos). Trabajé en el contenedor de Claude con los ficheros de `$M` traídos con `device_stage_files` y volví con `device_commit_files`. No miré `/tmp/ag/misiones/MG`.
- Probado en el contenedor sobre una copia limpia: `aplicar.sh` aplica (9 ficheros, 31 trozos), la segunda vez se niega, las 12 pruebas pasan y `medir_objetivos.py` da 0 sin destino. JSON válido; ids y flags de objetivos intactos.
- **No he podido buscar en toda `tests/`**: miré 43 ficheros (misiones, diario, aventuras, tienda, pergamino, aplicación). Ninguno fija un texto de objetivo; solo citan ids. Falta un `grep -rn` en el resto antes de compilar.
- No se ha compilado nada ni se ha jugado: la flecha no se ha visto en pantalla.

## Decisiones

- Dos etapas (hablar y luego ir): `["griffin", "assets/levels/interior_banos.json"]`, sin el nivel de la primera etapa, para que la flecha siga sirviendo después de hablar. Con la sastrería sí se pone el nivel (Luisarda queda lejos de la taberna).
- La cueva de Gorran: `obj2_espada` apunta a `skilla`, `mazmorra_64x64.json` y `ciudad_oeste.json` (Pico Dragón). La mazmorra no tiene puerta hoy; con Pico Dragón la flecha ya sirve.
- Fuera de Boundington están hechos: silbido de medianoche, señal de ceniza y camino del norte.

## Dudas para Oriol

- ¿Qué puerta lleva a la cueva de Gorran (y al bosque de la precuela)? Hoy `mazmorra_64x64.json` solo cuelga de `ejemplos_objetos.json`.
- `obj_pruebas` nombra la Iglesia de la Luz desde el primer día. ¿Mejor «pregunta por la luz» sin nombrar el sitio?
- `obj2_aliados` nombra al capitán y a los gitanos de Surysal. ¿O que los descubra el jugador?
- La precuela pone a Luisarda en la Taberna Humilde; en los niveles solo está en La Boutique de Madame. Seguí los niveles.
