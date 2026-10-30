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
3. Lo que **falta** y es el paquete: un emisor **con estado** (taxa con residuo, buffer acotado, edad/vida, semilla propia) que el sistema actual no tiene; los emisores de hoy no se pueden "pausar", ni acotar, ni reproducir tras una pausa.
4. **No hay presupuesto**: `EfectosDeCartas::m_activas` es un `std::vector` sin tope (`EfectosDeCartas.h:59`, `PintaDestellos.cpp:89`) y `SpriteBatch::m_vertices` crece sin tope (`SpriteBatch.cpp:69-75`; 6 vertices x 32 B por pixel de particula). Este paquete fija el primero.
5. **Reloj de pared**: las particulas actuales usan `steady_clock` en `render` (`PintaDestellos.cpp:88,94-100,103`) e ignoran la pausa del critico; el emisor nuevo debe avanzar con el `dt` de update (decision D-T6).
6. **RNG**: `RPG::Xoroshiro128p` existe (`RandomEngine.h:46-85`) pero **su interfaz solo da `roll_d6` y `roll_in_range(int,int)`**, y los primeros `next_double` de semillas pequenas **coinciden** (calculado: seeds 1, 2, 3 -> mismo `next_double` inicial; ver R3). Hay que mezclar la semilla y usar `roll_in_range`.
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
| P15 | Numeros flotantes de dano: `GolpesEnPantalla::lanza/avanza` con `kVida` | `include/HUD/Pergamino/GolpesEnPantalla.h:23-36`; `src/HUD/Pergamino/GolpesEnPantalla.cpp:21-24` | `edad += dt` y poda | E I T (`tests/HUD/GolpesEnPantallaTests.cpp`). Patron de **estado con edad y poda** ya presente (texto, no particulas) |
| P16 | Niebla de guerra: textura RGBA al tamano del viewport, **no es particula** | `src/Render/FogOfWar.cpp:56-57` | textura | E I. Ni humo ni niebla ambiental existen como particulas de mapa |
| P17 | `SpriteBatch`: quads con textura; sin instancing: `submit` empuja **6 vertices** (32 B cada uno: `Vertex{pos,uv,color}`) a un `std::vector` sin tope; ring de 3 VBO | `include/Render/SpriteBatch.h:82-86`, `:92`; `src/Render/SpriteBatch.cpp:53-76` | -- | E I |
| P18 | RNG de azar del motor: `RPG::RandomEngine` (interfaz **solo** `roll_d6`, `roll_in_range(int,int)`, `seed()`) y `Xoroshiro128p` (header-only, miembro `const uint64_t m_seed`) | `include/RPG/RandomEngine.h:31-38`, `:46-85` (ctor `:48-52`, `roll_in_range` `:58-62`, `next_uint64` `:74-80`, `next_double` `:82-84`, `m_seed` const `:68`) | uint64 semilla; **no copiable/asignable** (miembro const) | E I T (50 ficheros que lo mencionan) |
| P19 | Doc del renderer ya preve el caso: "un efecto de particulas futuro probablemente no [querra sombra]" | `include/Render/IRenderable.h:16-29`, `getSortKey` `:33` | -- | solo comentario |

