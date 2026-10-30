# Bandeja del product owner — lo que solo puede decidir o hacer Oriol

> Abierta el 14/10/2026 (08:30) por Claude, a petición de Oriol: «un documento md con todas las cosas
> que tengo que resolver yo como product owner, y así puedas seguir más cómodamente».
> Recoge lo que había repartido en el TABLERO («Esperando a Oriol»), en las decisiones `proposed`
> (D0003–D0012) y en los «Para Oriol» de las sesiones.

## Cómo se usa

- Cada punto tiene un número (**PO-xx**), la pregunta, por qué importa y **lo que haría yo**.
- Contesta debajo, en la línea **Tu respuesta:**. Basta con una palabra: «sí», «no», «A», «como dices».
  Si quieres otra cosa, escríbela con tus palabras.
- **Si un punto no tiene respuesta, se sigue con la recomendación**, y la sesión que lo use lo dice en
  voz alta. Los que de verdad bloquean llevan **⛔ Bloquea**: hasta que contestes, eso no se toca.
- Al contestar, la próxima sesión lo pasa a su decisión (`decisiones/`) y lo mueve a «Resuelto».
- Orden: primero lo que solo puedes **hacer** tú (§1), luego lo que estoy programando ahora (§2), luego
  la historia (§3) y al final lo técnico (§4), que puedes delegar entero.

---

### PO-45 — Los hechizos divinos: ¿por deidad o por clase? (AV12-05, Sonnet)
De los 210 hechizos, 200 son plegarias de deidad («cuenta para el límite de 3»): 100 cuelgan de deidades que
existen y 100 de **10 deidades que no existen**. Ningún código carga `spells.json`, y `prayerSkillIds` se carga
pero nadie lo lee. Detalle en `planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md`.
- **Recomendación:** por deidad (es lo que dicen las propias cartas). Faltaría escribir las 10 deidades y que Claude Cowork conecte `prayerSkillIds` en el motor.
- **Alternativa:** repartirlos entre las 6 clases del método (no encaja con el texto de las cartas).
- **Y si es por deidad:** ¿las 10 deidades que faltan las escribes tú o Gemini?

**Tu respuesta:** 
[[Caminata - Chronos y los Hijos del Mediodía]]
[[Caminata - Deithrok, el Mendigo Afortunado]]
[[Caminata - Deseo y Eros, los Casamenteros]]
[[Caminata - Egos, el Padre Rico]]
[[Caminata - Envidia, la que No Camina]]
[[Caminata - Forja, el Herrero de Paso]]
[[Caminata - Guerra, la Mercenaria Feliz]]
[[Caminata - Morfeo, el Actor Prestado]]
[[Caminata - Muerte, la Niña del Musgo]]
[[Caminata - Sofía, la Consejera que No Se Queda]]
[[Caminata - Vida, la Labradora]]

### PO-46 — La vida de los monstruos (AV12-05 y AV6-06, Sonnet)
Cada ficha tiene dos vidas: `maxHealth` y `derived.vida`, que es siempre la mitad exacta. El juego usa
`maxHealth`. La fórmula `base + paso × tier` (criatura 8+4t, élite 16+6t, jefe 30+10t) acierta 430 de 432; las dos
que no cuadran son `itx_centinela_de_salmuera` (18, la fórmula da 16) e `itx_matriarca_de_la_sal` (64, da 70).
36 monstruos pasan del tope de vida de su tier (hoy no importa: el combate no aplica el tope).
- **Recomendación:** `maxHealth` es la canónica; se adopta la fórmula para los monstruos nuevos; las dos fuera de
  fórmula se quedan como están si son a propósito; el tope de tier se decide en AV12-04 (equilibrado).
- **Y una más:** los 900 ataques de monstruo no tienen cifra de daño. ¿La necesitan (hoy el daño sale de la carta
  y los dados)?

**Tu respuesta:** opino que no te sentit guardar dos estats de vida maxim, sino un de vida maxim i un de vida actual, per saber quan li queda de vida, s'hauria de unificar parts d'aixo.
Els atacs del monstre haurien de funcionar igual que els de un pj o npc aliat, o inclus enemig, de tal manera que atacs fisics van derivats al seu estat d'atack fisic DES/CON depenent de si es un atac pesat o lleuger i en cas de ser una carta/habilitat els daus que indica la carta.


### PO-47 — Dos validadores de puertas (AV14-20, Haiku)
El nuevo `tools/validar_puertas.py` (en `out/claude_puertas02/`) mira los 116 niveles: puertas a niveles que
existen, llegada pisable y puerta de vuelta (22 avisos, ninguno de la ciudad). Ya existía `tools/validar_enlaces.py`,
que solo mira tiles de ciudad e interiores y no está en `validar_todo`.
- **Recomendación:** retirar `validar_enlaces.py` cuando el nuevo esté aplicado (hace lo mismo y más).
- **Avisos que son tuyos:** `recuerdo_taberna` sube a `interior_taberna_p1`, que no vuelve al recuerdo; ¿es a
  propósito? Los otros (9 llegadas sobre un PNJ en las ciudades del mundi) los arregla Claude.

