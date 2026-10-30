# H2-08-PRECIOS — el precio del equipo sale del catálogo real

**Estado: NO APLICADO.** EST-23b ya aterrizó (y reparado el 28/09), así que el
bloqueo que lo citaba ya no existe: el parche y sus pruebas de abajo están
listos para aplicarse. **No se ha tocado ni una línea de `GameSessionComercio.cpp`.**

## Prerrequisito resuelto el 28/09 (esto NO es el parche)

Antes de tocar precios salió un bloqueo **mayor** que EST-23b: **`setInventoryRules()` no se
llamaba en ningún sitio del motor**. `InventoryRules::equipment` era `nullptr`, así que
`equipmentFor()` devolvía `nullptr` **siempre** — equipar y vender respondían «no hay
catalogo de equipo cargado» y ninguna pieza era reconocible como equipo. Dos de los tres
rojos de `test_application_cableado` eran justo eso (`inventoryRules().equipment != nullptr`,
`equipmentFor("item_aguja_del_firmamento") != nullptr`); el comentario de
`Application.cpp:406` (H0-04) daba por hecho que lo inyectaba `loadLevel`, y no lo hacía.

Arreglado en `src/Engine/ApplicationNivel.cpp`, junto a `setConditionCatalog`:
`inventoryRules.equipment = &m_equipmentCatalog`. **Solo `equipment`**; `playerClass` y
`tiers` siguen a `nullptr` como estaban (cablearlos es otro item). `test_application_cableado`
pasa de 8/11 a 10/11, y las diez suites de sesión/inventario/negocio quedan en 0 fallos.

**Lo que sigue sin aplicarse es el parche de precios de abajo.** Decisión pendiente antes
de abrir el fichero: `basePriceOf()` vive hoy en `GameSession.cpp` (el de 14 paquetes) y el
destino natural es `GameSessionComercio.cpp`.

## El hallazgo

`EquipmentDefinition::price` no lo lee nadie en `src/Game/`. Los 413 precios de
`assets/catalogs/equipment.json` (100, 175, …) son datos generados que no llegan
a la economía del juego.

La ruta de precio real es `GameSession::basePriceOf()`, y hoy solo mira
`ObjectDefinition::pickup.price`, o sea el número que el nivel escribe a mano en
el objeto. Y ese número **no existe para el equipo**: en todos los catálogos de
objetos y niveles hay **0** objetos de equipo con `pickup.price`.

De ahí sale el daño, que es lo que hace esto Urge:

- **282 de las 413 piezas son alcanzables** como objeto (143 desde
  `loot_tables.json`, 74 desde `classes.json`, 38 desde `pregen_characters.json`,
  27 desde `monsters.json`).
- Esas vías no pasan por el catálogo de objetos: llegan por `loot` (H2-05),
  equipo inicial de clase o personaje pregenerado. En el objeto no hay precio.
- Precio 0 en la sesión significa dos cosas: `resolveShopOffers` **descarta**
  el artículo («a coste 0 sería oro infinito») y `sell` responde «eso no tiene
  precio aquí». Es decir: **el botín real de H2-05 no se puede vender.**

Esto no es un preciosismo: es el eslabón que faltaba entre H2-05 y el comercio.

## El parche (25 líneas, sin miembros nuevos)

`InventoryRules::equipment` ya lo inyecta H0-04, así que no hace falta tocar
`Application.cpp/.h` ni añadir nada a `GameSession.h`.

**1.** `src/Game/GameSessionComercio.cpp`, nueva definición justo antes de
`resolveShopOffers` (el cuerpo sale de `GameSession.cpp`, cuyo único consumidor
es este fichero — así no se pisa con EST-23b):

```cpp
// Precio de base de un objeto: la mitad del comercio (comprar y vender lo
// usan los dos) y el unico sitio donde tiene sentido.
//
// El precio de una pieza de EQUIPO es el del CATALOGO REAL
// (assets/catalogs/equipment.json: 413 piezas con precio generado), no el
// numero que el nivel escriba a mano en el objeto. No es un detalle de
// tienda: 282 de esas piezas llegan al jugador por vias que no pasan por el
// catalogo de objetos —el botin de los monstruos (H2-05), el equipo inicial
// de las clases, los personajes pregenerados— y en el objeto no hay
// `pickup.price`: valen 0, y 0 es «eso no tiene precio aqui», o sea, botin
// que no se puede vender. El precio del objeto del nivel queda como plan B
// para lo que no sea equipo.
int GameSession::basePriceOf(const std::string& objectId) const {
    if (const RPG::EquipmentDefinition* eq = equipmentFor(objectId)) {
        if (eq->price > 0) return eq->price;
    }
    const ObjectDefinition* def = m_catalog != nullptr ? m_catalog->find(objectId) : nullptr;
    return def != nullptr ? def->pickup.price : 0;
}
```

**2.** Borrar la definición antigua de `basePriceOf` en `src/Game/GameSession.cpp`
(solo se usa en `GameSessionComercio.cpp:246`, dentro de `sell`).

