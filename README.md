# Onegai WorkSpace

Monorepo de coordinación de **Egaroth / Onegai**. Reúne los cuatro componentes
del proyecto sin alterar la jerarquía de autoridad:

    canon (Onegai 2) -> cartas y mapa (dndWeebCC) -> ejecución (MotorGraphico)
                                              -> Onegai-Core (puente)

## Estructura

- Onegai 2/: corpus documental y canon. Es la fuente de verdad narrativa.
- dndWeebCC-master/: reglas, cartas y geometría del mundo.
- MotorGraphico-main/: motor C++ que ejecuta el contenido.
- Onegai-Core/: scripts e informes que conectan los componentes.
- EGAROTH_2000BF/ y world_grid.json: datos de coordinación del experimento
  distribuido del mundo.
- BITACORA_DEL_PROYECTO.md: registro obligatorio para agentes y colaboradores.

## Trabajo compartido

Antes de modificar contenido, lee AGENTS.md y la bitácora completa. Los
repositorios históricos de cada componente se han conservado localmente; esta
estructura publicada es una instantánea integrada y no copia sus directorios
.git, builds, caches, entornos ni material de terceros.
