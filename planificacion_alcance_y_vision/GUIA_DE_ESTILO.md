# Guía de estilo de Onegai

**Tarea AV15-06** · 14/10/2026 · Claude Sonnet · Borrador para Oriol · Solo documento: no se ha tocado código.

Sale de tres sitios: `HUD/Pergamino/Tema.h` y el código que lo usa (`Pincel`, `FuenteAtlas`, `Papiro`, `PaginaCuaderno`, `MesaCombateHud`), tus apuntes de la UOC (`Guies d’estil`, `Color`, `Tipografia per a interactius`, `Llegibilitat`, `Alineació`, `Reticles per a pantalla`, `Icones per a interactius`) y las capturas que ya existen (copiadas en `guia_de_estilo_capturas/`, lista en el Anexo B).

## 0. Cómo leerla

- **Para quién.** Para cualquiera (persona o agente) que añada una pantalla, un texto o un icono. Antes de integrar una pantalla nueva se pasa la lista del §8.
- **Quién manda.** `Tema.h` manda en colores y medidas; esta guía dice para qué sirve cada uno. Si discrepan, gana `Tema.h` y se corrige la guía. De `MODELO_VISUAL.md` vale el §13 (pergamino); el tema oscuro de sus §3–§5 ya no.
- **Marcas.**
  - **[hoy]**: existe en `Tema.h` o en el código y lo he comprobado.
  - **[propuesto]**: no existe todavía; lo propongo con piezas que ya hay.
  - **[medido]**: lo medí yo sobre una captura (contraste de los píxeles, aproximado).
  - **PAL-01, LET-02…**: número de regla. La tabla del §9 las cita.
  - **→ F3**: fila 3 de las tablas del §9.
- **Cada regla** dice dónde se cumple y dónde no, con la captura. Si no hay captura que lo enseñe, lo digo.

## 1. Principios

1. **Un solo estilo: el pergamino.** Cada cosa tiene su material: hoja (diálogos y avisos), papiro (menús de una página), cuaderno (menús con pestañas), retal (HUD de exploración) (MODELO_VISUAL §13.1). Nada de paneles oscuros sueltos.
2. **Un solo foco.** Lo que tiene el foco es lo que ejecuta ENTER, y se ve con la cinta carmín. Nunca dos cursores a la vez. El ratón señala y por tanto enfoca (`RatonMenus.h`).
3. **Nunca solo color.** Todo lo que el color distingue lleva además forma, texto o caja (apuntes `Color`: el 8 % de los hombres confunde rojo y verde).
4. **Reconocer, no recordar.** Un objeto se muestra con su icono y su nombre, no con un código.
5. **Nada se pisa y nada se corta sin avisar.** Cada pieza reserva su sitio; un texto que no cabe se parte o se recorta con «…» solo si la información completa está a un paso.
6. **El juego habla igual en todas partes** (§7).

## 2. Paleta

**PAL-01 · Tres colores de base.** Los apuntes dicen 2–3 colores de base. Los de Onegai son:
- **tinta** (`kTinta`, `kTinta2`, `kTinta3`: un color en tres escalones);
- **papel** (`kPapel`, `kPapelLuz`, `kPapelMancha`);
- **carmín** (`kCarmin`, `kCarmin2`).

El **cuero** (`kCuero`, `kClaroCuero`, `kClaroCuero2`) es material de las tapas, no un cuarto color. Los estados y los recursos son secundarios.
- Se cumple: `tienda_14-10.png` (tinta, papel y un solo carmín: la cinta y el botón COMPRAR).

**PAL-02 · El carmín es foco y acción principal, y nada más.** No decora. Los demás rojos tienen su sitio: `kError` (error, siempre con «!» y su caja), `kPv` (la vida), `kRombo` (la baldosa en el mundo).
- Se cumple: `tienda_14-10.png`; `has_caido_05-10.png`.

**PAL-03 · Los estados nunca van solos.** El texto del estado va con caja o con forma.
- Se cumple: `cuaderno_inventario_29-09.png` («!» + «Ahora no tendría efecto. No se gasta nada.»).
- Se cumple: `combate_turno_rival_14-10.png`: los tipos de carta llevan forma + color + rótulo. Está en `MesaCombateHud.cpp`:
  ```cpp
  case RPG::ActionType::FREE_ACTION: glifo = "▶"; color = Color::kExito;  rotulo = "ACCIÓN LIBRE"; break;
  case RPG::ActionType::REACTION:    glifo = "◆"; color = Color::kCarmin; rotulo = "REACCIÓN";     break;
  case RPG::ActionType::CHANNEL:     glifo = "●"; color = Color::kInfo;   rotulo = "CANALIZA";     break;
  case RPG::ActionType::MOVEMENT:    glifo = "▲"; color = Color::kAviso;  rotulo = "MOVIMIENTO";   break;
  ```

**PAL-04 · Qué color va con qué texto.**

