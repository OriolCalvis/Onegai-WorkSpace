# 2026-09-17 · Claude (Cowork) · E-14 selectores: colisión y aportación

**Encargo de Oriol:** coger el Gantt del motor y seguir programando con `MAPA_DEL_CODIGO`, teniendo en cuenta que hay otros agentes trabajando a la vez.

## Qué pasó

1. Leídos AGENTS, protocolo y Parte 3 de la BITACORA, TABLERO, `MAPA_DEL_CODIGO` y `seguimiento/`. Paquetes sin registro ni fila: E-05, E-06, E-14, E-16, F-*, G-01/02/04/05/06/12/13/15/16. F-01 y G-01 están bloqueados por DEC-06/DEC-13. Elegido **E-14 (mitad sin interfaz)**: su criterio se puede cumplir sin GL y, con la zona acotada, no toca ningún fichero de cruce.
2. **09:38** — fila reclamada en TABLERO. Implementación en copia aislada (`~/work` de la VM), no en el árbol compartido.
3. **09:44** — otra sesión sin fila hizo commit de `6a3be66` «E-14» en las mismas rutas (`CatalogPickerState.{h,cpp}`, test y cmake) y siguió editándolos hasta las 10:11 como mínimo. Su E-16 (`ProjectDatabaseState`, sin commit) ya los incluye. Esa misma sesión, que firma como «Oriol» y escribe los mensajes en catalán, está recorriendo el Gantt a gran velocidad (E-05, E-06, E-14, E-16, G-12b, G-13) sin filas en el TABLERO.
4. Oriol decidió: **dejar E-14 a esa sesión y aportar mi versión.** No he escrito en sus rutas.

## Entrega

MG `0929f32` — sólo tres ficheros nuevos en `planificacion/seguimiento/`:
- `E-14-alternativa-claude.patch`: se aplica contra `6a3be66` (`patch -p1 --dry-run` limpio).
- `E-14-alternativa-claude.md`: qué contiene, cómo se verificó y qué falta.
- `E-14-alternativa-claude.json`: registro anexo con las seis claves del contrato. `seguiment.py` lo acepta.

**Verificación:** 8/8 casos contra `6a3be66` aislado y contra el árbol vivo, y 8/8 con ASan+UBSan. Detecta las 6 mutaciones. La VM no tiene cmake ni red, así que se compiló con g++ 11 y la lista de fuentes de las librerías: **ni el CMake real ni la CI están verificados.**

**Hallazgo con assets reales:** al mezclar todos los `assets/objects/*.json` con los catálogos RPG salen 2574 ids repetidos; casi todos vienen de `libreria_completa`/`libreria_armas` frente a `equipment.json`. El índice del selector tiene que construirse con los catálogos que declara el proyecto.

## Para quien siga

- Los tests de `6a3be66` no comprueban el criterio de E-14: `loadCatalogs` devuelve `true` sin cargar nada y las aserciones pasan con el selector vacío. Los 8 casos del parche pueden servir de prueba de aceptación aunque se conserve su API.
- `pla/seguiment.py` hoy NO genera el plan por dos registros ajenos recién creados: `DEC-02.json` (sin `id`) y `G-13.json` (sin `actions`). No los he tocado.
- **Trampa nueva (añadida a la Parte 3):** en este sandbox, `git status` sin `--no-optional-locks` deja un `.git/index.lock` vacío, y `git update-ref` deja un `.git/HEAD.lock`. Los dos que dejé se renombraron al momento (`*.rancio.claude-e14-*`).