No existe en el repo (grep `particula|Particle|emitter|emisor|chispa|polvo|humo|niebla|SpriteBatch|instanc`
sobre `src include editor examples tests cmake CMakeLists.txt`): ningun emisor con estado, ningun buffer de
particulas, ningun instancing, ninguna niebla/humo ambiental de mapa, ningun fichero `Emisor*`.
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
    double vidaMin = 0.0, vidaMax = 0.0;      // s, (0..kMaxVida], vidaMin <= vidaMax
    double velocidadMin = 0.0, velocidadMax = 0.0;  // px/s, 0..kMaxVelocidad
    double anguloCentro = 0.0, abanico = 360.0;     // grados (0 = +x, y de pantalla hacia abajo), abanico (0..360]
    int    maximo = 0;            // aforo del emisor, 1..kMaxPorEmisor
    std::uint64_t semilla = 0;    // 0..2^53 (JSON numero double, sin perdida)
};
Result<DefinicionEmisor> leerEmisor(const JsonValue& json);   // Result global: Core/Errors/Result.h
```
- Claves JSON en espanol sin tildes: `familia`, `tasa`, `vida` (`[min,max]`), `velocidad` (`[min,max]`), `angulo`, `abanico`, `maximo`, `semilla`. Clave desconocida -> `Error` (D-T8).
- `familia` ausente o no listada en `familiasDeParticulas()` -> `Error` (recordar: `tipoDeParticula("no_existe")` devuelve `chispas` sin avisar, `FamiliasDeParticulas.cpp:78-82`). `tasa` NaN/inf/<0/`>kMaxTasa` -> `Error`; `vida<=0`, `vidaMax<vidaMin`, `vida>kMaxVida` -> `Error`; `maximo<1` o `>kMaxPorEmisor` -> `Error`; `semilla` negativa, no entera o `>2^53` -> `Error`.
- Limites explicitos (**propuestos, D-O2**): `kMaxTasa=500` part/s, `kMaxVida=10` s, `kMaxVelocidad=2000` px/s, `kMaxPorEmisor=256`.
- Archivos E02: `include/Game/Emisor/DefinicionEmisor.h`, `src/Game/Emisor/DefinicionEmisor.cpp`, `tests/Game/EmisorTests.cpp`, `cmake/Emisor.cmake`, 1 linea `include(cmake/Emisor.cmake)` en `CMakeLists.txt` tras el cierre de `motor_level` (`CMakeLists.txt:388`).
- Test/target: `test_emisor` (labels `unit;gl-free`), enlaza `motor_level motor_json`.

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
};
std::uint64_t mezclarSemilla(std::uint64_t semilla);                       // splitmix64
double uniforme01(RPG::RandomEngine& r);                                   // [0,1) en pasos de 1e-6 via roll_in_range(0, 999999)
Particula crearParticula(const DefinicionEmisor& d, const TipoDeParticula& tipo, RPG::RandomEngine& r);
```
- **Un `RPG::Xoroshiro128p` por emisor, propiedad del emisor**, construido con `mezclarSemilla(def.semilla)` (nunca `static`, nunca `rand()`, nunca `random_device`). Nunca se comparte entre emisores (el orden de consumo cambiaria al anadir un emisor).
- Orden de sorteos **fijo y documentado** (cambiarlo rompe repeticion): 1 `vida`, 2 `velocidad`, 3 `angulo`, 4 `variante`, 5 `otraForma`, 6 `ang0`, 7 `radio0`. Siempre se consumen los 7 (aunque la familia no use alguno) para que anadir una familia no cambie el flujo.
- Rangos: `vida in [vidaMin,vidaMax]`, `|v| in [velocidadMin,velocidadMax]`, direccion dentro de `anguloCentro +- abanico/2`.
- La velocidad inicial se compone con `tipo.vx/vy` segun el movimiento (D-T1); posicion inicial `(0,0)`.
- Archivos E04: `include/Game/Emisor/Particula.h`, `src/Game/Emisor/CrearParticula.cpp`, `tests/Game/EmisorTests.cpp`.

### E05 · Avanzar estado y caducar (90 min)