| Constante (`Tema.h`) | Valor | Sí | No |
|---|---|---|---|
| `kTinta` | `#1C1209` | Texto principal, nombres, cifras | — |
| `kTinta2` | `#3B2A1B` | Texto secundario, descripciones, verbos de las ayudas, pestañas inactivas | — |
| `kTinta3` | `#4F3B28` | Etiquetas de campo, motivos, texto apagado | Sobre `kPapelMancha`; en texto largo; como único portador de información |
| `kAdorno` | `#6B5038` | Filetes, rombos, punteados (siempre decoración) | Cualquier texto o icono que lleve información |
| `kLinea` | `#4A3421` | Contorno de todo material (2 px) | Relleno |
| `kCarmin` / `kCarmin2` | `#9A2A1C` / `#6E1A10` | Cinta de foco, botón principal, contorno del foco; `kCarmin2` es su relieve | Decorar; texto largo |
| `kLavadoFoco` / `kSubrayadoFoco` | carmín al 10 % / al 35 % | Fondo y subrayado de la fila con foco | Otros fines |
| `kExito` / `kAviso` / `kError` / `kInfo` | `#2E6337` / `#7A4409` / `#8E1B12` / `#29507A` | Insignias y líneas de estado, siempre con texto y caja | Solos; texto largo |
| `kPv` | `#A83A2A` | Relleno de la barra de vida | Texto |
| `kPapel` / `kPapelLuz` / `kPapelMancha` | `#E2CD9C` / `#F2E4C1` / `#C6A56C` | Fondo de hoja / pistas de barra, teclas y botón secundario / relieve de tecla y botón | Texto sobre `kPapelMancha` en tinta-3 |
| `kCuero` + `kClaroCuero` / `kClaroCuero2` | `#4E321D` + `#F2E4C1` / `#D9C49A` | Tapas del cuaderno y su texto (9,3:1 y 6,8:1) | — |
| `kVelo` / `kSombra` / `kSombraCuero` | negro cálido al 62 % / 45 % / 50 % | Mundo detrás de un menú / sombra de hoja de 4 px (`kSombraY`) | Panel con texto translúcido |
| `kBlanco` | `#FFFFFF` | Tinte neutro de iconos y retratos (sin tinte) | Texto |
| `kRombo` / `kRomboContorno` | `#C0392B` / `#2A1A0E` | Rombo de la baldosa en el mundo (sobre el suelo el carmín se pierde) | UI de papel |

**PAL-05 · El contraste se mide sobre el papel que se ve.** Mínimos, de tus apuntes de color (WCAG): 4,5:1 para texto e iconos que informan, 3:1 para texto grande, marcos y barras; lo decorativo no tiene mínimo.

`Tema.h` mide contra `kPapel` (`#E2CD9C`). Pero el papel que se ve en las capturas es más oscuro. Medí la mediana de píxeles sin texto:

| Superficie | Dónde sale | Color medido |
|---|---|---|
| `kPapel` (referencia de `Tema.h`) | — | `#E2CD9C` |
| Papiro | título, pausa, «Has caído» (`titulo_papiro_05-10.png`, `has_caido_05-10.png`) | `#D5B98A` |
| Hoja | tienda y retales del combate (`tienda_14-10.png`, `combate_tu_turno_14-10.png`) | `#AD9D79` (las manchas, `#95865C`) |

Contraste de las tintas y los estados de `Tema.h` sobre cada una (calculado con la fórmula WCAG):

| Constante | Sobre `kPapel` (lo que dice `Tema.h`) | Sobre papiro | Sobre hoja |
|---|---|---|---|
| `kTinta` | 11,8 | 9,8 | 6,9 |
| `kTinta2` | 8,8 | 7,3 | **5,1** |
| `kTinta3` | 6,8 | 5,6 | **4,0** |
| `kCarmin` | 4,9 | 4,1 | **2,9** |
| `kExito` | 4,5 | 3,8 | **2,7** |
| `kAviso` | 5,1 | 4,2 | **3,0** |
| `kError` | 5,8 | 4,8 | **3,4** |
| `kInfo` | 5,3 | 4,4 | **3,1** |
| `kLinea` | 7,5 | 6,2 | 4,4 |
| `kAdorno` | 4,8 | 3,9 | 2,8 |

Con la hoja de las capturas, solo `kTinta` y `kTinta2` pasan 4,5:1.

**Regla provisional [propuesto]**, hasta que AV15-07 lo resuelva:
- sobre la hoja, el texto que informa va en `kTinta` o `kTinta2`;
- `kTinta3` solo en texto grande o en lo que ya diga otro texto;
- los estados llevan siempre su caja y su texto en tinta.

Ojo: es una medición sobre capturas del harness. Confirmar jugando en Release que la hoja real es esa y no un efecto de la captura. → F18

**PAL-06 · Opaco y con velo.** Un panel con texto no es translúcido; detrás de un menú el mundo va bajo `kVelo`.
- Se cumple: `tienda_14-10.png` (mundo oscurecido).
- No se cumple del todo: el retal del personaje asoma bajo la tienda → F7.

**PAL-07 · Ningún RGB suelto.** Es la regla que escribe el propio `Tema.h`: «ningún componente escribe un RGB suelto». Si falta un color, se añade a `Tema.h` con nombre y con su contraste medido. Para una variante con transparencia, el helper ya existe (`conAlfa(Color::kTinta3, 0.6f)` en `PaginaCuaderno.cpp`).
- Hoy hay 38 líneas con `rgb(` fuera de `Tema.h` (Anexo A.1). → F15

## 3. Letras

**LET-01 · Dos voces, no más** (apuntes: como mucho 2 fuentes).
- **Pixel 5×7** nombra y cuenta: títulos, pestañas, teclas, insignias, cifras.
- **Lora** se lee: nombres de contenido, descripciones, diálogos, motivos.

