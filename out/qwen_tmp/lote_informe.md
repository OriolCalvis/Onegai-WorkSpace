# Qwen — lote de microtareas E (28/10)

**Asignado:** 28/10 por Claude (coordinador), a petición de Oriol. **Plan:** «Pla Boundington i motor: S→E» (Claude Docs). **Repo:** `Software/` (raíz git), motor en `MotorGraphico-main/MotorGraphico/` (en este fichero, `MG/`).

Cada tarea (Qn-nn, todas de tamaño E) es un encargo cerrado: un fichero, una cosa, una comprobación. **No hace falta leer nada más del proyecto** para hacerla. Si una E no se puede hacer tal como está escrita, no improvises: escribe en el INFORME `Qn-nn BLOQUEADA: <motivo en una línea>` y pasa a la siguiente.

## Reglas del lote (léelas una vez)

1. **Zona:** Python en `MG/tools/`, datos en `MG/assets/levels/`, `MG/assets/maps/`, `MG/assets/catalogs/rutinas_pnj.json` y `MG/assets/audio/`. **Nada de C++**, nada de `MG/assets/adventures/` (es de Codex).
2. **Una rama por grupo** (`agente/qwen/Q1`, `Q2`…), una E = un commit con el id en el mensaje: `Q1-03: resumen de avisos de puertas`.
3. **Fábrica** (`F=.agents/skills/fabrica/fabrica.py`, desde la raíz):
   - al empezar un grupo: `python3 $F inicio QW-<grupo>` (ej. `QW-Q1`);
   - al cerrar cada E: `python3 $F check` tiene que dar **VERDE**; si da ROJO, `python3 $F depurar <check>` y arreglas solo si el fallo está en tu fichero;
   - al cerrar el grupo: commit y `python3 $F entregar --encargo-md orquestador/qwen_lote_E_28-10.md`.
4. **iCloud:** si al leer un fichero sale `[Errno 35] Resource deadlock avoided`, el fichero no está descargado en el Mac. No es un error tuyo: E BLOQUEADA y sigue.
5. **Casillas:** `(x, y)` como en el JSON del nivel (`position: {x, y}`). «Libre» = pisable en el TMX, sin objeto, sin PNJ, y que no sea casilla de puerta ni de llegada de una puerta.
6. **Marcadores:** si un `objectId` no existe en el catálogo de objetos, se pone igual (es un marcador para Pixel) y se apunta en la lista de marcadores (Q3-15).

---

## Grupo Q1 · Herramientas de control (Python, `MG/tools/`)

**Q1-01 · Clasificador de cambios del árbol (plan: E-001)**
- Fichero: `MG/tools/inventari_arbre.py` (crear).
- Firma: `def classifica(porcelain: str) -> dict[str, list[str]]`.
- Entrada: salida de `git status --short`. Ejemplo: `" M src/Game/AuraMoral.cpp\n?? .claude_tmp/\n"`.
- Salida: `{"codi": ["src/Game/AuraMoral.cpp"], "dades": [], "docs": [], "config_local": [], "brossa": [".claude_tmp/"]}` (las cinco claves siempre, aunque vacías).
- Reglas: `.cpp .h .hpp .cmake CMakeLists.txt` → codi; ruta con `assets/` → dades; `.md` → docs; empieza por `.obsidian/` o `.qwen/` → config_local; empieza por `.papelera_sandbox/` o `.claude_tmp/` → brossa; lo demás → docs. Las rutas entre comillas (git las pone si llevan espacios o acentos) se devuelven sin comillas.
- Prueba: `MG/tools/test_inventari_arbre.py` con una línea de cada categoría, una ruta con espacio y acento entre comillas, y entrada vacía.
- Hecho cuando: `python3 MG/tools/test_inventari_arbre.py` sale con 0.

**Q1-02 · Informe del árbol (plan: E-003)** (depende de Q1-01)
- Fichero: `MG/tools/inventari_arbre.py` (añadir `main`).
- Firma: `def main() -> int`. Ejecuta `git status --short` en la raíz e imprime una tabla `categoría · número · primeros 5 ficheros`. Devuelve 0.
- Hecho cuando: `python3 MG/tools/inventari_arbre.py` imprime las 5 categorías.

