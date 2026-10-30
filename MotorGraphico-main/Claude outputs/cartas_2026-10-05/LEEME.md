# Cartas de cuatro clases en combate · H2-06 · 05/10/2026

Las cartas del **Bardo de las Cuatro Estaciones**, **El Apóstata**, el **Señor de
las Armas** y la **Furia de los Creadores** ya se juegan en el combate real del
juego. Cada carta hace lo que dice su texto (DEC-02): el efecto está escrito como
gancho (`effects[].mechanicHook`) en `skills.json` y `passives.json`, y el motor lo
ejecuta. No hay código por carta. Solo combate, como pidió Oriol: lo de fuera del
combate (descansos, el día, la estación fuera de la pelea) queda para el final.

Y lo que pidió a mitad: **la creación ya no reparte stats**. CON, DES, INT y CAR
empiezan en 0; la raza suma su bono y después solo los sube el equipo activo. La
partida nueva queda en 14 pasos (13 de la ficha más Apariencia).

## Cómo probarlo en el Mac

1. Compilar como siempre (hay una prueba nueva, `test_cartas_combate`, 30 casos).
2. `./juego` → **Nueva partida** → en «A qué te dedicas» elegir una de las cuatro
   clases → ENTER hasta **Empezar la aventura**.
3. Entrar en un combate. En la campaña el primer enemigo tarda en llegar; para
   probar rápido, en una **copia** de `assets`, poner en
   `levels/interior_vacio.json`
   `"objects": [{"objectId": "perdido_saqueador", "position": {"x": 7, "y": 9}}]`
   y dar un paso a la derecha (D). Las capturas están hechas así, con el prólogo
   apagado en esa copia (en el árbol no se ha tocado ningún nivel).
4. En combate: W/S elige, ENTER juega. Debajo de Atacar / Poción / Huir salen las
   cartas, y el cuadro de abajo explica la que está marcada.

## Qué hay aquí

| Fichero | Qué es |
| --- | --- |
| `combate_con_cartas.gif` | 16 momentos del juego real, las cuatro clases (2,2 s cada uno). |
| `capturas/` | 16 capturas a 1280×720: creación (sin reparto) y una pelea por clase. |
| `H2-06.patch` | Todos los cambios como parche sobre el árbol del Mac del 05/10 a las 10:02 (ya están aplicados; es copia de seguridad). |

Salen del juego compilado en Linux con OpenGL por software.

## Cómo se juega

- **La mano**: todas las cartas de la clase desde el principio. Las de creador
  (máx. 3 por clase) son de tier 2 en el catálogo: salen con «(tier 2)» y no se
  juegan hasta tier 2.
- **El turno**: una acción principal (cierra el turno), un movimiento, una
  canalización y las libres. Las **reacciones** se preparan sin gastar la acción y
  saltan solas cuando te atacan, una por ronda.
- Cada carta se **gasta** al jugarla. De momento cada combate empieza con la mano
  llena (todavía no hay descansos).
- Lo que no se puede jugar lo dice en la fila: (gastada), (tier 2), (preparada),
  (el escudo está vacío), (no hay canciones gastadas), (pendiente),
  (necesita posiciones). El motivo entero sale en el cuadro de abajo.
- El **Bardo** elige estación al empezar cada combate (acción libre).
- El registro usa el nombre del personaje y apunta la vida que de verdad se cura.

## Lo que hace cada clase

- **Bardo**: las cuatro canciones (Otoño: desventaja; Invierno: fuera de combate
  y defensa con desventaja; Primavera: cura 1d6; Verano: 3d6), Ola de Elementos
  devuelve una canción gastada, Bendición de las Estrellas bloquea un ataque,
  Invocación Elemental da ventaja. La estación: Primavera +2 a las curas, Verano
  ventaja al atacar, Otoño ventaja al defender, Invierno +2 dados de defensa
  física y refleja el primer turno.
