# Backlog de sprites necesarios

Este documento decide **qué arte falta para terminar niveles jugables**.
No es una lista de todos los IDs del catálogo: hoy hay 3.359 props, 924
pickups, 892 enemigos y 378 PNJ, pero generar una imagen distinta para cada
ID produciría ruido, duplicados y un editor peor. Se crea una **familia
visual reutilizable** y cada objeto decide su variante, escala, nombre y
propiedades desde el catálogo o el spawn.

Contrato de medidas y anclajes: [SPRITE_DIMENSIONS_GUIDE.md](SPRITE_DIMENSIONS_GUIDE.md).
Qué atlas es runtime y cuál solo sirve de ayuda: [SPRITE_AUTHORING_GUIDE.md](SPRITE_AUTHORING_GUIDE.md).

## Regla de decisión

| Si el elemento... | Entonces necesita... | No crear... |
| --- | --- | --- |
| Se pisa | Tile de suelo 128×64, rombo 2:1 | Un prop plano o un actor aplastado. |
| Bloquea y sobresale | Prop alto o tile alto anclado abajo | Un suelo 2:1 con una casa dibujada. |
| Habla, patrulla o pelea | Actor 64×64, pies abajo, idle + caminar | Un icono editorial como sprite final. |
| Se recoge | Icono/prop pequeño visible y variante de datos | Una hoja distinta por cada sabor de poción. |
| Solo organiza el editor | Icono editorial | Un asset de runtime. |

## Lo que ya existe y qué significa

| Recurso | Útil ahora | Limitación que hay que respetar |
| --- | --- | --- |
| `terreno_iso.png` | 40 suelos runtime | No contiene edificios altos ni reemplaza automáticamente las ciudades. |
| `ciudad_tileset.png` | Ciudades heredadas y colisiones | Es una familia urbana antigua; no mezclar sus GID con `terreno_iso` sin migración. |
| `race_npc_idle.png` | 43 PNJ raciales idle runtime | Son estáticos: falta caminar para los protagonistas y PNJ frecuentes. |
| `personaje.png` | Jugador y fallback temporal | No representa raza, profesión ni criatura final. |
| `editor_enemy_library_v1.png` | Elegir enemigos en el editor | Preview; no es arte animado de partida. |
| `editor_*` | Crear y catalogar contenido | Nunca asignarlo directamente como arte final de juego. |

## P0 — imprescindible para un vertical slice de Boundington

Estas familias permiten terminar taberna, bosque y alcantarillas sin usar
placeholders. Cada actor: 64×64, `idle` y `walk_down/up/left/right`, cuatro
frames por dirección. Cada prop: PNG con alpha, anclado por la base.

**Avance:** la primera hoja idle ya existe en
`assets/textures/boundington_story_actors_idle.png`; cubre Luisarda, Ben,
Griffin, Perdidos, carcelero, niña, guardia y mercader. Su contrato de
frames está en [assets/textures/boundington_story_actors.md](assets/textures/boundington_story_actors.md).
Lo que sigue pendiente para esos mismos actores es caminar, no otra lista de
retratos estáticos.

### Actores narrativos (12 familias)

| Familia | Variantes de datos, no hojas nuevas | Usos |
| --- | --- | --- |
| Aventurero jugador | 4 tintes / equipo | Protagonista y prueba de cámara. |
| Luisarda | normal, alarmada | Inicio de Los Perdidos. |
| Ben Kafka | guía, dubitativo | Misión de los niños. |
| Griffin | trabajo, herido | Alcantarillas. |
| Parroquiano / mercader | joven, mayor, viajero | Taberna y mercado. |
| Guardia de Boundington | patrulla, alerta | Puertas y zonas de riesgo. |
| Carterista perdido | neutral, huyendo | Encuentro urbano. |
| Cultista perdido | saqueador, fanático, carcelero | Cadena Los Perdidos. |
| Niña / niño | niño perdido, niña local | Rescate y vida urbana. |
| Saga del bosque | normal, sacrificio | Jefe de Ben Kafka. |
| Naga | exploradora, venenosa | Alcantarillas de Griffin. |
| Duende de porcelana | máscara A/B/C | Hostigador de ciudad y secta. |

### Enemigos de combate (8 familias)

| Familia | Variantes de catálogo | Mínimo de animación |
| --- | --- | --- |
| Slime | verde, ácido, grande | idle, salto, daño. |
| Murciélago | cueva, vampírico | volar, ataque. |
| Zombi / draug | draug, musgoso | idle, caminar, golpe. |
| Arlequín | duende, líder | idle, caminar, ataque. |
| Lobo corrompido | normal, esquelético | idle, caminar, mordisco. |
| Vampiro | noble, hambriento | idle, caminar, golpe. |
| Serpiente / naga hostil | agua, veneno | idle, desplazamiento, ataque. |
| Dragón (jefe futuro) | fuego, hielo | Solo cuando haya combate de jefe; no priorizar antes. |

### Props de nivel (16 familias)

