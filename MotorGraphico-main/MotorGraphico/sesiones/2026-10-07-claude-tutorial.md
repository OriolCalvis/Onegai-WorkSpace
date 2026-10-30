# La creación y el tutorial de Boundington, de punta a punta · 07/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-07-claude-particulas.md`. Oriol:

> «Limpiar la creación de personaje y depurar desde el inicio hasta completar la primera misión al
> llegar a Boundington».

Seguimiento: `planificacion/seguimiento/TUTORIAL-01.json`.

## 1. Cómo se ha jugado

Con el juego de verdad (`juego`), sin pantalla: GLFW en su plataforma nula, que lee un guion de teclas y
saca capturas (`GLFW_SCRIPT`, `GLFW_SHOT_DIR`). Las rutas por la ciudad, con un BFS sobre el TMX y los
objetos que bloquean. El recorrido:

1. Título → Nueva partida → las siete hojas (Ilse, humana de las Marcas, Acechador de Tejados).
2. El Vacío: la narración, el cofre, ponerse el kit.
3. El prólogo (la barca, el relato de Venides) y la llegada a Boundington.
4. El tutorial: el tendero del mercado, el joyero y el sastre; volver a la plaza; la emboscada del
   saqueador de los Perdidos; ganar (y, en otra pasada, huir); «Nuevo objetivo: Buscar cama en la
   Taberna Humilde» (objetivo 2/6).

## 2. Lo que fallaba y cómo queda

- **La hoja Clase**: el kit de cinco piezas pisaba «CON EL KIT PUESTO». Con cinco, la lista empieza más
  arriba y más junta.
- **El Vacío**: ponerse el kit subía el tope de vida pero no la vida (5/9 al empezar). Lo que el equipo
  sube o baja del tope se nota en la vida de ahora; quitarse algo no mata.
- **El sastre no estaba en ningún nivel**: el tutorial no se podía acabar. Va en la sastrería, junto a
  la mesa de corte. `validar_todo.py` lo ve ahora: con quien se habla tiene que estar en algún nivel.
- **Hablar con el tendero abría la tienda en el acto**: las líneas del beat solo iban al registro y
  ENTER compraba pociones. Ahora la hoja del diálogo y, al cerrarla, la tienda. El título de la tienda
  se partía palabra a palabra encima de la lista (la caja de 100 por defecto): en una línea.
- **La emboscada se saltaba lo que dice**: el combate empezaba en el acto y sus tres líneas se perdían.
  Ahora el combate queda montado y el diálogo va delante; al cerrarlo, se pelea. Lo mismo para los
  saqueadores del Ocaso y Gorran.
- **La victoria no se contaba**: «tut_victoria» esperaba a la siguiente charla. Acabado el combate, la
  aventura dice lo suyo, como al cerrar un diálogo.
- **El saqueador salía encima de Ilse**: ahora en una casilla libre de al lado.
- **Se salía de la sastrería a la otra punta de la ciudad**: tiene dos puertas a la calle y una sola
  salida, que llevaba a la del sur, escondida entre las casas (y ahí saltaba la emboscada). Ahora se sale
  por donde se entra: la puerta en la que se aparece lleva a la del nivel de antes. Pasaba igual en el
  banco, los baños, el mercado y la posada.
- **El diálogo partía las frases**: una línea larga se partía por renglones («…de un cadáver. Se
  acerca» / «sobre una barca…»). Ahora por frases.

## 3. Guardar la ficha (GUARDAR-FICHA)

La partida guardaba el nivel, el tier y los hitos, pero no quién es el héroe. En la misma ejecución no
se notaba (el jugador es el mismo); al abrir el juego otra vez y «Continuar», Ilse volvía sin clase (sin
cartas), sin equipo y sin su aspecto. Y peor: la aventura activa volvía a ser la del prólogo, ya hecho,
así que no disparaba ni un beat y el tutorial no se podía acabar.

Ahora el guardado lleva al héroe (`hero`: pueblo, clase, origen, deidad, las siete respuestas, lo puesto
y su aspecto; clave nueva, un guardado de antes carga igual) y, al cargar, se rehace la ficha, se viste
y la aventura vuelve a ser la del capítulo en el que iba. Comprobado: guardar a mitad del tutorial,
cerrar, abrir, «Continuar» y acabarlo con sus cartas.

Seguimiento: `planificacion/seguimiento/GUARDAR-FICHA.json`.

## 4. Los pueblos en castellano (RAZAS-01, 08/10)

La hoja Raza enseñaba textos a medias en catalán en 30 de los 41 pueblos jugables («Resistents al fred
extrem i amb gran orientación en terrenys difícils») y la CULTURA se cortaba en la ficha de la plantilla
vieja («Edad: Alcanzan la madurez... Tama...»). Ahora `races.json` va en castellano (35 pueblos tocados:
solo lo que estaba en catalán, y fuera la Sabiduría, que no es un atributo del juego) y la CULTURA es la
descripción sin esa ficha; el códice la sigue enseñando entera.

Seguimiento: `planificacion/seguimiento/RAZAS-01.json`.

## 4a. Las tildes de Boundington (TILDES-01, 08/10)

Los textos de la campaña (prólogo, primer día, segundo día y Ocaso) y los nombres y frases de los
objetos y PNJ de la ciudad recuperan sus tildes, eñes y ¿¡ («Así que eres el nuevo», «Poción», «un año»,
«¿Qué te sirvo?»). Solo ortografía: quitando las tildes, cada texto es idéntico al original.

Seguimiento: `planificacion/seguimiento/TILDES-01.json`.

## 4b. Lo que queda

1. La tienda sigue siendo el panel oscuro de antes del pergamino.
2. Las otras aventuras (el Norte, Este Norte...) siguen sin tildes.
3. Los rasgos de 38 pueblos son de la plantilla vieja (nombre genérico, trucos, 2d6 en cono): rehacerlos a
   cartas es de Oriol. Y los 43 son «poco común».
4. Huir de la emboscada da la derrota del tutorial, que habla de haber caído.
5. Guardar dentro de un interior no recuerda por qué puerta se entró: al salir, el destino de siempre.

## 5. Para Oriol

- ¿Se sale por donde se entra en todas las puertas? Si alguna tiene que llevar siempre al mismo sitio,
  se le pone a mano.
- ¿Primero se habla y luego se compra también con los tenderos sin beat?
- ¿Caer en una misión debería seguir la historia como huir, en vez de acabar la partida?
- ¿Rehacemos a cartas los rasgos de los 38 pueblos de la plantilla vieja? ¿Y qué religiones son canon:
  las de las razas (Chronos, Eros, Sofía...) o las deidades de `deities.json`?
