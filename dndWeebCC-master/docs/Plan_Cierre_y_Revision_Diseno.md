# Plan de cierre del proyecto y revisión de diseño

*Sistema de Cartas y Tiers — dndWeebCC*

Este documento tiene dos partes. La primera es un plan de 50 tareas para terminar lo que queda pendiente del proyecto, construido cruzando el GDD (`docs/Sistema_Cartas_Tiers.md`), el UML (`docs/UML_Sistema_Cartas_Tiers.mermaid`) y el estado real del código Java/Thymeleaf. La segunda es una revisión crítica del diseño visual actual (espaciado, legibilidad, jerarquía) con una comparación honesta frente a referentes profesionales del género.

---

## Parte 1 — Plan de 50 tareas

Están agrupadas en 13 bloques por dependencia lógica, no por orden estricto de ejecución (aunque dentro de cada bloque el orden sí importa). Cada tarea indica **por qué** está en la lista: o bien es contenido que el propio GDD marca como pendiente, o bien es una brecha entre lo que el GDD de edición 2 describe y lo que el código Java realmente implementa hoy (confirmado leyendo modelos, UML y las secciones 6-9 del GDD línea a línea).

### Bloque A — Regenerar contenido narrativo a edición 2 (CAR sustituye a SAB)

El propio GDD marca las secciones 14-18 con un aviso: están escritas para la edición 1 (SAB + recursos por puntos) y no son válidas bajo las reglas de las secciones 4-13. Es la deuda de contenido más visible del documento de diseño.

1. **Regenerar las 5 clases iniciales** (Guardián de Hierro, Sable Errante, Sombra del Camino, Arcanista, Voz del Alba) a edición 2: sustituir SAB por CAR en sus stats, y su recurso por puntos (energía/maná/aguante/foco) por el sistema de pilas.
2. **Regenerar las 5 razas iniciales** con bonos repartidos entre CON/DES/INT/CAR en vez de CON/DES/INT/SAB.
3. **Rediseñar los trasfondos como los 12 meses de nacimiento** (sustituyendo a los 7 arquetipos narrativos actuales — Soldado, Erudito, Criminal, Noble Caído, Artesano, Acólito, Superviviente): un trasfondo por mes, con su propia virtud/defecto/objetivo. Esto es un cambio de diseño nuevo, no solo una migración de SAB a CAR — queda pendiente definir con el usuario qué aporta mecánicamente cada mes (¿solo sabor narrativo, o también algún bono/afinidad estacional?) antes de escribir el contenido final de los 12.
4. **Regenerar las 10 habilidades físicas de ejemplo** (sección 17.1) con el campo `recovery` en vez de coste por puntos.
5. **Regenerar los 10 hechizos de ejemplo** (17.2) — depende del bloque C (la carta de Hechizo como tipo independiente aún no existe en Java).
6. **Regenerar las 10 pasivas de ejemplo** (17.3) — depende del bloque C (la Pasiva como tipo de carta independiente).
7. **Regenerar los 10 objetos de equipo de ejemplo** (17.4) a los símbolos de compatibilidad y campos actuales de `TierEquipment`.
8. **Regenerar los 5 personajes de ejemplo completos** (18.1-18.5: Borkun, Liesel, Renn, Hermana Aine, Grosh) con las fórmulas de vida de edición 2 (CON×3 + CAR×1).
9. **Actualizar la tabla comparativa de personajes de ejemplo** (final de la sección 18) con los nuevos totales de vida y sin la columna de "recurso máximo" por puntos.

### Bloque B — Sistema de pilas (Activa / Descanso Corto / Descanso Largo)

Esta es la brecha más importante entre el GDD y el código: la sección 6 del GDD describe que ninguna carta de edición 2 tiene coste en puntos, solo `recovery`. Pero `TierSkill.java` **todavía tiene** la clase `Cost` (`resource` + `amount`) heredada de la edición 1, y no existe ningún campo `recovery` en el modelo real.

