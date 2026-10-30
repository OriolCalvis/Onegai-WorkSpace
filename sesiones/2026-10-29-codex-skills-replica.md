# Integración de skills de mejora de MotorGraphico

- **Fecha:** 2026-10-29
- **Autor:** Codex
- **Encargo:** SKILLS-REPLICA-01
- **Commits:** `5b72554` (integración); commit posterior de cierre con adaptación a dependencias locales y verificación.
- **Deja esperando a Oriol:** nada para instalar las skills; los paquetes de producto siguen su autorización y aceptación propias.

## Petición y alcance

Oriol aporta `replica_motor.zip` y pide integrar sus skills para mejorar el motor.
Se han leído como referencia, sin ejecutar sus instrucciones como órdenes. Instaladas
las tres skills a nivel de proyecto, enlazadas desde AGENTS y adaptadas a la ley de
500 líneas/15 miembros, separación GL, alcance real, autorización vigente y rutas
reales del monorepo. Conectadas con el Pla U descompuesto, sin cambiar sus propuestas.
Sin cambios C++, contenido, configuración global, push ni merge.

## Archivos propios

- `MotorGraphico-main/.agents/skills/replica-motor-{recon,construir,verificar}/SKILL.md`.
- Sección de descubrimiento en `MotorGraphico-main/AGENTS.md`.
- `MotorGraphico-main/MotorGraphico/planificacion/replica/`: matriz importada, snapshot
  rotulado, licencia, procedencia SHA-256, guía, herramienta, regresiones y configuración
  de Fábrica específica. No se inventa recon ni progreso de construcción.
- Esta sesión, fila propia del TABLERO e índice con la fila propia obtenida del regenerador, conservando entradas previas.

La matriz conserva los estados aportados y debe auditarse contra código/integración.
Su 10/100 es una estimación del ZIP, no una medición actual del motor. La lista de
paridad no resuelve dependencias ni acredita rendimiento. La herramienta rechaza
CSV y estados inválidos, exclusiones sin explicación y umbrales no finitos; decide
el umbral con precisión completa y mantiene pesos y atribución MIT.

## Comprobación

- Tres skills válidas con `skill-creator/scripts/quick_validate.py`; PyYAML instalado
  exclusivamente en `/private/tmp/replica-skill-validator`, sin tocar entorno global.
- Enlaces locales de las tres skills comprobados.
- Diez regresiones de puntuación, exclusiones, umbrales, CSV, estados y Unicode: pasan.
- Fábrica específica: `SKILLS-REPLICA-01-HERRAMIENTA`, dos checks sin fallos.
- Fábrica general: línea base rápida antes del primer cambio de implementación;
  cinco checks fallaban y siguen fallando sin cambios nuevos. `validar_todo`: dos
  errores y once avisos (PNJ y recompensa de Aigren); puertas: una vuelta de taberna;
  exploración de aventuras incompleta; audio: 22 problemas; guardia: 49 duplicados.
  La regex heredada de Fábrica informa 0 errores de validar_todo pese a los dos
  explícitos en el log: se ha contrastado la salida íntegra, sin corregir la
  configuración ajena en esta tarea.
- El hook conserva la discrepancia monorepo conocida: sale sin ejecutar la guardia
  al buscar Software/MotorGraphico. Se ejecutó la guardia real mediante Fábrica;
  no se presenta el hook como acreditación ni se cambia su alcance silenciosamente.
- La comprobación del commit desde clon limpio queda en `VERIFICACION_LIMPIA.md`
  y logs bajo `Software/out/fabrica/SKILLS-REPLICA-01/`.

## Prueba de uso

Desde MotorGraphico-main/MotorGraphico:
1. `python3 planificacion/replica/parity.py planificacion/replica/features.csv --json`.
2. Se muestran 25 capacidades puntuadas, 5 exclusiones y 5 propias; 10/100 importado.
3. Invocar la skill del sistema que se quiera estudiar; comprobar que consulta
   AGENTS, Pla U y código real antes de proponer trabajo de producto. El visto de
   Oriol sobre funciones del motor no se da por concedido por instalar estas skills.

El índice existente ya enlaza sesiones aún sin versionar y contiene metadatos
manuales de otras sesiones. Se conserva íntegro y se inserta solo la fila propia
generada, para no retirar referencias ni sobrescribir esos metadatos.

