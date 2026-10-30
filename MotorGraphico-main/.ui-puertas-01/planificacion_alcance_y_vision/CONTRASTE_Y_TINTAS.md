# Contraste de cada texto sobre el pergamino, y las tintas que propongo

**Tarea AV15-07 (la parte de contraste)** · 14/10/2026 · Claude Sonnet · Borrador para Oriol · Solo documento: no se ha tocado código ni `Tema.h`.

Mide el contraste de cada texto del tema pergamino sobre el papel **que se ve** (la textura real, no el `kPapel` que supone `Tema.h`) y propone las tintas que hacen falta para llegar a 4,5:1. Las líneas de 45–75 caracteres, la trama del suelo, el tamaño del texto en Opciones y el temblor son otras partes de AV15-07 y no están aquí.

## 1. Resumen

- **La hoja de las capturas es el papel real, no un efecto del harness.** El papel de las hojas sale de `assets/textures/pergamino/hoja_a.png`, y su mediana (`#AE9F7D`) es la que medí sobre las capturas (`#AD9D79`). Es mucho más oscuro que el `kPapel` (`#E2CD9C`) con el que `Tema.h` calcula sus contrastes. Eso cierra la duda de la guía (F18).
- **Sobre la hoja pasan `kTinta` y `kTinta2`, y nada más.** `kTinta3` da 4,05:1 (3,62:1 en la fila con foco), y los cuatro colores de estado y el carmín dan entre 2,7 y 3,5:1.
- **Sobre el papiro** (título, pausa, opciones, «Has caído») pasan las tres tintas; fallan el carmín (4,09:1) y tres de los estados.
- **Las pestañas inactivas fallan** (3,14:1): su piel es más oscura que la hoja y ninguna tinta oscura llega a 4,5:1 sobre ella.
- **Pasan todos los textos claros sobre cuero, las teclas, los botones y los resultados de los dados** (cada uno sobre su chip claro).
- **Propuesta** (§5): una `kTinta3` más oscura (`#3C2D1E`), el color de estado solo como texto sobre un chip claro (o, si se quiere coloreado sobre la hoja, cinco tintas de texto nuevas), el contorno del foco en `kCarmin2` y una pestaña inactiva un 10 % más clara. Con eso todos los textos pasan 4,5:1 sobre la mediana de la hoja.
- **El precio:** `kTinta2` y `kTinta3` quedan casi iguales (ver §5). La jerarquía tendrá que salir del tamaño y de la cursiva, no del color.

## 2. Los papeles, medidos

Medí los píxeles de la textura que se estira bajo el texto (el cuerpo de la piel, sin los bordes de 9 cortes y con 4 px de margen). El texto va como mínimo a 24 px del borde (RET-02), así que cae sobre ese cuerpo.

| Papel | Dónde | Textura | Mediana | Luminancia: mediana · 5 % más oscuro |
|---|---|---|---|---|
| **Hoja** | diálogo, tienda, cuaderno, retales del HUD y del combate | `hoja_a.png` | `#AE9F7D` | 0,353 · 0,230 |
| **Hoja con foco** | fila con foco: `kLavadoFoco` (carmín al 10 %) sobre la hoja | `hoja_a.png` | `#AC9373` | 0,310 · 0,199 |
| **Papiro** | título, pausa, opciones, «Has caído» | `papiro.png` | `#D5B98A` | 0,507 · 0,352 |
| **Pestaña inactiva** | cuaderno y tienda | `pestana.png` | `#907546` | 0,191 · 0,098 |
| **Cuero** | tapas del cuaderno, barra de cartas | `cuero.png` | `#462D19` | 0,032 · 0,012 (el 5 % más claro: 0,054) |
| **Cuero liso** | rótulo del hablante | `cuero_liso.png` | `#5B2917` | 0,038 · 0,006 (el 5 % más claro: 0,180, el brillo) |
| Papel claro (plano) | teclas, botón secundario, cartas | `kPapelLuz` | `#F2E4C1` | 0,782 |
| Relieve de tecla y botón | | `kPapelMancha` | `#C6A56C` | 0,400 (sin texto encima) |

