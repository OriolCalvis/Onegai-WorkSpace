---
name: "fabrica-de-nit"
description: "Fàbrica de software amb subagents per a Software/ (MotorGraphico): tasques E en paral·lel, validador independent amb mutacions, commits a branques de grup del Mac i integració, sense supervisió."
---

# Fàbrica de nit: subagents programadors i validadors, tasques E, sense supervisió

És el procediment de la nit del 29/10. El coordinador (tu) no programa: talla la feina en E, escriu encàrrecs curts, llança un subagent programador per E i un validador independent que no ha vist la feina. Si la validació passa, fa el commit al Mac. Si no, la torna **una vegada** al programador. Després es reprograma i continua fins que la cua quedi buida.

Complementa la skill `fabrica-onegai` (el `fabrica.py` de línia base i entrega). Aquesta skill organitza qui fa què i com es mou la feina entre el núvol i el Mac.

## 0. Abans de començar

- **Accés al Mac:** `device_request_folder_access` amb `/Users/admin/Documents/Documentos - Oriol Os (2)/Software`. Totes les ordres git hi van amb `git -c core.trustctime=false -c core.checkStat=minimal` (iCloud). Al Mac no es pot esborrar, així que **no facis `git commit`, `checkout` ni `merge` a la còpia de treball**: els commits es fan amb fontaneria (`read-tree` → `update-index` → `write-tree` → `commit-tree` → `update-ref`) sobre un índex temporal, amb els scripts del §6.
- **Còpia al núvol** (més ràpida i hi ha GitHub per a GLM/GLFW): al Mac, `git archive --format=tar.gz -o out/_claude_exp/X.tgz <branca> MotorGraphico-main/MotorGraphico planificacion_alcance_y_vision/ECONOMIA.md ':(exclude)MotorGraphico-main/MotorGraphico/Claude outputs' ':(exclude)MotorGraphico-main/MotorGraphico/out'`, i després `device_stage_files`. Al núvol: `tar -xzf`, `git init` i un commit «base», per veure els diffs. Afegeix a `.git/info/exclude` `__pycache__/` i `planificacion_alcance_y_vision/`.
- **Compilar al núvol:** `cmake -S MG -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_X11=OFF` (GLFW sense finestres: compila `motor_core` i enllaça `juego`). Després `make -j4 suite_gl_free juego` (uns 7 min).
- **Línia base:** `ctest -L gl-free -j4 --timeout 180`. Desa els **noms** de les proves que fallen (`base_noms.txt`) i compara sempre per nom, perquè els números canvien quan s'afegeixen tests. El 29/10 n'eren 16.
- **Dues còpies** (`w` + `build`, `w2` + `build2`) per tenir dos programadors alhora sense que es trepitgin. Cada còpia du el seu commit local després de cada E.

## 1. Fitxers de la fàbrica (`fab/`)

- `REGLES.md` (per a tots els subagents), amb el model del §7.
- `VALIDADOR.md` (per als validadors), amb el model del §7.
- `cola.md`: taula `E · tipus · model · estat · nota` (estats: pendent | en curs | validant | ROJO 1 → correcció | hecha · branca commit | pendent-verif | BLOQUEADA | espera Oriol), més les incidències per a Oriol.
- `briefs/<E>.md` i `briefs/V-<E>.md`: un encàrrec i una validació per E.
- `ref/`: material que el subagent necessita i que no és a la còpia (textos aprovats, extraccions fetes al Mac).

## 2. Com és un encàrrec (≤ 40 línies)

1. «Llegeix primer `fab/REGLES.md`», i els 2 o 3 fitxers exactes que cal llegir.
2. **Context mínim:** el problema en 2 o 3 frases, amb `fitxer:línia`, i les decisions d'Oriol que manen, citades.
3. **Fes:** passos numerats, amb noms de fitxers, funcions i casos de prova concrets. Indica on van els fitxers nous si un fitxer gran ja és a prop de 500 línies.
4. **Verifica:** les ordres exactes.
5. **Hecho cuando:** un criteri comprovable. Si el canvi és visual: «pendent de verificació; digues què cal mirar al Mac».

