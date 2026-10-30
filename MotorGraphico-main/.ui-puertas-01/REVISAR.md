# Qué revisar a las 16:00 (14/10/2026)

Preparado por Claude a las 12:45; actualizado a las 21:55. Hay once paquetes y se aplican **en este orden**, desde la carpeta
`MotorGraphico-main/MotorGraphico`:

```bash
bash out/claude_dialogos04/aplicar.sh      # Claude: el combate y los sitios con nombre (COMBATE-BG3-01 a 11)
bash out/claude_diario01/aplicar.sh        # Opus 2: el diario de misiones (AV14-22)
bash out/claude_cicatrices01/aplicar.sh    # Sonnet: las cicatrices en la ficha (AV15-08)
bash out/claude_reputacion01/aplicar.sh    # Opus 2: reputación y rumores (AV14-23)
bash out/claude_confirmar01/aplicar.sh     # Opus 2: confirmar lo caro e irreversible (AV15-05a), añadido a las 13:15
bash out/claude_ciudad01/aplicar.sh       # Sonnet: ciudad sin ruido (AV15-03), añadido a las 15:05; no depende de los otros
bash out/claude_papel01/aplicar.sh         # Sonnet: la hoja un 30 % más clara (AV15-07, camino B); solo cambia hoja_a.png
bash out/claude_cicatrices02/aplicar.sh    # Claude: las cicatrices salen en la ficha (rellena el campo de cicatrices01)
# Añadidos a las 21:55, hechos por los agentes que lanzó Claude (van después de los ocho):
bash out/claude_dados01/aplicar.sh         # Opus 2: el crítico de dos 6 (D0013) y la probabilidad exacta (AV13-04)
bash out/claude_deuda01/aplicar.sh         # Sonnet: solo herramientas, mide la deuda de contenido (AV12-05)
bash out/claude_puertas02/aplicar.sh       # Haiku: validador de puertas (AV14-20); puertas01 es viejo, NO lo apliques
```

**Lo he probado los once juntos (21:55)**, sobre una copia de tus ficheros de ahora:
- **Los once se aplican** sin un solo «CAMBIADO» ni «NO ENCAJA».
- **Compila entero.**
- **Pruebas:** gl-free 226/227 y GL 11/11.
  - Sobre la copia falla `linea_base_core`, que sin el `.git` cuenta mal (lo mismo le pasó a ChatGPT).
  - En mi máquina de trabajo dice 0 de deuda nueva.

Dos cosas que debes saber:
- **He tocado el paquete de reputación.** Uno de sus anclajes (`out/claude_reputacion01/ediciones.json`,
  el de `PaginasJugador.cpp`) chocaba con el de las cicatrices. Lo he acortado y ahora encajan los dos.
- **Lo de ChatGPT ya va dentro de `claude_dialogos04`:**
  - las plantillas de área (AV13-08);
  - `lugares.json` (AV15-02b), con su prueba y el ajuste del validador de catálogos que proponía.

  No juntes sus ramas (`agente/codex/AV13-08`, `AV15-02a` y `AV15-02b`): duplicarían líneas de
  `CMakeLists.txt`.

En `out/claude_ciudad01/_to_delete/` Sonnet dejó sus pruebas a medias (versiones viejas de sus scripts). No
forman parte del paquete: bórralas cuando quieras.

El paquete del combate **no borra nada**: aparta 11 ficheros del combate antiguo a
`out/claude_dialogos04/borrados/`. Bórralos tú cuando lo hayas probado.

## Qué probar en Release

1. **El combate.** Ve a la plaza (`ciudad_centro`) y deja que salte la emboscada.
   - **La tira de arriba:** cada uno sale en su puesto de la iniciativa, con su tirada.
   - **El turno del rival:** todas las cartas en gris y el panel dice «Espera: juega …».
   - **En tu turno:**
     - las cartas tienen iconos distintos y una franja de color abajo;
     - clic en el suelo para andar;
     - atacas y te quedan pasos;
     - «F Terminar» cierra el turno.
   - **El registro:** dice «el ataque (1) supera la defensa (0,5). 3 de daño (1d6).»
   - **Una carta de área:** señálala y mira el carmín en el suelo. Pilla también a los tuyos.
