# Qwen Code — cola de tareas del 19/10: los mapas del Ocaso y los escenarios que faltan
**Asignada:** 19/10 por Claude (coordinador), a petición de Oriol: «plantea las tareas necesarias a Qwen y ChatGPT para actualizar el juego con todas las nuevas decisiones y mejoras de mapa más campaña» · **Repo:** `MotorGraphico-main` · **Tipo:** trabajo nuevo, una rama por tarea. La cola del 15/10 (toda hecha) está en `orquestador/historial/qwen_15-10.md`.

> ## 21/10 — SIGUES: cola nueva (sustituye al «para» de esta tarde)
> **Base de tus ramas: `claude/integracion-21-10`** (sale de `main`): `git worktree add ~/onegai-ramas/qwen-AV19-11b -b agente/qwen/AV19-11b claude/integracion-21-10`.
> **Lo que ya no se hace:** AV19-09 (el Casco en llamas y la huida). En `main` ya estaban, y el Casco es **exactamente el Excel de Oriol** (comprobado celda a celda); tu versión no se integra. **`ciudad_centro_ocaso` no lo toca nadie**: es el Excel de Oriol más `tools/ocaso_oeste.py` (el barrio del Oeste, con el camino que Oriol dibujó). Las Afueras al ocaso se llaman **`afueras_boundington_ocaso`** (no `afueras_ocaso`).
> **Decisiones de Oriol del 21/10 que te afectan:** el Ocaso empieza en el Casco, como ahora, al hablar con Venides en la taberna; la Base Militar es **solo una escena del día 3 por la mañana** (el aviso a Venides y las campanas); la única salida de Boundington es (12..16,63), al sur del Oeste, y ahí es el combate final; **las Afueras van después** del combate final.
> **Antes de cada tarea:** `git log main claude/integracion-21-10 -- assets/levels/<nivel>.json`; si alguien lo tocó después de tu base, dilo en el INFORME y no lo rehagas.
>
> 1. **AV19-11b — la Base Militar, de día.** Trae `base_militar` de tu rama `agente/qwen/AV19-11` (nivel, TMX, `tools/av19_base_militar.py`, objetos de `boundington_av19_objetos.json`) a la base nueva. **Quita** `salida_base_al_oeste`, `salida_base_al_norte` y la `puerta_base_militar` del Casco en llamas: la base solo se abre desde `ciudad_oeste` (`puerta_cuartel` (8,11) → portón) y vuelve allí; `interior_cuartel` se abre desde el patio. Es de día: la guardia haciendo instrucción, los caballos, el herrero militar. Mantén las casillas reservadas del contrato (Venides, capitán en la torre a 2, Xila, Aigren, exploradores, tejado) porque Codex pone ahí la escena del aviso y las campanas. `validar_puertas`, `validar_mundo_unido`, `conectividad` en verde.
> 2. **AV19-10b — las Afueras al ocaso, después del combate final** (`afueras_boundington_ocaso`, 40×32, la de `main`). Se llega desde la salida (12..16,63) del Casco y se sale a `huida_boundington`. Lo que tiene que caber (de la ficha AV19-10, que sigue valiendo): **Edul retenido por tres guardias contra el abrevadero** (Edul no pelea: los guardias, sí), **Venides de rodillas junto al molino** con tres guardias (pon el molino), **los huertos** por donde entran Zarela a caballo y los gitanos, **un costado** por donde cargan los bandidos de Gorran, barricadas de carros con dos de **fuego blanco**, la plaza interior del **árbol grande** con 7×7 libres. Déjalo en Excel en la hoja «Otros» (`tools/ciudad_a_excel.py -o out/fable_ciudad02/boundington_ciudad10.xlsx`) para que Oriol lo corrija.
> 3. **AV19-12 — la mansión por dentro** y **AV19-13 — el decorado de las escenas**: como están abajo, sin cambios.
> 4. **AV19-14 — patrullas del Ocaso**: en el Oeste, **solo dentro del camino** de `tools/ocaso_oeste.py` (no abras atajos ni pises fuego ni barricadas); en el Casco, por sus calles.



Tu parte: **los mapas**. Lo que Oriol ha dibujado en Excel se mete en el juego tal cual, y los escenarios que la campaña necesita y hoy no existen se construyen con lo que ya hay (generadores, kit de objetos, validadores). **Nada de C++**, nada de `assets/adventures/` (Codex/ChatGPT lleva la campaña), nada de catálogos nuevos de objetos (si te falta una pieza, marcador con el id acordado y a la lista de Pixel).

## 0. Lo que ha cambiado desde tu última cola (léelo antes)