10. ✅ **Hecho.** Campo `recovery` (`activa` \| `descanso_corto` \| `descanso_largo` \| `ninguno`) añadido a `TierSkill`. `Cost` se deja como legado (`@Deprecated`, ya no se edita desde el formulario) para no romper datos existentes; lo mismo con `upgradePath` (legado) frente al nuevo `evolvesInto`.
11. ✅ **Hecho.** Los 10 JSON de `data/cartas/habilidades/*.json` tienen ahora `recovery: "descanso_corto"` junto al `cost` heredado. Es un valor por defecto razonable (todas tenían coste real en edición 1, ninguna encaja en "ninguno" según la regla de la sección 10), **pero sigue pendiente el ajuste fino carta a carta** que corresponde a la regeneración de contenido del Bloque A — algunas quizá deberían ser `descanso_largo` o `activa` según su impacto real.
12. ⏳ Pendiente. Implementar el estado real de las tres pilas en una sesión de personaje: qué habilidades están en Activa vs Descanso Corto vs Descanso Largo *durante una partida concreta* (hoy `FichaPersonatge` no distingue esto — todas las habilidades preparadas se muestran como disponibles, tal como reconoce el propio comentario en `personatges/detall.html`). Esto requiere estado de sesión transitorio y se solapa con el Bloque D (motor de combate) — no es solo un cambio de modelo de datos.
13. ✅ **Hecho.** Formulario y plantillas de carta de Habilidad (álbum, ficha, editor) muestran/editan `recovery` en vez de coste por recurso.

### Bloque C — Tipos de carta descritos en el GDD pero no implementados en Java

✅ **Bloque cerrado (items 14-19).** El UML marcaba en verde solo 6 tipos de carta (`TierClass`, `TierRace`, `TierBackground`, `TierSkill`, `TierEquipment`, `TierFeat`). Los 6 tipos que describían las secciones 9.4-9.11 del GDD sin modelo/repositorio/servicio/controlador propios ya existen, siguiendo exactamente el mismo patrón Modelo→Form→Repository→Service→Controller→3 plantillas que los 6 originales:

14. ✅ **Hecho.** `TierPassive` — `/cartas/pasivas`, con `trigger`, `effect.scaling`, `synergyTags`, `unique`.
15. ✅ **Hecho.** `TierSpell` — `/cartas/hechizos`, con `castingStat` (INT arcano / CAR divino), `school`, `recovery`, `evolvesInto`.
16. ✅ **Hecho.** `TierConsumable` — `/cartas/consumibles`, con `actionType`, `uses` (el límite de 3 simultáneos sigue pendiente de validación real, ver Bloque E ítem 29).
17. ✅ **Hecho.** `TierSpecialTrait` — `/cartas/rasgos`, con `origin` (race/background/story/tier).
18. ✅ **Hecho.** `TierSummon` — `/cartas/invocaciones`, con vida, `attacks[]`, movimiento, pasiva y `control`. La Zona 5 de la ficha de personaje (`personatges/detall.html`) **todavía no lee estas cartas** — el tipo ya existe en el catálogo, pero `Personatge` no tiene un campo que referencie invocaciones activas; sigue pendiente esa última conexión.
19. ✅ **Hecho.** `TierDeity` — `/cartas/deidades`, con `domain`, `favor.scaling: CAR`, `compatibleWith`, `obligations`.
19.b ✅ **Hecho (no estaba en la lista original de 6, pero cierra el mismo hueco).** `TierCondition` — `/cartas/condiciones`, unifica Estado/Bendición/Maldición (`category`, `source`, `stackable`, `effects[]`, `cureConditions[]`). Cierra el aviso literal de `personatges/detall.html` Zona 5 ("Las cartas de Invocación, Estado, Bendición y Maldición todavía no existen como tipo de carta"). Primer ejemplo real: la carta `anclado`, usada por la clase Ancla del Vacío — cualquier habilidad, enemigo u objeto puede referenciarla por id en `requiredTags`/`incompatibleTags` en vez de esconder el estado en texto libre.
20. ✅ **Hecho (parcial).** Campo `evolvesInto` añadido a `TierSkill` (ya existía en el `TierSpell` nuevo desde el diseño). **La lógica de sustitución automática en el personaje (la carta mejorada ocupa el mismo lugar que la anterior) todavía no está implementada en `PersonatgeService`** — el campo existe y se puede rellenar en el catálogo, pero al subir de tier nadie aplica el cambio todavía.

