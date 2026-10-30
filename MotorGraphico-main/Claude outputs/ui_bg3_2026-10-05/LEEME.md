# UI-BG3-01 — Las pantallas de las maquetas, ya en el juego · 05/10/2026

Encargo de Oriol sobre las maquetas C01–C04: «aplícalo al juego, m'encanta». Mejor 7 hojas
que 14 pasos, y los kits del cofre parametrizados.

Las capturas de `capturas/` son del juego real (OpenGL por software, 1280×720), con el
árbol del Mac de las 13:40 UTC más este paquete. `combate_barra.gif` es un combate entero
con la barra de cartas. `kits_63.md` lista el kit que saca la regla para cada clase.

## Qué hay

**Ratón en todo.** Título, creación, pausa, opciones, derrota, cofre y combate. Pasar
por encima señala y el clic elige. La rueda pasa las listas largas. El teclado funciona
igual que antes.

**Título en papiro (C01).** «Continuar» apagado lleva su motivo debajo.

**Creación en 7 hojas (C02).**

- Las hojas son Raza, Aspecto, Clase, Origen, Historia, Deidad y Nombre. Q y E pasan de
  hoja; WASD o las flechas se mueven por la rejilla.
- Raza y Clase son rejillas de retratos. Al centro, el personaje andando con sus
  atributos; a la derecha, la ficha de lo señalado.
- La historia es una matriz de 7 × 5 tarjetas.
- El personaje sale sin equipo: el kit se lo da el cofre.

**El cofre de El Vacío (C04).**

- Con la E al lado se abre y el kit de la clase va a la mochila.
- Sale el objetivo «Ponte tu equipo (TAB, hoja Inventario)», y «Ponérmelo» abre el
  cuaderno.
- El prólogo espera a que lo lleves puesto (o al temporizador secreto, para que nunca se
  quede colgado).

**El kit de cada clase sale de una regla con parámetros** (`assets/rules/kit_basico.json`):

- el arma de la clase;
- piezas sin requisitos (tier 1, rareza hasta *uncommon*) que suben al menos +1 el
  atributo principal y +1 el secundario;
- 3 piezas como mucho.

Las 63 clases reciben un kit así (lo comprueba `test_kit_basico`). El del Bardo de las
Cuatro Estaciones es Sable Ligero de Viaje, Armadura de Cuero (+1 DES) y Amuleto de Foco
(+1 CAR), como en tu propuesta.

**Combate con barra de cartas (C03a–C03c).** Sustituye al panel del enemigo y al menú de
comandos.

| Pieza | Qué hace |
|---|---|
| Bandeja abajo | Básicas (1 Atacar, 2 Poción ×N, 3 Huir), la mano (4–0) y el hueco «+3 de creador» |
| 1–0 | Una vez señala la carta (la carta grande la explica); otra vez, la juega |
| A/D o ← → · ENTER | Mover y jugar |
| Ratón | Encima señala; el clic juega |
| Carta grande | Tipo, qué hace, a quién, cómo se defiende el rival, si cierra el turno, o por qué no se puede jugar |
| Tira del turno | Acción, movimiento, canaliza y reacción; lo gastado, tachado; «REACCIÓN LISTA» en carmín |
| Orden de turno | La ronda y los retratos: tu aspecto y el sprite del enemigo, con el marco en quien juega |
| Placa del enemigo | Vida y estados («Cancion del Otono · 2») |
| Registro | El log del combate; L o clic lo esconde |
| Estación del Bardo | Su emergente al empezar; «Ahora no» la deja para luego. La elegida sale en el retal del héroe (◆ INVIERNO) |
| F Pasar | Cierra el turno sin la acción principal |
| Turno del rival | Al cerrar el tuyo sale «TURNO DE PERDIDO SAQUEADOR» 0,8 s y luego actúa |
| Reacción | La preparada brilla en oro; cuando salta, aviso «¡Bendicion de las Estrellas!» |
| ESC | Con el emergente abierto, lo cierra; si no, pausa como siempre |

Sin ficha, en la prueba de nivel del editor, las habilidades de siempre (Tajo, Cura) salen
como cartas. Desde H2-06 no se podían elegir en el menú.

## Cómo probar el combate

En El Vacío no hay enemigos. Para probarlo sin jugar hasta el primero, yo puse en una
copia de `assets/levels/interior_vacio.json` (no en el repo) este objeto más, delante de
la salida:

    {"objectId": "perdido_saqueador", "position": {"x": 6, "y": 7}}

