# RUTINAS_PNJ — qué hace cada PNJ (carril `pnj_datos`, ronda del 15/10)

Sesión: [2026-10-15-claude-pnj-datos](../sesiones/2026-10-15-claude-pnj-datos.md) · Paquete: `MotorGraphico/out/claude_pnj_datos01/` ·
Contrato: [ronda_15-10.md](../orquestador/ronda_15-10.md).

Oriol pidió (14/10): «que los NPCs se movieran un poco y tuvieran algunas animaciones más y que cada uno hiciera alguna cosa; se pueden repetir algunas animaciones».
Este informe es la parte de **datos**: a cada tipo de PNJ, una rutina con sentido. El motor que la ejecuta es el carril `pnj_motor`.

## Resumen

- **108 tipos de PNJ colocados y 218 PNJ** en los 116 niveles de `assets/levels/` (los de categoría `npc` del catálogo de objetos, `npc_race_*` incluidos). Por zona: Boundington 79 · Este-Norte 48 · Boundington (int.) 29 · Recuerdos/prólogo 27 · Plaza razas 12 · Origen Norte 11 · Surysal 10 · Ejemplos 2.
- El catálogo `assets/catalogs/rutinas_pnj.json` da **clave exacta a los 108 tipos colocados** (ninguno cae en `porDefecto`) y 5 claves más para lo que aún no está colocado (familias `npc_race_*` y `npc_npc_*`, `ciervo`, `herrero`, `tabernero`).
- Reparto: 65 tipos con `actividad`, 21 `quieto`, 10 `vigilar`, 8 `deambular`, 2 `charlar`, 2 `ir_y_volver`.
- Los PNJ con papel en una misión (hay 60 ids con un beat `talk` en aventuras o cinemáticas) van `quieto`, `vigilar` o con **radio 0-1**, para que el jugador los encuentre donde espera. Los que se mueven más lejos no tienen misión: niños (radio 4), estudiantes, archivero y fletero (`ir_y_volver`), mujer lagarto del pantano y el relleno de `npc_race_*` / `npc_npc_*`.
- Solo se usan fotogramas que ya existen: `vaiven`, `golpe`, `giro`, `andar_sitio` y `respirar` (se repiten entre tipos, como pidió Oriol).
- Iconos de bocadillo (todos en `iconos.json`): `oro` (pregonan y contables), `inspiracion` (bardo), `lore` (bibliotecarios, archiveros, cronista), `curiosidad` (alquimista, astrónomos). El resto, sin bocadillo para no llenar la pantalla.
- Frases: 0-3 por tipo, ≤ 40 caracteres, solo en tipos de calle (pregonan, guardias, niños, ancianas…). Sin frases en los personajes de historia ni nada que adelante misiones.

## Tabla por tipo de PNJ

`radio` en casillas. «×n» = cuántos hay en ese nivel. Orden: más repetidos primero. Abreviaturas de nivel: `centro`, `este`, `oeste`, `surysal`, `int:taberna`…

