
---
# Archivado el 14/10/2026 (noche) por Claude (coordinador): fichero completo tal como estaba

# codex (ChatGPT) — tarea actual
**Asignada:** 14/10 (12:31) por Claude, a petición de Oriol · **Repo:** `MotorGraphico-main` · rama propia
(regla 2 del protocolo). Lee antes el PROTOCOLO de `orquestador/README.md`.

## AV15-02b — Los sitios por su nombre, en los datos
**PO-39, PO-40 y PO-41 ya están contestadas (14/10, 13:01) y la tabla ya lleva los cambios:**
- la posada es la **Taberna Rica**;
- la tienda de Luisarda es **La Boutique de Madame**;
- **Lingote de Oro** es la herrería de Aigren.

Puedes empezar.

Con tu tabla `planificacion_alcance_y_vision/NOMBRES_BOUNDINGTON.md`, ya revisada por Oriol:

- **Salida:** NUEVO `assets/catalogs/lugares.json`, una entrada por interior con puerta en la ciudad.
  - **Formato:**

    ```json
    {"lugares": [{"nivel": "assets/levels/interior_sastreria.json", "nombre": "La Boutique de Madame",
                  "funcion": "sastre y tela", "emblema": "<id de icono>", "quien": "Luisarda"}]}
    ```

  - **`nivel`:** el `targetLevel` de la puerta en `assets/levels/ciudad_centro.json`, `ciudad_este.json`
    y `ciudad_oeste.json`.
  - **`emblema`:** un id de `assets/textures/iconos.json` si alguno sirve. Si no, `""`, y apunta el
    encargo del icono en `orquestador/pixel.md` (16 px, un objeto reconocible en 5 s).
- **Prueba:** `tests/Game/LugaresTests.cpp`, con `cmake/Lugares.cmake` (CTest `unit;gl-free`). Comprueba
  que:
  - cada puerta de la ciudad que lleva a un interior tiene su lugar;
  - no hay nombres repetidos;
  - los emblemas que no son `""` existen en `iconos.json`.
- **Zona:** solo esos ficheros nuevos y **una línea** en `CMakeLists.txt`, el `include` de tu cmake junto
  a `include(cmake/PlantillaArea.cmake)`. **No** toques los niveles ni el motor: el cartel sobre la puerta,
  el minimapa y la tienda los hace Claude (Cowork) con tu fichero.
- **Entrega:** como AV13-08. Si no llegas a la carpeta, los ficheros enteros a `out/chatgpt_av15-02b/`.

---
## INFORME — AV15-02b (Codex, 14/10/2026)

- **Estado: hecho e integrado en el paquete de Claude (`claude_dialogos04`), confirmado por Oriol.**
  Datos, cinco pruebas y ajuste del validador ya están incluidos. Oriol confirma
  que las dos pruebas de contenido afectadas pasan. **PO-43 resuelta. No fusionar
  ni reaplicar las ramas de Codex:** duplicarían las líneas CMake ya integradas.
- **Rama y commits:**
  - `agente/codex/AV15-02b` · **`5a2e031`**, desde `76c8d7a`; worktree `~/onegai-ramas/codex-AV15-02b`.
  - Tabla aprobada: `agente/codex/AV15-02a` · **`0232e3e`** (corrige la propuesta `54724e2`).
  - Hooks activos; commits por `herramientas/git-safe.sh`; índices revisados por pathspec.
- **Qué ha cambiado:**
  - `assets/catalogs/lugares.json`: 17 interiores, nombres y funciones aprobados; seis emblemas existentes y once vacíos; responsables documentados o cadena vacía.
  - `tests/Game/LugaresTests.cpp`: cinco pruebas sobre assets reales: formato, cobertura exacta de puertas, nombres únicos, iconos válidos y correcciones PO-40/41.
  - `cmake/Lugares.cmake`: CTest `unit;gl-free`, enlazado solo a `motor_json`; lee assets fuente, sin depender de una copia vieja del build.
  - `CMakeLists.txt`: una línea, `include(cmake/Lugares.cmake)`, junto a PlantillaArea.
  - `orquestador/pixel.md`: once encargos de objetos de 16 px; laúd/pergamino conservan la prueba visual de cinco segundos pendiente.
  - Tabla, bandeja y TABLERO: PO-39…41 resueltas, D0015 ya tenía la corrección; no se duplicó ni se reescribió su histórico.
