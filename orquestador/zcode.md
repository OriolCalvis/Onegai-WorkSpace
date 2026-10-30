# Z.AI (ZCode) — cola de tareas del 15/10
**Asignada:** 15/10 por Claude (coordinador), a petición de Oriol: «planifícalo al máximo detalle para que ChatGPT, Z.AI y Qwen me terminen el trabajo» · **Repo:** `MotorGraphico-main` · **Tipo:** trabajo nuevo, una rama por tarea.
Lo anterior de este fichero está en `orquestador/historial/zcode.md`.

> **15/10 (ZCode): cola reanudada por orden de Oriol («continua»).** AV17-01 hecho (abajo); sigo con AV17-02.
> Nota: empecé con una rama base fotografando el índice staged mientras faltaba §0; cuando el repo se unificó
> (`main` nuevo en `Software/`), recreé la rama desde ahí y las ediciones encajaron limpias.

Tu parte: **la historia con teatro** (PLAN_15-10.md §3). Trabajas sobre los JSON de las aventuras (`assets/adventures/`, formato en `FORMATO_AVENTURAS.md`) y pueblas los interiores de tus escenas. **No tocas C++**: si necesitas una orden de cinemática que no existe, la pides en tu INFORME y en la BANDEJA (la hace Claude, AV17-10) y sigues con lo demás.

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

**Tu zona (no salgas de ella):** `assets/adventures/*.json`, `assets/catalogs/quests.json` (solo las fichas de tus misiones), `FORMATO_AVENTURAS.md` (secciones nuevas) y **poblar** los interiores de tus escenas (cripta, castillo, tienda abandonada, mansión: solo objetos/PNJ, no el suelo). **No** toques C++, `ciudad_centro.json`, los generadores ni otros niveles (Qwen).

---
## 1. AV17-01 — Segundas oportunidades
PLAN §3.1. Oriol: «si fallas la tirada se termina todo… me gustaría más oportunidades… con Luisarda y el herrero
es imposible hablar… como mínimo conseguir que vaya a cenar a la taberna».
1. Haz una **tabla de todas las tiradas** de las aventuras de Boundington (`boundington_*.json`): beat, tirada, qué
   abre el éxito, qué pasa al fallar hoy.
2. Para cada tirada que abre una vía de la historia, **tres salidas**:
   - éxito: lo de hoy;
   - fallo → **el PNJ ofrece otra cosa**: un favor («tráeme hilo de plata de La Perla»), otro precio, otra hora
     («vuelve esta noche»), con su objetivo en el diario y su `donde` (formato `["assets/levels/X.json","objectId"]`);
   - fallo otra vez → **una vía de explorar**: encontrar una prueba, escuchar una conversación, colarse.
3. Imprescindibles: Luisarda (que acepte ir a cenar a la Taberna Humilde), Aigren (la espada), el capitán, y las de la
   cripta y la fiesta (las rehaces en AV17-03 y AV17-06).
4. Comprueba: `python3 tools/validar_todo.py`, `python3 tools/medir_objetivos.py --solo-boundington --exigir`
   (todo objetivo con destino) y juega cada vía fallando a propósito (si no hay forma de forzar una tirada, dilo en el INFORME y prueba con varias partidas).
**Hecho cuando:** la tabla en tu sesión con las tres salidas de cada tirada; ninguna misión queda imposible.

### INFORME AV17-01
- **Estado:** **hecho** (Luisarda, Aigren y capitán con las tres salidas; cripta y fiesta quedan para
  AV17-03/AV17-06 como manda esta ficha; ninguna misión de Boundington queda imposible al fallar una tirada).
- **Rama y commits:** `agente/zcode/AV17-01` (worktree `~/onegai-ramas/zcode-AV17-01`), sobre el `main`
  unificado `0dbaee6`. Commit «AV17-01: segundas oportunidades — tres salidas por tirada…» con
  `boundington_primer_dia.json` (+10 beats, +2 objetivos) y `boundington_segundo_dia.json` (+4 beats,
  +1 objetivo). Sin C++, sin catálogos ajenos, sin niveles.