| Tipo (`objectId`) | Nombre | Cuántos | Dónde | Rutina elegida | Por qué |
|---|---|---|---|---|---|
| `anciana` | Anciana | 12 | centro×6, oeste×4, surysal×2 | deambular, radio 1 | Pasea despacio y poco; es la vieja a la que hablan dos aventuras, no se aleja de su banco. |
| `npc_race_medianos_urbanos` | Mercader mediano urbano | 11 | centro×5, oeste×3, surysal×2, plaza_de_las_razas | actividad «pregonar» (vaiven, icono oro), radio 1 | Mercader mediano: pregona junto al puesto; el de la plaza de las razas tiene misión, radio 1. |
| `npc_race_guardianes_de_la_luz` | Guardián de la luz | 10 | centro×6, este×3, oeste | vigilar, radio 0 | Guardianes de la luz: el cuerpo de guardia de la ciudad; vigilan quietos, más solemnes que el guardia raso. |
| `guardia` | Guardia | 9 | oeste×6, centro×2, este | vigilar, radio 0 | Vigila en su puesto girando de cara en cara; es el guardia del silbido, tiene que seguir donde se le espera. |
| `en_gongorguma_taberna_tendero` | Tabernero de Gongorguma | 7 | xiii_taberna, xiv_taberna, xv_taberna, guskedor_taberna y 3 más | actividad «limpiar_vasos» (vaiven), radio 0 | Tabernero de Gongorguma: limpia la barra; radio 0. |
| `npc_race_alquimistas_de_aegroum` | Alquimista de Aegroum | 7 | centro×4, oeste×2, plaza_de_las_razas | actividad «buscar_ingredientes» (vaiven, icono curiosidad), radio 1 | Alquimista: busca ingredientes con vaivén y una pregunta sobre la cabeza; radio 1 por su papel en la plaza. |
| `en_mistarium_taberna_tendero` | Tabernero de Mistarium | 6 | m_i_taberna, m_taberna, pueblo_m_i_taberna, pueblo_m_ii_taberna y 2 más | actividad «limpiar_vasos» (vaiven), radio 0 | Tabernero de Mistarium: limpia la barra; radio 0. |
| `en_gongorguma_heraldo` | Heraldo de clanes | 5 | xiii, xiv, xv, guskedor y 1 más | actividad «pregonar» (vaiven), radio 1 | Heraldo de clanes: pregona en la plaza; 5 ciudades distintas, radio 1. |
| `en_gongorguma_mercado_tendero` | Mercader de Gongorguma | 5 | xiii_mercado, xiv_mercado, xv_mercado, guskedor_mercado y 1 más | actividad «ordenar_cesta» (vaiven), radio 0 | Mercader de Gongorguma: ordena su mostrador; radio 0. |
| `npc_race_orcos_del_consejo` | Orca del consejo | 5 | oeste×3, surysal, plaza_de_las_razas | actividad «deliberar» (vaiven), radio 1 | Orcos del consejo: deliberan en corro; es de misión en la plaza, así que no charlan lejos. |
| `sho_noco` | Sho-Noco | 5 | prado_del_rio, recuerdo_aegroum, recuerdo_ceaseton, recuerdo_puerta y 1 más | quieto (fijo:abajo), radio 0 | El barquero espera mirando el río: quieto, mirada fija. |
| `anfitriona` | Anfitriona | 4 | este×3, int:casa_te | actividad «servir» (vaiven), radio 0 | Anfitriona de la casa de té: atiende en su sitio con un vaivén amable. |
| `mercader` | Mercader | 4 | centro×3, surysal | actividad «pregonar» (vaiven, icono oro), radio 1 | Pregona junto a su puesto, mirando a la calle; hay un beat suyo en Surysal, por eso radio 1. |
| `nino` | Niño | 4 | centro×2, oeste, surysal | actividad «corretear» (andar_sitio), radio 4 | Corretea: radio grande, pausas cortas y trotecito en el sitio; el único tipo con radio 4. |
| `npc_race_elfos_del_bosque` | Guardabosques elfo | 4 | centro×4 | actividad «otear» (giro), radio 1 | Guardabosques en la ciudad: oteando los cuatro rumbos, como quien sigue rastros entre casas. |
| `parroquiano_humilde` | Parroquiano | 4 | centro×2, oeste, surysal | actividad «murmurar» (vaiven), radio 0 | Parroquiano de calle con diálogo de historia: murmura mirando a un lado y a otro, sin moverse del sitio. |
| `chambelan` | Chambelán | 3 | este×2, int:castillo | deambular, radio 1 | Pasea un poco por el patio con paso digno; no se aleja de la puerta del castillo. |
| `en_mistarium_archivero` | Archivero ambulante | 3 | m, m_i, venordemn | ir_y_volver [3, 0], radio 0 | Archivero ambulante: va y vuelve entre dos puntos con sus papeles; sin misión. |
| `en_mistarium_mercado_tendero` | Mercader de Mistarium | 3 | m_i_mercado, m_mercado, venordemn_mercado | actividad «ordenar_cesta» (vaiven), radio 0 | Mercader de Mistarium: ordena su mostrador; radio 0. |
| `npc_race_enanos_forjadores` | Enana forjadora | 3 | centro×2, plaza_de_las_razas | actividad «forjar» (golpe), radio 0 | Enana forjadora: martillea en su sitio con el golpe rítmico; la de la plaza tiene misión, radio 0. |
| `sargento` | Sargento | 3 | oeste×2, int:cuartel | vigilar, radio 0 | Manda en el cuartel: vigila con giros cortos y pausas breves, impaciente. |
| `armero` | Armero | 2 | oeste, int:armeria | actividad «martillear» (golpe), radio 0 | Arregla armas en su sitio con golpes secos; radio 0. |
| `berto_jornalero` | Berto | 2 | int:taberna, recuerdo_taberna | actividad «brindar» (golpe), radio 0 | Parroquiano de la taberna: brinda; con misión, radio 0. |
| `bordadora` | Bordadora | 2 | int:taberna, recuerdo_taberna | actividad «bordar» (golpe), radio 0 | Borda con puntadas rítmicas; con misión, radio 0. |
| `carretero_jubilado` | Carretero jubilado | 2 | int:taberna, recuerdo_taberna | actividad «brindar» (golpe), radio 0 | Alza la jarra de vez en cuando; con misión, radio 0. |
| `encapuchado` | Encapuchado | 2 | int:taberna, recuerdo_taberna | quieto (fijo:abajo), radio 0 | Quieto y encapuchado, sin mirar a nadie; con misión. |
| `estudiante` | Estudiante | 2 | centro×2 | charlar, radio 2 | Charla con el compañero que tenga cerca; si no hay nadie, deambula. |
| `jornalero_callado` | Jornalero | 2 | int:taberna, recuerdo_taberna | quieto (fijo:abajo), radio 0 | No quiere que le metan en nada: quieto, mirando a la mesa. |
| `marido_bordadora` | Marido de la bordadora | 2 | int:taberna, recuerdo_taberna | actividad «escuchar» (vaiven), radio 0 | Escucha a su mujer, asintiendo; con misión, radio 0. |
| `npc_race_diplomaticos_de_bastrea` | Diplomática de Bastrea | 2 | este×2 | charlar, radio 1 | Diplomáticas: se juntan de dos en dos a conversar, sin alejarse de su barrio. |
| `pescador_ceaseton` | Pescador de Ceaseton | 2 | recuerdo_ceaseton×2 | quieto (fijo:abajo), radio 0 | Recuerdo: quieto con su caña. |
| `skilla` | Rosita, la tabernera | 2 | int:taberna, recuerdo_taberna | actividad «limpiar_vasos» (vaiven), radio 0 | Rosita limpia la barra de la Taberna Humilde; con misión, radio 0 y sin frases. |
| `tomas_jornalero` | Tomás | 2 | int:taberna, recuerdo_taberna | actividad «brindar» (golpe), radio 0 | Parroquiano de la taberna: brinda; con misión, radio 0. |
| `venides` | Venides, hijo de Nefaria | 2 | int:taberna, recuerdo_taberna | actividad «brindar» (golpe), radio 0 | Sentado bebiendo, de vez en cuando alza la jarra; con misión, radio 0. |
| `venides_vuelta` | Venides, 757 | 2 | recuerdo_aegroum, recuerdo_puerta | quieto (fijo:abajo), radio 0 | Recuerdo de 757: quieto, sin girarse hacia el jugador. |
| `aigren` | Aigren, hijo de Ceaseton | 1 | int:herreria | actividad «forjar» (golpe), radio 0 | Herrero de Lingote de Oro: martillea tras el yunque; con misión, radio 0. |
| `banquero` | Banquero | 1 | int:banco | actividad «contar_monedas» (vaiven, icono oro), radio 0 | Cuenta monedas tras la ventanilla, con la moneda sobre la cabeza; radio 0. |
| `bardo` | Bardo | 1 | centro | actividad «tocar» (golpe, icono inspiracion), radio 0 | Toca en su sitio con el golpe rítmico y la nota sobre la cabeza; tiene beat en el silbido, radio 0. |
| `ben_kafka` | Ben Kafka | 1 | int:taberna | actividad «otear» (giro), radio 0 | Busca a los pequeños con la mirada, girando; con misión, radio 0. |
| `bibliotecaria` | Bibliotecaria | 1 | int:biblioteca | actividad «ordenar_libros» (vaiven, icono lore), radio 0 | Ordena libros con el icono de saber; radio 0. |
| `capitan_aegroum` | Capitán de la guardia real | 1 | recuerdo_aegroum | quieto (fijo:abajo), radio 0 | Recuerdo: quieto, firme. |
| `carterista` | Chico del Casco Antiguo | 1 | centro | actividad «impaciente» (andar_sitio), radio 1 | El chico nervioso: no para quieto, hace pasitos y vigila la calle; radio 1 por su misión. |
| `chronos_disfrazado` | Viajero de paso | 1 | plaza_de_las_razas | quieto (jugador), radio 0 | Viajero de paso: quieto y atento, no quiere llamar la atención; de misión. |
| `edul` | Edul | 1 | int:herreria | actividad «forjar» (golpe), radio 0 | Ayudante de la herrería: golpea el yunque; con misión, radio 0. |
| `edul_joven` | Edul, 756 | 1 | recuerdo_ceaseton | quieto (fijo:abajo), radio 0 | Recuerdo de 756: quieto, sin girarse hacia el jugador. |
| `edul_puerta` | Edul, 757 | 1 | recuerdo_puerta | quieto (fijo:abajo), radio 0 | Recuerdo de 757: quieto, sin girarse hacia el jugador. |
| `ejemplos_guia` | Guía de ejemplos | 1 | ejemplos_objetos | quieto (jugador), radio 0 | Guía de pruebas del editor: quieta, mirando al jugador. |
| `ejemplos_tendero` | Tienda de ejemplo | 1 | ejemplos_objetos | actividad «ordenar» (vaiven), radio 0 | Tienda de pruebas: tendero tras el mostrador; radio 0. |
| `en_anciana_consejo` | Anciana del Consejo | 1 | guskedor | quieto (jugador), radio 0 | Anciana del consejo: quieta y solemne; con misión. |
| `en_archivera_espejos` | Archivera de los Espejos | 1 | venordemn | actividad «ordenar_libros» (vaiven, icono lore), radio 0 | Archivera de los Espejos: ordena sus fondos; con misión, radio 0. |
| `en_capitan_ventobelico` | Capitán Ventobelico | 1 | klimnebra | vigilar, radio 0 | Capitán de puerto: vigila el muelle girando; con misión, radio 0. |
| `en_choubar_banco_tendero` | Factor de la Casa de Fletamento | 1 | klimnebra_banco | actividad «contar_monedas» (vaiven, icono oro), radio 0 | Factor de la casa de fletamento: cuenta monedas; radio 0. |
| `en_choubar_fletero` | Fletero del río | 1 | klimnebra | ir_y_volver [-3, 0], radio 0 | Fletero del río: va y vuelve al embarcadero (3 casillas); sin misión. |
| `en_choubar_herreria_tendero` | Herrero de Choubar | 1 | klimnebra_herreria | actividad «forjar» (golpe), radio 0 | Herrero de Choubar: martillea en su yunque; radio 0. |
| `en_choubar_mercado_tendero` | Mercader de Choubar | 1 | klimnebra_mercado | actividad «ordenar_cesta» (vaiven), radio 0 | Mercader de Choubar: ordena su mostrador; radio 0. |
| `en_choubar_taberna_tendero` | Tabernero de Choubar | 1 | klimnebra_taberna | actividad «limpiar_vasos» (vaiven), radio 0 | Tabernero de Choubar: limpia la barra; radio 0. |
| `en_cronista_qethatos` | Cronista de Qethatos | 1 | qethatos | actividad «escribir» (golpe, icono lore), radio 0 | Cronista: escribe sus anales; con misión, radio 0. |
| `en_enviada_neblinia` | Enviada Neblinia | 1 | guskedor | quieto (jugador), radio 0 | Enviada de Neblinia: quieta y atenta; con misión de la señal de ceniza. |
| `en_gongorguma_arena_tendero` | Maestro de la arena de clanes | 1 | guskedor_arena | vigilar, radio 0 | Maestro de la arena de clanes: vigila los combates girando; radio 0. |
| `en_gongorguma_herreria_tendero` | Herrero de Gongorguma | 1 | guskedor_herreria | actividad «forjar» (golpe), radio 0 | Herrero de Gongorguma: martillea en su yunque; radio 0. |
| `en_mistarium_biblioteca_tendero` | Custodio del Archivo de la Bruma | 1 | venordemn_biblioteca | actividad «ordenar_libros» (vaiven, icono lore), radio 0 | Custodio del Archivo de la Bruma: ordena libros; radio 0. |
| `en_mistarium_herreria_tendero` | Herrero de Mistarium | 1 | venordemn_herreria | actividad «forjar» (golpe), radio 0 | Herrero de Mistarium: martillea en su yunque; radio 0. |
| `en_nocturnsea_farera` | Farera | 1 | grytoz | actividad «otear» (giro), radio 0 | Farera: gira despacio oteando el mar; radio 0. |
| `en_nocturnsea_herreria_tendero` | Herrero de Nocturnsea | 1 | grytoz_herreria | actividad «forjar» (golpe), radio 0 | Herrero de Nocturnsea: martillea en su yunque; radio 0. |
| `en_nocturnsea_mercado_tendero` | Mercader de Nocturnsea | 1 | grytoz_mercado | actividad «ordenar_cesta» (vaiven), radio 0 | Mercader de Nocturnsea: ordena su mostrador; radio 0. |
| `en_nocturnsea_posada_tendero` | Posadero del Faro | 1 | grytoz_posada | actividad «ordenar» (vaiven), radio 0 | Posadero del Faro: ordena tras el mostrador; radio 0. |
| `en_nocturnsea_taberna_tendero` | Tabernero de Nocturnsea | 1 | grytoz_taberna | actividad «limpiar_vasos» (vaiven), radio 0 | Tabernero de Nocturnsea: limpia la barra; radio 0. |
| `encargada_banos` | Encargada de los baños | 1 | int:banos | actividad «ordenar_toallas» (vaiven), radio 0 | Ordena toallas en la entrada de los baños; radio 0. |
| `griffin` | Griffin, limpiador de desagües | 1 | int:taberna | actividad «olfatear» (vaiven), radio 0 | Huele el aire buscando sal, a un lado y a otro; con misión, radio 0. |
| `joyero` | Joyero | 1 | int:joyeria | actividad «pulir» (vaiven), radio 0 | Pule una pieza tras el cristal; radio 0. |
| `luisarda` | Luisarda, hija de Boundington | 1 | int:sastreria | actividad «contar_mercancia» (vaiven), radio 0 | Cuenta pociones y víveres en La Boutique de Madame; con misión, radio 0. |
| `maestro_arena` | Maestro de la arena | 1 | int:coliseo | vigilar, radio 0 | Vigila los combates girando de cara en cara; radio 0. |
| `nina_del_gato` | Niña de Pico Dragón | 1 | oeste | quieto (fijo:arriba), radio 0 | Mira hacia arriba, al gato que no baja; quieta y con misión, el jugador la encuentra siempre. |
| `npc_race_aarakocra_de_las_montanas` | Guía aarakocra de las montañas | 1 | plaza_de_las_razas | actividad «otear» (giro), radio 1 | Guía aarakocra: otea el horizonte girando; en la plaza es de misión, radio 1. |
| `npc_race_drow_explorador` | Exploradora drow | 1 | plaza_de_las_razas | deambular, radio 1 | Exploradora drow: recorre su rincón con cautela, mirando la calle; de misión en la plaza. |
| `npc_race_elfos_de_las_estrellas` | Astrónomo elfo de las estrellas | 1 | este | actividad «mirar_estrellas» (giro, icono curiosidad), radio 0 | Astrónomo: gira despacio mirando el cielo, con la curiosidad sobre la cabeza. |
| `npc_race_hombres_lagarto_de_la_selva` | Hombre lagarto de la selva | 1 | plaza_de_las_razas | deambular, radio 1 | Hombre lagarto de la selva: paseo corto en la plaza; de misión, radio 1. |
| `npc_race_hombres_lagarto_del_pantano` | Mujer lagarto del pantano | 1 | surysal | deambular, radio 2 | Mujer lagarto de Surysal: deambula tranquila alrededor del campamento del vado. |
| `npc_race_minotauros_del_laberinto` | Minotaura del laberinto | 1 | plaza_de_las_razas | vigilar, radio 0 | Minotaura: planta en su sitio y vigila girando despacio; de misión en la plaza. |
| `npc_race_nagas_arcanos` | Naga arcano | 1 | plaza_de_las_razas | actividad «murmurar_conjuros» (vaiven), radio 0 | Naga arcano: murmura conjuros meciéndose; de misión en la plaza, radio 0. |
| `npc_race_race_elf_canopy` | Elfa del dosel | 1 | plaza_de_las_razas | actividad «otear» (giro), radio 1 | Elfa del dosel: acostumbrada a las alturas, otea girando; de misión en la plaza. |
| `npc_race_tieflings_de_la_sombra` | Tiefling de la sombra | 1 | plaza_de_las_razas | deambular, radio 1 | Tiefling: merodea receloso a un paso de su sitio; de misión en la plaza. |
| `on_anciana_lagarto` | Anciana de la aldea | 1 | on_ostad | deambular, radio 1 | Anciana de la aldea: paseo corto y lento; con misión, radio 1. |
| `on_archivera_ekait` | Archivera de Ekait | 1 | on_umedan | actividad «ordenar_libros» (vaiven, icono lore), radio 0 | Archivera de Ekait: ordena sus fondos; con misión, radio 0. |
| `on_barquero_celeste` | Barquero del Río Celeste | 1 | on_ostad | actividad «mirar_el_rio» (vaiven), radio 0 | Barquero: mira el río a un lado y a otro; con misión, radio 0. |
| `on_capataz_veta` | Capataz de la veta | 1 | on_bomengrid | vigilar, radio 0 | Capataz de la veta: vigila a los mineros girando; con misión, radio 0. |
| `on_chaman_bleek` | Chamán de los Bleek | 1 | on_havargruztak | actividad «murmurar_conjuros» (vaiven), radio 0 | Chamán: murmura meciéndose; con misión, radio 0. |
| `on_elfo_estrellas` | Elfo de las Estrellas | 1 | on_umedan | actividad «mirar_estrellas» (giro, icono curiosidad), radio 0 | Elfo de las estrellas: gira mirando el cielo; con misión, radio 0. |
| `on_guardacaminos` | Guardacaminos de Umedan | 1 | on_umedan | vigilar, radio 0 | Guardacaminos: vigila el camino girando; con misión, radio 0. |
| `on_portaestandarte` | Portaestandarte de los Cuatro | 1 | on_havargruztak | quieto (fijo:arriba), radio 0 | Portaestandarte: firme, mirando al frente; con misión. |
| `on_runista_rhenor` | Runista de Rhenor | 1 | on_bomengrid | actividad «grabar_runas» (golpe), radio 0 | Runista: graba runas a golpes de cincel; con tienda y misión, radio 0. |
| `on_thalgrim_menor` | Thalgrim el Menor | 1 | on_bomengrid | quieto (jugador), radio 0 | Thalgrim el Menor: quieto, grave; con misión. |
| `on_vigia_del_desierto` | Vigía del desierto | 1 | on_ostad | vigilar, radio 0 | Vigía: otea girando; con misión, radio 0. |
| `posadero` | Posadero | 1 | int:posada | actividad «ordenar» (vaiven), radio 0 | Tras el mostrador de la posada, ordena jarras y llaves; radio 0. |
| `profesora` | Profesora | 1 | int:universidad | actividad «dar_clase» (vaiven), radio 0 | Explica a un aula imaginaria mirando a un lado y a otro; radio 0. |
| `sacerdote` | Sacerdote | 1 | int:iglesia | actividad «rezar» (golpe), radio 0 | Inclina la cabeza en oración de vez en cuando; radio 0. |
| `sandes_joven` | Sandes, 756 | 1 | recuerdo_ceaseton | quieto (fijo:abajo), radio 0 | Recuerdo de 756: quieto, sin girarse hacia el jugador. |
| `sandes_puerta` | Sandes, 757 | 1 | recuerdo_puerta | quieto (fijo:abajo), radio 0 | Recuerdo de 757: quieto, sin girarse hacia el jugador. |
| `sastre` | Sastre | 1 | int:sastreria | actividad «coser» (golpe), radio 0 | Cose a puntadas rítmicas en su sastrería; radio 0. |
| `sectario_perdido` | Sectario de los Perdidos | 1 | centro | deambular, radio 1 | Merodea con la mirada en la calle; tiene diálogo de historia, así que radio 1 para que se le encuentre. |
| `tabernero_ceaseton` | Tabernero de Ceaseton | 1 | recuerdo_ceaseton | quieto (fijo:abajo), radio 0 | Recuerdo: quieto tras su barra. |
| `taquillera` | Taquillera | 1 | int:opera | actividad «vender_entradas» (vaiven), radio 0 | Atiende la taquilla del Ruiseñor; radio 0. |
| `tendero_mercado` | Tendero del mercado | 1 | int:mercado | actividad «ordenar_cesta» (vaiven), radio 0 | Detrás del mostrador del mercado, ordena la mercancía; radio 0. |
| `venides_joven` | Venides, 756 | 1 | recuerdo_ceaseton | quieto (fijo:abajo), radio 0 | Recuerdo de 756: quieto; el motor no lo mueve en cinemáticas y no debe girarse hacia ti. |
| `verina_joven` | Verina, 756 | 1 | recuerdo_ceaseton | quieto (fijo:abajo), radio 0 | Recuerdo de 756: quieta, sin girarse hacia el jugador. |
| `verina_puerta` | Verina, 757 | 1 | recuerdo_puerta | quieto (fijo:abajo), radio 0 | Recuerdo de 757: quieta, sin girarse hacia el jugador. |
| `viejo_surysal` | El viejo de Surysal | 1 | surysal | actividad «otear» (vaiven), radio 0 | Vigila el puente del vado mirando a ambos lados; tiene misión, radio 0. |
| `xila` | Xila | 1 | oeste | actividad «otear» (vaiven), radio 0 | Desconfía de todo: mira a un lado y a otro sin moverse; tiene misión, radio 0. |