| Familia | Variantes | Función editorial |
| --- | --- | --- |
| Puerta y salida | madera, reja, piedra, alcantarilla | Conecta mapas; debe leerse a distancia. |
| Cofre | cerrado, abierto, misión | Botín y recompensa. |
| Cartel | taberna, mercado, peligro, misión | Orientación; texto va en HUD, no dentro del sprite. |
| Antorcha / farol | apagado, encendido | Luz y lectura de caminos. |
| Mesa | redonda, larga, mercado | Taberna e interior. |
| Silla / taburete | madera, roto | Relleno de interiores. |
| Barra de taberna | corta, larga | Centro social. |
| Estantería | libros, botellas | Biblioteca, tienda y alquimia. |
| Cama | simple, rica | Posada y casa. |
| Barril / caja / saco | 3 variantes | Modularidad de mercado. |
| Pozo / fuente | seco, agua | Plaza y punto de interés. |
| Alcantarilla | tapa, abierta, rota | Entrada de Griffin. |
| Jaula | vacía, niños, rota | Bosque y rescate. |
| Altar oscuro | limpio, ritual | Sectarios y jefe. |
| Hongo / raíz corrupta | pequeño, grande, tóxico | Bosque de la saga. |
| Huesos / cadáver | humano, lobo, antiguo | Señal ambiental, no pickup. |

### Pickups e iconos de mundo (10 familias)

`pocion` usa `variant` y `effectOverride`; no hacer cuatro objetos idénticos.

1. Poción: curativa, maná, veneno, misión.
2. Comida: pan, pastel, ración.
3. Llave: hierro, celda, ritual.
4. Carta / nota: rumor, misión, prueba.
5. Moneda / bolsa.
6. Gema / componente arcano.
7. Libro / grimorio.
8. Arma en suelo: espada, daga, bastón.
9. Armadura / escudo.
10. Hierba / seta recolectable.

### Suelos y bordes P0 (20 tiles)

- **Naturaleza:** pradera limpia, pradera flores, tierra, sendero, barro,
  bosque-hojarasca, musgo, roca, arena, nieve, pantano, agua somera, río.
- **Boundington:** adoquín limpio, adoquín gastado, plaza, madera de
  taberna, suelo de mercado, piedra de alcantarilla, ruina húmeda.
- Para cada agua o borde visual: centro + borde exterior + esquina. No
  pintar una línea azul sobre césped: se percibe como un error de capa.

## P1 — expansión que aporta variedad sin romper la producción

### Estructuras altas (14 familias)

Muros de piedra/madera, muralla, valla, seto, puerta grande, árbol de
bosque, árbol seco, roca alta, toldo de mercado, puente, escalera, torreón,
estatua y ruina vertical. Requieren prop alto o soporte de tile alto: **no
forzarlas a 128×64**.

### Fauna y criaturas neutrales (10 familias)

Perro, gato, rata, cuervo, caballo, ciervo, jabalí, cabra, pez y mariposa.
Solo perro, rata, cuervo y ciervo necesitan caminar inicialmente; las demás
pueden usar idle hasta que tengan una función jugable.

### Variantes culturales de ciudad (12 familias)

Puesto de mercado, banco, herrería, sastrería, joyería, biblioteca, templo,
baños, universidad, ópera, coliseo y cuartel. Cada una se forma con las
familias P0; crear una fachada completa por comercio solo cuando la cámara
o la historia lo justifique.

## P2 — esperar a tener mecánica que los use

- Monturas, invocaciones, dragones completos y animales exóticos.
- Un sprite singular para las 43 razas: ya tienen idle base; primero
  terminar animación de las razas que aparezcan en campaña.
- Equipos por ranura, rareza y facción: usar iconos y tintes hasta que el
  inventario tenga pantalla y equipamiento visible.
- Decoración microscópica (vasos, cubiertos, 30 modelos de flor): solo
  añadirla al cerrar una zona concreta.

## Qué retirar de la lista de producción

1. Cualquier `editor_*` como objetivo de runtime: es una referencia, no
   una entrega de arte.
2. Un PNG por `objectId`: una familia + variantes de datos es más legible.
3. Edificios convertidos a suelo 2:1: rompen colisión, profundidad y la
   transparencia de paredes.
4. Iconos con texto embebido: se vuelven ilegibles con zoom y traducción.
5. Enemigos sin rol, mapa ni encuentro: crear antes la misión y el spawn.

## Orden de producción recomendado

1. Puertas, suelo de Boundington y los cuatro PNJ de taberna.
2. Perdidos, duendes de porcelana, cultistas y altar.
3. Ben Kafka, niños, saga, bosque corrupto y jaulas.
4. Griffin, nagas, ratas, tapas y tuberías de alcantarilla.
5. Cofres, pickups y señalización; después fauna y estructuras altas.

Antes de aceptar una familia: comprobar escala a 50/75/100/150/200% en el
editor, abrir el nivel con `P`, y validar que no use un atlas `editor_*` en
partida.
