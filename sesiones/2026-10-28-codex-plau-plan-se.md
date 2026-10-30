# Sessió PLAU-PLAN-SE — planificació per contractes de funció

- **Data:** 2026-10-28
- **Autor:** Codex
- **Commits:** `d296ccf45fb1cd1ad8cb5fab69f518ed5810c17f` + commit de tancament a `codex/plau-plan-se`; només planificació i fragments propis. La branca compartida no canvia.
- **Deja esperando a Oriol:** Revisar escala, sostre E proposat de 90 min i decisions del primer lot. Aquest pla no autoritza implementar el contingut dels adjunts.

## Resultat

Proposta completa de descomposició del Pla U a
`MotorGraphico-main/MotorGraphico/planificacion/pla_unity/descomposicio/`.
37 paquets i 128 jornades originals preservats, 20 lots A i 330 E
(318 de producte i 12 de preparació). Arbre complet S/A/B/C/D/E,
fitxes de paquet, índex E, contractes d’exemple i plantilla de delegació.

417 h identificades provisionals; no són nova estimació total ni dates.
La jornada original era ~6 h; la nova escala és 7 h 30. Fases S indeterminades.
U2–U4 són propostes d’API pendents de congelar; no s’afirma que existeixin.
Cap instal·lació de skill, ordre fabrica, C++, build, CTest, push o merge.

## Evidència i divergències

Lectura de BITACORA completa, AGENTS, TABLERO, handoff, estabilització i relleus.
Zip inspeccionat com a dades AST, sense executar-lo. Font SHA256 i 61 rutes
contrastades a pla.json. Base observada 522347660cf4ba9c4693781f21effa6748f35b1c.
Repo real Software/, motor nested. Hooks existents poden ometre la guardia
per una suposició de ruta; queda una E de diagnòstic, no s’ha corregit aquí.

Handles generacionals existents i deque vs vector; sis ranures vs tres;
tres franges vs quatre; zero fitxers d’àudio; integració de contacte necessita
GameSession; Application amb WIP i més de 500 línies. Tot registrat a AUDITORIA.
Cap canvi en les durades/dependències històriques ni en les regles de producte.

## Verificació

- renderitza.py i valida.py: 37/37 paquets, 330 E, IDs/cobertura, DAG combinat,
  tots els sostres i derivats reproduïbles en verd.
- Sis mutacions en memòria rebutjades: durada, cicle, font alterada,
  preparada sense contracte, cobertura duplicada i retorn absent.
- Tots els enllaços Markdown locals resolen.
- Guardià explícit --rapido --indice: VERMELL heretat (49 símbols duplicats
  en fonts «2.cpp» ja versionades); no es toca C++ i es documenta la fallada.
  El hook actual pot sortir 0 sense executar-lo per una ruta antiga del monorepo.
- Reproducció en clon net de `d296ccf`: valida.py i renderitza.py --comprovar
  en verd; el clon continua net, sense copiar WIP.
- Les nou fonts implicades en els 49 duplicats coincideixen byte a byte
  amb HEAD: la fallada és heretada i queda oberta abans d’implementar producte.

## Treball aliè

L’arbre tenia modificacions d’Application, ShaderManager, AuraJugador/AuraMoral,
HUD, sprites, demo, assets, documents de coordinació i sessions no versionades.
No s’han desfet ni commitejat. TABLERO i INDICE ja eren bruts: el commit
construeix només el fragment propi sobre HEAD mitjançant índex temporal,
sense substituir l’índex real ni moure la branca compartida.

Fila reclamada abans del primer fitxer i moguda a Hecho en lliurar el pla.
INDICE regenerat al disc amb l’eina oficial, preservant les sessions d’altres.
