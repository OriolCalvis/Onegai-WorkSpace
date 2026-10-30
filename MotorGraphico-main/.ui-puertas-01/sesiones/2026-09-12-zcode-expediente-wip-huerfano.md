# 12/09/2026 (2ª tanda) — Expediente WIP huérfano: 7 paquetes hechos sin commitear + detector

- **Commits:** mi detector y mi expediente quedaron versionados DENTRO de `68f992c` (commit D-05/DEC-05 de codex) tras una carrera de git — ver «Incidente». Mi commit accidental `a35d157` fue deshecho con `reset --soft` antes.
- **Deja esperando a Oriol:** la fila de la bandeja ampliada — autorización para ejecutar el rescate de los SIETE paquetes huérfanos siguiendo el expediente.

## Qué se encontró

Reclamé D-03 de nuevo... y descubrí que TODO el corte B libre está fantasmaado. El inventario final con autoría cierta (los propios autores dejaron `seguimiento/*.json`): **Claude (Cowork)** (C-15/C-17/C-18/E-01/E-04/E-07), **TRAE (Cowork)** (D-03/E-02/E-03) y **opencode** (D-08/D-09) tienen paquetes «hechos» cuyo código sigue sin commitear — dos familias de agentes nuevas que no usan TABLERO ni commit. Verificación por símbolo contra HEAD (no por grep de mensajes: mi propio D-04 nombra E-01 en prosa, y `05edf18` versionó C-13/C-15 sin citarlos): los huérfanos CIERTOS son **D-03, D-05-loaders, D-08, E-01, E-02, E-03, E-04 + la rodaja de E-07**. C-04/C-07/C-08/C-12/C-13/C-15/C-18 son falsos positivos (versionados por rescates sin nombrar ids).

## Qué se entregó

1. **`tools/detectar_wip_huerfano.py`** — triage reproducible: cruza seguimientos × `MAPA_DEL_CODIGO.md` × `git status` × TABLERO. Nivel 1 (ficheros nuevos sin versionar, seguro) / nivel 2 (compartidos reformados, con anotación del commit que nombra el id y la advertencia de por qué el grep solo anota). Ejecutado y afinado con dos iteraciones (bug de doble escape mío corregido).
2. **`planificacion/seguimiento/EXPEDIENTE-WIP-2026-09-12.md`** — los 7 verificados, qué falta para cerrar cada uno, exclusiones de las sesiones vivas (codex D-05, opencode D-11, `PlayerPopulateTests` de origen desconocido que hoy rompe el build), orden de rescate propuesto por dependencias (D-03+D-05-loaders → E-02/E-03 → E-01 (arreglando el link de su test, que hoy NO enlaza) → E-04/E-07 → D-08) y huellas SHA-256 de los 17 ficheros clave, patrón del `C-11-integracion.md`.

## Incidente: carrera de git con codex (y cómo se resolvió)

Mi commit `a35d157` arrastró 11 ficheros de codex que estaban **ya stageados** en el índice (su D-05 en validación): `git-safe` serializa operaciones, no transacciones — la trampa clásica de la bitácora. Lo deshice (`reset --soft` + `restore --staged` de los 10 suyos), pero en la ventana abierta la sesión de codex tomó el lock y commiteó `68f992c` («Se preserva el trabajo concurrente», 68/68 gl-free verificados) — incluyendo mis dos ficheros. Resultado final: trabajo de ambos versionado, nada perdido, nada reescrito. **Lección para el registro:** antes de `commit`, mirar `git diff --cached` completo aunque solo se haya hecho `add` de lo propio — el índice es compartido y puede traer stageado ajeno de una sesión interrumpida.

## Estado del árbol combinado (verificado)

Build: 2 roturas ajenas a los huérfanos — `test_event_layer_document` no enlaza (E-01: su cmake no linka la lib de Project) y `test_player_populate` no compila (sesión viva de origen desconocido). Ayer el mismo árbol dio content 28/28.

## Próximo paso (bloqueado en la bandeja)

Autorización de Oriol → ejecutar el rescate en el orden del expediente, un commit por paquete con SU test en el mismo commit, excluyendo siempre los ficheros de sesiones vivas. Con eso, el corte B pasa de «7 paquetes en el limbo» a «7 paquetes versionados» y las fases E/F quedan desbloqueadas de verdad.
