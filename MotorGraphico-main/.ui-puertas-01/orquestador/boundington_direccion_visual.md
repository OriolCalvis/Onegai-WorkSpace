# Boundington — dirección visual: el efecto que tiene que dar (Fable, 19/10/2026)

Para quien pinte, genere con PixelLab, coloque objetos o ajuste la luz. Parte de lo que ya manda: la paleta maestra (`assets/textures/pixellab/estil/paleta_mestra.json`, 20 rampas de oscuro a claro, sombra `#1a1c26` y luz `#f4efe2`), la isométrica a 64×32 por casilla, la luz ambiente del `IsometricRenderer` (`ambientColor`) y el fuego con luz y humo de AV17-11. Nada de esto es nuevo código: es criterio.

## 1. La idea en una frase

**Una ciudad de piedra vieja y madera, viva y algo gastada, bajo una luz que cambia tres veces al día y que la tercera noche se convierte en fuego.** Boundington no es ni la ciudad medieval limpia de postal ni la ciudad gótica oscura: es un sitio donde la gente vive, tiende la ropa y discute en la plaza, y donde el jugador tiene que notar, al tercer día, que todo eso se puede perder.

Tres sensaciones, por orden: **habitada** (siempre hay alguien haciendo algo), **legible** (desde lejos se sabe qué es cada edificio y por dónde se pasa) y **frágil** (madera, tela, paja, fuego: lo que arde).

## 2. Materiales y color

La ciudad se lee por materiales, no por colores sueltos. Cada material tiene su rampa y no se mezcla con otra:

| Material | Rampa | Dónde |
|---|---|---|
| Piedra clara, gastada | `pedra` | fachadas del Casco, iglesia, muralla por dentro, fuentes, bancos |
| Piedra oscura, húmeda | `socol` | zócalos, muralla por fuera, muelle, cripta, montaña, cueva |
| Madera vieja (nogal) | `noguera` | vigas, puertas, carros, barcas, tabernas por dentro |
| Madera cruda | `fusta_crua` | paradas de mercado, empalizadas, tendederos, barracas de la aldea de Gorran |
| Teja y ladrillo | `terracota` | tejados del Casco y del Comercial; es el color que domina la ciudad vista desde arriba |
| Cobre viejo | `verdet` | cúpulas de la ópera y la universidad, canalones, campanas |
| Latón y oro | `llauto` / `accent_or` | solo donde hay dinero: joyería, banco, Barrio Alto, la Furia de Alaro |
| Hierba, árboles, pino | `fulla` / `pi` | Afueras, prado del río, Campos del Otoño |
| Agua | `aigua` | el río, los charcos del muelle y de la cueva |
| Ropa de la gente | `roba_vermella`, `roba_blava`, `roba_porpra`, `pell` | PNJ; la ropa es lo que pone color en la calle |
| Acentos | `accent_vermell`, `accent_blau`, `accent_verd` | banderas, carteles, lo que debe verse a un vistazo |

Reglas:
- **El gris vive en la piedra, no en el aire.** El fondo del mapa (`fondos` de las cinemáticas, el `ambientColor`) nunca es gris neutro: es azul noche o ámbar, según la hora.
- **Los acentos son escasos.** Una bandera roja por plaza, un cartel por local. Si todo tiene acento, nada lo tiene.
- **Nada de negro puro ni blanco puro** en los sprites: la sombra es `#1a1c26` (azul casi negro) y la luz `#f4efe2` (hueso). Es lo que da el aire de pintura y no de cómic.
- **Cada barrio tiene su material dominante** (§4); se pasa de uno a otro al cruzar la muralla o el río y se nota.

## 3. La luz: tres franjas y una noche distinta

El día tiene tres franjas (mañana, tarde, noche) y la luz es lo que las hace sentir sin un reloj en pantalla:

| Franja | Ambiente (`ambientColor`) | Sombras | Fuentes de luz | Sensación |
|---|---|---|---|---|
| Mañana | neutra tirando a fría, alta: ≈ `{0.95, 0.97, 1.0}` | cortas, hacia el noroeste de la casilla | ninguna encendida | limpia, de mercado: es cuando la ciudad trabaja |
| Tarde | ámbar, un poco más baja: ≈ `{1.0, 0.92, 0.80}` | largas | las farolas aún apagadas; el brasero del herrero | la luz «amarilla que se tumba sobre los tejados» de los textos del día 3 |
| Noche | azul oscuro, baja: ≈ `{0.35, 0.38, 0.52}` | sin sombra proyectada | farolas, ventanas de las tabernas y la posada, hogueras de la Barriada, la fiesta del Barrio Alto | íntima; lo que importa está iluminado y lo demás se adivina |
| **Ocaso (día 3, noche)** | rojo y morado, muy baja: ≈ `{0.45, 0.22, 0.30}` | parpadeantes, por el fuego | el fuego en toda calle que arde; sin farolas; el cielo naranja en el horizonte (`fondos`) | hostil, lo contrario de la noche normal: todo está iluminado y nada es seguro |

La noche normal **y** el Ocaso tienen que verse como dos cosas distintas aunque ocurran a la misma hora: la primera es azul y cálida en puntos; la segunda es naranja por todas partes y azul en nada. Ese contraste es el efecto más importante del juego y vale más que cualquier detalle.

## 4. Barrio a barrio (lo que se ve y lo que se siente)

