# U4-07.E01 · Auditoria de particulas y contrato v1 del emisor generico

Estado: **documento de E01; no autoriza ningun cambio de codigo de producto.** Base `628f417`
(rama `replica-r1/U4-07`, sobre `replica-r1/U3-07`: ya trae `Interpolacion`/`EstadoTween` y `Camera`
integrada). Todas las rutas son relativas a `MotorGraphico-main/MotorGraphico/`. Cada afirmacion lleva
`fichero:linea` abierto en esta sesion y comprobado por contenido (registro:
`out/fabrica/REPLICA-R1/U4-07.E01_citas.log`, en la raiz del repo). Lo que no tiene cita va marcado
**(inferido)**. Sala limpia: nada de Unity copiado; el diseno parte del codigo del repo y de
`planificacion/CANON_PARTICULAS.md`. Nombres propuestos en espanol, como el repo (la ficha
`U4-07_ficha.md` usa ingles como placeholder: `parseEmitter`, `ParticleBuffer`...).

Leyenda: **E** = escrito; **I** = integrado en ejecutable; **T** = con test.

## 0. Resumen (10 lineas)

1. El juego **ya tiene** un sistema de particulas completo, de **funcion cerrada del tiempo** (sin estado): `Game::destellosDeBrillo` calcula posicion y alfa de cada particula con un hash determinista, dado un instante (`src/Game/Personajes/BrilloEquipo.cpp:205-358`). Lo usan el brillo del equipo (`AuraJugador`) y las cartas/golpes (`EfectosDeCartas`).
2. Lo reutilizable y que **no se reimplementa**: la tabla de 40 familias (`TipoDeParticula`, `FamiliasDeParticulas.cpp:14-73`), las formas en pixeles (`pixelesDeForma`/`contornoDeForma`), el DTO `Game::Destello` (`BrilloEquipo.h:91-97`) y la pintura `Render::pintaDestellos` + `MezclaAditiva` (`PintaDestellos.cpp:18-64`).
3. Existe un unico sistema **con estado**, ad hoc y acoplado a entidades (las particulas de las mascotas, `MascotesNivel`, P20); ninguno es generico. Lo que **falta** y es el paquete: un emisor **con estado** (taxa con residuo, buffer acotado, edad/vida, semilla propia) que el sistema actual no tiene; los emisores de hoy no se pueden "pausar", ni acotar, ni reproducir tras una pausa.
4. **No hay presupuesto** (ni en cartas ni en mascotas, cuyo numero de particulas lo fijan los datos, `VidaNivel.cpp:247`): `EfectosDeCartas::m_activas` es un `std::vector` sin tope (`EfectosDeCartas.h:59`, `PintaDestellos.cpp:89`) y `SpriteBatch::m_vertices` crece sin tope (`SpriteBatch.cpp:69-75`; 6 vertices x 32 B por pixel de particula). Este paquete fija el primero.
5. **Reloj de pared**: las particulas actuales usan `steady_clock` en `render` (`PintaDestellos.cpp:88,94-100,103`) e ignoran la pausa del critico; el emisor nuevo debe avanzar con el `dt` de update (decision D-O6).
6. **RNG**: `RPG::Xoroshiro128p` existe (`RandomEngine.h:46-85`) pero **su interfaz solo da `roll_d6` y `roll_in_range(int,int)`**, y los primeros `next_double` **y `roll_in_range`** de semillas pequenas **coinciden** (calculado: seeds 1, 2, 3 -> mismo `next_double` inicial; ver R3). Hay que mezclar la semilla (splitmix64) y descartar los primeros sorteos.
7. Hay 2 RNG no deterministas/estaticos cerca (no son particulas, no se tocan): `static std::minstd_rand temblor(7)` (`ApplicationCinematica.cpp:495`) y `std::random_device` (`:363`).
8. Ficheros >500 lineas que la integracion rozaria y solo pueden adelgazar: `Application.cpp` (1282), `CMakeLists.txt` (1639), `BrilloEquipoTests.cpp` (716); al borde: `ApplicationPantallas.cpp` (497).
9. Primer consumidor real propuesto: migrar `EfectosDeCartas::salpica` (chispas/polvo/sangre de los golpes, `ApplicationPantallas.cpp:433-442`) -> decision de producto D-O1.
10. E02-E06 pueden empezar sin decision de Oriol (solo D-T del Brain); E07 necesita D-O1+D-O3; E08 necesita D-O2 y D-O5.

## 1. Auditoria

### 1.1 Particulas y efectos parecidos existentes

| # | Que | Donde | Modelo / unidades | Estado |
|---|-----|-------|-------------------|--------|
| P1 | **Tabla de familias** `TipoDeParticula`: `vy`,`vx` en **px/s** (vy<0 sube), `vida` en **s**, `forma`, `suma` (luz que suma / tapa), `movimiento`, `colores`, `densidad`, `otraForma`, `conOtraForma`, `gira` | `include/Game/Personajes/FamiliasDeParticulas.h:30-41`; datos `src/Game/Personajes/FamiliasDeParticulas.cpp:14-73` (40 entradas; canon: 40 familias, `CANON_PARTICULAS.md:17`) | `tipoDeParticula(nombre)` **nunca falla**: un nombre desconocido devuelve `chispas` (`FamiliasDeParticulas.cpp:78-82`, test `BrilloEquipoTests.cpp:93`) | E I T |
| P2 | Enums `FormaDeParticula` (29 formas) y `MovimientoDeParticula` (8: Sube, Cae, Deriva, Converge, Orbita, Expande, Chispazo, Salpica) | `FamiliasDeParticulas.h:17-25`, `:29` | enum | E I T |
| P3 | Formas en pixeles: `pixelesDeForma` (1..16 pixeles por forma: Punto 1, Calavera 16, Engranaje 12) y `contornoDeForma` (borde oscuro de 14 formas de tema) | `FamiliasDeParticulas.h:54`, `:58`; `FamiliasDeParticulas.cpp:93`, `:192-222` | `{dx,dy,peso}` en pixeles del frame de 64x64 (`MontajePersonajes.h:63`) | E I T (`BrilloEquipoTests.cpp:466-499`) |
| P4 | **Simulacion de brillo** `destellosDeBrillo(hoja, frame, segundos, out)`: sin estado, hash `mezcla/azar01`; particulas por fuente `cuantas = int(e.particulas*vida*densidad)+1` | `include/Game/Personajes/BrilloEquipo.h:99`; `src/Game/Personajes/BrilloEquipo.cpp:205-358` (bloque de particulas `:252-356`, `cuantas` `:254`, switch de movimiento `:300-353`) | tiempo en **s** (double); posicion float en **pixeles del frame** | E I T (`BrilloEquipoTests.cpp:210-217,239,332,395,549,639`) |
| P5 | Hash determinista local: `mezcla(a,b,c)` y `azar01` en **namespace anonimo** (no reutilizable sin extraerlo) | `BrilloEquipo.cpp:17-30` | uint32 -> float [0,1) | E I |
| P6 | Escalera de intensidad: armas 0..18 part/s (`kEscalera`), ropa x0.4, abalorio `0.8+0.5*nivel` part/s | `BrilloEquipo.cpp:47-59`, `:62-68` (ropa), `:72-80` (abalorio; `:78`) | particulas **por segundo** | E I T |
| P7 | DTO de salida `Game::Destello{x,y,alfa,forma,suma,color}` y `FuenteDeBrillo`/`HojaDeBrillo` | `BrilloEquipo.h:91-97`, `:57-69`, `:74-78` | float px del frame | E I T |
| P8 | **Cartas jugadas / golpes**: `destellosDeCarta(familia,color,semilla,edad,out)` monta una hoja sintetica (abalorio +8, `parte=3`, 17x13 px de cuerpo) y llama a `destellosDeBrillo`; `kDuracionDeCarta=1.4 s`; entrada 0.08 s y salida 0.4 s por alfa | `include/Game/ParticulasDeCartas.h:54`, `:59-60`; `src/Game/ParticulasDeCartas.cpp:76-103` (hoja `:82-95`, llamada `:97`, fundidos `:98-102`) | `semilla` uint32 desordena con `0.61*(semilla%97)` s (`:97`) | E I T (`BrilloEquipoTests.cpp:667-716`) |
| P9 | Mapa carta->familia/color (`ParticulasDeCartas::cargar`, JSON `assets/textures/particulas_cartas.json`), `familiaDeElemento` | `include/Game/ParticulasDeCartas.h:26-45`, `src/Game/ParticulasDeCartas.cpp:25-40`, `:59-74` | datos; `Result<>` | E I T |
| P10 | **Pintura** `pintaDestellos(batch,destellos,esquina,tam,blanco)`: 3 pasadas (contorno oscuro, lo que tapa, lo que suma con `MezclaAditiva`); un `batch.submit` **por pixel de forma** | `include/Render/PintaDestellos.h:19-25`, `:28-29`; `src/Render/PintaDestellos.cpp:18-26`, `:28-64` (submit `:44`,`:58`) | escala `tam/64` px de mundo por px de frame (`:32-33`); coordenadas redondeadas (`:42-43`) | E I (GL: sin test) |
| P11 | **Cola de efectos** `Render::EfectosDeCartas`: `IRenderable` con `lanza`/`salpica`/`olvida`/`aLaVista`; `Activa{familia,color,semilla,esquina,tam,inicio}`; **`m_activas` sin tope**; poda y edad con `steady_clock` en `render` | `include/Render/EfectosDeCartas.h:24-64` (`Activa` `:51-57`, `m_activas` `:59`, `getSortKey` `:48`); `PintaDestellos.cpp:67-107` (`salpica` `:79-91`, poda `:94-100`) | edad **s** de pared | E I |
| P12 | Aura moral: `particulaAura(aspecto,i,t)` motas/volutas (seno + `semilla(i,canal)` por `sin`), `chispaAla` | `src/Game/AuraMoral.cpp:15-19`, `:261-287`; tipos `include/Game/AuraMoral.h:88`, `:109`, `:176` | `float` s; `AuraJugador::renderMotas` dibuja con quads propios (`AuraJugador.cpp:145-156`) | E I T. **Otro sistema, no se reutiliza** (la ficha lo prohibe: "no usar aura com a simulacio general") |
| P13 | Brillo del equipo en pantalla: `AuraJugador::render` -> `destellosDeBrillo` + `pintaDestellos`; reloj `segundos()` = `steady_clock` | `src/Render/AuraJugador.cpp:54-56`, `:138-142` | float s de pared | E I |
| P14 | Efecto de golpe en el cuerpo (`EfectosDeGolpe`): tinte/desplazamiento, **no son particulas**; pausa del critico `kPausaCritico=0.07` | `include/Game/EfectosDeGolpe.h:28-66` | `edad` float s con `avanza(dt)` | E I T (`tests/Game/EfectosDeGolpeTests.cpp`) |
| P15 | Numeros flotantes de dano: `GolpesEnPantalla::lanza/avanza` con `kVida` | `include/HUD/Pergamino/GolpesEnPantalla.h:23`, `:36`; `src/HUD/Pergamino/GolpesEnPantalla.cpp:21-24` | `edad += dt` y poda | E I T (`tests/HUD/GolpesEnPantallaTests.cpp`). Patron de **estado con edad y poda** ya presente (texto, no particulas) |
| P16 | Niebla de guerra: textura RGBA al tamano del viewport, **no es particula** | `src/Render/FogOfWar.cpp:56-57` | textura | E I. Ni humo ni niebla ambiental existen como particulas de mapa |
| P17 | `SpriteBatch`: quads con textura; sin instancing: `submit` empuja **6 vertices** (32 B cada uno: `Vertex{pos,uv,color}`) a un `std::vector` sin tope; ring de 3 VBO | `include/Render/SpriteBatch.h:82-86`, `:92`; `src/Render/SpriteBatch.cpp:53-76` | -- | E I |
| P18 | RNG de azar del motor: `RPG::RandomEngine` (interfaz **solo** `roll_d6`, `roll_in_range(int,int)`, `seed()`) y `Xoroshiro128p` (header-only, miembro `const uint64_t m_seed`) | `include/RPG/RandomEngine.h:31-38`, `:46-85` (ctor `:48-52`, `roll_in_range` `:58-62`, `next_uint64` `:74-80`, `next_double` `:82-84`, `m_seed` const `:68`) | uint64 semilla; **no copiable/asignable** (miembro const) | E I T (50 ficheros que lo mencionan) |
| P19 | Doc del renderer ya preve el caso: "un efecto de particulas futuro probablemente no [querra sombra]" | `include/Render/IRenderable.h:16-29`, `getSortKey` `:33` | -- | solo comentario |