### Bloque D — Motor de resolución de combate (GDD sección 7)

Toda la sección 7 del GDD (tirada de pool de d6, ventaja/desventaja, estructura de turno, condiciones, muerte y agonía) es hoy **solo documentación**: no hay ninguna clase Java que lance dados, aplique ventaja/desventaja, controle turnos o condiciones. El UML no la menciona porque no existe ni como propuesta todavía.

21. Implementar un servicio de resolución de tiradas: pool de `Nd6` (N = valor de la característica), sumando 1 éxito por cada 6 y 0,5 éxitos por cada 5, contra una CD (0,5/1/1,5/2/2,5+); éxito crítico si todos los dados son 6, fracaso crítico (pifia) si la tirada suma 0 éxitos.
22. Implementar ventaja y desventaja como tablas de conversión alternativas sobre el mismo pool de dados (ventaja: el 4 también vale 0,5 éxitos; desventaja: solo el 6 cuenta), con anulación mutua si coinciden ambas — nunca se cambia el número de dados.
23. Implementar el estado de turno de combate (acción principal, acción de movimiento, reacción, canalización) y el orden de iniciativa (determinista, `DES + modificadores`, sin tirada), como una entidad transitoria de sesión (no forma parte de `Personatge`, que es la ficha permanente).
24. Implementar el catálogo de condiciones (Sangrado, Caído, Agarrado, Cegado, Encantado, Asustado, Inconsciente, Paralizado, Envenenado, Fatiga) como estados aplicables durante una sesión.
25. Implementar muerte y agonía: estado Moribundo, salvación por muerte (`CON`d6 vs CD 1, con fracaso normal en <1 éxito y fracaso crítico en 0 éxitos), muerte instantánea, y la "Carga del Destino" (cicatriz permanente a cambio de sobrevivir una vez).
26. Diseñar la "sesión de combate en vivo" que finalmente dé contenido real a la Zona 4 (Recuperación) de la mesa de personaje — hoy las tres pilas se dibujan siempre a 0 porque no existe este estado.

### Bloque E — Símbolos de compatibilidad y balance real

27. Implementar los símbolos de **afinidad** (✦ Arcano, ☠ Maldito, ✝ Sagrado, ♞ Montado, ⚙ Mecánico) en `TierEquipment` y `TierClass` — hoy solo existen los símbolos de **peso** (△○□), que ya están implementados y validados en servidor.
28. Convertir `requiredTags` / `incompatibleTags` en una validación real de servidor (hoy son listas de texto sin ninguna lógica que las compruebe al guardar un personaje, a diferencia de `validarSlotsEquipo` / `validarCompatibilidadEquipo` / `validarLimiteMano`, que sí bloquean).
29. Implementar el límite de 3 consumibles simultáneos y el límite de invocaciones activas según INT como validaciones de `PersonatgeService`.

### Bloque F — Multiclase

30. Extender `Personatge` para soportar una segunda clase (`claseSecundariaId`) y aplicar la tabla de límite de mano reducido de la sección 11.1 (15→12 en tier 2, 20→16 en tier 3) cuando esté activa.
31. Actualizar el formulario y la ficha de personaje para mostrar ambas clases y repartir habilidades entre ellas cuando hay Multiclase.

### Bloque G — Bestiario (marcado "propuesto" en el UML)

32. Crear en Java el árbol `Enemigo` (abstracto) → `Criatura` / `Elite` / `Jefe` (con `FaseJefe` como composición), siguiendo exactamente las clases ya bocetadas en el UML.
33. Crear el catálogo `/cartas/enemigos` (CRUD + plantilla de 5 zonas) reutilizando el mismo patrón Modelo→Form→Repositorio→Servicio→Controlador de los 6 tipos ya implementados.
34. Poblar un bestiario de ejemplo a partir de la campaña "La Tomba del Rei Llop" ya estudiada para el UML.

### Bloque H — Mazo de aventura y estancias (también "propuesto")

