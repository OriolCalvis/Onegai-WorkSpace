# 17/09/2026 — E-07 cierre: referencias externas + la prueba de los seis defectos

- **Commits:** MotorGraphico `30ca8c4` (2 ficheros, hunks propios).
- **Deja esperando a Oriol:** nada.

## Qué se cerró

E-07 quedaba ABIERTO tras el rescate: la rodaja versionada validaba la coherencia interna del guion y su cabecera documentaba la mitad pendiente («necesita los catálogos del proyecto»). Esta tanda la cierra:

- **`ValidateAdventure(script, refs)`** con `AdventureCatalogRefs {skills, monsters}` — solo catálogos del lado RPG (el puente de D-08 resuelve ObjectCatalog→MonsterCatalog antes de llegar; motor_rpg sigue GL-free puro). Puntero nulo = familia sin comprobar; el overload base delega con todo nulo.
- **Diagnósticos nuevos** (Error, con el beat como ubicación): skillCheck con skillId ausente del SkillCatalog; startBattle con monsterId ausente del MonsterCatalog.
- **La prueba de la ficha**: aventura con SEIS defectos conocidos → seis diagnósticos con su ubicación (id duplicado, combate sin enemigo, tirada sin habilidad, timer no declarado, objetivo sin bandera, timer de duración cero), cada uno con su beat/objetivo/timer en `eventId`. `test_adventure_validation` **19/19**.

## Coordinación y epílogo de la tanda interrumpida

- La tanda del 14/09 quedó cortada a medias: la DECLARACIÓN del overload viajó sola en el push `b80dd43` y la definición quedó sin terminar. Al reanudar (17/09), además, mi inserción del 14 estaba DUPLICADA (el ancla aparecía dos veces y mi `str.replace` no llevaba `count`) — eliminada una copia. Este commit deja declaración y definición juntas.
- `AdventureValidation.cpp` está compartido EN VIVO con la sesión EVT-05 de opencode (interruptores locales, su fila): sus hunks de eventos quedan fuera del commit; solo entran los míos (región del overload, al final del fichero).
- Falsabilidad: la mutación que apaga el check de skills la caza el test; la del timer-cero quedó sin demostrar limpiamente (binario razeado por la contención en vivo del fichero) — registrada como pendiente menor.

## Verificación

Clon limpio de HEAD+parche (solo mis hunks): configure y build del target **0 errores**, `test_adventure_validation` **19/19**. En el árbol vivo (con EVT-05 dentro): 19/19 igualmente.
