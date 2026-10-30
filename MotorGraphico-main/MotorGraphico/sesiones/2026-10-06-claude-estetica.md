# La estética de cada objeto: forma + esencia + acabado · 06/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-06-claude-hitos.md`. Oriol:

> «Para cada arma y armadura necesitaremos vincularlo a un sprite, hay algunas que pueden
> compartir sprites, pero todas tienen que aplicar una estética.»
> «Forma + esencia + acabado + algunas únicas que sean míticas o extremadamente raras, y
> obviamente las únicas, aquellas que solo se puedan conseguir.»
> (De los que no tenían pieza) «Estos solo algunos, y yo pondría que dieran partículas al
> jugador, dependiendo de lo que den o su lore un brillo acorde.»

Seguimiento: `planificacion/seguimiento/ESTETICA-01.json`.

## 1. Cómo se ve cada objeto

Antes, 391 objetos se vestían con solo 58 piezas (76 con la misma daga de hierro) y 22 no se
veían. Ahora cada objeto de `equipment.json` sale de `tools/actores_estetica.py` con:

| | Qué es | De dónde sale |
|---|---|---|
| **Forma** | la fila del atlas | armas: 40 piezas base en `tools/actores_formas.py` (28 formas nuevas y 12 de siempre); ropa: la pieza que ya elegía `actores_catalogo.py` |
| **Esencia** | la paleta de lo principal | las 20 esencias de las armas, las 20 deidades de las reliquias, el nombre (series, armas con nombre) o las etiquetas (Sagrado, Arcano, Sigilo, Oscuro, Naturaleza, Conjuro) |
| **Acabado** | guarnición y remates | Milicia (acero y azul), Viaje (hierro y cuero), Insigne (plata, tinte más vivo), Sublime (oro, tinte más hondo); en las armas de esencia, la rareza: latón, plata, oro, oro rojo |
| **Única** | dibujo propio | las 19 armas con nombre y las 20 míticas de las deidades: de momento, su forma con su tema y oro rojo |

Resultado: **401 objetos se ven, 398 distintos** (comparten la cota de malla común y la armadura
de malla, y la *Diadema de Lectora Insigne* con la plateada de siempre). Quedan 12 sin pieza:
anillos, amuletos, lentes y las reliquias pequeñas de la mano; tendrán partículas.

## 2. Las formas

Cada forma se dibuja una vez con **tres materiales**: lo principal (hoja, cabeza, cara del escudo,
cristal), el mango y la guarnición (guarda, pomo, virolas, cerco). Así la esencia cambia la hoja y
el acabado la guarnición sin dibujar nada más.

- Hojas: daga, cuchillo de cazador, estilete, espada, espada corta, sable (con arco de guarda),
  mandoble, estoque.
- Cabezas: hacha, hacha de mano, gran hacha de dos filos, maza, martillo, macana tachonada.
- Astiles: lanza, pica, jabalina, tridente, guadaña.
- A distancia: arco, ballesta, honda, látigo.
- Bastones: bastón, bastón herrado, vara, cetro, tea.
- Mano izquierda: escudo de cometa, rodela, broquel, pavés (los cuatro embrazados: tapan el
  puño), brazal, orbe, grimorio, símbolo sagrado, talismán, kit de herramientas, espejo, farol.

Las formas de siempre (espada, daga, hacha...) tienen ahora la guarnición marcada como tercer
material; sin él se pintan igual que antes, así que las piezas viejas y los PNJ horneados no
cambian ni un pixel.

## 3. El tinte

Una pieza con tinte es su fila con los cinco tonos de unos materiales cambiados por los de
otros. Va en el texto de la pieza, por los mismos caminos que las demás:

```
"f_daga:hierro>plaga,laton>plata"     la daga con la hoja de plaga y la guarda de plata
```

- `personajes.json` → `paletas`: los cinco tonos de cada material (234).
- `personajes/equipo_catalogo.json` → cada objeto: `pieza` (la forma), `tinte`
  (`[[origen, destino], ...]`) y `estetica` (forma, esencia o deidad o tema, acabado, única).
- Motor: `Game::separarTinte`/`juntarTinte`, `PiezaDeObjeto::visible()`,
  `ManifiestoPersonajes::paletas`; `MontajePersonajes` cambia los colores al pintar la capa
  (`CapaDeMontaje::cambios`). `Vestuario::piezasVisibles` y la vista del kit en la creación
  llevan el tinte.