**Tu respuesta:** Sí


## 0. Resuelto (no se vuelve a preguntar)

| Fecha | Qué | Dónde queda |
|---|---|---|
| 15/10 20:46 | **PO-48:** no se cambian los nombres aprobados (PO-39/40/41). Las puertas duplicadas siguen llevando al mismo local, salvo **una**: la segunda sastrería (`puerta_ropa_sur`) pasa a ser un **local nuevo de artículos de pesca, caza y agricultura** (nombre provisional «El Zurrón del Vado», emblema `cana_pescar`, interior `interior_pesca_caza.json`; cañas, cebos, trampas y semillas: sirve a los oficios de D0017). Lo prepara ChatGPT (AV16-08); el destino de la puerta lo cambia Qwen (AV16-09b). **PO-49:** aprobada: las rentas cobran una vez por día de historia; actuar, una vez por sitio y día; los oficios gastan su franja. Lo programa Claude (AV18-07). | [codex.md](orquestador/codex.md) · [qwen.md](orquestador/qwen.md) · [D0017](decisiones/D0017-oficios-dan-oro-experiencia-y-reputacion.md) · [D0019](decisiones/D0019-local-pesca-e-ingresos-po48-po49.md) |
| 15/10 10:34 | **Contestadas en PENDIENTE_DE_REVISAR.md:** PO-01 (hecho), PO-02 (lanzar: cola del 15/10 para ChatGPT, Z.AI y Qwen), PO-25.4 (se quedan los dos mapas), PO-31 (profundizar más adelante), PO-33 (tope 50), PO-34 (regla del clon limpio), PO-37 (quemado, congelado, aturdido, silenciado; dados extra sí), PO-38 (temor/respeto por apariencia: diseño AV20-01), PO-42 (50 %), PO-44 (reubicar edificios: AV16-09), PO-45 (las deidades las escribe Oriol), PO-46 (daño y más acciones para los enemigos), PO-47 (revisar los nombres de las puertas), reglas [propuesto] de Sonnet («todas»). | [D0018](decisiones/D0018-respuestas-bandeja-15-10.md) · [PLAN_15-10](planificacion_alcance_y_vision/PLAN_15-10.md) |
| 14/10 | **PO-43 resuelta por integración:** Oriol confirma que Claude ya aplicó el ajuste del validador y las dos pruebas afectadas pasan; lugares.json y sus pruebas van dentro de claude_dialogos04. **No fusionar las ramas Codex**, porque duplicarían CMake. | [REVISAR.md](REVISAR.md) · [sesión](sesiones/2026-10-14-codex-av15-02b.md) |
| 14/10 13:01 | **PO-39:** tabla de 17 nombres/emblemas aprobada con PO-40/41; laúd y pergamino irán a la prueba de 5 s cuando haya dibujo. **PO-40:** Lingote de Oro = herrería de Aigren; Luisarda = La Boutique de Madame, carrete, «sastre y tela». **PO-41:** interior_posada = Taberna Rica, «cama y comida», cama. | [D0015, corrección 13:01](decisiones/D0015-respuestas-bandeja-po-14-10.md) · [tabla aprobada](planificacion_alcance_y_vision/NOMBRES_BOUNDINGTON.md) |
| 14/10 09:01 | **Tus respuestas a la bandeja** (todas las de abajo que ya no salen): acompañantes con su turno y jugados por ti; la ayuda de dados hasta 4 dados (y no vuelve); el crítico atraviesa el bloqueo salvo bloqueo crítico; la pifia da un ataque de oportunidad físico; morir → Game Over → último guardado con una cicatriz más; la esquiva, siempre tirada enfrentada; el sonido espera a qwen; la CON y el peso, la pifia de la salvación (daño y efecto doble), los kits, tus dioses; borrar el combate viejo midiendo antes; el prólogo (túnicas, y luego la capucha); la compañía de Rosita con el golpe mortal de Sandes; los nombres de los sitios, los menús viejos y la ciudad; D-MOR-09 y D-MIS-03 sí; el diario completo; las 950 cartas para monstruos; planificar lo que nadie usa; D0003, D0004 (B), D0006, D0009 y D0012 aceptadas; los restos. | [D0015](decisiones/D0015-respuestas-bandeja-po-14-10.md) · tareas AV13-28…36, AV14-21…24 y la fase AV15 |
| 14/10 09:01 | **Usabilidad (tus apuntes de la UOC)**: leídos los 75 ficheros y aplicados al juego. | [USABILIDAD_UOC.md](planificacion_alcance_y_vision/USABILIDAD_UOC.md) · fase AV15 |
| 14/10 08:21 | **La acción principal deja de cerrar el turno**, como en BG3: atacas y después puedes apartarte. El turno acaba con «Terminar turno» o cuando no queda nada que gastar. Cambia GAMEMACHINE B7. | D0014 §8 · COMBATE_BG3.md §4 |
| 14/10 08:21 | **Las distancias de las cartas**: al lado 1, corto 4 (6 m), medio 8 (12 m), largo 12 (18 m) casillas; más allá de 1, viéndole. | D0014 §8 · COMBATE_BG3.md §5 · `Game/AlcanceCartas.h` |
| 14/10 | El roce (el empate quita 1), la ayuda de dados en Tier 0–1, los esbirros a DES 1–2, BG3 como objetivo, el orden de trabajo. | D0014 |
| 08/10 | El crítico es «al menos dos 6»; los topes; el atributo de las cartas; el Tier 1. | D0013 |
| 29/09 | ¿El combate lleva iniciativa? **Sí**: 1d6 + DES, uno a uno (hecho el 14/10, AV13-11). | D-COMB-03 · COMBATE_BG3.md §3 |

