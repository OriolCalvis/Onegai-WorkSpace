# Cómo regenerar las capturas

Pensado para rehacer la galería después de tocar el HUD o los menús, sin
capturas a mano. Corre en Linux sin pantalla (el contenedor de Claude
sirve): OpenGL por software (Mesa llvmpipe, `EGL_PLATFORM=surfaceless`)
y una copia de GLFW 3.4 que usa su plataforma «null» con un guion de
teclas y ratón.

## Piezas

| Fichero | Qué es |
| --- | --- |
| `glfw_3.4_null_con_guion.patch` | Parche sobre GLFW 3.4. La plataforma null crea el contexto con EGL + pbuffer, se elige sola cuando no hay X11/Wayland y ejecuta el guion (`src/capture_script.c`) en cada `glfwPollEvents`. También trae un `GL/gl.h` vacío para que compile `imgui_impl_glfw`. |
| `parches_motor_solo_capturas.patch` | Tres parches del motor, SOLO para capturar (no van al repo): `refreshHud()` durante la creación de personaje, el registro de los widgets de Pausa/Opciones/Game Over (`CAPTURAS_COMO_HOY=1` los deja ocultos, como hoy) y `CAPTURAS_TITULO=1`, que pasa por título y creación de personaje aunque se use `--nivel`. |
| `tour.py` | Las escenas (`s01` … `s12`): cada una escribe un guion, lanza el binario y convierte las capturas a PNG. |
| `gen.py`, `nav.py` | Ayudas: órdenes del guion y rutas por la rejilla (colisión del TMX + objetos que bloquean). |
| `manifiesto.py`, `generar_indice.py` | Pies de foto y observaciones; generan `indice.html` y `LEEME.md`. |

## Pasos

1. Copiar al Linux el árbol `MotorGraphico/` y las fuentes de glm 1.0.1 y
   GLFW 3.4 (están en `out/build/*/_deps/{glm,glfw}-src`).
2. `patch -p1` del parche de GLFW sobre una copia de `glfw-src`, y el del
   motor sobre la copia del árbol.
3. Configurar y compilar:

```sh
CC=clang CXX=clang++ cmake -S mg -B build -G Ninja -DCMAKE_BUILD_TYPE=None \
  -DCMAKE_CXX_FLAGS="-O1 -w" -DCMAKE_C_FLAGS="-O1 -w" \
  -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
  -DFETCHCONTENT_SOURCE_DIR_GLM=/ruta/glm-src \
  -DFETCHCONTENT_SOURCE_DIR_GLFW=/ruta/glfw-parcheado \
  -DGLFW_BUILD_X11=OFF -DGLFW_BUILD_WAYLAND=OFF
ninja -C build juego motor_editor level_editor
ln -s ../mg/tools build/tools     # para que «B compilar» del level_editor encuentre build_proyecto.py
```

4. Ajustar las rutas del principio de `tour.py` y `nav.py` y lanzar
   `python3 tour.py` (todas las escenas) o `python3 tour.py s06 s07`
   (algunas). Después, `python3 generar_indice.py <carpeta de capturas>`.

## El guion

Variables de entorno: `GLFW_SCRIPT` (fichero de órdenes), `GLFW_SHOT_DIR`
(destino de los `.ppm`), `GLFW_SCRIPT_QUIT=1` (cerrar al acabar) y
`EGL_PLATFORM=surfaceless`.

```
wait N            espera N fotogramas
sleep MS          espera MS milisegundos de reloj
key K [hold]      pulsa K (ENTER, W, CTRL+S…) y la suelta tras hold fotogramas
down K / up K     mantener / soltar
char TEXTO        entrada de texto (ImGui)
move X Y          cursor en coordenadas de ventana
click X Y [btn]   clic (left|right|middle); dclick X Y = doble clic
scroll DY [DX]    rueda
shot NOMBRE       vuelca el siguiente fotograma a NOMBRE.ppm
log TEXTO         escribe en stderr
quit              pide cerrar la ventana
```

Trucos que costó descubrir: en el juego, los pasos van con `sleep 200`
entre teclas (un toque nuevo antes de 0,14 s sustituye al pendiente). En
el `motor_editor`, las herramientas solo responden al clic en el icono.
En el `level_editor`, la B cierra la búsqueda, así que no se puede
teclear una palabra con B; y los mensajes del hub se cierran con ENTER,
no con ESPACIO.
