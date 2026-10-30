# Los sets de tier 2 de la 3.ª iteración y las cartas del Monje y del Mago Chasqueador · 08/10/2026

Sesión de Claude (Cowork). Oriol:

> «Te pediré que con los conceptos que hicimos los sets del Nostale me hagas los sets del apóstata,
> señor de las armas, furia de los creadores, bardo de las 4 estaciones de tier 2, siguiendo los mismos
> criterios, y el mago de los chasquidos y el monje que hice, por cierto, me podrías dar descripciones a
> las habilidades de estos dos últimos, solo les puse la pasiva y el título hahahahah»

Y a mitad: «Por cierto el origen del bardo de las 4 estaciones salió en la cultura de los hombres gato,
lo digo para que adaptes mejor la estética porfi».

Seguimiento: `planificacion/seguimiento/SETS-03.json`. Canon: `planificacion/CANON_SETS_ITERACION3.md`.

## 1. Lo que se decidió (sin preguntar)

- **Los mismos criterios** que las SP y las clases propias: un set entero raro de tier 2 que la clase no
  lleva, el peso de su rol, +1 a los dos atributos (el torso, al principal), paletas propias y lo del
  catálogo teñido por el tema del set.
- **El arma del Monje y del Mago Chasqueador**, que no traen ninguna: los guanteletes del artista marcial
  (jade con los nudillos de oro) y los guantes con que chasquea (forma nueva), los dos en las dos manos.
- **El Bardo, de los hombres gato**: con el vault de Oriol (las Praderas de Oro, la cultura de las
  canciones alrededor del fuego, la Marcha de las Cuatro Estaciones y su balada), un bardo de camino:
  poncho de lana dorada con la Marcha tejida en las cenefas, en su orden (verano, otoño, invierno y
  primavera), pantalones de lana parda, botas blandas («se mueven en silencio») y la corona de hojas, que
  deja libres las orejas. Sustituye al jubón cuartelado con calzas partidas (que se parecía al
  Comediante). El poncho es una propuesta de dibujo, no un dato del canon.
- **Las cartas del Monje y del Mago Chasqueador**: escritas desde su nombre dentro del presupuesto de
  las clases propias, con su gancho. El Monje tira DES; el Mago, INT (todo lo lanza chasqueando). Abdomen
  de Marfil y Repliegue Mágico pasan a ser reacciones; la Recuperación Prestidigitadora, libre; el Paso
  Instantáneo, movimiento. El arranque del Monje pasa a ser pegar, aguantar y curarse (Tres Golpes,
  Abdomen, Jade). Las pasivas (Ki, Cascada Letal) se quedan como las dejó Oriol.

## 2. Lo que se arregló de paso

- El **Mazo del Desahucio** perdía su tinte: TILDES-02 lo renombró y el tema seguía con «deshaucio».
- **Las canciones del Bardo** perdían la etiqueta «Cancion» al volver a importar (tras las tildes, el
  nombre empieza por «Canción»): ahora se mira el slug.
- La importación circular de los trajes (los de la 3.ª iteración se importan al final de los de las
  clases propias).

## 3. Ojo

- **Las filas de PixelLab** del atlas de cabeza y de torso bajan 6 (`importar_prendas` las pone detrás
  de las generadas). El contenido es el mismo y `personajes.json` las lleva a su sitio nuevo.
- Tras `gen_personajes.py`, `pnj_mundo_1.json/png` salen distintos: no se commitean (`git checkout`),
  como `pnj_boundington`.

## 4. Pruebas

- `test_clases_companeros` 13/13 (los sets, las cartas del Monje y del Mago en combate).
- ctest con `EGL_PLATFORM=surfaceless`: 216/217, solo `linea_base_core` (sigue en 44).
- `test_validar_sprites` sin GeneradoresTests: 53/53. `test_personajes_son_reproducibles` falla como en
  la base (las filas de PixelLab no salen de `generar()`); el de props revienta con SIGFPE en el
  contenedor.
- `validar_todo`: 0 errores; `validar_catalogos_rpg`: sin fallos.
- Láminas de los seis sets en las cuatro tallas, y el Bardo en Cara Loca, Cazador Nocturno y humano.

## 5. Para Oriol

1. ¿Le gustan los seis sets, y el Bardo como bardo de camino de las Praderas de Oro?
2. La compañera bardo (Ilse de las Estaciones) sigue siendo una elfa del bosque provisional: ¿Cara Loca?
3. ¿Le valen las descripciones del Monje y del Mago Chasqueador?