35. Crear en Java `CartaAventura` (abstracto) → `Tesoro` / `Trampa` / `Evento` / `Encuentro` / `Misterio`.
36. Crear `Estancia` y `Aventura` (contenedor de estancias + composición de mazo de botín/eventos).
37. Migrar la aventura de ejemplo (100 cartas de botín, 20 estancias) al nuevo sistema como contenido semilla.

### Bloque I — Exportación e impresión

38. Terminar de verificar en máquina real (con `./mvnw`) el `PdfService` de cartas imprimibles que se está construyendo ahora mismo, y confirmar que abre correctamente en un lector de PDF real.
39. Añadir exportación en PDF de cartas **de catálogo** (no solo de un personaje), para poder imprimir un mazo completo de clases/habilidades/equipo sueltas.
40. Añadir la opción de exportar un subconjunto elegido (por ejemplo, solo las habilidades preparadas de un personaje) en vez de siempre el personaje entero.

### Bloque J — Ilustración (zona ② de la plantilla universal)

41. Diseñar un sistema de subida/gestión de una imagen por carta — es la única de las 5 zonas de la plantilla universal (sección 19.1) que ninguna página cubre todavía, reconocido explícitamente en el propio GDD.
42. Sustituir los iconos-emoji usados como marcador de ilustración (🛡🗡🔮❤⚔🧬📖⛑👕👖👢✦🎖) por arte real o generado, al menos en las cartas base (5 clases, 5 razas).

### Bloque K — Interfaz y accesibilidad

43. Sustituir la sidebar, que hoy **desaparece por completo** en móvil (`display: none` bajo 820px sin ninguna alternativa), por un menú desplegable/hamburguesa — ver Parte 2 para el detalle de este hallazgo.
44. Extraer los bloques `style="display:flex; justify-content:space-between..."` repetidos casi idénticos en más de 20 plantillas (cabeceras de página, rejillas de acciones) a clases CSS reutilizables (`.gm-page-header`, `.gm-page-actions`) en `components.css`.
45. Revisar accesibilidad de forma sistemática: contraste de `--fs-micro` (10-11px) sobre `--card-ink-soft`, tamaño de zonas táctiles en móvil, atributos `aria-*` en los paneles flotantes y los toggles de sección — ver Parte 2.

### Bloque L — Persistencia y robustez técnica

46. Evaluar sustituir la persistencia en disco basada en JSON plano (`data/cartas/*`, `data/personatges/*`) por una base de datos embebida (H2 o SQLite) — el sistema actual no tiene protección real ante escritura concurrente ni permite consultas más allá de cargar todo el directorio a memoria.
47. Añadir tests automatizados: hoy el proyecto depende de `spring-boot-starter-test` pero no hay ni un solo test escrito. Prioridad alta para `CalculadoraVida` (fórmulas puras, fáciles de testear) y para las validaciones de `PersonatgeService` (slots, compatibilidad de peso, límite de mano).
48. Añadir páginas de error 404/500 personalizadas y manejo explícito de excepciones en los controladores (hoy no se ha verificado qué ve un usuario si pide un `id` de carta inexistente).

### Bloque M — Documentación y cierre

49. Retirar el aviso de "contenido de edición 1, pendiente de regenerar" de la sección 14 del GDD una vez completado el Bloque A.
50. Escribir una guía breve de "primera partida": cómo se juega una sesión completa con este sistema de principio a fin (crear personaje, resolver una tirada, gestionar las pilas, terminar un descanso), pensada para un grupo que nunca ha visto el sistema — hoy el GDD explica cada regla por separado pero no hay un ejemplo de extremo a extremo.

### Resumen por prioridad

