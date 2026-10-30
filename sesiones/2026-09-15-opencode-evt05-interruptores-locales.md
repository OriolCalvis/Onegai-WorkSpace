# Sesión 2026-09-15 — opencode: EVT-05 interruptores locales por evento

## Resumen
Implementación completa de EVT-05 (interruptores locales por evento) según el Gantt.

## Cambios implementados

### 1. Core Engine
- **EventConditionKind::LocalSwitch** añadido a `MapEventCondition` (`include/Map/EventLayerDocument.h`)
- **EvaluateCondition**: evalúa interruptores locales con formato `"eventId.letra"` (A-D)
- **ConditionFromIf**: parsea clase `"local_switch"` en comandos `control.si`

### 2. Comandos
- **interruptor.fijar**: cableado en `SessionCommandHost::execute()`
  - Parámetros: `evento` (id), `letra` (A-D), `valor` (true/false/1/0/on/off)
  - Valida formato y delega a `GameSession::setLocalSwitch()`

### 3. Validación
- **AdventureValidation**: valida condiciones LocalSwitch
  - Formato correcto: `eventId.letra` (A-D)
  - Evento referenciado existe en el mapa
- **ValidateCommand**: valida parámetros de `interruptor.fijar`

### 3. Persistencia
- **ProjectSerializer**: serializa/deserializa `EventConditionKind::LocalSwitch`
- Round-trip test verifica ida y vuelta JSON

### 4. Tests (15 nuevos)
- **EventCommandsTests.cpp**: 7 tests (`evt05_*`)
  - Condición: true/false, negado, formato inválido, evento inexistente
  - Comando: `interruptor.fijar` end-to-end
  - Round-trip serialización
- **EventInterpreterTests.cpp**: 8 tests (`evt05_*`)
  - Evaluación condición, comando, formato inválido, evento inexistente

## Verificación
- 114/114 tests GL-free passing (pre-existente: rpg_declaration_check falla por AssetBrowserState WIP ajeno)
- Tests nuevos: 15/15 passing
- Commit: `4720b62`

## Archivos modificados
- `include/Map/EventLayerDocument.h`
- `include/RPG/EventCommand.h`
- `include/RPG/SessionCommandHost.h`
- `include/RPG/NarrativeEngine.h`
- `src/RPG/EventCommands.cpp`
- `src/RPG/SessionCommandHost.cpp`
- `src/RPG/AdventureValidation.cpp`
- `src/Project/ProjectSerializer.cpp`
- `tests/RPG/EventCommandsTests.cpp` (+7 tests)
- `tests/RPG/EventInterpreterTests.cpp` (+8 tests)
- `cmake/ObjectAndLootCatalogTests.cmake` (link fix)

## Zonas respetadas
- codex: E-13 (AssetBrowser), E-12 (LayerPanel), E-10 (shell)
- zcode: C-16 (Application), F1 (timers), D-02/D-04
- qwen: D-01, G-08, tutorial
- Solo toqué: EventLayerDocument, EventCommands, SessionCommandHost, AdventureValidation, ProjectSerializer, tests relacionados