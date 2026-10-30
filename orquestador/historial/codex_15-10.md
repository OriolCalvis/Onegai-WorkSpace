# codex (ChatGPT) — cola de tareas del 15/10
**Asignada:** 15/10 por Claude (coordinador), a petición de Oriol: «planifícalo al máximo detalle para que ChatGPT, Z.AI y Qwen me terminen el trabajo» · **Repo:** `MotorGraphico-main` · **Tipo:** trabajo nuevo, una rama por tarea.
Lo anterior de este fichero está en `orquestador/historial/codex.md`.

Tu parte: **los sitios repetidos y el botín y la economía** (PLAN_15-10.md §2.8 y §4). Es un sistema nuevo, con datos y pruebas, como hiciste con `lugares.json`.

**Antes de empezar (obligatorio):**
- Lee `Software/orquestador/README.md` (el PROTOCOLO), `Software/planificacion_alcance_y_vision/PLAN_15-10.md`
  (el plan de esta cola: cada tarea tiene allí su apartado con el porqué) y `Software/AGENTS.md`.
- **Comprueba que Oriol ha hecho el commit de §0 del plan** (`git log -1` en `MotorGraphico-main/MotorGraphico` debe
  ser del 15/10 o posterior, con «Paquetes del 14 y 15/10»). Si no, **para** y escríbelo en el INFORME: tu rama
  saldría del 08/10 y no vería nada nuevo.
- **Una rama por tarea**, fuera de la carpeta: `git worktree add ~/onegai-ramas/<agente>-<TAREA> -b agente/<agente>/<TAREA>`.
  Trabaja, compila, prueba y commitea allí (con `herramientas/git-safe.sh`). Nunca push ni merge.
- **Compilar y probar** (desde la raíz del worktree):
  `cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release && cmake --build build-release -j8`,
  `ctest --test-dir build-release -L gl-free --output-on-failure`, `python3 tools/validar_todo.py`, y juega
  (`./build-release/juego`) lo que hayas cambiado. Captura de pantalla de lo que se vea.
- **Límites del repo:** ningún fichero de `src/` o `include/` por encima de 500 líneas que no lo estuviera ya, ni más de
  15 miembros `m_` por cabecera (lo comprueba `tools/linea_base_core.py`).
- Escribe en castellano, frases cortas; comentarios en el código igual.
- **Cola:** haz las tareas **en orden**. Al acabar una: su INFORME (abajo, uno por tarea), su fila del TABLERO a
  «Hecho» y su fichero en `sesiones/`; y empieza la siguiente. Si una se bloquea (falta algo de otro), escríbelo en
  su INFORME y pasa a la siguiente que no dependa de ella.
- **Preguntas para Oriol:** a `BANDEJA_PRODUCT_OWNER.md` con número PO-xx (regla 7), y sigue con lo que no dependa.

**Tu zona (no salgas de ella):** `assets/catalogs/lugares.json` y validador; `planificacion_alcance_y_vision/ECONOMIA.md`; ficheros **nuevos** de botín (C++ en `Game/Contenedores.*`, catálogos y `tools/`); interiores `_sur` nuevos; lo mínimo por ancla en `GameSessionInteraccion.cpp` y `SaveGame*`. **No** toques `assets/adventures/` (Z.AI), `ciudad_centro.json` ni los generadores (Qwen), ni el combate, la creación o el render (Claude).

---
## 1. AV16-08 — Cada puerta, un sitio distinto
PLAN §2.8. Hoy el mapa grande enseña dos «La Boutique de Madame», dos «Taberna Rica» y dos «La Caja Firme».
1. Lista las puertas de `assets/levels/ciudad_centro.json` (y de las otras ciudades de Boundington) cuyo `targetLevel`
   lleva al mismo sitio de `assets/catalogs/lugares.json` que otra puerta (las `puerta_*_sur`, sobre todo).
2. **Primero la pregunta:** PO nueva en la BANDEJA con una tabla: puerta, sitio de hoy, **nombre propio propuesto**
   (con la voz de Boundington: mira `planificacion_alcance_y_vision/NOMBRES_BOUNDINGTON.md`), función, tendero y si
   comparte interior. Espera la respuesta para los nombres; mientras, prepara lo demás.