| Decisión de Oriol | Dónde está escrita | Qué te toca |
|---|---|---|
| 16/10: el jugador completa la trama de varias maneras; **todos los personajes están siempre en escena**; nada de texto que narre lo que nadie hace en pantalla | `claude/informe_e_escenas01.md` (y `out/claude_escenas01/README.md`: el modelo, la Barriada) | Cada mapa de escena lleva **sitio físico** para la gente de esa escena (banco, estrado, patio, pozo): tú pones el decorado y las casillas; los PNJ los coloca Codex con su parche encima de tu nivel |
| 18/10: **Oriol dibuja los niveles en el Excel** de toda Boundington (fuego, barricadas, enemigos `E`/`Ep`, PNJ). Cuando mueve una puerta o paso, **la casilla donde pone la `P` es desde donde se accede y no hay que recolocarla** | `out/fable_ciudad02/boundington_ciudad8.xlsx` (hojas «Ciudad», «Barrios», «Interiores», «Otros»); los PNG `*_oriol.png` de esa carpeta | El Excel manda sobre el generador. `tools/excel_a_nivel.py` / `tools/comparar_excel_ciudad.py` para meterlo y para dejar la lista de diferencias en la sesión |
| 18/10: **el Ocaso del día 3 son tres mapas** —el Casco en llamas, las Afueras al ocaso y el mapa de la huida— con **una ruta forzada que él marca** | mismo Excel, hoja «Otros» (`ciudad_centro_ocaso` 96×64, `huida_boundington` 40×40) + `ocaso_oriol.png`, `huida_oriol.png` | Tareas 1 y 2 |
| 18–19/10: la zona de Gorran son **cinco mapas** (sendero 48×48, Gola Fosca 38×38, aldea 64×64, galerías 38×38, cámara 38×38) y **los hace Fable entero** («no confío en el reparto para cosas de este calibre») | `claude/encargo_av19_07_aldea_gorran.md` | **No toques** `montana_gorran`, `antecova_gorran`, `cueva_gorran`, `galerias_gorran`, `camara_gorran` ni sus TMX |
| 19/10: la cripta de los Perdidos son cinco mapas (hechos por Fable, AV19-08) | `claude/informe_av19_08_cripta_secta.md` | No tocar `interior_iglesia_s1`, `dependencias_secta`, `santuario_subterraneo`, `camara_profunda` |
| 19/10: **dirección visual** (materiales por barrio, luz por franja, el Ocaso naranja y la huida azul) | `claude/boundington_direccion_visual.md` | Criterio para colocar fuego, farolas, hogueras y acentos: «luz con intención», «rastro de uso», «umbrales visibles» |

**Base de tus ramas:** `claude/AV18-cierre` (lleva la ciudad integrada, las afueras, la montaña y la cueva de AV19-04). Si Oriol ya ha fusionado `claude/escenas01` (la Barriada con todos en escena), sal de ahí. Compruébalo con `git log -1 --format=%s` y escríbelo en tu INFORME.

**Antes de empezar (obligatorio):** lo mismo de siempre: `orquestador/README.md` (el PROTOCOLO), `AGENTS.md`, una rama por tarea (`git worktree add ~/onegai-ramas/qwen-<TAREA> -b agente/qwen/<TAREA>`), compilar, `ctest -L gl-free`, `python3 tools/validar_todo.py` (0 errores: hoy hay 0 o 1 heredado de la daga de Aigren, ninguno más), `python3 tools/plano_nivel.py <nivel>` para la captura de cada mapa, jugarlo, sesión en `sesiones/`, fila del TABLERO. Preguntas a `BANDEJA_PRODUCT_OWNER.md` con PO-xx. Nunca push ni merge.

**Contrato con Codex (ChatGPT), que escribe la campaña encima de tus mapas:** cada tarea termina con una tabla en tu INFORME —**id del nivel · tamaño · puertas (objectId, casilla, destino) · casillas reservadas para cada personaje de la escena · zona de cada combate · qué objetos están «de día» y cuáles «al ocaso»**— porque sus cinemáticas andan por casillas concretas y `validar_cinematicas.py` las comprueba. Los ids de nivel de abajo **no se cambian**: ya están en los encargos de Codex.

---
## 1. AV19-09 — El Casco en llamas y la huida, del Excel de Oriol
Hoja «Otros» de `boundington_ciudad8.xlsx`: `ciudad_centro_ocaso` (96×64) y `huida_boundington` (40×40). Oriol los ha dibujado con fuego, barricadas, enemigos (`E` quietos, `Ep` patrullando), PNJ y la **ruta forzada** (`ocaso_oriol.png`, `huida_oriol.png`).
1. `ciudad_centro_ocaso` pasa de 64×64 a **96×64** tal cual lo dibujó (sustituye al de AV19-06; conserva el id, el `huida_a_egaroth` y los `fuego` como objetos, que ya tienen luz y humo de AV17-11). El fuego **cierra calles enteras**, de acera a acera (dirección visual §6), y la ruta buena queda legible desde lejos. Las barricadas de carros son el otro cierre.
2. `huida_boundington` (nuevo, 40×40): los Campos del Otoño de noche, sin fuego; hierba seca, camino de tierra, la ciudad como línea naranja al fondo (`fondos` en `cinematicas.json`: azul noche). Puerta de llegada desde las Afueras al ocaso (tarea 2) y salida al norte, hacia donde hoy lleva `huida_a_egaroth` (mantén ese destino; si el Excel marca otro, Excel).
3. Pasos: `ciudad_centro_ocaso` ↔ `afueras_ocaso` (tarea 2) por el oeste, donde Oriol pone la `P`; `afueras_ocaso` → `huida_boundington`. `validar_puertas`, `conectividad` (la ruta forzada tiene que ser **la única** que llega: comprueba con tu flood fill que los otros caminos están cerrados por fuego o barricada, no por el borde del mapa).
4. **Sitios de escena que la campaña necesita en el Casco ardiendo** (dales casilla aunque Oriol no los haya marcado, y dilo en la lista de diferencias): el **estrado** sobre las escaleras de la iglesia, con las telas grises ardiendo (Sandes, dos sectarios en portales, dos fanáticos en lo alto de las escaleras); la **esquina de la Taberna Humilde** con sitio para una docena de Perdidos y Rosita en medio de la calle; el **patio de los toneles** por el que Pulga te saca sin pasar por la plaza (un pasillo trasero real, cerrado por una puerta sin cerrojo); la **barricada de carros** a la salida oeste donde retienen a Verina (dos cultistas, ella atada: tres casillas libres delante). Velas en las ventanas de los suyos, cruces de hollín en las puertas de los demás: con los objetos que haya (`candelabro`, marcador `cruz_hollin` para Pixel).
5. Manifiesto de boundington, `lugares.json` (nombre y ficha de la huida), `repartir_botin.py` solo si Oriol marcó cofres.
**Entrega:** los dos niveles con TMX, capturas, la tabla del contrato con Codex, y la lista de diferencias con el Excel explicadas.

