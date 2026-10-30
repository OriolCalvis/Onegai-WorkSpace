---
title: "INFORME — Refactor de descripcions (Era 0 → Era Oscura)"
type: informe
tags:
  - TML
  - informe
  - manteniment
year: "2026-08-28"
estat: obert
---

# 🔎 INFORME — Refactor de descripcions (Era 0 → Era Oscura)

**Abast:** ERA 0 (Edad de los Dioses) · ERES I–III (Despertar de Egaroth) · ERA IV · ERA V (Era Oscura)
**Fet:** 55 fitxes d'EVENTOS que no tenien descripció ara porten frontmatter complet i bloc `> [!abstract] Definició canònica`.
**No tocat:** PERSONATGES, LLOCS i `BORRADORES_IMPORTADOS_tmp`.

> [!warning] Com llegir això
> Cap fitxa s'ha reescrit: el text original es conserva sencer sota la capçalera nova.
> Les ⚠️ que trobis dins dels abstracts són **preguntes obertes**, no canvis de canon.

---

## ✅ Què s'ha escrit

| Bloc | Fitxes | Detall |
|---|---|---|
| ERA 0 — Edad de los Dioses | 6 | Egaroth, Chronos, Sofía, Vida y Muerte, Día de los Nombres, La gran migración |
| Elementales | 15 | Els 4 Primordials, Shorm, els 5 fills, Alianza, Primer Conflicto, Contexto |
| Dracónidos | 8 | Contexto, Pérdida de Himetsumota, Congreso, Ordre, Nivells, Aïllament, Rompeaislamiento |
| Elfos | 9 | Itron, Torneo, Pacte de les Ànimes, les 4 subraces, Defensa Costes Oest |
| Contextos de raça | 14 | Enanos, Goblin, H. Gato, Orcos, Minotauro, Yokai, H. Lagarto, H. del Este, Sagas, Nagas, els 3 humans de l'Oest, Aarakocra |
| ERA V | 3 | Acantilados Marinos, Caída de la Oscuridad, El Oráculo |

L'ERA IV ja tenia els seus 8 esdeveniments complets: **no s'ha tocat res**.

---

## 🧩 Incoherències detectades (per decidir tu)

### CRÍTIC — Índex de l'ERA V desincronitzat amb la reordenació

L'índex declara **9 entrades** però la carpeta `EVENTOS/` en té **12**.

| Problema | Detall |
|---|---|
| Fitxes fora de l'índex | `05 - BATALLA EN LOS ACANTILADOS MARINOS`, `09 - La caida de la oscuridad`, `11 - La Fundación del clan Aegro` |
| Enllaç creuat mal | L'índex, a la fila «La Fundación de Aegroum» (`EV-8450-AEGROUM`), enllaça la fitxa **`10 - El oraculo para un nuevo mundo`**. L'Aegroum real és la fitxa **`11`** |
| Numeració | Falta el `00` a l'índex com a tal (hi surt a la fila 08 amb data 8457) i els números d'ordre del disc no coincideixen amb la columna `#` |
| Rang de l'era | `09 - 8445 b.f.` cau **fora** del rang declarat 8487–8450 b.f. i és posterior a l'oracle de 8450, tot i portar número d'ordre anterior |

➡️ **Acció:** regenerar la taula de [[00 - ÍNDICE - ERA V - LA ERA OSCURA - LA ALIANZA DE ENVIDIA Y LAS ÚLTIMAS FUNDACIONES]] a partir dels fitxers reals i decidir si el rang de l'era s'allarga fins a 8445 b.f.

### ALT — Paternitat d'Anwen Aergan

[[Anwen Aergan, la Aullidos del Relámpago]] es declara filla de «Zephyrus, **el Espíritu del Trueno**», però:

- el Primordial de l'aire és [[Zephyrus, el Espíritu del Viento]], que ja és pare de [[Sylphera Aergan, la Danzarina de los Cielos.|Sylphera]];
- el cos d'Anwen és **de foc i vent**;
- [[Primer Conflicto]] diu que Shorm ofereix **un fill a cada clan**, i el clan de Terrus ja rep [[Orun Aergan, el Guardián de la Tierra Profunda|Orun]].

➡️ Per simetria, Anwen hauria de ser filla d'**Ignis**. Decideix-ho i corregeix les tres fitxes alhora.

### ALT — Noms dracònids: sembradors vs descendents

[[Contexto]] dels Dracónidos fa servir **Zorvhan la Voz del Fuego**, **Tharkos el Aliento Ardiente** i **Shayrith la Ceniza Eterna**, que segons [[00 - LLINATGES I FACCIONS - Dracónidos]] són els **descendents de 8735 b.f.**, no els sembradors de la Migració ([[Zorvhek Voz Primera]], [[Tharkir Aliento Primero]], [[Shayra la Primera Ceniza]]). El text del Contexto explica la **llegada**, que és anterior. Cal decidir si el Contexto es refereix a 8735 o si canvien els noms.

### MITJÀ — Dates i noms solts

| On | Problema |
|---|---|
| [[Contexto]] dels Hombres Pájaro | dona **8786 b.f.**; la Cronologia Mestra registra la Purga de las Tormentas a **8760 b.f.** (`EV-8760-PURGA`) |
| `Kaelis Aegran, el Heraldo de las Mareas..md` | el nom del fitxer diu **Aegran**; la forma canònica és **Aergan**. A més té doble punt abans de `.md` |
| `Nagas/EVENTOS/Context.md` | es diu `Context.md`, no `Contexto.md` com la resta de races |
| [[04 - TML - Nacimiento de Egos y Eros]] | el títol anuncia Egos i Eros, però el contingut és només el Día de los Nombres |
| [[La gran migracion]] | duplica `EV-9071-MIGRACION`, que ja viu a [[9071 b.f. - La Primera Gran Migración]] |

### MITJÀ — Enllaços trencats heretats

- [[03 - TML - VIDA Y MUERTE]] acaba enllaçant `[[05 - TML -  CAMBIOS EN EGAROTH -]]`, que no existeix. El text corresponent sí que existeix a `BORRADORES_IMPORTADOS_tmp/TML -  CAMBIOS EN EGAROTH - EL DÍA DE LOS NOMBRES.md`.
- `DEFENSA DE LAS COSTAS OESTES 8749 b.f..md` obre l'Era III segons la Cronologia Mestra però **estava buida**: ara té capçalera i ID, però el relat continua sense escriure.

### BAIX — YAML invàlid (previ, no tocat)

Tres fitxes de PERSONATGES de l'Era IV tenen el frontmatter mal format (llista sense clau, línia 8):
`PERSONATGES/Enanos/Ladron.md`, `PERSONATGES/Goblin/Professor.md`, `PERSONATGES/Goblin/Sailor.md`.

---

## 🤖 Pas per les dues IA locals

S'ha deixat un script al projecte per fer-hi passar les fitxes des del Mac amb Ollama arrancat:

```bash
CFG="$HOME/Documents/Onegai 2/.onegai_ai_config/scripts"

# 1) MODE CRÍTIC — revisió de canon, escriu un informe, no toca cap fitxa
python3 "$CFG/revisar-tml-descripciones.py" --critic --limit 5     # prova curta
python3 "$CFG/revisar-tml-descripciones.py" --critic               # tot

# 2) MODE ESCRIPTOR — proposa el desenvolupament dels apartats buits
python3 "$CFG/revisar-tml-descripciones.py" --writer --solo "ERA V"
```

Les sortides van a `.onegai_ai_config/salidas/TML_<mode>_<data>.md`.
Cap dels dos modes escriu dins del vault: tu decideixes què hi enganxes.

---

⬆️ [[00 - LÍNIA TEMPORAL MESTRA]]