| P20 | **Particulas de las mascotas con estado**: `MascotesNivel::Particula{qui,vida,durada,x,y,dx,alcada,puja,frame}` (**segundos**, px de pantalla y celdas), `m_particules` + **una `MascotaEntity` por particula** (`m_puntets`) en la cola del renderer; `encen()` la reinicia al caducar; RNG propio xorshift32 `m_atzar` (semilla `llavor`, `VidaNivel.cpp:220`); avanza cada frame con `dt` desde `Application::update` | `include/Render/MascotesNivel.h:51-52`, `:61-62`, `:65`; `src/Render/VidaNivel.cpp:168-175` (struct), `:186-191` (`atzar`), `:203` (reset), `:247-252` (alta, `mg.quantes` por mascota), `:258-277` (`encen`), `:280-284` (`avanza`), `:305-311` (envejece y reinicia); `src/Engine/ApplicationBucle.cpp:176` | `dt` de update (float); **no consulta `MOTOR_MOVIMIENTO_REDUCIDO`** (grep en `VidaNivel.cpp` y `MascotesNivel.h`: 0 coincidencias); **sin tope** (lo fija el dato `quantes`) | E I. **Otro sistema; no se reutiliza ni se migra en U4-07** (D-T9) |

No existe en el repo (grep `particula|Particle|emitter|emisor|chispa|polvo|humo|niebla|SpriteBatch|instanc`
sobre `src include editor examples tests cmake CMakeLists.txt`): ningun emisor **generico** con estado (el unico con estado es P20, atado a `MascotaEntity`), ningun buffer de
particulas reutilizable, ningun instancing, ninguna niebla/humo ambiental de mapa, ningun fichero `Emisor*`.
`include/Render/PergaminoGL 2.h` (con espacio en el nombre, **versionado**, 88 lineas) es una **copia vieja**
de `PergaminoGL.h` que nadie incluye (`grep` sobre CMake/src/include/tests: 0 resultados): no tocarla.

### 1.2 Consumidores (lista completa del grep de la seccion 6)

| Consumidor | Usa | Cita |
|---|---|---|
| `Application::refreshHud` (cartas jugadas) | `EfectosDeCartas::olvida/vista/ve/lanza` | `src/Engine/ApplicationHud.cpp:175-212` (`refreshHud` empieza en `:82`; `olvida` `:182`; `lanza` `:209`) |
| Golpes (AV14-03) | `EfectosDeCartas::salpica("polvo"/"chispas"/"sangre")` | `src/Engine/ApplicationPantallas.cpp:433-442` (bucle de golpes `:408`; `avanza` `:444`) |
| Cola del mundo | `m_pergamino->delMundo()` -> `m_efectos` | `src/Engine/Application.cpp:508` (alta); `src/Engine/ApplicationCierre.cpp:119` (baja); `include/Render/PergaminoGL.h:70,76,92` |
| Propietario | `PergaminoGL` crea `EfectosDeCartas` y carga el mapa | `src/Render/PergaminoGL.cpp:171`, `:177-180` |
| Brillo del equipo | `AuraJugador` | `src/Render/AuraJugador.cpp:138-142` |
| Pintura compartida | `MezclaAditiva` tambien en el aura | `src/Render/AuraJugador.cpp:24`, `:80`, `:132` |
| Demo | `demo_brillo_equipo` (familias, `EfectosDeCartas`) | `examples/demo_brillo_equipo.cpp:120`, `:347-379` |
| Tests | `BrilloEquipoTests` (familias, brillo, cartas); target `test_brillo_equipo` | `tests/Game/BrilloEquipoTests.cpp:90-93,210-217,466-549,639,667-716`; `cmake/AparienciaTests.cmake:37-52` |
| Build | fuentes en `motor_level` (Game) y `motor_core` (Render) | `CMakeLists.txt:363-364`, `:375`, `:809` |
| Mascotas (sistema propio P20) | no usa el pipeline de `Destello`; comparte solo el `dt` de `Application::update` | `src/Engine/ApplicationBucle.cpp:176` |
| Datos/herramientas | `tools/particulas.py`, `tools/elementos.py`, `tools/particulas_cartas.py`, `tools/test_validar_sprites.py:399` | (python, fuera del motor C++) |

Todo lo no listado que el grep amplio devolvio (`Entity.cpp`, `IsometricRenderer.cpp`, `HudWidgets.cpp`,
`BattleState.cpp`...) coincide por `instanc*` ("instancia" de objeto, no instancing GL) o por otros terminos:
sin relacion con particulas (comprobado con `grep -n -i "particul|instanc"` sobre cada uno).

### 1.3 Fuentes de tiempo y azar (relevantes)