Tres notas del código (`BancoMateriales.cpp`), leídas pero no ejecutadas:
- **Todas las hojas usan la misma piel.** `hoja_b.png` está en `materiales.json`, pero la elección de piel solo cambia por parámetros y las dos hojas no tienen ninguno, así que gana la primera (`hoja_a`). `hoja_b` (`#A98D6A`, más oscura aún) no se usa.
- **El papel de las páginas del cuaderno es la misma hoja** (`papelHoja()` en `Cuaderno.cpp`). Solo la hoja de «dorso» se oscurece, y no lleva texto.
- **La textura tiene un entramado de píxeles claros y oscuros:** el contraste cambia de un píxel a otro. Por eso miro dos números: la **mediana** (lo típico) y el **5 % más oscuro** (lo peor).

## 3. Cada tinta sobre cada papel

Contraste WCAG (luminancia relativa). Solo cuentan como texto las que se usan como texto; `kAdorno` y `kLinea` van aparte porque son decoración y contorno.

| Tinta | Valor | Hoja (mediana) | Hoja con foco | Hoja (5 % peor) | Papiro | Pestaña inactiva |
|---|---|---|---|---|---|---|
| `kTinta` | `#1C1209` | **7,07** | **6,31** | **4,91** | **9,77** | 4,23 |
| `kTinta2` | `#3B2A1B` | **5,25** | **4,69** | 3,65 | **7,26** | 3,14 |
| `kTinta3` | `#4F3B28` | 4,05 | 3,62 | 2,81 | **5,60** | 2,42 |
| `kCarmin` | `#9A2A1C` | 2,96 | 2,64 | 2,06 | 4,09 | 1,77 |
| `kExito` | `#2E6337` | 2,72 | 2,43 | 1,89 | 3,77 | 1,63 |
| `kAviso` | `#7A4409` | 3,03 | 2,71 | 2,11 | 4,19 | 1,81 |
| `kError` | `#8E1B12` | 3,48 | 3,11 | 2,42 | **4,81** | 2,08 |
| `kInfo` | `#29507A` | 3,19 | 2,85 | 2,22 | 4,41 | 1,91 |
| `kLinea` (contorno) | `#4A3421` | 4,46 | 3,99 | 3,10 | 6,17 | 2,67 |
| `kAdorno` (decoración) | `#6B5038` | 2,85 | 2,54 | 1,98 | 3,94 | 1,70 |

En negrita, lo que pasa 4,5:1 sobre la mediana. Los contrastes que `Tema.h` apunta en sus comentarios (`11,8:1` para `kTinta`, `6,8:1` para `kTinta3`) son sobre `kPapel`, que no es el papel que se ve.

## 4. Cada texto

**Cómo se juzga.** ✓ = 4,5:1 o más sobre la mediana **y** 3:1 o más sobre el 5 % más oscuro. ~ = pasa la mediana pero no el peor 5 %. ✗ = no pasa la mediana. El 3:1 de los textos grandes de WCAG (Lora 24 y 28, pixel ×3 y ×4) no lo he necesitado: esos textos van en `kTinta`.

