# PixelLab para Claude · extensión de la app de escritorio

PixelLab genera pixel art con IA (personajes en 4/8 direcciones con sus
animaciones, tilesets, baldosas isométricas, objetos, iconos y paneles de UI).
Esta extensión se lo da a Claude en las sesiones de Cowork vinculadas a este Mac.

## Instalar (una vez)

1. Doble clic en `pixellab.mcpb`: se abre la app de Claude con el instalador.
   Si no se abre, en la app: **Ajustes → Extensiones** y arrastra el fichero.
2. Cuando lo pida, pega el **token de la API de PixelLab**
   (https://api.pixellab.ai/mcp). Se guarda en el llavero del Mac; no se
   escribe en ningún fichero del proyecto.
3. **Carpeta del proyecto**: deja `MotorGraphico-main`. Es el único sitio donde
   la extensión puede guardar lo que descargue.
4. Actívala. En la siguiente sesión de Cowork vinculada a este Mac, Claude ve
   las herramientas de PixelLab.

## Por qué una extensión y no el comando de Claude Code

Claude trabaja en un contenedor en la nube y en la máquina virtual de Cowork, y
los dos salen a internet por un proxy que solo deja pasar los dominios
permitidos por la cuenta, y `api.pixellab.ai` no está en esa lista (lo
comprobé el 01/10: el proxy rechaza la conexión desde los dos sitios). La
extensión corre en el Mac, dentro de la app, con tu red; la app reenvía sus
herramientas a las sesiones de Cowork.

El comando que da PixelLab (`claude mcp add pixellab https://api.pixellab.ai/mcp
-t http -H "Authorization: Bearer …"`) sirve para **Claude Code** en la
terminal; si lo usas, funciona tal cual allí.

## Qué hace

- Reenvía todas las herramientas de PixelLab (`create_character`,
  `animate_character`, `create_isometric_tile`, `create_map_object`,
  `create_tiles_pro`, `create_ui_asset`, `create_image_pixflux`…). Son
  asíncronas: devuelven un id y se consultan con su `get_…`.
- Añade **`guardar_en_proyecto(url, ruta)`**: descarga lo generado (las URLs de
  descarga de PixelLab) y lo guarda en `ruta`, relativa a la carpeta del
  proyecto. Nunca escribe fuera de esa carpeta y no sobrescribe sin
  `sobrescribir=true`.

**Cada generación gasta créditos de tu cuenta de PixelLab.** Claude pide permiso
antes de lanzar tandas.

## Fuente y pruebas

`fuente/` tiene la extensión entera: `manifest.json`, `server/index.js` (sin
dependencias, Node 18 o más; la app trae el suyo) e `icon.png`.
`fuente/pruebas/prueba_puente.py` la prueba contra un PixelLab falso, sin red
(29 comprobaciones: sesión, notificaciones, SSE, llamadas en paralelo, rutas
fuera de la carpeta, token que no sale en los registros…):

    cd fuente && python3 pruebas/prueba_puente.py

Para rehacer `pixellab.mcpb`: un zip con `manifest.json`, `icon.png` y
`server/index.js` en la raíz, con la extensión `.mcpb`.