- Probador: `--equipo "arma=f_daga:hierro>plaga"` (lo mismo que el motor).
- Los acabados de telas, cueros y maderas se hacen del material de base cuando un objeto los pide
  (`actores_materiales.acabado`): Insigne, más vivo y con la luz plateada; Sublime, más hondo y
  con la luz dorada. Los metales llanos pasan a plata u oro; los temáticos se mezclan con ellos.

## 4. Las paletas nuevas

Las 20 esencias (`roble_antiguo`, `hierro_santo`, `luna`, `niebla`, `penumbra`, `plaga`,
`trueno`, `tumba`, `estrella`, `juramento`, `rey_lobo`, `sangre`, `sombra_roja`, `corona_rota`,
`eternidad`, `vacio`; ceniza, cristal azul, oro viejo y plata ya estaban) y 37 de metales,
cueros y telas de los temas (`oro_rojo`, `hierro_forja`, `acero_tormenta`, `bronce_marino`,
`vidrio`, `plata_negra`, `malla_plata`, `malla_oro`, `cuero_rojo`, `cuero_vino`...).

## 5. Para revisarlo

`python3 tools/revision_estetica.py` deja en `out/revision_estetica/` las láminas: las 20
esencias en sus cinco formas, las 20 deidades (cabeza, torso, pies, la de la mano y la mítica),
las series con su Insigne y su Sublime, y el resto de armas y ropa por forma. Las armas salen
también solas y ampliadas (en el juego son pequeñas).

## 6. Pruebas

- `test_personajes_capas`: dos nuevas, `estetica_el_tinte_cambia_solo_los_tonos_del_material`
  (la daga de plaga montada con y sin tinte: solo cambian los pixeles de los tonos de origen, y al
  tono de destino que toca; una paleta que no existe es un error) y
  `estetica_todos_los_objetos_se_montan` (los 401 en un humano).
- `test_editor_apariencia`: la espada corta del caballero templario ya es `f_espada_corta`.
- `test_kit_basico` sigue saliendo exacto en las 63 clases. `validar_sprites`, `validar_pnj` en
  verde. ctest: las mismas 9 de antes. La guardia: las mismas 44 deudas.
- `test_validar_sprites`: falla `test_personajes_son_reproducibles` igual que antes de esto (el
  generador sin el paso de PixelLab no deja las filas de PixelLab en `personajes.json`).

## 7. Lo siguiente (Oriol, misma tarde)

> «Me gustaría que estos fueran cosas de las más básicas a nivel estético, me gustaría que
> tomáramos como referencia las armas y partículas de NosTale.»
> «Nada de implementar el sistema de mejoras de NosTale, he dicho estética en vez de mejoras;
> podemos igualar los brillos en relación a lo fuerte que sea el arma: de +1 a +2 Tier 1, +3 a
> +4 Tier 2, y así consecutivamente. Luego la epicidad y nivel de detalle en las armas [...]
> una escalera de progresión de lo que debería ser el arma o armadura más básica a la más
> fuerte.» Dibujo grande y trabajado: «desde raras».

Así que esto es la base, lo de las comunes y poco comunes. Encima van:

1. **Brillo por fuerza** en armas y armaduras: Tier 1 brilla como un +1/+2 de NosTale, Tier 2
   como un +3/+4, Tier 3 como un +5/+6, Tier 4 como un +7/+8 y Tier 5 como un +9/+10; el color
   y las partículas, de su esencia o su deidad.
2. **Escalera de detalle**: desde las raras, armas y armaduras más grandes y trabajadas, de la
   más básica a la más fuerte; las únicas con dibujo propio.
3. Las **partículas** de los abalorios (anillos, amuletos, reliquias pequeñas).

## 8. El brillo por fuerza (misma noche)

Hecho el punto 1. Oriol vio el prototipo (`brillo_escalera.gif`, `brillo_tipos.gif`,
`brillo_ropa.gif`) y el brillo quedó así:

- **Datos**: cada objeto de `equipo_catalogo.json` lleva `brillo` (nivel, color, partículas).
  El nivel: Tier × 2 − 1, y uno más si es la pieza fuerte de su tier (pide más nivel que la de
  entrada en `niveles_equipo`), una reliquia o una única. De los 401 que se ven: 83 a +1, 61 a +2,
  18 a +3, 64 a +4, 43 a +5, 97 a +6, 15 a +7 y 20 a +8 (no hay Tier 5 todavía).