## 2. AV19-10 — Las Afueras al ocaso (`afueras_ocaso`)
Oriol no lo ha dibujado todavía (solo dijo que es el segundo de los tres mapas). Hazlo a partir de `afueras_boundington` (40×32) con las reglas del Ocaso y **déjaselo en Excel** («Otros», mismo formato) para que lo corrija él; si lo corrige, vuelve a meterlo.
- Mismo plano, otra ciudad: nada se mueve de sitio; cambian la luz, el fuego y quién está. Bancales pisoteados, establo abierto, el molino parado (hoy no hay molino: ponlo), **barricadas de carros, muebles y sacos** cruzando las calles, dos de ellas con **fuego blanco** (el de Edul; marcador `fuego_blanco` si AV17-11 no tiene variante).
- La **manzana cerrada por una barricada en llamas**, medio destrozada, de la que salen gritos, y detrás la **plaza interior con el abrevadero y el árbol grande**: ahí pasan Edul retenido por tres guardias, el combate final contra Sandes y la llegada de la compañía de Rosita. Reserva 7×7 libres alrededor del árbol.
- **El molino** con sitio para Venides de rodillas y tres guardias; **los huertos** por donde entran los guerreros de la Barriada con Zarela a caballo (un pasillo desde el borde sur); un costado por donde cargan los bandidos de Gorran.
- Salida al norte a `huida_boundington`; entrada desde `ciudad_centro_ocaso` (la `P` de Oriol) y desde `ciudad_surysal` si se llega por el oeste.
- Perdidos `Ep` patrullando rectos, en grupos de dos o tres, visibles de lejos; la gente, desaparecida o tirada (orden `tumba` de las rutinas: deja casillas, Codex pone los cuerpos).
**Entrega:** nivel, TMX, Excel en «Otros», captura, tabla del contrato.

## 3. AV19-11 — La Base Militar (`base_militar`), el patio donde empieza el Ocaso
Hoy el cuartel es `interior_cuartel` 18×13 con un sargento: no cabe la escena (Venides con veinte, diez o ningún hombre; el capitán en la torre; Aigren entrando con un carro; Xila contra el pozo; tres exploradores heridos que entran por el portón; saqueadores que saltan la muralla por detrás; fanáticos en el tejado de los barracones; las tres salidas: oeste, norte, quedarse).
- Nuevo nivel exterior **`base_militar` 36×32**, en el distrito militar junto al castillo: recinto de piedra oscura (`socol`) con muralla baja, **portón doble** al sur (desde `ciudad_oeste`, donde hoy está la puerta de `interior_cuartel`; el interior se conserva y pasa a abrirse desde el patio), barracones a un lado (con tejado al que se puede «subir» como casilla de altura 1: dos fanáticos), **patio** con armeros, muñecos de instrucción, el **pozo**, un abrevadero y caballos, la **torre de vigía con campana** al norte (casilla de altura 2 para el capitán), estandartes y banderas.
- Casillas reservadas (dilas en el contrato): Venides en el centro del patio; una fila de hasta 20 soldados detrás (dos filas de 10) que solo se verán por bandera; el capitán en la torre; Aigren con el carro junto al portón; Xila junto al pozo; tres exploradores que entran por el portón; **la muralla trasera** por donde saltan tres saqueadores (3 casillas de muralla «baja»); el tejado para dos fanáticos.
- Tres salidas reales del recinto: **oeste** (hacia la barricada de Verina → `ciudad_centro_ocaso` por la `P` de Oriol), **norte** (calles que arden: se sale pero el mapa de destino es el Casco por el lado del fuego), y quedarse (no es puerta).
- De día (días 1 y 2) el patio existe igual, con la guardia haciendo instrucción: sirve también para el **patio de armas** que la escena del castillo del día 2 pide (herrero militar herrando, soldados con lanzas de madera, abrevadero). Si el castillo y la base comparten patio, dilo y que sea uno solo.
**Entrega:** nivel, TMX, pasos en los dos sentidos con `ciudad_oeste` e `interior_cuartel`, captura, tabla del contrato.