- `dt` de update (float s): `refreshHud` (`ApplicationHud.cpp:82`, spawns de cartas) y bucle de golpes en `ApplicationPantallas.cpp:408-444`; **render** corre aparte en la cola (`IRenderable::render`, `IRenderable.h:14`).
- Reloj de pared en render: `EfectosDeCartas::salpica` guarda `steady_clock::now()` (`PintaDestellos.cpp:88`) y `render` calcula edad y poda con ello (`:94-103`) -> **ignora pausa del mundo** (mismo hallazgo R7 de `U3-07_tweens.md`).
- Movimiento reducido: variable de entorno `MOTOR_MOVIMIENTO_REDUCIDO` leida con `static const bool` en 4 sitios (`ApplicationHud.cpp:76`, `ApplicationBucle.cpp:298`, `MesaCombateHud.cpp:421`, `DialogoHoja.cpp:219`); **las particulas actuales no la consultan** (grep: ninguna coincidencia en `FamiliasDeParticulas`/`BrilloEquipo`/`PintaDestellos`).
- Azar: las particulas actuales no usan RNG con estado (hash por `(fuente,indice,ciclo)`, `BrilloEquipo.cpp:17-30`; aura: `sin` de ruido, `AuraMoral.cpp:15-19`). Fuera de particulas: `static std::minstd_rand temblor(7)` (`ApplicationCinematica.cpp:495`, estado estatico global) y `std::random_device` (`:363`).

## 2. Que se reutiliza y que no

| Pieza | Decision |
|---|---|
| `TipoDeParticula`/`tipoDeParticula`/`familiasDeParticulas` (P1) | **Reutilizar tal cual** como "material" del emisor. `tipoDeParticula` no valida: el parser (E02) valida con `familiasDeParticulas()` (`FamiliasDeParticulas.h:47`). |
| `FormaDeParticula`, `pixelesDeForma`, `contornoDeForma` (P2, P3) | Reutilizar sin cambios. |
| `Game::Destello` (P7) como salida de E06 | **Reutilizar** (D-T2): E06 queda gl-free y la pintura es `pintaDestellos` sin codigo GL nuevo. Coordenadas del emisor en **pixeles de mundo**; se pintan con `esquina={0,0}`, `tam={64,64}` -> 1 px de frame = 1 px de mundo (`PintaDestellos.cpp:32-33`, `MontajePersonajes.h:63`). |
| `pintaDestellos`, `MezclaAditiva` (P10) | Reutilizar; E07 las llama. |
| `EfectosDeCartas` (P11) | E07 lo **adelgaza/sustituye por dentro** (misma API publica `lanza`/`salpica`/`olvida`/`aLaVista`), sin cambiar a sus 2 consumidores. |
| Formulas de movimiento del switch (`BrilloEquipo.cpp:300-353`) | Estan **inline en el bucle**, no invocables. D-T1: **portar** las 8 formulas a una funcion pura propia (no se toca `BrilloEquipo.cpp`: 360 lineas con 716 de tests que fijan byte a byte su determinismo). |
| `mezcla/azar01` (P5) | No reutilizable (anonimo) y no es un RNG con semilla de emisor; **no se usa**. |
| `AuraMoral`/`AuraJugador.renderMotas` (P12) | **No** (ficha). |
| `EfectosDeGolpe`, `GolpesEnPantalla` (P14, P15) | No son particulas; patron de `edad`+poda como referencia, nada que reutilizar. |
| `Interpolacion::curva`/`tweenEscalar`/`tweenVec2` (`include/Core/Math/Interpolacion.h:33,47,54`) y `EstadoTween` (`EstadoTween.h:25`) | **Opcional** en E06 para fundidos de alfa/tamano del `EmitterStyle` (curvas ya probadas); no obligatorio. No usar `avanzaTween` para la vida de una particula (un `EstadoTween` por particula es mas caro que `edad/vida`). |
| `RPG::Xoroshiro128p` (P18) | **Reutilizar** (ficha E04: "PRNG existent"), con mezcla de semilla (D-T3). |
| `Result<T>` (`Core/Errors/Result.h`), `JsonValue` (`include/Core/Json/JsonValue.h:40,58,65,72-73,105`) | Reutilizar en E02. |

## 3. Contratos propuestos E02..E08

Convenciones: namespace `Game::Emisor` (los hermanos `FamiliasDeParticulas`/`ParticulasDeCartas` estan en
`Game`, `FamiliasDeParticulas.h:13`); ficheros nuevos `include/Game/Emisor/` y `src/Game/Emisor/` (no
existen hoy), tests `tests/Game/` con `TestHarness.h`/`MOTOR_CHECK` (nunca `assert`); un `.cmake` propio
(patron `cmake/EfectosDeGolpe.cmake:1-14`) con `target_sources(motor_level PRIVATE ...)`, porque las
familias viven en `motor_level` (`CMakeLists.txt:363-364`) y no en `motor_math`. Unidades: **segundos
(double)** de tiempo; **pixeles de mundo (float)** de posicion; **px/s** de velocidad; **grados** solo en
datos JSON (internamente radianes); **particulas/s** de tasa. Sin `GridCoord` (entero logico, no se
interpola). Cabeceras <=15 miembros publicos y <=500 lineas por fichero.

### E02 · Leer parametros del emisor (90 min)

```cpp
// include/Game/Emisor/DefinicionEmisor.h   namespace Game::Emisor
struct DefinicionEmisor {
    std::string familia;          // material: una de familiasDeParticulas() (las 40)
    double tasa = 0.0;            // particulas/s, 0..kMaxTasa
    double duracion = 0.0;        // s de vida del EMISOR; 0 = continuo; 0..kMaxDuracion (una carta: kDuracionDeCarta)
    bool   cortar = false;        // true: al llegar a `duracion` se corta TODO (cartas/golpes); false: las vivas se agotan
    double vidaMin = 0.0, vidaMax = 0.0;      // s, (0..kMaxVida], vidaMin <= vidaMax
    double velocidadMin = 0.0, velocidadMax = 0.0;  // px/s, 0..kMaxVelocidad
    double anguloCentro = 0.0, abanico = 360.0;     // grados (0 = +x, y de pantalla hacia abajo), abanico (0..360]
    int    maximo = 0;            // aforo del emisor, 1..kMaxPorEmisor
    std::uint64_t semilla = 0;    // 0..2^53 (JSON numero double, sin perdida)
    // Paridad visual con cartas/equipo (ver E06/E07):
    float opacidad = 1.0f;        // (0..2]; alfa = min(1, vivo*opacidad), como EscalonDeBrillo::opacidad
    float contraste = 0.0f;       // [0,1]: parte que nace oscura y tapa (equipo Tier 5: 0.35)
    std::array<std::uint8_t,3> colorBase{255,240,210};  // se usa si tipo.colores esta vacio (agua, luz, chispas...)
};
Result<DefinicionEmisor> leerEmisor(const JsonValue& json);   // Result global: Core/Errors/Result.h
```
- Claves JSON en espanol sin tildes: `familia`, `tasa`, `duracion`, `vida` (`[min,max]`), `velocidad` (`[min,max]`), `angulo`, `abanico`, `maximo`, `semilla`, `cortar` (bool), `opacidad`, `contraste`, `color` (`"rrggbb"`, 6 hex como `leerColor`, `ParticulasDeCartas.cpp:14-21`; otro formato -> `Error`, **no** el color por defecto silencioso). Clave desconocida -> `Error` (D-T8).
- `familia` ausente o no listada en `familiasDeParticulas()` -> `Error` (recordar: `tipoDeParticula("no_existe")` devuelve `chispas` sin avisar, `FamiliasDeParticulas.cpp:78-82`). `tasa` NaN/inf/<0/`>kMaxTasa` -> `Error`; `vida<=0`, `vidaMax<vidaMin`, `vida>kMaxVida` -> `Error`; `maximo<1` o `>kMaxPorEmisor` -> `Error`; `semilla` negativa, no entera o `>2^53` -> `Error`; `opacidad` fuera de (0,2] o `contraste` fuera de [0,1] -> `Error`.
- Limites explicitos (**propuestos, D-O2**): `kMaxTasa=500` part/s, `kMaxVida=10` s, `kMaxDuracion=60` s, `kMaxVelocidad=2000` px/s, `kMaxPorEmisor=256`.
- Archivos E02: `include/Game/Emisor/DefinicionEmisor.h`, `src/Game/Emisor/DefinicionEmisor.cpp`, `tests/Game/EmisorTests.cpp`, `cmake/Emisor.cmake`, 1 linea `include(cmake/Emisor.cmake)` en `CMakeLists.txt` tras el cierre de `motor_level` (`CMakeLists.txt:388`).
- Test/target: `test_emisor` (labels `unit;gl-free`), enlaza `motor_level motor_json`.
- Renombres respecto a la ficha (que usa placeholders en ingles): `parseEmitter`->`leerEmisor`, `EmitterDefinition`->`DefinicionEmisor`, `EmissionState`->`EstadoEmision`, `EmissionStep`->`PasoEmision`, `emissionCount`->`emisiones`, `Particle`->`Particula`, `spawnParticle`->`crearParticula`, `ParticleBuffer`->`BufferParticulas`, `ParticleStep`->`PasoParticulas`, `stepParticles`->`avanzarEmisor` (E05; envejece/caduca/nace en un solo paso por emisor, porque el presupuesto global se consume en el nacimiento; `avanzarParticulas` queda como helper interno de envejecer), `particleSprites`->`proyectarParticulas`, `SpriteInstances`->`Game::Destello` (D-T2), `EmitterStyle`->`EstiloEmisor`.