```cpp
// include/Game/Emisor/BufferParticulas.h
class BufferParticulas {                       // aforo fijo, memoria reservada en el constructor
public:
    explicit BufferParticulas(std::size_t aforo);                 // reserve(aforo): sin alloc despues
    std::size_t vivas() const; std::size_t aforo() const;
    bool anade(const Particula& p);              // false si lleno (D-T4: no se anade, NO consume azar)
    const std::vector<Particula>& datos() const;
    void vacia();
};
struct PasoParticulas { int caducadas = 0; int rechazadas = 0; };
Result<PasoParticulas> avanzarParticulas(BufferParticulas& b, const TipoDeParticula& tipo, double dt);   // dt: s
```
- `dt` NaN/inf/<0 -> `Error` y el buffer **no cambia**; `dt==0` -> sin cambios. Se acota `dt <= kMaxPasoEmision`.
- Integracion segun `tipo.movimiento` (portada de `BrilloEquipo.cpp:300-353`, D-T1): `Sube/Cae/Deriva/Salpica` (`gravedad = 2*|vy|/vida`, vuelve a su altura al final, `:301-313`) integran `x,y`; `Orbita/Converge/Expande` usan `ang0/radio0` y la fraccion `edad/vida` respecto al origen del emisor; `Chispazo` (rayo): vida corta (`edad>0.12` oculta, `:269-271`).
- `edad += dt` (float); caduca si `edad >= vida`. **Compactacion estable** (conserva el orden de nacimiento; D-T7), O(n), sin `erase` por elemento.
- Una particula caducada **no se proyecta** (E06 y la poda ocurren en el mismo `avanzar`).
- No toca GL ni reserva memoria: `datos().capacity()` es constante tras el constructor (invariante de test).
- Archivos E05: `include/Game/Emisor/BufferParticulas.h`, `src/Game/Emisor/BufferParticulas.cpp`, `src/Game/Emisor/AvanzarParticulas.cpp`, `tests/Game/EmisorTests.cpp`.

### E06 · Proyectar a instancias visuales (60 min)

```cpp
// include/Game/Emisor/ProyeccionParticulas.h   (gl-free)
struct EstiloEmisor {
    float alfaMax = 1.0f;          // 0..1
    float entrada = 0.08f;         // s de fundido de entrada (alfa 0 -> alfa)
    float salida  = 0.4f;          // s de fundido de salida  (alfa -> 0)
    float escala  = 1.0f;          // 1 = un pixel de frame por pixel de forma (canon: D-O4)
    bool  reducido = false;        // movimiento reducido: ver D-O3
};
void proyectarParticulas(const BufferParticulas& b, const DefinicionEmisor& d, const Vector2& origen,
                         const EstiloEmisor& estilo, std::vector<Game::Destello>& out);   // origen: px de mundo
```
- Salida: `Game::Destello` (px de mundo float; `alfa` 0..1; `forma`; `suma` = `tipo.suma`; `color` = `tipo.colores[variante]` o `{255,255,255}`). **`out` lo posee quien llama y se reutiliza** (`out.clear()` y reserva una vez, como `EfectosDeCartas::m_destellos`, `EfectosDeCartas.h:60`): "sin reservar por particula por fotograma".
- Alfa = `alfaMax * (1 - edad/vida)` (misma ley que `BrilloEquipo.cpp:275-277`) por fundidos `min(1,edad/entrada)` y `min(1,(vida-edad)/salida)` (misma ley que `ParticulasDeCartas.cpp:98-102`); curvas opcionales con `Interpolacion::curva` (`Interpolacion.h:33`).
- `Expande` emite 10 `Destello` por particula (aro, `BrilloEquipo.cpp:337-347`): `out.size()` puede ser `10*vivas`; el presupuesto de quads (seccion 5) cuenta esto.
- **Ni GL ni textura ni `Texture*`**: la pintura es `pintaDestellos(batch,out,Vector2{0,0},Vector2{64,64},blanco)` en el adaptador de E07 (`PintaDestellos.cpp:28-64`).
- Archivos E06: `include/Game/Emisor/ProyeccionParticulas.h`, `src/Game/Emisor/ProyeccionParticulas.cpp`, `tests/Game/EmisorTests.cpp` (o `EmisorProyeccionTests.cpp` si pasa de ~400 lineas).

### E07 · Conectar un emisor a una escena real (90 min)