**Q1-03 · Resumen de problemas de puertas (plan: E-030)**
- Fichero: `MG/tools/validar_puertas.py` (añadir una función; no tocar `comprueba`).
- Firma: `def resum(probs: list[tuple]) -> dict[str, int]`.
- Entrada: la lista que devuelve `comprueba(raiz)`; cada elemento es `(nivel, objectId, campo, mensaje)`.
- Salida: cuenta por tipo: `{"icloud": n, "sin_vuelta": n, "destino_inexistente": n, "casilla_no_pisable": n, "otro": n}`. `icloud` = el mensaje contiene `Errno 35`; `sin_vuelta` = contiene `no tiene puerta de vuelta`; `destino_inexistente` = contiene `no existe`; `casilla_no_pisable` = contiene `pisable`.
- Prueba: caso nuevo en `MG/tools/test_validar_puertas.py` con 5 tuplas hechas a mano, una de cada tipo.
- Hecho cuando: `python3 MG/tools/test_validar_puertas.py` en verde.

**Q1-04 · Medir las puertas hoy (plan: E-031)** (depende de Q1-03)
- Ejecutar `resum(comprueba(MG))` y pegar el diccionario en el INFORME. Referencia del 28/10: 339 problemas, 338 de iCloud y 1 real (el de Q2-01).

**Q1-05 · Explorador de aventuras: cargar beats (plan: E-060)**
- Fichero: `MG/tools/explorar_aventura.py` (crear).
- Firma: `def carga(ruta: str) -> tuple[list[dict], str]`. Devuelve la lista `beats` del JSON y el `doneFlag` del **último** elemento de `objectives` (es la bandera de «aventura terminada»).
- Formato real: cada beat tiene `id`, `requires` (`allFlags`, `notFlags`, `anyFlags`, `variables`, `lugar`), `effects` y, opcional, `opciones` (cada una con `requires` y `effects`).
- Hecho cuando: `carga("MG/assets/adventures/boundington_precuela_taberna.json")` devuelve 10 beats.

**Q1-06 · Explorador: qué beats se pueden disparar** (depende de Q1-05)
- Firma: `def disponibles(estado: dict, beats: list[dict]) -> list[dict]`. `estado = {"flags": set[str], "vars": dict[str, int]}`.
- Reglas: un beat está disponible si todas sus `allFlags` están, ninguna `notFlags` está, al menos una `anyFlags` está (si la lista existe) y cada condición de `variables` se cumple. **Ignora** `trigger` y `lugar` (se supone que el jugador puede ir a cualquier sitio).
- Prueba: 3 beats hechos a mano, uno de cada clase de condición.

**Q1-07 · Explorador: aplicar efectos** (depende de Q1-05)
- Firma: `def aplica(estado: dict, efectos: list[dict], rng: random.Random) -> None` (modifica `estado`).
- Reglas: `setFlag`/`clearFlag` con `arg`; `setVariable`/`addVariable` con `arg` y `value`; `skillCheck` y `startBattle`: tira `rng.random() < 0.5` y aplica la rama de éxito o la de fallo según `MG/FORMATO_AVENTURAS.md` (busca la palabra `skillCheck` y `startBattle` en ese fichero); cualquier otro tipo se ignora.
- Prueba: un efecto de cada tipo y la misma semilla dos veces da el mismo estado.

**Q1-08 · Explorador: una partida y muchas (plan: E-060)** (depende de Q1-06, Q1-07)
- Firma: `def explora(ruta: str, partidas: int, semilla: int) -> dict`.
- Una partida: desde estado vacío, repite hasta 2000 pasos: toma los disponibles que **no** se hayan disparado ya en esa partida, elige uno al azar, aplica sus `effects` y, si tiene `opciones` disponibles, elige una al azar y aplica las suyas. Termina con «final» si aparece el `doneFlag` final, o con «atasco» si no queda ningún beat disponible.
- Salida: `{"finales": n, "atascos": n, "beats_atasco": {id_ultimo_beat: veces}, "no_visitados": [ids]}`.
- Prueba: aventura mínima en el test con 3 beats y un atasco hecho a propósito → `atascos > 0`.

**Q1-09 · Medir las 5 aventuras (plan: E-062)** (depende de Q1-08)
- `explora(..., partidas=300, semilla=1)` sobre `boundington_prologo`, `_precuela_taberna`, `_primer_dia`, `_segundo_dia` y `_ocaso`. Pegar la tabla en el INFORME. Las aventuras encadenadas empiezan desde las banderas de la anterior: si una sale con 100 % de atascos en el primer paso, anótalo como «necesita estado inicial» y no lo arregles.

