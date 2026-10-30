# MotorGraphico · Modelo visual del juego

28 de septiembre de 2026 · Continúa [ESTUDIO_UI_UX.md](ESTUDIO_UI_UX.md) · Propuesta para revisión de Oriol · Sin cambios en el repositorio

Este documento convierte el estudio en un sistema aplicable: tokens, dos voces tipográficas, componentes con sus estados, seis plantillas y 16 maquetas a 1280×720 del juego completo. Cada error o incongruencia detectado tiene una regla o un componente que lo resuelve (§7). Decisiones de partida, tomadas el 28/09:

- **Entregable:** especificación y maquetas. Las maquetas son HTML/CSS renderizado a PNG sobre fondos reales del juego; no son código del motor.
- **Tipografía:** píxel + lectura. La fuente 5×7 del motor queda para títulos, pestañas, teclas, insignias y cifras. Los textos se leen en Lora.
- **Alcance:** juego completo (título, creación, HUD y diálogo, menú TAB, pausa y opciones, tienda, negocio, combate y derrota). Los editores quedan para otra ronda.
- **Idioma:** interfaz en castellano, con glosario catalán al lado (§9).

## 1. Qué hay en la carpeta

| Ruta | Contenido |
|---|---|
| `MODELO_VISUAL.md` | Este documento. |
| `maquetas/M01…M09*.png` | 16 pantallas propuestas, 1280×720. |
| `maquetas/S1_tokens.png`, `S2_componentes.png` | Láminas del sistema: paleta medida, tipografía, espaciado, componentes y estados. |
| `maquetas/antes_despues/` | Una lámina por pantalla: captura de hoy frente a la propuesta, con los errores que corrige. |
| `maquetas/fuente/` | El HTML de cada maqueta, `sistema/tokens.css` y `sistema/componentes.css`, iconos, fondos y la fuente Lora (licencia OFL incluida). Se abre en cualquier navegador. |
| `maquetas/_generador/` | Scripts que escriben el HTML y lo renderizan (Python + Playwright), para rehacer las maquetas tras un cambio. |
| `maquetas/pergamino/` | Variante pergamino en validación (§13): 4 maquetas, lámina del tema, comparativas con el modelo oscuro y el prototipo de pasar hoja (HTML, GIF y MP4). |

**Cómo se hicieron.** Los fondos son capturas del juego con el HUD apagado (parche local `CAPTURAS_SIN_HUD`, igual que los parches de la galería de capturas: no está en el repo). El texto pixel se dibuja con los mismos glifos 5×7 de `BitmapFont.cpp`, con los ajustes de §4.2. Los iconos salen de `assets/textures/iconos.png` a escala ×2 o ×1. El minimapa se genera desde la colisión del TMX y los objetos de `ciudad_centro.json`. Nombres, textos, precios y reglas salen del repo; los valores inventados para ilustrar un estado se listan en §8.

## 2. Principios

1. **Un solo foco, visible sin color.** Fila con foco = fondo `--panel-3` + barra dorada de 4 px + marcador ▶ + peso 600. Nunca hay dos cursores a la vez. ENTER ejecuta lo que tiene el foco.
2. **Dos voces tipográficas.** El pixel nombra y cuenta: títulos, pestañas, teclas, insignias, cifras. Lora se lee: nombres de contenido, descripciones, diálogos, motivos.
3. **Cada región reserva su altura.** Cabecera, navegación, cuerpo y pie no se pisan. Nada se dibuja por coordenadas sueltas encima de otro control.
4. **Paneles opacos, mundo atenuado.** Un panel con texto nunca es translúcido. Detrás de un menú, el mundo queda bajo un velo del 72 %.
5. **La respuesta aparece junto a la acción.** Comprar, guardar o subir el alquiler se confirma en la misma ventana. El registro queda para el historial.
6. **La interfaz no inventa.** Todo dato sale del catálogo, del código o de la partida. Si falta, se muestra un estado vacío honesto y se anota como contenido pendiente.

## 3. Tokens

Fichero: `maquetas/fuente/sistema/tokens.css`. Lámina: `S1_tokens.png`.

### 3.1 Color

Contraste medido con la fórmula de WCAG 2.2 contra `--panel`. Referencia: texto ≥ 4,5:1; barras, marcos y foco ≥ 3:1. Es una medición de la paleta, no una certificación del juego.

| Token | Valor | Uso | Contraste |
|---|---|---|---|
| `--fondo` | `#151820` | Pantalla completa: título, asistente | — |
| `--panel` | `#222735` | Cuerpo de ventana. Siempre opaco (hoy `kPanelColor` tiene alfa 0,82) | — |
| `--panel-2` | `#2B3142` | Cabecera, pie, pestaña inactiva, tarjeta | — |
| `--panel-3` | `#353C51` | Fila con foco | — |
| `--pista` | `#12141B` | Fondo de barras y deslizadores | — |
| `--velo` | `rgba(10,11,16,.72)` | Mundo detrás de un menú | — |
| `--marco-ext` | `#0B0D12` | Línea exterior del marco (2 px) | 9,6:1 sobre suelo claro (medido con `#C9B48A`) |
| `--marco-int` | `#5A6386` | Línea interior del marco (2 px) | 3,0:1 sobre `--fondo` |
| `--divisor` | `#3A4158` | Separadores internos (decorativo) | — |
| `--texto` | `#F3EBDD` | Texto principal | 12,6:1 |
| `--texto-2` | `#C1C9D6` | Secundario, descripciones | 8,9:1 |
| `--texto-3` | `#8F99AB` | Terciario, etiquetas de campo | 5,2:1 (4,5:1 sobre panel-2; **3,8:1 sobre panel-3**) |
| `--acento` | `#EBC77D` | Foco, selección, acción principal | 9,2:1 |
| `--exito` | `#91D7B0` | Hecho, cumplido, tu negocio | 8,9:1 |
| `--aviso` | `#F2A65A` | Condición que bloquea (faltan puntos, falta oro) | 7,4:1 |
| `--error` | `#FFB2AC` | Algo falló (no se ha podido guardar) | 8,7:1 |
| `--info` | `#9CC3FF` | Novedad neutra (diario actualizado) | 8,3:1 |
| `--pv` / `--pm` / `--moral` | `#E8705F` / `#6A9BF4` / `#B79CF0` | Barras de recurso | 5,5 / 6,1 / 7,2:1 sobre `--pista` |

