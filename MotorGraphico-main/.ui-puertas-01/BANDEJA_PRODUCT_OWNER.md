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

## 0. Resuelto (no se vuelve a preguntar)

| Fecha | Qué | Dónde queda |
|---|---|---|
| 16/10 18:56 | **PO-50:** sí a los tres flecos de los oficios (exp 1/2/3 + 2 por receta nueva; tope por efecto = mitad del tope del tier; nivel por tier Aprendiz/Oficial/Maestro/Gran maestro). **PO-51:** arquitectura por el camino B (composición en 4 pasos, «lo más eficiente a la larga»); sin código hasta el paso 0. **PO-45:** las deidades que faltaban ya están escritas: las Caminatas del vault (Chronos, Guerra, Morfeo, Muerte, Sofía, Vida…). **PO-46:** una sola vida máxima y una vida actual (unificar `maxHealth`/`derived.vida`); los ataques de monstruo funcionan como los de un PJ: físicos por DES (ligero) o CON (pesado), y las cartas con los dados que diga la carta. **PO-47:** sí a los dos validadores de puertas. **PO-42 (matiz):** reventa al 50 %, salvo que lo hayas mejorado: entonces 50 % + muebles + mejoras. **PO-44:** abrir un callejón recto plaza → iglesia (ya cubierto por AV16-09: iglesia 40 % → 0 %). PO-33, PO-34, PO-37 y PO-38: confirmadas como estaban en D0018. | [D0020](decisiones/D0020-respuestas-bandeja-16-10.md) · `claude/oficios/` · `claude/arquitectura_entidades.md` · `claude/plan_av18_cierre.md` |
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


---

## 2. Lo nuevo (lo irá llenando cada sesión)

### PO-52 — El botín de los días 2 y 3 (AV18-05b, plan `claude/plan_av18_cierre.md`)

El motor **no lee** el metadato `botin.dia`: un cofre «del día 2» en `ciudad_centro` se abre el día 1. Y queda por
confirmar lo que Qwen acordó contigo el 15/10 sobre los cadáveres.
- **Recomendación 1:** `requisitos.dia` en el contenedor (cuatro líneas en `ContenedoresSesion.cpp`: si el día de la
  historia es menor, «Todavía no»). Qwen ya lo escribe en los spawns; Claude lo hace leer en AV18-07.
  Alternativa: poner el botín de los días 2 y 3 solo en mapas que no se pisan antes (cueva, montaña, ocaso).
- **Recomendación 2:** sin cadáveres-contenedor en esta ronda; el canal «combates» lo paga el motor (LootResolver y
  las aventuras de AV18-08), para no contarlo dos veces. Alternativa: cadáveres en cripta y cueva con un cuarto del
  canal de combates.
- Mientras no contestes, se sigue con las dos recomendaciones. **Aplicadas el 16/10** (rama `claude/AV18-cierre`): `disponibleDia` en el motor y sin cadáveres-contenedor.

**Tu respuesta:**

### PO-53 — Los márgenes de la economía y lo que pagan las aventuras (AV18-04b, ECONOMIA.md §9)

El simulador ya certifica (sale a 0), pero los saldos reales son más bajos que los objetivos del 15/10: las aventuras
pagan entre un cuarto y la mitad del presupuesto de misiones en un recorrido (las ramas son excluyentes y un jugador
solo recorre una). Historia acaba con ~330 de oro, no ~840. Ningún perfil se queda sin poder comprar la curación
básica; explora_todo deja sin comprar el anillo y la gema del día 3.
- **Recomendación:** aceptar los saldos reales (es lo que está escrito ahora en §6 y §8) y, si quieres más bolsa,
  pedir a Z.AI premios de 60–120 en los hitos de la ruta principal (§5 «hito de historia»): al reejecutar
  `simular_economia.py` los márgenes suben solos.
- **Alternativa:** mantener los objetivos del 15/10 como meta y dejar el simulador en rojo hasta que el contenido
  los pague.

**Tu respuesta:**

### PO-54 — El traidor del día 1 (Encargo C, informe C3)

`bnd1_unido_al_culto` (unirte de verdad a Edul en la cripta) y `bnd1_verdaderos_unido` (sumarte a los Verdaderos) se
ponen el día 1 y **nadie las lee después**: el día 2 y el Ocaso tratan al traidor igual que al héroe. No bloquea nada
(por eso no salió en la tabla de hallazgos), pero es el hueco de trama más grande que queda.
- **Recomendación:** cortar en el día 1 con aviso: las dos opciones dicen lo que cuestan («no habrá vuelta atrás con la
  guardia») y el día 2 empieza con un beat corto de consecuencia (Venides no te recibe; Skilla te cobra el doble) y el
  Ocaso te pone del lado del culto en la plaza (sin ruta nueva: las escenas de Sandes ya existen, cambia quién te habla).
  Coste: ~8 beats JSON, lo hace Fable.
- **Alternativa:** una ruta de traidor completa (día 2 y Ocaso propios). Coste alto: es otra aventura de ~60 beats.
- Mientras no contestes, no se toca: hoy el traidor juega como el héroe.

**Tu respuesta:**