Documento de adaptador + cambio minimo. Propuesta (D-O1, opcion A): `EfectosDeCartas::salpica(familia,esquina,tam)` pasa a crear/avanzar un `Emisor` interno (rafaga de `kDuracionDeCarta`) en lugar de `Activa` con reloj de pared; API publica sin cambios (`lanza`, `salpica`, `olvida`, `aLaVista`), por lo que `ApplicationHud.cpp:209` y `ApplicationPantallas.cpp:440-441` no cambian de firma.
- Propiedad: `PergaminoGL::m_efectos` (`PergaminoGL.h:92`) posee el estado; nace con `PergaminoGL()` (`PergaminoGL.cpp:171`), el mapa se carga en `carga` (`:177-180`); limpieza: `olvida()` ya se llama sin combate (`ApplicationHud.cpp:182`) y la baja de la cola en el cierre (`ApplicationCierre.cpp:119`). **Cambio de mapa**: hoy no hay gancho (D-O1/R9): el adaptador debe exponer `olvida()` y E07 decide el punto de llamada.
- El adaptador avanza con el `dt` de update (D-T6), no con `steady_clock`; el `render` solo proyecta+pinta (lectura).
- Movimiento reducido (`MOTOR_MOVIMIENTO_REDUCIDO`, patron `ApplicationHud.cpp:76`): politica D-O3.
- Archivos E07 (solo los que ya existen deben **adelgazar o crecer poco**): `include/Render/EfectosDeCartas.h` (66), `src/Render/PintaDestellos.cpp` (109), `src/Engine/ApplicationHud.cpp` (465; techo +35), `src/Engine/ApplicationPantallas.cpp` (**497; neto <= 0**), tests nuevos (no `BrilloEquipoTests.cpp`, 716). **No** tocar `Application.cpp` (1282), `CMakeLists.txt` (1639) salvo la linea de E02, `BrilloEquipo.cpp`, `FamiliasDeParticulas.cpp`, `PergaminoGL 2.h`.

### E08 · Medir limite y repetibilidad (60 min)

Documento + prueba gl-free: dos runs con la misma entrada y buffer lleno -> hash de estado identico; maximo respetado (`vivas <= aforo`, `rechazadas > 0`); coste medido en vertices por fotograma (`out.size()` x pixeles de forma x 6). **No se promete cifra de FPS** (ficha E08: "no inventar xifra"); se registra la maquina y el numero medido. Observacion visual = pendiente de Oriol (D-O5).

## 4. Orden por fotograma y propiedad del estado

Propietario: `Emisor` = `{DefinicionEmisor def; TipoDeParticula tipo; RPG::Xoroshiro128p rng; EstadoEmision emision; BufferParticulas buffer; Vector2 origen}`; lo posee el adaptador (`EfectosDeCartas` en E07), nada global ni `static`. Un hilo.

Update (con `dt` de `Application::update`, **pausable**: el mundo en pausa de critico no avanza particulas, a diferencia de hoy):
1. Consumo de eventos del juego -> `lanza/salpica` crean emisores o fijan origen (`ApplicationHud.cpp:175-212`, `ApplicationPantallas.cpp:433-442`).
2. `avanzarParticulas(buffer, tipo, dt)` — **primero** envejece y caduca las existentes.
3. `emisiones(estado, dt, tasa*escalaReducida)` -> `cuantas`; para cada una `crearParticula` + `anade` (las que no caben: `rechazadas`, sin sortear).
4. Las recien nacidas quedan con `edad=0` (no se envejecen en el fotograma en que nacen).
5. Poda de emisores acabados (rafaga terminada y buffer vacio).

Render (solo lectura, sin avanzar tiempo): `proyectarParticulas` -> `out` reutilizado -> `pintaDestellos` con `MezclaAditiva` dentro (`PintaDestellos.cpp:62`). Clave de orden: se mantiene `getSortKey() = INT_MAX-1` (`EfectosDeCartas.h:48`).

## 5. Presupuesto de particulas (puerta PRESSUPOST_PARTICULES)

Medido/estimado hoy (**inferido** salvo cita): una carta jugada produce `int(14.4 * vida * densidad) + 1` particulas (`parte=3` x `escalonDeAbalorio(8).particulas = 0.8+0.5*8 = 4.8`: `ParticulasDeCartas.cpp:86-90`, `BrilloEquipo.cpp:78`, `:254`): <= ~36 con `oro` (densidad 2) o `esporas`/`animas`. Un arma +10 emite 18 part/s (`BrilloEquipo.cpp:58`); varias fuentes a la vez, sin tope global. Coste por particula: hasta 16 pixeles de cuerpo (Calavera) + borde (`FamiliasDeParticulas.cpp:93,192-222`) = decenas de quads x 6 vertices x 32 B (`SpriteBatch.cpp:69-75`, `SpriteBatch.h:82-86`).

