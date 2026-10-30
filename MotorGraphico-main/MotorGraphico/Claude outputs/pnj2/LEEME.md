# PNJ-02 — Los pueblos del lore: el este, los yokai y los drows corrompidos

Oriol, 05/10/2026:

> Pueblos del este: Choubar (hombres pájaro), Mistarium (tieflings) y Nocturnsea
> (inhabitada, es la entrada a la casa de la muerte, dicen los antiguos). Gongorguma
> son orcos. Ayashe, drows. Y los yokai son espíritus, así que ya dejaron de tener
> cuerpo material: siguen en el lore, pero no son visibles. Los sagas son los drows,
> por eso quiero darles este toque de corrupción estética a los drow y ponerles
> musgos, cortezas, piedras o partes de animales mutando su aspecto de elfo al de un
> ser más perturbador sin abandonar la forma de su ancestro.

Y sus respuestas: los PNJ de Nocturnsea, «que sean muertos»; el yokai de la plaza,
«quitarlo»; en la creación de personaje, «que no se puedan elegir».

## Láminas

| Fichero | Qué es |
| --- | --- |
| `pnj2_01_drows_antes_ahora.png` | Las cuatro razas drow andando (vestidas como su PNJ de raza), antes y ahora. |
| `pnj2_02_este_antes_ahora.png` | Diez PNJ del este: de humanos a su pueblo. |
| `pnj2_03_juego_plaza_once.png` | La Plaza de las Razas en el juego: once pueblos, sin el yokai. |
| `pnj2_04_juego_este_de_cerca.png` | Los PNJ del este en el juego (Grytoz, Klimnebra, Venordemn, Guskedor) y el explorador drow de la plaza. |
| `pnj2_05_corrupcion_de_cerca.png` | Las cabezas de los cuatro drows, de frente, de perfil y de espaldas. |

## Qué ha cambiado

**El este.** La nación sale de la id del PNJ o, si no la dice, de los niveles donde
está colocado («Choubar - Klimnebra»):

- Choubar → aarakocra (del viento y de las montañas). El capitán Ventobélico también.
- Mistarium y el clan Neblinia → tieflings (la enviada de *La Señal de la Ceniza* ya
  era «una tiefling de librea oscura»). La archivera de los Espejos también.
- Nocturnsea → muertos, en los mismos sitios y con los mismos diálogos: la farera y el
  posadero de Grytoz son espectros; el herrero, el mercader y el tabernero, renacidos.
  Pelo blanco, gris o negro.
- Gongorguma y Qethatos → orcos (como antes; la anciana del Consejo ya es orca).
- Ayashii → drows (hoy no hay ningún PNJ colocado allí).

**Los yokai** ya no se ven en ningún sitio:

- Fuera los objetos `npc_race_yokai_*` del catálogo del editor (y de la cola de previews).
- La Plaza de las Razas tiene once pueblos: fuera su colocación, su beat y su flag.
  Los textos dicen «Once pueblos distintos…», «Once personas de once sitios…» y
  «Has escuchado a los once».
- La creación de personaje no los ofrece ni los acepta: `"playable": false` en
  `races.json`; el motor lo lee en `RaceDefinition::playable`.
- El generador de PNJ rechaza cualquier PNJ yokai.
- Siguen en el catálogo de razas, en el códice y en el atlas de capas: el lore no cambia.

**Los drows, corrompidos.** Conservan lo que los hace elfos: orejas puntiagudas,
cuerpo esbelto y pelo claro. La corrupción se les pega encima, de menos a más:

| Raza | Corrupción |
| --- | --- |
| Drow de la casa noble | Piedra: una corona de esquirlas de obsidiana que les nace del cráneo (crece más por un lado) y corteza en la mandíbula. |
| Drow explorador | Bosque: astas de rama con musgo y musgo en los hombros. |
| Saga de la locura | Lo peor del bosque: astas retorcidas (la otra, rota) con musgo colgando, corteza alrededor de un ojo y manos de corteza. |
| Saga del abismo | Lo peor de la piedra: cuernos de carnero de piedra negra, placas de piedra alrededor del otro ojo, manos de piedra y un tercer ojo que brilla. |

Las sagas son drows. Tienen orejas de elfo, ya no llevan la nariz ganchuda y su piel
es la del drow: enferma de musgo (locura) o helada de abismo.

Todo PNJ de estas razas cambia con ellas: el explorador drow de la plaza, los PNJ de
raza del editor y la saga del bosque de la historia.

## Números

- **248 PNJ en 324 aspectos.** Son dos menos que en PNJ-01: los yokai.
- Cambian 28 aspectos de PNJ por la nación: los del este y sus variantes.
- Cambian también los cuatro cuerpos drow.
- PixelLab: 0 generaciones.

## Ficheros

- **Nuevo:**
  - `tools/actores_corrupcion.py`: las plantillas, dibujadas por coordenadas.
  - `planificacion/seguimiento/PNJ-02.json`.
- **Generador:**
  - `tools/actores_pnj.py`: naciones, nación por nivel, sin yokai.
  - `tools/actores_razas.py`: los cuatro drows.
  - `tools/actores_rasgos.py`: registra las plantillas.
  - `tools/actores_materiales.py`: corteza, musgo, obsidiana, piedra del abismo, brillos y las pieles de saga.
- **Assets:**
  - `personajes/razas/{cuerpo,morro,rasgos}.png`.
  - `razas_andando.png`, `personajes_historia.png`.
  - `pnj_*.png/.json`, `personajes.json`, `personajes.md`.
  - `races.json`, `editor_npcs_por_raza.json`, `previews_pendientes.json`.
  - El nivel y la aventura de la plaza.
- **Motor (hay que recompilar):**
  - `RaceDefinition::playable` y su cargador.
  - `CharacterCreationScreen` (la lista) y `CharacterCreation::problemas` (la validación).
- **Pruebas:**
  - `tests/RPG/CharacterCreationTests.cpp`: caso nuevo `els_pobles_sense_cos_no_es_poden_triar`; las razas de prueba se eligen entre las jugables.
  - `examples/demo_plaza_razas.cpp`: 11 razas y 13 beats.
  - `examples/demo_nova_partida.cpp`: la primera raza jugable.

## Verificación

- `test_personajes_capas`: el motor monta byte a byte los 324 aspectos y los cuerpos nuevos.
- `test_character_creation`: 5/5. `test_character_creation_flow`: 5/5. `demo_plaza_razas` OK.
- `test_registro_sprites` y `test_editor_apariencia` OK.
- `validar_pnj` y `validar_todo`: 0 errores.
- `comprobar_contrato_cargadores`: OK.
- ctest: las 18 fallas de siempre. `linea_base_core`: las 35 deudas previas.
- Capturas del juego: plaza, Grytoz y su taberna y posada, Klimnebra y su herrería, Venordemn y Guskedor.

## Cómo se rehace

```sh
python3 tools/gen_personajes.py      # cuerpos, hojas y PNJ
python3 tools/validar_pnj.py
python3 tools/probador_personaje.py --raza sagas_del_abismo --gif
```

## Para decidir

- Hay **31 enemigos de la facción `yokai_del_umbral`**: cambiaformas, cuentacuentos
  hueco, el gato del dintel… No los he tocado. Si los yokai no se ven, ¿son
  manifestaciones o hay que cambiarles la facción?
- El yokai de la plaza decía «He visto caer tres ciudades. Todas olían así la víspera».
  Se ha ido con él. Si quieres conservar la frase, puede decirla otro.
- Los interiores de las ciudades del este (la taberna o la posada de Grytoz…) son salas
  vacías. Amueblarlas sería otro encargo.
