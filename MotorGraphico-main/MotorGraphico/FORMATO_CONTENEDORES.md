# Contenedores y botín — AV18-02

En un objeto de assets/objects, `contenedor` activa el comportamiento; ausente conserva el objeto antiguo. Tipos: cofre, barril, saco, armario, cadaver, escondite. Campos: `tabla` (id de contenedores_tablas.json), `presupuesto` (0..1.000.000), `garantizados` (hasta 64 ids) y `requisitos` opcional.

`requisitos.percepcion` y `requisitos.cd` son **dados del rival, de 0 a 8**, conforme a Ed. 3; el nombre cd se conserva del encargo, no designa una dificultad estática. `abrir` admite vacío, ganzua, fuerza o llave:<id>. Ganzúa exige llevar el objeto `ganzua` y tira DES/Juego de Manos; fuerza usa CON/Atletismo. La percepción usa la carta percepcion, o INT si no está. El empate no abre ni revela; éxito o crítico sí. Las llaves no se consumen. Fallar una cerradura permite intentarlo otra vez; un escondite hace una sola prueba por partida al pasar a distancia Manhattan 2 o menor.

Tabla: raíz `tablas`, con id único; cada tabla trae `tiradas` (0..64), `oro`: [mínimo,máximo] y `entradas`: [{id,valor,peso,unico}]. Valor y peso positivos. Region y tipo son metadatos para la validación de contenido. Los garantizados se descuentan primero; si no caben se rechaza la apertura, sin conceder nada. Después se tira el oro que cabe y se eligen solo objetos que caben. No se rebaja el precio de un objeto caro para meterlo en el presupuesto. Los precios positivos del catálogo de objetos deben coincidir con el valor de tabla.

Las variables `botin.semilla` y `botin.<nivel codificado>.<instancia codificada>.*` guardan contenido, oro, Percepción y apertura. La instancia es eventId si existe; si no, prefab más casilla. Para que mover un cofre entre revisiones mantenga su identidad hay que darle eventId. Los únicos se reservan en banderas globales de partida al generarse. Dos garantizados que pretendan reservar la misma única son un error de contenido. Una única garantizada debe figurar en las entradas de esa misma tabla con `unico: true`; el validador rechaza garantías que el generador no podría registrar como únicas.

El guardado ya serializa estas variables/banderas. No se añaden campos a SaveGame ni se sube la versión: las partidas antiguas, sin estado de botín, siguen cargando. Cambiar de nivel o cargar no rellena lo cogido. La semilla se fija al actualizar el primer nivel con contenedores y después viaja en la narrativa.

E abre el papiro existente; Coger todo transfiere los objetos y el oro restantes. Cerrar conserva el contenido. La API permite coger una unidad, aunque el papiro ofrece Coger todo. Las puertas pisadas conservan su prioridad. Los escondites no descubiertos tienen escala visual cero y no se abren; al descubrirlos se restaura la escala original y aparece el aviso en el registro.

Escena manual de prueba (no añadida a la campaña):

```bash
./build-release/juego --nivel tests/fixtures/contenedores/interior_cofre.json --catalogo assets/objects/ciudad_objetos.json --catalogo-extra tests/fixtures/contenedores/objetos.json
```

El fixture usa el interior de taberna y un cofre al lado del inicio; no cambia el interior publicado. La tabla prueba_cofre solo demuestra la integración. Las tablas de campaña y sus presupuestos se completan en AV18-03 y los niveles los coloca Qwen en AV18-05. No se activan todos los barriles decorativos del juego.

## Tablas publicadas y colocación (AV18-03 → AV18-05)

`botin_objetos.json` selecciona 50 reutilizables (14 existentes, 16 materiales/tesoros nuevos y 20 puentes a equipo común existente) y 12 únicas comunes de 40–60. `botin_boundington.json` añade materiales y contenedores. Las únicas también viven en `libreria_armas.json`, para que el contrato global de equipo siga cubierto. Los 20 puentes reutilizables de botin_boundington usan el id RPG sin prefijo, distinto del item_ antiguo: así conservan el precio RPG efectivo sin redefinir un objeto publicado. Las únicas prestan su `linkedSkill` al iniciar la mesa si están equipadas y tienen la etiqueta `BotinBoundington`; la ficha no aprende la carta. Quitarse la pieza la quita del siguiente combate. Cada carta es una acción principal de un uso por combate. No se activan las antiguas piezas con linkedSkill de otros catálogos.