Reglas de color:

- **El dorado es foco, no decoración.** Los títulos van en `--texto`. El oro del jugador se muestra con el icono de moneda, no en dorado.
- **`--aviso` pasa de `#F0C77C` a `#F2A65A`.** El valor del estudio era casi idéntico a `--acento` (`#EBC77D`): un aviso se habría confundido con el foco.
- **`--texto-3` nunca va sobre la fila con foco.** Allí baja a 3,8:1, así que el texto secundario de esa fila sube a `--texto-2`.
- **La barra de PV no cambia de color por tramos** (hoy verde, amarillo y rojo). Cuando queda un 25 % o menos, la cifra pasa a `--aviso` y aparece «!».

### 3.2 Marco y velo

Marco pixel de dos tonos: 2 px exteriores `--marco-ext` (separa el panel de un suelo claro) y 2 px interiores `--marco-int` (lo separa del fondo oscuro), con las esquinas recortadas 2 px. Con las dos líneas, el panel se distingue sobre cualquier suelo del juego sin depender de la transparencia.

### 3.3 Espacio y medidas

Escala 4 / 8 / 16 / 24 / 32 / 48. Área segura de 24 px. Ventana de gestión de hasta 1120×600, cabecera 56, navegación 44, pie 56 y fila 44 (como propone el estudio). Diálogos de 520 a 680 px de ancho. Iconos del atlas a ×2 (32 px) en listas y detalle, y a ×1 (16 px) en pestañas, insignias y cifras.

## 4. Tipografía

### 4.1 Roles

| Voz | Rol | Escala / tamaño | Ejemplo |
|---|---|---|---|
| Pixel 5×7 | Título de ventana o pantalla | ×3 (21 px de letra) | TIENDA, EN PAUSA, NUEVA PARTIDA |
| Pixel 5×7 | Logo | ×6 | LOS PERDIDOS DE BOUNDINGTON |
| Pixel 5×7 | Pestañas, etapas, insignias, teclas, rótulos de bloque | ×2 | INVENTARIO, EN VENTA, ENTER |
| Pixel 5×7 | Cifras en listas y HUD (monoespaciadas: se alinean solas) | ×2, ×3 en totales | 28/28, 30, ×2 |
| Lora 600 | Nombre en un detalle, pregunta del asistente | 28 px | Aarakocra de las Montañas |
| Lora | Entradas de menús cortos (título, pausa, derrota) | 22–24 px | Nueva partida |
| Lora | Filas de lista, texto de lectura, efectos | 20 px | Recupera 15 PV. |
| Lora | Diálogo | 22 px, máximo 60 caracteres por línea | Antes esta plaza era un huerto. |
| Lora | Motivos, ayudas, pie de teclas | 18 px | No hay partidas guardadas |
| Lora | Solo metadatos no esenciales (leyenda, «Paso 4 de 14», etiqueta de campo) | 16 px | Paso 4 de 14 |

Reglas:

- El pixel siempre en mayúsculas (el atlas no tiene minúsculas) y a escala entera. Nunca para párrafos ni para nombres de contenido largos.
- Lora en caja normal, con la ortografía completa: tildes, ñ, ç, l·l, «», ¿?, ¡!
- Los números que se leen en una frase van en Lora («Te quedarán 90 de oro»). Los que se comparan en columna van en pixel.
- Verbo concreto en botones y pies: Usar, Comprar, Guardar, Cargar. Nunca «Confirmar» ni «OK».

### 4.2 Ajustes propuestos para la fuente pixel del motor

Al ampliar las capturas aparecen dos defectos de `BitmapFont` que ningún color arregla. En `S1_tokens.png` se ven lado a lado.

1. **Letras pegadas.** `drawText` avanza `m_glyphWidth` = 5 × escala y los glifos usan las 5 columnas, así que las letras vecinas se tocan: «INVENTARIO» y «POCION» se leen apelmazadas (04-01). **Propuesta:** avance de 6 columnas (5 + 1 de aire). Cambian `measureText`, `wrapText` y sus pruebas (`demo_bitmap_font.cpp`).
2. **Tildes aplastadas.** Las mayúsculas acentuadas meten la tilde en la fila 0 y comprimen la letra en 6 filas, y la tilde queda tocando la letra. «REVISIÓN» se lee «REVISION», y en la precuela «LEÑA HÚMEDA» se lee «LENA HUMEDA» (12-01). El texto sí lleva la ñ; lo que falla es el dibujo, no los datos. **Propuesta:** la letra conserva sus 7 filas y la tilde sube al interlineado (fila −2, con la −1 libre); la línea ya mide 9 filas (`kLineSpacing` = 2). Agudo `00110`, grave `01100`, diéresis `01010`, virgulilla de dos filas `01101/10010`.
3. **Glifos que faltan.** Se proponen, con el mismo encaje, los que usan las maquetas o el catalán: `Ï « » ← → ↑ ↓ ▶ ◀ ✓ × ● ▲ ■ ◆`. Sin «Ï» no se puede escribir DEÏTAT en pixel.

### 4.3 Fuente de lectura: Lora

- **Por qué.** Serifa de lectura con aire caligráfico, que encaja con la fantasía sobria. Tiene buena altura de x a 18–22 px y cubre castellano y catalán: á é í ó ú ñ ç l·l « » ¿ ¡. Trae cifras tabulares (`tnum`).
- **Licencia.** SIL Open Font License 1.1. Se puede empaquetar con el juego si se incluye la licencia (`maquetas/fuente/fuentes/OFL.txt`). Versión probada: Lora 3.008 variable (400–700).
- **Alternativa si no convence:** cualquier fuente OFL con la misma cobertura. La regla de dos voces no depende de Lora.

## 5. Componentes

Fichero: `maquetas/fuente/sistema/componentes.css`. Lámina: `S2_componentes.png`.