---

## 1. Para las 16:00 (lo que dijiste que mirarías en casa)

### PO-04 — Cómo darme permiso para commitear y publicar
Tu pregunta era cómo. Hay dos partes:
1. **Commitear en tu carpeta:** git borra sus propios ficheros de bloqueo (`.git/index.lock`), y
   borrar en tus carpetas está apagado. Si me dices «puedes commitear», te pido permiso de borrado para
   `Software` (sale una ventana en la app y lo apruebas), y a partir de ahí hago yo los commits, con un
   mensaje por paquete.
2. **Publicar (push):** desde aquí no tengo tus credenciales de GitHub (están en el llavero de tu Mac).
   Dos maneras:
   - **A (la simple):** a las 16:00 ejecutas tú `git push` en `MotorGraphico-main`. Es un comando.
   - **B:** en claude.ai, Ajustes → Conectores → GitHub, conectas tu cuenta y das acceso a los repos
     (`MotorGraphico`, `Onegai-Biodiversitat`). Entonces puedo subir ramas y abrir pull requests yo mismo.

**Tu respuesta:**


### PO-33 — El trinquete de los «muertos» (D0007)
Hay una prueba que cuenta cuántas piezas terminadas no usa ningún ejecutable, y no deja que suba (hoy
el tope es 42). Al cablear cosas salieron 22, pero entraron otras (cabeceras que antes no se contaban),
y el número real es 50. **La pregunta es si se sube el tope a 50** (se acepta la foto de hoy) y después
se baja limpiando, **o se limpia primero hasta 42**. Lo que haría: subirlo a 50 y que AV14-24 (el plan
de integración) lo vaya bajando.

**Tu respuesta:**

### PO-34 — Comprobar desde un clon limpio (D0011)
Antes de dar algo por hecho, compilarlo y probarlo desde una copia recién clonada del repositorio, no
desde la carpeta de trabajo. En la carpeta hay ficheros sin commitear que hacen que todo pase, y en el
commit faltan (ya pasó con un `.cmake` sin su línea en `CMakeLists.txt`). **La pregunta es si se
convierte en regla para todas las IAs.** Lo que haría: sí. Cuesta unos minutos por entrega.

**Tu respuesta:**
 sí. Cuesta unos minutos por entrega.
### PO-37 — Qué pueden hacer las cartas (D0005)
Las 702 cartas describen su efecto **en prosa**: el motor no puede leer «quema 2 rondas». Para que el
combate las ejecute hace falta un vocabulario cerrado. Lo que hay que decidir:
1. **Los estados:** hoy hay 16 (agarrado, anclado, asustado, caído, cegado, encantado, endeudado,
   enfurecido, envenenado, fatiga, guardia rota, inconsciente, marcado, paralizado, ralentizado…).
   ¿Faltan **quemado, congelado, aturdido, silenciado**?
2. **«Tallar» y «devastar»** salen en muchas cartas: ¿qué hacen exactamente? Tallar aplica hemorragia, y devastar hace daño penetrante.
3. **¿Dados extra?** (las cartas que dicen «+1d6 si…»): ¿se permiten? Sí
4. **¿Quién escribe la parte que lee el motor?** Gemini en los datos (AV13-21), revisado por ti.

**Tu respuesta:** ya rspondi

---

## 2. Lo nuevo (lo irá llenando cada sesión)