Per a les feines grans, primer fes un **encàrrec d'investigació** (informe + proposta dividida en E de ≤ 150 línies) i després una E per part.

## 3. Models

- Programadors de C++, dades i integració: `sonnet`. Validadors: `sonnet`, perquè les proves de mutació necessiten criteri.
- Informes de lectura i classificació: `haiku` va bé, però comprova'n les conclusions (ha deixat 105 «dubte» que es podien resoldre).
- Petites correccions d'1 a 3 línies, o comprovar una mutació: fes-ho tu, és més barat que un subagent.

## 4. El cicle d'una E

1. Llança el programador (en segon pla): «Executa l'encàrrec de `fab/briefs/<E>.md` … Còpia `<w>`, build `<build>`. No programis tasques ni recordatoris. No deixis processos en segon pla.»
2. Quan acabi, escriu `V-<E>.md` i llança el validador. A més de les ordres, li demanes:
   - **Mutacions:** trencar cada regla al codi i comprovar que algun test falla; ha de llistar les que sobreviuen.
   - El **risc concret** d'aquella E. Per exemple: «busca tot el que després de les campanes demana `franja` 2», o «tots els camins que canvien l'estat sense avaluar».
   - Fitxers dins l'abast i ≤ 500 línies.
   - `ctest -L gl-free` sense fallades noves, comparat per nom.
   - «Deixa el repo com l'has trobat. Primera línia: VERD o ROJO (ROJO només per errors reals).»
3. Si dona **ROJO**, envia un `SendMessage` al programador amb els punts exactes (una sola volta). Si el ROJO és d'enfocament, canvia l'enfocament en lloc de tapar forats (exemple: una avaluació central cada fotograma en lloc de crides a cada camí).
4. Si dona **VERD**, o si tu has comprovat la correcció:
   - Escriu l'evidència `<E>.md`: estat, què s'ha fet, validació, heretades / noves / dependències, i què cal mirar al Mac.
   - Fes el commit local a la còpia.
   - Puja-ho al Mac (§6).
   - Actualitza `cola.md`.
5. Llança la següent E de la cua a la còpia que hagi quedat lliure.

## 5. Continuar sol tota la nit

- Cada notificació d'un subagent et torna a despertar. Afegeix un **batec** de seguretat amb `send_later` cada 45-50 minuts («revisa `cola.md` i els subagents; rellança el que s'hagi aturat; programa el següent batec»).
- Si el contenidor es reinicia, el disc es conserva però els subagents s'aturen. Mira `git status` de cada còpia i rellança des de l'encàrrec, avisant que hi ha hagut un intent anterior.
- **Els subagents mai no programen tasques** (els de `haiku` ho van fer dues vegades). Si en crea alguna, apunta-la a les incidències per a Oriol; no l'esborris tu, perquè el sistema ho denega.
- Quan la cua quedi buida o tot depengui d'Oriol, de PixelLab o del Mac: comprova la integració (§8), escriu el resum i para.

## 6. Pujar al Mac (scripts a `$HOME` del Mac)

- Fitxers: `cp` a `/mnt/user-data/outputs/fab/<e>/`, `device_commit_files` a `Software/out/_claude_exp/in/<e>/`.
- **`cf.sh BRANCA BASE MISSATGE ruta=fitxer…`**: substitueix fitxers sencers sobre la branca (la crea des de BASE si no existeix). Fes-lo servir només si la base del fitxer al núvol és la mateixa que a la branca (comprova-ho amb `git rev-parse BRANCA:ruta` contra `BASE:ruta`).
- **`cp.sh BRANCA PEDAÇ MISSATGE [ruta=fitxer…]`**: aplica `git diff HEAD` (fet amb `git add -N .` i després `git reset`) amb `git apply --cached --3way` sobre la branca. És la manera segura quan la còpia porta canvis d'altres grups. Els avisos «does not exist in index / Falling back» dels fitxers nous són normals.
- Cada commit acaba amb `Co-Authored-By` i `Claude-Session`, i amb el missatge en català. Després d'`update-ref`, comprova que no queda cap `.lock` a `.git/refs`; si n'hi ha un d'un error teu, mou-lo (`mv`) a `.git/stale-…`.