- **Entrega directa:** [chatgpt_av15-02b.tgz](../MotorGraphico-main/MotorGraphico/out/chatgpt_av15-02b.tgz) · [README](../MotorGraphico-main/MotorGraphico/out/chatgpt_av15-02b/README.md).
  Incluye los tres archivos nuevos idénticos al commit, un parche CMake de una
  línea aplicable al paquete actual de Claude, manifiesto SHA-256, tabla aprobada,
  encargos Pixel y logs. El parche separado es un recibo histórico: Claude ya lo aplicó en su paquete.
- **Cómo lo pruebo (en el worktree preparado; copiar y pegar):**

```bash
cd "$HOME/onegai-ramas/codex-AV15-02b/MotorGraphico"
cmake --build out/build/codex-av15-02b --target test_lugares -j4
ctest --test-dir out/build/codex-av15-02b -R '^test_lugares$' --output-on-failure
ctest --test-dir out/build/codex-av15-02b -L gl-free --output-on-failure
python3 tools/linea_base_core.py --rapido --indice
```

Resultado: **5/5 casos**, **5/5 mutaciones rechazadas**, guardia rápida y hooks
verdes. Medición histórica de la rama original, antes de la integración: **210/213**; fallaba la deuda
previa de `linea_base_core` y dos comprobaciones de contenido por el contrato P0
RPG aplicado a `lugares.json`. Esos dos rojos eran de esta entrega y Oriol confirma que ya están corregidos en su paquete; no se presentan como deuda ajena. Propuesta del validador probada sobre copia
de todos los catálogos: original exit 1, propuesta exit 0.

- **Qué tiene que mirar Oriol / qué recibe Claude:**
  - **Lingote de Oro = Aigren**, **La Boutique de Madame = Luisarda** y
    **interior_posada = Taberna Rica**. No quedan preguntas de nombres pendientes.
  - Reutilizados: `oro`, `lore`, `ranura_cabeza`, `ranura_accesorio`,
    `ranura_mano_principal`, `ranura_mano_izquierda`. Los once IDs pedidos a Pixel
    aún no existen y por eso no se referencian en el JSON.
  - No se cambian niveles ni motor; Claude integra el cartel, minimapa y tienda.
  - **PO-43 resuelta:** el integrador aplicó el ajuste. Comprobación propia del
    tar: datos/test/cmake idénticos a `5a2e031`, filtro del validador presente y
    un único include de Lugares.cmake. No se ha repetido la suite de Claude.
  - **Ruta de aplicación:** seguir [REVISAR.md](../REVISAR.md). Las ramas
    `agente/codex/AV13-08`, `AV15-02a` y `AV15-02b` quedan solo como referencia;
    no hacer merge ni volver a aplicar sus commits/parches.
- **Fichero de sesión:** [sesiones/2026-10-14-codex-av15-02b.md](../sesiones/2026-10-14-codex-av15-02b.md).

Los dos worktrees quedan limpios. Sin push ni merge; trabajo ajeno intacto.


---
# Historial (tareas anteriores)

# (anterior)
**Asignada:** 14/10 (09:51) por Claude, a petición de Oriol («tareas para ChatGPT para trabajar ahora»)
· **Repo:** `MotorGraphico-main` · **Tipo:** trabajo nuevo → rama propia (regla 2 del protocolo).
Lee antes el PROTOCOLO de `orquestador/README.md`.

## AV13-08 — Plantillas de área sobre casillas (lo que espera el combate de Claude)
Plan: `planificacion_alcance_y_vision/` (fase AV13, carril paralelo) y `COMBATE_BG3.md` §5. Claude lo
necesita para AV13-14: que una carta de área pille a todos los de dentro, aliados incluidos, y se vea en
el suelo antes de confirmar.

- **Zona (solo esto):** NUEVOS `include/Game/PlantillaArea.h`, `src/Game/PlantillaArea.cpp`,
  `tests/Game/PlantillaAreaTests.cpp`, `cmake/PlantillaArea.cmake`, y **una línea** en `CMakeLists.txt`:
  el `include(cmake/PlantillaArea.cmake)` junto a `include(cmake/RutaCasillas.cmake)`, y
  `src/Game/PlantillaArea.cpp` en la lista de `motor_level`, junto a `src/Game/RutaCasillas.cpp`. Sin GL.
  No toques nada más (el combate, `BattleState`, `TableroCombate` y el HUD son de Claude).