### PO-38 — Cuánta moral cuesta caer (AV13-33)
D0015 §7 dice «cicatriz y moral», pero no cuánta. Hoy, al caer, la escena de noqueo o rescate sale, la
cicatriz se apunta y la moral **no se toca**. La moral va de −100 a 100 (`Moral`, con tramos).
- **Recomendación:** −5 por caída, y −10 si te tienen que rescatar (alguien de los tuyos se arriesga por
  ti). Se aplica a la partida que cargas, como la cicatriz, para que no se borre al cargar.
- **Alternativa:** 0. La cicatriz ya es el castigo y la moral queda para lo que eliges hacer.

**Tu respuesta:** la solucion ya la respondi y era generar una variable de apariencia y que tuviera impacto en como la gente te percibe.

### PO-42 — Cuánto te devuelven al revender un negocio (AV15-05a, EH-30)

Ahora lo que compras en un cartel de negocio se puede revender, y la hoja dice lo que pierdes antes de
aceptar («Recibes 60 de los 120 que pagaste»). Falta decidir el porcentaje: está en una sola constante
(`Game::NegocioEstado::kReventaPct`), así que cambiarlo es una línea.
- **Recomendación:** el 50 %. Comprar sigue siendo una decisión de peso (si te equivocas, pierdes la
  mitad), pero no te deja atrapado con un negocio que ya no quieres.
- **Alternativas:** el 75 % (equivocarse sale barato y se compra más a la ligera) o un porcentaje que
  baje con el tiempo que lo has tenido (más fiel, pero hay que explicarlo en la hoja).
- Mientras no contestes, el juego usa el 50 %.

**Tu respuesta:** 50%, a no ser que lo mejores, que entonces habria que revaluar la viviendo siendo 50%+muebles+mejoras

### PO-44 — ¿Abrimos un paso entre la plaza y la iglesia? (AV15-03)

Desde la plaza del Casco, la puerta de la iglesia está a 26 pasos y en línea recta serían 18: un 44 % de desvío. Es la
única puerta que pasa del 25 % del cierre de AV15-03, y **no es por el decorado** (los objetos suman 0 pasos): las
casitas del relleno del Casco la rodean. Arreglarlo es cambiar el suelo (quitar una o dos casitas o abrir un callejón),
no mover objetos, así que toca `gen_ciudad.py` y regenera el TMX.
- **Recomendación:** no ahora. La iglesia es «el sitio raro» del Casco («deteriorada y llena de secretos») y
  26 pasos no se sienten largos; antes del paso conviene ver cómo queda el fundido en el juego.
- **Alternativas:** abrir un callejón recto entre la plaza y la puerta de la iglesia (una tarea de medio día, con su
  medida antes y después); o bajar el cierre a «las tiendas», que es lo que decía el texto («de la plaza a cada
  tienda») y la iglesia no lo es.

**Tu respuesta:**
brir un callejón recto entre la plaza y la puerta de la iglesia 

### PO-50 — Oficios: los tres flecos de los documentos de Fable (16/10)

Los nueve oficios y la tabla de materiales están en el proyecto (`claude/oficios/`). Lo decidido está escrito como
cerrado; solo quedan estos tres puntos, y en cada uno hay una propuesta ya escrita que vale si no contestas.
- **Exp de los oficios de fabricar:** 1/2/3 por grado (como servir) más +2 la primera vez que haces una receta.
  Así Oficial llega al final de la campaña 1 y Maestro queda para después. Alternativa: 2/4/6 (como estudiar), si
  quieres que el bono de Maestro se toque en Boundington.
- **Herrería, tope por tipo de efecto:** la mitad del tope del tier, redondeando arriba (1/1/2/4/8 para tier 1–5).
- **Herrería, nivel de oficio por tier del objeto:** tier 1 Aprendiz, tier 2 Oficial, tier 3 Maestro, tier 4 y 5
  Gran maestro.

**Tu respuesta:** Sí

### PO-51 — Arquitectura de entidades: ¿camino B? (16/10)

`claude/arquitectura_entidades.md` propone no hacer una jerarquía de humanoides, sino tres piezas compartidas
(una sola regla de «puedo pisar», un solo paso visual, un solo rumbo) y un campo `uso` + `VehiculoData` en el
catálogo de objetos, en 4 pasos de 5–7 días que no cambian ningún JSON ni partida guardada. **Sin tocar código
hasta que contestes.**
- **Recomendación:** camino B, empezando por el paso 0 (tests que fijan lo de hoy) y el 1 (la regla de pisar).
- **Alternativas:** A (jerarquía `Humanoide` con subclases, 8–12 días, más riesgo) o C (solo la regla de pisar, 1 día).
- Dos decisiones menores que puedes dejar para cuando llegue cada paso: si en el paso 1 se conservan las
  diferencias de hoy (el PNJ no cruza pickups; en combate se cruzan acantilados) o se unifican ya; y si el enemigo
  pasa a interpolar su patrulla en el paso 3.

**Tu respuesta:** Lo que hayas valorado mas eficiente a la larga.