**3.** `src/Game/GameSessionComercio.cpp:44`, en `resolveShopOffers`:

```cpp
-        const int price = item.price > 0 ? item.price : itemDef->pickup.price;
+        // El precio del PNJ manda; si no lo pone, el de base: que ya
+        // consulta el catalogo real de equipo y solo cae al
+        // `pickup.price` del objeto para lo que no sea equipo.
+        const int price = item.price > 0 ? item.price : basePriceOf(item.objectId);
```

Cadena final: **precio del PNJ > precio del catálogo real > precio del objeto**.
`equipmentFor()` es de H0-04, igual que lo usa `repartirBotin` de H2-05.

> Si al aplicar esto EST-23b ya ha movido los miembros a `SessionData`, la línea
> del `def` pasa a `sessionData()->m_catalog->find(objectId)` (así ha quedado en
> el resto del fichero).

## La prueba (2 casos en `tests/Game/GameSessionTests.cpp`)

Fixture `makeCatalogJoyeria()`: dos piezas de equipo **sin** precio en el objeto,
una poción que no es equipo y sí lo tiene (40), y un tendero que le pone precio
(250) a una de las dos piezas.

`MOTOR_TEST_CASE(el_precio_del_equipo_sale_del_catalogo_real)`:

1. La oferta del anillo es **100** (el del catálogo). Antes valía 0 y la tienda
   lo descartaba: `shopOffers().size()` era 1 en vez de 3.
2. La poción sigue a **40**: lo que no es equipo no cambia.
3. El colgante vale 80 en el catálogo y el tendero lo vende a **250**: el precio
   del PNJ manda.
4. `sell(0)` paga 100 * 60 / 100 = **60**. Antes devolvía `false`.

Falsabilidad: sin el hunk 3 el anillo no entra en la oferta y el caso falla en la
primera comprobación; sin el hunk 1 el `sell` falla. Con `pickup.price` a mano
en el objeto, el anillo valdría 0 y el caso también falla (que es el bug).

## Por qué no se ha aplicado (actualizado 28/09)

Las dos razones que había ya **no existen**:

1. `GameSessionComercio.cpp` estaba en vuelo por EST-23b. Aterrizó (`f128e4a`).
2. `motor_level` no compilaba con EST-23b en el aire. Ya compila, y el fallo que
   tenía (la lista de inicialización vaciada, ver la fila EST-23b del tablero de
   estabilización) está reparado: `test_game_session` 24/24.

Lo único que queda es la **decisión de sitio**: `basePriceOf()` está hoy en
`GameSession.cpp`, el fichero de 14 paquetes, y el destino natural es
`GameSessionComercio.cpp` (donde ya vive la resolución de ofertas de la tienda).
Esa decisión es de quien vaya a abrir el fichero, y por la ley del repo el testigo
de un fichero grande lo lleva uno solo.

Lo que sí se ha hecho: el hallazgo está medido y es reproducible con los comandos
del barrido (`grep -rlw` sobre las claves de cada catálogo, y el recuento de
piezas alcanzables). Queda también el segundo rostro del mismo bug en
`src/Game/InventoryMenuState.cpp:56` (`pickup.price <= 0` → `ItemCategory::Quest`):
una pieza de equipo sin precio se clasifica como objeto de misión en la mochila.
No se toca: `InventoryMenuState` está en el carril de G12 (Codex).

## Lo que el barrido descartó antes (y por qué, para que no se repita)

| Candidato | Por qué no |
|---|---|
| H2-08 consumibles | `consumables.json` (31) es **prosa pura**: cero `healHp`/`clearOneCondition`/… ejecutables, solo `effect.description`. Cablearlo sería inventar semántica. |
| `conditions.json: cureConditions` | 13 de 16 son frases («Superar la tirada de liberación»), no una lista de condiciones. |
| `equipment.json: restrictions` | «Precisión: DES», «Habilidad de Oro Viejo: …» — prosa, no `requiredClass`. |
| H2-04 tiers/hitos | `grantMilestoneOnEnd` está vacío en todos los assets y `CharacterSheet::tierUp()` no tiene llamantes: sin dato, cablearlo es otra ruta muerta. |
| H0-08 eventos de mapa | El código **ya está entero** (el intérprete se crea en `ApplicationNivel.cpp:314`, arranca los eventos y corre cada frame en `ApplicationBucle.cpp:186`), pero **ningún nivel declara `mapDocument`** y la capa de eventos de `mision_llave.json` tiene **0 eventos** publicados. Es un bloqueo de contenido, no de código: el generador (`demo_mision_llave`) no compila porque su `.cmake` exige `TARGET motor_slots` y ese target no existe. |
| H1-04 creación de personaje | 2,5 j cruzando `Application` (escena nueva) + Render/HUD (14 pasos) + `Player::populate_from_class_race_background`, en zonas de Codex. |
