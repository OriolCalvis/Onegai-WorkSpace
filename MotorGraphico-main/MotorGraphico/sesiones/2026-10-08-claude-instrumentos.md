# Los instrumentos, actuar en la calle y las opciones en el diálogo · 08/10/2026

Sesión de Claude (Cowork). Oriol, tras los sets de la 3.ª iteración:

> «Aprovechamos para crear una variedad de instrumentos musicales? Escobas y otros objetos que no sean
> armas pero podamos usar en un juego de rol? Para hacer el tonto, por ejemplo una almohada, cerveza, una
> antorcha, estandarte, shirukens, por ejemplo podamos hacer una performance en la calle y depende de
> nuestra competencia musical y tiradas, atraer a la gente a nuestro espectáculo, por ejemplo también una
> cuerda, hacemos monturas, solo disp fuera de la ciudad después de terminar boundington, alguna cosa más
> que veas que pueda ser necesaria en este aspecto? Sino continuamos integrando la historia de
> boundington»

Seguimiento: `planificacion/seguimiento/INSTRUMENTOS-01.json`, `ACTUAR-01.json` y `OPCIONES-01.json`. Canon:
`planificacion/CANON_OBJETOS_DE_ROL.md` y `CANON_ACTUAR_EN_LA_CALLE.md`.

El orden que se le dijo: primero los objetos, luego la actuación en la calle (ACTUAR-01), las monturas
más adelante (MONTURAS-01) y después, la historia de Boundington.

## 1. Lo que se decidió (sin preguntar)

- **Equipo de la mano principal**, no consumibles: así se ven en la mano y se pueden «usar» para hacer el
  tonto (la escoba, la almohada, la sartén y la jarra pegan como un arma improvisada).
- **Lo añadido a la lista de Oriol**: la pala, el pico, la caña de pescar, el catalejo, la sartén, las
  bolas de malabares y las ganzúas (estas, solo a la mochila).
- **Las tiendas**: ninguna de las tres del tutorial (el tendero del mercado, el joyero y el sastre), que
  las fijan las pruebas tal y como las presenta el tutorial.
- **El brillo**: brillan como todo el equipo de tier 1 (poco). Se probó que no brillaran, pero el motor
  exige que toda pieza brille (`BrilloEquipoTests`), y se dejó como el resto.
- **El id**: OBJETOS-01 ya era el de los objetos de Este Norte (otra sesión); este trabajo es
  INSTRUMENTOS-01.

## 2. Ojo

- `ciudad_objetos.json` pasa de 98 a 121 objetos (`ObjectCatalogTests`, con su comentario, como cuando
  TIENDA-03 añadió la daga).
- Tras `gen_personajes.py`, `pnj_mundo_1.json/png` salen distintos: no se commitean (`git checkout`).
- El laúd y el tambor de los bardos aún no llevan la etiqueta «Instrumento»: se les pondrá en ACTUAR-01,
  que es quien la necesita.

## 3. Pruebas

- ctest con `EGL_PLATFORM=surfaceless`: solo `linea_base_core` (sigue en 44).
- `test_validar_sprites` sin GeneradoresTests: 54/54 (con `test_los_objetos_de_rol`).
- `validar_todo`: 0 errores; `validar_catalogos_rpg`: sin fallos.
- Láminas de los 22 en la mano.

## 4. Para Oriol

1. ¿Te valen los instrumentos y los objetos, y dónde se venden?
2. El Señor de las Armas: la imagen es la de un personaje conocido, que no se copia; ¿dos o tres diseños
   propios para él?
3. Sigue pendiente de SETS-03: Ilse de las Estaciones, ¿Cara Loca?, y las cartas del Monje y del Mago.

## 5. Actuar en la calle (ACTUAR-01)

Lo segundo del orden que se le dijo. Decidido sin preguntar:

- **Un sitio, un objeto**, como el cartel de negocio: el rincón del músico (una caja) con su bloque
  `actuacion` (los dados del rival, la afluencia y el lugar). Cuatro en Boundington: la fuente, la puerta de
  la ópera, la plaza del mercado y la taberna.
- **La tirada es la de Interpretación** (CAR), enfrentada a la indiferencia de los que pasan, con la foto en
  la hoja del diálogo. El instrumento de la mano suma (+1, +2 si es raro, +3 si es épico o mejor), y las bolas
  de malabares también.
- **El grado da el público y el oro** (nadie, un cuarto, la mitad o todos; 1, 2 o 3 de oro por cabeza) y el
  sitio **descansa 120 pasos** (el tiempo del juego son los pasos). Da unas monedas, no una mina (canon §3).
- **El código vive en `Game/Actuacion`**: `GameSession.h` está en su techo de líneas, como ya pasó con el
  cofre del Vacío.
- **El laúd y el tambor de los bardos** pasan a ser «Instrumento» (cp_bardos). Reimportar las SP cambiaba
  también la «fuente» de dos cartas (sp_mago: «Maná» con tilde): no se ha tocado, es de otra sesión.

Pruebas de ACTUAR-01: `test_actuacion` 5/5; ctest, solo `linea_base_core` (44); sprites 54/54; los
validadores sin errores; una captura del juego con las cajas en la plaza.

Para Oriol: ¿le vale cómo funciona y lo que da? ¿Más sitios? ¿Se puede actuar durante el Ocaso?

## 6. La historia de Boundington: opciones y variables (OPCIONES-01)

«Sino continuamos integrando la historia de boundington»: el plan está en el documento «Los Perdidos de
Boundington — revisión para jugarla entera» (05/10), que pone primero el motor (fase 1: opciones en el
diálogo, variables, franjas del día, huir y pelear sin matar) y después el contenido del docx (fases 2 a 4).
El docx no está en el contenedor y el Mac no estaba conectado, así que se hizo el motor:

- **Opciones en el diálogo**: un beat ofrece hasta cuatro respuestas con sus requisitos, sus líneas y sus
  efectos. En el juego, la lista sale encima de la hoja del diálogo en la última página; W/S, 1-4 o el ratón
  eligen y ENTER (o un clic) decide. Con una elección pendiente el diálogo no se cierra.
- **Variables**: `setVariable`, `addVariable` (con topes) y `requires.variables` (valor exacto o rango).
  Servirán para los puntos, el afecto de Venides, la reputación y las franjas.
- **Ninguna escena nueva**: las que piden elegir (Sandes, el culto, Verina, Venides retenido) salen del docx.

Pruebas: `test_opciones_dialogo` 7/7; ctest, solo `linea_base_core` (44); `validar_todo` sin errores; jugado
en el juego con un beat de prueba (sin tocar el contenido).

Para Oriol: ¿le vale la lista de opciones? Y el acceso a la carpeta del vault para leer la Campaña 01.