Presupuesto propuesto v1 (constantes unicas en `DefinicionEmisor.h`; **valores provisionales, D-O2**):

| Limite | Valor propuesto | Que pasa al superarlo |
|---|---|---|
| `kMaxPorEmisor` (aforo de un emisor) | 256 | `anade` devuelve `false`; la particula **no nace** y **no se sortea** (D-T4); `PasoParticulas::rechazadas++` |
| `kMaxEmisores` (activos a la vez, global) | 64 | `lanza/salpica` devuelve `false` (como hoy cuando la carta no echa particulas: `EfectosDeCartas.h:40`); no se evicta ninguno |
| `kMaxParticulasGlobal` (suma de vivas) | 2048 | los emisores nuevos nacen con aforo reducido al hueco; si no hay hueco, `false` |
| `kMaxQuadsPorFotograma` (suma de `Destello` x pixeles de forma) | 8192 (~1.5 MB de vertices/fotograma) | `proyectarParticulas` corta la salida y cuenta `descartadasVisuales`; el estado de simulacion no cambia (determinismo) |
| `kMaxPasoEmision` (catch-up por llamada) | 0.25 s | `recortado=true`, tiempo sobrante descartado |

Regla: **superar el presupuesto degrada visibilidad, nunca estado ni determinismo ni memoria**. Crecimiento
en memoria tras construir: 0 (`reserve` en constructor, E05). E08 mide; FPS/memoria dependen de maquina
acordada (ficha E08), no se declara cifra.

## 6. Casos de prueba deterministas (semilla)

Todos gl-free en `test_emisor`; semillas fijas; **los valores dorados se registran al implementar**
(E04/E05 ejecutan y fijan el hash), no se inventan aqui. Los de abajo son invariantes comprobables ya.

E02 `leerEmisor`: JSON valido -> campos exactos; negativos (cada uno `Error`): `familia` ausente; `familia:"no_existe"`; `tasa:-1`; `tasa:1e9`; `tasa:"x"`; `vida:[0,1]`; `vida:[2,1]`; `maximo:0`; `maximo:100000`; `semilla:-1`; `semilla:1.5`; `semilla:1e300`; clave desconocida. Mutacion: si el parser acepta `no_existe` (por delegar en `tipoDeParticula`), el test falla.
E03 `emisiones`: `(tasa=10,dt=0.05)` -> 0, residuo 0.5; segundo paso -> 1, residuo 0; `tasa=0` -> 0; `dt=0` -> 0; `dt=NaN` -> `Error`; `dt=5` -> `recortado`, `cuantas <= tasa*0.25 + 1`; **60 x (1/60) a tasa 10 == 10**; 1000 x 0.001 a tasa 7 == 7; particion 1 vs n equivalente.
E04 `crearParticula`: misma semilla -> 100 particulas bit a bit iguales (comparar `memcmp` de campos); rangos respetados en 10 000 muestras; **semillas 1 y 2 producen secuencias distintas** (>= 90% de particulas difieren en `vida`) — guarda R3; dos emisores con semillas distintas no comparten estado; ningun `rand/static` (revisar por `grep` en el cierre).
E05 `avanzarParticulas`: vida 1.0, pasos 0.5+0.5 -> caduca en el segundo; vida 1.0 y 100 pasos de 0.0099 -> caduca (tolerancia); aforo 8 con `tasa=500` durante 1 s: `vivas <= 8` siempre, `rechazadas > 0`, `capacity()` constante; `dt<0`/NaN -> `Error` y estado identico (hash antes == despues); orden estable tras caducidad; ninguna `edad >= vida` sobrevive.
E06 `proyectarParticulas`: `out.size() == vivas` para familias sin `Expande`; `10*vivas` con `ondas`; `alfa in [0,1]`; `suma` coincide con `tipo.suma` (comprobar `sombra` tapa, `luz` suma, `BrilloEquipoTests.cpp:90`); color dentro de `tipo.colores`; salida identica en dos llamadas; `escala` cambia solo el tamano.
E08 repetibilidad: dos runs de 600 pasos de `1/60` con buffer lleno -> hash FNV-1a de los bits de todas las particulas identico; con otra semilla, distinto.
Caso negativo obligatorio de la ficha ("Diff... prova amb cas negatiu"): cada E trae al menos uno (arriba, columna "Error"/"false").

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
src/Game/Emisor/BufferParticulas.cpp         (E05)
src/Game/Emisor/AvanzarParticulas.cpp        (E05)
include/Game/Emisor/ProyeccionParticulas.h   (E06)
src/Game/Emisor/ProyeccionParticulas.cpp     (E06)
tests/Game/EmisorTests.cpp                   (E02-E06)  target: test_emisor
cmake/Emisor.cmake                           (E02) + 1 linea include(...) en CMakeLists.txt (cierra motor_level en :388)
EXISTENTES QUE TOCA E07 (tamano -> regla)
src/Render/PintaDestellos.cpp                109   -> puede crecer
include/Render/EfectosDeCartas.h              66   -> puede crecer
src/Engine/ApplicationHud.cpp                465   -> techo +35 (<500)
src/Engine/ApplicationPantallas.cpp          497   -> neto <= 0 (solo adelgazar)
NO TOCAR: src/Engine/Application.cpp (1282), CMakeLists.txt (1639; solo la linea de E02),
          tests/Game/BrilloEquipoTests.cpp (716), src/Game/Personajes/BrilloEquipo.cpp (360),
          src/Game/Personajes/FamiliasDeParticulas.cpp (224), include/Render/PergaminoGL 2.h (copia muerta)