### E03 · Emisiones con residuo (60 min)

```cpp
// include/Game/Emisor/EmisionConResiduo.h
struct EstadoEmision { double residuo = 0.0; };                 // particulas fraccionarias pendientes, [0,1)
struct PasoEmision   { EstadoEmision estado; int cuantas = 0; bool recortado = false; };
constexpr double kMaxPasoEmision = 0.25;    // s: tope de catch-up por llamada (propuesto)
Result<PasoEmision> emisiones(const EstadoEmision& e, double dt, double tasa);   // dt: s; tasa: part/s
```
- `dt` NaN/inf/<0 o `tasa` NaN/inf/<0/`>kMaxTasa` o `residuo` fuera de [0,1) -> `Error`. `tasa==0` o `dt==0` -> `cuantas=0`, `estado` igual.
- `dt > kMaxPasoEmision` -> se usa `kMaxPasoEmision` y `recortado=true` (el tiempo sobrante **se descarta**, no se acumula: tras un parón de 5 s no sale una rafaga).
- `acumulado = residuo + tasa*dt`; `cuantas = floor(acumulado + kTolEmision)`, `kTolEmision = 1e-9` (mismo patron que `kToleranciaFinTween`, `EstadoTween.h:24`: 60 pasos de `1/60` a `tasa=10` acumulan `9.999999999999998` y deben dar 10); `residuo = max(0, acumulado - cuantas)`.
- Particiones equivalentes: n trozos == un trozo (misma `cuantas` total, residuo con tolerancia `1e-9`), con la salvedad del recorte.
- Archivos E03: `include/Game/Emisor/EmisionConResiduo.h`, `src/Game/Emisor/EmisionConResiduo.cpp`, `tests/Game/EmisorTests.cpp`.

### E04 · Crear particula con semilla explicita (60 min)

```cpp
// include/Game/Emisor/Particula.h  (E04 crea; E05 la consume)
struct Particula {
    float x = 0, y = 0;           // px de mundo, relativo al origen del emisor
    float vx = 0, vy = 0;         // px/s
    float edad = 0, vida = 0;     // s; viva mientras edad < vida
    float ang0 = 0, radio0 = 0;   // rad / px: parametros fijos de Orbita/Converge/Expande (sorteados al nacer)
    std::uint16_t variante = 0;   // indice en TipoDeParticula::colores
    std::uint8_t  otraForma = 0;  // 0/1: usa TipoDeParticula::otraForma
    std::uint8_t  oscura = 0;     // 0/1: nace del tono oscuro que tapa (contraste)
};
std::uint64_t mezclarSemilla(std::uint64_t semilla);                       // splitmix64
double uniforme01(RPG::RandomEngine& r);                                   // [0,1) en pasos de 1e-6 via roll_in_range(0, 999999)
Particula crearParticula(const DefinicionEmisor& d, const TipoDeParticula& tipo, RPG::RandomEngine& r);
```
- **Un `RPG::Xoroshiro128p` por emisor, propiedad del emisor y guardado en `std::unique_ptr`** (R17): `Xoroshiro128p` no se puede asignar (`RandomEngine.h:68`) y un `Emisor` por valor no soportaria la poda `erase`. Construido con `mezclarSemilla(def.semilla)` y descartando 4 sorteos (nunca `static`, nunca `rand()`, nunca `random_device`). Nunca se comparte entre emisores.
- Orden de sorteos **fijo y documentado**: 1 `vida`, 2 `velocidad`, 3 `angulo`, 4 `variante`, 5 `otraForma`, 6 `ang0`, 7 `radio0`, 8 `oscura` (`uniforme01 < contraste`, como `BrilloEquipo.cpp:282-286`). Siempre se consumen los 8.
- Rangos: `vida in [vidaMin,vidaMax]`, `|v| in [velocidadMin,velocidadMax]`, direccion dentro de `anguloCentro +- abanico/2`.
- La velocidad inicial se compone con `tipo.vx/vy` segun el movimiento (D-T1); posicion inicial `(0,0)`.
- Archivos E04: `include/Game/Emisor/Particula.h`, `src/Game/Emisor/CrearParticula.cpp`, `tests/Game/EmisorTests.cpp`.

### E05 · Avanzar estado, caducar y presupuesto global (90 min)

```cpp
// include/Game/Emisor/BufferParticulas.h
struct PresupuestoParticulas {                  // UNICO por adaptador; lo comparten todos los emisores
    std::size_t maximo = kMaxParticulasGlobal;  // particulas vivas entre todos los emisores
    std::size_t vivas = 0;
    bool reserva();                             // vivas<maximo ? ++vivas,true : false
    void libera(std::size_t n);
};
class BufferParticulas {                        // aforo fijo, memoria reservada en el constructor
public:
    explicit BufferParticulas(std::size_t aforo);                 // reserve(aforo): sin alloc despues
    std::size_t vivas() const; std::size_t aforo() const;
    bool llena() const;
    void anade(const Particula& p);               // precondicion: !llena() y la reserva ya hecha
    const std::vector<Particula>& datos() const;
};
struct PasoParticulas { int nacidas = 0; int caducadas = 0; int rechazadas = 0; bool recortado = false; };
// dt: s. Hace: envejecer+caducar (libera presupuesto), luego nacer `cuantas` (E03) en orden.
Result<PasoParticulas> avanzarEmisor(Emisor& e, double dt, double escalaTasa, PresupuestoParticulas& p);
```
- `class Emisor` (en `include/Game/Emisor/Emisor.h`, E05): `DefinicionEmisor`, `TipoDeParticula`, `std::unique_ptr<RPG::Xoroshiro128p>`, `EstadoEmision`, `BufferParticulas`, `Vector2 origen`, `double tiempo`; **movible y asignable por movimiento** (poda con `remove_if`+`erase`), no copiable.
- **Presupuesto global compartido, consumido en cada nacimiento y ANTES de sortear**: `if (buffer.llena() || !presupuesto.reserva()) { ++rechazadas; continue; }` y solo entonces `crearParticula`. El aforo "calculado sobre vivas" no basta (64 emisores recien creados y vacios sumarian 16 384 frente a 2048): el limite se garantiza por la reserva, no por el aforo. Orden de admision **fijo**: emisores en orden de creacion dentro del vector; particulas de cada uno en orden de nacimiento. Las caducadas liberan (`libera`) antes de que nazcan las nuevas del mismo fotograma.
- `dt` NaN/inf/<0 -> `Error` y el emisor **no cambia**; `dt==0` -> sin cambios. Se acota `dt <= kMaxPasoEmision` (recorte compartido con E03; `recortado=true`).
- Integracion segun `tipo.movimiento` (portada de `BrilloEquipo.cpp:300-353`, D-T1): `Sube/Cae/Deriva/Salpica` (`gravedad = 2*|vy|/vida`, vuelve a su altura al final, `:301-313`) integran `x,y`; `Orbita/Converge/Expande` usan `ang0/radio0` y la fraccion `edad/vida` respecto al origen; `Chispazo` (rayo): oculto con `edad>0.12` (`:269-271`).
- `edad += dt` (float); caduca si `edad >= vida`. **Compactacion estable** (D-T7), O(n). Una caducada no se proyecta.
- Duracion y corte: `Emisor::tiempo` acumula el `dt` (ya recortado). Con `duracion>0`, al cruzar `tiempo >= duracion - 1e-9` deja de emitir; si `cortar`, ademas **vacia el buffer en ese mismo paso** (liberando presupuesto) y `acabado()` es verdadero; si no `cortar`, `acabado()` = buffer vacio tras la emision. Esto reproduce que hoy una carta desaparece de golpe a `kDuracionDeCarta` (`ParticulasDeCartas.cpp:78`), aunque a una particula le quedara vida.
- Sin GL ni reservas: `datos().capacity()` constante tras el constructor.
- Archivos E05: `include/Game/Emisor/BufferParticulas.h`, `include/Game/Emisor/Emisor.h`, `src/Game/Emisor/BufferParticulas.cpp`, `src/Game/Emisor/AvanzarEmisor.cpp`, `tests/Game/EmisorTests.cpp`.

### E06 · Proyectar a instancias visuales (60 min)