| Componente | Anatomía | Estados y reglas | Se ve en |
|---|---|---|---|
| **Ventana** | Marco pixel; cabecera 56 (título pixel ×3 + contexto a la izquierda, datos a la derecha), navegación 44, cuerpo, pie 56 | La ventana no crece con el texto: el cuerpo desplaza su contenido | M04, M05, M06, M07 |
| **Pestañas** | Fila horizontal dentro de la ventana; icono ×1 + etiqueta pixel ×2; teclas [←] [→] en los extremos | Activa: fondo `--panel`, línea dorada arriba y unida al cuerpo. Inactiva: `--texto-2`. La pestaña activa no es un segundo cursor | M04, M06 |
| **Fila de lista** | Marcador 16 · icono 32 · nombre (Lora 20) y motivo · valor a la derecha | Normal, **foco**, **elegida** (✓ en `--exito`), **deshabilitada con motivo**, foco sobre deshabilitada. Alto mínimo 44; con motivo, dos líneas | Todas |
| **Detalle** | Nombre (Lora 28) + insignias · efecto con icono · datos en rejilla · acciones abajo | La acción principal y su motivo viven aquí, no en el pie | M02a, M04a, M06 |
| **Botón** | Etiqueta pixel ×2, alto 44 (36 en pies) | Principal (dorado), secundario, deshabilitado. El foco de botón es un contorno crema de 2 px. Si el foco está en la lista, el botón principal no lleva contorno | M02c, M06, M07 |
| **Tecla y ayuda** | Tecla: caja 28 px con relieve inferior y texto pixel. Ayuda: tecla + verbo en Lora 18 | Las ayudas salen del mapa de acciones real; nunca se escribe «A/D» si funcionan las flechas | Todas |
| **Insignia** | Etiqueta pixel ×2 sobre fondo tintado | Neutra, éxito, info, aviso, error. Estados excluyentes: EN VENTA o TU NEGOCIO, nunca los dos | M02a, M07 |
| **Barra de recurso** | Icono + sigla + cifra **encima**; barra de 10 px debajo | La barra no tapa la etiqueta. 28/28 llena la barra. Con ≤ 25 %, la cifra pasa a `--aviso` con «!» | M03, M08 |
| **Escala de moral** | Sigla + tramo (VILLANO…HÉROE) + escala de −100 a 100 con el centro marcado | El tramo se escribe con tilde (hoy `HEROE`). En la ficha, el valor entre paréntesis | M03a, M04b |
| **Contador** | [−] valor [+] | Límite alcanzado = botón deshabilitado. Se usa para atributos (1 a 6) y alquiler (50 % a 200 %) | M02b, M07b |
| **Deslizador** | Pista con 11 marcas (0–10), relleno y mando; porcentaje a la derecha | Relleno dorado solo en la fila con foco | M05b |
| **Selector** | ◀ valor ▶ | Idiomas con su nombre propio: Català, Español, English | M05b |
| **Aviso** | Barra lateral de color + icono o glifo + título + detalle | Éxito y novedad se van solos; aviso y error se quedan hasta resolverse | M03a, M05a, M07a, S2 |
| **Línea de estado** | Glifo + texto, dentro de la ventana, debajo de la acción | Resultado de la última acción de esa ventana. Se limpia al cambiar de sección | M05a, M06, M07b |
| **Diálogo** | Panel inferior de ancho fijo; pestaña con el hablante; texto Lora 22; pie con página N/M y tecla | Hablante vacío en los datos = «NARRACIÓN». La última página dice «Cerrar», no «Continuar» | M03b, S2 |
| **Aviso de interacción** | Panel pequeño con tecla + verbo + destino, con flecha hacia el objetivo, y rombo en su baldosa | Solo aparece si la acción existe. «Hablar con Anciana», «Ver negocio», «Ir a Plaza de las Razas» | M03a |
| **Rastreador de objetivo** | Rótulo OBJETIVO + contador, misión, paso actual, pista breve | Solo objetivos conocidos por la partida | M03a |
| **Minimapa** | Nombre de zona + mapa + leyenda | Formas distintas por tipo: ▲ tú, ● personaje, ■ entrada, ◆ objetivo seguido | M03a |
| **Etapas** | Etapas del asistente con número, ✓ al completarse y la actual subrayada | Paso N de 14 aparte, en texto pequeño | M02 |
| **Tarjeta de partida** | Icono de lugar + zona + tier + fecha, hora, tipo de guardado y personaje | Estados previstos: vacía, válida, incompatible y dañada | M09, S2 |
| **Estado vacío** | Icono + frase de qué falta + cómo conseguirlo | Nunca una sección vacía que parezca rota | S2 |

## 6. Plantillas

| Plantilla | Uso | Medidas a 1280×720 |
|---|---|---|
| **T1 · Menú corto** | Título, pausa, derrota | Panel de 400–520 px con filas de Lora 22–24. En pausa y derrota el mundo va detrás del velo |
| **T2 · Gestión lista/detalle** | Menú TAB, tienda | Ventana de 1120×600 (tienda: 940×544). Lista 38 %, detalle 62 %, línea de estado sobre el pie |
| **T3 · Asistente** | Nueva partida | 1232×672. Cabecera con etapas, pregunta en Lora 28, cuerpo lista/detalle y pie con Atrás y Continuar |
| **T4 · Ficha modal** | Negocio, resultado de combate | 560–640 px de ancho, cabecera con insignia de estado y pie propio |
| **T5 · Exploración** | HUD + mundo | Columna izquierda de 300 px (personaje y objetivo), minimapa arriba a la derecha y ayudas abajo a la derecha. En diálogo queda solo el bloque de PV y PM |
| **T6 · Combate** | Turno del jugador | Franja inferior de 1232×212 en cuatro columnas (personaje, acciones, detalle, últimas acciones) y placa del enemigo sobre él |

## 7. Errores e incongruencias → solución

Evidencias: capturas de `../capturas_menus_2026-09-28/`, código y datos del repo. **V** visible, **D** documentado, **C** comprobado ahora en código o datos.

### 7.1 Interfaz