En el código son `Letra::Pixel` y `Letra::Lora…` (`FuenteAtlas.h`). Se cumple en todas las capturas.

**LET-02 · Pixel.** Siempre en MAYÚSCULAS (el atlas no tiene minúsculas; `coloca()` las convierte) y a escala entera. Nunca en párrafos ni en nombres largos.

| Escala | Para qué | Usos en el código |
|---|---|---|
| ×1 | flechas ▲▼ de desplazamiento | 3 |
| ×2 | pestañas, teclas, insignias, rótulos de bloque, botones, cifras | 79 |
| ×3 | título de ventana, cifras grandes | 6 |
| ×4 | logo del título | 3 |

- Se cumple: `tienda_14-10.png` (TIENDA ×3, pestañas ×2); `titulo_papiro_05-10.png` (logo ×4).

**LET-03 · Lora: tamaños y papeles** [hoy; recuento aproximado de usos en `src/HUD/Pergamino/`, sin ficheros de prueba].

| Letra | Papel | Dónde se ve | Usos |
|---|---|---|---|
| Lora 28 negrita | Nombre en un detalle | cuaderno, tienda (`d.titulo`) | 3 |
| Lora 24 / 24 negrita | Nombre de personaje, nombre de carta ampliada, tendero | `tienda_14-10.png` («Tendero del mercado»), cuaderno | 9 |
| Lora 22 / 22 negrita | Diálogo; entradas de papiro (la del foco, en negrita) | `titulo_ciudad_14-10.png` (NARRACIÓN), `has_caido_05-10.png` | 5 |
| Lora 20 / 20 negrita | Filas de lista, opciones de diálogo | `tienda_14-10.png` (Poción, Éter, Pan) | 12 |
| Lora 18 / 18 negrita / 18 cursiva | Texto de lectura de retales, verbos de las ayudas de teclas | `tienda_14-10.png` (pie) | 101 |
| Lora 16 / 16 cursiva | Etiquetas de campo, motivos, metadatos | `cuaderno_ficha_05-10.png` (Raza, Clase…), `titulo_papiro_05-10.png` (motivo) | 81 |
| Lora 13 | Solo los nombres de las cartas del combate (UI-BG3-01) | `combate_tu_turno_14-10.png` | 2 |

**Regla [propuesto]:** el mínimo es **16 px** para cualquier texto que informe (apuntes: cuerpo de 14–16 px). La Lora 13 es una excepción que hoy se rompe → F3. Lora 16 se queda para etiquetas y motivos, no para el dato que el jugador necesita (hoy los valores de la historia personal de la ficha van en Lora 16 → `cuaderno_ficha_05-10.png`).

**LET-04 · Interlineado.** Los apuntes dicen ≈150 %. Hoy en el código [hoy]:

| Letra | Interlineado |
|---|---|
| Lora 22 (diálogo, 34 px) | 155 % |
| Lora 16 (ficha, 23 px) | 144 % |
| Lora 18 (25 px) | 139 % |
| Lora 20 (26 px) | 130 % |

Regla [propuesto]: ≥ 140 % en texto de lectura. El diálogo ya cumple (`titulo_ciudad_14-10.png`, esquina inferior derecha).

**LET-05 · Longitud de línea de 45 a 75 caracteres** (apuntes). El diálogo mide 840 px de ancho de texto (`DialogoHoja::kAnchoTexto`); con Lora 22 son unos 75 caracteres por línea [estimado, no medido], el borde alto del rango. En las hojas de detalle caben menos y cumplen.

**LET-06 · Alineación.** El texto va a la izquierda. El centrado es solo para títulos de hoja y papiro, y rótulos cortos (apuntes `Alineació`).
- Se cumple: `has_caido_05-10.png`, `cuaderno_ficha_05-10.png`.

**LET-07 · Ortografía completa.** Tildes, ñ, ¿¡, «». Lora la soporta, y el pixel desde PERGAMINO-01 (la tilde sube al interlineado).
- Se cumple: `tienda_14-10.png` («Poción», «Éter»), `has_caido_05-10.png` («HAS CAÍDO»).
- No se cumple: «0 de dano» y «Escalofrio» en el combate; «Otono» en la ficha → F4, F11.

**LET-08 · Jerarquía de cuatro niveles como mucho por pantalla** (apuntes `Tipografia`: definir un sistema, diferencias evidentes):
1. título (pixel ×3);
2. nombre (Lora 22–28 negrita);
3. cuerpo (Lora 18–20);
4. etiqueta (Lora 16 o pixel ×2, en tinta-3).

La negrita es para el foco y los nombres, la cursiva para los motivos.

## 4. Retícula y márgenes

**RET-01 · Base de 8 px, con medio paso de 4.** Tus apuntes (`Reticles`): la distancia vertical entre elementos, siempre 8 px o múltiplos. En `Tema.h`:

```cpp
constexpr float kAreaSegura = 24.0f;   // margen de la pantalla (3 x 8)
constexpr float kSombraY = 4.0f;       // sombra de un material (medio paso)
constexpr float kFila = 44.0f;         // 40 + 4
constexpr float kTecla = 28.0f;        // 24 + 4
```