- **Qué ha cambiado:**
  - **Luisarda**: al fallar ya no cierra la tienda: ofrece cobrarle al sastre el hilo de plata (favor, en la
    misma sastrería) o que vuelvas de noche; de noche se oye a Luisarda y el sastre hablar de Sandes, y con
    eso también va a cenar con Rosita. La cena es alcanzable por las tres vías.
  - **Aigren**: pifia con su hijo → el envío nocturno (cargas el carro, te cuenta lo del líder y las espadas);
  fallo con los Perdidos → «papel»: una orden del capitán con sello (día 2) que le entrega las espadas a la
  guardia y te da su daga; y de noche, entrar en la herrería muestra a las capas grises cargando espadas.
  - **Capitán**: tras el «no», volver al castillo con una prueba (cualquier pista) o con Venides respondiendo
  por ti (afecto ≥ 1) también prepara a la guardia. El rechazo deja pistá en el registro.
  - **Gorran**: derrota o huida ya no cierran la cueva para siempre (la misión de la espada revivió).
  - **Diario**: 4 objetivos opcionales nuevos en las fichas de `quests.json` con `visibleFlag` — solo se ven
  al entrar en la vía secundaria (hilo del sastre / envío de Aigren / orden del capitán / prueba para el
  capitán), cada uno con su `donde`. Actos reutilizados, `actos.json` intacto.
- **Cómo lo pruebo (comandos exactos):** desde el worktree (`~/onegai-ramas/zcode-AV17-01/MotorGraphico-main/MotorGraphico`):
  `python3 tools/medir_objetivos.py --solo-boundington --exigir` (30/30), `python3 tools/validar_todo.py`
  (Aventuras OK; el FAIL es `validar_pnj` muriendo en `numpy`, preexistente), `ctest --test-dir
  build-release -L gl-free` (4 rojos preexistentes: 2×numpy, 2×`linea_base_core` D0007),
  `./build-release/juego 90` (exit 0, sin «NO cargo»). Las vías de fallo verificadas con una simulación del
  motor (14 rutas + regresiones de los éxitos): la tabla completa de tiradas y los detalles, en la sesión.
- **Qué tengo que mirar yo (Oriol):**
  1. **No hay forma de forzar una tirada en el juego**: si quieres probar a mano las vías de fallo, hace
     falta un modo de depuración («forzar grado de la tirada») — que lo añada Claude. Mientras tanto está la
     simulación (14 rutas, en la sesión §3).
  2. Los objetivos de ruta secundaria van en las **fichas** (`quests.json`, con `visibleFlag`) porque los de
     la aventura no admiten condiciones y el test del diario los exige 1:1 con los apartados.
  3. El «éxitos 4» (Luisarda sin recado, Aigren-Perdidos) es canon tuyo (BND-ALT-01) y **no lo he bajado**;
     con las tres salidas, fallarlo ya no cierra nada.
  4. El envío nocturno de Aigren solo vale la noche del día 1; el día 2 lo cubre la vía de la orden.
- **Fichero de sesión:** [sesiones/2026-10-15-zcode-av17-01-segundas-oportunidades.md](../sesiones/2026-10-15-zcode-av17-01-segundas-oportunidades.md) (con la tabla de todas las tiradas) · el bloqueo previo, en [sesiones/2026-10-15-zcode-av17-cola-bloqueada.md](../sesiones/2026-10-15-zcode-av17-cola-bloqueada.md)

---
## 2. AV17-02 — Pasar el tiempo y el día 3 entero
PLAN §3.2. Oriol: «en la taberna puedes pasar el tiempo; añadiría, aparte de la noche, adelantar a mediodía y tarde»;
«el tercer día debo tener el día entero para terminar misiones, no empezar directamente por la noche».
- Busca cómo hace hoy «Esperar a que anochezca» (diálogo de Rosita en la taberna, captura del 15/10 10:32:51) y qué
  orden cambia la hora; añade **mediodía, tarde y noche** (y en la posada, Taberna Rica, igual). Si el motor solo
  sabe saltar a la noche, pide la orden en el INFORME/BANDEJA y deja los textos listos.
- Día 3 (`boundington_ocaso.json` y el paso del día 2 al 3): empieza **por la mañana**; el ocaso empieza al elegirlo
  (hablar con alguien) o al caer la noche; avisa al jugador de que el ocaso se acerca.