3. Con la respuesta: entradas nuevas en `lugares.json` y, si un segundo local comparte interior, una copia del
   interior (`interior_<x>_sur.json`) con otro tendero (el mismo mobiliario vale). Las puertas `_sur` apuntan a su
   interior propio: **ese cambio en `ciudad_centro.json` lo hace Qwen** (está reordenando la ciudad, AV16-09):
   déjale la lista exacta (puerta → nuevo `targetLevel`) en tu INFORME y en su ficha (`orquestador/qwen.md`, al final).
4. Validador: `tools/validar_lugares.py` (+ prueba): ningún nombre repetido, cada puerta con sitio.
**Hecho cuando:** `validar_lugares.py` en verde y los nombres aprobados por Oriol.

### Respuesta de Oriol a PO-48 y PO-49 (15/10, 20:46) — manda sobre los puntos de arriba
- **Los nombres no cambian.** No crees La Moneda Chica, La Tina Caliente, La Cesta del Vado ni El Candil del Sur:
  esas cuatro puertas duplicadas siguen llevando al local que ya tienen (mismo nombre, mismo interior). Cambia el
  validador para que eso sea válido (varias puertas → un mismo local, a propósito) y no un error.
- **Solo un local nuevo:** `puerta_ropa_sur` deja de ser la segunda sastrería y pasa a ser un **local de artículos de
  pesca, caza y agricultura**. Nombre provisional «El Zurrón del Vado» (si se te ocurre uno mejor con la voz de
  `NOMBRES_BOUNDINGTON.md`, propónlo en tu INFORME), emblema `cana_pescar`, función «pesca, caza y huerto».
  Crea `interior_pesca_caza.json` (puede partir del mobiliario de la sastrería, con tendero propio) y su entrada en
  `lugares.json`. Su tienda vende cañas, cebos, trampas, semillas y aperos: mira `oficios.json` y
  `oficios_tablas.json` en `out/claude_oficios_datos01/`, que son los útiles que suman dados en D0017.
- El cambio de destino de la puerta (`puerta_ropa_sur` → `interior_pesca_caza.json`) **ya está en la ficha de Qwen**
  (AV16-09b). Tú no tocas `ciudad_centro.json`.
- **PO-49 aprobada** (rentas una vez por día de historia; actuar una vez por sitio y día; los oficios gastan su
  franja). Lo programa Claude (AV18-07). Cuando cierres AV16-08, sigue con AV18-03.

### INFORME AV16-08
- **Estado:** hecho en la rama; listo para integrar con Qwen AV16-09b. Los nombres anteriores siguen iguales. Solo se añade El Zurrón del Vado; PO-49 queda en el carril de Oriol/Claude.
- **Rama y commits:** `agente/codex/AV16-08`, `c83c2d9` + `65365fc` + `43fa13d`. No hay push ni merge. El último commit completa la entrega anterior.
- **Cambios:** varias puertas pueden compartir una ficha sin lista de permisos; siguen fallando nombres/fichas duplicados y puertas sin local. Nuevo interior y TMX, tendero con aspecto propio y rutina de mostrador, siete ofertas (caña, cebo, trampa, semillas, cuerda, pala, pico). Caña y pala coinciden con `herramienta` en oficios.json; oficios_tablas.json describe capturas, no otros útiles. Los tres suministros nuevos se venden sin inventarles bonificaciones. Rótulo exacto «pesca, caza y huerto». Emblema `cana_pescar` dibujado en el generador y publicado en el atlas (frame 57); las 56 celdas anteriores conservan sus píxeles. El cartel conserva su id, precio e ingreso, y muestra el nuevo nombre. Sin C++ de producto ni cambios de ciudad.
- **Contrato con Qwen:** `dfd2ffe` ya lleva `puerta_ropa_sur` en (56,58) → `assets/levels/interior_pesca_caza.json`, llegada (1,7). La salida se actualiza a ciudad_centro (56,59), acera transitable frente a la puerta. Las otras cuatro parejas comparten los locales de siempre. Copia conjunta de revisión en `/private/tmp/codex-av1608-cierre-qwen`, sin fusionar ramas.
- **Pruebas:** 17 Python (cuatro regresiones fallaban antes), siete C++ leyendo catálogos reales, rutinas 0 errores/0 avisos, validar_lugares 22 puertas/0 errores. Copia limpia con el destino de la puerta: validar_todo 0 errores/22 avisos. Con toda la ciudad `dfd2ffe`: enlaces 189/0 errores y siete C++ en verde; Release completa y 227/231 gl-free. Los cuatro rojos son las dos guardias heredadas y dos invocaciones de validar_todo que detectan cuatro desajustes PNJ ya reproducidos en la rama Qwen sin esta tienda (estudiante, nino, elfos del bosque y enanos forjadores). No se retocan sus variantes para ocultarlo.
- **Juego real:** captura en el chat del rótulo con caña y del diálogo propio; tienda con las ofertas, compra de caña 150→135 monedas y cantidad 0→1. Se cargan todos los catálogos del manifiesto. La partida de prueba queda cerrada.
- **Sesión:** `sesiones/2026-10-15-codex-av16-08-cierre-local.md`. Sigue AV18-03 reutilizando su rama, con 50 reutilizables/12 únicas ya preparados.