- **El Apóstata**: Escudo de Sombras (cada golpe que das le suma 2, hasta 10, y
  absorbe daño), Destructor del Escudo (hace el daño que tenía el escudo),
  Absorción de Alma (canaliza: drena 1d6 tres rondas), Maldición de la Debilidad,
  Invocación Oscura (espectro menor o cuervo, 10 turnos, atacan solos), Marca
  Profana, Estruendo Desolador, y **Desgarro Sangriento** para recuperar una carta
  gastada.
- **Señor de las Armas**: Escudo Divino (reacción: bloquea y ciega), Ráfagas del
  Arsenal (tres golpes), Arsenal del Rey, Cadena Real, Corte Real.
- **Furia de los Creadores**: Castigo Divino (1d6 por turno, tres turnos), Ataque
  del Rayo, Protección del Creador (reacción), Furia: Fe (golpeas y te curas lo que
  haces), Furia Justiciera, Ira Imparable, Puño Volcánico.

Bloqueadas, con su motivo: Paso Ilusorio (necesita posiciones), Susurro Demoníaco
y Ataque Maldito (el canon no los cierra), la pasiva y dos cartas del Señor de las
Armas (no están definidas).

## Reglas provisionales (solo combate)

- Daño de carta = suma de los dados × el grado de la tirada enfrentada (fallo 0,
  parcial ½ con mínimo 1, éxito ×1, crítico ×1,5).
- «+2 a la CA» = +2 dados de defensa física.
- Carta sin `castingStat` → DES (lo que ya decía D-02).
- «En un área» = el objetivo elegido: todavía no hay posiciones en el combate.

## Creación sin reparto

- Fuera el paso «Reparto» y su bolsa de 12 puntos. La base es 0 y el resumen
  enseña base + raza. El tope de 6 en tier 0-1 se sigue comprobando (por si llega
  una ficha pregenerada).
- Pruebas y demos al día. De paso, `demo_creacion_personaje` esperaba 61 clases y
  hay 63 desde el 29/09.

## Pendiente y preguntas para Oriol

1. **Cartas de creador**: el catálogo las pone en tier 2, así que a tier 1 no se
   juegan. ¿Se quedan así o deberían estar desde tier 1?
2. **«+2 CA»** como +2 dados de defensa física: ¿vale?
3. **Stat de lanzamiento**: las cartas sin `castingStat` tiran con DES. ¿El bardo
   debería tirar con CAR y el Apóstata con INT?
4. **Invocación Elemental**: «obtiene 2 PA». ¿Qué son los PA?
5. **Desgarro**: la carta se recupera y el contador sube, pero el contador todavía
   no hace nada. ¿Qué pasa cuando crece?
6. **Furia**: ¿cómo entra la clase «En Furia» y qué cartas son «de magia»?
7. **Señor de las Armas**: faltan su pasiva y dos cartas en el canon.
8. **Con base 0 hay dos efectos a decidir**:
   - En las pruebas, la vida a tier 1 quedó entre 10 y 24 según raza y clase, y
     un golpe básico de enemigo quita 10 (15 con crítico). El personaje murió dos
     veces en uno o dos golpes.
   - 28 de las 182 piezas de equipo inicial de las clases piden stats (cota de
     malla CON 1, armadura de placas CON 3, bastón arcano INT 2). Sin el bono de
     raza se quedan en la mochila. ¿El equipo inicial debería saltarse el
     requisito?
9. Las invocaciones traen **FUE** y el cargador no lo lee.
10. Fuera del combate (descansos, el día, la estación): hablarlo al final.

## Encontrado de paso (no es de este paquete, no está arreglado)

- **«Volver al título» desde «Has caído» o desde la pausa revienta el juego.** Se
  borra la sesión y `Application::update()` la sigue usando antes de cambiar de
  escena; además la escena de título no vuelve a cargar ningún nivel.
- `demo_nova_partida` falla en Linux: coge la «primera» clase de un
  `unordered_map` y allí sale una sin equipo inicial. En el Mac el orden es otro.

## Para el commit

Todo está en el árbol, sin commit. `cmake/CartasCombateTests.cmake` va en el mismo
commit que su `include()` en `CMakeLists.txt`, junto con los cinco `.cpp` nuevos
de `src/Game/`: la guardia lo avisa mientras no estén versionados.