Excepciones: contornos y filetes de 2 px. Cualquier otra medida va a múltiplo de 4.
- Se cumple: `Tema.h` entero.
- No se cumple: 17 medidas del código (Anexo A.2). → F16

**RET-02 · Pantalla de referencia 1280×720**, con un **área segura de 24 px** (`Medida::kAreaSegura`). El texto va como mínimo 24 px dentro del borde de la hoja, para dejar fuera los cantos oscuros (MODELO_VISUAL §13.2).

**RET-03 · Tamaños de las piezas** [hoy].

| Pieza | Medida | Dónde está |
|---|---|---|
| Fila de lista | 44 | `Tema.h` (`kFila`) |
| Botón | 44 de alto; ancho = texto pixel ×2 + 40 | `PaginaCuaderno.cpp` (`pintaBoton`) |
| Tecla | 28 + contorno 2; ancho mínimo 28; +12 de aire | `Pincel::tecla` |
| Insignia | 26 de alto, borde 2, aire lateral 8 | `Pincel::insignia` |
| Carta del combate | 72×100, paso 80 | `MesaCombateHud.cpp` |
| Retal del personaje | 290×122 | `HudRetales.cpp` |
| Diálogo | 920×162, a 28 px del borde inferior | `DialogoHoja.h/.cpp` |

**RET-04 · Espacio entre piezas** [propuesto]: 8 px entre hermanas, 16 entre grupos, 24 entre bloques y al borde. Una caja de texto tiene aire a los cuatro lados.

**RET-05 · «Pixel perfect»** (apuntes `Alineació`): coordenadas redondeadas y escala entera. Hoy se cumple por código (`std::round` en `Pincel`); no hay captura que lo demuestre.

**RET-06 · Plantillas.**

| Plantilla | Uso | Captura |
|---|---|---|
| Papiro (1 columna, hasta ~520 px) | título, pausa, opciones, «Has caído» | `has_caido_05-10.png`, `opciones_29-09.png` |
| Cuaderno (lista a la izquierda, detalle a la derecha) | menú del jugador, tienda | `cuaderno_inventario_29-09.png`, `tienda_14-10.png` |
| Hoja inferior | diálogo | `titulo_ciudad_14-10.png` |
| Retales | HUD de exploración y combate | `combate_tu_turno_14-10.png` |

## 5. Botones y controles

**BOT-01 · Tipos.** Principal (lacre: carmín con texto claro), secundario (papel claro con contorno de tinta) y entrada de lista (fila con cinta). Como mucho **un principal por hoja**.
- Etiqueta en pixel ×2 y mayúsculas, con **verbo concreto** (§7).
- Se cumple: `tienda_14-10.png` («COMPRAR 1 · 30 ORO»).

**BOT-02 · Los cinco estados.** Los apuntes dicen: normal, encima, pulsado, visitado, desactivado. Onegai no tiene enlaces, así que «visitado» no aplica y se añade el **foco** (teclado).

| Estado | Cómo se ve | Señal que no es color | Hoy |
|---|---|---|---|
| **Normal** | Principal: relleno `kCarmin`, relieve de 4 px en `kCarmin2`, contorno `kLinea` de 2 px, texto `kPapelLuz`. Secundario: relleno `kPapelLuz`, relieve de 3 px en `kPapelMancha`, contorno `kLinea`, texto `kTinta` | Relieve y contorno | **[hoy]**: `tienda_14-10.png` (principal). Ver Anexo A.3: el principal tiene dos contornos distintos y el secundario no lleva el relieve → F17 |
| **Encima** (ratón) | Igual que el **foco**: en Onegai el ratón señala y por eso enfoca (un solo cursor) | La misma que el foco | **[hoy]** en los papiros: `RatonMenus.h`. **[propuesto]** para botones sueltos: sin captura |
| **Pulsado** | Baja 2 px y pierde el relieve inferior mientras se mantiene (mínimo ≈100 ms para que se vea) | Movimiento y relieve | **[propuesto]**. No existe y no hay captura |
| **Desactivado** | Sin relleno, contorno discontinuo de 2 px (4 de trazo y 4 de hueco) en `kTinta3`, texto `kTinta3`. **Siempre con su motivo al lado** (Lora 18 en `kTinta2`; «!» si es aviso) | El contorno discontinuo: forma, no color | **[hoy]** solo para el principal: `cuaderno_inventario_29-09.png` («USAR» + «! Ahora no tendría efecto. No se gasta nada.») |
| **Foco** | Botón: contorno `kCarmin` a 4 px del borde (propuesto). Fila: `kLavadoFoco` + `kSubrayadoFoco` + cinta de marcapáginas. Carta: contorno `kCarmin` y sube 6 px | Forma (cinta, contorno) | Fila y carta **[hoy]**: `tienda_14-10.png` (fila Poción), `has_caido_05-10.png`. Botón suelto **[propuesto]**: sin contorno de foco en el código |

Si el foco está en la lista, el botón principal no lleva contorno (MODELO_VISUAL §5).

**BOT-03 · Otros controles interactivos.**

