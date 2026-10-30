# D0020 — Respuestas de Oriol a la bandeja (16/10/2026, 18:56)

Estado: **aceptada** (son decisiones del product owner). Origen: `BANDEJA_PRODUCT_OWNER.md`, movidas a «Resuelto».

| PO | Decisión | Dónde se aplica |
|---|---|---|
| PO-50 | Oficios: exp 1/2/3 por grado + 2 la primera vez que se hace una receta; herrería: tope por tipo de efecto = la mitad del tope del tier (1/1/2/4/8); nivel por tier: 1 Aprendiz, 2 Oficial, 3 Maestro, 4–5 Gran maestro. | `claude/oficios/00_reglas_y_materiales.md`, `herreria.md`; datos en `oficios.json` (ChatGPT) |
| PO-51 | Arquitectura de entidades por el **camino B** (composición: transitabilidad, paso visual, rumbo, `ActorAnimado`; `uso` + `VehiculoData` en el catálogo), en los 4 pasos del documento. Sin código hasta aprobar el paso 0. | `claude/arquitectura_entidades.md` |
| PO-45 | Las deidades que faltaban están escritas por Oriol como Caminatas en el vault (Chronos, Guerra, Morfeo, Muerte, Sofía, Vida, Deithrok, Deseo y Eros, Egos, Envidia, Forja). Las plegarias cuelgan de ellas. | `claude/plegarias_creadores.md`; `prayerSkillIds` en el motor |
| PO-46 | Una sola vida máxima y una vida actual por monstruo (unificar `maxHealth`/`derived.vida`). Los ataques de monstruo funcionan como los de un PJ o PNJ: físicos derivados de DES (ligero) o CON (pesado); cartas/habilidades con los dados que indica la carta. | AV18-06 / AV12-04 |
| PO-47 | Sí a los dos validadores de puertas. | `tools/validar_puertas.py` + `validar_enlaces.py` |
| PO-42 (matiz) | Reventa de negocios al 50 %; si se ha mejorado, 50 % + muebles + mejoras. | `Game::NegocioEstado` (cuando existan mejoras) |
| PO-44 | Abrir un callejón recto de la plaza a la iglesia. Ya cubierto por AV16-09 (iglesia 40 % → 0 %). | `gen_ciudad.py` |
| PO-33/34/37/38 | Confirmadas como en D0018. | — |

Nuevo en la bandeja: **PO-52** (botín de los días 2 y 3: `requisitos.dia` y sin cadáveres-contenedor), de `claude/plan_av18_cierre.md`.