**Casco Antiguo (plaza de la fuente seca y la estatua sin cara).** Piedra clara `pedra`, suelo de mármol gastado en la plaza (`marmol_real`), tejas. La plaza se ve vacía de objetos a propósito: lo que la llena es la gente (vendedores, el niño del gato, los guardianes) y el estrado del tercer día. La iglesia manda: es el edificio más alto y lo que el jugador busca cuando busca algo. Sensación: el corazón; aquí pasan las cosas públicas.

**Comercial (filas 14–20).** Madera cruda y toldos: paradas, jardineras, la fuente del viento con el sitio de actuar. Más color de ropa que en ningún sitio. Es ruidoso a la vista: carteles, paradas en la fila 16, gente parada. Sensación: dinero pequeño, movimiento.

**Norte monumental (castillo, ópera, universidad, biblioteca).** Cuatro bloques grandes con cobre `verdet` y piedra oscura `socol`; la calle principal de tres es un pasillo. Aquí los acentos son banderas y poco más; los bancos y las farolas van a los lados de las puertas, nunca delante. Sensación: poder, orden, un poco frío. Es donde el jugador se siente pequeño.

**Muelles y puente.** Piedra húmeda, bolardos, barcas `noguera`, juncos, agua `aigua` con el muro del muelle. Es el único barrio con horizonte abierto. Sensación: salida, respiro; por aquí entra lo de fuera (el camino a Egaroth).

**Oeste: Base Militar, Pico Dragón y la Barriada.** Dos mundos pegados: el cuartel (piedra oscura, estandartes, armero) y la Barriada de Surysal (tendederos, hogueras, empalizadas, barcas en la orilla). Es el barrio más vivo de noche: hogueras encendidas, gente fuera. Sensación: la ciudad que no sale en los mapas oficiales; es donde Xila y Zarela están en casa.

**Este y Barrio Alto.** Casas nobles (`paret_noble_*`, latón, cristal), la mansión de la fiesta con luz cálida por las ventanas y música. De noche es el único sitio con farolas en todas las esquinas. Sensación: lo que el jugador no tiene; la fiesta se ve antes de llegar.

**Surysal.** Entre la ciudad y el campo: piedra y hierba, el puente, la puerta a las Afueras. Sensación: umbral.

**Afueras.** Bancales de huerto, cuatro robles, el establo, el sendero a la montaña. Ningún acento. Sensación: campo pobre y tranquilo, lo que hace que verlo arder el tercer día duela.

**Montaña y aldea de Gorran.** Roca `socol`, hierba seca, antorchas; la cornisa con el vacío al lado. La aldea es «lletja, densament habitada, cutre però funcional» (Oriol): barro, losa oscura, lonas, humo, luz de hoguera y de brasero, nada de piedra clara. Lo que la hace real es la gente haciendo cosas: beber, jugar, tender, dormir, vigilar. Sensación: refugio y antro a la vez.

## 5. Lo que separa un mapa correcto de uno con alma

1. **Gente antes que objetos.** Un banco con una anciana vale por diez bancos. Cada plaza tiene al menos tres PNJ con rutina (ronda, parada, charla) y una mascota o un pájaro.
2. **Rastro de uso.** Basura junto a la taberna, botellas en el muelle, huellas de carro en la tierra, ropa tendida entre dos casas, un carro volcado. Siempre fuera de los carriles.
3. **Umbrales visibles.** Cada puerta tiene su losa de umbral (`adoquin_gastado`) y, si es un local, su cartel al lado, no delante. Se entiende a dónde se entra sin leer.
4. **Verticalidad donde la hay.** Alturas solo en lo natural (montaña, cueva, galerías), nunca en la ciudad: la ciudad se lee plana y el campo se lee subiendo.
5. **Luz con intención.** Una farola o una hoguera señalan algo: una puerta, un cruce, un PNJ. Nunca se ponen para rellenar.
6. **Animación corta y repetida.** Un pájaro, el humo de una chimenea, una bandera, el agua: tres o cuatro cosas que se mueven en pantalla a la vez y ninguna que grite.

## 6. El Ocaso: cómo tiene que doler

- **Mismo mapa, otra ciudad.** Que el jugador reconozca la plaza donde compró pan y la vea con el estrado ardiendo. Nada se mueve de sitio; cambian la luz, el fuego y quién está en la calle.
- **Las rutas que no convienen arden.** El fuego cierra calles enteras (a una de cada casilla, no salpicado): se ve desde lejos por dónde se puede ir. Las barricadas de carros son el otro cierre, oscuras contra el naranja.
- **Los Perdidos se ven antes de tocarlos.** Patrullas rectas, antorchas, dos o tres juntos. Encontrarse uno es malo; verlo venir es el juego.
- **La gente ha desaparecido o está tirada** (orden `tumba`): ni vendedores ni niños; huesos, runas, humo. Las Afueras al ocaso, igual: los bancales pisoteados y el establo abierto.
- **La huida es oscura.** Los Campos del Otoño no arden: azul noche, hierba seca, el camino de tierra y, detrás, «Boundington entera como una línea naranja contra el cielo». Es el único momento del día 3 con la luz de una noche normal, y por eso alivia.

## 7. Qué pedir a PixelLab con esto

- Cada pieza nueva se pide con su material de la tabla del §2 y pasa por `aplicar_paleta.py` con esa rampa (`--rampes`): una barraca con `fusta_crua,socol`, una tela colgada con `roba_porpra,roba_vermella`, un perro con `noguera,pell`.
- Las piezas de la aldea de Gorran llevan suciedad y hollín (sombra `#1a1c26` en los bordes de abajo); las del Barrio Alto, luz `#f4efe2` en los bordes de arriba.
- Nada se pide «medieval genérico»: se pide «de Boundington», con el material, la rampa y en qué barrio va.