- **Qué:** las casillas que pilla cada forma desde un origen, y una dirección para el cono y la línea.
  Mira `include/Game/TableroCombate.h` y `include/Game/RutaCasillas.h` para el estilo: `GridCoord`,
  comentarios en castellano, funciones puras, callback `std::function<bool(int x, int y)>`.
  - **Círculo** de radio N: las casillas cuyo centro está a ≤ N (distancia euclídea, en casillas).
  - **Cono de 90°** de N casillas, hacia una de las 8 direcciones.
  - **Línea** de N casillas (Bresenham), hacia cualquier casilla destino.
  - **Cuadrado** de lado N, centrado.
  - **Corte por la vista:** opcional, con `bloquea(x, y)`: una casilla solo cuenta si la línea desde el
    origen no pasa por un muro (como `TableroCombate::seVen`).
  - Interfaz sugerida: `struct PlantillaArea { enum class Forma { Circulo, Cono, Linea, Cuadrado }; ... };`
    y `std::vector<GridCoord> casillas(const PlantillaArea&, GridCoord origen, GridCoord haciaDonde,
    const std::function<bool(int,int)>& bloquea = {})`, ordenadas por filas y sin repetidas.
- **Hecho cuando:**
  - cada forma tiene pruebas con su dibujo ASCII esperado (radio 1, 2 y 3; el cono en las 4
    direcciones rectas y una diagonal; una línea diagonal; el cuadrado de lado 3);
  - hay una prueba con el corte de un muro;
  - el test se registra en CTest con la etiqueta `gl-free`, como `cmake/RutaCasillas.cmake`;
  - `ctest -L gl-free` sigue igual de verde.
- Pide tu fila en el TABLERO («Pendiente» → «En curso» → «Hecho») y escribe tu fichero de sesión.
- **Si no puedes abrir el repositorio** (ChatGPT sin acceso a la carpeta): pide a Oriol que te pegue
  `TableroCombate.h`, `RutaCasillas.h` y `cmake/RutaCasillas.cmake`. Entrega los cuatro ficheros
  enteros en la respuesta, y Oriol los deja en `MotorGraphico/out/chatgpt_av13-08/`; Claude los integra.

---
## SIGUIENTE (solo si Oriol te lo dice al pegar el mensaje) — AV15-02a — Los sitios de Boundington por su nombre
Oriol (14/10, D0015 §14): «los sitios tienen que llamarse por su nombre, por encima de su función:
donde está Luisarda, "El lingote de oro"». Es texto, no código.

- **Entrada:** los interiores de Boundington que tienen puerta en la ciudad
  (`assets/levels/ciudad_centro.json`, `ciudad_este.json`, `ciudad_oeste.json`):
  - banco, baños (las termas), biblioteca, castillo, coliseo, herrería, iglesia, joyería;
  - mercado, ópera, posada, sastrería (la de Luisarda: **«El lingote de oro»**, lo decide Oriol);
  - taberna (la **Taberna Humilde**, ya es canon), universidad, casa de té, armería, cuartel.

  El tono y quién está en cada sitio, en `MotorGraphico/planificacion/CANON_BOUNDINGTON_CAMPANA01.md`.
- **Salida:** `planificacion_alcance_y_vision/NOMBRES_BOUNDINGTON.md`, con una tabla de una fila por
  sitio:
  - el nombre propio («El lingote de oro»);
  - la función, en dos o tres palabras («sastre y tela», para el cartel);
  - el emblema del cartel: un objeto reconocible en 5 segundos (yunque, frasco, jarra), sin símbolos
    abstractos (es una regla de los apuntes de usabilidad de Oriol, `USABILIDAD_UOC.md`);
  - quién lo lleva, si el canon lo dice.
- **Reglas:**
  - no cambies los nombres que ya son canon (Taberna Humilde, Taberna Rica, El lingote de oro…);
  - nombres cortos, en castellano, con el sabor del canon; nada de chistes modernos;
  - una nota al final con las dudas para Oriol.
- **Hecho cuando:** la tabla está completa y Oriol la ha revisado. Claude pone los carteles (AV15-02).

**Después** (cuando Oriol lo diga): `E10-ENCUADRAR-MAPA`, tu fila viva del editor. Se queda en tu fila
del TABLERO tal cual.

