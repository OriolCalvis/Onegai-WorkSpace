# Boundington jugable entera: el motor que faltaba y las escenas del vault · 08/10/2026

Sesión de Claude (Cowork). Oriol, con la lista «Boundington: lo que hay que sacar del gestor documental»:

> «Té, continua la programació dins el motor gràfic del següent:»

Seguimiento: `planificacion/seguimiento/FASE1-BND-01.json` (el motor) y `BND-ESCENAS-01.json` (las
escenas). Canon: `planificacion/CANON_BOUNDINGTON_CAMPANA01.md`.

## 1. Lo que se hizo

1. **Sacar las escenas del vault** (la carpeta «Onegai 2»): la Campaña 01 [C01] y PARTIDAS.docx [PART], capítulo
   a capítulo, en fichas de escena con el texto literal entre «», «NO ESTÁ» donde el vault calla y las dos
   versiones donde los docx no coinciden. Las notas de Obsidian de la campaña están casi vacías (0 bytes).
2. **El motor (FASE1-BND-01)**: las franjas del día (`pasarFranja`; el día y la franja son variables), la tabla
   aleatoria (`tablaAleatoria`: el 1d4 del traje roto), los descansos (`descanso` corto y largo), huir a 0 PV
   (`huyen`) y pelear sin matar (`sinMatar`) en los combates, y dar objetos (`darObjeto`). El HUD pone «Día 1 ·
   Mañana» debajo del nombre de la zona.
3. **Las escenas (BND-ESCENAS-01)**: el primer día entero (78 beats), el segundo día entero (47) y la primera
   parte del Ocaso (27). En `CANON_BOUNDINGTON_CAMPANA01.md` §9 y §10, escena a escena, cómo queda y qué decidí.

## 2. Lo que se decidió (sin preguntar; para revisar)

Está en el canon, §9.1 y §10.1. Lo gordo:

- **El reloj lo lleva el jugador**: Skilla ofrece comer (pasa a la tarde), esperar a la noche o subir a dormir.
- **La niña del gato o los Verdaderos**, por la hora (por la mañana el gato).
- **Venides, 20 hombres**: una prueba (la iglesia o la guarida) y tres testimonios (la variable `pistas`).
- **La fiesta es la segunda noche** (como en el juego y en el plan del 05/10); en los docx es la primera. Por eso
  un rumor literal cambia «en dos días» por «mañana».
- **Carisma → Persuasión** e **Historia → Conocimiento Arcano** (no hay Historia en el catálogo).

## 3. Ojo

- Las aventuras se escribieron con un generador (no está en el repo): el JSON es la fuente. Se mantuvo el formato
  (sangría de 2 y los `grantLevel` en una línea).
- Las partidas guardadas a mitad de un día, de antes de las franjas, llegan sin reloj: `b1_reloj`, `b2_reloj` y
  `b3_reloj` lo ponen en marcha en el primer tick.
- `b2_cierre` ya no pide haber resuelto los gitanos y la fiesta: se puede dormir sin hacerlo.
- Jugando con el arnés, W/S en el título no cambiaban la imagen. Por el código, el papiro del título solo se
  repinta en `refreshHud()`, y en el título no se llama al mover la selección (`Application::processInput`). No es
  de esta tarea y no lo toqué: queda anotado.

## 4. Pruebas

- `test_fase_uno_boundington` 9/9; `demo_boundington` (los tres días, 152 de 152 beats disparados);
  `demo_campana_boundington`; `test_boundington_tutorial`.
- ctest en la VM del Mac (Ubuntu aarch64, Xvfb): todo en verde salvo `linea_base_core`, que sigue en 44.
- `validar_todo`: 0 errores (los 10 avisos de antes).
- **Jugado en el juego**, en la VM del Mac con Xvfb y GLFW X11 con un guion de teclas (el arnés de antes, pasado de
  la plataforma nula a X11; capturas en `out/claude_traspaso1008/ver/`): el rótulo del día y la franja; Skilla y sus
  cuatro respuestas; «Esperar a que anochezca»; Aigren, su tirada y la herrería cerrada; Venides de noche (Edul y la
  zona rica); el amanecer del segundo día; el Barrio Alto de noche, el vigilante y su tirada.

## 5. Para Oriol

Las 12 preguntas de `CANON_BOUNDINGTON_CAMPANA01.md` §8. Las que deciden lo siguiente:

1. **Sandes**: ¿con cuánta vida y quién le acompaña? (su ficha del docx es de 105 PV).
2. **Los guardias** que retienen a Edul y a Venides: hace falta su ficha.
3. **¿La fiesta es la primera o la segunda noche?**
4. **Los textos que faltan** (los rumores de las mesas, el sermón de la plaza, las palabras de Sandes, Edul y
   Verina): ¿los escribo yo con lo que hay, o los escribes tú?

## 6. Lo siguiente

La segunda parte del Ocaso: Sandes (con `huyen`), Edul y Venides retenidos, el fuego de la barricada y los mapas del
máster. Después, las monedas (pp, pb, po) cuando se sepa el cambio.