### Claves para lo que aún no está colocado

| Clave | Rutina | Por qué |
|---|---|---|
| `npc_race_*` | deambular, radio 3 | Resto de razas del editor: paseo corto y amable (radio 3); ninguna colocada fuera de las de arriba. |
| `npc_npc_*` | deambular, radio 2 | PNJ generados del catálogo npcs.json (≈130, ninguno colocado aún): paseo corto. |
| `ciervo` | deambular, radio 3 | Animal de bosque: pasea sin prisa por su radio; mira a la calle, no al jugador. |

Reglas del catálogo (las cumple el motor): la clave exacta gana a un prefijo `*`; entre prefijos, el más largo; sin clave, `porDefecto` (`deambular`, radio 2, pausa 3-8 s, mira al jugador).

## Validador

`python3 tools/validar_rutinas_pnj.py` (`--resumen` lista qué rutina toca a cada tipo; `--estricto` hace que los avisos rompan).
Comprueba el contrato: versión, claves, valores permitidos, rangos (radio 0-6, pausas y `cada` de 0,5 a 60 s, `destino` hasta 6 casillas), iconos que existen en `iconos.json`, frases ≤ 40 caracteres y ≤ 3 por tipo, claves exactas que sean de verdad un `npc` del catálogo.
Avisa (sin romper) de: PNJ de los niveles que caen en `porDefecto` sin querer; prefijos que no casan con nada; tenderos de interior que se alejan del mostrador; PNJ con diálogo de historia (`trigger talk`) que se pasean a más de 1 casilla.
Prueba: `python3 tools/test_validar_rutinas_pnj.py` (49 casos con catálogos de mentira, uno por regla, más una prueba de humo sobre el repo).
Sobre los niveles de verdad: **0 errores, 0 avisos**, 108 tipos / 218 PNJ.