**Q1-10 · Entradas de la fábrica (plan: E-064 y E-066)**
- Fichero: `.agents/skills/fabrica/fabrica.json`, lista `checks` (añadir dos entradas, no tocar las demás).
- `{"id": "validar_puertas", "fase": "rapida", "cwd": "{motor}", "cmd": "{python} tools/validar_puertas.py", "timeout": 300}` y lo mismo con `"id": "explorar_aventuras"` y `"cmd": "{python} tools/explorar_aventura.py"` (requiere que Q1-08 tenga un `if __name__ == "__main__"` que ejecute Q1-09 y salga con 1 si hay atascos).
- Hecho cuando: `python3 $F check` ejecuta 4 checks.

---

## Grupo Q2 · Puertas

**Q2-01 · Puerta de vuelta de la mazmorra de ejemplos (plan: E-032)**
- Fichero: `MG/assets/levels/mazmorra_64x64.json` (añadir un objeto).
- Problema: `ejemplos_objetos.json` tiene `ejemplos_puerta` → `mazmorra_64x64.json`, y la mazmorra no tiene puerta de vuelta.
- Dato: un objeto de puerta con `targetLevel: "assets/levels/ejemplos_objetos.json"` y `targetPosition` = la casilla libre al lado de `ejemplos_puerta` en `ejemplos_objetos.json`. Colócalo en la casilla de llegada de `ejemplos_puerta` dentro de la mazmorra (su `targetPosition`) o en la libre más cercana.
- Hecho cuando: `validar_puertas` no lista `ejemplos_puerta`.

---

## Grupo Q3 · Decorado que piden las escenas (AV19-13, partido)

Una E = un objeto o un grupo idéntico. Nada de esto mueve calles, puertas ni PNJ. Comprobación común: `python3 MG/tools/validar_todo.py` sin errores nuevos y `python3 MG/tools/plano_nivel.py <nivel>` para la captura del INFORME.

**Q3-01 · Círculo de piedras de la hoguera de la Barriada** — `ciudad_surysal.json`. 4 objetos `circulo_piedras` en las diagonales de la hoguera (24,9): (23,8), (25,8), (23,10), (25,10). Las ortogonales quedan libres para llegar a la hoguera. Casilla ocupada → sáltala y dilo.

**Q3-02 · Yunque del forjador** — `ciudad_surysal.json`. 1 `yunque` junto a la hoguera-forja (9,10): la primera libre de (8,10), (9,9), (8,9). El forjador está en (10,11): no lo tapes.

**Q3-03 · Cabras** — `ciudad_surysal.json`. 3 `cabra`, en casillas libres a 2–4 casillas de algún `carromato*`, **fuera** de la empalizada (x 21–27, y 5–18), separadas entre sí al menos 2.

**Q3-04 · Tendederos** — `ciudad_surysal.json`. 3 `tendedero_colores`, cada uno en la casilla libre entre dos carromatos que estén a distancia 2 (en línea recta). Si no hay tres pares así, los que haya.

**Q3-05 · Estrado del sermón** — `ciudad_centro.json`. La puerta de la iglesia está en (42,50). 3 objetos `estrado_sermon` en (41,52), (42,52), (43,52). No pises (42,51) (llegada de la puerta). Si alguna no es libre, sube la fila a y=53.

**Q3-06 · Antorchas clavadas** — `ciudad_centro.json`. 4 `antorcha_clavada` en (39,53), (41,54), (43,54), (45,53). Codex las hará visibles solo el día 3: tú solo las colocas.

**Q3-07 · Hueco de la Furia** — `interior_taberna.json`. La barra va de (1,3) a (5,3). 1 `hueco_espada` en (3,2). 1 `furia_en_pared` también en (3,2). Antes, busca con `grep` si algún nivel ya tiene dos objetos en la misma casilla; si ninguno lo tiene, pon `furia_en_pared` en (4,2) y dilo.

**Q3-08 · Tranca de la puerta de la taberna** — `interior_taberna.json`. La salida está en (1,10). 1 `tranca` en (2,10) si es libre; si no, (1,9).

