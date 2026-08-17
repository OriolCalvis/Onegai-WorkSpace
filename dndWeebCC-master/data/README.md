# Dades del projecte

`data/` és la font de veritat d'execució del joc. L'aplicació no usa una base de dades relacional: els repositoris Java carreguen i guarden JSON directament des d'aquest directori.

## Carpetes principals

| Carpeta | Contingut | Consum principal |
|---|---|---|
| `cartas/clases` | Classes jugables i progressió per tiers | `TierClassRepository`, `PersonatgeService` |
| `cartas/razas` | Races i bons base | `TierRaceRepository`, `PersonatgeService` |
| `cartas/transfondos` | Trasfons narratius i mecànics | `TierBackgroundRepository` |
| `cartas/habilidades` | Habilitats de classe o generals | `TierSkillRepository`, `PersonatgeService` |
| `cartas/hechizos` | Encanteris i divines | `TierSpellRepository`, `PersonatgeService` |
| `cartas/armas` | Equip, armes, armadures i objectes equipables | `TierEquipmentRepository` |
| `cartas/dotes` | Talents i millores | `TierFeatRepository` |
| `cartas/pasivas` | Passives de classe o build | `TierPassiveRepository` |
| `cartas/consumibles` | Consumibles | `TierConsumableRepository` |
| `cartas/rasgos` | Trets especials | `TierSpecialTraitRepository` |
| `cartas/invocaciones` | Invocacions | `TierSummonRepository` |
| `cartas/deidades` | Deitats | `TierDeityRepository` |
| `cartas/condiciones` | Condicions i estats | `TierConditionRepository` |
| `cartas/enemigos` | Enemics regulars, elits i vilans | `CatalogoAventuraRepository`, `MonstruoController` |
| `cartas/trampas` | Trampes | `CatalogoAventuraRepository`, `TesoroController`/aventures |
| `aventuras` | Aventures heretades i aventures per actes | `AventuraRepository`, `ConstructorAventuraService` |
| `historias` | Cartes d'història i escenes | `HistoriaRepository`, `AventuraService` |
| `npcs` | PNJs i repartiment | `CatalogoAventuraRepository`, `NpcController` |
| `loot` | Taules de botí | `CatalogoAventuraRepository`, `TesoroController` |
| `eventos` | Events del mon | `EventoRepository`, `MapaController` |
| `mapa` | Geografia, regions i punts del mapa mundi | `GeografiaMapaService` |
| `personatges` | Personatges creats | `PersonatgeRepository` |
| `kits` | Kits o presets de creació | Scripts i fluxos auxiliars |

## Contractes pràctics

- Els ids han de ser estables, en `snake_case`, i no han de canviar si ja estan referenciats.
- Les referències entre fitxers es guarden com ids, no com còpies del contingut.
- Les aventures per actes poden conviure amb les aventures heretades: si `cartasHistoria` està buit, la pantalla antiga continua funcionant.
- Abans d'executar generadors massius, crea un commit o una branca de seguretat.
- Després de generar dades, revisa `/diagnostico` i executa `python3 scripts/auditar_campos_referencia.py`.
