# D0012 — Las clases y las métricas del motor suben al creador de cartas (reglamento Ed. 4)

- **Estado:** `accepted` en lo que Oriol encargó el 08/10/2026 —integrar en el creador web de cartas
  (`dndWeebCC`) las clases trabajadas en el motor, poder descargar sus cartas para jugar en la mesa y
  unificar el reglamento—; `proposed` en dos cosas que **esperan a Oriol**: las preguntas «en verde»
  que quedan en el reglamento (`dndWeebCC-master/docs/Sistema_Cartas_Tiers.md` §23; el 08/10
  Oriol cerró cuatro en `D0013`) y qué hacer a medio plazo con
  el sentido del flujo (§4 de este fichero).
- **Fecha:** 2026-10-08 · **Autor:** Claude (Cowork) · **Tarea:** CARTAS-CLASES-01 (TABLERO).
- **Encargo de Oriol (08/10):** «Vull que agafis tot el que hem fet sobre les classes dins el motor
  gràfic i m'ho integris al creador web de cartes [...] poder descarregar les cartes d'habilitat per
  poder jugar les classes al joc de rol físic [...] també s'haurien d'actualitzar les regles per
  unificar-ho; de moment crec que les mètriques del motor gràfic estan més polides respecte al joc
  que volem, però tu em diràs.»

## 1. Contexto

La regla de propiedad (BITACORA §2.1) dice que `dndWeebCC` manda en reglas, fórmulas, tiers y
balance, y que el contenido baja `canon → cartas → motor`, sin aristas de vuelta. En la práctica,
desde el 27/09 las clases se escriben **en el motor**: `tools/importar_clases_companeros.py`
(3.ª iteración), `tools/sp_*.py` + `importar_clases_sp.py` (las 44 SP) y `tools/cp_*.py` (las 20
propias) escriben directamente en `MotorGraphico/assets/catalogs/`, a partir de los documentos de
Oriol. Y los números se han ido decidiendo jugando en el motor: la vida a la mitad y el daño por
diferencia (DANO-02), el kit y los topes (KIT-TOPES-01), el crítico de dos dados y la escalera de
dificultad (ACOMPANANTES-01), los hitos por tier (NIVELES-01).

Resultado, medido el 08/10:

| | Motor | Creador de cartas |
|---|---:|---:|
| Clases | 127 (6 de la 3.ª iteración, 20 propias, 44 SP, 57 de la 2.ª) | 61 (2.ª iteración) |
| Cartas de habilidad | 1.191 (700 con ganchos) | 483 (sin ganchos) |
| Vida base de un tanque | 8 | 16 (y la plantilla de la Ed. 2) |
| Reglamento | Ed. 3 + lo decidido del 17/09 al 08/10 | GDD Ed. 2: CA = 10 + DES, tabla de CD, techo de vida 35-125, daño = éxitos × arma |

El GDD contradecía a la Ed. 3 (que quitó la CA y la CD el 17/08) y no estaba simulado. Y había un
riesgo concreto: volver a convertir desde `dndWeebCC` (`tools/convertir_libreria_dnd.py`) pisaba con
lo viejo lo que el motor ya tenía (relevo de Claude del 08/10, pregunta 8).

## 2. Decisión

1. **Las clases suben del motor al creador de cartas** con `dndWeebCC-master/scripts/importar_clases_motor.py`:
   lee los catálogos del motor (solo lectura) y escribe una carta por fichero —las 127 clases, sus
   habilidades, hechizos, pasivas, las invocaciones que traen, su equipo inicial, su set de Tier 2 y
   las condiciones que nombran—. Upsert por id, sin borrar nada, idempotente (la segunda pasada da
   «1.665 iguales»). Se vuelve a ejecutar cada vez que el motor cambie las clases. Es una **arista de
   vuelta declarada**, no escondida.
2. **El reglamento adopta las métricas del motor como Ed. 4**: `Sistema_Cartas_Tiers.md` §3-§7, §10,
   §11 reescritas, y §21-§23 nuevas (encuentros, las clases y su mazo, lo que sigue en verde). Los
   números viven en tres sitios que se cambian juntos: `MotorGraphico/assets/rules/tier_rules.json`,
   el GDD (§3 y §5) y `CalculadoraVida` (web).