## 4. AV19-12 — La fiesta dentro de la mansión (`mansion_barrio_alto` + `_p1`)
La fiesta hoy se narra en la calle. Oriol la quiere **jugada dentro**: unas cuarenta personas con máscaras, un salón con suelo de damero, escalera doble, un cuarteto junto a la chimenea, sirvientes con bandejas, el guardarropa, el jardín con setos recortados, fuente iluminada y faroles de papel, y arriba un pasillo de puertas cerradas y el **despacho** con el escritorio de patas de león.
- Amplía `mansion_barrio_alto` a **32×28**: verja con jardín delante (verja de hierro con hojas doradas: marcador `verja_dorada`), la **ventana por la que se trepa** (existe `salida_ventana_mansion`: déjala donde el jardín la alcance con `trepa`), el salón con damero (`marmol_real` o losa clara/oscura alternada si el tileset lo permite), la escalera doble (dos `escalera_mansion_p0_sube`), la chimenea con hueco para cuatro músicos, el guardarropa (cuarto pequeño con `perchero_ropa` ×4 y `baul`), la cocina por donde sale un criado.
- `mansion_barrio_alto_p1`: pasillo de puertas cerradas y el despacho (escritorio, atril, cofre); una ventana al jardín para salir corriendo si te pillan.
- Casillas libres para **40 invitados**: no los pongas tú (Codex los coloca por bandera `bnd2_fiesta_dentro`); deja el salón con sitio para 30 y el jardín para 10, y los sitios fijos de los anfitriones en la puerta, el noble que brinda junto a la chimenea, los mercaderes en la escalera, el seguidor con la copa, el cuarteto.
- Fuera, en `ciudad_este` de noche: farolas en todas las esquinas (dirección visual §4), **cortinas con luz** en las ventanas (marcador `ventana_luz`), el **cruce del cobrador** y la **esquina del vigilante** (casillas), y la calle donde Edul te espera.
**Entrega:** niveles, TMX, capturas, tabla del contrato.

## 5. AV19-13 — Decorado que las escenas ya hechas piden (una rama, varios niveles)
Pequeño y concreto; nada de esto cambia la geometría de las calles.
- **`ciudad_surysal`** (la Barriada, ESCENAS-01 ya está encima): un **círculo de piedras ennegrecidas** alrededor de la hoguera (24,9) (marcador `circulo_piedras` para Pixel si no hay prop), **cabras** (marcador `cabra`), **tendederos de colores** entre carromatos, y la **hoguera-forja** del forjador en (9,10) con un yunque al lado. No muevas nada de la empalizada x21–27, y5–18, ni a los PNJ que ya hay dentro.
- **`ciudad_centro`** (día 3 por la mañana): el **estrado de tablas** sobre las escaleras de la iglesia con **antorchas clavadas en fila** (las de Luisarda) como objetos que solo se ven el día 3 (Codex los condiciona con `apariciones`; tú dales objectId `estrado_sermon`, `antorcha_clavada`), sillas en los portales, cortinas abiertas con bordados grises (marcador).
- **`interior_taberna`**: el **hueco de la Furia** detrás de la barra con dos clavos (`hueco_espada`) y la espada colgada como objeto aparte (`furia_en_pared`, visible por bandera); la **tranca** de la puerta (`tranca`), el mapa con dos jarras en la mesa de Venides.
- **`interior_castillo`**: torre del capitán con **mesa de papeles, brasero y tintero volcado** (p1 o p2); si el patio de armas va en `base_militar` (tarea 3), aquí solo la puerta que lo conecta.
- **`interior_herreria`**: la fila de **frascos tapados con cera** en el banco (`frascos`), el estante de las treinta espadas vacío (aparece/desaparece por bandera: dos objetos, `armero_espadas` y `armero_vacio`), la red de Ceaseton descolgada.
- **`tienda_abandonada`**: maniquíes sin cabeza, el **espejo partido en tres**, baúles abiertos, polvo (marcadores si no hay).
**Entrega:** una sesión con la lista de objectIds nuevos (los marcadores) para Pixel y Codex.

## 6. AV19-14 — Patrullas y rutinas del Ocaso (después de 1, 2 y 3)
`assets/catalogs/rutinas_pnj.json` (es datos, está en tu zona para esto): los Perdidos del Ocaso patrullan rectos (`vigilar`/`patrulla`), los saqueadores de la muralla «esperan» fuera de vista, la gente tirada con `tumba`. Y en `ciudad_surysal` de noche: los guerreros de la Barriada con rutina de vigilar junto a la empalizada. Comprobación: `validar_rutinas_pnj.py` y que ninguna ronda pise fuego, barricada ni una puerta.

---
## Orden y ritmo
1 → 3 → 2 → 4 → 5 → 6. Las tareas 1 y 3 desbloquean a Codex (sus ESCENAS-04 y 05 esperan tus mapas); avísale en tu INFORME en cuanto cada una esté en su rama, con la tabla del contrato. Si una se bloquea por una pieza de arte, marcador y sigue.

## INFORMES (uno por tarea, aquí debajo)

### INFORME AV19-09 — el Casco en llamas 96×64, la huida 40×40 (y las Afueras al ocaso, para que las puertas resuelvan)

**Rama** `agente/qwen/AV19-09` · **base** `claude/escenas01` = **`bfc5a22`** (comprobado con `git log -1`: Oriol ya la
ha fusionado, así que salgo de ahí y no de `claude/AV18-cierre`) · **worktree** `~/onegai-ramas/qwen-AV19-09` ·
**sesión** [sesiones/2026-10-20-qwen-av19-09-ocaso-huida.md](../sesiones/2026-10-20-qwen-av19-09-ocaso-huida.md)

**@Codex: ESCENAS-05 (Verina, Rosita, Sandes en el Casco) y ESCENAS-06 (Afueras, final y huida) ya tienen mapas.** Los
tres niveles están en la rama con las casillas reservadas de abajo. `validar_cinematicas.py` sigue en 0 errores.

#### Los tres niveles

