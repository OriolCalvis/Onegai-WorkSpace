# Tablero de desarrollo paralelo

Objetivo inmediato: que Boundington sea jugable y fácil de ampliar sin que
dos personas modifiquen los mismos subsistemas a la vez.

## En curso — Codex

| Tarea | Resultado verificable | Archivos principales |
| --- | --- | --- |
| Cargar catálogos compuestos | `juego --catalogo-extra` y F7 del editor ven todos los objetos del proyecto | `Application.*`, `juego.cpp`, `level_editor.cpp` |
| Biblioteca racial real | 43 sprites idle 64×64 seleccionados por `raceId` en juego | `assets/textures/race_npc_*`, `Application.cpp` |
| Plaza de las Razas base | Nivel jugable con 12 PNJ de razas distintas y regreso a ciudad | `assets/maps/plaza_de_las_razas.tmx`, `assets/levels/plaza_de_las_razas.json` |

## Lista para Claude — sin bloquear a Codex

| Prioridad | Tarea | Entrega / aceptación | Zona de propiedad |
| ---:| --- | --- | --- |
| P0 | Animaciones raciales | Para 6 razas representativas: idle + caminar arriba/abajo/lados, 64×64, 4 frames por dirección y una ficha que documente frames | `assets/textures/race_npc_animations/`, documentación de arte. No tocar `Application.cpp`. |
| P1 | Encuentros de la Plaza | Ampliar la plaza base con 3 conversaciones encadenadas, una misión corta y su aventura de entrada; conservar sus IDs y no tocar el renderer | `assets/adventures/`, `assets/levels/plaza_de_las_razas.json`. |
| P1 | Misión de Luisarda | Cadena Los Perdidos: pista, encuentro, decisión y resultado; reutilizar IDs existentes | `assets/adventures/`, `assets/levels/`. |
| P1 | Misión de Ben Kafka | Bosque, niños y saga: rutas moralmente distintas, derrota/rescate/fallo | `assets/adventures/`, `assets/levels/`, catálogo de objetos nuevo. |
| P1 | Misión de Griffin | Alcantarillas, nagas y recompensa; crear mapa y conexión desde ciudad | `assets/maps/`, `assets/levels/`, `assets/objects/`. |
| P2 | Editor: inspector de variantes | UI para escala, nombre local, variante y propiedades por instancia según `OBJECT_VARIANTS_FOR_EDITOR.md` | `EditorState.*`, `level_editor.cpp`. Coordinar antes de editar ambos. |

## Aviso de colisión (16/08)

**Se construyeron dos Plazas de las Razas con un minuto de diferencia** —
`plaza_razas` y `plaza_de_las_razas`— porque dos agentes cogieron la misma
fila P0 a la vez. Se consolidó sobre `plaza_de_las_razas`, que era la que
estaba registrada en el manifiesto del proyecto; la otra se retiró.

La regla 1 de aquí abajo existe justo para esto y no bastó: **nadie marca
la fila al empezar**. Propuesta: añadir una columna «tomada por / desde»
y escribirla ANTES de crear el primer fichero.

## Reglas de integración

1. Una tarea por rama y por directorio de propiedad.
2. No modificar `Application.cpp`, `EditorState.*` o formatos JSON de otra tarea sin avisar.
3. Cada entrega incluye un mapa/aventura que cargue y un comando de prueba.
4. IDs en minúsculas con guiones bajos; los datos viven en catálogo y los
   spawns en nivel.
5. Antes de integrar, ejecutar `cmake --build build --target juego level_editor`.

## Próximo corte

La siguiente integración es enriquecer la Plaza de las Razas con una aventura.
La prueba base ya es: `./juego --nivel assets/levels/plaza_de_las_razas.json
--catalogo assets/objects/test_objects.json --catalogo-extra
assets/objects/editor_npcs_por_raza.json --catalogo-extra
assets/objects/plaza_razas_objetos.json 1` desde `build/`.