3. **El creador de cartas descarga el mazo físico** de cada clase (y de cada familia, en un PDF o en
   un ZIP): A4, 9 cartas de 63,5 × 88,9 mm, con la clase, la pasiva, las 7 + 3 cartas, las
   invocaciones, el equipo y cinco cartas de reglas de mesa (`data/reglas/referencia_mesa.json`).
4. **El editor web ya no borra lo que no conoce**: los modelos de carta guardan los campos que no
   tienen (ganchos, rareza, familia, set...) y los vuelven a escribir; al editar una carta desde el
   formulario se conservan.

## 3. Por qué el motor y no el GDD

Oriol preguntó si las métricas del motor están más pulidas o si es pronto para decidirlo. Lo
primero, con matices:

- **Están probadas.** Cada regla del motor tiene pruebas (`test_cartas_combate`, `test_kit_basico`,
  `test_sheet_milestones`...) y el balance sale de `simulador_dificultad`: 41 razas × cuatro clases ×
  decenas de combates por caso, con la mesa de verdad. Ninguna cifra del GDD Ed. 2 se había simulado.
- **Son coherentes con la Ed. 3.** El GDD seguía con CA y CD; el motor resuelve todo enfrentado.
- **Responden a lo que Oriol pidió del juego**: que un uno contra tres mate en torno al 40 % y un
  tres contra tres casi nunca; que el equipo sea lo que hace crecer al personaje; que las tiradas
  enfrentadas y el dado de seis se lean de un vistazo.
- **Pero no está todo cerrado.** Quedan preguntas en verde (GDD §23): las que más pesan, el peso de
  la CON y la muerte y la agonía; y 57 clases son todavía de la 2.ª iteración. El 08/10 Oriol cerró
  cuatro (`D0013`): el crítico son dos 6, los topes del equipo son ley, las cartas alternan los dos
  atributos principales de la clase (las de Creador, casi cualquiera) y basta con el Tier 1.

## 4. Lo que queda por decidir: el sentido del flujo (`proposed`)

| # | Opción | Consecuencia |
|---|---|---|
| 4.a | **El motor escribe las clases y el creador las importa** (lo de hoy, declarado). Se anota en la BITACORA §2.1 que las clases son la excepción. | Cero trabajo. Pero la web no es la fuente: lo que se edite en ella lo pisa la siguiente importación. |
| 4.b | **Los generadores de clases pasan a `dndWeebCC/scripts`** (`cp_*.py`, `sp_*.py`, `importar_clases_*.py`) y escriben aquí; el motor importa desde las cartas. | Vuelve el `canon → cartas → motor`. Hay que mover los scripts y su prueba, y las piezas de dibujo (trajes, sets, partículas) se quedan en el motor. |
| 4.c | **Mixto**: las cartas mandan en los números y los textos; el motor, en el dibujo. | El más limpio a largo plazo y el que más trabajo pide. |

**Recomendación:** 4.a mientras las clases sigan cambiando cada día, y 4.b cuando se calmen.

## 5. Consecuencias

- `data/cartas` gana 1.223 cartas y actualiza 442 (la vida a la mitad, los textos con tildes, las
  cartas de la 3.ª iteración...). No se borra ninguna: las cartas de la 2.ª iteración que la 3.ª
  sustituyó (`cancion_de_invierno`, `luz_de_las_estaciones`...) siguen en disco, sin clase que las use.
- Las 61 clases de antes conservan sus cartas, con la vida base a la mitad, como en el motor.
- La vida de la web (fichas de personaje y ficha de clase) se calcula con la fórmula Ed. 4. Las
  **defensas numéricas** de la ficha de personaje (CA, defensa mental, resistencia física) **siguen
  siendo las de la Ed. 2**: pasarlas a dados de defensa es trabajo pendiente del módulo de personajes.
- Las razas y las deidades del motor (RAZAS-02, DIOSES-01) **no** se han subido: no eran parte del
  encargo. Reconvertir desde el creador con `convertir_libreria_dnd.py` las seguiría pisando.
- `MotorGraphico-main/GDD_MG.md` (Manual Sagrat) queda superado por el GDD Ed. 4: falta marcarlo
  como histórico (no se ha tocado: es zona del motor).