```cpp
// include/Game/Emisor/ProyeccionParticulas.h   (gl-free)
struct GrupoDeDibujo {                 // el espacio al que pertenecen las coordenadas de un emisor
    Vector2 esquina{0,0};              // px de mundo de la esquina del dibujo (p. ej. Entity::dibujoDePersonaje)
    Vector2 tam{64,64};                // px de mundo del dibujo; 64x64 = 1 px de frame por px de mundo
};
// El mismo calculo que PintaDestellos.cpp:32-33,42-44,55-58: px = tam/64 (kLado); x frame redondeado antes de escalar.
Vector2 mundoDePixel(const GrupoDeDibujo& g, float xFrame, float yFrame, int dx, int dy);   // esquina de un pixel de forma
struct EstiloEmisor {
    float alfaMax = 1.0f;          // 0..1
    float entrada = 0.08f;         // s: envolvente del EMISOR, sube 0 -> 1 durante los primeros `entrada` s de Emisor::tiempo
    float salida  = 0.4f;          // s: envolvente del EMISOR, baja 1 -> 0 durante los ultimos `salida` s antes de `duracion`
    float escala  = 1.0f;          // 1 = un pixel de frame por pixel de forma (canon: D-O4)
    bool  reducido = false;        // movimiento reducido: ver D-O3
};
struct ResumenProyeccion { std::size_t destellos = 0; std::size_t quads = 0; std::size_t descartados = 0; };
// quads = SUMA de pixelesDeForma(f).size() + contornoDeForma(f).size() de cada Destello con alfa>0
std::size_t quadsDeDestellos(const std::vector<Game::Destello>& d);
ResumenProyeccion proyectarParticulas(const Emisor& e, const EstiloEmisor& estilo, std::size_t quadsMax,
                                      std::vector<Game::Destello>& out);
```
- **Espacio de coordenadas**: las posiciones de un emisor y su `origen` son **px de frame del grupo** (`GrupoDeDibujo`), como los `Destello` de hoy (`Destello.x/y` en un frame de 64x64, `BrilloEquipo.h:91-97`). La transformacion explicita a mundo es `mundo = esquina + (round(x) + dx) * tam/64`, con tamano de pixel `tam/64` (`PintaDestellos.cpp:32-33`, `:42-44`, `:55-58`). Un emisor sin sprite usa `GrupoDeDibujo{origenMundo, {64,64}}`. **`Destello` no lleva escala ni grupo** (`BrilloEquipo.h:91-97`), por eso el grupo vive en el `Emisor` y se pinta **una llamada a `pintaDestellos` por grupo distinto** (emisores consecutivos con el mismo `esquina`/`tam` se agrupan), exactamente como hoy una por `Activa` (`PintaDestellos.cpp:101-105`): sin regresion en flushes. Ejemplo: `Destello.x=110` con `tam.x=128` es un frame-x de 110 -> 220 px de mundo mas `esquina`; **no** se mezcla un `x` de un grupo con el `tam` de otro.
- E07 refactoriza `pintaDestellos` para llamar a `mundoDePixel` (misma formula, mismo codigo en test y en pintura).
- **Dos capas de alfa, separadas** (antes mezcladas): (a) **por vida de particula**: `min(1, vivo * d.opacidad)`, `vivo = 1 - edad/vida` (`BrilloEquipo.cpp:274-277`; cartas: `kOpacidad[8] = 1.45`, `:38`, nivel 8 de `ParticulasDeCartas.cpp:86`); (b) **envolvente por tiempo del emisor**: `min(1, tiempo/entrada) * min(1, (duracion - tiempo)/salida)` (clamp 0..1; `salida` solo si `duracion>0`), calculada con la edad TOTAL del emisor, como `ParticulasDeCartas.cpp:98-102` usa la edad de la carta. Alfa final = `alfaMax * (a) * (b)`. Ejemplo: carta en `tiempo=1.39 s`, particula con `edad=0.10 s`, `vida=1 s`: (b)=`(1.4-1.39)/0.4=0.025`, no 1.
- **Corte**: si `cortar` y `tiempo >= duracion`, `proyectarParticulas` devuelve 0 destellos (corte duro a 1.4 s, `ParticulasDeCartas.cpp:78`; `kDuracionDeCarta` `ParticulasDeCartas.h:54`); no se proyectan particulas nacidas al final que "sobreviven" tras la duracion.
- Color: `tipo.colores[variante]` o, si no hay, `d.colorBase` (`BrilloEquipo.cpp:278-280`); `oscura`: `*42/100` y `suma=false` (`:282-286`). No hay blanco por defecto.
- **Contornos**: `pintaDestellos` hace un `submit` por pixel de cuerpo **y** de contorno (`PintaDestellos.cpp:37-44` cuerpo, `:48-58` contorno, solo con `alfa>0`). Calavera = 16 + 18 = 34 quads. `quadsMax` corta en orden de nacimiento y cuenta `descartados`.
- `out` lo posee quien llama y se acumula entre emisores **del mismo grupo** (`clear()` por grupo; patron `EfectosDeCartas::m_destellos`, `EfectosDeCartas.h:60`). `Expande` emite 10 `Destello` `Punto` por particula (`BrilloEquipo.cpp:337-347`).
- Curvas opcionales con `Interpolacion::curva` (`Interpolacion.h:33`). **Ni GL ni textura**.
- Archivos E06: `include/Game/Emisor/ProyeccionParticulas.h`, `src/Game/Emisor/ProyeccionParticulas.cpp`, `tests/Game/EmisorTests.cpp` (o `EmisorProyeccionTests.cpp`).

### E07 · Conectar un emisor a una escena real (90 min)

Documento de adaptador + cambio minimo. Propuesta (D-O1, opcion A): `EfectosDeCartas::salpica(familia,esquina,tam)` crea un `Emisor` (rafaga de `kDuracionDeCarta`) en lugar de `Activa` con reloj de pared; API publica de `lanza`/`salpica`/`olvida`/`aLaVista` sin cambios (`ApplicationHud.cpp:209`, `ApplicationPantallas.cpp:440-441` no cambian de firma). Se **anade** `void avanza(float dt)`.
- **Reloj y orden concretos**: `avanza(dt)` se llama **una vez por tick de simulacion fijo**, no por fotograma: `Application::update` es el `fixedUpdate` de la escena (`src/Engine/ApplicationBucle.cpp:85`, lambda `update(dt)` en `:101`), invocado por el bucle a pasos de `fixedStepSeconds = 1/60 s` (`include/Engine/ApplicationLoop.h:14`), **hasta 6 veces por render** (`ApplicationLoop.h:22`; bucle `src/Engine/ApplicationLoop.cpp:66-75`; `render` una vez despues, `:94`). Se llama con `mundo` (`ApplicationBucle.cpp:141`: `0` durante los 70 ms de pausa del critico), justo despues de `m_player->update(mundo)` (`:142`) y **antes** de `refreshHud()` (`:325`) y de `ApplicationCinematica::combate` (`:173`). **Implicacion**: un emisor creado en el tick k puede envejecer en los ticks k+1..k+5 antes de su primer render (hasta 5/60 = 0,083 s, justo la `entrada` de 0,08 s): puede aparecer ya con la envolvente de entrada en 1; no es un error, y la prueba de E07 no debe asumir `edad=0` en el primer render. No va dentro de `refreshHud`, que tambien se llama **fuera de la ruta de update**: `ApplicationBucle.cpp:122`, `ApplicationCinematica.cpp:514` (que ademas llama a `render(false)` con el juego en pausa: los emisores quedan congelados, coherente), `Application.cpp:726`, `:923`, `:976`. Productores: `lanza` (en `refreshHud`, `ApplicationHud.cpp:209`) y `salpica` (`ApplicationPantallas.cpp:440`, alcanzado por `combate` `ApplicationBucle.cpp:173` -> `golpesALaVista` `ApplicationCinematica.cpp:442` -> `salpica`). **Solo en la ruta de update** ambos ocurren despues de `avanza` en el mismo tick; los `refreshHud` de fuera de update pueden crear emisores sin que `avanza` haya corrido.
- **Parametros de paridad**: `opacidad=1.45`, `contraste=0` (`kOpacidad[8]`; el contraste solo desde nivel 9, `BrilloEquipo.cpp:41`), `colorBase = ParticulasDeCartas::colorDe(familia)` (`ParticulasDeCartas.cpp:54-57`; el JSON da `agua 5ab4ff`, `chispas ffd28c`, `luz fff0b4`, `rayos fff27a`: `assets/textures/particulas_cartas.json:970,976,992,1004`), `semilla = ++m_lanzadas` (como `PintaDestellos.cpp:85`), `duracion = kDuracionDeCarta`.
- Propiedad: `PergaminoGL::m_efectos` (`PergaminoGL.h:92`) posee el estado y el `PresupuestoParticulas`; nace en `PergaminoGL()` (`PergaminoGL.cpp:171`), el mapa se carga en `carga` (`:177-180`); `olvida()` ya se llama sin combate (`ApplicationHud.cpp:182`); baja de la cola en el cierre (`ApplicationCierre.cpp:119`). **Cambio de mapa**: sin gancho conocido (R9).
- Render: solo proyecta (un `out` por `GrupoDeDibujo`, E06) y pinta con `pintaDestellos` (una llamada por grupo; `GrupoDeDibujo` de una carta/golpe = `{esquina, tam}` que ya recibe `lanza/salpica`, tamano real variable: `Entity.cpp:89-92` `m_visualScale`, `ApplicationHud.cpp:193-209`). Se elimina `steady_clock` de `EfectosDeCartas`. Parametros de cartas: `duracion = kDuracionDeCarta`, `cortar = true`, `entrada = 0.08`, `salida = 0.4`.
- Movimiento reducido (`MOTOR_MOVIMIENTO_REDUCIDO`, patron `ApplicationHud.cpp:76`): `escalaTasa` segun D-O3.
- **Ambito E07** (con topes): `include/Render/EfectosDeCartas.h` (66), `src/Render/PintaDestellos.cpp` (109), `include/Render/PergaminoGL.h` (97; solo si hace falta exponer `avanza`), `src/Render/PergaminoGL.cpp` (397; techo +30), `src/Engine/ApplicationBucle.cpp` (473; techo +20, una linea de `avanza`), `src/Engine/ApplicationHud.cpp` (465; techo +34 para quedar en 499), `src/Engine/ApplicationPantallas.cpp` (**497; neto <= 0**), `examples/demo_brillo_equipo.cpp` (406; techo +30: crear/`avanza` por fotograma, usa `efectos` en `examples/demo_brillo_equipo.cpp:347`, `:379` y el render `:383-386`), test nuevo (no `BrilloEquipoTests.cpp`, 716). **No** tocar `Application.cpp` (1282), `CMakeLists.txt` salvo la linea de E02, `BrilloEquipo.cpp`, `FamiliasDeParticulas.cpp`, `PergaminoGL 2.h`, ni el sistema de mascotas (D-T9).

