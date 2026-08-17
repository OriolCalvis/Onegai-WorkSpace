# Guia de control de versions — ONEGAI

*Com treballar amb Git en aquest projecte. Pensada per a una persona sola, sense
cerimònia innecessària però amb la xarxa de seguretat que abans no hi era.*

---

## 1. Què hi ha muntat

- Repositori Git inicialitzat a l'arrel del projecte, branca **`main`**.
- Tres commits inicials: estat base → eines noves del wireframe → documentació.
- Etiqueta **`v0.1.0`** marcant la primera versió completa de catàleg, món i eines
  de creació.
- `.gitignore` ampliat: fora `target/`, `logs/`, `.DS_Store`, `*.bak` i `data_backup/`.
- **`data/` sí que es versiona**: les 2.017 cartes, 300 històries, 133 PNJs i la
  geografia són el contingut que més val la pena protegir.

Comprova-ho en qualsevol moment:

```bash
git log --oneline --graph --decorate
git tag -l
```

---

## 2. Regla d'or

> **Commit petit i freqüent val més que commit perfecte i rar.**

Si has tocat res que funciona, fes commit. No cal que estigui acabat: la gràcia de tenir
historial és poder tornar enrere, i no pots tornar a un punt que mai has desat.

---

## 3. Flux de treball diari

```bash
# 1. Mira què has tocat
git status

# 2. Revisa els canvis abans de desar-los (evita commits accidentals)
git diff

# 3. Afegeix i fes commit
git add -A
git commit -m "feat(cartes): afegeix carta de muntura al catàleg"

# 4. Quan una tanda de feina està acabada i provada
git tag -a v0.2.0 -m "Motor de tirades de dau"
```

Si t'equivoques i encara **no** has fet commit:

```bash
git restore <fitxer>        # descarta els canvis d'un fitxer
git restore .               # descarta-ho tot (amb compte!)
```

Si ja has fet commit i te'n penedeixes:

```bash
git revert <hash>           # crea un commit que desfà l'anterior (segur)
git reset --soft HEAD~1     # desfà l'últim commit però conserva els canvis
```

---

## 4. Format dels missatges de commit

`tipus(àmbit): què fa el commit, en imperatiu i en una línia`

**Tipus:**

| Tipus | Quan |
|---|---|
| `feat` | Funcionalitat nova |
| `fix` | Correcció d'un error |
| `content` | Contingut nou o modificat a `data/` (cartes, històries, mapa) |
| `docs` | Documentació a `docs/` |
| `style` | CSS, plantilles, res de lògica |
| `refactor` | Reorganització sense canvi de comportament |
| `test` | Tests |
| `chore` | Manteniment (dependències, scripts, `.gitignore`) |

**Àmbits habituals:** `cartes`, `personatges`, `aventures`, `historias`, `npcs`, `mapa`,
`ui`, `dades`, `build`.

**Exemples reals per a aquest projecte:**

```
feat(mapa): línia temporal que filtra el món per any
fix(personatges): el límit de mà no comptava els hechizos
content(dotes): 20 dotes noves per cobrir tiers 3-5
refactor(mapa): parteix el JS inline en mòduls a static/js/mapa/
docs: actualitza el pla de tancament amb els blocs ja tancats
```

Si el commit necessita explicació, deixa una línia en blanc i escriu el cos a sota —
com els tres commits inicials d'aquest repositori.

---

## 5. Branques

Amb una sola persona treballant, `main` n'hi ha prou per al dia a dia. Obre branca
**només** quan el canvi sigui prou gros com per deixar el projecte trencat un temps:

```bash
git switch -c feat/motor-combat     # comença
# ... treballa i fes commits normals ...
git switch main
git merge feat/motor-combat         # incorpora quan funcioni
git branch -d feat/motor-combat     # neteja
```

Candidats naturals a branca segons el DAFO: `refactor/mapa-modular`,
`feat/motor-combat`, `feat/multiclasse`.

> **Branques permanents:** a mesura que el projecte ha crescut hem afegit un
> esquema de branques permanents (`public`, `development`, `eines`) més les
> temporals (`qa`, `feature/*`, `fixing/*`). La rutina simple d'aquesta secció
> continua sent vàlida dins de `development`; per al flux complet entre branques
> i la política de tags, vegeu `Guia_Branques.md`.

---

## 6. Versions i etiquetes

Semàntic simplificat: **`vMAJOR.MINOR.PATCH`**

- **PATCH** (`v0.1.1`) — correccions i contingut nou sense canviar regles.
- **MINOR** (`v0.2.0`) — funcionalitat nova (motor de combat, multiclasse, línia temporal).
- **MAJOR** (`v1.0.0`) — quan el sistema es pugui jugar sencer des de l'app: crear
  personatge, resoldre tirades, gestionar piles i tancar un descans.

```bash
git tag -a v0.2.0 -m "Descripció del que aporta"
git tag -l                     # llista
git show v0.1.0                # què hi havia en aquell punt
```

---

## 7. Còpia de seguretat remota

Ara mateix el repositori és **només local**: protegeix contra errors teus, no contra
una avaria del disc. Quan vulguis un remot (GitHub, GitLab o un disc extern):

```bash
# GitHub / GitLab
git remote add origin git@github.com:USUARI/onegai.git
git push -u origin main --tags

# Alternativa sense núvol: repositori nu en un disc extern
git clone --bare . /Volumes/DISC/onegai.git
git remote add extern /Volumes/DISC/onegai.git
git push extern main --tags
```

A partir d'aquí, `git push` després de cada tanda de feina.

---

## 8. Rutina recomanada

| Quan | Què |
|---|---|
| Cada cop que una cosa funciona | `git add -A && git commit -m "..."` |
| Abans de començar un canvi gros | Commit del que hi hagi pendent, i branca si cal |
| Abans d'executar un script generador | Commit — els generadors escriuen massivament a `data/` |
| Al final de cada sessió de treball | Commit + `git push` si hi ha remot |
| Quan una fase del pla es tanca | `git tag -a vX.Y.0` |

> **Avís concret d'aquest projecte:** els scripts de `scripts/generar_*.py` escriuen
> centenars de fitxers a `data/` d'una tacada. Fes commit **abans** d'executar-ne cap:
> així, si el resultat no t'agrada, `git restore data/` ho desfà tot en un segon.

---

## 9. Ordres de rescat

```bash
git log --oneline -- data/historias        # historial d'una carpeta
git show HEAD~2:src/.../Historia.java      # com era un fitxer fa dos commits
git restore --source=HEAD~1 <fitxer>       # recupera una versió anterior d'un fitxer
git diff v0.1.0..HEAD --stat               # què ha canviat des d'una versió
git clean -n                               # llista fitxers no versionats (prova en sec)
```