| nivel | tamaño | objetos | inicio | de dónde sale el suelo |
|---|---|---|---|---|
| `ciudad_centro_ocaso` | 96×64 | 1145 | (48,59) | `ciudad_oeste` (x 0..31) + `ciudad_centro` (x 32..95), con **169 casillas** cambiadas por el Excel |
| `afueras_ocaso` | 40×32 | 70 | (18,2) | `afueras_boundington`, con 10 casillas del Excel |
| `huida_boundington` | 40×40 | 21 | (37,22) | pintado entero desde el Excel (529 casillas; no hay mapa anterior) |

Generador nuevo **`tools/av19_ocaso.py`** (determinista, reejecutable, `--seco` / `--sonda x,y` / `--contrato` /
`--xlsx`). El suelo **no** se repinta desde los colores: se compone de los TMX vivos y solo se cambia la casilla donde
el Excel dice otra *clase*, así que las variantes reales de cada edificio (`edifici_o0..o3`, `rio_muro_*`,
`puente_pretil_*`, `adoquin_ajedrez`) se conservan. Los objetos salen de los comentarios de las celdas (1122 en el
Casco: 852 `fuego`, 72 carros, 51 Perdidos, `verina_joven` (90,19), `edul` (79,33), `huida_a_egaroth` (64,62)).

**`afueras_ocaso` se construye en esta tarea aunque sea la 2**, porque las puertas del Casco y de la huida apuntan a
él: sin el nivel, `validar_puertas` y `validar_mundo_unido` no pasan y la rama queda roja. AV19-10 lo amplía con lo que
pide su ficha (molino, barricadas con fuego blanco, la plaza 7×7 del árbol, los huertos) y lo deja en Excel para Oriol.

#### Contrato con Codex

**Puertas** (todas comprobadas: la casilla se anda, se alcanza desde el inicio y la llegada no es otra puerta):

| objectId | casilla | destino | llegada |
|---|---|---|---|
| `paso_ocaso_a_afueras` | (62,63) (63,63) (64,63) (65,63) (66,63) | `afueras_ocaso` | (16,1) (17,1) (18,1) (19,1) (20,1) |
| `paso_afueras_a_ocaso` | (16,0) (17,0) (18,0) (19,0) (20,0) | `ciudad_centro_ocaso` | (64,61) (65,61) (66,61) (66,61) (66,61) |
| `paso_afueras_a_huida` | (1,17) (1,18) (1,19) (1,20) (1,21) | `huida_boundington` | (38,18) (38,18) (38,19) (38,20) (38,21) |
| `paso_huida_a_afueras` | (39,18) (39,19) (39,20) (39,21) (39,22) | `afueras_ocaso` | (2,18) (2,19) (2,21) (2,21) (2,21) |
| `fin_de_la_huida` | (1,13) | — (marcador) | `eventId: huida_a_egaroth`, el de siempre |

La ruta forzada es **Casco → puerta sur (62-66,63) → Afueras → borde oeste (1,17-21) → huida por el este (39,18-22) →
oeste → (1,13) Egaroth**. El fuego y los carros cierran el resto: inundando con el fuego y los carros abiertos se llega
a 1006 casillas en las Afueras y a 1385 en la huida, y lo que queda cerrado lo cierran ellos, no el borde del mapa.

**Casillas reservadas por personaje** (libres y alcanzables; el generador falla si dejan de estarlo):

| sitio | decorado que pongo | casillas para Codex | zona de combate |
|---|---|---|---|
| Estrado del sermón (escaleras de la iglesia) | `estrado_sermon` (70,49) (70,50); `tela_gris_ardiendo` (69,48) (69,51) | **Sandes** (71,50) · **2 fanáticos** (70,48) (70,51) · **2 sectarios en portales** (71,47) (71,52) | plaza de la iglesia: x 66..76, y 46..54 |
| Esquina de la Taberna Humilde (puerta en (48,58); el jugador empieza en (48,59)) | — | **Rosita en medio de la calle** (48,60) · **12 Perdidos** (46,59) (47,59) (49,59) (50,59) (45,60) (46,60) (47,60) (49,60) (50,60) (46,61) (47,61) (48,61) | la calle de la taberna: x 44..55, y 59..62 |
| Patio de los toneles (Pulga) | `barriles_pila` (44,52) (45,52) (46,52); `barril` (47,53); `puerta_sin_cerrojo` (44,56) | **Pulga** (45,54) · **jugador** (46,54) · huecos (44,55) (47,54) | el patio: x 44..51, y 52..55 |
| Barricada de Verina | `carro` (87,19); `carro_o1` (87,20) — Oriol pone a **Verina en (90,19)** y el fuego cierra la calle al este | **2 cultistas** (89,19) (90,20) · **3 casillas libres delante de ella** (88,19) (88,20) (89,20) | el bolsillo de Verina: x 86..90, y 18..21 |
| Velas y cruces | `vela_ventana` (46,56) (47,56) (49,56) (50,56) (72,47) (73,48) · `cruz_hollin` (46,58) (50,58) (52,55) (53,56) (72,49) (73,52) | van **sobre casilla de edificio** (por eso son `flotante: true`): no estrechan ninguna calle | — |

**El pasillo trasero de Pulga es real**: patio x 44..51, y 52..55 → pasillo (44,56)-(44,59) → la calle de la taberna en
y=59. No pasa por la plaza del mármol (x 55..62), y el generador comprueba en cada pasada que el pasillo está libre.

**De día / al ocaso:** los tres niveles son **solo del día 3 al ocaso**; no llevan `disponibleDia` porque no se pisan
antes. Los mapas de día (`ciudad_centro`, `ciudad_oeste`, `afueras_boundington`) no se tocan.