**Q3-09 · Mapa y jarras en la mesa de Venides** — `interior_taberna.json`. Venides en (4,7), mesa en (3,7). 1 `mapa_jarras` en (3,6) si es libre; si no, (2,7).

**Q3-10 · Frascos de cera** — `interior_herreria.json`. Banco en (6,1). 1 `frascos` en (7,1); si está ocupada, (5,1).

**Q3-11 · Armero vacío** — `interior_herreria.json`. `armero_espadas` en (8,1). 1 `armero_vacio` en (8,1) con la misma regla de Q3-07 (si no hay precedente de dos objetos en una casilla, la libre más cercana en la fila y=1 y dilo).

**Q3-12 · Red de Ceaseton descolgada** — `interior_herreria.json`. 1 `red_descolgada` en la primera casilla libre de la fila y=1 con x ≥ 11.

**Q3-13 · Torre del capitán** — `interior_castillo_p2.json`. `mesa_papeles`, `brasero` y `tintero_volcado` en tres casillas libres contiguas de la sala más al norte. Anota las tres casillas en el INFORME (Codex las necesita).

**Q3-14 · Tienda abandonada** — `tienda_abandonada.json`. 3 `maniqui_sin_cabeza`, 1 `espejo_partido`, 2 `baul_abierto`, 4 `polvo`. Ninguno a menos de 2 casillas de la puerta. Lista de casillas en el INFORME.

**Q3-15 · Lista de marcadores para Pixel** — `MG/SPRITE_BACKLOG.md` (añadir al final): una fila por `objectId` nuevo de este grupo que no exista en el catálogo de objetos: `objectId · nivel · tamaño en casillas · una línea de qué es`.

---

## Grupo Q4 · La fiesta dentro de la mansión (AV19-12, partido)

Base: `mansion_barrio_alto.json` (+ su TMX) y `mansion_barrio_alto_p1.json`. Usa la clase `Nivel` de `MG/tools/gen_niveles_av19.py`, como hiciste en AV19-11 (`tools/av19_base_militar.py`). Script nuevo: `MG/tools/av19_mansion.py`; cada E añade una función y la llama desde `main`, así el nivel se regenera entero y es idempotente.

**Q4-01 · Lienzo 32×28** — función `lienzo(n: Nivel)`: suelo de interior, muro perimetral, y las puertas que hoy tiene `mansion_barrio_alto.json` con su mismo `objectId` y destino (léelas antes y lístalas en el INFORME). Hecho cuando: `validar_puertas` no da problemas nuevos.

**Q4-02 · Salón con damero** — función `salon(n)`: zona de 14×10 en el centro con suelo alternado claro/oscuro (`marmol_real` o las dos losas del tileset; dilo). Casillas libres para 30 invitados: no coloques ningún objeto en 20 de sus casillas.

**Q4-03 · Escalera doble** — función `escaleras(n)`: dos `escalera_mansion_p0_sube` simétricas en el lado norte del salón, que lleven a `mansion_barrio_alto_p1.json`.

**Q4-04 · Chimenea y cuarteto** — función `chimenea(n)`: una chimenea en la pared este del salón; reserva 4 casillas libres delante (cuarteto) y 1 al lado (el noble que brinda). Lista de casillas en el INFORME.

**Q4-05 · Guardarropa** — función `guardarropa(n)`: cuarto de 4×3 junto a la entrada, con 2 percheros y un mostrador.

**Q4-06 · Jardín** — función `jardin(n)`: franja sur de 32×6 con setos recortados en dos filas, una fuente central y 6 faroles de papel (`farol_papel`). Casillas libres para 10 invitados.

**Q4-07 · Verja dorada** — función `verja(n)`: `verja_dorada` cerrando el jardín por el sur, con la puerta de entrada desde `ciudad_este` en el centro (misma `targetPosition` que la entrada actual).

**Q4-08 · Ventana para trepar** — mueve `salida_ventana_mansion` a una pared del jardín alcanzable desde una casilla pisable del jardín. Hecho cuando: `conectividad.py` llega a ella desde la verja.

**Q4-09 · Planta alta: pasillo y despacho** — `mansion_barrio_alto_p1.json`: pasillo con 4 puertas cerradas (objetos decorativos, sin destino) y el despacho con `escritorio_patas_leon`, `atril` y `cofre`.