| Control | Estados que tiene | Se ve en |
|---|---|---|
| Fila de lista | normal (tinta) · foco (lavado + subrayado + cinta) · elegida (✓) · desactivada (tinta-3 + motivo en Lora 16 cursiva) | `tienda_14-10.png`, `titulo_papiro_05-10.png` («Continuar» apagado con motivo) |
| Pestaña | activa (papel claro, unida al cuerpo) · inactiva (texto `kTinta2` sobre papel o cuero) | `tienda_14-10.png`, `cuaderno_inventario_29-09.png` |
| Tecla | caja de 28 px, `kPapelLuz`, contorno 2, relieve 3 | todas |
| Insignia | caja de 26 px con borde del color del estado | `cuaderno_inventario_29-09.png` («CONSUMIBLE») |
| Carta | normal · foco · apagada (gastada) · preparada | `combate_turno_rival_14-10.png` (cartas apagadas en el turno del rival) |
| Deslizador | pista con 11 marcas, mando; relleno carmín solo en la fila con foco | `opciones_29-09.png` |

**BOT-04 · Ratón y teclado, equivalentes** (Oriol, 05/10). Pasar el ratón señala, un clic acepta. Las ayudas de teclas salen del mapa de acciones real.
- Se cumple: `titulo_papiro_05-10.png` («o haz clic con el ratón»).

**BOT-05 · Zona de clic de 44 px de alto como mínimo** en filas y botones (apuntes `Icones`: área de activación de 45–50 px). Filas (`kFila` = 44) y botones del cuaderno (44) cumplen; el botón del combate mide 46.

**BOT-06 · Lo caro o irreversible pide resumen y confirmación** (comprar una casa, salir sin guardar). **[propuesto]**, es AV15-05; hoy no hay pantalla pergamino que lo enseñe.

## 6. Iconos

El atlas es `assets/textures/iconos.png` (copia: `guia_de_estilo_capturas/iconos_atlas.png`). Tiene 56 iconos de 16×16 px, en rejilla de 8×7, con contorno oscuro de 1 px y relleno plano.

**ICO-01 · Cada icono significa una cosa, y cada cosa lleva un solo icono** (apuntes: «evitar iconos similares para funciones diferentes, o diferentes iconos para una misma función»).

Hoy se rompe en seis sitios → F1, F2, F5, F14:

| Icono (frame) | Problema |
|---|---|
| `car` (4) y `dote` (37) | La misma estrella dorada para el atributo Carisma y para «dote» |
| `dote` | Es el icono por defecto de todas las cartas de la mano: siete cartas iguales (Anexo A.4) |
| `cerrar` (56) | Es la X en caja gris, y se usa para **Huir** |
| `consumible` (51) | El mismo frasco rojo para Poción, Éter y Pan |
| `comprar` (54) y `vender` (55) | Dos bolsas casi idénticas; solo cambia un trazo del símbolo |
| `objetivo` (42) | Un cuadrado vacío: abstracto |

**ICO-02 · Regla de los 5 segundos** (apuntes). Si un jugador nuevo tarda más de 5 s en saber qué es, no sirve. Prueba [propuesto]: enseñar el icono solo a 3–5 personas. No se ha hecho ninguna prueba.

**ICO-03 · Objetos reconocibles, nada abstracto.** El icono dibuja la cosa (espada para atacar, frasco para poción, casco para la cabeza, corazón para la vida). Lo abstracto lleva un texto al lado (apuntes).
- Se cumple: `combate_tu_turno_14-10.png` (la espada de «Atacar»); `cuaderno_ficha_05-10.png` (iconos de atributos con su nombre).

**ICO-04 · Es un sistema.** Todos con el mismo contorno de 1 px, relleno plano con una luz, sin degradados y con la misma paleta saturada. Como mucho 3 colores de relleno por icono (apuntes) **[por verificar en los 56]**.
- Se dibujan a escala **×1** (pestañas, insignias, cifras) o **×2** (listas y detalle), nunca a otra.

**ICO-05 · Un icono no decora.** Si no comunica, se quita (apuntes).

**ICO-06 · Forma + color + texto en los tipos de acción** (PAL-03): ▶ acción libre, ■ acción principal, ▲ movimiento, ● canaliza, ◆ reacción.
- Se cumple en las cartas: `combate_turno_rival_14-10.png`. No en la leyenda de la barra, donde ACCIÓN y CANALIZA llevan el mismo ● y las cartas llevan ■ (EH-15 de `EVALUACION_HEURISTICA.md`, captura `combate_rival_juega_14-10.png`).

**ICO-07 · Los iconos que faltan** [propuesto]:
- uno por objeto de la tienda (pan, éter…);
- «Huir»;
- uno por familia de carta;
- los emblemas de las tiendas (AV15-02).

## 7. La voz

**VOZ-01 · Castellano, siempre.** El catalán y el inglés solo salen por el selector de idioma (glosario en MODELO_VISUAL §9.2). Nada de «Game over»: el juego dice «Has caído».

**VOZ-02 · Tuteo.** Se habla al jugador de «tú»: «Te quedarán 41 de oro», «Tus PV ya están al máximo». Nunca «usted», nunca primera persona del juego.

**VOZ-03 · Sobrio, concreto y sin chistes modernos.** Frases cortas, sin guiños de hoy (memes, anglicismos de internet, saltarse la cuarta pared). Los nombres tienen el sabor del canon. (Es la consigna de tu encargo AV15-06 y del de los nombres, AV15-02a.)

**VOZ-04 · Verbo concreto en botones y ayudas.** USAR, COMPRAR 1 · 30 ORO, ELEGIR, CANCELAR, GUARDAR, Cargar último guardado. Nunca «OK» ni «Confirmar». «Aceptar» solo cuando la entrada ya lleva su propio verbo (Nueva partida).

