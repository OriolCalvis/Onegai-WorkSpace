# Catálogo de flujos secundarios (FS-01 … FS-08)

Los siete flujos nucleares (`wf-01` … `wf-07`) están especificados en profundidad porque **todo el juego pasa por ellos**.
Los ocho de este catálogo son igual de importantes para la visión, pero se especifican en ficha corta por tres motivos:

1. **Dependen de un nuclear**: no se pueden cerrar antes (el crimen necesita ruido y visión; la prisión necesita calendario; el romance necesita reputación).
2. **Necesitan contenido aguas arriba**: rutinas de NPC, catálogo de facciones, delitos, recetas — y el flujo es `canon → cartas → motor` (BITÁCORA, regla 1).
3. **Su coste de diseño es alto y su urgencia es baja** en el primer corte jugable.

Cada ficha dice: fuente, por qué no es nuclear, qué existe ya, qué tareas del Gantt lo cubren, de qué depende y cuál es su corte mínimo.
Cuando un FS pase a nuclear, se convierte en `wf-0N-*.md` con la plantilla completa de 13 apartados.

| FS | Flujo | Fuente | Prioridad | Depende de | Tareas |
|---|---|---|---|---|---|
| FS-01 | Crimen, detección y niveles de alerta | §10.1 A-C | Alta | WF-02 (ruido), WF-04 (reputación), FS-06 (patrullas) | AV8-01…AV8-06 |
| FS-02 | Prisión profunda, trabajos forzados y fuga | §10.1 C | Media | FS-01, calendario (AV10-02) | AV8-07, AV8-08 |
| FS-03 | Identidad, disfraz y subculturas | §10.2 | Baja | WF-04 (apariencia), FS-01 | AV8-09, AV8-10 |
| FS-04 | Vínculos, cortejo, romance, familia y corrupción | §10.4, §10.5 | Media | WF-04, FS-06, FS-08 | AV4-10…AV4-12, AV3-07 |
| FS-05 | Oficios, crafteo y jornada laboral | §10.8 | Media | WF-03, WF-05, calendario | AV4-07…AV4-09 |
| FS-06 | IA viva: rutinas 24 h, patrullas y fauna | §10.13 | Alta | WF-02 (ruido), editor (AV6-02, AV6-11) | AV9-01…AV9-04 |
| FS-07 | Escenografía viva: distritos, clima y objetos | §10.14 | Media | WF-01 (cobertura), editor (AV6-13) | AV9-05…AV9-08 |
| FS-08 | Hogar, decoración y vida doméstica | §10.9, §10.4.5 | Baja | WF-05 (propiedades) | AV4-04, AV4-12 |

---

## FS-01 — Crimen, detección y niveles de alerta (§10.1 A-C)

**Qué debe pasar.** El delito se clasifica por gravedad (menor / medio / mayor), se detecta por visión (cono de 120°: zona central 6 m instantánea, periférica 12 m con barra de sospecha, espalda ciega) o por ruido, y escala en cinco niveles de alerta: 1 sospecha e interrogatorio, 2 orden de detención local, 3 alarma general con campana y rastrillos cerrados, 4 cazarrecompensas de élite en caminos y vados, 5 proscripción imperial con cartel de «se busca vivo o muerto».

**Corte mínimo.** Cono de visión + barra de sospecha + nivel 1 y 2 + un delito de cada gravedad. Sin cadáveres arrastrables todavía.

**Qué existe ya.** Nada de detección: el motor no tiene ruido (WF-02), ni sospecha, ni conos. Existe el `Engine::World` con entidades y handles estables (C-17) y `EnemyBrain` para comportamiento, que es el sitio natural del estímulo sonoro.

**Riesgo de diseño.** Es el flujo que más depende de decisiones ajenas: si el movimiento sigue siendo por celdas (D-MOV-01), el cono de visión debe discretizarse a celdas y no a metros.

---

## FS-02 — Prisión profunda, trabajos forzados y fuga (§10.1 C)

**Qué debe pasar.** Entrar en prisión confisca el equipo (cofre del alcaide), hace saltar el calendario (se pierden cosechas y rentas), y ofrece rutina viva: hablar con presos, trabajos forzados que reducen condena pero fatigan y ensanchan la musculatura, y marcaje corporal permanente (tatuaje). Salidas: cumplir condena y pagar fianza, o fugarse (ganzúa del carcelero borracho, pozo ciego, cloacas).

**Corte mínimo.** Confiscación + salto temporal + fianza. La fuga y los trabajos forzados después.

**Qué existe ya.** El inventario y el equipo están bien modelados (WF-06) para la confiscación; **falta el calendario** (AV10-02), sin el cual «perder rentas y cosechas» no se puede expresar. `SaveGame` ya persiste inventario, oro y negocios.

**Dependencia dura.** WF-05 (rentas) y FS-05 (jornada laboral) necesitan el mismo reloj.

---

## FS-03 — Identidad, disfraz y subculturas (§10.2)

**Qué debe pasar.** El disfraz tiene un *Passing Score* que combina ropa y atrezo, cosmética, anatomía contra disfraz y magia de ilusión, y se evalúa contra la percepción de cada NPC que te mira a menos de 3 m. Las consecuencias son culturales y territoriales: en ciudades cosmopolitas la transgresión se celebra y abre líneas de diálogo y escenarios; en teocracias se considera afrenta, los tenderos se niegan a comerciar y los alguaciles expulsan de la ciudad amurallada. Permite infiltrarse en zonas segregadas (conventos de clausura, harenes, baños exclusivos, cofradías cerradas).

**Corte mínimo.** Una prenda de disfraz con su bonificación y una región que la acepta y otra que la castiga.