| ID | Evidencia | Qué pasa | Regla o componente | Maqueta |
|---|---|---|---|---|
| E01 | 04-01 (V), `BitmapFont::drawText` (C) | Letras pixel pegadas: avance de 5 columnas sin aire | §4.2.1 avance 6 | S1 |
| E02 | 12-01, 02-01 (V) | Tildes y ñ ilegibles: «LEÑA» se lee «LENA» y «REVISIÓN» se lee «REVISION» | §4.2.2 tildes en el interlineado | S1, M02 |
| E03 | 02-11…15, 03-01…10 (V) | Todo en mayúsculas monoespaciadas: lectura pesada | Dos voces (§4.1): Lora para leer | M02a, M03b |
| E04 | Catálogos (C) | La mayoría del contenido está escrito sin tildes: «Pocion», «Eter», «Sastreria», «El Primer Dia en Boundington» | Contenido con ortografía completa (C10); la fuente ya la soporta | Todas |
| E05 | 05-01…16, 08-07, 09-01 (V), `PauseMenu.cpp`, `GameOverScreen.cpp`, `BattleState.cpp` (C) | Idiomas mezclados: «Reprender, Desar, Carregar, Opcions, Tornar al títol, Sortir», «GAME OVER», «Èxit/Fracàs/Crític», «efecte rebutjat». «Reprender» ni siquiera es catalán: en castellano significa regañar | Una lengua por sesión y glosario (§9) | M05a, M08a, M09 |
| E06 | 02-16, 04-09, 01-03, 05-14 (V) | Claves y rutas al jugador: `PERSONALITY_DE_LAGONIA_1`, `ASSETS/LEVELS/…JSON`, «(G-10)», «(H1-03)», «DAMAGE» | Nunca ids; si falta nombre, contenido pendiente (C06) | M02c, M04b |
| E07 | 02-01, 04-01, 06-02, 07-02, 07-06, HUD (V) | Textos, barras y pestañas se pisan | Regiones con altura reservada; barra con etiqueta encima | M02–M07 |
| E08 | 04-01 (V) | Las pestañas del TAB son una lista vertical flotante sobre el contenido | Pestañas horizontales dentro de la ventana | M04a, M04b |
| E09 | 02-xx, 04-01, 06-02 (V) | Ventanas enormes con dos filas; ayudas al fondo, lejos de la acción | T2 lista/detalle; acción y motivo en el detalle | M04a, M06 |
| E10 | Todas (V), `kPanelColor` alfa 0,82 (C) | Panel translúcido sobre suelo muy texturado | Panel opaco + velo 72 % | Todas |
| E11 | 01-06, 05-17 (V) | El panel del título pasa de unos 500 a 170 px según haya partida | Ancho estable; motivo en segunda línea | M01 |
| E12 | 01-01 (D) | El cursor visible no coincide con lo que ejecuta ENTER | Un solo foco, el que ejecuta; sin partida, en «Nueva partida» | M01 |
| E13 | 04-01 (V) | Dos cursores amarillos a la vez (sección y fila) | La sección activa es una pestaña, no un cursor | M04a |
| E14 | 06-03, 07-04, 08-04 (V) | Foco débil: flecha y amarillo sin fondo | Fila con foco completa (§2.1) | S2 |
| E15 | 01-06, 04-03, 02-10 (V) | Deshabilitado sin motivo, o con un motivo que ensancha la fila | Deshabilitado con motivo al lado; no consume | M01, M02b, M04a |
| E16 | 04-01 (V) | «ENTER: CONFIRMAR» no dice qué pasa | Verbo concreto en botón y pie | M04a, M06 |
| E17 | 05-05 (D) | Guardar no da respuesta visible | Línea de estado con lugar y hora; error con reintento | M05a |
| E18 | 06-04, 07-07 (V) | El resultado sale en el registro, lejos y mezclado con diálogo | Línea de estado en la ventana | M06, M07b |
| E19 | 04-04, 05-15 (V) | Mensajes heredados: aviso de maná en Equipo, hint de controles en pausa | La línea de estado pertenece a su sección | M04a |
| E20 | 08-09 (D) | Recoger un objeto no avisa | Aviso «Éter ×1 · recogido» | S2 |
| E21 | 03-13, 04-01 (V), LEEME (D) | HUD: «HEROE 150 ORO» parte la línea y sustituye al nombre; PV, PM y moral quedan tapados por sus barras; con 28/28 la barra de PV se pinta al 28 % y en amarillo; FPS siempre visibles | Bloque de personaje: nombre, tier, oro, barras con etiqueta encima y escala de moral; FPS solo en diagnóstico | M03a |
| E22 | 03-12/13 (V) | El encargo queda en prosa; no hay objetivo visible | Rastreador de objetivo | M03a |
| E23 | 03-14, 03-16, 06-01, 07-01 (V) | Nada indica con quién o con qué se puede interactuar | Aviso de interacción + rombo en la baldosa | M03a |
| E24 | 03-13…19 (V) | Minimapa de puntos de color sin leyenda | Formas distintas + leyenda + zona | M03a |
| E25 | 03-01…10, 03-15 (V) | Diálogo sin hablante; «ENTER CONTINUAR» fuera del panel | Diálogo con pestaña de hablante y pie dentro | M03b |
| E26 | 02-01…22 (V) | La cabecera «1/14» invade la primera opción; las ayudas pisan el detalle | T3 asistente con etapas | M02a–c |
| E27 | 02-06 (D), `PantallaNuevaPartida.cpp` (C) | La pista dice «A/D suben y bajan», pero en menús solo funcionan ←/→ | Ayudas generadas del mapa de acciones | M02b |
| E28 | 02-22 (V) | El resumen omite las siete elecciones de historia | Revisión en tres grupos con «Editar» | M02c |
| E29 | 06-04/06 (V) | Comprar y vender en la misma lista; la fila de venta aparece y mueve el foco | Pestañas Comprar / Vender | M06 |
| E30 | 06-05 (V) | Antes de vender no se ve cuánto se cobra (15) | Botón «VENDER 1 · RECIBES 15» en la pestaña Vender | §6 T2 |
| E31 | 07-06…08 (V) | Sigue «EN VENTA» junto a «TUYO» | Insignias excluyentes | M07a, M07b |
| E32 | 07-07/08 (V) | Las consecuencias del alquiler salen en el registro; «ciclo» no se explica | Contador + efecto calculado con `GameSession::occupancyPercent` e `incomeFor`; un ciclo son 40 pasos (`kStepsPerRentCycle`) | M07a, M07b |
| E33 | 08-03 (V) | Comandos en una esquina, vida del enemigo arriba y registro abajo: tres zonas lejanas | T6 franja inferior + placa del enemigo | M08a |
| E34 | 08-04/05 (V) | «Poción» sin cantidad ni efecto; «Huir» sin explicación | «Poción ×2» + detalle de la acción | M08a |
| E35 | 08-08 (V) | El combate termina sin resultado | T4 resultado: Victoria, Huida o Derrota | M08b |
| E36 | 05-00, 09-00 (V/D) | Pausa y derrota no se dibujan | T1 con velo y título | M05a, M09 |
| E37 | 05-09…14 (V) | Opciones: 8/10/5 sin escala; placeholder de controles; el idioma no cambia nada | Deslizador con porcentaje y ayuda; lista agrupada (§7.3 I5) | M05b |
| E38 | 09-01 (V) | La derrota no dice qué partida se recupera | Tarjeta de partida + «Cargar último guardado» | M09 |
| E39 | `Application.cpp` (C) | Teclas incoherentes: en menús W/S mueven pero ↑/↓ no existen; ←/→ mueven pero A/D no | Propuesta: en menús, ↑/↓ = W/S y A/D = ←/→ (en el mundo A/D siguen moviendo) | Pies de todas |