---
## 2. AV18-01 — La economía cerrada de los tres días de Boundington
PLAN §4.1. Fichero nuevo `planificacion_alcance_y_vision/ECONOMIA.md`. Parte del análisis que Oriol pegó al final
de `Software/PENDIENTE_DE_REVISAR.md` (tablas de recompensas, cofres, presupuesto por mapa, venta al 25 %, tres
perfiles) y **adáptalo a lo que ya hay**:
- precios de hoy en `assets/catalogs/` (pan 8, cerveza 6, daga 60, espada corta 120…) y en la tienda
  (`TiendaEstado`, con la reputación de `claude_reputacion01`: el precio ya cambia por facción);
- oficios: `assets/catalogs/oficios.json` (servir paga ~8 por turno) y `decisiones/D0017`;
- los negocios se revenden al 50 % (PO-42): eso no cambia; la venta de **objetos** al 25 %;
- el juego son **tres días** (día 1, día 2, día 3 + ocaso): presupuesto por día y por mapa (ciudad, interiores,
  cripta, afueras, cueva), repartido en cofres visibles, combates, misiones, entorno y tesoros ocultos.
Incluye: tabla de precios por rareza, tabla de recompensas, presupuesto por mapa, reglas (cada cofre una vez, lo
mundano vale poco, lo valioso cuesta esfuerzo, nada esencial en un secreto) y los tres perfiles con su saldo esperado.
Si propones cambiar un precio que ya existe: PO en la BANDEJA.
**Hecho cuando:** ECONOMIA.md escrito y su PO (si hace falta) en la BANDEJA.

### INFORME AV18-01
- **Estado:** hecho (entrega documental). ECONOMIA.md escrito, coherente con los precios y la bolsa inicial real de 150; PO-49 aprobada y recogida en D0019. La integración de la venta al 25 % y el límite de ingresos repetibles corresponde al montaje posterior y no se presenta como resuelta.
- **Rama y commits:** `agente/codex/AV18-01`; entrega `9e6c25b`. Sin push ni merge.
- **Qué ha cambiado:** corregido el saldo inicial de 0 a 150 tras comprobar los dos arranques de Application; recalculados los tres perfiles y sus márgenes. Se mantienen los 15 presupuestos (800/1600/1200, V=3600), los precios, la separación de objetos al 25 % y negocios al 50 %, y PO-49 sobre rentas/actuación repetibles.
- **Cómo lo pruebo (comandos exactos):** desde el worktree, el bloque `json economia` de `planificacion_alcance_y_vision/ECONOMIA.md` se verifica con aritmética racional: perfiles historia 254,90/625,32/836,91; explora todo 287,88/694,19/579,81 (corrección AV18-04); optimiza 493,50/1201,75/1616,25. Nueve saldos dentro de márgenes. CMake y CTest de la base idéntica, salvo documento, constan en AV16-08: Release OK y 229/231, con las dos guardias de deuda ajena.
- **Qué tengo que mirar yo (Oriol):** presupuesto y PO-49. El documento es un diseño con escenarios declarados; AV18-04 debe contrastarlo con tablas y niveles poblados. No hay cambio de precios existente que requiera otra PO.
- **Fichero de sesión:** `sesiones/2026-10-15-codex-av18-01-cierre.md`.