**Qué existe ya.** El equipo tiene ranuras y rareza (WF-06), pero **no hay sistema de apariencia** (AV3-01) ni regiones con cultura declarada. `CharacterSheet` ya lleva `raceId` y `deityId`, que son entradas del Passing Score.

---

## FS-04 — Vínculos, cortejo, romance, familia y corrupción (§10.4, §10.5)

**Qué debe pasar.** Cada NPC tiene tres medidores hacia el jugador (afecto, respeto, confianza, 0…100) y el cortejo recorre cinco fases: primera impresión (higiene, ropa, aspecto y cicatrices), conexión dialéctica con tiradas sociales y descubrimiento de gustos, regalos y citas, intimidad con consentimiento explícito, y enlace con convivencia, descendencia y beneficios de hogar. En paralelo, la brújula moral de un NPC puede corromperse (el sacerdote que deja el hábito) o redimirse (la mercenaria que jura defender inocentes).

**Corte mínimo.** Los tres medidores y la fase 1-2 con un NPC de prueba. El matrimonio y la descendencia después.

**Qué existe ya.** La ficha tiene `bondId`, `fearId`, `flawId`, `goalId`, `idealId`, `personalityId`, `virtueId` — la personalidad ya es dato. `Companeros.h` existe. El motor narrativo puede llevar las fases como beats (WF-07). **Falta** el modelo de los tres medidores y la apariencia que alimenta la primera impresión (AV3-01, AV4-10).

---

## FS-05 — Oficios, crafteo y jornada laboral (§10.8)

**Qué debe pasar.** Herrería completa: comprar o picar mineral, calentar al rojo, minijuego de ritmo y precisión en el yunque, temple y afilado, con calidades Tosca / Estándar / Refinada / Obra Maestra (esta con ranura de runa). Alquimia: recolectar en el bosque, triturar, mezclar y destilar para pociones, filtros, venenos y elixires. Cierre de jornada: contar monedas, pagar diezmo e impuestos y ganar maestría.

**Corte mínimo.** Un oficio (herrería) con dos calidades y venta en el mostrador propio.

**Qué existe ya.** El catálogo de objetos y el `ObjectCatalog` dan el material; WF-03 da la venta; WF-05 da el mostrador propio y los impuestos. **Falta** el minijuego, las recetas y la maestría (AV4-07…AV4-09). El doc ofrece dos resoluciones (minijuego y tirada): D-NEG-06.

---

## FS-06 — IA viva: rutinas 24 h, patrullas y fauna (§10.13)

**Qué debe pasar.** Los ciudadanos tienen horario circadiano completo (06:00 despertar y aseo → 08:00 jornada → 13:30 almuerzo → 18:30 cierre → 19:30 taberna → 23:00 sueño con cono de visión desactivado y radio auditivo al 10 %), diálogo ambiental con memoria de sucesos recientes, y las patrullas tienen cuatro estados: verde (desprevenida, paradas tácticas de 10 s), amarillo (estímulo sospechoso, desenvaina e investiga 15 s), naranja (indicio grave, silbato y enjambre a 60 m en menos de 10 s) y rojo (persecución coordinada: frontal, flanqueo por calles paralelas, arqueros ralentizando y rodeo del edificio si subes al tejado). Fauna urbana que delata y pilluelos que roban.

**Corte mínimo.** Una rutina de 4 franjas por NPC + estado verde/amarillo de una patrulla con waypoints.

**Qué existe ya.** `EnemyBrain`/`EnemyBrainData` para comportamiento táctico; el editor puede pintar rutas (AV6-11); el diálogo ambiental puede colgarse del motor narrativo (WF-07). **Falta** el reloj de mundo (AV10-02) — sin él no hay «23:00»— y los árboles de comportamiento (AV9-01).

---

## FS-07 — Escenografía viva: distritos, clima y objetos interactivos (§10.14)

**Qué debe pasar.** Cuatro distritos con estética, seguridad y sonido propios (noble, mercado, arrabales, muelles). Clima dinámico que cambia el juego: sol con sombras nítidas y terrazas; lluvia con adoquín especular, charcos físicos y chapoteo, transeúntes que corren a los soportales; niebla con visibilidad a 8 m y conos de guardia al 50 %; viento con letreros chirriando. Objetos que hacen algo: mesas volcables como cobertura, jarras y sillas arrojadizas, cortinas y toneles que prenden y propagan el incendio.

**Corte mínimo.** Un distrito con su paleta y su audio + lluvia que afecta a la visibilidad + una mesa volcable como cobertura.

**Qué existe ya.** `RPG/Audio.h` (audio con instancias y descarga), capas de baldosas ordenables desde el panel (E-12), y el editor de atmósfera propuesto (AV6-13). **Falta** el clima como estado del mundo (AV9-07) y los objetos interactivos con efecto (AV9-05), que es prerrequisito de WF-01 (cobertura e incendios).

---

## FS-08 — Hogar, decoración y vida doméstica (§10.9, §10.4.5)

**Qué debe pasar.** Al cruzar la puerta de tu casa se activa la cuadrícula isométrica del suelo y colocas camas (descanso y fatiga), baúles privados (botín que nadie saquea), mesas de trabajo y estanterías con lore. Si hay cónyuge, prepara comidas con bonificación de descanso («Consuelo del Hogar») y gestiona los ingresos pasivos de la vivienda.

**Corte mínimo.** Cuadrícula de una habitación, tres muebles con su efecto (cama, baúl, mesa) y guardado de la disposición.

**Qué existe ya.** El motor es isométrico y por celdas, y el editor ya coloca objetos y capas: la base técnica existe. `SaveGame` persiste inventario y negocios. **Falta** el modo decoración (AV4-04), la capacidad de muebles por inmueble (AV6-10 declara el campo) y los efectos de descanso.