**Q4-10 · Ventana de huida de la planta alta** — `mansion_barrio_alto_p1.json`: una salida por ventana al jardín de la planta baja (destino: una casilla libre del jardín de Q4-06).

**Q4-11 · Farolas de la calle de la mansión** — `ciudad_este.json`: una `farola` en cada esquina libre de las calles que tocan la mansión (dirección visual §4 de `claude/boundington_direccion_visual.md`: «luz con intención»). Lista en el INFORME.

**Q4-12 · Ventanas con luz** — `ciudad_este.json`: `ventana_luz` en las fachadas de las 6 casas más cercanas a la mansión.

**Q4-13 · Tabla del contrato con Codex** — en el INFORME: id del nivel · tamaño · puertas (objectId, casilla, destino) · casillas reservadas (anfitriones, cuarteto, noble, mercaderes en la escalera, invitados) · qué es de día y qué de noche.

---

## Grupo Q5 · Rutinas del Ocaso (AV19-14, partido)

**Requisito:** que Oriol o Claude hayan fusionado tus ramas `agente/qwen/AV19-09`, `AV19-10` y `AV19-11` (hechas el 20–21/10, aún sin fusionar el 28/10). Sin eso, todo el grupo BLOQUEADO.
Fichero de todas: `MG/assets/catalogs/rutinas_pnj.json`. Comprobación de todas: `python3 MG/tools/validar_rutinas_pnj.py` en verde y ninguna ronda pisa fuego ni barricada.

**Q5-01 · Perdidos del Casco** — `ciudad_centro_ocaso`: 3 grupos de 2 Perdidos con `patrulla` en línea recta por las calles de la ruta forzada de Oriol, cada ronda de 6–10 casillas.
**Q5-02 · Perdidos de las Afueras** — `afueras_ocaso`: 2 grupos de 3 con `patrulla`.
**Q5-03 · Saqueadores que esperan** — `base_militar`: los 3 saqueadores de la muralla trasera con `esperar`, fuera del anillo de muralla.
**Q5-04 · Gente tirada** — `afueras_ocaso`: 6 casillas con orden `tumba` (Codex pone los cuerpos): 2 en el molino, 2 en los huertos, 2 en la calle de la barricada.
**Q5-05 · Guardia de la Barriada de noche** — `ciudad_surysal`: 4 guerreros con `vigilar` junto a la empalizada (x 21–27, y 5–18), uno por lado.

---

## Grupo Q6 · Sonido (el juego tiene hoy cero ficheros de audio)

El catálogo de sonidos ya existe en `MG/src/Game/AudioDirector.cpp` (líneas 41–61) y pide exactamente estos 11 ficheros, con ruta relativa a `MG/assets/audio/`. Cada E = un fichero.

Reglas comunes: licencia **CC0** (o dominio público) y nada más; música en OGG Vorbis estéreo 44,1 kHz, entre 60 y 180 s, que haga bucle sin corte; efectos en WAV mono 16 bits 44,1 kHz, menos de 2 s; cada fichero añade una fila a `MG/assets/audio/LICENCIAS.md`: `fichero · autor · URL de origen · licencia · fecha de descarga`. Sin URL y licencia escritas, la E no está hecha.

**Q6-01** `musica/titulo.ogg` — tranquila, fantasía medieval.
**Q6-02** `musica/exploracion.ogg` — ciudad medieval, ritmo bajo.
**Q6-03** `musica/combate.ogg` — percusión, tensa.
**Q6-04** `musica/tienda.ogg` — ligera, cuerda pulsada.
**Q6-05** `musica/derrota.ogg` — lenta, menor.
**Q6-06** `sfx/interfaz.wav` — clic corto de madera o papel.
**Q6-07** `sfx/paso.wav` — un paso sobre piedra.
**Q6-08** `sfx/golpe.wav` — golpe de espada contra cuero.
**Q6-09** `sfx/cura.wav` — campanilla suave.
**Q6-10** `sfx/dado.wav` — un dado sobre madera.
**Q6-11** `sfx/victoria.wav` — fanfarria corta (≤ 2 s).

