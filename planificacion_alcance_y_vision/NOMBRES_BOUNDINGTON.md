# AV15-02a — Nombres de los sitios de Boundington

Propuesta de Codex, 14/10/2026. **Revisada por Oriol el 14/10 a las 13:01 (PO-39 a PO-41): aprobada con dos cambios, ya puestos en la tabla.** Cubre los
17 interiores distintos a los que llevan las puertas de `ciudad_centro.json`,
`ciudad_este.json` y `ciudad_oeste.json`. Una fila por interior; varias puertas
pueden llevar al mismo sitio.

El nombre propio se muestra primero; debajo, la función. El emblema es un objeto
concreto que permita reconocer el uso en cinco segundos. Claude aplicará el
mismo nombre y emblema al cartel, minimapa, tienda y diálogo en AV15-02.

Fuentes: [encargo](../orquestador/codex.md),
[D0015, punto 14](../decisiones/D0015-respuestas-bandeja-po-14-10.md),
[usabilidad, §3](USABILIDAD_UOC.md) y
[canon de la campaña, §2–3, §6 y §11–13](../MotorGraphico-main/MotorGraphico/planificacion/CANON_BOUNDINGTON_CAMPANA01.md).
Los nombres nuevos están aprobados para los rótulos; este documento no añade
historia ni responsables que las fuentes no documenten. «No consta» significa que las fuentes consultadas no nombran al responsable.

| Sitio / interior | Nombre propio | Función en el cartel | Emblema: un objeto | Quién lo lleva, según las fuentes | Base del nombre |
|---|---|---|---|---|---|
| Banco · `interior_banco` | La Caja Firme | banco y cambio | Moneda | No consta. | Propuesta. |
| Baños / termas · `interior_banos` | Las Aguas Claras | baños y descanso | Tina con agua | No consta. | Propuesta. |
| Biblioteca · `interior_biblioteca` | El Libro Abierto | consulta de libros | Libro abierto | No consta. | Propuesta. |
| Castillo · `interior_castillo` | Castillo de Boundington | corte y audiencia | Torre | No consta; el capitán del castillo no tiene nombre en el canon (§6). | Conserva el nombre propio del nivel. |
| Coliseo · `interior_coliseo` | La Arena Roja | arena de combate | Casco de gladiador | No consta. | Propuesta. |
| Herrería · `interior_herreria` | Lingote de Oro | forja y reparación | Yunque | Aigren, hijo de Ceaseton (§3, D1-02). Edul lo sustituye en el juego el segundo día si Aigren se fue (§12–13). | Canon D1-02 (confirmado por Oriol, PO-40). |
| Iglesia · `interior_iglesia` | Iglesia de la Luz | culto y oración | Campana | No consta quién regenta el templo; la presencia de los Perdidos no acredita propiedad. | Conserva el nombre del nivel, sin atribuirle una deidad nueva. |
| Joyería · `interior_joyeria` | La Perla | joyas y gemas | Anillo | No consta; el joyero del juego no tiene nombre canónico (§6). | Propuesta. |
| Mercado · `interior_mercado` | El Peso Justo | víveres y provisiones | Cesta con pan | No consta. Sandes trabaja en el mercado de Killville (§6), no se le atribuye este. | Propuesta. |
| Ópera · `interior_opera` | El Ruiseñor | ópera y teatro | Laúd | No consta. | Propuesta. |
| Posada · `interior_posada` | Taberna Rica | cama y comida | Cama | No consta. | Canon D1-12; Oriol (PO-41): la posada es la Taberna Rica. |
| Sastrería · `interior_sastreria` | La Boutique de Madame | sastre y tela | Carrete de hilo | Luisarda, hija de Boundington (§3, D1-03; §6). | Canon D1-03; Oriol (PO-40) corrige D0015 §14: «Lingote de Oro» es la herrería. |
| Taberna · `interior_taberna` | Taberna Humilde | bebida y comida | Jarra | Skilla en [C01] (§2 y §6); Rosita en las escenas actuales (§11–13). El ID del juego sigue siendo `skilla`. | Nombre canónico conservado. |
| Universidad · `interior_universidad` | La Casa del Saber | estudio y enseñanza | Pergamino | No consta. | Propuesta. |
| Casa de té · `interior_casa_te` | La Hoja Verde | té y reposo | Tetera | No consta. | Propuesta. |
| Armería · `interior_armeria` | El Buen Filo | armas y equipo | Espada | No consta. | Propuesta. |
| Cuartel · `interior_cuartel` | La Casa de Guardia | guardia y alojamiento | Escudo | No consta quién manda aquí; no se identifica con el capitán del castillo. | Propuesta. |

Las palabras «de la Luz» ya están en el nivel de la iglesia; conservarlas no
afirma qué dios se veneraba allí. Los nombres y emblemas de cada puerta que
comparte destino deben coincidir: por ejemplo, `puerta_sastreria` y
`puerta_ropa_sur` llevan a `interior_sastreria`. Los carteles actuales distinguen
algunos negocios del sur aunque sus puertas usan el mismo interior; no se
inventan sucursales ni se cambian esos enlaces en esta tarea.

## Revisión resuelta por Oriol (14/10, 13:01)

- **PO-39:** tabla aprobada con las dos correcciones siguientes. El laúd y el pergamino se comprobarán con la prueba de cinco segundos cuando haya dibujo.
- **PO-40:** Lingote de Oro es la herrería de Aigren. Luisarda conserva **La Boutique de Madame**, carrete y «sastre y tela». Corrige el ejemplo de D0015 §14.
- **PO-41:** `interior_posada` es la **Taberna Rica**: «cama y comida», emblema cama.

Las respuestas están en §0 de [BANDEJA_PRODUCT_OWNER.md](../BANDEJA_PRODUCT_OWNER.md) y en la corrección de [D0015](../decisiones/D0015-respuestas-bandeja-po-14-10.md). Ya no quedan dudas de nombre ni ubicación pendientes en esta tabla.