| Id | Texto | Tinta | Sobre | Mediana | Peor 5 % | |
|---|---|---|---|---|---|---|
| H1 | Nombres, cifras y filas principales (Lora 18–28, pixel) | `kTinta` | hoja | 7,07 | 4,91 | ✓ |
| H2 | Lo mismo, en la fila con foco | `kTinta` | hoja con foco | 6,31 | 4,37 | ✓ |
| H3 | Descripciones y verbos de las ayudas de teclas (Lora 18) | `kTinta2` | hoja | 5,25 | 3,65 | ✓ |
| H4 | Lo mismo, en la fila con foco | `kTinta2` | hoja con foco | 4,69 | 3,25 | ✓ |
| H5 | Etiquetas de campo, motivos y cabeceras: «Precio», «Tienes», «Tu oro», «Raza», «0 + 1 raza» (Lora 16–18, pixel ×2) | `kTinta3` | hoja | 4,05 | 2,81 | ✗ |
| H6 | Lo mismo, en la fila con foco | `kTinta3` | hoja con foco | 3,62 | 2,51 | ✗ |
| H7 | Éxito: «Te quedarán 41 de oro», insignias | `kExito` | hoja | 2,72 | 1,89 | ✗ |
| H8 | Error: «Te faltan N de oro», insignias | `kError` | hoja | 3,48 | 2,42 | ✗ |
| H9 | Aviso: insignias de estado | `kAviso` | hoja | 3,03 | 2,11 | ✗ |
| H10 | Info: insignias | `kInfo` | hoja | 3,19 | 2,22 | ✗ |
| H11 | Carmín como texto (Lora 16: combate, historia) | `kCarmin` | hoja | 2,96 | 2,06 | ✗ |
| P1 | Papiro: título, entradas | `kTinta` | papiro | 9,77 | 7,05 | ✓ |
| P2 | Papiro: verbos de las ayudas | `kTinta2` | papiro | 7,26 | 5,24 | ✓ |
| P3 | Papiro: motivos («No hay ninguna partida guardada.») | `kTinta3` | papiro | 5,60 | 4,04 | ✓ |
| P4 | Papiro: carmín en pixel | `kCarmin` | papiro | 4,09 | 2,95 | ✗ |
| P5 | Papiro: error en pixel | `kError` | papiro | 4,81 | 3,47 | ✓ |
| T1 | Pestañas inactivas (pixel ×2) | `kTinta2` | pestaña inactiva | 3,14 | 1,94 | ✗ |
| K1 | Teclas (pixel ×2) y botón secundario | `kTinta` | `kPapelLuz` | 14,60 | 14,60 | ✓ |
| K2 | Botón desactivado (texto) | `kTinta3` | `kPapelLuz` | 8,37 | 8,37 | ✓ |
| K3 | Botón principal (pixel ×2) | `kPapelLuz` | `kCarmin` | 6,11 | 6,11 | ✓ |
| C1 | Tapas del cuaderno, rótulo del hablante, barra de cartas | `kClaroCuero` | cuero | 10,09 | 7,99 | ✓ |
| C2 | Ayudas sobre cuero | `kClaroCuero2` | cuero | 7,46 | 5,91 | ✓ |
| C3 | Rótulo del hablante, sobre el cuero liso | `kClaroCuero` | cuero liso | 9,50 | 3,62 | ✓ |
| D1 | Dados: «Éxito» | `kExito` | su chip `#D5E8C8` | 5,48 | 5,48 | ✓ |
| D2 | Dados: «Medio» | `kAviso` | su chip `#F3DCA4` | 5,86 | 5,86 | ✓ |
| D3 | Dados: «Pifia» | `kError` | su chip `#F4CCC2` | 6,16 | 6,16 | ✓ |

**Contornos y marcos** (WCAG pide 3:1 para lo que no es texto):
- `kLinea` sobre la hoja: 4,46:1 (peor 5 %: 3,10) ✓.
- `kCarmin` (la cinta, el contorno del foco) sobre la hoja: **2,96:1 (peor 5 %: 2,06)** ✗. Es el único marco que no llega, aunque el foco lleva además lavado y subrayado.
- La barra de vida (`kPv`) sobre su pista `kPapelLuz`: 5,04:1 ✓.

**Dónde se usan las tintas que fallan** (llamadas a texto con el color escrito en la misma línea, en `src/HUD/Pergamino/`; los colores que llegan por variable no entran, así que son un mínimo): `kTinta3`, 40 llamadas en 11 ficheros; `kError`, 10; `kExito`, 7; `kCarmin`, 4; `kAviso`, 3; `kInfo`, 1.

**Comprobación con una captura.** `tienda_14-10.png` (tintas actuales): la mediana del papel sin texto es `#AD9D79`, la misma de la piel (`#AE9F7D` con el margen de 4 px). Las etiquetas «Precio» y «Tienes» (`kTinta3`) se midieron de 3,9 a 4,6:1 sobre el papel que tienen alrededor, consistente con el 4,05:1 de la mediana y el 3,4:1 del cuarto más oscuro.

## 5. Las tintas que propongo

Parto de un criterio: **un texto que informa pasa 4,5:1 sobre la mediana de la hoja, también en la fila con foco**. La fila con foco (`kLavadoFoco`) oscurece el papel hasta una luminancia de 0,310, así que el texto necesita una luminancia de **0,030 o menos**. Con 0,029, el contraste es 5,10:1 sobre la hoja, 4,55:1 con foco y 3,54:1 sobre el 5 % más oscuro (y 7,05:1 sobre el papiro).