### E08 · Medir limite y repetibilidad (60 min)

Documento + prueba gl-free: dos runs con la misma entrada y buffer lleno -> hash de estado identico; maximo respetado (`vivas <= aforo` y `presupuesto.vivas <= maximo`, `rechazadas > 0`); coste medido en **quads por fotograma** con `quadsDeDestellos` (cuerpo + contorno; con `calavera` el caso peor). `SpriteBatch::m_vertices` es privado (`SpriteBatch.h:94`), asi que el numero real de `submit` no se lee desde un test gl-free: la prueba compara `quadsDeDestellos` contra un recuento independiente que replica las condiciones de `pintaDestellos` (`alfa>0`, `pixelesDeForma`+`contornoDeForma`), y la medida real en un ejecutable GL queda como observacion manual de E08. **No se promete cifra de FPS** (ficha E08). Fichero de prueba propio: `tests/Game/EmisorCargaTests.cpp`, target `test_emisor_carga` en `cmake/Emisor.cmake`. Observacion visual = pendiente de Oriol (D-O5).

## 4. Orden por fotograma y propiedad del estado

Propietario: el adaptador (`EfectosDeCartas` en E07) posee `std::vector<Emisor>` (orden de creacion) y **un** `PresupuestoParticulas`; nada global ni `static`. Un hilo.

Update (con `mundo`, pausable, **una vez por tick fijo de 1/60 s** desde `Application::update`; hasta 6 ticks por render; E07):
1. `avanza(dt)`: para cada emisor en orden de creacion, `avanzarEmisor` = (a) envejecer y caducar existentes (libera presupuesto); (b) `emisiones(...)` -> `cuantas`; (c) cada nacimiento: `llena()||!reserva()` -> `rechazadas`, si no `crearParticula`+`anade`. Los recien nacidos quedan con `edad=0`.
2. Poda de emisores `acabado()` (`erase` estable).
3. Despues, en el mismo `update`: `lanza/salpica` crean emisores (`ApplicationHud.cpp:175-212`, `ApplicationPantallas.cpp:433-442`), con `kMaxEmisores` como tope (rechazo = `false`).

Render (solo lectura, sin avanzar tiempo): para cada grupo de emisores consecutivos con el mismo `GrupoDeDibujo`: `out.clear()`, `proyectarParticulas(..., quadsRestantes, out)` por emisor, `pintaDestellos(batch,out,grupo.esquina,grupo.tam,blanco)` con `MezclaAditiva` dentro (`PintaDestellos.cpp:62`). Clave de orden: `getSortKey() = INT_MAX-1` (`EfectosDeCartas.h:48`).

## 5. Presupuesto de particulas (puerta PRESSUPOST_PARTICULES)

Medido/estimado hoy (**inferido** salvo cita): una carta jugada produce `int(14.4 * vida * densidad) + 1` particulas (`parte=3` x `escalonDeAbalorio(8).particulas = 0.8+0.5*8 = 4.8`: `ParticulasDeCartas.cpp:86-90`, `BrilloEquipo.cpp:78`, `:254`): <= ~36 con `oro` o `esporas`/`animas`. Un arma +10 emite 18 part/s (`BrilloEquipo.cpp:58`); varias fuentes a la vez, sin tope global. **Las mascotas suman aparte**: `mg.quantes` particulas por mascota, una entidad (un quad de 4 frames) por particula (`VidaNivel.cpp:247-252`, **inferido** 1 quad), sin tope; no entran en este presupuesto (D-T9) pero se registran como consumo concurrente en E08. Coste por particula del emisor: cuerpo + contorno (Calavera 16 + 18 = 34 quads; `FamiliasDeParticulas.cpp:93,192-222`) x 6 vertices x 32 B (`SpriteBatch.cpp:69-75`, `SpriteBatch.h:82-86`).

Presupuesto propuesto v1 (constantes unicas en `DefinicionEmisor.h`; **valores provisionales, D-O2**):

| Limite | Valor propuesto | Que pasa al superarlo |
|---|---|---|
| `kMaxPorEmisor` (aforo de un emisor) | 256 | el nacimiento se **rechaza antes de sortear**; `PasoParticulas::rechazadas++` |
| `kMaxEmisores` (activos a la vez) | 64 | `lanza/salpica` devuelve `false` (como hoy cuando la carta no echa particulas: `EfectosDeCartas.h:40`); no se evicta ninguno |
| `kMaxParticulasGlobal` (vivas entre todos, `PresupuestoParticulas`) | 2048 | **cada nacimiento reserva antes de sortear**; sin hueco, se rechaza. Garantia por reserva, no por aforo (64 x 256 = 16 384 > 2048): prueba con 64 emisores vacios simultaneos |
| `kMaxQuadsPorFotograma` (**cuerpo + contorno** de todos los `Destello` con `alfa>0`) | 8192 (~1.5 MB de vertices/fotograma; =240 calaveras de 34 quads; 512 calaveras serian 17 408) | `proyectarParticulas` corta la salida por orden de nacimiento y cuenta `descartados`; el estado de simulacion no cambia |
| `kMaxPasoEmision` (catch-up por llamada) | 0.25 s | `recortado=true`, tiempo sobrante descartado |

Regla: **superar el presupuesto degrada visibilidad, nunca estado ni determinismo ni memoria**. Crecimiento en memoria tras construir: 0 (`reserve` en constructor). Politica de rechazo (drop-new) tiene efecto visible (que destellos no salen en una rafaga muy cargada): se somete a Oriol dentro de D-O2. E08 mide; FPS/memoria dependen de maquina acordada.

## 6. Casos de prueba deterministas (semilla)

Todos gl-free en `test_emisor`/`test_emisor_carga`; semillas fijas; **los valores dorados se registran al implementar** (E04/E05 ejecutan y fijan el hash), no se inventan aqui.