```
Ningun fichero nuevo previsto >500 lineas ni cabecera >15 miembros. Comando de prueba (E02+): `cmake --build <build> --target test_emisor && ctest --test-dir <build> -R "test_emisor|test_brillo_equipo"` (el segundo, para no romper el sistema actual). Esta E01 **no lanza builds** (build de fondo en curso).

## 8. Decisiones

### Tecnicas (Brain, D-T) — propuestas, bloqueo indicado

| ID | Pregunta | Recomendacion | Bloquea |
|---|---|---|---|
| D-T1 | Movimiento de particula: portar las 8 formulas (`BrilloEquipo.cpp:300-353`) a una funcion pura propia, o extraerlas de `destellosDeBrillo` | **Portar** (no se toca `BrilloEquipo.cpp`; sus 716 lineas de tests fijan el determinismo actual). Coste: duplicacion de ~50 lineas; mitigacion: test de paridad visual cualitativa (misma forma de trayectoria), no byte a byte | E05 |
| D-T2 | Salida de E06: `Game::Destello` existente o nuevo `SpriteInstances` | **`Destello`** (cero GL nuevo, reutiliza `pintaDestellos`); nuevo tipo solo si Oriol pide tamanos/rotaciones que `Destello` no expresa | E06 |
| D-T3 | RNG: `Xoroshiro128p` con `uniforme01` sobre `roll_in_range(0,999999)` + `mezclarSemilla` (splitmix64); alternativa: anadir `next_double` a `RandomEngine` | **Sin tocar `RandomEngine.h`** (50 ficheros que lo mencionan, C-21 "una sola interfaz", `RandomEngine.h:26-30`). Resolucion 1e-6 basta para px/vida | E04 |
| D-T4 | Al superar aforo: no admitir (drop-new) vs reciclar la mas vieja | **drop-new**, sin consumir azar (flujo reproducible y barato) | E04, E05 |
| D-T5 | Donde vive: `motor_level` via `cmake/Emisor.cmake` (con `target_sources`) vs `motor_math` | **`motor_level`** (depende de `FamiliasDeParticulas`, que esta alli, `CMakeLists.txt:364`) | E02 |
| D-T6 | Reloj: `dt` de update (pausable) vs `steady_clock` actual | **`dt` de update**; cambia el comportamiento durante la pausa del critico (`EfectosDeGolpe.h:30`) y es el motivo de que sea D-O1/D-O5 visible | E07 |
| D-T7 | Compactacion estable vs swap-remove | **Estable** (orden = nacimiento, necesario para proyeccion y hash reproducibles) | E05 |
| D-T8 | JSON: clave desconocida -> `Error` (estricto) | **Estricto** (evita erratas silenciosas, como el fallback a `chispas`) | E02 |

### De producto/visuales (propietario, D-O)

| ID | Pregunta | Opciones | Recomendacion | Bloquea |
|---|---|---|---|---|
| D-O1 | Primera escena real del emisor (E07) | **A** salpicaduras de golpe (chispas/polvo/sangre) y cartas ya existentes, migradas al emisor; **B** polvo de pasos del jugador (familia `polvo` "levantan los pasos", `CANON_PARTICULAS.md:110`), efecto nuevo visible en el mapa; **C** humo/niebla ambiental de mapa (requiere datos de mapa que no existen) | **A**: ya tiene familia, consumidores y tests; B despues (necesita gancho de pasos y limpieza por cambio de mapa) | E07 |
| D-O2 | Cifras de presupuesto (seccion 5) y maquina de referencia | aceptar las propuestas / dar otras / "reducidas para el portatil" | Aceptar las propuestas como provisionales; E08 las valida con medidas, sin prometer FPS | constantes de E02/E05 (se pueden fijar provisionales); cierre en E08 |
| D-O3 | Movimiento reducido (`MOTOR_MOVIMIENTO_REDUCIDO`) | apagar particulas no esenciales / reducir tasa a 25% / sin cambio | **Tasa x0.25 y sin parpadeo de aro (`Expande`)**; cartas y golpes siguen visibles | E07 |
| D-O4 | Tamano de particula (pregunta abierta `CANON_PARTICULAS.md:460-461`, canon §7.4) | 1 px de sprite (canon actual) / temas mas grandes | **1 px** en v1; `EstiloEmisor::escala` queda como hook | no bloquea (E06 usa 1.0) |
| D-O5 | Aprobacion visual/jugable de E07/E08 | observacion real de Oriol | no se declara por test | cierre de E07/E08 |

Paridad con el canon: el emisor **no inventa** colores/formas; cada familia viene de `tipoDeParticula`
(canon §2: suma vs tapa, contorno oscuro de 1 px, 8 movimientos). Una carta o golpe migrados deben verse
como hoy (misma familia y colores) salvo el fundido por `dt`.

## 9. Riesgos y trampas

- **R1. Dos modelos conviven**: funcion cerrada (brillo del equipo, cartas) vs simulacion con estado (emisor). Se solapan en familias y pintura; no mezclar en un mismo `Destello` vector sin que cada uno haga `clear()` (patron de `AuraJugador.cpp:138-142`).
- **R2. `tipoDeParticula` nunca falla**: devuelve `chispas` si el nombre no existe (`FamiliasDeParticulas.cpp:78-82`); un typo en datos se veria "bien". Validar con `familiasDeParticulas()` (E02, caso negativo).
- **R3. Correlacion de semillas de `Xoroshiro128p`**: el constructor fija `s0=seed`, `s1=C^seed` sin calentamiento (`RandomEngine.h:48-51`) y el primer `next_uint64 = s0+s1` (`RandomEngine.h:75`). Reproduje el calculo en esta sesion: semillas 1, 2 y 3 dan el mismo primer `next_double` (0.5204511821...). `roll_d6` y `next_double` usan los bits altos (`:54-56`, `:82-84`); `roll_in_range` usa `% range` (bajos, `:58-62`) y varia, pero con correlacion. Por eso: `mezclarSemilla` (splitmix64) **y** descartar los primeros 4 sorteos. Test de guarda en E04.
- **R4. Sin snapshot del RNG**: `Xoroshiro128p` tiene `m_seed` const y estado privado (`RandomEngine.h:66-68`): no se puede guardar/restaurar a mitad. La reproduccion es "misma semilla desde el principio" (una partida guardada no reanuda particulas; no se guardan: son efimeras, **inferido** aceptable).
- **R5. `roll_in_range` y aritmetica**: `b-a+1` en `int` (`RandomEngine.h:60`); con rangos pequenos no hay problema; no usar para 64 bits.
- **R6. Reloj de pared y pausa**: el sistema actual ignora la pausa (`PintaDestellos.cpp:88,94-103`); el emisor la respeta (D-T6): primer cambio de comportamiento visible.
- **R7. Residuo en coma flotante**: 60 x (1/60) x 10 no suma exactamente 10; usar tolerancia `kTolEmision` y test explicito (leccion de `U3-07_tweens.md` sobre `kTolFin`).
- **R8. Catch-up tras un tirón** (ventana minimizada, breakpoint): sin tope saldrian miles de particulas; `kMaxPasoEmision` descarta el exceso.
- **R9. Limpieza por cambio de mapa**: hoy `olvida()` solo se llama al terminar el combate (`ApplicationHud.cpp:182`); no hay gancho de cambio de mapa conocido (**inferido**: no se encontro en esta auditoria). Un emisor ligado a mapa (D-O1 B/C) necesita ese gancho antes de E07.
- **R10. Estado compartido en tests**: nada `static` mutable en el emisor; las unicas `static` aceptables son tablas `const` (`FamiliasDeParticulas.cpp:14`, `ParticulasDeCartas.cpp:61`). `static std::minstd_rand temblor(7)` (`ApplicationCinematica.cpp:495`) es el antipatron a no copiar.
- **R11. Coste GL por pixel**: `pintaDestellos` hace un `submit` por pixel de forma (`PintaDestellos.cpp:44`) y `MezclaAditiva` fuerza dos `flush()` por llamada (`:18-26`): muchos emisores = muchos flush. E07 debe pintar **todos los emisores en una sola llamada** (`out` concatenado) en vez de una por emisor.
- **R12. Ficheros calientes**: `CMakeLists.txt` (1639) lo editan varios agentes; 1 linea y `.cmake` propio. `ApplicationPantallas.cpp` a 497 lineas: cualquier adicion lo pasa de 500.
- **R13. Copia muerta** `include/Render/PergaminoGL 2.h`: editar la equivocada compila igual (nadie la incluye) y no hace nada. Editar `PergaminoGL.h`.
- **R14. Datos en python**: `tools/particulas.py` y `tools/test_validar_sprites.py:399` fijan que las dos listas de familias son la misma; si el emisor anadiera familias propias, romperia esa prueba. El emisor **no anade familias**.
- **R15. Dependencias de paquete** (ficha: U3-07, PREP.E10): no verificadas aqui mas alla de que `Interpolacion.h` existe en la base. Anotadas.

## 10. Cierre de E01

- Comandos: `grep -rIlniE "particula|particle|chispa|emitter|emisor|polvo|humo|niebla|SpriteBatch|instanc" src include editor examples tests cmake CMakeLists.txt`; `grep -rnE "EfectosDeCartas|ParticulasDeCartas|destellosDeCarta|pintaDestellos|destellosDeBrillo|tipoDeParticula|pixelesDeForma|familiasDeParticulas|contornoDeForma|MezclaAditiva|salpica|lanza"` (consumidores); `grep -rlE "RandomEngine|Xoroshiro"` (50 ficheros); `grep -rnE "\brand\(|srand|mt19937|random_device|uniform_real"`; `wc -l` de cada fichero de integracion; lectura con `sed -n` de cada tramo citado. Sin build ni tests (documental; build de fondo en curso).
- Registro de citas: `out/fabrica/REPLICA-R1/U4-07.E01_citas.log` (primera y ultima linea de cada rango + asercion de contenido).
- Base `628f417`. Dependencias de paquete y de E (PREP.E10) anotadas, no verificadas.
- Pendiente de Oriol: D-O1, D-O2, D-O3, D-O5 (D-O4 no bloquea).