- **Corrección posterior comprobada en AV18-04:** bolsa inicial 150; cesta del explorador del día 2 355, no 455; referencia vigente 287,88 / 694,19 / 579,81. La copia de revisión completa está en `~/onegai-ramas/codex-AV18-06/planificacion_alcance_y_vision/ECONOMIA.md`. PO-49 está aprobada (D0019), su ejecución sigue en Claude.

---
## 3. AV18-02 — Contenedores en el motor
PLAN §4.2. Ficheros nuevos `include/Game/Contenedores.h`, `src/Game/Contenedores.cpp` (sin GL, en `motor_level`),
`tests/Game/ContenedoresTests.cpp`, `cmake/Contenedores.cmake` y su `include()` en `CMakeLists.txt`.
- **Datos:** un bloque `contenedor` en la ficha del objeto (`assets/objects/*.json`; lo lee `src/Level/ObjectCatalog.cpp`,
  como hoy el bloque `actuacion`): `tipo` (cofre, barril, saco, armario, cadaver, escondite), `tabla`, `presupuesto`,
  `garantizados` [ids], `requisitos` {`percepcion`: CD para ver un escondite, `abrir`: ganzua|llave:<id>|fuerza, `cd`}.
- **Motor:** al interactuar (E) con un contenedor: si pide tirada, tirada enfrentada como `Game::Actuacion`
  (`include/Game/Actuacion.h`); si se abre, se rellena **una sola vez** con la tabla y el presupuesto (semilla fija
  por partida y contenedor) y se enseña en el **papiro del cofre que ya existe** (`m_cofre` en Application; mira
  cómo se usa hoy); lo cogido sale; lo abierto queda abierto. Todo se guarda en la partida (las flags del estado
  narrativo, como ACTUAR-01, o un campo nuevo en `SaveGame`, **solo añadir**, compatible con partidas viejas).
- **Escondites:** al pasar a 2 casillas, tirada de Percepción una vez por escondite; si sale, aviso y se ve.
- Toca lo mínimo de `GameSessionInteraccion.cpp` y `SaveGame*` (zona de Claude): **por ancla**, un trozo pequeño,
  y dilo en el INFORME.
- Pruebas: abrir, con llave, fallar la ganzúa, escondite visto y no visto, recargar no cambia el contenido.
**Hecho cuando:** pruebas en verde y una captura de un cofre abierto en el juego (pon uno de prueba en un interior
en tu rama, y quítalo antes del commit final o déjalo y dilo).

### INFORME AV18-02
- **Estado:** hecho e integrado en main `4b5ae22`: contenedores integrados y 17 pruebas; papiro probado en el juego. La tabla de prueba no pretende ser el reparto de campaña de AV18-03/05.
- **Rama y commits:** `agente/codex/AV18-02`; entrega `7ca2202`. Integrado en main `4b5ae22`; sin push.
- **Qué ha cambiado:** tres fuentes nuevas en motor_level, contrato `Game/Contenedores.h`, bloque ObjectCatalog, persistencia narrativa existente, tiradas de apertura y Percepción, aviso y visibilidad de escondites. Papiro reutilizado con Coger todo/Cerrar. Anclas precisas en la sesión; sin datos nuevos en Application o GameSession. No hay cambios a SaveGame.
- **Cómo lo pruebo (comandos exactos):** desde MotorGraphico-main/MotorGraphico, `cmake --build build-release -j8`; `ctest --test-dir build-release -R 'test_contenedores|test_save_game|test_object_catalog|test_game_session|test_validar_todo' --output-on-failure`; `python3 tools/validar_todo.py` usando Python con NumPy. Escena manual y comandos en FORMATO_CONTENEDORES.md. Primera suite completa: 228/232; los dos fallos de contenido se corrigieron y repitieron en verde; quedan las dos guardias heredadas. Comprobación limpia consignada en la sesión.
- **Qué tengo que mirar yo (Oriol):** en la escena de prueba, E abre, Cerrar deja el botín y Coger todo lo retira. Captura real en el chat: 3 panes/5 monedas; al reabrir, vacío y oro 155. Los fixtures se dejan fuera de los niveles de campaña. `cd` y `percepcion` son dados rivales (0..8). Qwen debe asignar eventId a cada instancia para que sobreviva a una recolocación.
- **Fichero de sesión:** `sesiones/2026-10-15-codex-av18-02-contenedores.md`.