**PNJ que ya están en el nivel** (los puso Oriol en el Excel, como objetos): `verina_joven` (90,19), `edul` (79,33),
32 `perdido_saqueador`, 19 `perdido_fanatico`, 2 `perdido_devoto`. Los que falten van por bandera encima de las
casillas reservadas.

#### Marcadores para Pixel (10 ids nuevos en `boundington_av19_objetos.json`)

Esperan arte: **`estrado_sermon`**, **`tela_gris_ardiendo`** (spriteId 3 provisional), **`vela_ventana`**,
**`cruz_hollin`** (spriteId -1, invisibles hoy), **`puerta_sin_cerrojo`** (spriteId 23). Los cuatro `paso_*` son
invisibles a propósito (`spriteId -1`, como los de `boundington_pasos.json`) y `fin_de_la_huida` reutiliza el sprite 23
y el `eventId` de `huida_a_egaroth`.

#### Cinco decisiones donde el Excel manda sobre la ficha

1. **El paso a las Afueras está al SUR** (62-66,63), no «por el oeste» como decía la ficha: ahí pone Oriol las cinco
   `P`, y la regla del 18/10 es que la casilla de la `P` no se recoloca.
2. **La salida de la huida está al OESTE** (1,13), no al norte; y la llegada desde las Afueras entra por el **este**
   (39,18-22). «Si el Excel marca otro, Excel».
3. **El id es `afueras_ocaso`**, no `afueras_boundington_ocaso` (que es como lo llaman el Excel y el trabajo anterior de
   Fable). Los ids de nivel no se cambian: están en los encargos de Codex. El generador traduce el alias.
4. **La ficha de la huida va en `calles`, no en `lugares`.** `tests/Game/LugaresTests.cpp` exige que toda ficha de
   `lugares` sea un `interior_*` y que haya exactamente una por puerta de interior, sin sobrantes: la primera pasada la
   puso ahí y rompió dos comprobaciones. En `calles` sí, que es donde ya está el Casco en llamas.
5. **El botín del Excel pasa a decorado** (`botin_ciudad_barril`→`barril`, `botin_ciudad_saco`→`sacos`,
   `botin_afueras_cofre_40`→`cofre`, `botin_afueras_escondite`→`juncos`; mismo `blocksMovement`, ningún paso cambia). El
   presupuesto del día 3 ya está entero comprometido por las instancias con `disponibleDia: 3` que `repartir_botin.py`
   puso en los mapas **de día**. **Registrado como PO-57**, con la recomendación de que `repartir_botin.py` reparta el
   día 3 en los mapas del Ocaso: hoy ese botín está en mapas que el día 3 no se pisan. No ejecuto `repartir_botin.py`
   (es la herramienta de Codex y redistribuiría los 51 niveles).

#### Diferencias con el Excel

169 casillas de suelo en el Casco, casi todas en dos sitios: **la iglesia ya no está** (de 77 casillas de
`viejo_iglesia`, Oriol deja 10 sueltas entre adoquín, casas, grava y ruinas: 30+28+9 cambios, más 10 de ruinas a
adoquín y 8 al revés; por eso el estrado va delante de una ruina) y **la muralla sur se abre donde está la `P`**
(`viejo_muralla -> sendero_tierra` en (12..16,63) y (66,63), `adoquin_gastado -> sendero_tierra` en (62..65,63): sin eso
las cinco puertas caen dentro de la muralla). El resto son ajustes finos: la columna (93,43..53), el patio de adoquín
delante de la iglesia, el muelle bajo el ★ en (48,59), y el puente/muro del río en (1,46..48) que pasa a tierra (el
bloque no lleva ninguna puerta a `ciudad_surysal`, así que el Ocaso queda cortado de la Barriada por el oeste).
Afueras: 10 casillas, todas `roca -> pradera/sendero` en el borde oeste, por donde sale el paso a la huida. Además,
**6 llegadas de puerta movidas** porque el Excel las escribía encima de otra puerta (el jugador rebotaría de un mapa a
otro en bucle): ninguna herramienta del proyecto lo comprobaba.

Dos huecos andables dentro de la ruina de la iglesia, (75,49) y (76,50), no se alcanzan ni quitando el fuego: por eso
los sectarios van a los portales de la plaza y no ahí.

#### Verificación

- `validar_todo.py`: **2 errores, los dos heredados** (comprobado ejecutándolo en el árbol principal, que está en
  `bfc5a22`: `validar_pnj` por seis PNJ de la Barriada sin fila propia, y la daga de Aigren que la ficha ya daba por
  conocida). 22 avisos, los de siempre.
- `validar_puertas.py`: 126 niveles, 365 puertas, **0 errores**, 12 avisos ajenos. `conectividad.py`: **0 errores**
  (99 % / 100 % / 100 %). `validar_mundo_unido.py`: en verde tras añadir los dos niveles nuevos a EXCLUIDOS con su
  motivo. `validar_botin.py`: 58 instancias, **0 errores**. `validar_cinematicas.py`: 0 errores.
- **`ctest -L gl-free`: 222 pasan, 14 fallan, 0 «Not Run».** Los 14 son **exactamente los mismos en la línea base**:
  revertí mis seis ficheros versionados, aparté los cinco nuevos, resincronicé `copiar_assets` y volví a ejecutar solo
  esos 14 en el mismo build. Ninguno es de esta tarea. Dos apuntes de entorno: `test_nivel_excel`,
  `catalog_content_validation` y `test_recompensas_economia` son Python y ctest los lanza con `/opt/homebrew/bin/python3`,
  que no tiene `openpyxl` (PEP 668 no deja instalárselo sin `--break-system-packages`, y no lo he hecho); y
  `test_lugares` pide 17 fichas cuando la base ya tiene 18.
