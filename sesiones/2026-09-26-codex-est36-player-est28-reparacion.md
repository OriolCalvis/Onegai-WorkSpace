# EST-36 Player y reparación del relevo EST-28

Fecha: 26/09/2026. Autor: Codex, con subagente para selección, pruebas y diagnóstico.
Commits MotorGraphico: 46dda54 (reserva), f1fbb40 (índice) y e4d427d (Player).

## Coordinación

Encargo: continuar el Gantt con MAPA_DEL_CODIGO.md y agentes simultáneos.
Leídos Biblia (protocolo, propiedad, trampas), TABLERO, handoff, mapa y estabilización.
Hooks ya activos. Se reclamó EST-36 Player, libre tras 8e12709, antes de crear ficheros.
La reserva inicial no pudo commitearse: el hook detectó cuatro métodos ProjectIndex
duplicados en HEAD. EST-28 figuraba terminado, ambas fuentes estaban limpias y se
reclamó una reparación acotada antes de modificarlo. La reserva se versionó cuando
la guardia rápida volvió a pasar; no se saltó el hook.

Se conservaron los cambios ajenos de Application, GameSession, BattleState,
editores, entrada, SaveGame, AdventureValidation, herramientas, arte y CMake.
Los commits usan git-safe e índice temporal. El CMake de f1fbb40 se construyó
desde HEAD + una línea; el índice real se sincronizó sólo para nuestras rutas.
No se hizo push. Las reservas propias quedan liberadas tras este registro.

## Player

Los ocho campos privados se movieron a PlayerData (header de 32 líneas, sin GL);
Player queda con un colaborador y 261 líneas. La ficha sigue siendo externa no
propietaria, los PV actuales siguen siendo propios, y los setters legacy no
modifican la ficha. No cambian reglas, flujo ni transiciones que requieran nuevos logs.

Cinco pruebas nuevas: defaults de datos, aislamiento de instancias, lectura viva
de ficha, HP parcial al conectar/desconectar y setters legacy con límites.
Comparación mecánica del código con HEAD: métodos equivalentes después de sustituir
únicamente nombres de acceso e ignorar comentarios. Línea base: se retira Player.h
de estado (4 entradas reales -> 3; el total previo 6 estaba desfasado).
G-12 recibe las rutas de Player en pla/dades.py. MAPA_DEL_CODIGO.md se regenera
desde esa fuente e inventario real, de ahí las actualizaciones adicionales de
referencias y lista de ficheros sin paquete.

## ProjectIndex

d31b91a había añadido ProjectIndexDuplicate.cpp sin quitar cuatro métodos del origen
ni enlazar la fuente. Además su ayudante escribía _indice.json/projects en vez del
index.json/proyectos original. Al enlazarlo, el test existente
duplicar_deja_la_copia_registrada_y_reabrible falla: 12/13 casos.
Se preservaron esc/escribeIndice originales, verificados byte a byte, y se retiró
el bloque E-11 duplicado. Los cuatro métodos mantienen sus cuerpos originales.
ProjectIndex.cpp baja 682 -> 402; la fuente extraída mide 359. motor_project añade
sólo su línea. La línea base retira este cuello (16 -> 15).

## Verificación

Build aislada /private/tmp/motor-est36-player-build con dependencias locales y sin red.
- Player 40/40, populate 5/5, inventory_menu 9/9, sheet_equipment_cap 5/5: 4/4 CTest.
- project_naming 13/13 después de corregir el escritor; project_levels,
  project_naming, project_manifests, project y project_serialization: 5/5 CTest.
- motor_editor y level_editor compilan y enlazan.
- medir_alcance.py --comprobar, JSON temporal: PlayerData llega a juego;
  ambas fuentes ProjectIndex llegan a los tres ejecutables.
- Guardia rápida y git diff --check verdes; índice real sin cambios preparados.

## Límites ajenos medidos

El enlace de juego falla por vtable de RPG::SessionCommandHost desde
ApplicationNivel.cpp, en el cableado WIP de eventos. No se tocó ese flujo.
La guardia completa tiene 0 fallos instantáneos y 1 deuda ajena:
Application.h sube 104 -> 107 miembros. No se elevó esa línea base.
La foto compartida alcance_real.json permanece intacta; la medición nueva va a /tmp.
No se afirma humo del juego ni cierre global de la estabilización ni CI remota.