### 5.1 Tintas (Camino A: no se toca el papel)

| Constante | Hoy | Propuesta | Hoja | Hoja con foco | Peor 5 % | Papiro |
|---|---|---|---|---|---|---|
| `kTinta` | `#1C1209` | sin cambio | 7,07 | 6,31 | 4,91 | 9,77 |
| `kTinta2` | `#3B2A1B` | sin cambio | 5,25 | 4,69 | 3,65 | 7,26 |
| `kTinta3` | `#4F3B28` | **`#3C2D1E`** | 5,10 | 4,55 | 3,54 | 7,05 |
| `kLinea` | `#4A3421` | sin cambio (contorno) | 4,46 | 3,99 | 3,10 | 6,17 |
| `kAdorno` | `#6B5038` | sin cambio (solo decoración) | — | — | — | — |
| Contorno del foco sobre la hoja | `kCarmin` `#9A2A1C` | **`kCarmin2` `#6E1A10`** | 4,43 | 3,96 | 3,08 | — |

**El precio de `kTinta3`.** `#3C2D1E` es casi `kTinta2` (`#3B2A1B`): la diferencia de luminancia es de 0,0024. Las etiquetas de campo y las descripciones quedan del mismo color. Lo que las separa tiene que ser el tamaño (Lora 16 frente a 18), la cursiva (los motivos) y el sitio. La alternativa para conservar un tercer escalón visible es aclarar la hoja (§5.4).

### 5.2 Los colores de estado

El éxito, el aviso, el error, la información y el carmín **no pueden ser texto sobre la hoja** sin oscurecerse tanto que se confunden con la tinta. Propongo esta regla [propuesto], que además es la de la guía (PAL-03):

> Sobre la hoja y el papiro, **el texto de un estado va en `kTinta`** y el color del estado va en el borde, el icono o el relleno de su chip. El color solo es texto sobre un chip claro (`kPapelLuz` o su tono claro).

Sobre un chip claro ya pasan hoy: éxito 5,63:1, aviso 6,26:1, error 7,19:1, información 6,60:1, carmín 6,11:1 (sobre `kPapelLuz`), y con los chips tintados de los dados, de 5,48 a 6,16:1.

Si se prefiere texto coloreado sobre la hoja sin chip, estas son las tintas de texto que harían falta (cada una, `kXxx` oscurecida hasta luminancia 0,029; todas dan 5,10 / 4,55 / 3,54 sobre la hoja y 7,05 sobre el papiro):

| Nueva constante (nombre propuesto) | Sale de | Valor |
|---|---|---|
| `kExitoTexto` | `kExito` `#2E6337` | `#19361E` |
| `kAvisoTexto` | `kAviso` `#7A4409` | `#482805` |
| `kErrorTexto` | `kError` `#8E1B12` | `#5F120C` |
| `kInfoTexto` | `kInfo` `#29507A` | `#19314B` |
| `kCarminTexto` | `kCarmin` `#9A2A1C` | `#5A1910` |

Sobre la hoja se ven como un marrón oscuro, un verde oscuro, un rojo oscuro y un azul oscuro de casi la misma luminosidad que la tinta: el color ya no alcanza para distinguir el estado, y por eso **no lo recomiendo como camino principal**.

### 5.3 Las pestañas inactivas

El texto oscuro no llega a 4,5:1 sobre la piel de la pestaña (`#907546`): ni siquiera `kTinta` (4,23:1). Hay que aclarar la piel de la pestaña:

| Piel de la pestaña inactiva | Mediana | `kTinta` | `kTinta2` |
|---|---|---|---|
| Hoy | `#907546` | 4,23 | 3,15 |
| **+10 % hacia `kPapelLuz`** | `#9A8052` | **4,90** | 3,65 |
| +20 % | `#A48B5F` | 5,65 | 4,20 |
| +30 % | `#AD966B` | 6,47 | **4,82** |

**Propuesta:** la piel un 10 % más clara y la etiqueta de la pestaña inactiva en `kTinta` (no en `kTinta2`), que da 4,90:1. Con +30 % pasaría incluso `kTinta2`, pero aleja la pestaña del resto del cuaderno. Es un cambio de arte (`pestana.png`), no de código.