### 7.2 Contenido que la interfaz destapa

No se cambia el canon desde aquí: son avisos para autoría.

| ID | Dónde | Incongruencia | Propuesta |
|---|---|---|---|
| C01 | `PantallaNuevaPartida.cpp` | El primer nombre sugerido es «Luisarda», igual que la PNJ canónica «Luisarda, hija de Boundington», que tiene tienda: el jugador se compraría pociones a sí mismo | Sacar de la lista los nombres de PNJ canónicos. Las maquetas usan «Nerea», de la misma lista |
| C02 | `boundington_npcs.json` | Luisarda dice «Pociones a cincuenta» y la poción cuesta 30 | Ajustar el diálogo o el precio |
| C03 | `boundington_primer_dia.json`, `boundington_enemigos.json` | El mismo enemigo se llama «Saqueador de los Perdidos» (hablante del atraco) y «Perdido saqueador» (objeto) | Un solo nombre |
| C04 | `races.json` | El rasgo de Aarakocra de las Montañas cita «Sabiduría», un atributo que no existe (CON, DES, INT, CAR) | Revisar el rasgo |
| C05 | `backgrounds.json`, `races.json`, `deities.json` | Deidades citadas que no existen: el Mes de la Agonía cita `el_sepulturero_amable` y `la_madre_de_los_doce`; la raza cita «Sofía y Eros», que según `_correccion` eran los nombres anteriores de La Reina Velada y El Oráculo Fragmentado (Eros sale 16 veces en `races.json`) | Enlazar a ids existentes o crear las entradas |
| C06 | `CharacterCreation.cpp` | Las personalidades usan `title` y el cargador lee `name`: sin nombre, se pinta el id | Leer `title` como alternativa o renombrar el campo |
| C07 | `classes.json` y 04-04 | La clase define equipo inicial (arco, armadura, botas), pero Equipo muestra siete ranuras vacías | Entregarlo o no prometerlo en la revisión |
| C08 | `ciudad_centro.json` | El nivel se llama «Los Perdidos de Boundington», igual que el juego | Nombre de zona propio; las maquetas usan «Centro de Boundington» |
| C09 | `races.json` | Razas en plural («Elfos del Bosque») y en singular («Elfo del Dosel») mezcladas; «Revenants» en inglés | Elegir forma para la ficha |
| C10 | Aventuras y objetos | De 2 973 cadenas de 12 o más caracteres en aventuras, objetos de ciudad y de Boundington, solo 34 llevan tilde, ñ o ¿¡. Ejemplos: «Manana no lo sera», «Los jovenes ya no saludan», «Fanatico del culto» | Pasada ortográfica del contenido |
| C11 | `classes.json` y 04-06 | La clase trae tres habilidades (Golpe en Picado, Garfio Ligero, Tejado Amigo), pero Acciones sale vacía y el combate solo ofrece Atacar, Poción y Huir | Conectar Acciones a las habilidades o mostrar un estado vacío honesto |
| C12 | `Morality.h`, `races.json` | La moral usa etiquetas sin tilde («HEROE») por una limitación de la fuente que ya no existe. Las razas traen alineamientos de D&D («Neutral Bueno o Caótico Bueno») que no son el eje único de moral | Un solo vocabulario de moral |
| C13 | `boundington_primer_dia.json` | La derrota del atraco no es fatal (`tut_derrota`: −15 de oro y la historia sigue) | El cierre de combate distingue «Derrota: la historia sigue» de «Has caído» (M09) |

### 7.3 Incongruencias entre los documentos del estudio

| ID | Dónde | Decisión del modelo |
|---|---|---|
| I1 | ESTUDIO habla de 4 etapas de creación; ANALISIS_JUGADOR, de 5 | 4 etapas de decisión (Origen, Atributos, Historia, Identidad) + Revisión. El indicador muestra las cinco marcas |
| I2 | La sección se llama «Estado» en el código y en ESTUDIO, y «Personaje» en ANALISIS_JUGADOR | «Ficha». «Personaje» ya es la entrada de pausa que abre el menú y «Estado» choca con los estados (condiciones) |
| I3 | Cabecera de tienda: «Luisarda · Sastrería» o «Luisarda · Tienda» | El tipo de ventana en pixel (TIENDA) + el comerciante en Lora. El lugar va en el rótulo de zona |
| I4 | Pausa con 6 entradas (ESTUDIO) o 7 (JUGADOR) | 7, con un separador antes de «Volver al título» y «Salir del juego» |
| I5 | Categorías de opciones distintas en cada documento | Lista agrupada (Sonido · Idioma y texto · Controles): ←/→ ya ajustan valores, así que las pestañas chocarían con ellos. Lo de accesibilidad va en «Idioma y texto» cuando exista |
| I6 | «La aventura ha terminado» o «Has caído» | Título «HAS CAÍDO» y una línea que dice qué se puede recuperar |
| I7 | Los solapes son P1 en ESTUDIO y P0 en COMERCIO_COMBATE | Se mantiene la prioridad del ESTUDIO, que es el documento rector |
| I8 | Aviso `#F0C77C` casi igual al acento `#EBC77D` | Aviso `#F2A65A` (§3.1) |
| I9 | El estudio propone Esc = volver un nivel; hoy Esc abre la pausa o sale | Las maquetas usan la propuesta. Implantarla exige cambiar el mapa de acciones (lote B) |
| I10 | ESTUDIO §5.04 dice «esta alternativa se muestra en el prototipo», pero no había prototipo | Ahora sí: M04a y M04b |