- **Verificación de entrega:** árbol limpio compila juego y test_contenedores; 17 casos, validar_todo 0 errores/22 avisos. Captura del cofre con 3 panes/5 monedas y reapertura vacía en el chat.

---
## 4. AV18-03 — Tablas de botín y objetos
PLAN §4.3. Tras AV18-01 (y con los estados de AV16-14, que hace Claude: quemado, congelado, aturdido, silenciado; si
aún no están, usa los 16 que hay y deja los nuevos marcados).
- 40-60 objetos reutilizables (consumibles, materiales, equipo común y poco común, ropa, tesoros vendibles) y 10-15
  **únicos con efecto táctico** (empujar 2 casillas al rematar, marcar, quemar al golpear…), con el formato de los
  catálogos de `assets/catalogs/` (mira `objects`/`items` reales y sus validadores en `tools/`).
- Tablas por región (calle de Boundington, interiores, cripta, afueras, cueva) y tipo de contenedor.
- `tools/validar_botin.py` (+ prueba): ids existen, presupuestos dentro de ECONOMIA.md, únicos no repetidos.
**Hecho cuando:** validador en verde; `python3 tools/validar_todo.py` sigue en OK.

### INFORME AV18-03
- **Estado:** hecho e integrado en main `4b5ae22`. 50 reutilizables, 12 únicas tácticas y 30 tablas. Se retoma tras cerrar AV16-08, conforme a Oriol; se reutiliza la entrega preparada. Reparto de campaña en AV18-05: cero instancias aún, aviso explícito.
- **Rama y commits:** `agente/codex/AV18-03`; `5ff835d` + `ddab68b` + `b6045e5`. Este último lleva a su propia rama la protección de únicas que ya existía en la cadena AV18-06. Fusionado en main `4b5ae22`; sin push.
- **Cambios:** 14 objetos existentes, 16 materiales/tesoros y 20 puentes a equipo común, sin alterar precios publicados. Doce piezas únicas con linkedSkill nativa prestada al equiparlas al iniciar batalla, sin aprender la carta; quitarlas elimina su carta del siguiente combate. 30 tablas (cinco regiones × seis tipos) y 26 prefabs válidos con presupuestos reales. Se rechaza que un garantizado único no figure en su misma tabla con unico=true: el generador necesita esa marca para reservarlo una sola vez por partida. Contrato documentado en FORMATO_CONTENEDORES.md.
- **Pruebas:** 15 casos Python, cinco C++ (doce efectos tácticos reales, préstamo/restauración y doble ranura, 26 prefabs × 32 semillas = 832 generaciones). Pasan también test_contenedores (17 casos), test_catalog_content y test_validar_todo. Repetido en copia limpia: 15 Python, cinco C++ y validar_todo 0 errores/22 avisos. Sin cambios a C++ en esta revisión; Release de la entrega original ya compilada.
- **Qué mirar Oriol:** nombres y efectos de las doce piezas. Funcionan con los 16 estados actuales; quemado/congelado/aturdido/silenciado siguen marcados como pendientes de Claude. Las únicas son comunes nuevas de 40–60, sin atributos: única no significa rara. Qwen debe colocar con eventId y botin{dia,region,canal,orden}. No se presenta el presupuesto documental como botín ya colocado. PO-49 queda fuera.
- **Sesión:** `sesiones/2026-10-15-codex-av18-03-cierre-tablas.md`; primera implementación en `sesiones/2026-10-15-codex-av18-03-botin.md`.

