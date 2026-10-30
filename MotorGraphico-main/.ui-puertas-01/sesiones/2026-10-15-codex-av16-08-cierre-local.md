# AV16-08 — cierre de El Zurrón del Vado

- **Autor:** Codex / ChatGPT
- **Fecha:** 2026-10-15
- **Commits:** `43fa13d` (sobre `65365fc` y `c83c2d9`), rama `agente/codex/AV16-08`.
- **Deja esperando a Oriol:** integrar las ramas cuando revise; el local y su emblema están terminados. PO-49 sigue en su carril.

## Corrección pedida

No se crean los cuatro locales descartados ni se cambian sus nombres. El validador deja de considerar duplicado un destino compartido; se elimina la lista restrictiva accesosCompartidos. Conserva la unicidad de nombres/fichas y exige ficha para cada puerta. Cuatro regresiones fallan con el código anterior; 17 casos pasan después.

El Zurrón tiene interior/TMX y tendero propios, caña/cebo/trampa/semillas/cuerda/pala/pico. Se consultaron los catálogos oficios y oficios_tablas publicados y la propuesta out/claude_oficios_datos01: caña y pala usan los ids exactos de los útiles de pesca y huerto. No se añaden reglas de oficio, bonificaciones ni cambios de PO-49. Las descripciones de los suministros hablan del objeto, sin detalles de implementación.

La rutina por defecto hacía pasear al tendero por la tienda durante la prueba. Se añade solo su entrada en rutinas_pnj.json con radio cero y actividad preparar_aparejos; ahora atiende desde el mostrador. El único cambio de ciudad_objetos.json son los dos textos del cartel_ropa_sur: el identificador, precio 850 e ingreso 21 permanecen.

El emblema cana_pescar se añade al final de gen_iconos.py. Regeneración con el importador PixelLab existente: 57 iconos, 128×128, frame nuevo 57. Comparación RGBA exacta de las 56 celdas antiguas: idénticas. No se toca render/HUD ni el índice de Pixel.

## Enlace y pruebas

Qwen `dfd2ffe` pone la puerta en (56,58), no la antigua (56,57). La vuelta propia se corrige a (56,59); esa casilla es transitable y su cartel no bloquea. No se escribe ciudad_centro ni su generador.

Comandos desde MotorGraphico-main/MotorGraphico:

```sh
python3 tools/test_validar_lugares.py -v
python3 tools/validar_lugares.py
python3 tools/validar_rutinas_pnj.py
cmake --build build-release --target test_lugares juego -j8
```

Se usa el Python empaquetado con NumPy/Pillow para los validadores gráficos. El binario test_lugares se ejecuta también desde las copias de revisión, para leer allí los assets combinados.

- 17 Python y siete C++ en verde. El contrato nuevo lee los catálogos del manifiesto con ObjectCatalog, resuelve las siete ofertas, comprueba los útiles de oficio, la rutina y el cartel.
- Copia limpia `/private/tmp/codex-av1608-limpio`, con el único enlace previamente propuesto: validar_todo 0 errores/22 avisos. No se presenta como la ciudad nueva integrada.
- Copia completa Qwen+local `/private/tmp/codex-av1608-cierre-qwen`: Release completa, 22 puertas con ficha, 189 enlaces sin error; gl-free 227/231. Los dos tests de contenido fallan por el mismo validar_pnj: estudiante 2 aspectos/1 colocación, nino 4/7, elfos del bosque 4/2, enanos forjadores 3/2. Reproducido con Python completo en Qwen sin modificaciones. Las otras dos guardias arrastran deuda de base (49 instantáneos/44 deudas); no se toca su línea base.
- Sin nuevas fuentes/cabeceras de producto; sin ampliar Application, GameSession o HUD. git diff --check correcto.

Logs en /private/tmp: codex_av1608_cierre_{build_limpio,ctest,validar,aislado,guardia}.log. Son evidencia local, no archivos entregables versionados.

## Prueba en el juego

Pesca.app ejecuta juego con el nivel nuevo y todos los catálogos del manifiesto. Capturas reales en el chat: caña y rótulo «pesca, caza y huerto», diálogo propio y tienda. Compra de una caña: oro 150→135, inventario 0→1. Se cierra la partida al acabar. No se crea un guardado de campaña.

El trabajo sin commit de Claude en el árbol compartido (Application, HUD, creación, Lugares, personajes, catálogos y sus pruebas), y la coordinación de Qwen, siguen intactos. Esta sesión escribe código solo en su worktree. Se retoma AV18-03, ya preparado, sin duplicar su catálogo.
