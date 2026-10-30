# 12/09/2026 — D-10: CI valida els assets reals (verificació)

- **Gantt:** corte B, fase D. Fila reclamada en `TABLERO.md`.
- **Commits:** ninguno a propósito.
- **Deja esperando a Oriol:** nada.

## Qué se ha fet

D-10 verificat: la validació Python de catàlegs (`validar_todo.py --skip-phase1`) funciona i passa amb 0 errors i 14 avisos coneguts (deitats que falten, projectes sense entrada). El test CTest `catalog_content_validation` també passa.

### Estat actual

- `cmake/CatalogValidationTests.cmake` ja existia i està registrat a `CMakeLists.txt:982`.
- `test_catalog_content` **28/28** en verd (D-07 mounts + D-03 monsters + D-02 skills + D-05 equipment + D-06 deities + D-07 summons + locations + NPC + adventures).
- `validar_todo.py --skip-phase1` → 0 errors, 14 warnings (tot content: deitats 432/442, projectes sense entrada).

## Verificació

- `ctest -L gl-free -R catalog_content` → 2/2 passen.
- `test_catalog_content` → 28/28.
- `validar_todo.py --skip-phase1` → OK.

## Pendient

- Els 14 warnings són de contingut (deitats que falten al catàleg, projectes sense `entrada`). No són bugs de motor. Quan Oriol decideixi quines deitats afegir o quins projectes marcar com a jugables, els warnings desapareixeran.