**Q6-12 · Validador de audio (plan: E-124)**
- Fichero: `MG/tools/validar_audio.py` (crear).
- Firma: `def comprueba(motor: Path) -> list[str]`: lee las rutas `"musica/…ogg"` y `"sfx/…wav"` de `src/Game/AudioDirector.cpp` con una regex y devuelve los problemas: fichero que no existe en `assets/audio/`, o que no tiene fila en `LICENCIAS.md`.
- Prueba: `MG/tools/test_validar_audio.py` con una carpeta temporal con 1 fichero bien, 1 sin licencia y 1 que falta.
- Hecho cuando: con Q6-01…Q6-11 hechas, `python3 MG/tools/validar_audio.py` sale con 0, y está añadido a `fabrica.json` como en Q1-10.

---

## Orden

Q1 → Q2 → Q3 → Q6 → Q4 → Q5. Q1 primero porque sus validadores comprueban el resto. Q5 espera la fusión de tus ramas AV19.

## INFORMES (una línea por E, aquí debajo)

`Qn-nn · hecha | BLOQUEADA · commit · lo que haya que saber en una línea`

Rama de trabajo: `agente/qwen/Q1` → `Q2` → `Q3` → `Q6` (apiladas, en ese orden). Ejecutado el 28/10 por Claude. Los commits se hicieron con comandos de git de bajo nivel (`write-tree`/`commit-tree`) porque `git commit` da *Bus error* con los ficheros de iCloud sin descargar.

**Q1**
- Q1-01 · hecha · dede330 · `classifica` + `test_inventari_arbre.py` (0).
- Q1-02 · hecha · dede330 · `main` imprime las 5 categorías.
- Q1-03 · hecha a medias · 045d73e · `resum` hecha y comprobada a mano; el caso nuevo en `test_validar_puertas.py` BLOQUEADA: el fichero está sin descargar de iCloud (Errno 35).
- Q1-04 · hecha · — · `resum(comprueba(MG))` = `{"icloud": 338, "sin_vuelta": 1, "destino_inexistente": 0, "casilla_no_pisable": 0, "otro": 0}` (339, igual que la referencia). Tras Q2-01: 338 y sin_vuelta 0.
- Q1-05..Q1-08 · hecha · 1b3ecdc · `explorar_aventura.py` + `test_explorar_aventura.py` (5 pruebas en verde). El tope de 2000 pasos cuenta como atasco.
- Q1-09 · hecha · 1b3ecdc · 300 partidas, semilla 1:

| aventura | finales | atascos | no visitados | nota |
|---|---|---|---|---|
| prologo | 0 | 300 | 58 | todos paran en `prologo_caida` (sin `trigger` el primer beat no abre la cadena) |
| precuela_taberna | 300 | 0 | 1 | |
| primer_dia | 29 | 271 | 40 | atascos en `b1_pulga_amigo`, `b1_carterista_robo`, `rec_fuego_regalo` |
| segundo_dia | 0 | 300 | 180 | necesita estado inicial |
| ocaso | 0 | 300 | 96 | necesita estado inicial |

- Q1-10 · hecha · 1b3ecdc · `validar_puertas` y `explorar_aventuras` en `fabrica.json`. Los dos salen con código 1 hoy (338 avisos de iCloud; atascos), así que cuentan como ROJO nuevo: volví a tomar la línea base (`inicio --rapida`) y entonces `check` da VERDE con 4 heredados. Ejecuta 4 checks en fase rápida.

**Q2**
- Q2-01 · hecha · d2109c6 · puerta `ejemplos_puerta` en (6,19) de la mazmorra → `ejemplos_objetos.json` (5,4). Uso `ejemplos_puerta` porque es la única ficha de puerta del proyecto `ejemplos` (su nombre dice «Ir a la mazmorra de prueba»; conviene una ficha de salida). `validar_puertas` ya no la lista.