- **Motor** (`Game/Personajes/BrilloEquipo`, sin OpenGL): la escalera de cada nivel, el halo
  ya pintado (alrededor del arma; en la ropa solo por fuera del cuerpo), los destellos, el
  barrido de la ropa y las partículas, deterministas por instante. `MontajePersonajes::montar`
  dice qué capa queda arriba en cada pixel (`MapaDeCapas`): el brillo sale solo de lo que se ve.
  `Vestuario::brillosVisibles`, `HojaPersonaje` (el halo en otra textura) y `AuraJugador`
  (lo pinta encima del jugador; `Entity::dibujoDePersonaje` le da el rectángulo y el frame).
  `PantallaNuevaPartida` lo monta al vestir al jugador y `ApplicationHud` se lo pasa al aura.
- **Pruebas**: `test_brillo_equipo` (5: la escalera, el nivel de cada objeto, lo que se ve, el
  halo del arma y el de la ropa, destellos deterministas). `demo_brillo_equipo` en el motor de
  verdad: seis humanos de +1 a +8 y uno con un juego de reliquias.

## 9. La escalera de detalle (misma noche)

Hecho el punto 2 para las armas. Oriol, «desde raras»: lo común y lo poco común se quedan con
la forma básica; desde las raras, cada objeto sube un escalón de su forma
(`actores_formas.ESCALERAS`): **raro** (más larga y ancha, guarda con las puntas vueltas, pomo
con remate), **épico** (gema en el pomo y la guarda, runas en la hoja) y **mítico** (corona de
gema, guarda con alas, la más grande). La gema es un cuarto material de las armas (`f g h i j`
en las rejillas; `actores_armas.TONO_4`), y la esencia la tiñe.

- 50 escalones: los tres de las cinco formas de esencia y 35 del resto de formas que llevan
  objetos raros o épicos (espada, estoque, mandoble, hacha, martillo, maza, lanza, tridente,
  guadaña, arco, ballesta, vara, cetro, bastón, tea, farol, espejo, escudo, rodela, broquel,
  pavés, orbe, grimorio, símbolo y talismán). Los perfiles salen de la vista de frente
  inclinada (`perfil()`), en espejo o con el canto de la básica.
- 128 objetos suben de escalón (53 raros, 55 épicos y 20 míticos); 400 aspectos distintos de
  401. Todos caben en las cuatro tallas sin tocar el borde de la celda (no era así: `pintar_arma`
  recortaba sin avisar lo que se salía; corregido en el punto 12).
- La lámina `escalera_armas_esencia.png` enseña las cinco formas de esencia en sus cuatro
  escalones.

Falta: los dibujos propios de las 20 míticas de las deidades y de las 19 armas con nombre, las
reliquias pequeñas de la mano, la escalera de la ropa y las partículas de los abalorios.

## 10. Las partículas de los abalorios

Hecho el punto 3. Los 12 objetos que no se ven puestos (los anillos, los amuletos, las lentes,
el monóculo, el tercer ojo y las reliquias pequeñas de la mano) llevan su `brillo` en
`equipo_catalogo.json` → `sinPieza`, por lo que dan o por su lore (Oriol: «si su lore es de algo
malo, partículas oscuras»): el anillo de vitalidad echa luz roja; el amuleto de foco, estrellas;
la argolla del Toro, chispas; la caracola de las Mareas, agua; el cuerno de la Corte Salvaje,
hojas; las lentes del Cuervo, motas de tinta; el monóculo y el sello de la Serpiente, veneno; el
tercer ojo del Oráculo, rayos. En el motor van como `abalorio:<id>` (`Vestuario::
brillosVisibles`), y `montarBrillo` los echa desde el cuerpo entero, sin halo y con partículas
desde el Tier 1 (`escalonDeAbalorio`). Prueba: `brillo_los_abalorios_brillan_en_el_cuerpo`.

## 11. Las únicas: las veinte míticas de las deidades (7/10, madrugada)