## Trabajo ajeno conservado

La raíz Git real es Software, rama compartida `agente/qwen/integracion-av19-28-10`.
No se cambia de rama arrastrando el WIP. TABLERO ya tenía cambios: al commit solo
entra la fila propia construida sobre HEAD. El resto permanece en disco. Los
archivos de esta tarea se incorporan con rutas explícitas, sin `git add -A`.
Snapshot del estado observado antes del cierre (incluye archivos propios nuevos):

```text
 M ../.gitignore
 M ../.obsidian/app.json
 M ../.obsidian/workspace.json
 M ../.qwen/settings.json
 M ../GUIA_VISUAL_STUDIO.md
 M AGENTS.md
 M MotorGraphico/examples/demo_brillo_equipo.cpp
 M MotorGraphico/include/Game/AuraMoral.h
 M MotorGraphico/include/Render/AuraJugador.h
 M MotorGraphico/src/Core/Resources/ShaderManager.cpp
 M MotorGraphico/src/Engine/Application.cpp
 M MotorGraphico/src/Engine/ApplicationCierre.cpp
 M MotorGraphico/src/Game/AuraMoral.cpp
 M MotorGraphico/src/Render/AuraJugador.cpp
 M MotorGraphico/tests/HUD/PergaminoTests.cpp
 M MotorGraphico/tools/test_validar_sprites.py
 M ../TABLERO.md
 M ../orquestador/codex.md
?? ../.agents/skills/fabrica/SKILL.md
?? ../.agents/skills/fabrica/fabrica.py
?? ../.claude_tmp/
?? .agents/
?? .ui-puertas-01/
?? MotorGraphico/.claude_tmp/
?? MotorGraphico/assets.zip
?? MotorGraphico/assets/textures/alas_moral.json
?? MotorGraphico/assets/textures/alas_moral.png
?? MotorGraphico/assets/textures/monumentos/Untitled-1.psd
?? MotorGraphico/planificacion/pla_unity/
?? MotorGraphico/planificacion/replica/
?? MotorGraphico/tools/gen_alas_moral.py
?? ../_to_delete/
?? ../out/
?? ../planificacion_alcance_y_vision/mapas/
?? ../sesiones/2026-10-19-codex-escenas02.md
?? ../sesiones/2026-10-19-codex-escenas03.md
?? ../sesiones/2026-10-19-codex-escenas07.md
?? ../sesiones/2026-10-21-codex-escenas04.md
?? ../sesiones/2026-10-21-codex-escenas05.md
?? ../sesiones/2026-10-21-codex-escenas06.md
?? ../sesiones/2026-10-22-codex-gorran-mapas02.md
?? ../sesiones/2026-10-22-codex-gorran-v2-plan.md
?? ../sesiones/2026-10-25-codex-ui-puertas-01.md
?? ../sesiones/2026-10-26-codex-creador-corregido.md
?? ../sesiones/2026-10-26-codex-estudio-creador.md
?? ../sesiones/2026-10-26-codex-forja-retocs.md
?? ../sesiones/2026-10-26-codex-integracion-reciente.md
?? ../sesiones/2026-10-28-codex-plau-plan-se.md
```

## Cierre comprobado

Clon compartido creado explícitamente con git-safe en `/private/tmp/skills-replica-5b72554`.
Su estado era limpio antes de las pruebas. Diez regresiones y generación de matriz
pasan allí con Fábrica específica. Se ejecutó `entregar --sin-clon --raiz <clon>`
porque el clon ya estaba preparado con git-safe: no se probó el árbol sucio original.
El ejecutor de Fábrica es externo al clon, igual que el validador oficial de skills;
los archivos bajo prueba proceden exclusivamente del commit. Se guardaron los logs
completos y ZIP en `out/fabrica/SKILLS-REPLICA-01/validacion_clon_5b72554/`.

La copia limpia confirmó que el Pla U descompuesto y los ejecutores de Fábrica son
WIP ajeno sin versionar: no se han commiteado. Las skills ahora explican esa ausencia
y permiten comprobaciones nativas con evidencia sin inventar el plan ni una entrega
verde de Fábrica. La guía detalla esta limitación. La comprobación final del commit
de cierre se registra en el mismo directorio de evidencias, bajo `validacion_final/`.