### 5.4 Camino B: aclarar la hoja

Es la alternativa a 5.1 si se quiere conservar el tercer escalón de tinta. Mezclar `hoja_a.png` con `kPapelLuz`:

| Hoja más clara | Mediana | `kTinta2` mediana · foco | `kTinta3` (la de hoy) mediana · foco | Error | Éxito |
|---|---|---|---|---|---|
| Hoy | `#AE9F7D` | 5,25 · 4,69 | 4,05 · 3,62 | 3,48 | 2,72 |
| +20 % | `#BCAD8B` | 6,17 · 5,45 | 4,76 · 4,20 | 4,09 | 3,20 |
| **+30 %** | `#C2B491` | 6,67 · 5,86 | **5,14 · 4,52** | 4,41 | 3,46 |
| +40 % | `#C9BB98` | 7,19 · 6,29 | 5,54 · 4,85 | 4,76 | 3,73 |

Con +30 % la `kTinta3` actual pasa también en la fila con foco (4,52:1) y no hace falta tocarla. Pero la hoja pierde su tono de pergamino viejo (se acerca al gris crema), y **los estados siguen sin pasar** (error 4,41, éxito 3,46): la regla de 5.2 hace falta igual. Cambiar la textura es una decisión de arte, que no es mía.

### 5.5 Lo que recomiendo

1. **Camino A** (5.1) + **la regla de 5.2** + **la pestaña de 5.3**. Cambia una constante de `Tema.h` (`kTinta3`), el color del contorno del foco (`kCarmin2`), la piel de una pestaña y los sitios donde hoy hay un estado coloreado sobre la hoja.
2. El **Camino B** solo si quieres recuperar el escalón `kTinta3`.

Y las dos cosas hay que **mirarlas jugando** antes de darlas por buenas (§7).

## 6. Dónde cambiaría cada cosa (sin tocar nada aquí)

| Qué | Dónde | Cambio |
|---|---|---|
| `kTinta3` | `Tema.h`, línea del `kTinta3` | `#4F3B28` → `#3C2D1E`, y el comentario `6,8:1` → los números de este informe |
| Los comentarios de contraste | `Tema.h` (`kTinta`, `kTinta2`, `kTinta3`, `kClaroCuero`) | Medidos sobre la hoja, no sobre `kPapel` |
| Contorno del foco sobre la hoja | `Tema.h` o su uso (`MesaCombateHud.cpp`, `Pincel::filaFoco`) | `kCarmin` → `kCarmin2` |
| Estados coloreados sobre la hoja: insignias (`PaginaCuaderno.cpp`, línea 125: color del estado y fondo al 10 %), «Te faltan N de oro» y «Te quedarán» (`TiendaHoja.cpp`), carmín en Lora 16 (`MesaCombateHud.cpp`, `PaginasCreacionHistoria.cpp`) | cada llamada | Texto en `kTinta`; el color, en borde, icono o chip claro |
| Pestaña inactiva | `assets/textures/pergamino/pestana.png` (la piel `activa: false`) y `Cuaderno.cpp` (`Color::kTinta2` de la etiqueta inactiva) | Piel +10 % más clara y etiqueta en `kTinta` |
| Carmín en pixel sobre el papiro (`Papiro.cpp`, 2 llamadas) | | Pasaría a `kTinta` con icono, o a `#8D271A` (4,57:1 sobre el papiro) |

## 7. Lo que no he comprobado