Hecho el dibujo propio de las veinte míticas de deidad (`actores_formas.UNICAS`, piezas
`f_u_<id>`): cada una con su idea y sus materiales, sin tinte, y el brillo de su deidad. La Aguja
del Firmamento (pomo de estrella, el hilo rojo), el Alfiler del Velo (perla violeta y el velo que
cuelga), el Arco de la Cacería Interminable (cuerna con hojas), el Astil del Rayo Escrito (punta
en rayo y esquirlas de vidrio), el Bastón del Camino Compartido (calabaza y hatillo), el Bordón
del Cruce (las tablillas del cruce de caminos), el Colmillo del Último Invierno, la Espada de la
Brasa Inextinguible (hoja en llama), la Espada del Primer Rayo (guarda de sol naciente), el
Estilete del Firmante (plumín y serpiente, con la gota de tinta en la punta), la Guadaña de la
Buena Siega (espigas atadas), el Hacha del Cuerno Roto, la Hoja del Año Nuevo (doce muescas), la
Hoz del Cuarto Menguante (una luna), el Mandoble del Estampido, el Martillo de la Primera Chispa
(la cabeza al rojo), la Pala del Último Servicio (con su farolillo), la Pluma-Espada del Índice
(una pluma de cuervo), el Tridente de la Marea Alta (perla) y la Vara del Farol Encendido.

- Las de asta son más altas que quien las lleva: la cabeza del arma queda por encima de la suya
  y no le tapa la cara (las lanzas y bastones de siempre sí).
- Paleta nueva: `tinta_negra` (negro con lustre azul: la pluma y la tinta).
- 20 filas nuevas al final de `arma_<talla>.png` (99 filas); ninguna se mueve.
- `demo_brillo_equipo` acepta ahora hasta siete objetos (`demo_brillo_equipo 96 id id ...`):
  `unicas_en_el_motor.gif` son las veinte en el motor, con su brillo.

Las 19 armas con nombre (raras y épicas) siguen con su escalón y su tema; su dibujo, después.

## 12. Que nada se salga de la celda

Al poner las únicas en la mano salió que `pintar_arma` recortaba sin avisar lo que se salía de la
celda de 64: 55 piezas perdían la punta en alguna talla (el sable mítico, 6 filas en los humanos;
hasta la espada de hierro de siempre, en enanos y medianos). Ahora `actores_armas.forma_en_talla`
acorta cada vista lo justo para su talla, con todos los fotogramas a la vez: quita tramos
repetidos de la hoja o del astil (filas iguales, o corridas una columna en las de perfil) de los
tramos más largos, y si no hay, los de la misma silueta (alguna runa) y al final la fila de en
medio de lo que cuelga (la hoja en llama). La punta, la guarda y el pomo se quedan. Lo que ya
cabía sale igual pixel a pixel: cambian las filas de esas 55 piezas en sus tallas y tres PNJ de
`pnj_mundo_1.png` (sus armas, ahora con punta).

Lo que queda para Oriol: con la celda de 64, una hoja que cuelga del puño no puede ser más larga
de unos 14 píxeles en los humanos (10 en los medianos), así que de frente las espadas de la
escalera se distinguen por el ancho, la guarda, la gema y el brillo, no por el largo. Si se
quiere que las grandes se vean grandes también de frente, hay que llevarlas de otra manera (al
hombro o en alto, como las de asta) y eso es una decisión de estilo.

Pruebas (en test_validar_sprites, con el mismo fallo de antes en la reproducibilidad):
`test_ningun_arma_se_sale_de_la_celda` y `test_acortar_quita_tramos_de_la_hoja_y_deja_la_punta`.
ctest con las mismas 9; la guardia con las mismas 44.


## 13. Cómo se coge cada arma: los vídeos de NosTale (7/10)

Oriol: «les falta un montón para parecerse a lo que te he pedido [...] las armas en la mano del
personaje no se ven nada épicas [...] hay que hacer un poco más grande todas las armas, porque son
demasiado finas y se pierden», «animando las armas y dándoles un poco más de juego» y, sobre la
primera prueba (todo al hombro, la lanza recta): «las dagas se cogen y la hoja apunta hacia atrás
como el assassin de nostale; el espadón, si va hacia atrás que apunte abajo, y si va adelante que
apunte arriba o abajo en reposo; la cuerda del arco va hacia atrás, no hacia delante [...] la
lanza o cualquier arma de palo no la puedes llevar como un bastón en combate [...] recta así me
parece horrible».

