# EGAROTH — Experimento 4 IAs paralelas (año 2000 b.f. / ERA XIII)

> **Referencia canónica del experimento:** [BITACORA_DEL_PROYECTO.md §2.16](file:///Users/admin/Documents/Documentos%20-%20Oriol%20Os%20(2)/Software/BITACORA_DEL_PROYECTO.md#L337-L368).
>
> **Objetivo:** 4 IAs distintas, 4 regiones, 1 mundo compartido — ensamblable después SIN conflictos.
> **Motivo del año 2000 b.f.:** fecha del mapa canónico (§1.3 de Bitácora). Todas las regiones son el "antes" de las Cruzadas que empiezan justo en esta época.

---

## 1 — Reparto (ciudades ~ balanceado 95/4 ≈ 24)

| Carpeta | IA | Región | Naciones | Ciudades |
|---|---|---|---|---|
| [01_CLOUDE_OUEST_NORD](./01_CLOUDE_OUEST_NORD) | **CLOUDE** | Occidente Septentrional | Ascaria (capital Ascaris), Aegroum, Bastrea | ~24 |
| [02_TRAE_OUEST_SUD](./02_TRAE_OUEST_SUD) | **TRAE** | Occidente Meridional | **Ecla, Udrax** | ~24 |
| [03_ZCODE_EST_NORD](./03_ZCODE_EST_NORD) | **ZCODE** | Oriente Septentrional | Choubar, Bosmurg, Esmua | ~24 |
| [04_CHATGPT_EST_SUD](./04_CHATGPT_EST_SUD) | **CHATGPT** | Oriente Meridional | Tabaxi, Gongorguma | ~24 |

> **⚠️ FRONTERAS ADYACENTES** (prueba de continuidad — hablad que haya consistencia):
> - CLOUDE ↔ TRAE: frontera **Ascaria-sur / Ecla-norte** (pasos de montaña, ríos, 3 fortalezas Cruzadas compartidas).
> - TRAE ↔ CHATGPT: **Udrax-sur archipiélago → mar de los Mil Dones hacia Tabaxi-este**.
> - ZCODE ↔ CHATGPT: **Bosmurg-sur / Gongorguma-norte**.
> - CLOUDE ↔ ZCODE: **Aegroum-Este Mar Interior ↔ Choubar Oeste**.

---

## 2 — Contrato OBLIGATORIO (mismas reglas Bitácora §2.16)

### 2.1 Fecha fija
Todo es **2000 b.f. — ERA XIII (Inicio de las Catorce Grandes Cruzadas)**.
- Puedes referenciar eventos hasta el 2005 inclusive (EV-1995 Asalto a Nubes = 5 años después es el límite).
- Nada de eventos 1500, 1000, 500 b.f. ni era actual.

### 2.2 Coordenadas (rejilla world_grid.json 200×150)
- Si la ciudad **ya existe en el Atlas** (fichero `canon/locations_atlas.csv` que genera este script), su `{x,y}` es CANÓNICA y NO se modifica.
- Si la ciudad era `Aldea IX` o `Capital I` → ahora la inventas tú, pero elige (x,y) que estén **libres** (más de 5 casillas de cualquier ciudad ocupada).
- Campo nuevo `origin: "TRAE"` / `"CLOUDE"` / etc. Marca quién la creó.

### 2.3 Tier (amenaza)
Cada nación/ciudad lleva `tierRange.min / max` (1..5, como TierRules.h):
- Frontera Cruzadas / bastiones: min≥3.
- Capital: tier=5 (amenaza + cultura + logística).
- Aldea rural: 1..2.
- Mazmorra cercana: `dungeon.tier` coherente (no hay tier 5 en aldea minera tier 1).

### 2.4 Nivel 2 MÍNIMO por NACIÓN
1. Campo `kind="nation"`, `name`, `id`, `capitalId`.
2. Gobierno (monarquía, república, teocracia, tribu…).
3. Bioma general + `regions[]` (zonas del Atlas que cubre).
4. `eraXIIIEvents[]` al menos 1 evento histórico (batalla, tratado, partida Cruzada).
5. `factions[]` ≥ 3 (nobleza, ejército, iglesia, gremio, clan, asamblea…).
6. `dungeons[]` ≥ 1 mazmorra por nación: `{ id, name, tier, coord, theme }`.
7. Lore de 3 párrafos: (a) fundación / mito, (b) estado militar Cruzadas 2000, (c) economía.

### 2.5 Nivel 2 MÍNIMO por CIUDAD
1. `id, name, kind="city", nationId, coord:{x,y}`.
2. `population` (estimación: capital 25k, gran ciudad 8k, pueblo 1.2k, aldea 300).
3. `primaryActivity`: (minería, puerto, plaza fuerte, agrícola, santuario, universidad…).
4. `faction`: facción que controla el día a día.
5. `tier`: amenaza local (1..5).
6. `pnjPrincipal`: { name, race (de las 43 existentes), classId (de las 61), role, hook }.
7. `quests[]` ≥ 1 hook simple: string con suiciente lore para GDD (sin guión completo).

### 2.6 Entrega por IA (5 ficheros / carpeta)
| Fichero | Tipo | Descripción |
|---|---|---|
| `00_[IA]_[REGION]_README.md` | MD | Resumen lore, mapa ASCII, datos clave, eventos ERA XIII, tablas facciones/mazmorras |
| `05_naciones.json` | JSON | Array `LocationDefinition[]` con `kind: nation` |
| `10_ciudades.json` | JSON | Array `LocationDefinition[]` con `kind: city` |
| `15_zona_mazmorras.json` | JSON | Array zonas + mazmorras (`kind: zone`) |
| `20_catalogo_objetos_unicos.json` | JSON | OPCIONAL. ≥5 objetos únicos de la zona compatibles EquipmentCatalog schema |

### 2.7 Prohibido
- ❌ Inventar dioses nuevos. Usa los 17 existentes (`RPG::Catalogs::DeityCatalog` → 10 deidades principales + 7 locales). Si falta alguna, marcar **`[TBD Oriol]`**.
- ❌ Inventar razas inteligentes nuevas (43 ya existen).
- ❌ Inventar clases nuevas (61 ya existen).
- ❌ Coordenadas que no estén en la rejilla 200×150.

---

## 3 — Schema validación (obligatorio pasar)

Schema JSON válido en: [schemas/location_level2.schema.json](./schemas/location_level2.schema.json).

Cualquier fichero de una IA debe cargar en este comando (desde `MotorGraphico/build`):
```bash
# Carga y muestra: naciones + ciudades
./demo_mundo <(cat 05_naciones.json 10_ciudades.json | jq -s 'add')
```

---

## 4 — Extracto canónico ERA XIII (leer antes de generar)

Ver [canon/ERA_XIII_2000BF_extracto.md](./canon/ERA_XIII_2000BF_extracto.md). Eventos clave:

| Año | Evento | Regiones |
|---|---|---|
| 2000 | Proclamación de las Catorce Cruzadas en Ascaria | C1-C2 CLOUDE |
| 2001 | Cruzados Occidentales invaden la franja de Ecla-norte | CLOUDE ↔ TRAE |
| 2003 | Batalla del Mar Rojo: Tabaxi bloquea flota Cruzada | CHATGPT |
| 2004 | El Ejército Verde de Bosmurg envía refuerzos a Choubar | ZCODE |
| 1995 | EV: Asalto a Nubes (último del periodo permitido) | Ascaria sur + Ecla frontera |

---

## 5 — Controles manuales al ensamblar

Cuando las 4 IAs entreguen sus entregables, se ejecutará:
1. `python3 tools/validar_regiones_4ias.py` → chequea coord únicas, ids únicos, naciones no solapadas.
2. `cat 0*/05_naciones.json > all_nations.json` y `demo_mundo` contra él.
3. Alineación manual de fronteras adyacentes (§1) si hay conflicto.
4. Volcado a `assets/catalogs/locations_complete.json` si todo ok.