- **Nada de esto está mirado jugando.** Los números salen de las texturas (`assets/textures/pergamino/`) y de las constantes de `Tema.h`, y se contrastan con una captura de la tienda. Antes de cambiar `Tema.h` conviene ver las capturas con las tintas nuevas.
- **Cada texto del juego, uno a uno, no.** Medí por rol (H1 a D3), con los colores escritos en el código. Los que llegan por variable (los colores de las insignias, los de los estados de combate) se infieren de su uso, y puede haber alguna combinación que no he visto.
- **La textura se aplica sin más** según el código; no he ejecutado el motor para comprobarlo, aunque la captura de la tienda coincide con la piel.
- **Los textos grandes:** no cuento con el 3:1 de WCAG para ninguno.
- **Daltonismo:** no he simulado nada. La regla «nunca solo color» (§5.2) lo cubre en lo que es texto.
- **El brillo del cuero liso:** C3 pasa (9,50:1 de mediana), pero en el 5 % más brillante el contraste baja a 3,62:1, y `kClaroCuero2` sobre ese brillo llegaría a 2,68:1. No he visto que se use `kClaroCuero2` ahí.
- **Los iconos** (3:1 al ser información): no medidos.
- **Los textos «apagados» a propósito** (`conAlfa(kTinta3, 0.6)` en las filas desactivadas: 2,21:1 sobre la hoja): WCAG exime a los controles desactivados, pero el motivo de por qué lo están debe ir en `kTinta2` (BOT-02 de la guía).

## Anexo A · Cómo se mide

Contraste WCAG 2.x entre dos colores: `(L1 + 0,05) / (L2 + 0,05)`, con `L1` la luminancia relativa del más claro. La luminancia relativa de un color sRGB: se pasa cada canal de 0–255 a 0–1, se linealiza (`c / 12,92` si `c ≤ 0,03928`; si no, `((c + 0,055) / 1,055)^2,4`) y se calcula `L = 0,2126 R + 0,7152 G + 0,0722 B`.

Para cada piel de `assets/textures/pergamino/` (hoja, papiro, pestaña, cuero), se tomó el cuerpo del 9-slice (el área que se repite bajo el texto; los cortes están en `materiales.json`), menos 4 px de margen, con los píxeles opacos, y se sacó la luminancia mediana y el 5 % más oscuro (el 5 % más claro, en los textos claros sobre el cuero). La fila con foco mezcla `kLavadoFoco` (`rgb(154, 42, 28)` al 10 %) con la mediana de la hoja. Cuando el texto es oscuro sobre fondo claro, el peor caso es el 5 % más oscuro del fondo; cuando es claro sobre fondo oscuro, el 5 % más claro.

```python
from PIL import Image
import numpy as np

def lin(c):
    c = np.asarray(c, float) / 255
    return np.where(c <= 0.03928, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)

def lum(rgb):
    c = lin(rgb)
    return 0.2126 * c[..., 0] + 0.7152 * c[..., 1] + 0.0722 * c[..., 2]

def ratio(l1, l2):
    hi, lo = max(l1, l2), min(l1, l2)
    return (hi + 0.05) / (lo + 0.05)

def cuerpo(ruta, cortes, margen=4):          # cortes = (arriba, derecha, abajo, izquierda)
    a = np.array(Image.open(ruta).convert("RGBA"))
    t, r, b, l = cortes
    h, w = a.shape[:2]
    px = a[t + margen:h - b - margen, l + margen:w - r - margen].reshape(-1, 4)
    return px[px[:, 3] > 200][:, :3]

hoja = cuerpo("assets/textures/pergamino/hoja_a.png", (18, 18, 18, 18))
l = lum(hoja)
mediana, peor = np.median(l), np.percentile(l, 5)
tinta3 = lum(np.array([0x4F, 0x3B, 0x28]))
print(ratio(tinta3, mediana), ratio(tinta3, peor))   # 4.05 y 2.81
```

## Anexo B · Fuentes

- `MotorGraphico/include/HUD/Pergamino/Tema.h` (tintas y papeles, valores y comentarios).
- `MotorGraphico/assets/textures/pergamino/` (`hoja_a.png`, `hoja_b.png`, `papiro.png`, `pestana.png`, `cuero.png`, `cuero_liso.png`, `materiales.json`).
- `MotorGraphico/src/HUD/Pergamino/BancoMateriales.cpp` (cómo se elige y se estira la piel), `Cuaderno.cpp`, `PaginaCuaderno.cpp`, `Pincel.cpp`, `TiendaHoja.cpp`, `DadosPergamino.cpp` (qué texto va sobre qué papel).
- `planificacion_alcance_y_vision/guia_de_estilo_capturas/tienda_14-10.png` (comprobación).
- Apuntes de la UOC: `Color`, `Llegibilitat` (WCAG 4,5:1 y 3:1).
