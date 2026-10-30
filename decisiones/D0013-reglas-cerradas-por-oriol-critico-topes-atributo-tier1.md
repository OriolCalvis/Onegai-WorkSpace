# D0013 — Reglas que Oriol da por cerradas (08/10/2026): el crítico son dos 6, los topes del equipo son ley, con qué atributo tira cada carta y el Tier 1 basta

- **Estado:** `accepted` — lo decide Oriol el 08/10/2026, al revisar CARTAS-CLASES-01. Es inmutable:
  **no se vuelve a preguntar.** Si algún día cambia, será otra decisión que sustituya a esta.
- **Fecha:** 2026-10-08 · **Decide:** Oriol · **Lo escribe:** Claude (Cowork).
- **Sustituye a:**
  - como crítico, «todos los dados en 6» (Ed. 3, bitácora 2.17, y la forma en que se le preguntó el
    05/10 en ACOMPANANTES-01), «la mitad del pool en seises» (revisión Ed. 4 del 17/09, punto 1 y
    cabo suelto 3) y «la mitad o más de los dados son 6» (D0008, fila 1, D-COMB-02);
  - la etiqueta de «propuesta» que KIT-TOPES-01 (05/10) puso a los topes de dados del equipo;
  - los puntos 1, 2, 3 y 11 de «Lo que sigue en verde» del GDD Ed. 4 (§23) y lo que D0012 §3 decía
    de ellos.

## Por qué existe este fichero

Oriol, 08/10: «lo dels putos daus de 6 m'ho has preguntat dos mil putos cops i últimament, des de
què vaig decidir com, sempre m'ho tornes a preguntar». Sus respuestas estaban repartidas en
sesiones, ficheros de seguimiento y bandejas, y cada agente volvía a preguntar desde el último sitio
que había leído. Aquí quedan juntas y en firme, con sus palabras.

## Decisión

1. **Crítico: salen al menos dos 6.**
   - Con un dado no hay crítico (05/10: «¿El crítico de "todos 6" debe pedir al menos dos dados? Sí»).
   - Con dos dados, los dos en 6.
   - Con tres o más, basta con dos 6 (08/10: «ha de sortir 2 6 si tens més de 3 daus o més»).
   - Lo que hace el crítico no cambia (GDD §7.4 y §7.5): +1d6 al golpe; un dado más en la magia.
2. **Los topes de dados del equipo son ley: 2, 4, 6, 8, 8 y 8**, del Tier 0 al Tier 5 (08/10: «No,
   són llei»). Salen de su regla del 05/10 —«hay que respetar los caps de dados máximos que cada
   tier puede ofrecer a la espec más alta [...] y nunca jamás un jugador puede tener las tres a lo
   más alto»— y se aplican como en el motor desde KIT-TOPES-01 (`include/RPG/TopesDados.h`): el
   atributo con más bono del equipo llega a T; el segundo, a ¾ de T + 1; los otros dos, a la mitad
   de T. En Tier 1, como mucho 4, 4, 2 y 2.
3. **Con qué atributo tira cada carta:** las cartas estándar de una clase van alternando entre sus
   dos atributos principales (el principal y el secundario); las de Creador pueden tirar con casi
   cualquier atributo (08/10: «Normalment va alternant entre els dos atributs principals de les
   classes, menys els de creador, que aquests poden ser quasi qualsevol atribut»). Cada carta dice
   el suyo: «si no lo dice, tira DES» deja de ser una regla.
4. **El estudio del Tier 1 basta** (08/10: «Tier 1 ja està bé»): no hace falta simular los tiers
   2 a 5.

## Consecuencias

- **Reglamento:** `dndWeebCC-master/docs/Sistema_Cartas_Tiers.md` (§3, §4, §7.3, §21 y §23) y las
  cartas de reglas de mesa (`dndWeebCC-master/data/reglas/referencia_mesa.json`) ya lo dicen así.
  D0012 remite aquí.
- **El motor todavía juega «todos 6 con dos dados o más»** (`include/RPG/DicePoolEngine.h`,
  `roll_pool`). Cambiarlo es la tarea **CRITICO-DOS-SEIS-01** del TABLERO (Pendiente). Con dos 6 el
  crítico deja de ser casi imposible en las tiradas grandes:

  | Dados | «Todos 6» (hoy en el motor) | «Al menos dos 6» (ley) |
  |---:|---:|---:|
  | 2 | 2,8 % | 2,8 % |
  | 3 | 0,46 % | 7,4 % |
  | 4 | 0,08 % | 13,2 % |
  | 5 | 0,01 % | 19,6 % |
  | 6 | 0,002 % | 26,3 % |

  Es la probabilidad de que la tirada lo saque; el crítico solo cuenta si además se gana. Por eso,
  después del cambio hay que repetir el estudio de dificultad del Tier 1 (GDD §21), que se midió
  con «todos 6».
- **Las cartas que no dicen su atributo son un hueco de contenido, no una pregunta:** en el creador,
  584 de las 935 habilidades de las clases no tienen `castingStat` (50 de 60 en la 3.ª iteración,
  348 de 440 en las SP y 186 de 235 en la 2.ª iteración), y las 20 clases propias lo tienen pero
  casi siempre es el principal (194 de 200 cartas), sin alternar. Hasta que se completen, el
  videojuego (`CatalogLoadersHabilidades.h` pone DES por defecto) y el mazo impreso las tiran con
  DES. Se arregla cuando se retomen las clases.
- **Topes:** el motor ya los aplica (`assets/rules/tier_rules.json`, `topeDadosEquipo`); solo
  cambia que dejan de ser propuesta. Sigue pendiente, como contenido, rehacer el equipo de Tier 1
  para que el 4, 4, 2, 2 exista y sea muy difícil de juntar (KIT-TOPES-01).
- **Bandeja de Oriol:** las filas del 17/09 («Reglamento Ed. 4 — cinco cabos sueltos») y del 28/09
  (restos de D0008) llevan anotado que lo del crítico está respondido aquí. Solo él las vacía.