E02 `leerEmisor`: JSON valido -> campos exactos; negativos (cada uno `Error`): `familia` ausente; `familia:"no_existe"`; `tasa:-1`; `tasa:1e9`; `tasa:"x"`; `vida:[0,1]`; `vida:[2,1]`; `maximo:0`; `maximo:100000`; `semilla:-1`; `semilla:1.5`; `semilla:1e300`; `opacidad:0`; `contraste:2`; `color:"zzzzzz"`; clave desconocida. Mutacion: si el parser acepta `no_existe` (delegando en `tipoDeParticula`), falla.
E03 `emisiones`: `(tasa=10,dt=0.05)` -> 0, residuo 0.5; segundo paso -> 1, residuo 0; `tasa=0` -> 0; `dt=0` -> 0; `dt=NaN` -> `Error`; `dt=5` -> `recortado`, `cuantas <= tasa*0.25 + 1`; **60 x (1/60) a tasa 10 == 10**; 1000 x 0.001 a tasa 7 == 7; particion 1 vs n equivalente (sin recorte).
E04 `crearParticula`: misma semilla -> 100 particulas bit a bit iguales; rangos respetados en 10 000 muestras; **semillas 1 y 2 producen secuencias distintas** (>= 90% de particulas difieren en `vida`) tras `mezclarSemilla` y descarte (guarda R3); `static_assert(std::is_move_assignable_v<Emisor>)` y poda de un vector de emisores conserva el RNG de los supervivientes (R17); dos emisores con semillas distintas no comparten estado.
E05 (**casos corregidos; el recorte `kMaxPasoEmision=0.25` se aplica**): vida 1.0 con pasos `0.5+0.5` -> cada paso se recorta a 0.25, `edad=0.5`, **sobrevive** y `recortado=true`; vida 1.0 con `4 x 0.25` -> `edad=1.0`, **caduca** (exacto en float); vida 0.2 con un paso `0.25` -> **caduca** (un paso que cruza la vida); `100 x 0.0099` (edad ~0.99, en float 0.9899988...) -> **sobrevive**, y `102 x 0.0099` -> caduca; recorte y precision se prueban en casos separados. Aforo 8 con `tasa=500` durante 1 s: `vivas <= 8`, `rechazadas > 0`, `capacity()` constante. **Presupuesto global**: 64 emisores vacios, `tasa=500`, `maximo=256`, `PresupuestoParticulas.maximo=2048`: tras un paso, `sum(vivas) <= 2048` y `== presupuesto.vivas`; el reparto es el de la orden de creacion (los primeros emisores consumen hasta agotar), identico en dos runs; tras caducar todo, `presupuesto.vivas==0`. `dt<0`/NaN -> `Error` y estado identico. Orden estable tras caducidad.
E06 `proyectarParticulas`: `destellos == vivas` salvo `Expande` (`10*vivas`); `alfa in [0,1]`; `suma` coincide con `tipo.suma`; **paridad**: `agua` con `colorBase=5ab4ff` -> todos los `Destello.color == {0x5a,0xb4,0xff}`; `chispas` con `colorBase=ffd28c` -> `{0xff,0xd2,0x8c}`; con `opacidad=1.45` y `vivo=0.5` el alfa previo a fundidos es `min(1,0.725)=0.725`, y con `vivo=0.8` es `1.0` (saturado); `contraste=1` -> todas `suma=false` y color `*42/100`; **Calavera**: 1 particula -> `quadsDeDestellos == 34`, 512 -> 17 408 y con `quadsMax=8192` `descartados == 512-240`; `escala` cambia solo el tamano. **Envolvente del emisor** (cartas: `duracion=1.4`, `cortar`, `entrada=0.08`, `salida=0.4`): emisor en `tiempo=1.39` con una particula `edad=0.10`, `vida=1`: alfa de envolvente `0.025` (tolerancia `1e-6`), no 1; en `tiempo=1.40` -> 0 destellos y buffer vacio, presupuesto liberado; particulas nacidas en los ultimos 0.1 s no sobreviven a `1.4`; en `tiempo=0.04` la entrada vale `0.5`. **Transformacion mundo/grupo**: dos emisores con la misma particula en frame `(10,20)`: grupo A `esquina=(100,50), tam=(64,64)` y grupo B `esquina=(300,80), tam=(128,128)`; con forma `Dragon` (cuerpo y **contorno**) `mundoDePixel(A,10,20,1,0) == (111,70)` y `mundoDePixel(B,10,20,1,0) == (322,120)`, tamano de pixel 1 y 2; ningun pixel de B se calcula con el `tam` de A.
E08 repetibilidad: dos runs de 600 pasos de `1/60` con buffer lleno -> hash FNV-1a de los bits de todas las particulas identico; con otra semilla, distinto.
Caso negativo obligatorio de la ficha: cada E trae al menos uno (arriba).

## 7. Ambito de ficheros congelado (propuesto)

```
NUEVOS (0 lineas hoy)
include/Game/Emisor/DefinicionEmisor.h       (E02)
src/Game/Emisor/DefinicionEmisor.cpp         (E02)
include/Game/Emisor/EmisionConResiduo.h      (E03)
src/Game/Emisor/EmisionConResiduo.cpp        (E03)
include/Game/Emisor/Particula.h              (E04)
src/Game/Emisor/CrearParticula.cpp           (E04)
include/Game/Emisor/BufferParticulas.h       (E05)
include/Game/Emisor/Emisor.h                 (E05)
src/Game/Emisor/BufferParticulas.cpp         (E05)
src/Game/Emisor/AvanzarEmisor.cpp            (E05)
include/Game/Emisor/ProyeccionParticulas.h   (E06)
src/Game/Emisor/ProyeccionParticulas.cpp     (E06)
tests/Game/EmisorTests.cpp                   (E02-E06)  target: test_emisor
tests/Game/EmisorCargaTests.cpp              (E08)      target: test_emisor_carga
cmake/Emisor.cmake                           (E02, E08) + 1 linea include(...) en CMakeLists.txt (cierra motor_level en :388)
EXISTENTES QUE TOCA E07 (tamano -> regla)
src/Render/PintaDestellos.cpp                109   -> puede crecer
include/Render/EfectosDeCartas.h              66   -> puede crecer
include/Render/PergaminoGL.h                  97   -> solo si hace falta
src/Render/PergaminoGL.cpp                   397   -> techo +30
src/Engine/ApplicationBucle.cpp              473   -> techo +20 (una linea de avanza)
src/Engine/ApplicationHud.cpp                465   -> techo +34 (<500)
src/Engine/ApplicationPantallas.cpp          497   -> neto <= 0 (solo adelgazar)
examples/demo_brillo_equipo.cpp              406   -> techo +30
NO TOCAR: src/Engine/Application.cpp (1282), CMakeLists.txt (1639; solo la linea de E02),
          tests/Game/BrilloEquipoTests.cpp (716), src/Game/Personajes/BrilloEquipo.cpp (360),
          src/Game/Personajes/FamiliasDeParticulas.cpp (224), include/Render/PergaminoGL 2.h (copia muerta),
          include/Render/MascotesNivel.h y src/Render/VidaNivel.cpp (mascotas, D-T9)
```
Ningun fichero nuevo previsto >500 lineas ni cabecera >15 miembros. Comando de prueba (E02+): `cmake --build <build> --target test_emisor && ctest --test-dir <build> -R "test_emisor|test_brillo_equipo"`. Esta E01 **no lanza builds**.

## 8. Decisiones

### Tecnicas (Brain, D-T) — propuestas, bloqueo indicado

| ID | Pregunta | Recomendacion | Bloquea |
|---|---|---|---|
| D-T1 | Movimiento de particula: portar las 8 formulas (`BrilloEquipo.cpp:300-353`) o extraerlas de `destellosDeBrillo` | **Portar** (no se toca `BrilloEquipo.cpp`; sus tests fijan el determinismo actual). Coste: ~50 lineas duplicadas | E05 |
| D-T2 | Salida de E06: `Game::Destello` o nuevo `SpriteInstances` | **`Destello`** (cero GL nuevo; reutiliza `pintaDestellos`) | E06 |
| D-T3 | RNG: `Xoroshiro128p` + `uniforme01` sobre `roll_in_range` + `mezclarSemilla`; alternativa: tocar `RandomEngine.h` | **Sin tocar `RandomEngine.h`** (50 ficheros lo mencionan, C-21 "una sola interfaz", `RandomEngine.h:26-30`) | E04 |
| D-T4 | Al superar aforo/presupuesto: no admitir (drop-new) vs reciclar la mas vieja | **drop-new**, rechazo antes de sortear. **Borderline producto** (decide que destellos se pierden en rafagas cargadas): se somete a Oriol dentro de D-O2 | E04, E05 |
| D-T5 | Donde vive: `motor_level` via `cmake/Emisor.cmake` vs `motor_math` | **`motor_level`** (depende de `FamiliasDeParticulas`, `CMakeLists.txt:364`) | E02 |
| D-T7 | Compactacion estable vs swap-remove | **Estable** (orden = nacimiento) | E05 |
| D-T8 | JSON: clave desconocida -> `Error` | **Estricto** | E02 |
| D-T9 | Particulas de mascotas (P20): migrar a `Emisor` o dejar | **Dejar**: estan atadas a `MascotaEntity`, cel·las y 4 frames de sprite, no a `Destello`; migrarlas es otro paquete. U4-07 las registra (R6, seccion 5) y no las toca | E07 (alcance) |
| D-T10 | Propiedad del RNG en `Emisor` (no asignable) | **`std::unique_ptr<RPG::Xoroshiro128p>`** dentro de `Emisor` (R17); alternativa rechazada: reconstruir el RNG desde la semilla (pierde el estado) | E04, E05 |

### De producto/visuales (propietario, D-O)