Crea el personaje, pasa los diálogos y pulsa W dos veces.

## Decisiones que conviene revisar

1. **El rival ya no actúa en el acto.** Lo hace tras su rótulo (0,8 s), como en la
   maqueta C03c. Mientras, la barra no juega.
2. **F Pasar es una acción nueva de la mesa** (`#pasar`, en `CartasEnCombate`). Cierra el
   turno (lo dice el registro) y actúan las invocaciones, como al cerrar con la acción
   principal.
3. **El atlas de letras tiene Lora de 13 px**, para que los nombres quepan en las cartas
   («Bendicion / de las / Estrellas»). El resto del atlas sale idéntico, comprobado
   píxel a píxel; se regenera con `tools/gen_letras_pergamino.py`.
4. **Los nombres de cartas y estaciones llevan el texto del catálogo**, que va sin tildes
   («Cancion del Otono», «OTONO»). Viene así de `skills.json` y no lo he tocado.

## Preguntas abiertas

- **¿Es lo esperado para el primer combate?** Un crítico del Perdido saqueador hace 22
  de daño a un bardo de 19 PV sin el kit. En mis pruebas, jugando solo «Atacar», cuatro
  de cinco combates se pierden en la segunda ronda. Con la estación, la reacción y la
  Canción del Otoño se gana.
- **El fondo del título** es El Vacío, no la ciudad de la maqueta.
- **Si no cargan las letras del pergamino**, el título, la creación y el combate se
  quedan sin pantalla: los widgets viejos ya no están. El resto del juego cae al HUD de
  siempre.
- Siguen abiertas las de H2-06 (cartas de creador a tier 1, etc.).

## Verificación

- **Pruebas nuevas, todas en verde.**
  - `test_barra_combate` 8/8: la mesa real del Bardo y del Apóstata; estación, «Ahora
    no», 1–0, opciones, Pasar, espera del rival y aviso de la reacción.
  - `test_pergamino` 49/49, con 4 casos nuevos del combate pintado: zonas del ratón,
    nada se pisa, cabe en 1280×720 y en 1600×900 baja y se centra.
  - `test_cuaderno_creacion` 12/12 y `test_kit_basico` 11/11.
  - `test_cartas_combate` 30/30, con `#pasar`.
- **ctest entero:** 203/211. Las 8 que fallan fallan igual sin el paquete:
  - `demo_nova_partida`, `test_gdb12_screens`, `test_catalog_content` y la guardia
    entera;
  - `test_g14_hud_timers_commerce` no enlaza en Linux;
  - `test_application` y `test_application_cableado` revientan igual en la base;
  - `test_application_scenes` baja de 5 fallos a 4: la prueba del combate ya pasa.
- **Guardia:** las mismas 44 deudas que sin el paquete.
  - `Application.h` baja de 519 a 515 líneas y de 109 a 89 miembros.
  - `Application.cpp` baja de 1347 a 1188 líneas.
  - Ningún fichero nuevo pasa de 500 líneas, y todo lo nuevo de lógica va sin GL.
- **Juego real:** título, las 7 hojas con clics, cofre y kit puesto (19/20 PV) y combates
  completos: victoria, derrota, reacción, pasar y registro. Lo enseñan las capturas.
- **Aplicado al árbol del Mac** el 05/10 a las ~15:50 (hora del Mac): 74 ficheros, 30
  nuevos y 44 modificados.
  - Comprobé la pre-imagen md5 de los 74.
  - Otras sesiones habían cambiado tres desde las 09:40 UTC: `interior_vacio.json`, que
    ahora tiene mapa propio (el cofre sigue en suelo, en (7,8));
    `assets/proyectos/boundington.json` y `src/Engine/ApplicationNivel.cpp`. Los fusioné
    a 3 bandas sin conflictos.
  - Antes de escribir, compilé y probé entero ese árbol (Mac 13:40 UTC + paquete).
  - Post-imagen 74/74.
  - Sin commit. La copia del paquete está en `MotorGraphico/out/claude_bg3/ui_bg3_01.tar.gz`.

## Para el commit

Los ficheros nuevos van en el **mismo commit** que sus líneas de CMake:

- `cmake/CreacionBg3Tests.cmake` y su `include()` en `CMakeLists.txt`;
- las fuentes nuevas de `motor_level` en `CMakeLists.txt` y las de `motor_pergamino` en
  `cmake/Pergamino.cmake`.

Si no, la guardia avisa de fuentes sin versionar. También van `letras_pergamino.png` y
`letras_pergamino.json` (regenerados).