| Bloque | Nº de tareas | Por qué importa | Prioridad sugerida |
|---|---:|---|---|
| A. Contenido edición 2 | 9 | El propio GDD lo marca como "siguiente tarea pendiente del proyecto" | Alta |
| B. Sistema de pilas | 4 (2 hechas, 1 parcial, 1 pendiente) | `recovery` ya existe en el modelo y en los 10 JSON de ejemplo; falta el estado de pilas en vivo (solapa con Bloque D) | Alta |
| C. Tipos de carta que faltan | 7 (6 hechas, 1 parcial) | ✅ Los 6 tipos de carta ya existen con CRUD completo; falta conectar Invocación a `Personatge` y la lógica de sustitución de `evolvesInto` | Alta → Media (lo que queda es conexión, no creación) |
| D. Motor de combate | 6 | Sin esto, la sección 7 del GDD (la mitad del reglamento) no es jugable en la app, solo en papel | Media |
| E. Compatibilidad y balance | 3 | Symbolos de afinidad y validaciones que hoy son solo texto decorativo | Media |
| F. Multiclase | 2 | Descrita en detalle en la sección 11.1 pero `Personatge` solo admite una clase | Media |
| G. Bestiario | 3 | Marcado "propuesto"; necesario para que el motor de combate tenga con quién pelear | Media-baja |
| H. Aventuras/estancias | 3 | Marcado "propuesto"; depende de G | Baja |
| I. PDF/exportación | 3 | En curso ahora mismo | Alta (lo ya empezado) / Media (el resto) |
| J. Ilustración | 2 | Único hueco reconocido en la plantilla de 5 zonas | Media |
| K. Interfaz/accesibilidad | 3 | Ver hallazgos concretos en la Parte 2 | Alta (móvil) / Media (resto) |
| L. Persistencia/tests | 3 | Cero tests automatizados es el riesgo técnico más silencioso del proyecto | Alta |
| M. Documentación | 2 | Cierre formal, bajo esfuerzo | Baja |

---

## Parte 2 — Revisión de diseño: espaciado, legibilidad y comparación con referentes

Esta revisión se basa en una lectura directa de `tokens.css`, `base.css` y `components.css`, y en las plantillas de cartas de clase, la ficha de personaje y el formulario de creación ya construidos.

### 2.1 Lo que funciona bien

**Escala de espaciado consistente y con base clara.** La escala `--sp-1` a `--sp-8` (4/8/12/16/24/32/48/64px) sigue una progresión de múltiplo de 4 razonable y se usa de forma disciplinada en casi todos los componentes (`gm-card`, `gm-panel`, `gm-mesa-zona`). Esto es exactamente lo que hace cualquier sistema de diseño profesional serio (Material Design usa 4/8, Tailwind usa 4px de base) — no hay valores "mágicos" sueltos tipo `13px` o `22px` fuera de la escala tipográfica.

**Tipografía con dos juegos de valores fijos (escritorio/móvil) en vez de escalado fluido.** Es una decisión defendible: evita el riesgo de que un `clamp()` mal calibrado produzca texto ilegible en anchos intermedios. El coste es mantenimiento manual si se añaden más breakpoints en el futuro.

**Paleta de carta fija e independiente del tema día/noche.** Es una idea de diseño con criterio: una carta física no cambia de color según la luz de la habitación, así que aquí tampoco debería cambiar con el toggle. Es coherente con el principio rector "todo elemento importante es una carta" — el objeto en sí tiene identidad visual propia, separada del chrome de la aplicación.

**La plantilla universal de 5 zonas aplicada de forma sistemática.** Cabecera/Ilustración/Reglas/Pie/Identificador se repite igual en los 6 catálogos y en la ficha de personaje (a tamaño reducido). Esta consistencia es, de hecho, más disciplinada que la de muchas herramientas de VTT reales, donde cada tipo de objeto tiene su propio layout ad-hoc.

**La "mesa" de personaje como silueta física es una idea más original que la de la mayoría de fichas digitales.** D&D Beyond, Pathfinder Nexus o Foundry VTT muestran el equipo como una lista o una rejilla de inventario; la silueta de cuerpo (cabeza arriba, arma/torso/arma en fila media, piernas y pies abajo) comunica de un vistazo qué slots están vacíos sin necesidad de leer etiquetas, algo que ninguno de esos productos hace realmente bien.

### 2.2 Problemas de legibilidad y jerarquía