## Animaciones que harían falta de verdad (para Pixel)

Hoy todo se hace con los fotogramas que hay (andar y quieto en 4 rumbos + respiración). Estas serían las que de verdad mejorarían la ciudad, por orden de lo que más se ve:

| Animación nueva | Quién la usa | Qué la sustituye hoy |
|---|---|---|
| **Martillear / golpear yunque** (brazo arriba y abajo) | herreros, armero, enanas forjadoras, runista | `golpe` (el cuerpo baja y sube 2 px) |
| **Sentarse** (en banco o taburete; idle sentado) | parroquianos y jornaleros de la taberna, Venides, anciana, bordadora | de pie con `golpe`/`vaiven`; no se sienta |
| **Beber / brindar** (jarra a la boca) | parroquianos, Venides, carretero | `golpe` |
| **Barrer / limpiar la barra** | taberneros, Rosita, posaderos, tenderos | `vaiven` (mira a izquierda y derecha) |
| **Pregonar** (brazo en alto, boca abierta) | mercaderes, heraldos | `vaiven` + icono `oro` + frase |
| **Tocar el laúd** | bardo | `golpe` + icono `inspiracion` |
| **Coser / bordar / escribir** (manos juntas, cabeza baja) | sastre, bordadora, cronista, archiveros | `golpe` suave |
| **Rezar / arrodillarse** | sacerdote | `golpe` (inclina la cabeza) |
| **Saludar con la mano** | niños, anfitriona, guardias | no hay; solo gira hacia el jugador |
| **Saltar / corretear de verdad** | niños | `andar_sitio` (trotecito sin avanzar) |
| **Mirar arriba / señalar** | niña del gato | `quieto` con `fijo:arriba` |

