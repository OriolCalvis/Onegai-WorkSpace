# Emblemas propuestos para Gorran: pendientes de Oriol

Origen: `sesiones/2026-10-25-codex-ui-puertas-01.md` (rama ui-puertas-01), marcados alli "pendiente de Oriol".
Estaban en `assets/catalogs/lugares.json`, lista `lugares`, pero eran salidas (`salir`) que el cargador (`src/Game/Lugares.cpp`) lee como `calles`.
Al moverlas a `calles` (H3, 02/11, decision de Oriol: "corregir y guardar los emblemas") se pierden `funcion`, `emblema` y `_propuesta_ui_puertas_01`, porque el esquema `calles` solo tiene `nivel`, `nombre`, `salir` (no hay `emblema` en `Calle`).
Se copian aqui literalmente. Si Oriol los acepta hay que darles un sitio real: o ampliar `Calle` con `emblema`, o ficha de `lugares` con `funcion` de 2 a 5 palabras (la prueba lo exige).

Los emblemas existen en el atlas? `lore` y `mapa`: comprobar con `assets/textures/iconos.json` antes de usarlos.

| nivel | campo | valor |
|---|---|---|
| assets/levels/antecova_gorran.json | funcion | entrada a las galerias |
| assets/levels/antecova_gorran.json | emblema | lore |
| assets/levels/antecova_gorran.json | _propuesta_ui_puertas_01 | Propuesta: libro de secretos para la boca de la cueva; pendiente de Oriol. |
| assets/levels/recinto_gorran.json | emblema | mapa |
| assets/levels/recinto_gorran.json | _propuesta_ui_puertas_01 | Propuesta: mapa para senalar los pasos subterraneos; pendiente de Oriol. |
| assets/levels/galerias_gorran.json | emblema | mapa |
| assets/levels/galerias_gorran.json | _propuesta_ui_puertas_01 | Propuesta: mapa para senalar los pasos subterraneos; pendiente de Oriol. |
| assets/levels/rio_gorran.json | emblema | mapa |
| assets/levels/rio_gorran.json | _propuesta_ui_puertas_01 | Propuesta: mapa para senalar los pasos subterraneos; pendiente de Oriol. |
| assets/levels/mina_gorran.json | emblema | mapa |
| assets/levels/mina_gorran.json | _propuesta_ui_puertas_01 | Propuesta: mapa para senalar los pasos subterraneos; pendiente de Oriol. |
| assets/levels/mina2_gorran.json | emblema | mapa |
| assets/levels/mina2_gorran.json | _propuesta_ui_puertas_01 | Propuesta: mapa para senalar los pasos subterraneos; pendiente de Oriol. |
| assets/levels/galeria_minas_cojo.json | emblema | mapa |
| assets/levels/galeria_minas_cojo.json | _propuesta_ui_puertas_01 | Propuesta: mapa para senalar los pasos subterraneos; pendiente de Oriol. |

Contexto de cada entrada movida (nombre / salir, sin cambios): la Gola Fosca / la galeria; el recinto del jefe, galerías de contrabando, el río subterráneo, la otra red minera, La mina vieja del Cojo, Las galerias de salida del Cojo / el paso.

Nota aparte: el diagnostico pedia anadir `afueras_ocaso` a `calles`, pero ese nivel no existe en esta rama (solo `afueras_boundington_ocaso.json`, que ya quedo en `calles` con su nombre "las Afueras, al ocaso"). No se ha anadido.

## Efecto colateral al mover antecova_gorran (H3)

`tests/HUD/CartelesPuertasTests.cpp:105` (`cartel_y_flecha_unicos_sin_contorno_negro`, en test_pergamino) usa `antecova_gorran` como puerta de ENTRADA con ficha en `lugares` (nombre "la Gola Fosca", funcion, emblema `lore`) y espera 2 hojas y 7 bandas de luz. Sin la ficha, `pintaCarteles` no pinta nada (0 hojas). Es la propuesta UI-PUERTAS-01 (cartel con emblema): depende de que ese nivel sea una ficha de `lugares`, y `test_lugares` exige que toda ficha sea `interior_*`.
Salidas posibles (decide Oriol): (1) mantener `antecova_gorran` en `lugares` y relajar la regla `interior_` de test_lugares para fichas con `funcion`/`emblema`; (2) apuntar la prueba del cartel a un interior real con emblema (p. ej. `interior_banco`, "La Caja Firme", emblema `oro`).

La prueba de carteles (`tests/HUD/CartelesPuertasTests.cpp`, `cartel_y_flecha_unicos_sin_contorno_negro`) usaba antecova_gorran; ahora usa interior_banco (La Caja Firme, emblema `oro`); si Oriol devuelve la ficha de Gorran, se puede volver.