---
## INFORME (Codex, 14/10/2026)

- **Estado: AV13-08 entregada para AV13-14; AV15-02a pendiente de revisión.**
  La geometría y las pruebas de AV13-08 están completas. Oriol confirma el relevo:
  Claude hará las áreas en el combate (AV13-14). El alcance desde un ejecutable
  sigue pendiente de esa integración. AV15-02a tiene la tabla completa y espera
  la revisión de Oriol.
- **Ramas y commits:**
  - `agente/codex/AV13-08` · `76c8d7a` · `~/onegai-ramas/codex-AV13-08`.
  - `agente/codex/AV15-02a` · `54724e2` · `~/onegai-ramas/codex-AV15-02a`.
  Ambos parten de `11ff327`; commits con `herramientas/git-safe.sh` y hooks activos.
- **Qué ha cambiado:**
  - `MotorGraphico/include/Game/PlantillaArea.h`: datos de forma/tamaño y función pura `Game::casillas`.
  - `MotorGraphico/src/Game/PlantillaArea.cpp`: círculo euclídeo, cono de 90° en ocho direcciones, línea Bresenham y cuadrado; corte opcional por vista y orden por filas.
  - `MotorGraphico/tests/Game/PlantillaAreaTests.cpp`: 20 casos, dibujos ASCII, muros, degenerados, orden y coordenadas extremas.
  - `MotorGraphico/cmake/PlantillaArea.cmake`: CTest `unit;gl-free`, enlaza `motor_level`, sin compilar fuentes sueltas.
  - `MotorGraphico/CMakeLists.txt`: exactamente dos inserciones, fuente en `motor_level` e include de su CMake.
  - [NOMBRES_BOUNDINGTON.md](../planificacion_alcance_y_vision/NOMBRES_BOUNDINGTON.md): 17 interiores, función de 2–3 palabras, emblema y responsables documentados. Copia idéntica versionada en la rama AV15-02a.
  - `TABLERO.md`, `BANDEJA_PRODUCT_OWNER.md` y el fichero de sesión: coordinación y PO-39…41.
- **Cómo lo pruebo (copiar y pegar):**

```bash
cd "$HOME/onegai-ramas/codex-AV13-08/MotorGraphico"
cmake -S . -B out/build/codex-av13-08 -DCMAKE_BUILD_TYPE=Release \
  -DFETCHCONTENT_SOURCE_DIR_GLM="/Users/admin/Documents/Documentos - Oriol Os (2)/Software/MotorGraphico-main/MotorGraphico/build/_deps/glm-src" \
  -DFETCHCONTENT_SOURCE_DIR_GLFW="/Users/admin/Documents/Documentos - Oriol Os (2)/Software/MotorGraphico-main/MotorGraphico/build/_deps/glfw-src" \
  -DMOTOR_PYTHON3="/Users/admin/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3"
cmake --build out/build/codex-av13-08 --target suite_gl_free simulador_dificultad -j6
ctest --test-dir out/build/codex-av13-08 -R '^test_plantilla_area$' --output-on-failure
ctest --test-dir out/build/codex-av13-08 -L gl-free --output-on-failure
python3 tools/linea_base_core.py --rapido --indice
```

Resultado: **20/20 casos de la plantilla**; **211/212 CTest GL-free**. Solo falla
`linea_base_core`: 0 fallos instantáneos y 44 de deuda nueva respecto a su foto.
Ese mismo resultado se reprodujo sobre un `git archive 11ff327`. La guardia rápida
y los hooks de los dos commits pasan. El Python habitual carecía de NumPy;
se usó el runtime instalado, sin instalar paquetes. El simulador se construye
explícitamente porque HEAD registra su CTest pero no lo agrega a `suite_gl_free`.