**VOZ-05 · Qué pasó + qué puedes hacer, sin culpar.**
- Lo apagado dice por qué: «No hay ninguna partida guardada.», «Ahora no tendría efecto. No se gasta nada.».
- Un error dice el remedio: «No se ha podido guardar · Pulsa ENTER para reintentar».
- Se cumple: `titulo_papiro_05-10.png`, `cuaderno_inventario_29-09.png`, `has_caido_05-10.png` («La aventura se detiene aquí.»).

**VOZ-06 · Quién habla, siempre.** El hablante va en el rótulo de cuero; «NARRACIÓN» solo cuando habla el narrador.
- Se cumple: `titulo_ciudad_14-10.png`, esquina inferior derecha («Oscuridad. Caes desde el vacío hacia el vacío.»).

**VOZ-07 · Sin jerga interna.** Nunca ids, claves ni códigos de tarea al jugador (`PERSONALITY_…`, «(G-10)»).

**VOZ-08 · Ortografía completa** (LET-07).

## 8. Revisar una pantalla nueva

Antes de integrarla:

1. ¿Todos los colores salen de `Tema.h`, sin un `rgb(` suelto? (PAL-07)
2. ¿Hay tres colores de base como mucho, el carmín solo en foco y acción principal? (PAL-01, PAL-02)
3. ¿Cada texto que informa pasa 4,5:1 **sobre el papel que se ve**? (PAL-05)
4. ¿Algo se distingue solo por color? (PAL-03)
5. ¿Solo Pixel + Lora, tamaños de la tabla, mínimo 16 px, mayúsculas solo en pixel? (LET-01 a LET-03)
6. ¿Tildes y ñ completas? (LET-07)
7. ¿Las medidas son múltiplos de 4 y el texto está 24 px dentro del borde? (RET-01, RET-02)
8. ¿Un solo foco visible? ¿Los cinco estados del botón, con motivo en el desactivado? (BOT-02)
9. ¿Cada icono significa una cosa, se reconoce en 5 s y no se repite en otra función? (ICO-01, ICO-02)
10. ¿Tuteo, verbo concreto, sin chistes modernos, sin jerga interna? (VOZ-01 a VOZ-07)
11. ¿Se puede jugar con ratón y con teclado, y los atajos salen en el botón? (BOT-04)
12. ¿Se ve bien con el texto más largo posible (nombres de cartas, de objetos)? (principio 5)

## 9. Pantallas que no cumplen

No pongo gravedad: esa escala es de AV15-01 (evaluación heurística). Las propuestas no tocan código aquí.

### 9.1 Por pantalla

| Fila | Pantalla | Regla | Qué se ve | Captura (dónde mirar) | Propuesta |
|---|---|---|---|---|---|
| **F1** | Combate | ICO-01 | Las siete cartas de «TU MANO» llevan la misma estrella | `combate_tu_turno_14-10.png` (barra inferior) | Icono por familia de carta; hasta entonces, el de su condición |
| **F2** | Combate | ICO-01, ICO-02 | «Huir» lleva `cerrar` (X en caja gris): se lee «cerrar», no «huir» | `combate_tu_turno_14-10.png` (tercera carta de «BÁSICAS») | Icono de huida propio |
| **F3** | Combate | LET-03, principio 5 | Los nombres de carta van en Lora 13 (<16) y dos se cortan con «…» («Escalofrío del espíritu de la ven…») | `combate_turno_rival_14-10.png` (cartas 6 y 7) | Nombre en 2 líneas a 16 px o carta más ancha. El nombre entero ya sale en el panel de la izquierda al marcar la carta |
| **F4** | Combate | LET-07, VOZ-08 | El registro escribe «0 de dano» sin ñ; la carta 6 dice «Escalofrio» sin tilde, mientras «Péndulo» y «espíritu» sí la llevan | `combate_turno_rival_14-10.png` (panel REGISTRO y carta 6) | Revisar los textos del código del combate. Origen sin comprobar: AGENTS.md pide ASCII en los JSON de aventura y puede haberse extendido |
| **F5** | Tienda | ICO-01 | Poción, Éter y Pan llevan el mismo frasco rojo | `tienda_14-10.png` (lista COMPRAR) | Un icono por objeto |
| **F6** | Tienda | PAL-05 | Las etiquetas «Precio», «Tienes», «Te quedarán», la cabecera OBJETO y «Tu oro» (tinta-3) miden 3,9–4,6:1 [medido] sobre la hoja; la pestaña inactiva VENDER, 4,0:1 | `tienda_14-10.png` | Regla provisional de PAL-05 |
| **F7** | Tienda | PAL-06, principio 5 | El retal del personaje («Sulivan T1») asoma por detrás de la hoja de la tienda | `tienda_14-10.png` (esquina superior izquierda) | Ocultar el HUD de exploración mientras hay una hoja modal |
| **F8** | Título | LET-01, jerarquía | El logo crema se dibuja sobre la ciudad, que mete ruido: «DE BOUNDINGTON» cae sobre un tejado verde claro [no medido]. Sobre fondo plano (05/10) se lee bien | `titulo_ciudad_14-10.png` (los tres cuadros del título) frente a `titulo_papiro_05-10.png` | Velo o banda de pergamino detrás del logo |
| **F9** | Título | RET-01, principio 5 | Las ayudas del papiro se parten en tres líneas: «W S Elegir · ENTER Aceptar», «ESC Salir» sola y «o haz clic con el ratón» | `titulo_papiro_05-10.png` (pie del papiro) | Que las ayudas quepan en una fila o se agrupen: dos líneas como mucho |
| **F10** | Cuaderno (ficha) | principio 5, RET-01 | «0 + 1 raza + 1 equipo» (Lora 18, tinta-3) desborda la columna ATRIBUTOS y pisa HISTORIA PERSONAL. El código sigue sin ancho máximo en ese texto (Anexo A.5) | `cuaderno_ficha_05-10.png` (filas INT y CON) | Recortar o partir el detalle al ancho de la columna |
| **F11** | Cuaderno (ficha) | principio 5, VOZ | El rasgo se corta a media palabra: «Otono: tienes ventaj…» (y «Otono» sin ñ) | `cuaderno_ficha_05-10.png` (RASGO) | Que el rasgo entero quepa, o «Ver más» |
| **F12** | Cuaderno | PAL-05, BOT-03 | Las pestañas inactivas (INVENTARIO…CÓDICE) miden 3,6:1 [medido, tintas de 05/10]. En la tienda de 14/10, VENDER mide 4,0:1 | `cuaderno_ficha_05-10.png`, `tienda_14-10.png` | Regla provisional de PAL-05 o pestaña más clara |
| **F13** | Negocio (comprar casa) | principio 1, principio 5, LET-01 | Estilo oscuro anterior al pergamino; «COMPRAR» y «SALIR» pisan «TIENES 150 ORO»; pixel apelmazado y sin tildes. **Captura del 28/09: AV15-05 dice que sigue así; confirmar con una captura nueva** | `negocio_28-09_estilo_viejo.png` | Rehacerla en pergamino (AV15-05) |