**Q3** (la comprobación de libres no vio los catálogos `mundi_lugares`, `on_enemigos`, `vacio_objetos`, sin descargar; por seguridad conté como ocupada toda casilla con objeto)
- Q3-01 · hecha · 9d77c2a · 4 `circulo_piedras` en (23,8) (25,8) (23,10) (25,10).
- Q3-02 · hecha · 9d77c2a · `yunque` en (8,10).
- Q3-03 · hecha · 9d77c2a · 3 `cabra`: (10,10), (3,9), (8,5). La de (10,10) queda junto a la hoguera-forja y al forjador; muévela si no te gusta.
- Q3-04 · hecha · 9d77c2a · 3 `tendedero_colores` (hay exactamente 3 pares a distancia 2): (5,10), (13,24), (4,37).
- Q3-05 · BLOQUEADA: (42,51)…(43,53) y toda la fila y=53 son edificio de la iglesia (colisión en el TMX); la calle libre delante de la puerta (42,50) está al oeste, x ≤ 41.
- Q3-06 · BLOQUEADA: solo (39,53) es libre; (41,54), (43,54) y (45,53) caen dentro del edificio. Va con Q3-05.
- Q3-07 · hecha · a6ed4b2 · `hueco_espada` y `furia_en_pared` ambos en (3,2); sí hay precedente de dos objetos en una casilla (`prado_del_rio` (14,0), `tienda_abandonada` (8,1), `mansion_barrio_alto` (11,1)).
- Q3-08 · hecha · a6ed4b2 · `tranca` en (2,10).
- Q3-09 · BLOQUEADA: (3,6) y (2,7) son taburetes.
- Q3-10 · hecha · 4c9692c · `frascos` en (7,1).
- Q3-11 · hecha · 4c9692c · `armero_vacio` en (8,1), encima de `armero_espadas` (hay precedente).
- Q3-12 · hecha · 4c9692c · `red_descolgada` en (11,1).
- Q3-13 · BLOQUEADA: `interior_castillo_p2.json` sin descargar de iCloud (Errno 35).
- Q3-14 · hecha · 0d93bc7 · `maniqui_sin_cabeza` (3,3) (7,3) (10,3); `espejo_partido` (5,1); `baul_abierto` (4,8) (9,8); `polvo` (5,2) (8,5) (2,7) (11,5). Puerta en (1,5).
- Q3-15 · hecha · 0383a4a · 13 filas añadidas a `SPRITE_BACKLOG.md` (`yunque` ya tiene ficha). `validar_todo.py` revienta con iCloud (ya en la línea base), así que la comprobación común fue `fabrica check` en VERDE; no hay capturas de `plano_nivel.py`.

**Q6**
- Q6-01..Q6-11 · BLOQUEADAS: sin acceso de red a bancos de audio CC0 (opengameart.org y freesound.org no responden desde el entorno).
- Q6-12 · hecha · 2a120ec · `validar_audio.py` + `test_validar_audio.py` (2 pruebas) + entrada en `fabrica.json`. Hoy da 22 problemas (los 11 ficheros × falta/licencia), por lo que sale con 1 hasta que haya audio.

**Q4**
- Q4-01..Q4-13 · BLOQUEADAS: la rama `agente/qwen/AV19-12` (3f8a02e, «la fiesta dentro de la mansión 32×28») ya tiene la mansión hecha, con 240 objetos, y sigue sin fusionar; rehacerla desde el 22×16 de HEAD chocaría con ella. Además Q4-09/10 necesitan `mansion_barrio_alto_p1.tmx`, sin descargar de iCloud. Decisión de Oriol: fusionar AV19-12 y repartir Q4 sobre ella.

**Q5**
- Q5-01..Q5-05 · BLOQUEADAS: las ramas `agente/qwen/AV19-09`, `AV19-10` y `AV19-11` no están fusionadas en HEAD.

**Actualización 28/10 (mañana, tras bajar iCloud)**
- Q1-03 · hecha · e38d19f · prueba `ResumDeProblemas` en `test_validar_puertas.py` (20 pruebas en verde).
- Q3-13 · hecha · d840b41 · `mesa_papeles` (9,3), `brasero` (10,3), `tintero_volcado` (11,3) en la sala este del norte (la de armario, tocador y espejo); `mesa_papeles` y `tintero_volcado` añadidos a `SPRITE_BACKLOG.md`.
- `validar_puertas` sin iCloud: 0 de iCloud y 11 reales (10 llegadas bloqueadas por un PNJ en `mundi_landmass_*`; 1 `recuerdo_taberna` → `interior_taberna_p1` sin vuelta). Q2-01 sigue bien. Línea base de la fábrica retomada.
- Siguen sin bajar muchos ficheros (`BRANCHING.md`, `Claude outputs/*.png`, `assets/proyectos/index.json`…) y 56 objetos de `.git`; `validar_todo` y `fabrica entregar` siguen sin poder correr.

---

## Actualització 28/10 (tarda, Claude): Git recuperat, unió de línies, portes, Q4 i Q5

