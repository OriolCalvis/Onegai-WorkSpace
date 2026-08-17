# Guia de branques (esquema git-flow adaptat)

Aquest document defineix el sistema de branques permanents i temporals del projecte ONEGAI. Complementa `Guia_Control_Versions.md` (que cobreix la rutina diària de commits per a una sola persona): aquí afegim l'estructura de branques per separar **versió estable**, **desenvolupament**, **validació**, **hotfix** i **eines**.

> Motivació: el projecte ha crescut i ja no es pot treballar tot sobre una sola branca. Volem poder seguir avançant el dia a dia mentre es manté una versió `public` sempre jugable i validable.

## Branques permanents

| Branca | Rol | Neix de | Tracking remot |
|---|---|---|---|
| `public` | Versió estable i etiquetada (release). Sempre ha d'estar en estat jugable. | — (arrel del nostre històric) | `origin/public` |
| `development` | Treball diari. Aquí s'integren les features i es prepara la propera iteració. | `public` | `origin/development` |
| `eines` | Branca llarga per a scripts, generadors i eines auxiliars que no són codi de l'app. | `development` | `origin/eines` |

## Branques temporals

| Branca | Quan es crea | Neix de | Es mergea a | Vida |
|---|---|---|---|---|
| `qa` | Quan es tanca una iteració de `development` i es vol validar abans de pujar-la a `public`. | `development` | `public` + `development` | Esborrada després del merge |
| `feature/<nom>` | Per a una funcionalitat concreta (ex. `feature/ws-d-combate`). | `development` | `development` | Esborrada després del merge |
| `fixing/<id>` | Hotfix urgent sobre la versió estable. | `public` | `public` + `development` | Esborrada després del merge |

## Diagrama de flux

```
public (release estable + tags vX.Y)
   ↑ merge de qa validada + tag nou
qa (iteració tancada en validació)
   ↑ neix de development
development (treball diari)
   ├─ feature/XYZ   → mergea a development (esborrada)
   ├─ eines         → mergea a development (long-lived)
   └─ fixing/XYZ    → mergea a public + development (esborrada)
```

## Rutina per cada flux

### 1. Dia a dia (development)
```bash
git switch development
git pull
# treballa, commit petit i freqüent (vegeu Guia_Control_Versions.md)
git push
```

### 2. Feature nova (opcional per canvis grans)
```bash
git switch development && git pull
git switch -c feature/ws-d-combate
# treballa...
git switch development && git merge feature/ws-d-combate
git branch -d feature/ws-d-combate   # esborra la branca temporal
```

### 3. Tancar una iteració → qa → public
```bash
# Des de development, obrim qa per validar
git switch development
git switch -c qa
# (validació manual, proves, revisió)
# Si tot OK, pugem a public i etiquetem
git switch public && git merge qa
git tag -a v0.X.0 -m "v0.X.0 — <descripció de la iteració>"
git push origin public --tags
# Recupero els canvis a development i esborro qa
git switch development && git merge qa
git branch -d qa
```

### 4. Hotfix urgent (fixing)
```bash
git switch public && git pull
git switch -c fixing/<id>
# corregeix...
git switch public && git merge fixing/<id>
git tag -a v0.X.Y -m "v0.X.Y — hotfix <id>"
git push origin public --tags
# porta el fix a development
git switch development && git merge fixing/<id>
git branch -d fixing/<id>
```

## Política de tags

- Semver simplificat: `vMAJOR.MINOR.PATCH` (vegeu `Guia_Control_Versions.md` secció 6).
- **Cada merge a `public` porta un tag nou.** No es fa mai commit directe a `public` sense tag.
- Tag annotated (`git tag -a`) amb missatge descriptiu de la iteració.
- Els tags es pugen amb `git push origin <tag>` o `git push origin public --tags`.

## Regles d'or

1. **Mai commit directe a `public`** excepte via merge de `qa` o `fixing`.
2. **`public` ha d'estar sempre jugable.** Si una iteració no valida, es queda a `development`.
3. **Branques temporals s'esborren després del merge** (`feature/*`, `qa`, `fixing/*`). Només `eines` és long-lived entre les auxiliars.
4. **Cap force push a `public` ni a `development`** sense consens.
5. **Commits petits i freqüents** dins de cada branca (vegeu `Guia_Control_Versions.md`).

## Nota sobre el remot

El repositori remot (`origin`) conté a més una branca `master` amb un històric divergent i diverses branques `codex/*` amb PRs oberts, corresponents a un altre flux de treball. **No les toquem**: el nostre històric viu a `public` / `development` / `eines`. Es recomana canviar la branca per defecte del remot a `public` (GitHub → Settings → Branches) perquè els nous clones apuntin a la nostra línia estable.
