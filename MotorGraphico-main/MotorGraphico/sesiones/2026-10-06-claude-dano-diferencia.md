# El daño por diferencia de éxitos, la magia con salvación y las vidas a la mitad · 06/10/2026

Sesión de Claude (Cowork). Sigue a `2026-10-06-claude-acompanantes-dificultad.md`. Oriol, al ver
la fórmula de daño del estudio:

> «Lanzas tus dados y comparas éxitos y tiras dados de 6 según los éxitos [...] saco 2d6 y tengo
> un 5 y un 6, tú sacas 1d6 y sacas un 6, pues yo ahora lanzo un dado de 6, porque es la
> diferencia de lo que pego y te bloqueas; te hago por ejemplo 1d6 de daño más habilidades y daño
> de bonificadores de daño este turno.»
> «Las cartas con dados propios [...] depende de si es magia o no, porque la magia es tirada de
> salvación y eso es otra cosa. Lo demás está bien. Hay que bajar vidas 100 %.»
> «Al superarla reciben la mitad del daño y jamás sus debufos, a no ser que se indique lo
> contrario en una carta.»

Seguimiento: `planificacion/seguimiento/DANO-02.json`.

## 1. La fórmula (`include/RPG/DanoEnfrentado.h`)

**Golpe físico** (Atacar, las armas, las invocaciones, los acompañantes, las cartas que no son
magia): los dos tiran y se restan los éxitos; la diferencia, redondeada hacia arriba, son los d6.

| Tirada | Daño |
|---|---|
| 2 contra 1 (un 5 y un 6 contra un 6) | 1d6 |
| 1,5 contra 1 | 1d6 (medio éxito, un dado) |
| empate o pierde | 0 |
| el defensor pifia | todos los éxitos del atacante en d6 (mínimo 1) |
| todos 6, con dos dados o más | +1d6 |
| el atacante pifia | 0 |

Las cartas con dados propios (Corte Real 2d6, Arsenal del Rey 1d6) los suman a los de la
diferencia **solo si se gana**. Los bonos del turno se suman encima: Puño Volcánico (+2d6 al
arma) y la marca de Marca Profana (+1d6), que ahora vale también para las cartas.

**Magia** (la carta dice contra qué se salva): el lanzador tira contra la salvación del
objetivo.

| Salvación | Daño | Estados |
|---|---|---|
| falla | los dados de la carta enteros | sí |
| pifia (o el lanzador saca todos 6) | un dado más de la carta | sí |
| la supera (o empata) | la mitad | nunca |
| el lanzador pifia | nada | no |

La pifia de la salvación da un dado más (era la propuesta «×1,5 o un dado más»; elegí el dado,
igual que el crítico físico). Las cartas que digan otra cosa irán con su gancho cuando lleguen.

**Habilidades de monstruo** (las legadas, con `power`): pegan como un golpe y suman un d6 por cada
10 de poder, solo si ganan.

## 2. Qué cartas son magia

Las que piden salvación llevan `defenseStat` en `skills.json` (el importador,
`tools/importar_clases_companeros.py`, tiene la lista en `SALVACION`). De las cuatro clases:

| Carta | Clase | Salvación |
|---|---|---|
| Canción del Otoño | Bardo | defensa mental (CAR) |
| Canción del Invierno | Bardo | resistencia física (CON) |
| Canción del Verano | Bardo | resistencia física |
| Absorción de Alma | Apóstata | resistencia física |
| Destructor del Escudo de Sombras | Apóstata | resistencia física |
| Maldición de la Debilidad | Apóstata | defensa mental |
| Marca Profana | Apóstata | defensa mental |
| Estruendo Desolador | Apóstata | defensa mental |
| Castigo Divino | Furia | resistencia física |
| Ataque del Rayo | Furia | resistencia física |
| Desarme | Furia | defensa mental |

Son golpes (físicas): Corte Real, Cadena Real, Ráfagas del Arsenal, Arsenal del Rey (lleva la
etiqueta «Mágico», pero es un arma que ataca), Ira Imparable, Furia: Fe, Fuerzas Internas, A las
Armas y Destructor. El daño por turno (Castigo Divino, Absorción de Alma) es un estado: si se
salva, no lo recibe.

## 3. Las vidas, a la mitad

Un golpe hace ahora 3,5 por dado (antes 10 por éxito), así que todo baja a la mitad,
redondeando hacia arriba:

- **Héroe**: la vida base de las 63 clases (`classes.json`) y el tope y el bono de cada tier
  (`tier_rules.json`: 14, 18, 25, 35, 48 y 63). La CON sigue valiendo ×3: es una de las preguntas
  abiertas. Con el kit, un héroe de Tier 1 tiene de 13 a 18 PV.
- **Enemigos**: los 432 monstruos (`monsters.json` y las librerías `libreria_enemigos` y
  `libreria_completa`), la biblioteca del editor, la fauna, `on_enemigos` y Boundington.
- **Invocaciones** (`summons.json`) y **acompañantes** (Venides 8, Xila 3).

## 4. La escalera nueva

Con la diferencia, un enemigo que ataca con un dado (DES 0) casi nunca hiere: el estándar pasa a
tener DES 1. Escalera de Tier 1 (`simulador_dificultad`, 41 razas × 25 combates por clase):