**Git:** objectes d'iCloud baixats; `git fsck` net. Còpia a `~/dev/Software` pendent (2,9 GB lliures al Mac; cal ≥6 GB i crear `~/dev`).

**Branques:** `agente/qwen/integracion-av19-28-10` (AV19-09→11→10→12 sobre el lot E; canvis locals d'Oriol al commit 3d0ca3d) i **`claude/unificacion-28-10`** = tot això sobre `codex/revision-reciente-20261026` (decisió d'Oriol). Ocaso: el d'Oriol (de Qwen no entren `afueras_ocaso` ni `av19_ocaso.py`). Mansió: la 32×28. Base militar: entre `ciudad_oeste` i `interior_cuartel`.

**Fet:**
- Portes: 10 arribades de `mundi_landmass_*` dues caselles al sud del PNJ (8d2223b). `validar_puertas` a la línia unificada: 3 problemes (abans 14; queden `recuerdo_taberna` i dos de Gorran que ja hi eren).
- Q3: fitxes de catàleg per als 17 marcadors (0d853ad). Q3-05/06 retirats de `ciudad_centro`: l'església de revision-reciente és a un altre lloc (porta 56,52). Pendent de nova ubicació.
- Q4 sobre la 32×28: festa del dia 2 traslladada (convidats, músics, criats), jardí connectat amb la casa (pas a 20,19), 7 cinemàtiques `fiesta_*` amb coordenades noves, sortida per la finestra a (4,13).
- Q4-13 (contracte): `mansion_barrio_alto` 32×28, entrada (2,25).
  - Portes: `salida_mansion_barrio_alto` (1,25) → ciudad_este (7,17) · `salida_cocina_mansion` (20,5) → (7,18) · `salida_ventana_mansion` (21,8) → (4,13) · `escalera_mansion_p0_sube` (17,22)/(18,22) → p1 (11,7)/(14,7).
  - Caselles reservades: cuartet (8..11,3) · seguidor_brinda (5..7,2), al costat de la xemeneia (6,0) · anfitriona (6,25) i mercader (6,26) · mercaders a l'escala (15,16,19 × 23,24) · nobles en escaquer (2..12 × 4..8) · criats (12,10) (3,11) (9,17) i a dalt (18,6).
  - Planta alta 22×12: escala (12,7)/(13,7) → p0 (17,23)/(18,23) · `ventana_huida_mansion` (20,6) → jardí (21,9).
  - Q4-11/12 (fanals i finestres il·luminades): ja els va posar AV19-12 (26 `farola`, 37 `ventana_luz`); n'entren 58 de 63, la resta descartats perquè la casella ha canviat.
- Q5-01: revisades les 23 rondes del Casco; la del `perdido_devoto` (56,41) travessava foc: ara arriba fins a x 65.

**Pendent / per decidir amb Oriol:**
- Q3-05/06: on van l'estrado i les torxes davant de l'església nova, on Codex ja hi té l'escena de l'estrado.
- Q5-01: la ronda `perdido_fanatico` (50,51)→(56,51) topa amb `verina_segunda` i `sectario_portal`.
- Q5-02/04: eren per a `afueras_ocaso`, que ja no hi és. Q5-03: la base militar no té saquejadors. Q5-05: la Barriada té 2 guerrers i en caldrien 4. Tal com estan escrites, Q5 demanen rutines (`patrulla`, `esperar`, `tumba`) que `rutinas_pnj.json` no admet: són ordres de cinemàtica i, per als enemics, `patrolMin/patrolMax`.
- `recuerdo_taberna` (escala) i els 10 `playerStart` bloquejats: no autoritzats.
- Q6-01..11: el proxy rebutja opengameart.org, freesound.org i kenney.nl (403 de política, des del núvol i des del Mac). Cal ampliar la llista de dominis o que Oriol en baixi els fitxers.
- `fabrica entregar` i la validació completa es faran a la còpia `~/dev`.

**Validació de `claude/unificacion-28-10`** (exportació sense imatges ni àudio): `validar_todo` 1 error (`validar_botin`, el mateix que revision-reciente), cinemàtiques 0 errors, `validar_pnj` OK, `conectividad` OK, `validar_mundo_unido` OK, `validar_rutinas_pnj` amb els mateixos 37 errors que revision-reciente, tests d'eines del lot OK.