### 9.2 Reglas que fallan en todo el juego

| Fila | Regla | Qué pasa | Evidencia |
|---|---|---|---|
| **F14** | ICO-01 | `car` y `dote` son la misma estrella; `comprar` y `vender` casi iguales; `objetivo` es un cuadrado vacío | `iconos_atlas.png` (frames 4 y 37; 54 y 55; 42) |
| **F15** | PAL-07 | 38 líneas con `rgb(` suelto fuera de `Tema.h` | Anexo A.1 |
| **F16** | RET-01 | 17 medidas fuera de la retícula de 4 px | Anexo A.2 |
| **F17** | BOT-02 | El botón principal tiene dos contornos distintos y el secundario no lleva relieve; no hay aspecto de «pulsado» ni de foco en botones sueltos | Anexo A.3 |
| **F18** | PAL-05 | Los contrastes de `Tema.h` están medidos sobre `kPapel`, y la hoja que se ve es más oscura | §2 (PAL-05) |

## 10. Lo que no he podido comprobar

- **No hay captura del cuaderno posterior a TINTAS-01** (08/10, tintas más oscuras). La ficha y el inventario de las capturas son anteriores: aprovechan para la estructura, no para los contrastes. Pedir una captura nueva de INVENTARIO, EQUIPO y FICHA.
- **Botones en «encima», «pulsado» y «foco»**: no existen en el código ni en capturas. El §5 los propone con piezas de `Tema.h`.
- **La hoja de tienda y combate (`#AD9D79`)**: falta confirmar jugando en Release que es el papel real y no un efecto del harness de capturas.
- **Termas y pausa/opciones actuales**: solo hay capturas del 29/09 (pausa y opciones) y ninguna de las termas.
- **Prueba de los 5 segundos** con personas: no hecha.
- **Que cada icono tenga ≤3 colores**: no verificado en los 56.

---

## Anexo A · Evidencias en el código

### A.1 `rgb(` sueltos fuera de `Tema.h` (38 líneas)

Cuenta por fichero de `src/HUD/Pergamino/`:

| Fichero | Líneas |
|---|---|
| `PaginasCreacion.cpp` | 10 |
| `MesaCombateHud.cpp` | 8 |
| `DadosPergamino.cpp` | 6 |
| `MesaCombateRetales.cpp` | 4 |
| `Cuaderno.cpp` | 3 |
| `PaginasCreacionHistoria.cpp` | 2 |
| `PaginasCreacionHojas.cpp` | 2 |
| `HudRetales.cpp`, `InterfazPergamino.cpp`, `TiendaHoja.cpp` | 1 cada uno |

Ejemplo, `PaginasCreacion.cpp` (líneas 197–198). Son `kPapelLuz` y `kAdorno` con alfa, escritos a mano:
```cpp
k.l.color(r, activo ? Color::kTinta2 : rgb(242, 228, 193, 0.6f));
k.p.contorno(k.l, r, 1.0f, activo ? Color::kTinta : rgb(107, 80, 56, 0.6f));
```
Con el helper que ya existe sería `conAlfa(Color::kPapelLuz, 0.6f)` y `conAlfa(Color::kAdorno, 0.6f)`.

### A.2 Medidas fuera de la retícula de 4 px

