# Ejemplos del motor

Proyecto de pruebas separado de la campaña de Boundington (D-05 / DEC-05).
En el título del juego y en el selector del editor aparece como **Ejemplos
del motor**. También se abre con `./level_editor --proyecto ejemplos`.

La galería usa `ejemplo_mapa.tmx` y contiene:

- Un arbusto, una llave, una poción, un éter y un slime del fixture original.
- Un guía con diálogo y un tendero con dos consumibles.
- Un cartel de negocio y una puerta a la mazmorra de rendimiento de 64×64.

Las cuatro categorías del motor (prop, pickup, enemy y npc) están presentes.
Los valores de la tienda y del negocio son ejemplos de prueba, sin valor
canónico. La galería se puede ampliar desde el editor. La mazmorra conserva
sus datos originales y también se puede elegir desde el árbol del proyecto.

Los archivos `test_objects.json` y `mazmorra_64x64.*` permanecen en sus rutas
para no romper los demos y las regresiones que los usan. La separación la
establece el manifiesto: Boundington ya no los declara. Los nuevos IDs de
interacción llevan el prefijo `ejemplos_`; no se repiten IDs entre los dos
catálogos de este proyecto.

Al abrir un proyecto, la paleta carga únicamente sus catálogos, en el orden
declarado y desde `settings.catalogsDirectory`. F7/P usa esas mismas rutas.
Un proyecto sin catálogos queda vacío y su playtest usa `empty_objects.json`.
El modo suelto del editor, sin proyecto, mantiene el fixture de pruebas.
