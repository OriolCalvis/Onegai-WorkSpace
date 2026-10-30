# 2026-10-08 · Claude (Cowork) · MORAL-01 + BND-ALT-01 — la moral que no se corrompe deprisa y el primer día con alternativas

Encargo de Oriol (08/10): «ahora plantea para cada escenario un par de alternativas que se conecten con la trama
principal [...] lo ideal sería parametrizar bien la moral, porque al final, si nos corrompemos muy rápido no tendría
gracia para nada». La propuesta fue el documento «Boundington: alternativas, karma y moral» (claude.ai); Oriol
contestó en él. Detalle de la sesión, pruebas y decisiones:
[`MotorGraphico/sesiones/2026-10-08-claude-moral-alt.md`](../MotorGraphico-main/MotorGraphico/sesiones/2026-10-08-claude-moral-alt.md).
Canon: `MotorGraphico/planificacion/CANON_BOUNDINGTON_CAMPANA01.md` §11.

- **Commits:** ninguno en el Mac; aplicado en el árbol de trabajo de MotorGraphico con preimagen (`out/claude_moral_alt01/`: 58 ficheros; `out/claude_alt02/`: 15; comprobados uno a uno).
- **Deja esperando a Oriol:** revisar lo que decidió Claude en el primer día (canon §11.2), sus textos (§11.3) y lo del segundo día (§11.5); ojo con los «éxitos a la letra»: con Carisma 3, 2 éxitos salen el 8,8 % y 4 éxitos nunca.
- **Sigue:** BND-ALT-03 (el Ocaso, con lo que era BND-ESCENAS-02). BND-ALT-02 (el segundo día) está hecho: 76 beats, el saqueador de Surysal y tres objetos nuevos (`assets/objects/boundington_objetos.json`).

| Ruta (`MotorGraphico/`) | Qué |
|---|---|
| `include/Game/Morality.h`, `src/Game/Morality.cpp` | La moral: libro del día, asentar al dormir con los seis números, siete tramos, pureza y marcas. |
| `include/Game/ReglasMoral.h`, `src/Game/ReglasMoral.cpp`, `assets/rules/moral.json`, `assets/rules/actos.json` | NUEVOS — los seis números y el catálogo de actos (58). |
| `include/Game/MoralDeLaSesion.h`, `src/Game/MoralDeLaSesion.cpp`, `GameSessionNarrativa.cpp` | NUEVOS — apuntar los actos de un beat y asentarlos en el descanso largo; la herida y los éxitos fijos. |
| `src/RPG/Narrative/MoralAventura.*`, `NarrativeBeat.h`, `NarrativeState.*`, `NarrativeEngine.*`, `BeatsAventura.cpp` | `acto`, `herida`, `requires.moral`, `skillCheck.exitos`. |
| `include/Game/AuraMoral.h`, `src/Game/AuraMoral.cpp`, `src/Render/AuraJugador.cpp` | El aura de los siete tramos, con alas en Héroe y Villano. |
| `assets/adventures/boundington_primer_dia.json` (+ segundo día y Ocaso: Rosita) | El primer día con las alternativas: 94 beats. |
| `examples/demo_boundington*.{cpp,h}`, `examples/demo_campana_boundington.cpp`, `tests/Game/MoralTests.cpp`, `cmake/MoralTests.cmake` | Las pruebas: ningún beat muerto; 12 casos de la moral. |
| `FORMATO_AVENTURAS.md`, `planificacion/CANON_BOUNDINGTON_CAMPANA01.md`, `planificacion/seguimiento/MORAL-01.json`, `BND-ALT-01.json` | La documentación. |

## Punto de guardado (08/10, tarde) — commit `11ff327` en `MotorGraphico-main`

Oriol: «revisa si podem commitejar la feina que hem desenvolupat aquests dies [...] hi ha algun element critic a
revisar abans?». La revisión y lo que se hizo:

- **Lo pendiente:** 4.824 rutas (451 MB) de unas veinte sesiones. Unos 370 MB eran salidas y basura que no entran:
  `Claude outputs/` (en la raíz y en `MotorGraphico/`), `_to_delete/` (un `.tar.gz` de 69 MB), `Testing/`,
  `terminal_logs/`, `cableado_test_objects_25.json` y `MotorGraphico/CMakeLists 2.txt`, una copia que dejó la
  sincronización el 05/10 (comparada con `CMakeLists.txt`: no se pierde nada). Sin secretos ni marcas de conflicto.
- **Comprobado sobre una copia exacta de lo que entra**, en la VM: compilan los 648 objetivos y pasan 221 de 222
  pruebas. Falla `linea_base_core`: 45 ficheros que no alcanza ningún ejecutable, deuda de otras sesiones (D0007); sigue
  fallando con `alcance_real.json` regenerado.
- **Lo nuestro no se podía commitear solo:** se apoya en trabajo de otras sesiones que tampoco estaba en HEAD, y un
  commit parcial dejaba HEAD sin compilar (el aviso del 01/10).
- **Oriol eligió el punto de guardado en la rama:** `11ff327` con 3.708 cambios (396 modificados, 4 borrados y 3.308
  nuevos), comprobado fichero a fichero contra la copia compilada (4.743 de 4.743 iguales). Aviso a todos en el TABLERO.
- **Sin push:** la rama va 225 commits por delante de `origin`, y el gancho pre-push lo veta mientras D0007 siga
  abierta. Lo hace Oriol: `git push --no-verify` para esta copia, o aceptar D0007 y actualizar la línea base.
- **Los locks:** el sandbox no puede borrar, así que `git-safe.sh` deja su lock y git deja `HEAD.lock`. Los dos
  renombrados: `.git/git-safe.lock.rancio.claude-guardado-*` y `.git/stale-HEAD-lock-claude-guardado-*`.
- **Fuera de este repo:** `Onegai 2` tiene 387 cambios sin commitear y ningún remoto; `dndWeebCC`, 1.698; y `Software/`
  no es un repo.