| Medida | Valor | Dónde |
|---|---|---|
| `kHueco` | 3 | `DadosPergamino.cpp:88` |
| `kArriba` | 26 | `PaginaCuaderno.cpp:20` |
| `kAbajo` | 30 | `PaginaCuaderno.cpp:21` |
| `kInicioRejilla` | 54 | `PaginaCuaderno.cpp:22` |
| `kHuecoFila` | 2 | `PaginaCuaderno.cpp:24` |
| `kHueco` | 2 | `Papiro.cpp:20` |
| `kCabecera` | 103 | `Papiro.cpp:22` |
| `kEstado` | 131 | `Papiro.cpp:25` |
| `kMotivo` | 46 | `Papiro.cpp:30` |
| `kNota` | 26 | `Papiro.cpp:32` |
| `kFilaPie` | 46 | `Papiro.cpp:52` |
| `kCabecera` | 22 | `TiendaHoja.cpp:27` |
| `kPie` | 50 | `TiendaHoja.cpp:33` |
| `kAnchoRetal` / `kAltoPersonaje` | 290 / 122 | `HudRetales.cpp:21-22` |
| `kBasicasX` / `kManoX` / `kCartaY` | 166 / 426 / 586 | `MesaCombateHud.cpp:98` |
| `kAlto` (diálogo) | 162 | `DialogoHoja.h:39` |
| alto del botón del combate | 46 | `MesaCombateRetales.cpp:345` |

### A.3 El botón principal, en dos versiones

En el cuaderno (`PaginaCuaderno.cpp`, `pintaBoton`) el contorno es `kLinea`:
```cpp
l.color(Rect{x - 2.0f, y - 2.0f, ancho + 4.0f, 48.0f}, Color::kLinea);
l.color(Rect{x, y, ancho, 44.0f}, Color::kCarmin);
```
En la creación de personaje (`PaginasCreacion.cpp`) y en el combate (`MesaCombateRetales.cpp:346`) es `kCarmin2`:
```cpp
k.l.color(Rect{r.x - 2.0f, r.y - 2.0f, r.w + 4.0f, r.h + 4.0f}, Color::kCarmin2);
k.l.color(r, Color::kCarmin);
```
El CSS del tema (`tema_pergamino.css`) pide `0 0 0 2px var(--linea)` en el principal y `inset 0 -3px 0 var(--papel-mancha)` (relieve) en el secundario. El secundario de `PaginasCreacion.cpp` no lleva ese relieve.

### A.4 Icono por defecto de las cartas

`MesaCombateHud.cpp`:
```cpp
if (c.id == "#huir") return "cerrar";
...
default: return "dote";
```
Las cartas de la mano sin condición propia caen en `dote`; por eso las siete llevan la misma estrella.

### A.5 El detalle de los atributos de la ficha

`PaginaCuaderno.cpp`, `pintaFicha`:
```cpp
p.texto(l, Letra::Lora18, s.detalle, x2 + 228.0f, c2 + 5.0f, 26.0f, Color::kTinta3);
```
No pasa por `recorta()` ni por `envuelve()`: el texto sale tan largo como sea y la columna siguiente empieza a `x2 + 384`, solo 156 px más allá.

## Anexo B · Capturas usadas

Todas están copiadas en `guia_de_estilo_capturas/` con estos nombres. «Antes de TINTAS» = anterior a las tintas más oscuras del 08/10 (`kTinta` 11,8:1 en vez de 10,5:1).

| Fichero | Origen | Fecha | Nota |
|---|---|---|---|
| `titulo_papiro_05-10.png` | `Claude outputs/ui_bg3_2026-10-05/capturas/01_titulo.png` | 05/10 | Antes de TINTAS. Estructura y papiro |
| `titulo_ciudad_14-10.png` | `MotorGraphico/out/claude_dialogos03/capturas/titulo.png` | 14/10 00:07 | Hoja de contacto 2×2: tres títulos y un diálogo con NARRACIÓN |
| `combate_tu_turno_14-10.png` | `MotorGraphico/out/claude_dialogos04/combate1.png` | 14/10 00:58 | Tintas actuales |
| `combate_turno_rival_14-10.png` | `MotorGraphico/out/claude_dialogos04/carta1.png` | 14/10 00:59 | Tintas actuales |
| `tienda_14-10.png` | `MotorGraphico/out/claude_dialogos04/tienda1.png` | 14/10 01:14 | Tintas actuales |
| `cuaderno_inventario_29-09.png` | `Claude outputs/pergamino_2026-09-29/img/11_inventario_eter.png` | 29/09 | Antes de TINTAS; textos sin tildes, ya corregidos |
| `cuaderno_ficha_05-10.png` | `Claude outputs/kit_tiradas_grupos_2026-10-05/capturas/03_ficha_con_el_kit.png` | 05/10 | Antes de TINTAS |
| `has_caido_05-10.png` | `Claude outputs/kit_tiradas_grupos_2026-10-05/capturas/11_has_caido.png` | 05/10 | Antes de TINTAS |
| `opciones_29-09.png` | `Claude outputs/pergamino_2026-09-29/img/30_opciones.png` | 29/09 | Antes de TINTAS |
| `negocio_28-09_estilo_viejo.png` | `Claude outputs/capturas_menus_2026-09-28/07_juego_negocio/07-02_negocio_en_venta.png` | 28/09 | Estilo oscuro, antes del pergamino |
| `iconos_atlas.png` | `MotorGraphico/assets/textures/iconos.png` | — | Atlas de 56 iconos de 16×16 |