---
## 5. AV18-04 — Simulador de economía
PLAN §4.4. `tools/simular_economia.py` (+ prueba): con ECONOMIA.md, las tablas y los niveles, simula los tres días
para los tres perfiles (explora todo, solo historia, optimiza) y saca el saldo por día y lo que pudo comprar.
**Hecho cuando:** los tres perfiles dentro de los márgenes de ECONOMIA.md (si no, ajusta presupuestos y dilo).

### INFORME AV18-04
- **Estado:** a medias: recorrido de recompensas implementado y probado; calibración de campaña pendiente de un reparto AV18-05 compatible y de la cobertura/horarios/PO-49. No se declaran cumplidos los tres perfiles.
- **Rama y commits:** `agente/codex/AV18-04`; base `f0a91b7`, continuación **`98369be`**. Esta continuación va aparte de la punta AV18-06; no se ha fusionado, copiado ni aplicado a otras ramas de código. Sin push ni merge.
- **Qué ha cambiado:** `recorrer_recompensas` usa el `NarrativeEngine` real, con decisiones declaradas en `tools/rutas_economia.json`; no duplica el intérprete en Python. Los tres escenarios llegan al epílogo y devuelven beat/opción, oro y objetos. Auditoría completa de 29 efectos económicos (19 grantGold/10 darObjeto) en las seis aventuras del manifiesto; las seis misiones declaran cero premios. El Silbido de Medianoche (+30) queda fuera por no estar en el proyecto. Aventuras, quests y niveles solo leídos, sin cambios.
- **Contabilidad:** costes en su orden real, cama sin doble cobro con reserva_historia, saldo limitado a cero como GameSession y premios narrativos conservados por unidad. El escenario optimiza cobra al carterista (+10) y roba el guardarropa (+40), sin añadir el chantaje excluyente (+30). Las alternativas no tomadas quedan inventariadas, sin fingir cobertura exhaustiva ni óptimo global.
- **Cómo probar:** desde su motor, `cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release`; `cmake --build build-release -j8`; `ctest --test-dir build-release -R '^(test_simular_economia|test_recompensas_economia)$' --output-on-failure` (**13+13 casos**). Suite completa gl-free **234/236**, solo las dos guardias heredadas (49 instantáneos/44 deuda). `python3 tools/simular_economia.py --json`, con `--ajuste -25` y `--ajuste 50`: exit 2 esperado (modelo ejecutado, no calibrado). SHA-256 de aventuras/misiones en la auditoría.
- **Saldos neutrales provisionales:** historia **5/5/5**, explora todo **5/2/2**, optimiza **37/14/6**; cero contenedores nativos y nueve márgenes fuera. Éxitos/victorias impuestos, moral neutral, nivel 1 y sin movimiento/tiempo. Tenderos, oficios, ventas y reservas siguen siendo supuestos declarados; no se bajan márgenes ni se cambian precios para dar verde.
- **AV18-05 ya publicado, contraste solo lectura:** Qwen `da120b1` en `~/onegai-ramas/qwen-AV16-09b`: 17 `loot` fijos y cero contenedores nativos AV18-02/03; `eventId` repetido tres veces en ciudad_centro, interior_armeria y afueras_boundington. Falta tabla/contenedor, botin{dia,region,canal,orden}, identidad por instancia y contraste de los días 2/3. La herramienta lo detecta con `--reparto /Users/admin/onegai-ramas/qwen-AV16-09b/MotorGraphico-main/MotorGraphico`, sin editar/copiar mapas ni fusionar ramas. Su aviso de mínimo escondite 40 está desfasado: ECONOMIA.md vigente ya permite 20.
- **Para recalibrar:** repetir el comando con `--reparto` cuando esté adaptado el contrato; `--rutas` permite revisar decisiones sin tocar aventuras. Guía y tabla de las 29 alternativas: `planificacion_alcance_y_vision/RECORRIDO_RECOMPENSAS_AV18-04.md` en la rama AV18-04. La salida conserva calibrado=false mientras siga abierta esta comprobación de campaña.
- **Sesión:** `sesiones/2026-10-15-codex-av18-04-recompensas.md`.

