# Contrato de identificadores

S1 del `ROADMAP_RPG_EDITOR.md`: los IDs son datos de contrato. Este
documento fija las reglas; `tools/renombrar_id.py` ejecuta la única forma
legítima de cambiar uno.

## 1. Forma

1. **ASCII en minúsculas con guiones bajos**: `[a-z0-9_]+`. Sin tildes,
   sin espacios, sin mayúsculas. Los ids acaban siendo nombres de fichero,
   claves JSON y argumentos de herramientas.
2. **Estables**: una vez que algo referencia un id, el id no cambia. Si
   hay que cambiarlo, SOLO con `tools/renombrar_id.py`, que migra las
   referencias en el mismo paso. Renombrar a mano deja referencias
   muertas que no fallan al cargar: objetos invisibles, puertas que no
   llevan a nadie, beats que nunca se disparan.
3. **Únicos dentro de su namespace** (ver §3). Dos catálogos del mismo
   namespace con el mismo id no son un error de carga:
   `ObjectCatalog::loadFromString` hace merge y el último fichero gana
   en silencio.

## 2. Prefijos: el dueño se declara

| Prefijo | Propietario |
|---|---|
| (sin prefijo) | motor base y Boundington (`ciudad_centro`, `interior_taberna`, `pocion`…) |
| `on_` | cuadrante Oeste Norte |
| `os_` | cuadrante Oeste Sur (reservado) |
| `en_` | cuadrante Este Norte |
| `es_` | cuadrante Este Sur |
| `mundi_` | mapamundis generados desde la rejilla canónica |
| `editor_`, `test_`, `ejemplo_` | fixtures y material de prueba del editor |
| `itx_` | material de la campaña de Itx (contenido de ejemplo, ver bitácora 2.2) |

Reglas:

- No crees contenido con prefijo ajeno. Si un nivel de tu cuadrante
  necesita un objeto genérico existente, referéncialo; no lo dupliques
  con tu prefijo.
- Los manifiestos de proyecto (`assets/proyectos/<id>.json`) llevan el
  prefijo del pack en su campo `prefijo` (debe acabar en `_`, o ser
  vacío). `ProjectIndex::create()` ya lo hace cumplir.
- Un prefijo nuevo nace aquí, escrito, antes de usarse.

## 3. Namespaces

Cada familia de ids vive en su propio universo de referencias; el
validador (`tools/validar_todo.py`) los cruza por separado:

| Namespace | Dónde vive | Quién lo referencia |
|---|---|---|
| Objeto (`ObjectDefinition.id`) | `assets/objects/*.json` | `ObjectSpawn.objectId` en niveles, `ShopItem.objectId`, beats `talk.target` y `startBattle.monsterId` |
| Nivel | `assets/levels/*.json` (nombre de fichero) | `LevelDefinition.map` (ruta), `ObjectSpawn.targetLevel`, manifiestos `niveles` |
| Aventura | `assets/adventures/*.json` | manifiestos `aventuras`, `entrada` |
| Skill/hechizo | `assets/catalogs/skills.json` + `spells.json` (namespace COMPARTIDO) | `startingCards.*`, `linkedSkill`, `skillCheck.skillId`, `deities.grantedSpells` |
| Catálogos RPG por tipo | `assets/catalogs/<tipo>.json` | referencias internas (ver AUDITORIA_CONTRATOS.md §1.5) |
| Flag narrativa | no existe en disco: la inventa el contenido | `requires`, `doneFlag`, flags de `skillCheck`/`startBattle` |
| Proyecto | `assets/proyectos/index.json` | ids `[a-z0-9_]`; `ProjectIndex::create()` los valida |

## 4. Renombrar

```bash
python3 tools/renombrar_id.py pocion pocion_curativa            # dry-run: lista
python3 tools/renombrar_id.py pocion pocion_curativa --aplicar  # escribe
python3 tools/validar_todo.py                                   # verifica después
```

El script solo toca el namespace de objetos (S1); los demás namespaces se
añadirán cuando haga falta. Antes de aplicar comprueba que el id nuevo no
existe ya, y después de escribir re-parsea cada fichero para confirmar que
el JSON sigue siendo válido.
