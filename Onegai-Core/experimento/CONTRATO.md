# El experimento de los cuatro — contrato

*Cuatro IAs construyen Egaroth a la vez, un cuadrante cada una, para el motor gráfico.*
*15 de agosto de 2026 · Egaroth, año 2000 b.f.*

---

## 0. Lo primero: el mundo ya existe

**No se está creando Egaroth desde cero.** Existe, es canon y costó trabajo levantarlo: 19 naciones con polígono, 95 ciudades con coordenadas, 26 zonas, fechado en 2000 b.f. — el año en que empieza la Primera Cruzada.

Este experimento **rellena** ese mundo con contenido jugable, no lo reinventa. Es la diferencia entre cuatro urbanistas construyendo en parcelas de una ciudad trazada, y cuatro urbanistas trazando cuatro ciudades distintas encima de la misma.

> Si alguna IA inventa una nación, mueve una frontera o cambia una capital, **el validador lo rechaza**. No es un castigo: es que ese dato ya existe y hay tres compañeras trabajando contra él.

---

## 1. El reparto

| Cuadrante | IA | Prefijo | Naciones | Celdas |
|---|---|---|---|---:|
| **Oeste-Norte** | Claude | `on_` | Ecla, Udrax, Ostad, Gliaddokx, *Bastrea* | 54 |
| **Oeste-Sur** | Trae | `os_` | Aegroum, *Bastrea*, Ascaria | 50 |
| **Este-Norte** | Zcode | `en_` | Gongorguma, Choubar, *Ashye*, *Bosmurg* | 44 |
| **Este-Sur** | ChatGPT | `es_` | Tabaxi, *Bosmurg*, Esmua, Wulcain, *Ashye* | 47 |

El reparto no es arbitrario: sale de las dos masas de tierra reales del atlas, partidas por su mitad. El continente occidental (104 celdas) es el **frente oeste de las Cruzadas**; el oriental (91), el **frente este**. Están separados por mar, y por eso la cronología numera las Cruzadas por separado hasta la Cuarta.

Datos de máquina: `cuadrantes.json`.

### Las tres costuras

**Bastrea, Ashye y Bosmurg están a caballo entre dos cuadrantes.** Dos IAs escribirán sobre la misma tierra sin hablar entre ellas.

Es a propósito. Es la parte del experimento que más información da: la pregunta no es si cada IA sabe inventar un continente — es si cuatro trabajos independientes **encajan**. `fusionar.py` mide exactamente eso.

| Nación | Capital canónica | La comparten |
|---|---|---|
| Bastrea | Numandum | Claude + Trae |
| Bosmurg | Teshkorr | Zcode + ChatGPT |
| Ashye | Zathor'aetz | Zcode + ChatGPT |

---

## 2. Qué es intocable

| Intocable | Por qué |
|---|---|
| Las **19 naciones** y sus nombres | Salen del índice de lugares de Onegai. El lore manda |
| Las **capitales** de la tabla de arriba | Reconciliadas una a una contra el corpus |
| Las **fronteras** y la rejilla 26×26 | La cronología las corrobora: las guerras ocurren entre vecinos |
| El año **2000 b.f.** | El mapa es una instantánea, no el mundo eterno |
| Todo lo que ya está en `assets/` | Es de otro, o es de Boundington |

**Excepción conocida:** *Ostad no tiene capital en el canon* — el índice dice literalmente "más Ostad sin capital". Quien lleve Oeste-Norte puede proponerla, y quedará marcada como propuesta hasta que Oriol la confirme.

## 3. Qué es terreno libre

Ciudades menores, aldeas, facciones locales, cultura, oficios, conflictos entre vecinos, mazmorras, enemigos, PNJs, aventuras, objetos, rumores. **Todo lo que hace que un sitio se pueda jugar.**

---

## 4. Qué hay que entregar

Contenido que **el motor cargue de verdad**. No documentos: assets.

### Por cada nación del cuadrante, como mínimo

| # | Qué | Dónde | Formato |
|---|---|---|---|
| 1 | **Un nivel de capital** | `assets/maps/<pre>_<nacion>.tmx` + `assets/levels/<pre>_<nacion>.json` | TMX ortogonal 64×32, tileset `ciudad_tileset.png` (36 tiles) |
| 2 | **PNJs** de esa capital | `assets/objects/<pre>_npcs.json` | `{"objects":[…]}`, categoría `npc` |
| 3 | **Una aventura** | `assets/adventures/<pre>_<nacion>.json` | beats + flags. Ver `FORMATO_AVENTURAS.md` |
| 4 | **Enemigos propios** | `assets/objects/<pre>_enemigos.json` | categoría `enemy`, con `combat.maxHealth` |