Repasados sus vídeos (Espadachín SP1, Cruzado, Berserker, Monje de batalla, Segador, Asesino,
Ángel vengador, Cazador de demonios, Cañonero, Destructor, Explorador, Cazasoles, Guardián salvaje
y los del artista marcial). Lo que se ve, en la lámina `como_se_coge_cada_arma.png`:

- Las armas son enormes y salen de la silueta. Quieto, el cuerpo casi no se mueve: la vida la ponen
  el brillo, las partículas, las alas, la capa y la mascota.
- La espada descansa baja, en diagonal hacia fuera, con la punta en el suelo; corriendo va detrás,
  casi horizontal. La katana, abajo y hacia atrás. El espadón, vertical delante.
- Las dagas del Asesino, con agarre invertido: la hoja sale del puño hacia atrás.
- La lanza del Monje de batalla, tendida a la cadera con la punta hacia fuera (de espaldas, en
  diagonal). La guadaña del Segador, cruzada: la hoja abajo junto al pie y el astil tras el hombro.
- La ballesta y el lanzacohetes, horizontales a la cintura; el cañón, al hombro; los revólveres,
  abajo junto a la pierna.
- El artista marcial lleva guanteletes enormes en las dos manos (no tenemos esa familia).

## 14. Armas al doble y el reposo animado (7/10)

Hecho, en el generador y en el motor:

- **Al doble** (`actores_poses.doblar`): cada forma en la mano es el doble de grande, con el
  contorno de un píxel; el puño sigue en un solo píxel.
- **Cogida como su tipo, quieta y andando** (`actores_poses.GRUPOS`): la tabla está en
  `assets/textures/personajes.md` («Armas al doble, cogidas como en NosTale»). El espadón quieto va
  delante con la punta arriba: con la punta en el suelo no cabe (el puño va a la cadera y quedan
  unos 16 píxeles hasta los pies). Las hachas de una mano, mazas y macanas, que al hombro se veían
  raras, cuelgan junto a la pierna con la cabeza hacia el suelo.
- **El reposo**: 8 frames más en cada capa (13-20, dos por rumbo: quieto y respirando, con el arma
  como se lleva parado). Los atlas de capas pasan a 20 columnas (1280 px); los horneados siguen con
  12. `MontajePersonajes::kFrames` = 20, `framesDeReposo`, `cicloDeReposo`;
  `HojaPersonaje::animarReposo`; `Player` pasa al reposo de su rumbo a los 0,2 s de pararse. La
  vista previa de la creación anda, descansa y gira.
- **Que quepa**: `formas_en_talla` acorta la maestra al doble por cada lado del puño (lo mismo en
  todos los pasos de un estado) y, si lo que se sale es el ancho de la cabeza, la endereza un poco.
  `_desborde` mira solo la caja de lo pintado y `_acortar_abajo` quita varios bloques de un tramo de
  una vez: todo el generador sigue tardando lo mismo (~50 s).
- `importar_prendas.py` (PixelLab) escribe ya las filas de 20 frames: antes recortaba los atlas de
  torso y cabeza a 12 columnas.
- `demo_brillo_equipo` con `DEMO_BRILLO_PASEO=1`: las figuras andan sin moverse y descansan, girando.
  `motor_espadas_dagas_lanza.gif`, `motor_arco_baston_guadana_ballesta.gif` y
  `motor_martillo_hachas_maza.gif` son eso, en el motor y con el brillo.

Pruebas: `test_cada_arma_se_coge_como_en_nostale`, `test_el_arma_no_cambia_de_largo_al_andar` y
`test_veinte_frames_doce_de_andar_y_ocho_de_reposo` (test_validar_sprites, 38/39 con el fallo de
siempre); `apariencia_reparto_de_la_hoja` con el reposo; `player_pasa_al_reposo_de_su_rumbo_al_quedarse_parado`
y `player_sin_reposo_se_queda_en_el_quieto_de_andar`. Ningún arma se sale en ninguna talla ni en
ningún frame. ctest con las mismas 9; la guardia con las mismas 44. El presupuesto de la caché de
atlas del montaje pasa de 96 a 160 MB (los atlas son 20/12 más anchos).

En el Mac los recursos se regeneran allí (`python3 tools/gen_personajes.py`, 14 s): lo que hay ahora
es exactamente lo que el generador saca de sus datos, así que solo cambia lo que cambia el código.

## 15. Que ningún arma tape la cara (7/10)