2. **Los acompañantes.** En el Ocaso (los combates con Venides y Xila):
   - cada uno tiene su turno;
   - los mueves tú;
   - Venides ataca dos veces.
3. **Caer.** Deja que te tumben en un combate que no perdona:
   - sale la escena (rescate si Venides o Xila siguen en pie);
   - ENTER pasa la frase;
   - al final, «Has caído» con «Te queda una cicatriz.»;
   - la cicatriz también sale en tu ficha (TAB).
4. **El diario.** TAB, hoja Misiones: la misión activa con «Ahora: …», sus apartados y lo que elegiste.
5. **La tienda.** El precio cambia con tu reputación ante su facción, y la ficha dice con quién te
   llevas bien.
6. **Confirmar (EH-22 a 30).**
   - En la pausa, «Salir», «Volver al título», «Cargar» y «Guardar» encima de otra preguntan con la hora.
   - Comprar una casa enseña el resumen; «Revender» dice lo que pierdes.
7. **Los sitios con nombre (AV15-02).** Sal a la calle:
   - sobre las puertas cercanas, un cartel con emblema, nombre y función («La Caja Firme / banco y cambio»);
   - al acercarte, el aviso dice «Entrar en La Caja Firme»;
   - el minimapa marca las entradas y abajo pone «Cerca: …»;
   - al entrar, el minimapa se titula con el nombre y arriba sale el rótulo (debajo de «Nuevo objetivo»
     si coinciden);
   - en una tienda, la cabecera dice «El Peso Justo · Tendero del mercado».
8. **La prueba de nivel del editor (F7).** Un combate ahí ya va con las cartas básicas, no con el combate
   antiguo.
9. **La ciudad sin ruido (AV15-03).** En `ciudad_centro`:
   - un carril recto sin nada por la avenida y por cada calle comercial;
   - las termas (sureste del Casco) se pueden entrar;
   - un árbol, una farola o una bandera por delante de ti se ve a través (40 %); ¿demasiado o poco?
   - compilado y probado por Claude a las 15:50 en la cadena de ocho (Sonnet no tenía `cmake`); jugado, aún no.
10. **El papel (AV15-07).** Cualquier hoja (TAB, la tienda, la pausa) es más clara que antes y el borde, igual.
    Compara con `out/claude_papel01/comparacion_papel.png`.
11. **El crítico de dos 6 (D0013).** En un combate, un ataque con dos 6 entre sus dados ya es crítico (antes
    tenían que ser todos 6). Con 4 dados pasa más o menos 1 de cada 8 tiradas.
12. **Las herramientas nuevas** (no cambian el juego):
    - `python3 tools/medir_deuda_contenido.py`: la deuda de contenido (lee `planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md`);
    - `python3 tools/validar_todo.py`: ahora avisa de las puertas rotas (22 avisos, 0 errores).

## Lo que tienes por contestar en la bandeja

- **PO-38:** cuánta moral cuesta caer. Recomiendo −5, y −10 si te rescatan.
- **PO-42 (Opus 2):** cuánto te devuelven al revender una casa. Hoy, el 50 %.
- **PO-44:** ¿abrimos un paso entre la plaza y la iglesia (44 % de desvío)? Recomiendo que no, por ahora.
- **PO-45 y PO-46 (Sonnet):** los hechizos divinos (¿por deidad?, faltan 10 deidades) y la vida de los monstruos.
- **PO-47 (Haiku):** retirar el validador viejo de puertas y si la escalera de `recuerdo_taberna` es a propósito.
- **Sonnet:** ¿adoptas las reglas marcadas [propuesto] de la guía de estilo y de
  `CONTRASTE_Y_TINTAS.md`? (mínimo 16 px, los estados de los botones, las tintas sobre el papel)
- **Las de siempre:** PO-01, 02, 04, 25.4, 31, 33, 34 y 37.