```bash
# cf.sh
set -e; cd "$HOME/mnt/Software"; G="git -c core.trustctime=false -c core.checkStat=minimal"
BR=refs/heads/$1; BASE=$2; MSG=$3; shift 3
if $G rev-parse -q --verify $BR >/dev/null; then OLD=$($G rev-parse $BR); else OLD=""; fi
PARENT=${OLD:-$($G rev-parse $BASE)}; export GIT_INDEX_FILE=$HOME/idx_cf_$$; rm -f $GIT_INDEX_FILE
$G read-tree $PARENT
for pr in "$@"; do r=${pr%%=*}; l=${pr#*=}; h=$($G hash-object -w "$l"); $G update-index --add --cacheinfo 100644,$h,"$r"; done
T=$($G write-tree); C=$($G commit-tree $T -p $PARENT -F "$MSG"); $G update-ref $BR $C $OLD; rm -f $GIT_INDEX_FILE
# cp.sh: el mateix, però amb `$G apply --cached --3way --whitespace=nowarn "$PEDAÇ"` després de read-tree, i amb aturada si `git ls-files -u` no és buit
```

## 7. Models de REGLES i VALIDADOR

**REGLES.md (resum):**
- Repo, build i ordres.
- Toca només els fitxers de l'encàrrec; cap `git commit`, `reset`, `checkout` ni `clean`.
- Estil: castellà al codi; ≤ 500 línies per fitxer; un test nou va a `cmake/<Nom>Tests.cmake` i a `MOTOR_GL_FREE_TARGETS`.
- No canviïs narrativa, mapes ni economia; els textos d'Oriol es copien tal qual.
- Línia base de ctest per nom; els timeouts sota càrrega es tornen a passar sols.
- Si una bandera passa a encendre-la un esdeveniment, els tests de «qui l'encén» han de comptar `assets/eventos`.
- No programis tasques.
- Resposta curta: fitxers, ordres i resultat, què no s'ha pogut fer.

**VALIDADOR.md (resum):**
- Revisor independent.
- Llegeix el diff sencer, busca errors de lògica i proves que passarien encara que el codi estigués malament.
- Executa les ordres. No modifiquis res.
- Primera línia VERD o ROJO, i després com a màxim 8 punts amb `fitxer:línia`.

## 8. Integració i tancament

- **Cada branca tal com és al Mac:** `git archive` de la branca, compilació neta al núvol (`juego` + `suite_gl_free`), `ctest` comparat per nom i `validar_todo`. Els pedaços aplicats amb `--3way` no s'han compilat mai junts, i això és el que ho comprova.
- **Branca de prova conjunta** (`prueba/claude/integracion-…`), si Oriol vol jugar-ho tot: un índex temporal sobre la unificada, amb `git apply --cached --3way` de `git diff U...branca` per a cada grup (amb tres punts: des de la base comuna). Resol els conflictes amb `git merge-file` (normalment són unions de línies afegides al mateix lloc). Després es compila i es passen les proves al núvol. No fusiona res.
- **Resum:**
  - `orquestador/lote_F_evidencia/NOCHE_<data>.md` a la unificada: taula d'integració, què s'ha fet per grup, què ha de decidir Oriol, què cal mirar al Mac i incidències.
  - L'índex d'una línia per E al document del lot.
  - Una còpia al projecte (`claude/fabrica_nit_<data>.md`).
- Al matí, a Oriol, en català: el que s'ha fet en 5 línies, les decisions que li toquen numerades, i què cal mirar al Mac.