### INFORME AV17-02
- **Estado:** **hecho** (mediodía pendiente de motor: PO-50; mediodía≈«comer algo» mientras tanto).
- **Rama y commits:** `agente/zcode/AV17-01`, commit `26f66fa` (encima del `8aa86c6` de AV17-01; son
  commits separados, se pueden integrar por separado). Tres ficheros: `boundington_primer_dia.json`,
  `boundington_segundo_dia.json` y `boundington_ocaso.json`.
- **Qué ha cambiado:**
  - **Rosita (días 1 y 2)**: sus esperas ahora se conditionan por franja — «Comer algo…» solo por la
    mañana (pasa a la tarde), «Esperar a que anochezca» hasta la tarde, «Subir a dormir» desde la tarde.
    Máximo 4 opciones visibles (límite duro del motor: `kMaxOpciones`).
  - **Posadero nuevo (Taberna Rica, días 1 y 2)**: «Esperar a la tarde» (gratis), «Esperar a que
    anochezca» y «Subir a dormir» (encierra la flag del día y cierra como dormir en la taberna).
  - **Día 3**: sin plan ya **no** se lo lleva la noche de golpe — `b3_dia_pasa` solo dispara al caer el
    sol (franja noche); nuevo aviso `b3_tarde_aviso` («El Ocaso se acerca…» + registro) al llegar la tarde
    del día 3; y la Rosita del día 3 permite «Esperar a la tarde» y «Esperar a que anochezca» (elegir el
    momento). Con plan, seguirá empezando al avisar a Venides o llegar a la Base, como hoy.
- **Cómo lo pruebo (comandos exactos):** desde el worktree: `./build-release/test_diario_completo`
  (5/5), `python3 tools/medir_objetivos.py --solo-boundington --exigir` (31/31),
  `./build-release/juego 60` (exit 0). Flujos de tiempo verificados por simulación: mañana→tarde→noche en
  taberna y posada, día 3 jugable de mañana con aviso de tarde y ocaso solo al anochecer (detalle en la
  sesión). ctest gl-free: solo los rojos preexistentes (numpy ×2, `linea_base_core` ×2).
- **Qué tengo que mirar yo (Oriol):**
  1. **PO-50 en la bandeja**: la franja «mediodía» necesita una orden del motor (Claude); mientras
     tanto, «comer algo» hace de mediodía.
  2. Dormir ya solo se ofrece desde la tarde (antes se podía de mañana y te saltabas el día entero).
  3. Nota de proceso: perdí media hora porque los menús de 5 opciones los rechaza el cargador en
     silencio para el validador de Python (todo-o-nada); ya está documentado en la sesión.
- **Fichero de sesión:** [sesiones/2026-10-15-zcode-av17-02-tiempo-y-dia-3.md](../sesiones/2026-10-15-zcode-av17-02-tiempo-y-dia-3.md)

---
## 3. AV17-03 — La cripta bajo la iglesia, jugable
PLAN §3.3. Hoy es un texto (captura 10:30:58, «Cripta de los Perdidos»). Hazla escena:
- **Personajes en el nivel** (pon sus objetos en el JSON de la cripta: sectarios, **Edul y Verina**), que se mueven y
  hablan con las órdenes de cinemática que existen (mira `FORMATO_AVENTURAS.md`; si falta algo, pídelo).
- Tiradas con consecuencias: **engañar** a los sectarios (Engaño/Persuasión), **unirte** a ellos (abre una rama: anótala
  como decisión del diario, D0016), **escuchar** el ritual (Percepción), **escapar** (Atletismo/Sigilo), y qué pasa si
  te descubren (combate o huida). Cada una con sus tres salidas (AV17-01).
**Hecho cuando:** al menos 4 tiradas con salidas distintas, personajes que se mueven, y unirse abre la rama.

### INFORME AV17-03
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:**
- **Qué ha cambiado:**
- **Cómo lo pruebo (comandos exactos):**
- **Qué tengo que mirar yo (Oriol):**
- **Fichero de sesión:**

---
## 4. AV17-04 — El castillo con personajes
PLAN §3.4. «La historia del castillo también debería tener los personajes ahí, pon un poco de cinemática». El capitán y
la guardia en la sala; al entrar, se giran, uno se acerca; al salir, te acompañan a la puerta.