| ID | Pregunta | Opciones | Recomendacion | Bloquea |
|---|---|---|---|---|
| D-O1 | Primera escena real del emisor (E07) | **A** salpicaduras de golpe y cartas ya existentes, migradas al emisor; **B** polvo de pasos del jugador (familia `polvo`, `CANON_PARTICULAS.md:110`), efecto nuevo; **C** humo/niebla ambiental de mapa (requiere datos de mapa inexistentes) | **A**: ya tiene familia, consumidores y tests; B despues (necesita gancho de pasos y limpieza por cambio de mapa) | E07 |
| D-O2 | Cifras de presupuesto (seccion 5), **politica de rechazo (drop-new)** y maquina de referencia | aceptar / dar otras / reducidas para el portatil | Aceptar como provisionales; E08 las valida con medidas, sin prometer FPS | no bloquea E02-E06 (constantes provisionales); cierra en E08 |
| D-O3 | Movimiento reducido (`MOTOR_MOVIMIENTO_REDUCIDO`) | apagar no esenciales / tasa al 25% / sin cambio | **Tasa x0.25 y sin aro `Expande`**; cartas y golpes siguen visibles | E07 |
| D-O4 | Tamano de particula (pregunta abierta, `CANON_PARTICULAS.md:460-461`) | 1 px de sprite / temas mas grandes | **1 px** en v1; `EstiloEmisor::escala` como hook | nada |
| D-O5 | Aprobacion visual/jugable de E07/E08 | observacion real de Oriol | no se declara por test | cierre de E07/E08 |
| D-O6 | Reloj de las cartas y golpes: hoy de pared (siguen durante los 70 ms de pausa del critico y en cualquier pausa); con el emisor pasan a `dt` de update (se detienen con `mundo=0`) | mantener reloj de pared / `dt` pausable | **`dt` pausable** (coherente con el resto del mundo; el cambio visible es minimo: 70 ms) | E07 |

Paridad con el canon: el emisor **no inventa** colores/formas; cada familia viene de `tipoDeParticula` (canon §2: suma vs tapa, contorno oscuro de 1 px, 8 movimientos). Una carta o golpe migrados deben verse como hoy (misma familia, **mismo color base de `ParticulasDeCartas::colorDe`, misma opacidad**) salvo el cambio de reloj de D-O6.

## 9. Riesgos y trampas

- **R1. Dos modelos conviven**: funcion cerrada (brillo del equipo, cartas) vs simulacion con estado (emisor). Se solapan en familias y pintura; no mezclar en un mismo `Destello` vector sin que cada uno haga `clear()` (patron de `AuraJugador.cpp:138-142`).
- **R2. `tipoDeParticula` nunca falla**: devuelve `chispas` si el nombre no existe (`FamiliasDeParticulas.cpp:78-82`); un typo en datos se veria "bien". Validar con `familiasDeParticulas()` (E02, caso negativo).
- **R3. Correlacion de semillas de `Xoroshiro128p`**: el constructor fija `s0=seed`, `s1=C^seed` sin calentamiento (`RandomEngine.h:48-51`) y el primer `next_uint64 = s0+s1` (`RandomEngine.h:75`). Como `s + (C^s) = C + 2*(s & ~C)`, cualquier semilla cuyos bits esten contenidos en `C=0x853C49E6748FEA9B` da **exactamente el mismo primer valor `C`**: semillas 1, 2 y 3 dan `next_double = 0.5204511821398793` **y** `roll_in_range(0,999999) = 949339` (calculado en esta sesion con replica del constructor; Codex y Haiku lo confirmaron). Es decir, **tambien `roll_in_range` coincide en el primer sorteo** (corrige la version anterior de este contrato). Tampoco difiere el **segundo** `next_double` (0.370477 para las semillas 1, 2, 3, 4 y 1000, replica en esta sesion); el tercero ya difiere (0.431092, 0.806107, 0.681122, 0.056138...), y el primer `roll_in_range` varia para la semilla 4 (949347) y 1000 (950043). Con 4 sorteos descartados queda fuera de la zona correlada. Por eso: `mezclarSemilla` (splitmix64) **y** descartar los primeros 4 sorteos; test de guarda en E04.
- **R4. Sin snapshot del RNG**: `Xoroshiro128p` tiene `m_seed` const y estado privado (`RandomEngine.h:66-68`): no se puede guardar/restaurar a mitad. La reproduccion es "misma semilla desde el principio" (una partida guardada no reanuda particulas; no se guardan: son efimeras, **inferido** aceptable).
- **R5. `roll_in_range` y aritmetica**: `b-a+1` en `int` (`RandomEngine.h:60`); con rangos pequenos no hay problema; no usar para 64 bits.
- **R6. Reloj de pared y pausa**: el sistema de cartas/golpes ignora la pausa (`PintaDestellos.cpp:88,94-103`); el emisor la respeta: primer cambio visible (D-O6). Las particulas de mascotas **si** avanzan con `dt` (`ApplicationBucle.cpp:176`) pero **no** respetan `MOTOR_MOVIMIENTO_REDUCIDO` ni la pausa de golpe (corre fuera de `mundo`, `:141`); el emisor nuevo debe consultar la reduccion (D-O3) y E07 no debe asumir que "ninguna particula la consulta" cuando se compare con las mascotas.
- **R7. Residuo en coma flotante**: 60 x (1/60) x 10 no suma exactamente 10; usar tolerancia `kTolEmision` y test explicito (leccion de `U3-07_tweens.md` sobre `kTolFin`).
- **R8. Catch-up tras un tirón** (ventana minimizada, breakpoint): sin tope saldrian miles de particulas; `kMaxPasoEmision` descarta el exceso.
- **R9. Limpieza por cambio de mapa**: hoy `olvida()` solo se llama al terminar el combate (`ApplicationHud.cpp:182`); no hay gancho de cambio de mapa conocido (**inferido**: no se encontro en esta auditoria). Un emisor ligado a mapa (D-O1 B/C) necesita ese gancho antes de E07.
- **R10. Estado compartido en tests**: nada `static` mutable en el emisor; las unicas `static` aceptables son tablas `const` (`FamiliasDeParticulas.cpp:14`, `ParticulasDeCartas.cpp:61`). `static std::minstd_rand temblor(7)` (`ApplicationCinematica.cpp:495`) es el antipatron a no copiar.
- **R11. Coste GL por pixel**: `pintaDestellos` hace un `submit` por pixel de forma (`PintaDestellos.cpp:44`) y `MezclaAditiva` fuerza dos `flush()` por llamada (`:18-26`): muchos emisores = muchos flush. E07 debe pintar **todos los emisores en una sola llamada** (`out` concatenado) en vez de una por emisor.
- **R12. Ficheros calientes**: `CMakeLists.txt` (1639) lo editan varios agentes; 1 linea y `.cmake` propio. `ApplicationPantallas.cpp` a 497 lineas: cualquier adicion lo pasa de 500.
- **R13. Copia muerta** `include/Render/PergaminoGL 2.h`: editar la equivocada compila igual (nadie la incluye) y no hace nada. Editar `PergaminoGL.h`.
- **R14. Datos en python**: `tools/particulas.py` y `tools/test_validar_sprites.py:399` fijan que las dos listas de familias son la misma; si el emisor anadiera familias propias, romperia esa prueba. El emisor **no anade familias**.
- **R16. Escala y posicion por sprite**: `pintaDestellos` aplica `tam/64` a la posicion **y** al tamano del pixel (`PintaDestellos.cpp:32-33,42-44,55-58`) y `Destello` no lleva escala (`BrilloEquipo.h:91-97`): mezclar emisores de sprites con `tam` distinto en un `out` desplazaria posiciones (ej.: `x=110` con `tam.x=128` sale en 220). Solucion v1: `GrupoDeDibujo` por emisor y una llamada por grupo (E06/E07); `mundoDePixel` comun a pintura y test.
- **R17. Emisor no asignable por el RNG**: `Xoroshiro128p` tiene miembro `const` (`RandomEngine.h:68`), asi que se puede construir/mover pero **no asignar**: `std::vector<Emisor>` con `remove_if`+`erase` no compilaria si `Emisor` guardara el RNG por valor. Solucion v1: el `Emisor` guarda `std::unique_ptr<RPG::Xoroshiro128p>` (D-T10), sin tocar `RandomEngine.h`; `static_assert(std::is_move_assignable_v<Emisor>)` en el test.
- **R15. Dependencias de paquete** (ficha: U3-07, PREP.E10): no verificadas aqui mas alla de que `Interpolacion.h` existe en la base. Anotadas.

## 10. Cierre de E01

- Comandos: `grep -rIlniE "particula|particle|chispa|emitter|emisor|polvo|humo|niebla|SpriteBatch|instanc" src include editor examples tests cmake CMakeLists.txt`; `grep -rnE "EfectosDeCartas|ParticulasDeCartas|destellosDeCarta|pintaDestellos|destellosDeBrillo|tipoDeParticula|pixelesDeForma|familiasDeParticulas|contornoDeForma|MezclaAditiva|salpica|lanza"` (consumidores); `grep -rlE "RandomEngine|Xoroshiro"` (50 ficheros); `grep -rnE "\brand\(|srand|mt19937|random_device|uniform_real"`; `wc -l` de cada fichero de integracion; lectura con `sed -n` de cada tramo citado. Sin build ni tests (documental; build de fondo en curso).
- Registro de citas: `out/fabrica/REPLICA-R1/U4-07.E01_citas.log` (primera y ultima linea de cada rango + asercion de contenido).
- Base `628f417`. Dependencias de paquete y de E (PREP.E10) anotadas, no verificadas.
- Pendiente de Oriol: D-O1, D-O2, D-O3, D-O5 (D-O4 no bloquea).
