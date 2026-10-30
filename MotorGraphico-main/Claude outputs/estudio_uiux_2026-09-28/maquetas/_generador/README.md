# Generador de las maquetas

Escribe el HTML de cada maqueta en `../fuente/` y lo fotografía a 1280×720 con Chromium.

## Requisitos

Python 3 con Pillow y Playwright (`pip install pillow playwright` y `playwright install chromium`).

## Órdenes

```sh
python3 pantallas.py            # M01…M09 -> ../M*.png
python3 pantallas.py M06 M07b   # solo algunas
python3 sistema.py              # ../S1_tokens.png y ../S2_componentes.png
python3 comparativas.py         # ../antes_despues/*.png (necesita la galería de capturas)
python3 minimapa.py             # ../fuente/img/minimapa_ciudad.png (necesita el repo)
```

## Piezas

| Fichero | Qué hace |
| --- | --- |
| `lib.py` | Texto pixel con los glifos 5×7 del motor (`glifos_5x7.json` + propuestos), iconos del atlas, componentes, página y render. `px(..., hoy=True)` dibuja como hoy (avance 5, tildes en la fila 0). |
| `pantallas.py` | Una función por maqueta. El contenido sale del repo; los valores de ejemplo están en MODELO_VISUAL.md §8. |
| `sistema.py` | Láminas de tokens y componentes; calcula el contraste WCAG de la paleta. |
| `comparativas.py` | Láminas antes/después con la captura de hoy. |
| `minimapa.py`, `nav.py` | Minimapa real desde la colisión del TMX y los objetos del nivel. |

## Rutas

Por defecto se suponen las carpetas del proyecto: `../fuente`, `../../../capturas_menus_2026-09-28` y `../../../../MotorGraphico/assets`. Se cambian con `MODELO_FUENTE`, `MODELO_SALIDA`, `MODELO_CAPTURAS` y `MOTOR_ASSETS`.

Los fondos de `../fuente/img/fondos/` son capturas del juego sin HUD (parche local `CAPTURAS_SIN_HUD` sobre el harness de la galería de capturas); no se regeneran desde aquí.

## Variante pergamino (`pergamino/`)

| Fichero | Qué hace |
| --- | --- |
| `pergamino/texturas.py` | Texturas pixel art generadas: hoja, papiro, cuero, pestaña, filete, sello y chincheta. |
| `pergamino/muestra.py` | Las cuatro maquetas P03a, P03b, P04a y P05a (van a `../pergamino/`). |
| `pergamino/prototipo.py` | El prototipo autocontenido de pasar hoja; con `--gif` graba también el GIF, el MP4 y la tira de fotogramas (necesita ffmpeg). |
| `pergamino/laminas.py` | La lámina S3 del tema y las comparativas oscuro → pergamino. |

Se ejecutan desde `pergamino/`: `python3 muestra.py`, `python3 prototipo.py --gif`, `python3 laminas.py`. Las texturas se generan en `../fuente/img/pergamino/` la primera vez que se piden.