---
## 6. AV18-06 — Enemigos con más acciones (PO-46)
PLAN §4.6. Oriol: «hay que ampliar las posibilidades y acciones de los enemigos». En `assets/catalogs/monsters.json`
(y donde estén las cartas): cifra de daño en los ataques de monstruo (fórmula por tier y rango; mira
`planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md` §7, que ya las tiene medidas) y 2-3 cartas propias para cada
monstruo que sale en Boundington (lobo, saqueador, sectario, rata, naga…), usando los estados. Validador en verde.

### INFORME AV18-06
- **Estado:** hecho. Revisado el montaje de Claude sin reescribir sus ficheros. Los enemigos usan y alternan las cartas nativas con sus efectos.
- **Rama y commits:** `agente/codex/AV18-06`; datos `3f9ea7e`; documentación de Claude `1d78915`; montaje `ff64e5c` («AV18-06: montaje de Claude — los enemigos juegan su repertorio nativo de cartas»). Este commit apareció durante la verificación, firmado por Oriol con coautoría de Claude; se conserva sin duplicarlo. Sin push ni merge.
- **Qué ha cambiado:** se mantienen las 900 cifras y 31 cartas de la entrega de datos. Claude añade `CartasEnemigas.cpp`, el ancla en `resolverEnemigo`, el cursor por enemigo y el registro CMake de fuente/pruebas. El saqueador alterna Corte sin limpiar y Cuchillada; Gorran alterna Finta, Tajo e impulso, sin repetir el impulso aún activo. Reutiliza objetivo/provocación, reacciones, enfrentadas y DANO-02; `magnitudeHint` sigue siendo referencia editorial.
- **Verificación del 15/10:** `cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release`; `cmake --build build-release -j8` completa; `ctest --test-dir build-release -L gl-free --output-on-failure`: **235/237**. Pasan `test_cartas_combate`, `test_monstruos_acciones`, su validador y `test_validar_todo`. Solo fallan `linea_base_core_configura` (49 instantáneos) y `linea_base_core` (49 instantáneos/44 de deuda), mismos dos rojos que antes del montaje. Ningún fallo observado se atribuye a estos cambios; no se arregla código de Claude.
- **Documentación y límites:** `FORMATO_ACCIONES_MONSTRUOS.md` llegó durante la verificación y se commiteó intacto en `1d78915`; ya explica el montaje hecho. El campo informativo `ejecucion_enemiga` y la salida del validador aún conservan el aviso histórico de IA pendiente: se registra, sin modificarlo en esta revisión. Los cuatro estados nuevos y la colocación de las cinco plantillas siguen fuera de esta entrega.
- **Sesión:** `sesiones/2026-10-15-codex-av18-06-montaje.md`.

---

### Integración de la cola de Codex

La punta `agente/codex/AV18-06` (`0230a33`, código `3f9ea7e`) ya contiene AV18-01/02/03/04/06 y el primer validador de AV16-08. No se reaplican las cinco ramas ancestras. AV18-03 también queda autocontenida en `b6045e5` (sobre `ddab68b`): la guardia de únicas es idéntica a la que ya lleva `3f9ea7e`, por lo que no hay que aplicarla dos veces.

El local completo va aparte: `65365fc` + `43fa13d` de `agente/codex/AV16-08` (el padre `c83c2d9` ya está en la cadena). `3394990` registra su cierre. Conservar en el manifiesto tanto botin/libreria_armas como pesca_caza y su nivel/TMX. Usar el validador de lugares actualizado de `43fa13d`, que permite accesos compartidos sin lista de permisos. La ciudad es Qwen `dfd2ffe`: puerta (56,58), llegada al interior (1,7), salida del local (56,59). No se ha fusionado ninguna rama.

Los informes y TABLERO son coordinación: no reemplazar con versiones históricas las filas de otros agentes. La copia conjunta de Qwen+local compila, enlaza 189 puertas y pasa los siete contratos del local, pero la validación total encuentra cuatro desajustes de variantes PNJ ya presentes en Qwen; detalle en AV16-08. El local aislado y el botín limpio dan validar_todo 0 errores/22 avisos.

