# Biblioteca visual de PNJ por raza

Los cuatro atlas `editor_race_npcs_01.png` a `editor_race_npcs_04.png`
contienen 43 sprites editoriales: uno por cada `raceId` de
`assets/catalogs/races.json`. El mapeo está centralizado en
`previewNpcRacial()` de `examples/level_editor.cpp`, para que el editor
muestre la silueta correspondiente al colocar los PNJ base de
`editor_npcs_por_raza.json`.

| Atlas | Rejilla | Razas |
| --- | --- | --- |
| 01 | 4×3 | Aarakocras, alquimistas, cara loca, cazador, diplomática, dracónidos, drows y elfos. |
| 02 | 4×3 | Enanos, espectro, guardianes, hechicero, hombres lagarto, medianos, mercenaria y minotauro de llanura. |
| 03 | 4×3 | Minotauro de laberinto, nagas, nobles/orcos, sangre feérica, fraguas profundas, dosel, marcas, Gongorguma y revenant. |
| 04 | 4×2 | Sagas, tieflings, traidor oscuro, yokais y una ciudadana genérica de reserva. |

Son previews de autoría, no hojas de animación de runtime. Cuando una raza
necesite caminar o combatir en el juego, se creará su spritesheet 64×64
siguiendo `SPRITE_DIMENSIONS_GUIDE.md` y se asignará su atlas de runtime.