### INFORME AV17-04
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:**
- **Qué ha cambiado:**
- **Cómo lo pruebo (comandos exactos):**
- **Qué tengo que mirar yo (Oriol):**
- **Fichero de sesión:**

---
## 5. AV17-08 — Teatralidad: guía y las 10 escenas
PLAN §3.8. Escribe la **guía de escena** (sección nueva en `FORMATO_AVENTURAS.md`): entrar, girarse, acercarse, pausas,
bocadillos, cámara; con ejemplos JSON. Aplícala a las 10 escenas principales de Boundington (lista en tu sesión, con
qué tenía antes y qué tiene ahora). Ninguna escena principal solo con texto.

### INFORME AV17-08
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:**
- **Qué ha cambiado:**
- **Cómo lo pruebo (comandos exactos):**
- **Qué tengo que mirar yo (Oriol):**
- **Fichero de sesión:**

---
## 6. AV17-05 — El gato en el árbol
PLAN §3.5. **Necesita la orden «trepar» (AV17-10, Claude).** Si no está, déjala para el final. El héroe va al árbol más
cercano, trepa, coge al gato y baja (captura 10:31:37: hoy solo un texto).

### INFORME AV17-05
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:**
- **Qué ha cambiado:**
- **Cómo lo pruebo (comandos exactos):**
- **Qué tengo que mirar yo (Oriol):**
- **Fichero de sesión:**

---
## 7. AV17-06 — La fiesta del Barrio Alto, jugable
PLAN §3.6. **Necesita los niveles de Qwen (AV19-05: tienda abandonada y mansión).** Cadena:
1. Mañana del día 2: **Xila** delante de la taberna (solo si os hicisteis amigos el día 1; si no, otra vía: un rumor
   en la taberna) te cuenta la fiesta y te lleva a la **tienda abandonada**.
2. La tienda: colarte (tiradas: forzar la puerta, trepar por detrás, distraer al vigilante), investigar y explorar,
   encontrar **trajes de lujo** (objeto de misión).
3. Noche: **la fiesta jugable** en la mansión: moverte entre invitados, hablar, escuchar, espiar, robar o salir; con
   el traje puesto entras; sin él, por la ventana (lo de hoy, captura 10:33:21).
Todo con las tres salidas de AV17-01.

### INFORME AV17-06
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:**
- **Qué ha cambiado:**
- **Cómo lo pruebo (comandos exactos):**
- **Qué tengo que mirar yo (Oriol):**
- **Fichero de sesión:**

---
## 8. AV17-07 — El ocaso en llamas
PLAN §3.7. **Necesita la ciudad del ocaso (Qwen, AV19-06) y el fuego (Claude, AV17-11).** Perdidos patrullando, las rutas
que no convienen en llamas, casas ardiendo, gente muerta; la huida se juega por la ciudad. Beats, textos y quién está
dónde; el contraste con el día lo es todo.

### INFORME AV17-07
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:**
- **Qué ha cambiado:**
- **Cómo lo pruebo (comandos exactos):**
- **Qué tengo que mirar yo (Oriol):**
- **Fichero de sesión:**

---

### INFORME AV18-08 (recompensas y alternativas de escenas — hecho por Claude, 15/10, a petición de Oriol)
- **Estado:** hecho (sin poder correr `recorrer_recompensas` de C++ por falta de cmake).
- **Rama y commits:** `agente/zcode/AV18-08` (un commit).
- **Qué ha cambiado:** premios y alternativas en las 3 aventuras de Boundington; libro + auditor + prueba; documentos.
- **Cómo lo pruebo:** `python3 tools/validar_todo.py` · `python3 tools/auditar_recompensas_escenas.py` · `python3 tools/test_auditar_recompensas_escenas.py`
- **Qué tengo que mirar yo (Oriol):** jugar una vuelta y ver si los textos de los premios suenan bien; decidir lo de Furia de Alaro y los premios del Ocaso (ver RECOMPENSAS_ESCENAS.md).
- **Fichero de sesión:** `sesiones/2026-10-15-claude-av18-08-recompensas-escenas.md`