- **Qué tiene que mirar Oriol / qué recibe Claude:**
  - En HEAD todavía no están `RutaCasillas` ni `TableroCombate`: se leyeron del
    paquete `out/claude_dialogos04/dialogos04.tgz`, sin aplicarlo. Las inserciones
    CMake están junto a `BattleState`/`TargetSelection`; al integrar, agruparlas con
    `RutaCasillas` y conservar el include nuevo si se retira el menú antiguo.
  - AV13-14 debe consumir `Game::casillas` para previsualizar y aplicar el área a
    todos los ocupantes. `medir_alcance` recién ejecutado confirma que la cabecera
    y su fuente todavía solo llegan a la prueba. La foto versionada de alcance y
    la línea base no se alteraron; no se cierra producto con una pieza sin llamador.
  - Convenciones documentadas: círculo/cuadrado incluyen origen, cono/línea lo
    excluyen; cono de radio euclídeo con bordes incluidos; línea de N pasos que se
    prolonga más allá de un destino cercano; cuadrado par desplazado media casilla
    arriba/izquierda; vista con extremos excluidos, como `seVen()`.
  - **PO-39:** revisar la tabla de nombres y emblemas. **PO-40:** el resumen llama
    «Lingote de Oro» a la herrería y D0015 fija «El lingote de oro» para Luisarda;
    se conservan ambos, distinguidos por yunque/carrete. **PO-41:** ubicar la
    Taberna Rica; no se le asigna la posada por suposición.
- **Fichero de sesión:** [sesiones/2026-10-14-codex-av13-08-av15-02a.md](../sesiones/2026-10-14-codex-av13-08-av15-02a.md).

Los dos worktrees quedan limpios. No se ha hecho push ni merge. La rama principal,
el código de Claude y la fila E10-ENCUADRAR-MAPA conservan su trabajo.

### Entrega directa para AV13-14

**Paquete listo:** [chatgpt_av13-08.tgz](../MotorGraphico-main/MotorGraphico/out/chatgpt_av13-08.tgz).
**Guía/API:** [README](../MotorGraphico-main/MotorGraphico/out/chatgpt_av13-08/README.md).
Los cuatro archivos son los de `76c8d7a`. Usar `CMake-integracion.patch` del paquete
para insertar la fuente y el include junto a RutaCasillas en el combate actual:
no reponer los anclajes de TargetSelection, que Claude ya retira.

Comprobado el 14/10 sobre el paquete de Claude de las 11:42: CMake patch aplicable,
20/20 casos de la plantilla y 6.912 comparaciones de corte por vista idénticas a
TableroCombate::seVen. Código de producto sin cambios; rama limpia. Los logs y
el manifiesto están en el paquete. El relevo está documentado en la sesión.


---
## Hasta el 15/10 (pasada a historial al poner la cola del 15/10)

# codex (ChatGPT) — tarea actual
**Asignada:** 14/10 (noche) por Claude (coordinador) · **Repo:** `MotorGraphico-main` · **Tipo:** tu fila viva del TABLERO
(«E10-ENCUADRAR-MAPA», Codex, desde el 08/10) → rama propia (regla 2). Lee antes el PROTOCOLO de `orquestador/README.md`.
La tarea anterior (AV15-02b, ya integrada en `claude_dialogos04`) y su informe están en `orquestador/historial/codex.md`.
**No juntes ni reapliques** `agente/codex/AV13-08`, `AV15-02a` ni `AV15-02b` (duplicarían CMake).

## E10-ENCUADRAR-MAPA — cerrarla
El código ya está en HEAD desde el punto de guardado `11ff327` (08/10): `include/Editor/MapFraming.h`,
`src/Editor/MapFraming.cpp`, `tests/Editor/MapFramingTests.cpp`, `cmake/EditorMapFraming.cmake` (test
`test_editor_map_framing`, `unit;gl-free`) y su uso en `editor/src/EditorVistaDeMapa.cpp` (línea 4). Pero entró
mezclado con todo lo demás, sin la verificación ni el informe que pedía tu fila.
- **Qué falta (lo que dice tu fila en «Entrega verificable»):**
  - comprobar en un worktree de HEAD que «Abrir/Centrar» encuadra las baldosas con margen usando el tamaño real del
    visor, dentro del zoom 0.15..1, se aplica una vez y conserva paneo/zoom tras editar;
  - el humo de UI a escala 1 y 2 (captura de cada una);
  - si algo no cumple, arréglalo **solo** en tu zona: `editor/include/EditorVistaDeMapa.h`,
    `editor/src/EditorVistaDeMapa.cpp` y los cuatro ficheros de `MapFraming`. Nada de `EditorApplication`, paneles ni `Camera`.
- **Hecho cuando:** `test_editor_map_framing` verde y las dos capturas en tu fichero de sesión; tu fila pasa a «Hecho»
  con el enlace a la sesión. Si no hace falta cambiar código, no hay rama: dilo en el INFORME.

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:** `agente/...` · hashes
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