## 8. Maquetas

Valores de ejemplo (ilustran un estado; no salen de una partida real): la hora 17:42 y el tipo de guardado de M05a y M09; PV 19/28 tras la victoria; 120 de oro y 3 pociones en M06; la Sastrería ya comprada y al 110 % en M07b (hacen falta 1000 de oro); el reparto a medias de M02b; el nombre «Nerea». Son **propuestas sin soporte hoy**: el buscador de razas, «Tamaño del texto», «Ver teclas», «Elegir partida» y la variante azul del icono para el Éter.

| Maqueta | Captura de hoy | Qué demuestra |
|---|---|---|
| `M01_titulo` | 01-01 | Portada con lugar real, foco inicial correcto y motivo en segunda línea |
| `M02a_creacion_raza` | 02-01 | Asistente: etapas, pregunta, lista buscable 1/43 y detalle separado en juego y cultura |
| `M02b_creacion_atributos` | 02-08 | Contadores, bonificación de raza, total y «Continuar» bloqueado con motivo |
| `M02c_creacion_revision` | 02-22 | Las 14 decisiones en tres grupos, con Editar y Empezar aventura |
| `M03a_hud_exploracion` | 03-14 | HUD completo, objetivo, minimapa real, aviso de interacción y aviso de nuevo objetivo |
| `M03b_dialogo` | 03-15 | Diálogo con hablante y página 1/2; HUD reducido |
| `M04a_tab_inventario` | 04-01 | Seis pestañas, lista/detalle y acción deshabilitada con motivo |
| `M04b_tab_ficha` | 04-05 | Identidad, estado, atributos con su origen, rasgo canónico e historia personal |
| `M05a_pausa_guardado` | 05-05 | Pausa visible, siete entradas y «Partida guardada» junto a la acción |
| `M05b_opciones` | 05-09 | Lista agrupada, deslizadores con porcentaje e idioma con su nombre propio |
| `M06_tienda` | 06-04 | Comprar/Vender en pestañas, saldo posterior y resultado local |
| `M07a_negocio_en_venta` | 07-03 | Ficha legible, ciclo explicado y motivo antes de comprar |
| `M07b_tu_negocio` | 07-07 | Estado propio excluyente y alquiler con efecto calculado por el juego |
| `M08a_combate` | 08-03 | Franja inferior unificada, objetivo marcado y detalle de la acción |
| `M08b_victoria` | 08-08 | Resultado con lo que cambió y objetivo cumplido; sin botín inventado |
| `M09_derrota` | 09-01 | Derrota en castellano, tarjeta de la partida y salidas claras |

El combate se montó en la plaza con el Perdido saqueador, que es el atraco del tutorial (`tut_emboscada`). Hoy ese combate no llega a lanzarse (LEEME de la galería), así que el nivel `zz_capturas_emboscada.json` es una copia de `ciudad_centro` con el enemigo en (34,32), solo para la foto. Así se evita usar la galería de pruebas y su suelo de rayas.

## 9. Textos y glosario

### 9.1 Textos fijos del juego

| Situación | Texto |
|---|---|
| Continuar sin partida | «No hay partidas guardadas» |
| Usar con PV llenos | «Tus PV ya están al máximo. No se gasta nada.» |
| Guardar | «Partida guardada · [zona] · [hora]» / «No se ha podido guardar · Pulsa ENTER para reintentar» |
| Comprar | «Comprar 1 · 30 oro» → «Poción comprada · −30 de oro · ahora tienes N» |
| Vender | «Vender 1 · Recibes 15» |
| Falta oro | «Te faltan 850 de oro» |
| Atributos | «Te quedan N puntos por repartir» |
| Interacción | «[E] Hablar con [nombre]» · «[E] Ver negocio» · «[E] Ir a [zona]» |
| Registro de combate | «[Nombre] ataca a [objetivo] · Éxito · 6 de daño» |
| Recogida | «Éter ×1 · recogido» |

### 9.2 Glosario castellano → catalán

El catalán usa letras que el pixel debe tener: À È É Í Ï Ò Ó Ú Ü Ç y l·l (la Ï es un glifo nuevo, §4.2).

| Castellano | Català | Castellano | Català |
|---|---|---|---|
| Continuar | Continuar | Nueva partida | Partida nova |
| Cargar partida | Carregar la partida | Guardar partida | Desar la partida |
| Partida guardada | Partida desada | Opciones | Opcions |
| Reanudar | Reprendre | Volver al título | Tornar al títol |
| Salir del juego | Sortir del joc | En pausa | En pausa |
| Personaje | Personatge | Ficha | Fitxa |
| Inventario | Inventari | Equipo | Equipament |
| Acciones | Accions | Misiones | Missions |
| Códice | Còdex | Objetivo | Objectiu |
| Nuevo objetivo | Nou objectiu | Objetivo cumplido | Objectiu complert |
| Usar | Fer servir | Cerrar | Tancar |
| Aceptar | Acceptar | Elegir | Triar |
| Volver | Tornar | Buscar | Cercar |
| Sección | Secció | Paso 4 de 14 | Pas 4 de 14 |
| Raza | Raça | Clase | Classe |
| Trasfondo | Rerefons | Atributos | Atributs |
| Constitución | Constitució | Destreza | Destresa |
| Inteligencia | Intel·ligència | Carisma | Carisma |
| Puntos por repartir | Punts per repartir | Rasgo | Tret |
| Vínculo | Vincle | Miedo | Por |
| Defecto | Defecte | Meta | Meta |
| Ideal | Ideal | Personalidad | Personalitat |
| Virtud | Virtut | Deidad | Deïtat |
| Sin deidad | Sense deïtat | Nombre | Nom |
| Revisión | Revisió | Empezar aventura | Començar l'aventura |
| Moral | Moral | Héroe / Justo | Heroi / Just |
| Neutral | Neutral | Canalla / Villano | Canalla / Vilà |
| Condiciones | Condicions | Ninguna | Cap |
| Tienda | Botiga | Comprar / Vender | Comprar / Vendre |
| Tu oro | El teu or | Te faltan 850 de oro | Et falten 850 d'or |
| Negocio | Negoci | En venta | En venda |
| Tu negocio | El teu negoci | Alquiler | Lloguer |
| Ocupación | Ocupació | Ciclo | Cicle |
| Poción | Poció | Éter | Èter |
| Elige una acción | Tria una acció | Atacar | Atacar |
| Huir | Fugir | Daño | Dany |
| Éxito / Parcial | Èxit / Parcial | Fracaso / Crítico | Fracàs / Crític |
| Victoria | Victòria | Has caído | Has caigut |
| Cargar último guardado | Carregar l'última partida desada | Elegir partida | Triar partida |
| No hay partidas guardadas | No hi ha cap partida desada | No hay botín | No hi ha botí |
| Hablar con | Parlar amb | Narración | Narració |
| Música / Efectos / Ambiente | Música / Efectes / Ambient | Idioma | Llengua |
| Tamaño del texto | Mida del text | Ver teclas | Veure les tecles |