## Dudas para Oriol

1. **Los recuerdos comparten ids con la taberna.** `interior_taberna` y `recuerdo_taberna` (y los otros `recuerdo_*`) usan los mismos `objectId`: lo que decide la rutina es el tipo, no el nivel. Los de la taberna brindan y limpian; si `recuerdo_taberna` se puede recorrer a pie fuera de la cinemática, también lo harían allí. Los que solo salen en recuerdos (`*_joven`, `*_puerta`, `capitan_aegroum`…) van `quieto` mirando fijo, nunca al jugador. ¿Vale, o prefieres una clave por nivel?
2. **Los de la plaza de las razas van con radio 0-1**, no con el radio 3 del ejemplo del contrato, porque tienen diálogo de misión. El radio 3 queda solo para las razas sin colocar (`npc_race_*`).
3. **Guardia, anciana, mercader y bardo tienen beats de misión** y aparecen muchas veces por la ciudad. Como la rutina es por tipo, los he dejado cortos (radio 0-1) en todas partes. Si quieres que las ancianas «paseen de verdad», habría que distinguir los PNJ de misión de los de relleno (p. ej. otro `objectId` para los de relleno).
4. **El bardo, el sacerdote y la sastre usan `golpe`** porque es lo único rítmico que hay; hasta que Pixel haga las animaciones, tocar, rezar y coser se verán igual que martillear (solo cambia el icono y la frase).
5. **Los ~130 `npc_npc_*` generados (catálogo `npcs.json`) no están colocados en ningún nivel.** Les he dado un paseo corto genérico por prefijo; si los pones en la ciudad, mejor una rutina por `role`.

## Notas de integración

- El JSON lleva un `_nota` en la raíz (como `iconos.json`); el validador lo admite. Si el cargador de `pnj_motor` fuese estricto con claves desconocidas, basta quitar esa línea.
- `tools/validar_todo.py` no se ha tocado (es del carril `puertas`): para vigilar el catálogo en cada validación, añadir `"validar_rutinas_pnj.py"` a `VALIDADORES_EXISTENTES`. Sale con código 1 solo si hay errores.
- El paquete solo añade 3 ficheros nuevos; `aplicar.sh` se niega si alguno ya existe. No depende de otros paquetes y puede ir antes de `pnj_motor`.