| Escalón | PV | CON | DES | 1 contra 3 con el kit |
|---|---|---|---|---|
| F1 esbirro | 4 | 0 | 0 | 2 % |
| F2 recluta | 6 | 1 | 0 | 8 % |
| F3 estándar | 8 | 1 | 1 | 38 % |
| F4 veterano | 6 | 1 | 2 | 50 % |
| F5 élite | 8 | 2 | 2 | 64 % |
| F6 élite dura | 6 | 2 | 3 | 74 % |
| F7 campeón | 8 | 2 | 3 | 85 % |
| F8 campeón fiero | 10 | 3 | 3 | 91 % |
| F9 jefe | 8 | 3 | 4 | 95 % |

Cada nivel de equipo con su escalón (modo `niveles`):

| Equipo | Contra | 1 contra 3 | 2 contra 3 | 3 contra 3 |
|---|---|---|---|---|
| H1 kit del cofre | F3 | 38 % | 8 % | 2 % |
| H2 tope 3 en la clase | F4 | 36 % | 5 % | 0,5 % |
| H3 tope 4 en la clase | F6 | 36 % | 6 % | 1 % |
| H4 tope 4 en CON y DES | F8 | 32 % | 6 % | 1 % |
| Un escalón más fuertes | | 48–51 % | 9–13 % | 1–5 % |

Las razas siguen pesando: del 0 % (hombres lagarto de la selva, CON 4) al 88 % (guardianes del
elemento, CON 2 y DES 1), media 36 %.

**Boundington**: devoto F3 (8 PV, CON 1, DES 1), fanático F4 (6 PV, CON 1, DES 2), carcelero
10 PV CON 1 DES 1, saqueador 11 PV CON 1, Gorran 17 PV CON 1 DES 1. El Ocaso con Venides: 0,1 %
con 20 hombres, 0,5 % con 10, 7 % sin hombres (con Xila, 5 %); antes 0 %, 0,1 % y 6 %. Gorran
mata un 4 % de las veces en 1 contra 1, como antes; con DES 2 sería un 10 % y con CON 2 y DES 3,
un 25 %.

Los combates son algo más largos: 6 a 13 rondas en 1 contra 3 (modo `log`).

## 5. Código

- `include/RPG/DanoEnfrentado.h` (nuevo, solo cabecera): `dadosFisicos`, `salvacion`,
  `dadosExtraMagia`, `danoMagico`.
- `src/Game/CartasEnCombateInterno.h`: `Tirada` guarda los éxitos de los dos lados
  (`exitos`) en vez de la magnitud; `dadosDeGolpe`, `danoDeCarta`, `esMagia`.
- `src/Game/CartasTiradas.cpp`: `tirar` y `tirarSinRival` llenan los éxitos; `golpeDeArma` tira
  los d6 de la diferencia («Crítico»/«Éxito sin tirar»: el objetivo no se defiende); la
  habilidad de monstruo; `bonoDanoContra` (nuevo, también para las cartas); el empate sale
  como «Empate» en el registro.
- `src/Game/CartasJugar.cpp`: el daño de las cartas con `danoDeCarta`; las físicas tiran con la
  defensa física (la CA de los estados cuenta).
- `assets/catalogs/skills.json` (11 `defenseStat`), `tools/importar_clases_companeros.py`
  (`SALVACION`, vida base del monje y del mago chasqueador, el espectro y el cuervo).
- Vidas: `classes.json`, `tier_rules.json`, `monsters.json`, `summons.json`,
  `boundington_enemigos.json`, `boundington_ocaso.json`, `libreria_enemigos.json`,
  `libreria_completa.json`, `editor_biblioteca_enemigos.json`, `editor_biomas_fauna_equipo.json`,
  `on_enemigos.json`.
- `examples/simulador_dificultad.cpp`: la escalera nueva y tres variables para probar sin tocar
  los datos (`MG_VIDA_HEROE`, `MG_VIDA_ENEMIGO`, `MG_ESCALERA`).
- `examples/demo_narrativa_combate.cpp` (el saqueador tiene 11), `FORMATO_NIVELES.md` (la escala
  de vida).

## 6. Pruebas

- La mesa de prueba (`tests/Game/CartasPrueba.h`) tiene ahora un lobo de DES 5: con el dado en 5
  los dos aciertan por medio éxito, 1d6 = 5. Las cuentas de las pruebas de cartas, acompañantes
  y objetivos, rehechas con la fórmula nueva.
- Nuevas: `dano_fisico_son_tantos_d6_como_la_diferencia`,
  `dano_magico_entero_si_falla_la_salvacion_y_la_mitad_si_la_supera`,
  `una_carta_fisica_suma_sus_dados_solo_si_gana`,
  `una_carta_de_magia_hace_la_mitad_a_quien_se_salva_y_nunca_sus_estados`.
- `test_cartas_combate`, `test_barra_combate` 19/19, `test_game_session` 31/31,
  `test_enemy_catalog_bridge` 14/14, `test_pergamino` 62/62, `demo_battle`,
  `demo_campana_boundington` y la prueba de humo del simulador en verde. `test_catalog_content`
  (el arma flotante tiene 2 PV) falla solo en lo que ya fallaba. ctest: las mismas 9 de antes.
  La guardia: las mismas 44 deudas.

## 7. Para Oriol

1. La pifia de la salvación: ¿un dado más (como está) o ×1,5?
2. ¿Están bien elegidas las once cartas de magia y su salvación? Arsenal del Rey lleva «Mágico»
   pero lo dejé como golpe.
3. Siguen abiertas: si las cartas tiran con el atributo principal (hoy DES), el peso de la CON
   en la vida (×3), Gorran más duro, los topes 2, 4, 6, 8, 8 y 8, y si ponerse el kit cura.