## 10. Llevarlo al motor

Son notas para el lote B del estudio, no una orden de implementación. Antes de tocar nada, se revalida el P0 del lote A sobre una compilación identificada, porque hay otras sesiones trabajando a la vez en el repo.

1. **Tokens sin OpenGL.** Los colores y medidas pasan a constantes en una unidad libre de GL, partida en grupos de 15 miembros como máximo (superficies, texto, estados, medidas), como pide AGENTS.md. Sustituyen a `kPanelColor`, `kBorderColor` y a los colores sueltos de las barras.
2. **BitmapFont.** Avance de 6 columnas y tildes en el interlineado (§4.2). Cambia la medida de todos los textos pixel: conviene hacerlo antes de recolocar paneles. Hay que añadir los glifos nuevos.
3. **Fuente de lectura.** Se hornea Lora a 16/18/20/22/28 px en un atlas al arrancar. El repo ya trae `third_party/imgui/imstb_truetype.h`; para el juego, que no enlaza ImGui, es más limpio añadir `stb_truetype.h` a `third_party/stb`. La carga y la medida van sin GL y la textura en la capa GL. Con ajuste de línea por ancho en píxeles, no por número de glifos.
4. **Componentes.** Uno por unidad (`HudVentana`, `HudPestanas`, `HudFilaLista`, `HudBarraRecurso`, `HudAviso`, `HudDialogo`…), cada fichero por debajo de 500 líneas. El estado (foco, selección, motivo) vive en la capa de juego sin GL, como ya hacen `PauseMenu` y `OptionsState`.
5. **Mapa de acciones.** Las ayudas de los pies se generan desde el mismo mapa que procesa el input (E27). Se añaden los alias de E39 y, si se aprueba, Esc como «volver un nivel» (I9).
6. **Datos.** Lectura de `title` en las personalidades (C06), pasada ortográfica (C10) y los avisos de §7.2 a quien lleve el contenido.
7. **Aceptación.** Las 16 maquetas son la referencia visual. Una pantalla se da por buena cuando su captura con el harness de la galería cumple la lista «Aceptar cuando» del estudio y no hay solapes a 1280×720. Una maqueta no cierra producto: cuenta lo que llega a `juego`.

## 11. Fuera de esta ronda

- **Editores** (`motor_editor`, `level_editor`): el estudio ya tiene su análisis (ANALISIS_EDITORES.md). Reutilizarán tokens, marco y tipografía, con filas de 32–36 px.
- **Pantallas sin evidencia** (ESTUDIO §7): selector de guardados completo, remapeo de controles, respuestas de diálogo ramificadas, selección de habilidad y objetivo, inventario lleno.
- **Pestañas del TAB no maquetadas:** Equipo, Acciones, Misiones y Códice siguen la misma plantilla T2. El estado vacío de Acciones está en S2.
- **Prueba con usuarios:** sigue la propuesta de ESTUDIO §9. Estas maquetas sirven como prototipo en papel para esa sesión.

## 12. Cómo regenerar las maquetas

En `maquetas/_generador/`: `lib.py` (texto pixel, iconos, componentes y render), `pantallas.py` (M01–M09), `sistema.py` (S1 y S2), `comparativas.py` (antes/después) y `minimapa.py`. Necesita Python 3, Pillow y Playwright con Chromium. `python3 pantallas.py M06` rehace una maqueta. Las rutas del principio de cada script apuntan al entorno donde se generaron y hay que ajustarlas. Para ver el HTML de `maquetas/fuente/` basta con abrirlo en el navegador: la fuente Lora va incluida.

## 13. Variante pergamino (muestra para validar)

28/09/2026 · Petición de Oriol: una estética de pergamino y fantasía en lugar de paneles oscuros genéricos, y que al cambiar de pestaña se pase la hoja. Decisiones tomadas:

- **Pasar hoja:** la hoja cae de arriba abajo, con la bisagra en el borde inferior. Al volver a una pestaña anterior, esa hoja sube y tapa la actual.
- **Alcance del pergamino:** menús, tienda y diálogos en pergamino completo; el HUD de exploración en retales pequeños, para no tapar el mundo.
- **Ritmo:** primero una muestra de cuatro pantallas y el prototipo. El resto se pasa al pergamino cuando se apruebe la dirección.

Se mantiene todo lo de §2 a §9: estructura, medidas, componentes, dos voces tipográficas, un solo foco, textos y la tabla de errores. Cambian los materiales, la paleta, el color del foco y la transición entre pestañas.

### 13.1 Materiales