La continuación de AV18-04 (`98369be`, sobre `f0a91b7`) se entrega aparte. La punta AV18-06 conserva la versión anterior del simulador: para recoger el recorrido nuevo hace falta incorporar ese commit después de la revisión, sin reaplicar las ramas ancestras. El reparto Qwen `da120b1` ya existe, pero todavía no satisface el contrato nativo; los tres perfiles siguen sin certificar.

PO-48/49 están recogidas en D0019. AV18-06 está hecho en `ff64e5c`: la ejecución y selección de cartas enemigas ya no están pendientes de Claude. AV18-04 continúa con el recorrido de recompensas; los tres perfiles quedan sin certificar hasta contrastar el reparto AV18-05 con las rutas. PO-49 conserva su montaje económico pendiente, independiente de AV18-06.

### Fusión de AV18-02/03 autorizada por Oriol

Fusión autorizada en main desde `b3711aa`, incorporando `agente/codex/AV18-03` (`0175994`), que ya contiene AV18-02, AV18-01 y el validador inicial de AV16-08. Se conserva el trabajo sin commit del árbol principal; no se incluyen AV18-04/06 ni las ramas de Qwen. Sin push.

Release completa y gl-free **232/234** (solo las dos guardias heredadas: 49 instantáneos/44 de deuda). `test_contenedores`, `test_botin_equipo`, `test_validar_botin` y `test_validar_todo` pasan. El reparto AV18-05 sigue necesitando el contrato nativo; esta fusión incorpora motor/tablas, sin fusionar Qwen ni certificar la economía. Registro: `sesiones/2026-10-15-codex-merge-av18-02-03.md`.

**Fusión aplicada:** `4b5ae22` (padres `b3711aa` y `0175994`). main conserva el trabajo local sin commit; índice limpio. El juego de main con WIP compila en build temporal y pasan **8/8**: contenedores, botín/equipo, validadores de botín/lugares, guardado, sesión, objetos y validación total. No se ha hecho push.

---
## 7. AV18-04b — Calibrar con el reparto nativo de Qwen
Plan: `claude/plan_av18_cierre.md` (proyecto). Tu AV18-04 (`98369be`) dejó el simulador listo y nueve márgenes fuera
porque no había instancias compatibles. Qwen entrega `agente/qwen/AV18-05b` con el contrato nativo (prefabs
`botin_<region>_<tipo>`, eventId único, `botin{dia,region,canal,orden}`, tres días).

1. `tools/simular_economia.py --reparto <árbol de AV18-05b>` (solo lectura, como ya lo tienes). Contrato compatible
   = instancias > 0, 0 errores de `validar_botin.py`, eventIds únicos.
2. `rutas_economia.json`: cobertura por perfil según ECONOMIA §6 (historia 60/65/75/0/0; explora todo 100×5;
   optimiza 100×5), y usa `orden` para no comprar con oro de un cofre futuro. Oficios: solo «un servir con éxito al
   día, +8» en optimiza (§6). Nada más por ahora.
3. Palancas para meter los nueve márgenes (3 perfiles × 3 días): cobertura, cestas de §6 y reparto entre instancias
   de la misma región/día/canal (dile a Qwen qué mover, en tu informe). **Prohibido**: precios, presupuestos §4,
   venta 25 %, reventa 50 %.
4. Informe: la tabla 3×3 (perfil × día: saldo, margen, dentro/fuera) antes y después, y por cada margen que no entre
   sin tocar lo prohibido, cuál y por qué (va a la bandeja). Mientras PO-49 no esté en el runtime (Claude, AV18-07),
   el código de salida sigue siendo 2 con el texto «calibrado salvo PO-49»; cuando entre, reejecutas sobre main y
   el 0 cierra AV18-04.
5. Después, cuando los oficios entren en el motor (AV18-09): modélalos como canal «entorno» con techo (recoger: 2
   unidades por franja a precio de material 2–12; fabricar: para usar, no para vender, 25 % si se vende) según
   `claude/oficios/00_reglas_y_materiales.md` §2, y comprueba que optimiza sigue dentro. Eso cierra economía+oficios.
