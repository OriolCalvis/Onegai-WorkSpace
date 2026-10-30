# 2026-09-17 · Claude (Cowork) · Publicar en GitHub

Petición de Oriol: subir todo a GitHub (admin, dndWeebCC, MotorGraphico, Onegai-WorkSpace).

## Qué se hizo
- La VM no tiene red; el contenedor de la nube sí, pero el proxy de git **deniega el push (403)**:
  los repos no están en las fuentes autorizadas de la sesión. No se reintentó.
- `MotorGraphico-main` (origin = Onegai-WorkSpace, rama `feature/editor-s1-contratos`):
  el remoto tenía un commit hecho desde la web (3085d04 «Create Revision 14-08», añade
  `Revisio 14/Revision 14-08`, una línea vacía). Se fusionó en la nube con los 67 commits
  locales (sin conflictos) → **28dc3ca**, y se trajo a la rama local por avance rápido
  (update-ref con valor antiguo fd9027d, índice y fichero sincronizados).
  Queda listo para `git push` sin forzar. No se tocó el WIP sin commitear de otros agentes.
- Escaneo de secretos del diff a publicar: nada.

## Estado de cada repo
| Repo | Local | Estado |
|---|---|---|
| Onegai-WorkSpace | MotorGraphico-main | listo: `git push` (69 commits) |
| MotorGraphico | — (historia distinta, main de julio) | sólo como rama nueva: `git push https://github.com/OriolCalvis/MotorGraphico.git feature/editor-s1-contratos` |
| dndWeebCC | dndWeebCC-master (rama appmod/java-upgrade-…) | ya sincronizado; sólo hay WIP sin commitear (pom.xml) |
| admin | repo en /Users/admin | fuera de las carpetas conectadas |
| — | Onegai 2 | repo sin remoto |

`Software/` no es un repo git: la «carpeta entera» sólo existe en la rama remota
`codex/workspace-structure`; refrescarla implicaría publicar trabajo sin commitear de otros.

## Trampas
- `git bundle verify/unbundle` fallan en este repo por una ref rota `refs/stash 2`
  (copia de macOS); se usó `git unpack-objects` con el pack extraído del bundle.
- `device_commit_files` no puede escribir dentro de `.git`.
- Restos: `.git/claude-push-*.bundle`, `.git/claude-merge-28dc3ca.bundle`, locks `*.rancio.claude-push-*`.

## Segona petició: pujar-ho tot a Onegai-Biodiversitat
- També 403 (repo fora de les fonts autoritzades de la sessió). No s'ha reintentat.
- `Software/` sencer fa 9,1 GB → no és viable com a instantània a GitHub. Es pugen les
  històries commitejades de cada repo amb prefix de branca.
- Nou: `herramientas/pujar_a_biodiversitat.sh` (sense --force; omet eigent/* i refs *.lock*).
  Provat contra un repo buit local: 12 branques + `main` = 28dc3ca. Cap blob > 50 MB.

## Tercera petició: només el codi, amb gitignore
- `herramientas/biodiversitat.gitignore`: ignora-ho tot i torna a admetre extensions de codi
  i fitxers de build/config; exclou out/, build/, target/, entorns, informes HTML de
  planificacion, .github/modernize, gestió documental i secrets (.env, *.pem, *.key…).
  No es posa com a `.gitignore` de Software/ per no amagar res al repo «admin».
- `pujar_a_biodiversitat.sh` reescrit: carpeta temporal → clona main de GitHub (o repo nou)
  → tria amb `git check-ignore --no-index` → copia amb tar → commit (amb els HEAD de cada
  subrepo al missatge) → push sense forçar. `NOMES_LLISTA=1` només llista.
- Selecció: 1152 fitxers, ~21 MB (el més gran, miniaudio.h, 4 MB). Escaneig de secrets: net
  (només noms de fitxer a AuthorAgent/gateway/.../system.ts). `.env` d'AuthorAgent exclòs.
- Trampes: els `.gitignore` dels subprojectes amagaven StoryCraftr sencer → només s'admet
  `/.gitignore`. Hi ha fitxers d'iCloud no descarregats (StoryCraftr) que la VM no pot llegir
  (EDEADLK); al Mac, tar els descarrega en llegir-los.
- Provat contra un repo local amb README inicial: 2 execucions (la 2a diu «Res de nou»),
  README conservat, autor Oriol.