**Texto en `--fs-micro` (11px, 10px en móvil) usado para información que sí importa.** El identificador de carta (`gm-card__id`), los badges de peso/rareza y buena parte del pie de las tarjetas mini de la mesa de personaje bajan hasta 8-9px con el modificador `.gm-card--mini`. Un tamaño de 8px es difícil de leer incluso con buena vista, y por debajo de las recomendaciones habituales de accesibilidad (WCAG no fija un mínimo estricto en px, pero 12-13px es el suelo práctico aceptado para texto que un usuario necesita leer, no solo detectar). Recomendación: reservar `--fs-micro` para metadatos verdaderamente secundarios (el ID técnico, por ejemplo) y subir a `--fs-small` (13px) cualquier badge que comunique una regla del juego (peso, rareza, tier).

**Alta densidad de "cajas dentro de cajas" en la mesa de personaje.** Cada zona (`gm-mesa-zona`) contiene una rejilla de `gm-card gm-card--mini`, y cada mini-carta reproduce las 5 zonas completas (cabecera, ilustración, reglas, pie, id) a una escala muy reducida. El resultado fiel al principio de diseño, pero visualmente denso: en la zona de Equipamiento, por ejemplo, cada slot de la silueta lleva su propia mini-cabecera + mini-ilustración + mini-id, lo que en pantallas de portátil puede sentirse recargado comparado con, por ejemplo, los slots de equipo de D&D Beyond (que muestran solo icono + nombre + un número, sin repetir la estructura completa de carta a esa escala). Vale la pena, a futuro, una variante aún más simplificada de carta para contextos de muy poco espacio (quizá una "zona 6" sin ilustración ni pie, solo cabecera + una línea de regla).

**El acordeón (`gm-panel-section`) obliga a varios clics para leer una ficha completa.** En las páginas de detalle de catálogo, solo la primera sección suele venir con `is-expanded`; el resto (equipo permitido, especializaciones, requisitos de stat) empieza colapsado. Es una buena decisión para la vista de álbum (evita una pared de texto), pero en la ficha de detalle de una sola carta — donde el usuario ya decidió que quiere verla entera — obliga a varios clics para ver información que cabría perfectamente en una sola pantalla continua. Referentes como Gatherer (la base de datos oficial de Magic) o D&D Beyond muestran la ficha de un objeto/hechizo entera sin acordeones: todo el texto de regla visible de una vez, porque el usuario ya pagó el coste de navegación al entrar en esa página.

**Repetición masiva de estilos en línea.** Prácticamente cada plantilla (`lista.html`, `detalle.html` de los 6 catálogos, `personatges/*`) repite el mismo bloque `style="display:flex; justify-content:space-between; align-items:flex-end; gap: var(--sp-4); margin-bottom: var(--sp-5); flex-wrap: wrap;"` para la cabecera de página, y variantes casi idénticas para las rejillas de acciones. No es un problema visible para el usuario final, pero es el tipo de deuda que hace que un cambio de espaciado futuro (por ejemplo, "que las cabeceras de página tengan menos margen inferior") requiera tocar 20+ archivos en vez de una sola clase.

### 2.3 Un punto para verificar visualmente, no un defecto confirmado

**Contraste temático entre el tema Noche y la paleta fija de carta.** El tema noche usa un fondo casi negro con violeta (`#0e0a17` / acento `#9b5de5`), mientras que la carta siempre es pergamino/bronce (`#f4e9cd` / `#8a6a2f`), sin importar el tema. Esto puede leerse como un contraste elegante (una carta física iluminada sobre una mesa oscura, que es literalmente la metáfora que persigue el proyecto) o como un choque de dirección de arte (fantasía de pergamino envejecido conviviendo con un chrome de interfaz que tira a estética arcana/neón). No se puede confirmar sin verlo renderizado — vale la pena revisarlo en pantalla real con el toggle de noche activado antes de decidir si hace falta ajustar el violeta del acento hacia un tono más cálido/dorado que dialogue mejor con el marco de la carta.

### 2.4 Navegación móvil: el hallazgo más concreto