| Material | Uso | Regla |
|---|---|---|
| **Cuaderno** (tapas y lomo de cuero, hojas de pergamino) | Menús con pestañas: menú del jugador (6 hojas), tienda (Comprar y Vender) | Cada pestaña es una hoja con su número (1/6). Es el único material que pasa hoja |
| **Papiro** (hoja con rodillos de madera y remates de latón) | Menús de una sola página: título, pausa, opciones, derrota, fichas modales | Sin animación de pestaña; aparece con un fundido de 150 ms como máximo |
| **Hoja** (pergamino suelto de canto rasgado) | Diálogos, avisos y fichas | El hablante va en un rótulo de cuero |
| **Retal** (trozo pequeño de pergamino) | HUD de exploración: personaje, objetivo, minimapa, ayudas | Tamaño mínimo; el minimapa se dibuja a tinta sobre el propio retal |

Las texturas son pixel art generado (`_generador/pergamino/texturas.py`): paleta de seis tonos de papel, tramado Bayer y escala ×2 como los sprites. No hay arte de terceros; se pueden sustituir por arte final.

### 13.2 Paleta

Contraste medido sobre el papel base `#E2CD9C`. El texto va siempre sobre papel base o claro: el margen interior de 24 px deja fuera los cantos oscuros y las manchas.

| Token | Valor | Uso | Contraste |
|---|---|---|---|
| `--tinta` | `#2B1D12` | Texto principal | 10,5:1 |
| `--tinta-2` | `#553E2B` | Secundario | 6,4:1 |
| `--tinta-3` | `#6B5038` | Terciario, etiquetas | 4,8:1 |
| `--carmin` | `#9A2A1C` | **Foco**, selección y acción principal (lacre) | 4,9:1 |
| `--exito` / `--aviso` / `--error` / `--info` | `#2E6337` / `#7A4409` / `#8E1B12` / `#29507A` | Estados | 4,6 / 5,1 / 5,8 / 5,3:1 |
| `--pv` / `--pm` / `--moral` | `#A83A2A` / `#2F5E9E` / `#6A4A8C` | Barras sobre fondo `#F2E4C1` con contorno de tinta | 5,0 / 5,2 / 5,6:1 |
| `--cuero` + `--claro-cuero` | `#4E321D` + `#F2E4C1` | Texto sobre las tapas | 9,3:1 |
| `--linea` | `#4A3421` | Contorno de todo material: lo separa del suelo arenoso | 5,8:1 sobre arena |

Reglas nuevas del tema:

- **El foco es una cinta de marcapáginas carmín** a la izquierda de la fila, con un lavado carmín del 10 % y el texto en 600. Sustituye al marcador ▶ dorado: el dorado del tema oscuro (`#EBC77D`) da 1,0:1 sobre el papel y desaparece.
- **El error también es rojo**, así que nunca va solo: lleva «!» y su propia caja. El foco es una cinta; el error, un aviso.
- **Botones:** el principal es lacre (carmín con texto claro, 6,1:1), el secundario es papel claro con contorno de tinta y el deshabilitado lleva el contorno discontinuo (señal de forma, no de color).
- **Adornos:** filetes y rombos de tinta desvaída para separar secciones. No se usa el carmín para decorar.

### 13.3 Pasar hoja

Se prueba en `maquetas/pergamino/prototipo_pasar_hoja.html` (se abre en el navegador, sin dependencias) y se ve en `pasar_hoja.gif` y `pasar_hoja.mp4`.

| Aspecto | Especificación |
|---|---|
| Bisagra | Borde inferior de la hoja, donde está el lomo del cuaderno |
| → siguiente | La hoja actual gira hacia el jugador de 0° a 180° y cae. Se oscurece hasta un 30 % y a partir de 90° se ve su reverso, más oscuro. La hoja nueva ya está debajo, con una sombra que se va aclarando |
| ← anterior | La hoja de la pestaña anterior sube de 180° a 0° y tapa la actual. Frena al final con un rebote de unos 5° |
| Duración y curvas | 360 ms. Al caer arranca y acelera (0,25·t + 0,75·t²); al subir frena y se asienta |
| Respuesta | La pestaña activa y los verbos del pie cambian al pulsar, no al terminar la vuelta |
| Pulsaciones seguidas | Una pulsación durante la vuelta la completa y encadena la siguiente. No se pierden pulsaciones ni se bloquea la entrada |
| Extremos | Sin vuelta circular: en la primera y la última hoja, ←/→ no hacen nada |
| Clic en una pestaña lejana | Una sola vuelta directa a esa hoja, sin pasar por las intermedias |
| Animaciones reducidas | Fundido de 160 ms (opción en Opciones → Idioma y texto) |
| Sonido | Roce de papel corto; el repo ya incluye miniaudio |

### 13.4 Llevarlo al motor

Son notas para el lote B y no autorizan implementar nada.

1. **Hoja a textura.** `Render::Framebuffer` ya existe (lo usa `IsometricRenderer`). La hoja saliente se dibuja una vez en un FBO del tamaño de la hoja y durante la vuelta se anima como un quad.
2. **Versión sencilla** (sin tocar `SpriteBatch`): escalar el quad en vertical desde la bisagra (alto × |cos θ|), oscurecerlo con el tinte y cambiar a la textura del reverso al pasar de 90°. Se lee como una hoja que cae.
3. **Versión con perspectiva:** añadir a `SpriteBatch` un envío de cuatro vértices libres para el trapecio (el borde de arriba se ensancha al acercarse). El `Vertex` actual ya tiene posición, UV y color.
4. **Paneles.** `HudPanel` no tiene 9-slice; hace falta para hoja y retal (cantos rasgados en esquinas y bordes, papel en mosaico) y para las bandas de cuero. Las maquetas usan texturas generadas a medida solo por fidelidad.
5. **Estado.** Página de origen, destino y progreso viven en la capa de juego sin GL, como `PauseMenu`; el dibujo, en la capa GL.

### 13.5 Pendiente si se aprueba

- Pasar al pergamino el resto de pantallas: título (papiro), tienda (cuaderno de dos hojas), negocio, opciones, combate, victoria y derrota.
- Decidir si el asistente de creación también pasa hoja: Continuar haría caer la hoja y Atrás la devolvería. Es una secuencia, no pestañas, y encaja con el libro del personaje.
- Decidir qué hacer en los extremos: ahora no hay vuelta circular; como alternativa, un pequeño tope visual.
- Revisar la muestra en una pantalla real: tamaño de los retales, legibilidad del minimapa a tinta y ritmo de la animación.