### La regla que evita el desastre: prefijos

**Todo id que crees empieza por el prefijo de tu cuadrante.** Sin excepción.

```
on_umedan_tabernero      ✅
tabernero                ❌ ya existe en Boundington, lo pisas
on_puerta_forja          ✅
puerta_herreria          ❌ ya existe
```

Es la única regla puramente mecánica del contrato, y es la que decide si los cuatro trabajos se pueden juntar o hay que tirar tres. **El validador la comprueba primero.**

### Excepción: las flags de aventura

Las flags también llevan prefijo (`on_udrax_visitado`), **salvo** las que consultan canon compartido. Si tu aventura necesita saber si el jugador ya pasó por Boundington, se lee `bnd_*` — no se inventa una nueva.

---

## 5. Cómo se valida

```bash
python3 Onegai-Core/experimento/validar_contribucion.py oeste_norte
python3 Onegai-Core/experimento/fusionar.py            # los cuatro a la vez
```

`validar_contribucion.py` comprueba, **de tu cuadrante solo**:

1. Todos los ids llevan tu prefijo
2. No pisas ningún id existente
3. Las naciones y capitales que citas son las canónicas
4. Cada nivel carga: TMX bien formado, `playerStart` transitable
5. **Todo lo transitable es alcanzable** (flood fill). Un umbral rodeado de muros carga sin error y es contenido muerto
6. Cada `objectId` usado en un nivel tiene ficha en algún catálogo
7. Cada aventura parsea, y **ningún beat es inalcanzable**
8. Ninguna flag consumida se queda sin que nadie la encienda

`fusionar.py` comprueba **los cuatro juntos**:

- Colisiones de id entre cuadrantes
- **Divergencia en las costuras**: si Claude y Trae describen Bastrea de forma incompatible, sale aquí
- Que el catálogo resultante siga pasando `validar_catalogos_rpg.py`

---

## 6. Las trampas que ya nos mordieron

> Están todas en `BITACORA_DEL_PROYECTO.md`, parte 3. Estas cuatro son las que más van a doler aquí.

**El parser JSON del motor no decodifica `\uXXXX`.** Es deliberado y está documentado en `JsonValue.cpp`. Y `json.dump` de Python escapa así cualquier no-ASCII **por defecto**. Una tilde en el nombre de un nivel basta para que deje de cargar, con el error inútil *"loadFromFile devolvió error"*.
→ **Usa `ensure_ascii=False`, o nombres sin acentos.**

**`assert()` no evalúa en Release.** Está documentado en `examples/Check.h` y afectó a los 20 demos del repo.
→ **En cualquier demo que haga de test, usa `require()`.**

**Los assets se copian al configurar CMake, no al compilar.** Hay un target `copiar_assets`; si añades un demo que lea assets, cuélgalo de él.

**Un beat que no dispara nunca es contenido muerto.** Se escribió, se revisó y no lo ve nadie. El validador lo detecta; hazle caso.

---

## 7. Cómo se mide el experimento

Esto es un test de carga, así que hay números:

| Métrica | Qué dice |
|---|---|
| Colisiones de id entre cuadrantes | Si el contrato de prefijos aguanta |
| Divergencia en las tres costuras | **La medida principal**: si cuatro trabajos independientes encajan |
| Beats muertos por cuadrante | Calidad interna de cada contribución |
| Niveles con zonas inalcanzables | Si el generador de cada IA entiende la conectividad |
| Referencias rotas tras fusionar | Si los cuadrantes se citan bien entre ellos |
| Naciones/capitales inventadas | Si el canon aguanta cuatro autores a la vez |

**Predicción a registrar antes de empezar:** lo que se romperá primero no serán los mapas, serán **las costuras**. Bastrea tendrá dos capitales de facto, dos culturas y dos conjuntos de facciones, porque nadie va a preguntar. Si me equivoco, mejor.

---

## 8. Por dónde empezar

1. Lee `BITACORA_DEL_PROYECTO.md` — 10 minutos, te ahorra las siete trampas
2. Mira `cuadrantes.json` — tus naciones, tus capitales, tu prefijo
3. Copia el patrón de **Oeste-Norte**, que va primero a propósito para servir de ejemplo:
   - `tools/gen_ciudad.py` para el mapa (`Mapa`, `edificio()`, `conectar()`)
   - `assets/adventures/boundington_primer_dia.json` para la aventura
   - `assets/objects/boundington_npcs.json` para los PNJs
4. Valida antes de entregar

**Referencia obligada:** `MotorGraphico/FORMATO_AVENTURAS.md` y `MotorGraphico/FORMATO_NIVELES.md`. Los dos están escritos desde el código que parsea de verdad, no desde un diseño sobre el papel.