Hay 30 tablas `botin_<region>_<tipo>`: regiones ciudad/interiores/cripta/afueras/cueva y los seis tipos. Hay 26 prefabs contenedor con esos mismos ids. No hay prefab de cadáver en interiores (presupuesto de combate cero), ni barril/saco/armario en cripta (entorno cero); sus tablas existen, pero crear esas instancias exige antes redistribuir presupuesto. Las tablas caras contienen equipo, aunque solo se selecciona si cabe en el presupuesto concreto. Una única figura en una sola tabla y su registro de partida impide repetirla.

Cada spawn colocado por Qwen lleva `eventId` único dentro del nivel y metadatos editoriales:

```json
{"objectId":"botin_ciudad_cofre","eventId":"botin_plaza_01","position":{"x":10,"y":10},"botin":{"dia":1,"region":"ciudad","canal":"cofres_visibles","orden":10}}
```

`dia` es la primera disponibilidad (1/2/3), no rellena el cofre cada día. `orden` permite al simulador comprobar compras antes de premios futuros. Canales: cofre→cofres_visibles, escondite→tesoros_ocultos, cadáver→combates, resto→entorno. La suma de presupuestos de TODAS las instancias de una región/día/canal no puede superar ECONOMIA.md; ciudad/interiores agregan todos sus mapas. No dupliques el presupuesto en cada edificio. El prólogo se carga contra el día 1. El `botin` es metadato de autoría y no modifica ObjectSpawn/SaveGame.

`tools/validar_botin.py` rechaza ids, precios y estados desconocidos, tablas incompletas, únicas repetidas, garantizados imposibles e instancias sin disponibilidad o sobre presupuesto. Cero instancias da aviso: confirma datos, no confirma la economía jugable. No poner curación imprescindible ni llaves de historia detrás de Percepción. El juego de `ganzuas` ya publicado sirve para el requisito ganzua.

Estados futuros quemado/congelado/aturdido/silenciado siguen marcados en botin_objetos.json. Los doce efectos actuales ejecutan marcado, cegado, sangrado, caída, limpiar, curar, ventaja, penalización de dados y escudo que crece al acertar golpes. No prometen empujar, quemar ni efectos al rematar sin ejecutor.

## Simulación (AV18-04)

Desde el motor: `cmake --build build-release --target simular_contenedores -j8`, después `python3 tools/simular_economia.py --json`. `--semilla` es el valor persistido de botin.semilla (1..1.000.000.000), no el estado entero del RNG de combate. El proceso C++ carga los mismos catálogos y llama a generaContenedor; Python no reproduce el algoritmo ni el azar. Cada perfil tiene estado propio para sus únicas. La selección es entera y por presupuesto del canal; no se fracciona un cofre. Compras a orden 90, reserva narrativa a 95: un premio posterior no puede pagarlas. `--ajuste -25|0|50` permite comparar reputación en compras.

La herramienta lee las cestas del bloque económico, los tenderos realmente colocados y sus precios explícitos, las tablas y las instancias con disponibilidad. Vende unidades enteras al 25 %, mínimo 1; conserva el resto como opciones y lo lista. La reserva narrativa separa efectivo previsto, sin fingir que se ejecutó una escena. Los éxitos de servir son hipótesis del perfil, con pago/duración del catálogo; su integración sigue en AV18-07. No suma todos los grantGold de todas las ramas de aventuras: los cuenta en auditoría como fuentes aún sin recorrido. No imputa a niveles vacíos el presupuesto completo del documento.

Salida 0: todas las bolsas del recorrido modelado están en margen; 2: simulación ejecutada, calibración pendiente; 1: error de datos/generador. La auditoría sigue diciendo qué fuentes faltan modelar y que PO-49 requiere montaje real; el estado del simulador no confirma la economía del runtime. Sin reparto AV18-05 quedan los nueve márgenes rojos. No se bajan los márgenes para disimular la ausencia de recompensas.