- **No los he jugado en el motor**: son datos nuevos sobre el binario de la base. La comprobación es la de los
  validadores, el flood fill del generador y las capturas.
- Capturas: `out/planos/ciudad_centro_ocaso.png`, `afueras_ocaso.png`, `huida_boundington.png`.

#### Dos trampas nuevas (las dos mordieron de verdad)

1. **La colisión no se puede sacar de la unión de todos los TMX.** `test_map.tmx`, `mazmorra_64x64.tmx` y
   `ejemplo_mapa.tmx` declaran `hierba_seca` infranqueable: con la unión de `assets/maps/` entero, los Campos del Otoño
   salían con 274 casillas andables de 1600 y las cinco puertas de la huida inalcanzables. La tabla se hace con
   `terreno_iso.json` + solo los TMX del manifiesto de boundington.
2. **Un catálogo que solo añade no se corrige nunca.** La primera pasada escribió `cruz_hollin` sin `flotante`; cuando
   se lo añadí, `escribir_catalogo` vio que el id ya estaba y no lo tocó, y `conectividad.py` siguió en rojo dos pasadas
   más. Ahora reconcilia: añade lo que falta **y** sustituye lo que ha cambiado.

#### Desviación de protocolo: un worktree para toda la cola

La ficha pide un worktree por tarea. El repo pesa 17 GB, un checkout ~1,4 GB y un build ~0,5-1 GB, y el disco está al
**95 % (23 GB libres)**: seis worktrees con su build no caben. Uso **uno** con ramas **encadenadas** en el orden de la
cola (AV19-11 saldrá de `agente/qwen/AV19-09`) y un solo build reutilizado (`out/build/qwen-19-10`); como no toco C++,
las tareas siguientes no recompilan. Se fusionan en el orden de la cola.

**Trabajo ajeno encontrado, no tocado:** en `assets/levels/` hay `.bak` sin versionar de todos los niveles, y entre
ellos un `ciudad_centro_ocaso.json.bak` de 1122 objetos y `huida_boundington.json.bak` /
`afueras_boundington_ocaso.json.bak`: es el mismo bloque del Excel aplicado por alguien el 19/10 a las 13:53 sobre un
TMX que seguía siendo de 64×64 (el primer objeto cae en x=81, fuera del mapa), y por eso no carga. Mi versión parte del
Excel, no de esos `.bak`. `_to_delete/` sigue en pie. **Entorno:** `openpyxl` y `numpy` instalados con `pip --user` para
el `python3` del sistema (sin ellos no arrancan ni `tools/excel_a_nivel.py` ni `validar_pnj.py`); reversible con
`pip uninstall`.

### INFORME AV19-11 — `base_militar` 36×32, el patio donde empieza el Ocaso

**Rama** `agente/qwen/AV19-11` (encadenada sobre `agente/qwen/AV19-09` = `18fec4e`) · mismo worktree y mismo build ·
**sesión** [sesiones/2026-10-20-qwen-av19-11-base-militar.md](../sesiones/2026-10-20-qwen-av19-11-base-militar.md)

**@Codex: ESCENAS-04 (el sermón y la Base Militar) ya tiene el patio.** Está en `tools/av19_base_militar.py --contrato`,
que imprime la tabla de abajo; es reejecutable.

#### El nivel

`base_militar` **36×32 exterior**, 28 objetos, **888 casillas pisables**, inicio (18,29) dentro del portón, 3 regiones
de altura. Recinto de `viejo_muralla` con **portón doble al sur**, **salida al oeste** y **salida al norte**; fuera del
anillo, `roca`, para que nadie salga a pasear por el borde del mapa. Patio de `patio_fortificado` con dos caminos de
adoquín (del portón al centro, y el transversal de la instrucción).

**Las alturas ya son de verdad:** en AV19-03 el campo `alturas` existía pero el binario lo ignoraba; ahora lo leen
`LevelLoader`, `GameSession`, `GameSessionTablero` e `IsometricRenderer` (AV19-02 entró).

- **Barracones** (oeste): el tejado es una **meseta a nivel 1**, x 4..11, y 4..11. El motor no apila casillas, así que
  un bloque no puede ser a la vez barracón que bloquea y tejado pisable: se sube por cualquier lado (un nivel se pasa,
  y en combate cuesta el doble) y la pared la dibuja la altura, que es lo que dice FORMATO_NIVELES §Altura.
- **Torre de vigía con campana** (norte): zócalo de `viejo_militar` que bloquea (x 16..20, y 3..7) y **adarve a nivel 2**
  (x 17..19, y 4..6); se entra por el hueco (18,7) y su peldaño (18,8), los dos a nivel 1. `campana_torre` en (19,5).
- **Patio**: pozo (14,18), abrevadero (24,12), tres caballos, cuatro armeros con armas, tres muñecos de instrucción,
  cuatro banderas, la **forja del herrero militar** (28,20) con su yunque (29,20) y el carro de Aigren (19,27).
- **Muralla trasera baja**: (34,14-16) con `muralla_baja`, que no bloquea: por ahí saltan los saqueadores.

#### Contrato con Codex

