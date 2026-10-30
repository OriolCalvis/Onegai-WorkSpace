# D0004 — Edificios con volumen: paredes verticales en el render isométrico

- **Estado:** `proposed` — espera a Oriol.
- **Fecha:** 2026-09-07 · **Autor:** qwen.
- **Encargo:** «dale mas cariño a los edificios, intenta construirles paredes
  verticales y ponerle elemento isometrico 3D, no que sean simplemente un suelo
  distinto con puerta».
- **Alcance:** cómo se representa y se dibuja un edificio en MotorGraphico.
  No decide el estilo de arte ni entra en autotiles (que D0002 aparca a 1.1).

## Contexto: lo que hay hoy

Un edificio es `tools/gen_ciudad.py:146`:

```python
def edificio(self, x, y, w, h, t, puerta_id):
    """... objeto-puerta. El motor pinta 1 objeto por celda: el volumen son ..."""
    self.objetos.append({"objectId": puerta_id, "position": {"x": px, "y": py}})
```

Es decir: un rectángulo de suelo con otro tileset y **un objeto puerta**. La
descripción del encargo es literalmente exacta.

Y el dato no da para más: `RuntimeTile` (`include/Runtime/RuntimeTile.h`) solo
lleva `m_tilesetID`, `m_variant` y `m_collision`. **No hay altura.** Tampoco en
`RuntimeTileLayer` ni en `RuntimeTileMap`. La proyección es 2:1 (fuente 128×64,
render 64×32, `AGENT_HANDOFF.md`).

Ningún paquete del corte B cubre esto, y D0002 aplaza explícitamente los
autotiles a la versión 1.1. Así que antes de programar hace falta decidir la
forma, porque las tres opciones no cuestan lo mismo ni se pueden mezclar.

## Opciones

### A — Campo de altura por celda

`RuntimeTile` gana una altura (o un nivel de elevación) y el renderer dibuja
las caras verticales de cada celda elevada, ordenándolas con la profundidad que
ya resuelve para entidades.

- **A favor:** es el modelo general; sirve también para acantilados, puentes,
  escaleras y el mapamundi, no solo edificios.
- **En contra:** toca el dato más caliente del motor. `RuntimeTileMap` acaba de
  ser consolidado en C-22 (`be527ec`, 07/09) retirando precisamente campos que
  nadie consumía; añadir altura obliga a pasar por `MapDocument`,
  `EntityLayerDocument`, el importador TMX, el round-trip JSON y los esquemas
  — y C-13/C-14 están vivos sobre esos ficheros. Aumenta el tamaño de cada
  celda de los 97 niveles existentes.

### B — El edificio es una instancia con huella y altura (recomendada)

Un edificio deja de ser suelo: es una entidad del documento de mapa con una
huella (w×h), una altura y un sprite volumétrico. El suelo vuelve a ser suelo.

- **A favor:** reutiliza lo que ya existe y está probado —`MapEntityDocument`,
  los once campos de autoría que C-14 ya hace ir y volver, el round-trip de
  `test_map_round_trip`, y la ordenación de profundidad de entidades del
  renderer—. No toca `RuntimeTile` ni `RuntimeTileMap`, así que no se cruza con
  C-22 ni con C-13/C-14 en el mismo fichero. Encaja con la regla de
  `AGENT_HANDOFF.md`: «mantener las instancias de nivel como datos: la variante
  o el tamaño debe configurarse desde el editor, no mediante nuevas subclases».
  Y es lo que hace el editor de niveles hoy con los props.
- **En contra:** no da altura arbitraria por celda de terreno (un acantilado
  seguiría sin poderse). La oclusión —que una pared tape al jugador que está
  detrás— hay que resolverla contra entidades, y `OcclusionRules` hoy trabaja
  por celdas de mapa.

### C — Solo arte: edificios pintados, sin volumen real

Sprites de fachada más altos que la celda, colocados como objetos. Es lo que ya
pasa con los props altos (64×64 con los pies en el borde inferior).

- **A favor:** días, no semanas. No toca el motor.
- **En contra:** no hay pared, no hay oclusión y no hay profundidad correcta: el
  jugador puede quedar pintado por encima del tejado. Es exactamente «un suelo
  distinto con puerta» pero con un dibujo mejor, que es lo que el encargo
  descarta.

## Recomendación

**B**, con una primera rodaja acotada y verificable sin abrir el editor:

1. Una fachada volumétrica en el tileset (`tools/gen_tileset_iso.py`) para una
   altura de referencia, respetando la relación 2:1 y la regla de los pies al
   borde inferior.
2. Campo de altura (o id de volumen) en la instancia de edificio del documento
   de mapa, con su round-trip cubierto por `test_map_round_trip`.
3. El renderer dibuja el volumen y lo ordena con las entidades; la oclusión del
   jugador detrás de una fachada se resuelve con `OcclusionRules` ampliado.
4. `gen_ciudad.py` emite los edificios como instancias volumétricas y se
   regenera la ciudad con `tools/regenerar_ciudad.sh` (nunca los scripts
   sueltos: Parte 3, «el orden de los generadores importa»).

Cada paso tiene prueba propia, y el 3 es el que decide si esto es «3D de
verdad» o decorado. Si la rodaja 3 no sale, se ha gastado el trabajo de las 1 y
2 y se vuelve a C sin haber roto nada.

## Lo que necesita Oriol

- ¿B o A? La diferencia es si el motor tendrá altura de terreno general (A) o
  solo edificios con volumen (B). A es más capaz y bastante más caro, y se
  cruza con C-13/C-14/C-22.
- ¿Entra en el corte B como paquete nuevo con su estimación (las estimaciones
  son de Codex, D0002), o se queda en 1.1 junto a los autotiles?
- La oclusión (3) es lo que hace que se vea 3D de verdad. Si se recorta, el
  resultado es la opción C con más pasos.