En `base.css`, la media query de 820px hace `display: none` completo sobre `.gm-sidebar` y oculta las etiquetas de texto de `.gm-navlink`, dejando solo iconos en la barra superior. No existe ningún botón de menú, drawer lateral ni alternativa: en móvil, un usuario **pierde el acceso a la navegación por secciones de la sidebar** (Clases, Razas, Trasfondos, Habilidades, Equipo, Dotes, Personajes) salvo que ya sepa la URL o que el header exponga esos mismos enlaces (lo cual no está confirmado en el CSS revisado). Esto es una regresión de usabilidad real, no una cuestión de gusto — cualquier referente profesional (D&D Beyond, Pathfinder Nexus, Roll20) resuelve el equivalente de esta sidebar con un menú hamburguesa que sigue dando acceso a las mismas secciones en móvil.

### 2.5 Comparación con referentes profesionales

| Aspecto | dndWeebCC (estado actual) | D&D Beyond | MTG Arena / Gatherer | Hearthstone |
|---|---|---|---|---|
| Ilustración de carta | Ausente (zona ② pendiente, reconocida en el propio GDD) | Iconos de clase/hechizo, sin arte por objeto | Arte a color en cada carta, ocupa ~40-60% del espacio | Arte a página completa, domina la lectura de la carta |
| Jerarquía de texto | Muy sistemática (5 zonas) pero con metadatos hasta en 8-10px | Texto de regla siempre legible, sin miniaturización agresiva | Tipografía de regla fija ~14-16px real, nunca microtexto | Casi sin texto: se apoya en iconografía |
| Ficha de personaje | Metáfora de "mesa" con silueta de equipo — más física que la mayoría | Lista/tabla tradicional de hoja de personaje | No aplica (no es un RPG de personaje persistente) | No aplica |
| Navegación móvil | Sidebar desaparece sin alternativa | Menú hamburguesa completo | App nativa, no aplica el patrón sidebar | App nativa |
| Consistencia de plantilla entre tipos de carta | Alta (mismas 5 zonas en los 6 catálogos) | Media (cada tipo de recurso tiene su propio layout) | Alta (todas las cartas comparten marco) | Alta |
| Tema claro/oscuro con identidad de carta fija | Sí, deliberado | Solo tema oscuro/claro de interfaz, sin "cartas" como objeto separado | No aplica (cartas físicas o escaneadas) | Tema único |

**Lectura general:** el sistema actual está por delante de la mayoría de referentes en *consistencia estructural* (la plantilla de 5 zonas es más disciplinada que la de D&D Beyond, que mezcla layouts distintos por tipo de recurso) y en la *metáfora física* de la ficha de personaje (la silueta de equipo no tiene equivalente directo en ninguno de los cuatro referentes). Está claramente por detrás en *peso visual del arte* — es la brecha más grande y la más citada en el propio GDD — y tiene un problema de accesibilidad real y concreto en navegación móvil que ningún referente profesional tiene. El microtexto (8-11px) y el acordeón por defecto en fichas de detalle son fricciones menores pero fácilmente corregibles sin rehacer nada estructural.

### 2.6 Recomendaciones concretas, en orden de esfuerzo/impacto

1. **Bajo esfuerzo, alto impacto:** subir el suelo tipográfico de los badges de regla (peso, rareza, tier) de `--fs-micro` a `--fs-small`; dejar `--fs-micro` solo para el identificador técnico.
2. **Bajo esfuerzo, alto impacto:** añadir un botón de menú móvil que abra la sidebar como drawer superpuesto en vez de ocultarla.
3. **Medio esfuerzo, alto impacto:** en las páginas de detalle de una sola carta (no en el álbum), quitar el acordeón y mostrar todas las secciones expandidas por defecto — el usuario ya decidió entrar a ver esa carta entera.
4. **Medio esfuerzo, impacto acumulativo:** extraer los bloques de cabecera de página repetidos a una clase `.gm-page-header` reutilizable.
5. **Alto esfuerzo, el impacto más visible de todos:** resolver la zona ② (ilustración), aunque sea con un primer paso intermedio de ilustraciones genéricas por rol/tipo en vez de arte único por carta — es la diferencia más grande entre esta interfaz y cualquier producto de cartas coleccionables real.