| objectId | casilla | destino | llegada |
|---|---|---|---|
| `porton_base_militar` (doble) | (17,30) (18,30) | `ciudad_oeste` | (8,12) |
| `salida_base_al_oeste` | (1,16) | `ciudad_centro_ocaso` | (3,12) |
| `salida_base_al_norte` | (10,1) | `ciudad_centro_ocaso` | (8,3) |
| `puerta_cuartel_patio` | (12,10) | `interior_cuartel` | (2,10) |
| `puerta_base_militar` *(en el Casco)* | (8,11) | `base_militar` | (17,29) |

Las tres salidas reales del recinto son **oeste** (hacia la barricada de Verina), **norte** (las calles que arden) y
**quedarse**, que no es puerta. Ojo: la barricada de Verina está donde la puso Oriol en el Excel, en **(87..90,19)** del
Casco, no al oeste del mapa; desde la salida oeste se llega andando por la ciudad en llamas.

**Dos puertas de siempre redirigidas** (el interior se conserva y pasa a abrirse desde el patio, como pedía la ficha):
`ciudad_oeste.puerta_cuartel` (8,11) → `base_militar` (17,29), e `interior_cuartel.salida_interior_cuartel` (1,10) →
`base_militar` (8,13).

**La puerta de vuelta al Casco no la podía traer el Excel** (`base_militar` no existía cuando Oriol dibujó). Sin ella,
`validar_puertas` avisa y `validar_mundo_unido` dice **«sin retorno al mapamundi» de los tres mapas del Ocaso**, porque
la base los mete en el grafo del mundo al apuntar a ellos. Va en `PUERTAS_EXTRA` de `av19_ocaso.py`, en (8,11): justo
donde el mapa de día tiene `puerta_cuartel`.

**Casillas reservadas** (libres, pisables y alcanzables desde el inicio; el generador falla si dejan de estarlo):

| quién | casilla | altura |
|---|---|---|
| **Venides, en el centro del patio** | (18,16) | 0 |
| **el capitán, en la torre** | (18,5) | **2** |
| **Xila, junto al pozo** | (15,18) | 0 |
| **Aigren, junto al carro del portón** | (16,27) | 0 |
| **tres exploradores que entran por el portón** | (17,28) (18,28) (19,28) | 0 |
| **dos fanáticos en el tejado** | (6,6) (9,9) | **1** |
| **tres saqueadores que saltan la muralla trasera** | (33,14) (33,15) (33,16) | 0 |
| **herrero militar herrando (día 2)** | (27,20) | 0 |
| **20 soldados en dos filas de 10** (solo por bandera) | y=13: x 13..22 · y=14: x 13..22 | 0 |
| **soldados con lanzas de madera (día 2)** | (22,18) (23,18) (24,18) (22,19) | 0 |

**Zonas de combate:** el patio de la arenga (x 12..24, y 12..20) · el portón (x 14..22, y 25..29) · la muralla trasera
(x 30..33, y 12..18) · el tejado, altura 1 (x 4..11, y 4..11) · la torre, altura 2 (x 17..19, y 4..6). Las patrullas y
las rutinas de PNJ **no cruzan acantilados** (FORMATO_NIVELES §Altura), así que las de la torre y el tejado van aparte.

**De día / al ocaso:** el patio existe igual los tres días, y **el patio de armas del día 2 es este**: la escena del
castillo (herrero herrando, soldados con lanzas de madera, abrevadero) se juega aquí, no en `interior_castillo`. AV19-13
solo añade en el castillo la puerta que conecta, si Oriol quiere que también se entre desde dentro. El nivel no lleva
`disponibleDia` porque no tiene botín nativo.

**12 marcadores nuevos** en `boundington_av19_objetos.json`: las cinco puertas (`porton_base_militar`,
`salida_base_al_oeste`, `salida_base_al_norte`, `puerta_cuartel_patio`, `puerta_base_militar`) y, esperando arte,
**`muralla_baja`**, **`abrevadero`**, **`caballo`**, **`armero_armas`**, **`muneco_instruccion`**, **`campana_torre`**,
**`forja_militar`**, **`lanza_madera`**.

#### Una trampa desactivada de paso

`tools/av19_niveles.py::NIVELES_AV1906()` devolvía `[ocaso()]`, o sea el `ciudad_centro_ocaso` de **64×64** de AV19-06.
Cualquiera que ejecutara `gen_niveles_av19.py` **reescribía el mapa de Oriol de 96×64 y sus 1145 objetos con la versión
vieja**, sin fallar ni avisar. Ahora devuelve `[]` con el motivo escrito; `ocaso()` se queda como histórico.

#### Verificación

- `validar_mundo_unido.py`: **todo se alcanza y de todo se vuelve** (117 niveles, 369 puertas). `validar_puertas.py`:
  127 niveles, 371 puertas, **0 problemas nuevos**. `conectividad.py`: `base_militar` 888/888 (100 %), 0 errores.
- `validar_todo.py`: **2 errores, los dos heredados** (PNJ de la Barriada y la daga de Aigren), igual que en la base.
- Guardas propias: ni una reserva en colisión ni tapada, ni dos objetos en una casilla, ningún acantilado entre casillas
  pisables, y flood fill **contando la altura**: las 13 reservas, las 20 casillas de soldados, las 4 de lanzas y las 5
  puertas se alcanzan todas desde el inicio.
- Los tres fallos que encontró la primera pasada (torre y tejado solapados, una bandera tapando a un soldado, y la guarda
  de acantilados denunciando el zócalo de la torre) están en la sesión.
- Captura: `out/planos/base_militar.png`. **No lo he jugado en el motor**: las alturas deberían verse, pero la
  comprobación es la de los validadores y el flood fill.