Oriol, viendo los GIF: «en las animaciones hay que evitar siempre tapar la cara del jugador, a no
ser que sea la estética que queramos darle a alguna clase». Ahora es una regla del generador: donde
el arma va encima del cuerpo (de frente y de perfil), ningún píxel cae en la cara (la cabeza sin la
coronilla; de perfil, su mitad de delante). Si la pose la tapa, se prueba más inclinada hacia fuera;
lo que sube por encima del puño de un arma que cuelga se acorta; y si nada basta, el arma se lleva
colgada. Andando de perfil, bastones y cetros llevan la cabeza hacia atrás. Lo que la tapaba: el
espadón quieto de perfil, la cabeza de la gran hacha y del martillo de frente, el arco y el cetro
en las tallas pequeñas, y el pomo de algunas espadas al andar. `test_ningun_arma_tapa_la_cara`
(todas las armas, tallas y frames); `sin_tapar_la_cara.gif` en el motor.

## 16. Las correcciones de Oriol a los agarres y las armas con volumen (7/10)

Oriol, viendo los GIF: «no me terminan de gustar, cuesta entenderlas un poco». Lo que pidió y lo que
quedó:

- **Bastón**: «más recto cuando ando» y que fallaba la inclinación de frente y de espaldas (cruzaba
  el cuerpo en diagonal). Lo enderecé del todo y se pasó («me gusta más inclinado [...] te has
  pasado»): queda algo más recto que antes. Le gustó que el personaje tape el arma, «pero solo en
  los frames frontales si fuera necesario y solo el trozo justo»: de frente, si el bastón pasaría
  por la cara, la cabeza lo tapa (`TAPA_LA_CABEZA`); si no, nada. «Arco y bastones súper bien.»
- **Arco**: sin cuerda (`SIN_CUERDA`).
- **Daga**: «en la frontal pasa por delante de la mano la empuñadura». Probé tres puños (A como
  estaba, B la guarda pegada al puño, C el antebrazo tapando el mango): eligió A. Se queda igual.
- **Gran hacha y martillo**: «con las dos manos hacia el hombro y para correr la dejaría caer hacia
  atrás como un espadón». El primer intento (cogidos por el final, la cabeza junto a la del
  personaje, andando tirados en el suelo) lo empeoraba todo, y lo deshice. Luego: «le falta
  perspectivas [...] se entienden fatal» y «si aumentas el número de sprites por lado de cada arma
  todo quedaría súper mejor». De ahí `tools/actores_volumen.py`: la maestra extruida en vóxeles con
  el grosor de cada material y girada en 3D en cada vista y paso (detalle en
  `assets/textures/personajes.md`, «Armas con volumen»). Quietos van cruzados por delante del
  cuerpo hacia el hombro contrario, con el mango largo para que la cabeza pase del hombro; de
  perfil, al hombro con la cabeza detrás; andando de perfil, caídos atrás arrastrando la cabeza.
- **Hachas de una mano**: la hoja en forma de D se leía como una piedra. Maestra al doble hecha
  con polígonos (`MAESTRAS_2X`): hoja de barba, cuello estrecho, ojo de metal y el astil asomando;
  virolas, gema y runas según el escalón; el Hacha del Cuerno Roto con hoja de cuerno. Con volumen y
  la cabeza arriba, junto al hombro y hacia fuera.
- **Látigo**: dos opciones (enrollado o colgando); eligió la tralla colgando hasta el suelo, que de
  perfil se arrastra detrás.
- **Ballesta**: «no sabe ni cómo la está cogiendo». Ni la de antes ni la nueva (cruzada abajo de un
  puño al otro) le convencen. Para cogerla de verdad con las dos manos hace falta otra postura de
  brazos en el cuerpo (los dos puños juntos delante): pendiente, junto con un ciclo de correr si se
  quiere distinguir de andar.

Pruebas: `test_cada_arma_se_coge_como_en_nostale` con las poses nuevas, `test_el_arco_no_lleva_cuerda`
y `test_ningun_arma_tapa_la_cara`, que para lo que tapa la cabeza mira la capa ya pintada
(`actores_armas.pixeles_en_la_cara`). test_validar_sprites 40/41 (el fallo de siempre); ctest con
las mismas 9 (con `pnj_boundington` regenerado; con el del repositorio, `test_personajes_capas` no
casa con las capas nuevas, como cada vez que cambia un arma de un PNJ de Boundington).
