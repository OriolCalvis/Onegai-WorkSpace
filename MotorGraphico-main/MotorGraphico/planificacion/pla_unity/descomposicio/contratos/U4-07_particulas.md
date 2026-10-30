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
5. **Reloj de pared**: las particulas actuales usan `steady_clock` en `render` (`PintaDestellos.cpp:88,94-100,103`) e ignoran la pausa del critico; el emisor nuevo (polvo de pasos) avanza con el `dt` de `VidaNivel::avanza` (D-O6); cartas y golpes conservan su reloj de pared (no se migran).
6. **RNG**: `RPG::Xoroshiro128p` existe (`RandomEngine.h:46-85`) pero **su interfaz solo da `roll_d6` y `roll_in_range(int,int)`**, y los primeros `next_double` **y `roll_in_range`** de semillas pequenas **coinciden** (calculado: seeds 1, 2, 3 -> mismo `next_double` inicial; ver R3). Hay que mezclar la semilla (splitmix64) y descartar los primeros sorteos.
7. Hay 2 RNG no deterministas/estaticos cerca (no son particulas, no se tocan): `static std::minstd_rand temblor(7)` (`ApplicationCinematica.cpp:495`) y `std::random_device` (`:363`).
8. Ficheros >500 lineas que la integracion rozaria y solo pueden adelgazar: `Application.cpp` (1282), `CMakeLists.txt` (1639), `BrilloEquipoTests.cpp` (716); al borde: `ApplicationPantallas.cpp` (497).
9. **Decision de Oriol D-O1 = B**: la primera escena real (E07) es el **polvo de pasos del jugador** (efecto nuevo; familia `polvo` del canon, `CANON_PARTICULAS.md:110`). Cartas y golpes **no se migran** en U4-07: siguen con su sistema sin estado (migracion = paquete futuro, seccion 9bis).
10. E02-E06 pueden empezar sin decision de Oriol (solo D-T del Brain); E07 necesita D-O3 (movimiento reducido) y D-O6 (pausa del polvo); E08 necesita D-O2 y D-O5.

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
| `Game::Destello` (P7) como salida de E06 | **Reutilizar** (D-T2): E06 queda gl-free y la pintura es `pintaDestellos` sin codigo GL nuevo. Las coordenadas de un emisor son **px de frame del `GrupoDeDibujo`** del emisor (`mundo = esquina + (round(x)+dx) * tam/64`, `PintaDestellos.cpp:32-33,42-44`); un emisor sin sprite (el polvo) usa `GrupoDeDibujo{origenMundo, {64,64}}`, donde 1 px de frame = 1 px de mundo (`MontajePersonajes.h:63`). |
| `pintaDestellos`, `MezclaAditiva` (P10) | Reutilizar; E07 las llama. |
| `EfectosDeCartas` (P11) y su uso por cartas/golpes | **No se toca** en U4-07 (D-O1 = B). Migracion futura: seccion 9bis. |
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
con el patron de `cmake/Interpolacion.cmake:3-14` (`target_sources(...)` + tests con `add_test`,
`set_tests_properties(... LABELS "unit;gl-free")`, `set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS ...)`;
el `if(TARGET suite_gl_free)` solo sirve si la suite ya existe: ver `U3-07_tweens.md` seccion 6) pero sobre
`motor_level`, porque las familias viven alli (`CMakeLists.txt:363-364`) y no en `motor_math`. **Unidades**:
tiempo en **segundos (double)**; posicion y velocidad en **px del espacio del `GrupoDeDibujo` del emisor**
(px de frame; = px de mundo cuando `tam = 64x64`, caso del polvo): posicion **px**, velocidad **px/s**,
`kMaxVelocidad` en esos mismos px/s; **grados** solo en datos JSON (internamente radianes); tasa en
**particulas/s**. Sin `GridCoord` (entero logico, no se interpola) salvo en el adaptador de E07, que lo
convierte a px de mundo. Cabeceras <=15 miembros publicos y <=500 lineas por fichero. `DefinicionEmisor` ya
tiene 15 campos: **no admite mas** sin partirse.

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
- Claves JSON en espanol sin tildes: `familia`, `tasa`, `duracion`, `vida` (`[min,max]`), `velocidad` (`[min,max]`, px/s del espacio del grupo), `angulo`, `abanico`, `maximo`, `semilla`, `cortar` (bool), `opacidad`, `contraste`, `color` (`"rrggbb"`, 6 hex como `leerColor`, `ParticulasDeCartas.cpp:14-21`; otro formato -> `Error`, **no** el color por defecto silencioso). Clave desconocida -> `Error` (D-T8).
- `familia` ausente o no listada en `familiasDeParticulas()` -> `Error` (recordar: `tipoDeParticula("no_existe")` devuelve `chispas` sin avisar, `FamiliasDeParticulas.cpp:78-82`). `tasa` NaN/inf/<0/`>kMaxTasa` -> `Error`; `vida<=0`, `vidaMax<vidaMin`, `vida>kMaxVida` -> `Error`; `maximo<1` o `>kMaxPorEmisor` -> `Error`; `semilla` negativa, no entera o `>2^53` -> `Error`; `opacidad` fuera de (0,2] o `contraste` fuera de [0,1] -> `Error`.
- Limites explicitos (**propuestos, D-O2**): `kMaxTasa=500` part/s, `kMaxVida=10` s, `kMaxDuracion=60` s, `kMaxVelocidad=2000` px/s (del espacio del `GrupoDeDibujo`), `kMaxPorEmisor=256`.
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
    float x = 0, y = 0;           // px del espacio del grupo, relativo al origen del emisor
    float vx = 0, vy = 0;         // px/s
    float edad = 0, vida = 0;     // s; viva mientras edad < vida
    float ang0 = 0, radio0 = 0;   // rad / px: parametros fijos de Orbita/Converge/Expande (sorteados al nacer)
    std::uint16_t variante = 0;   // indice en TipoDeParticula::colores
    std::uint8_t  otraForma = 0;  // 0/1: usa TipoDeParticula::otraForma
    std::uint8_t  oscura = 0;     // 0/1: nace del tono oscuro que tapa (contraste)
    std::uint16_t k = 0;          // indice de nacimiento (balanceo sin estado oculto; lo asigna avanzarEmisor)
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
class BufferParticulas {                        // aforo fijo, memoria reservada en el constructor
public:
    explicit BufferParticulas(std::size_t aforo);                 // reserve(aforo): sin alloc despues
    std::size_t vivas() const; std::size_t aforo() const; bool llena() const;
    void anade(const Particula& p);               // precondicion: !llena()
    std::vector<Particula>& datos();              // acceso mutable: SOLO avanzarEmisor integra aqui
    const std::vector<Particula>& datos() const;
    std::size_t compacta();                       // retira las de edad>=vida, orden estable; devuelve cuantas
    void vacia();                                 // retira todas (olvida/corte)
};
// include/Game/Emisor/Emisor.h
struct GrupoDeDibujo {                 // DUENO: Emisor.h (E05). El espacio al que pertenecen las coordenadas del emisor
    Vector2 esquina{0,0};              // px de mundo de la esquina del dibujo
    Vector2 tam{64,64};                // px de mundo del dibujo; 64x64 = 1 px de frame por px de mundo
};
struct PresupuestoParticulas {                  // UNICO por adaptador; lo comparten todos sus emisores
    std::size_t maximo = kMaxParticulasGlobal;
    std::size_t vivas = 0;
    bool reserva();                             // vivas<maximo ? ++vivas,true : false
    void libera(std::size_t n);
    void recuenta(const std::vector<Emisor>& e);  // vivas = suma de buffer.vivas(): la FUENTE DE VERDAD
};
struct PasoParticulas { int nacidas = 0; int caducadas = 0; int rechazadas = 0; bool recortado = false; };
// dt: s. Hace: tiempo+=dt; envejecer+integrar+compactar (libera); luego nacer `cuantas` (E03) en orden.
Result<PasoParticulas> avanzarEmisor(Emisor& e, double dt, double escalaTasa, PresupuestoParticulas& p);
```
- `class Emisor` (`Emisor.h`, E05): `DefinicionEmisor`, `TipoDeParticula`, `std::unique_ptr<RPG::Xoroshiro128p>`, `EstadoEmision`, `BufferParticulas`, `Vector2 origen` (px del espacio del grupo), `GrupoDeDibujo grupo`, `double tiempo`; **movible y asignable por movimiento** (poda con `remove_if`+`erase`), no copiable. Con ese API, `avanzarEmisor` envejece (`datos()` mutable), compacta (`compacta()`), vacia en el corte (`vacia()`) y anade, sin acceder a internos.
- **Presupuesto global compartido, consumido en cada nacimiento y ANTES de sortear**: `if (buffer.llena() || !presupuesto.reserva()) { ++rechazadas; continue; }` y solo entonces `crearParticula`. El aforo calculado sobre vivas no basta (64 emisores recien creados y vacios sumarian 16 384 frente a 2048). Orden de admision **fijo**: emisores en orden de creacion; particulas de cada uno en orden de nacimiento. Las caducadas liberan antes de que nazcan las nuevas del mismo paso.
- **Fuga de presupuesto (H2)**: cualquier destruccion o poda forzada de un emisor (`olvida`, cambio de mapa, cierre, `acabado` por corte, destructor) deja `presupuesto.vivas` desfasado si solo se lleva la cuenta incremental. Regla: el presupuesto es **derivado**; `recuenta(emisores)` se llama (a) al inicio de cada `avanza` del adaptador, (b) tras `olvida()` y tras toda poda `erase`. Asi una fuga queda acotada a un tick y nunca persiste. Prueba obligatoria: crear un emisor, llenarlo y destruirlo sin que caduque, 100 veces -> tras `recuenta`, `vivas == 0` y se siguen admitiendo `maximo` nacimientos.
- `dt` NaN/inf/<0 -> `Error` y el emisor **no cambia**; `dt==0` -> sin cambios. Se acota con `kMaxPasoEmision` (**definida una sola vez, en `EmisionConResiduo.h`, E03**; aqui y en E03 solo se usa) y `recortado=true`.
- Movimientos (portados de `BrilloEquipo.cpp:300-353`, D-T1; `vida` = **`Particula::vida`**, la de la particula, no `tipo.vida`):
  - `Sube/Cae` y el resto por defecto (`BrilloEquipo.cpp:348-352`): `x += tipo.vx*sin(edad*3 + k)*0.4 + lado*tipo.vx*t*0.5`, `y += tipo.vy*t`, con `k` = **indice estable de nacimiento** (`Particula` lo guarda: `std::uint16_t k`) para el balanceo.
  - `Deriva` (`BrilloEquipo.cpp:314-319`): `x += sentido*tipo.vx*t + sin(edad*5 + k)*0.6`, `y += tipo.vy*t`.
  - `Salpica` (`BrilloEquipo.cpp:301-313`, **el unico con gravedad**): `g = 2*|tipo.vy|/Particula::vida`, `x += lado*tipo.vx*t`, `y += tipo.vy*t + 0.5*g*t*t` (vuelve a su altura al final de SU vida).
  - `Orbita/Converge/Expande` (`BrilloEquipo.cpp:321-347`): usan `ang0/radio0` y `edad/vida` respecto al `origen`.
  - **Ocultar** (rayo): si `tipo.forma == Rayo` **o** `tipo.movimiento == Chispazo`, y `edad > 0.12`, la particula no se proyecta (`BrilloEquipo.cpp:269-271`).
- `edad += dt` (float); caduca si `edad >= vida`. Compactacion estable (D-T7). Una caducada no se proyecta.
- Duracion y corte: `Emisor::tiempo` acumula el `dt` ya recortado. Con `duracion>0`, al cruzar `tiempo >= duracion - 1e-9` deja de emitir; si `cortar`, ademas **vacia el buffer en ese mismo paso** (liberando presupuesto) y `acabado()` es verdadero; si no `cortar`, `acabado()` = emision terminada y buffer vacio (las vivas se agotan visibles).
- Sin GL ni reservas: `datos().capacity()` constante tras el constructor.
- Archivos E05: `include/Game/Emisor/BufferParticulas.h`, `include/Game/Emisor/Emisor.h`, `src/Game/Emisor/BufferParticulas.cpp`, `src/Game/Emisor/AvanzarEmisor.cpp`, `tests/Game/EmisorTests.cpp`.

### E06 · Proyectar a instancias visuales (60 min)

```cpp
// include/Game/Emisor/ProyeccionParticulas.h   (gl-free; incluye Emisor.h, que ya declara GrupoDeDibujo)
// El mismo calculo que PintaDestellos.cpp:32-33,42-44,55-58: px = tam/64 (kLado); coordenada de frame redondeada ANTES de escalar.
// std::round explicito (mitades lejos de cero): round(-3.5) = -4, round(0.5) = 1.
Vector2 mundoDePixel(const GrupoDeDibujo& g, float xFrame, float yFrame, int dx, int dy);   // esquina de un pixel de forma
struct EstiloEmisor {
    float alfaMax = 1.0f;          // 0..1
    float entrada = 0.08f;         // s: envolvente del EMISOR, sube 0 -> 1 durante los primeros `entrada` s de Emisor::tiempo
    float salida  = 0.4f;          // s: SOLO con cortar=true: baja 1 -> 0 durante los ultimos `salida` s antes de `duracion`
    float escala  = 1.0f;          // 1 = un pixel de frame por pixel de forma (canon: D-O4)
};
struct ResumenProyeccion { std::size_t destellos = 0; std::size_t quads = 0; std::size_t descartados = 0; };
// quads = SUMA de pixelesDeForma(f).size() + contornoDeForma(f).size() de cada Destello con alfa>0
std::size_t quadsDeDestellos(const std::vector<Game::Destello>& d);
// descartados = numero de DESTELLOS (no quads) que no entran por quadsMax (tope restante del fotograma)
ResumenProyeccion proyectarParticulas(const Emisor& e, const EstiloEmisor& estilo, std::size_t quadsMax,
                                      std::vector<Game::Destello>& out);
```
- **Espacio de coordenadas**: posiciones y `origen` del emisor son **px de frame del `GrupoDeDibujo`** (declarado en `Emisor.h`, E05, y solo **usado** aqui: el ambito de E06 no toca `Emisor.h`). `mundo = esquina + (std::round(x) + dx) * tam/64` y tamano de pixel `tam/64` (`PintaDestellos.cpp:32-33`, `:42-44`, `:55-58`). `Destello` no lleva escala ni grupo (`BrilloEquipo.h:91-97`), por eso se pinta **una llamada a `pintaDestellos` por grupo distinto** (consecutivos con el mismo `esquina`/`tam` se agrupan). `reducido` (movimiento) no es del estilo: lo aplica el adaptador como `escalaTasa` (E07).
- E07 refactoriza `pintaDestellos` para llamar a `mundoDePixel` (misma formula en pintura y test).
- **Dos capas de alfa, separadas**: (a) **por vida de particula**: `min(1, vivo * d.opacidad)`, `vivo = 1 - edad/vida` (`BrilloEquipo.cpp:274-277`); (b) **envolvente por tiempo del emisor**: `entradaF = min(1, tiempo/entrada)`; `salidaF = cortar ? min(1, (duracion - tiempo)/salida) : 1` (solo si `duracion>0`). **Con `cortar=false` NO hay envolvente de salida**: las particulas se agotan visibles por (a) y el emisor puede seguir proyectando hasta vaciarse. Alfa final = `alfaMax * (a) * entradaF * salidaF`. Ejemplo `cortar=true`, `duracion=1.4`, `salida=0.4`: `tiempo=1.39` -> `salidaF=0.025`.
- **Corte**: si `cortar` y `tiempo >= duracion`, devuelve 0 destellos (corte duro).
- Color: `tipo.colores[variante]` o, si no hay, `d.colorBase` (`BrilloEquipo.cpp:278-280`); `oscura`: `*42/100` y `suma=false` (`:282-286`).
- **Contornos**: `pintaDestellos` hace un `submit` por pixel de cuerpo **y** de contorno (`PintaDestellos.cpp:37-44` cuerpo, `:48-58` contorno, solo con `alfa>0`). Calavera = 16 + 18 = 34 quads; `Voluta` (polvo) = 4 + 0. `quadsMax` corta en orden de nacimiento y cuenta `descartados`; el adaptador resta `quads` del tope restante al pasar al emisor siguiente.
- `out` lo posee quien llama y se acumula entre emisores **del mismo grupo** (`clear()` por grupo). `Expande` emite 10 `Destello` `Punto` por particula (`BrilloEquipo.cpp:337-347`).
- Curvas opcionales con `Interpolacion::curva` (`Interpolacion.h:33`). **Ni GL ni textura**.
- Archivos E06: `include/Game/Emisor/ProyeccionParticulas.h`, `src/Game/Emisor/ProyeccionParticulas.cpp`, `tests/Game/EmisorTests.cpp` (o `EmisorProyeccionTests.cpp`).

### E07 · Polvo de pasos del jugador (90 min)

**Decision de Oriol D-O1 = B.** Efecto nuevo y visible: al dar un paso, el jugador levanta una pequena nube de polvo en los pies. No hay paridad que igualar; cartas y golpes quedan como estan.

**Familia y forma (canon)**: `polvo` (Tierra; forma `Voluta`, movimiento `Deriva`, **tapa** (`suma=false`), colores `#c8aa82` `#a08664`; "polvo y arena que levantan los pasos", `CANON_PARTICULAS.md:110`; datos `FamiliasDeParticulas.cpp:70`). Una `Voluta` son 4 pixeles sin contorno (inferido de la tabla: contorno solo en las formas de tema, `FamiliasDeParticulas.cpp:192-222`). Un solo polvo para todos los suelos en v1.

**Evento (donde nace)**: cuando el jugador **cambia de celda a una celda adyacente**. `Player::moveTo` fija `gridPosition` al destino al empezar el paso (`src/Render/Player.cpp:149`; duracion `kStepDuration = 0.16 s`, `:27`), mientras que un muro (`Player::bump`, `:75-91`) **no** cambia de celda y no levanta polvo. La deteccion ya existe como patron en `Application::update` (`m_lastCameraCell`, `src/Engine/ApplicationBucle.cpp:164`) y `VidaNivel::avanza(dt, celdaJugador)` ya recibe la celda cada tick (`ApplicationBucle.cpp:176`, `src/Render/VidaNivel.cpp:366-369`): **el detector vive en el adaptador, no en `Application`**, comparando la celda con la del tick anterior. El polvo se levanta en la **celda que se deja**, en los pies: `pies = IsoMath::gridToScreen(celdaAnterior, tileW, tileH) + (tileW*0.5, tileH*0.9)` (misma formula que `AuraJugador.cpp:76`; la elevacion del terreno (`ApplicationBucle.cpp:167`) **se ignora en v1**, inferido: `VidaNivel` no recibe el mapa de alturas).

**Salto de celda no adyacente o centinela** (carga de nivel, teletransporte, cinematica: `ApplicationCinematica.cpp:300` llama `avanza(dt, GridCoord{-1000,-1000})`): no hay polvo y **se hace `olvida()`** de todos los emisores; la primera celda tras `monta`/`olvida` solo se memoriza.

**Parametros (propuestos, D-O2/D-O4)** por paso: un `Emisor` con `familia="polvo"`, `tasa=40` part/s, `duracion=0.15` s (=> ~6 particulas), `cortar=false` (se agotan visibles), `vida=[0.5,0.9]` s, `velocidad=[3,8]` px/s, `angulo=-90` (hacia arriba en pantalla), `abanico=140` grados, `maximo=16`, `opacidad=1`, `contraste=0`, `semilla = mezclarSemilla(llavor ^ contadorDePasos)`; `EstiloEmisor{alfaMax=0.7, entrada=0.02}` (translucido: **inferido**, a ajustar con Oriol en D-O5). `GrupoDeDibujo{esquina = pies, tam = {64,64}}`, `origen = {0,0}`.

**Quien lo posee y donde se pinta**: dos piezas, para no obligar a enlazar `glad` (M3):
- `Game::Emisor::PasosDePolvo` (**gl-free**, `motor_level`): posee `std::vector<Emisor>`, **un** `PresupuestoParticulas{maximo=256}`, la ultima celda y el contador de pasos. API: `monta(std::uint32_t llavor, int tileW, int tileH)`, `olvida()`, `avanza(double dt, const GridCoord& celda, bool reducido)`, `const std::vector<Emisor>& emisores() const`. Tope `kMaxEmisoresPolvo = 16` (rechazo = no se crea ese puff; no se evicta ninguno).
- `Render::PolvoDePasos : IRenderable` (GL, `motor_core`, junto a `PintaDestellos.cpp` en `CMakeLists.txt:809`): contiene un `PasosDePolvo` y `Texture* blanco`; `render(batch)` proyecta cada emisor (`proyectarParticulas`) y pinta con `pintaDestellos(batch, out, grupo.esquina, grupo.tam, blanco)` (una llamada por grupo; con 16 emisores = hasta 16 pares de flush, como hoy una por `Activa`, `PintaDestellos.cpp:101-105`: R11). `getSortKey()`: clave de la celda del polvo mas reciente con el mismo calculo que las entidades (`Entity.cpp:77-80`, `include/Render/CapaDeDibujo.h:41`) y `sesgo` -1 para quedar justo bajo una entidad en esa celda (detalle **inferido**; se fija en E07).
- Propietario: **`Render::VidaNivel`** (la capa de vida del nivel: `monta` por `loadLevel`, `avanza` por tick, `desmonta` al salir; `include/Render/VidaNivel.h:32-48`) con un miembro `PolvoDePasos m_polvo`. Cambios (VidaNivel.h 61 lineas, VidaNivel.cpp 371): `monta` -> `m_polvo.monta(llavor, tileW, tileH)` + `renderer.removeFromQueue(&m_polvo); renderer.addToQueue(&m_polvo)` (`removeFromQueue` es no-op si no estaba, `include/Render/IsometricRenderer.h:76-81`, `src/Render/IsometricRenderer.cpp:61-65`); `avanza` -> `m_polvo.avanza(dt, jugador, reducido)`; `desmonta` -> `renderer.removeFromQueue(&m_polvo)` + `olvida()`. Textura blanca 1x1: `VidaNivel::ponBlanco(Texture*)`, llamada desde donde `m_aura->ponBrillo(..., m_whiteTexture.get())` ya recibe la blanca (`src/Engine/ApplicationHud.cpp:316`; `m_whiteTexture` nace en `Application.cpp:551`); **no** se toca `Application.cpp` (1282 lineas). E07 verifica que esa linea corre tras crear `m_vida` (`Application.cpp:368`); si no, otro punto de `ApplicationHud.cpp`.
- **Limpieza al cambiar de mapa (gancho)**: `loadLevel` llama a `m_vida->monta(...)` (`src/Engine/ApplicationNivel.cpp:359`, dentro del bloque de `:327`) -> `monta` hace `m_polvo.olvida()` primero; ademas el salto de celda no adyacente (arriba) y el centinela de cinematica olvidan por su cuenta; `ApplicationCierre.cpp:133-134` desmonta al salir. `ApplicationNivel.cpp` (506 lineas) **no se toca**. Respaldo: el polvo vive < 1.1 s, asi que un gancho fallido no deja residuos permanentes. `olvida()` termina con `presupuesto.recuenta(emisores)` (H2).
- **Pausa y reloj (D-O6, limitada a este efecto)**: `PasosDePolvo::avanza` recibe el `dt` de `VidaNivel::avanza` (`ApplicationBucle.cpp:176` pasa `deltaTime`, no `mundo`), igual que las mascotas: se congela cuando `update` no corre. Es una llamada por tick fijo de 1/60 s (`ApplicationBucle.cpp:85`, `ApplicationLoop.h:14`), hasta 6 por render (`ApplicationLoop.h:22`): un puff recien creado puede llegar a su primer render con hasta 5 ticks de edad; no se asume `edad=0`.
- **Movimiento reducido (D-O3)**: `reducido` se lee con el patron `static const bool` de `ApplicationHud.cpp:76` (`MOTOR_MOVIMIENTO_REDUCIDO`) en `VidaNivel::avanza` y se pasa como `escalaTasa` (0 = apagado).
- **Ambito E07** (con topes): nuevos `include/Game/Emisor/PasosDePolvo.h`, `src/Game/Emisor/PasosDePolvo.cpp` (gl-free; en `cmake/Emisor.cmake`), `include/Render/PolvoDePasos.h`, `src/Render/PolvoDePasos.cpp` (GL), `tests/Game/PolvoDePasosTests.cpp` (target **`test_polvo_pasos`**, `unit;gl-free`, enlaza `motor_level motor_json`, **sin `glad`**: solo prueba `PasosDePolvo`); existentes: `include/Render/VidaNivel.h` (61; +~10), `src/Render/VidaNivel.cpp` (371; +~25), `src/Engine/ApplicationHud.cpp` (465; techo +34), `src/Render/PintaDestellos.cpp` (109; refactor a `mundoDePixel`), `CMakeLists.txt` (**una** linea `src/Render/PolvoDePasos.cpp` junto a `CMakeLists.txt:809`; 1639 lineas, unica adicion permitida ademas de la de E02). **No** tocar `Application.cpp`, `ApplicationNivel.cpp` (506), `ApplicationPantallas.cpp` (497), `EfectosDeCartas.*`, `PergaminoGL.*`, `Player.cpp`, `BrilloEquipo.cpp`, `MascotesNivel`/`VidaNivel` en lo que afecte a mascotas, `include/Render/PergaminoGL 2.h`, `src/Engine/ApplicationPantallas 2.cpp` (copia muerta, R13).

### E08 · Medir limite y repetibilidad (60 min)

Documento + prueba gl-free: dos runs con la misma entrada y buffer lleno -> hash de estado identico; maximo respetado (`vivas <= aforo` y `presupuesto.vivas <= maximo`, `rechazadas > 0`); coste medido en **quads por fotograma** con `quadsDeDestellos` (cuerpo + contorno; con `calavera` el caso peor). `SpriteBatch::m_vertices` es privado (`SpriteBatch.h:94`), asi que el numero real de `submit` no se lee desde un test gl-free: la prueba compara `quadsDeDestellos` contra un recuento independiente que replica las condiciones de `pintaDestellos` (`alfa>0`, `pixelesDeForma`+`contornoDeForma`), y la medida real en un ejecutable GL queda como observacion manual de E08. **No se promete cifra de FPS** (ficha E08). Fichero de prueba propio: `tests/Game/EmisorCargaTests.cpp`, target `test_emisor_carga` en `cmake/Emisor.cmake`. Observacion visual = pendiente de Oriol (D-O5). Medida concreta del polvo: caminar sin parar (un paso cada 0,16 s) debe mantener `emisores <= 16` y `vivas <= 256`.

## 4. Orden por tick y propiedad del estado

Propietario: el adaptador (`PasosDePolvo` en E07) posee `std::vector<Emisor>` (orden de creacion) y **un** `PresupuestoParticulas`; nada global ni `static`. Un hilo.

Update (una vez por tick fijo de 1/60 s dentro de `Application::update` -> `VidaNivel::avanza`, `ApplicationBucle.cpp:176`; hasta 6 ticks por render):
1. `presupuesto.recuenta(emisores)` (fuente de verdad, H2).
2. Deteccion de paso: celda actual vs ultima -> si es adyacente, crear el emisor del puff (en los pies de la celda anterior); si no, `olvida()` o solo memorizar.
3. Para cada emisor en orden de creacion, `avanzarEmisor` = (a) envejecer, integrar y compactar (libera); (b) `emisiones(...)` -> `cuantas`; (c) cada nacimiento: `llena()||!reserva()` -> `rechazadas`, si no `crearParticula`+`anade`. Los recien nacidos quedan con `edad=0`.
4. Poda de emisores `acabado()` (`erase` estable) seguida de `recuenta`.

Render (solo lectura, sin avanzar tiempo): para cada grupo de emisores consecutivos con el mismo `GrupoDeDibujo`: `out.clear()`, `proyectarParticulas(..., quadsRestantes, out)` por emisor, `pintaDestellos(batch,out,grupo.esquina,grupo.tam,blanco)` con `MezclaAditiva` dentro (`PintaDestellos.cpp:62`).

## 5. Presupuesto de particulas (puerta PRESSUPOST_PARTICULES)

Medido/estimado hoy en cartas, que **no se migran** (**inferido** salvo cita): una carta jugada produce `int(14.4 * vida * densidad) + 1` particulas (`parte=3` x `escalonDeAbalorio(8).particulas = 0.8+0.5*8 = 4.8`: `ParticulasDeCartas.cpp:86-90`, `BrilloEquipo.cpp:78`, `:254`): <= ~36 con `oro` o `esporas`/`animas`. Un arma +10 emite 18 part/s (`BrilloEquipo.cpp:58`); varias fuentes a la vez, sin tope global. **Las mascotas suman aparte**: `mg.quantes` particulas por mascota, una entidad (un quad de 4 frames) por particula (`VidaNivel.cpp:247-252`, **inferido** 1 quad), sin tope; no entran en este presupuesto (D-T9) pero se registran como consumo concurrente en E08. Coste por particula del emisor: cuerpo + contorno (Calavera 16 + 18 = 34 quads; `FamiliasDeParticulas.cpp:93,192-222`) x 6 vertices x 32 B (`SpriteBatch.cpp:69-75`, `SpriteBatch.h:82-86`).

Presupuesto propuesto v1 (constantes unicas en `DefinicionEmisor.h`; **valores provisionales, D-O2**):

| Limite | Valor propuesto | Que pasa al superarlo |
|---|---|---|
| `kMaxPorEmisor` (aforo de un emisor) | 256 | el nacimiento se **rechaza antes de sortear**; `PasoParticulas::rechazadas++` |
| `kMaxEmisores` (activos a la vez, generico) | 64 | la creacion devuelve `false`; no se evicta ninguno (el polvo usa su propio `kMaxEmisoresPolvo = 16`) |
| `kMaxParticulasGlobal` (vivas entre todos, `PresupuestoParticulas`) | 2048 | **cada nacimiento reserva antes de sortear**; sin hueco, se rechaza. Garantia por reserva, no por aforo (64 x 256 = 16 384 > 2048): prueba con 64 emisores vacios simultaneos |
| `kMaxQuadsPorFotograma` (**cuerpo + contorno** de todos los `Destello` con `alfa>0`) | 8192 (~1.5 MB de vertices/fotograma; =240 calaveras de 34 quads; 512 calaveras serian 17 408) | `proyectarParticulas` corta la salida por orden de nacimiento y cuenta `descartados`; el estado de simulacion no cambia |
| `kMaxPasoEmision` (catch-up por llamada; **definida solo en `EmisionConResiduo.h`**) | 0.25 s | `recortado=true`, tiempo sobrante descartado |
| Polvo de pasos (E07): `kMaxEmisoresPolvo` / aforo / `PresupuestoParticulas.maximo` | 16 / 16 / 256 | no se crea el puff / no nace / no nace; peor caso 256 x 4 quads (`Voluta`) = 1024 quads, muy por debajo de 8192 |

Regla: **superar el presupuesto degrada visibilidad, nunca estado ni determinismo ni memoria**. Crecimiento en memoria tras construir: 0 (`reserve` en constructor). Politica de rechazo (drop-new) tiene efecto visible (que destellos no salen en una rafaga muy cargada): se somete a Oriol dentro de D-O2. E08 mide; FPS/memoria dependen de maquina acordada.

## 6. Casos de prueba deterministas (semilla)

Todos gl-free en `test_emisor`/`test_emisor_carga`; semillas fijas; **los valores dorados se registran al implementar** (E04/E05 ejecutan y fijan el hash), no se inventan aqui.

E02 `leerEmisor`: JSON valido -> campos exactos; negativos (cada uno `Error`): `familia` ausente; `familia:"no_existe"`; `tasa:-1`; `tasa:1e9`; `tasa:"x"`; `vida:[0,1]`; `vida:[2,1]`; `maximo:0`; `maximo:100000`; `semilla:-1`; `semilla:1.5`; `semilla:1e300`; `opacidad:0`; `contraste:2`; `color:"zzzzzz"`; clave desconocida. Mutacion: si el parser acepta `no_existe` (delegando en `tipoDeParticula`), falla.
E03 `emisiones`: `(tasa=10,dt=0.05)` -> 0, residuo 0.5; segundo paso -> 1, residuo 0; `tasa=0` -> 0; `dt=0` -> 0; `dt=NaN` -> `Error`; `dt=5` -> `recortado`, `cuantas <= tasa*0.25 + 1`; **60 x (1/60) a tasa 10 == 10**; 1000 x 0.001 a tasa 7 == 7; particion 1 vs n equivalente (sin recorte).
E04 `crearParticula`: misma semilla -> 100 particulas bit a bit iguales; rangos respetados en 10 000 muestras; **semillas 1 y 2 producen secuencias distintas** (>= 90% de particulas difieren en `vida`) tras `mezclarSemilla` y descarte (guarda R3); `static_assert(std::is_move_assignable_v<Emisor>)` y poda de un vector de emisores conserva el RNG de los supervivientes (R17); dos emisores con semillas distintas no comparten estado.
E05 (**casos corregidos; el recorte `kMaxPasoEmision=0.25` se aplica**): vida 1.0 con pasos `0.5+0.5` -> cada paso se recorta a 0.25, `edad=0.5`, **sobrevive** y `recortado=true`; vida 1.0 con `4 x 0.25` -> `edad=1.0`, **caduca** (exacto en float); vida 0.2 con un paso `0.25` -> **caduca** (un paso que cruza la vida); `100 x 0.0099` (edad ~0.99, en float 0.9899988...) -> **sobrevive**, y `102 x 0.0099` -> caduca; recorte y precision se prueban en casos separados. Aforo 8 con `tasa=500` durante 1 s: `vivas <= 8`, `rechazadas > 0`, `capacity()` constante. **Presupuesto global**: 64 emisores vacios, `tasa=500`, `maximo=256`, `PresupuestoParticulas.maximo=2048`: tras un paso, `sum(vivas) <= 2048` y `== presupuesto.vivas`; el reparto es el de la orden de creacion (los primeros emisores consumen hasta agotar), identico en dos runs; tras caducar todo, `presupuesto.vivas==0`. `dt<0`/NaN -> `Error` y estado identico. Orden estable tras caducidad. **H2**: crear, llenar y destruir un emisor sin caducar x100 -> tras `recuenta`, `presupuesto.vivas == 0` y se admiten `maximo` nacimientos; `olvida()` deja `vivas == 0`. Movimientos: `Salpica` con `vida` de particula 0.5 vuelve a `y=0` en `edad=0.5` (gravedad con `Particula::vida`, no con `tipo.vida`); `rayos` con `edad=0.13` -> oculta.
E06 `proyectarParticulas`: `destellos == vivas` salvo `Expande` (`10*vivas`); `alfa in [0,1]`; `suma` coincide con `tipo.suma`; **paridad de color/opacidad**: `agua` con `colorBase=5ab4ff` -> todos los `Destello.color == {0x5a,0xb4,0xff}`; `chispas` con `colorBase=ffd28c` -> `{0xff,0xd2,0x8c}`; con `opacidad=1.45` y `vivo=0.5` el alfa de particula es `0.725` y con `vivo=0.8` es `1.0`; `contraste=1` -> todas `suma=false` y color `*42/100`. **Envolvente `cortar=true`** (`duracion=1.4`, `entrada=0.08`, `salida=0.4`): `tiempo=0.04` -> entrada `0.5`; `tiempo=0.08` y `0.5` -> `1` (clamp: tiempo >= entrada); `tiempo=1.39` con particula `edad=0.10`, `vida=1` -> salida `0.025` (tolerancia `1e-6`); `tiempo=1.40` -> 0 destellos y buffer vacio, presupuesto liberado; nada nacido en los ultimos 0.1 s sobrevive. **Envolvente `cortar=false`** (`duracion=1`, `tiempo=0.99`): salida **ausente** (factor 1); `tiempo=1.2` con particulas vivas -> siguen proyectandose y se agotan por su `vivo`. **Calavera**: 1 particula -> `quadsDeDestellos == 34`; **dos emisores de 256** (512 calaveras en total, no caben en un solo emisor) con `quadsMax=8192`: se proyectan `floor(8192/34) = 240` y `descartados == 512 - 240 = 272` (cuenta destellos); `escala` cambia solo el tamano. **Transformacion mundo/grupo**: `mundoDePixel(A,10,20,1,0) == (111,70)` y `mundoDePixel(B,10,20,1,0) == (322,120)` con A `esquina=(100,50), tam=64`, B `esquina=(300,80), tam=128` (tamano de pixel 1 y 2), con forma `Dragon` (cuerpo y contorno); **negativos**: `mundoDePixel(B, 0.5, -3.5, -1, -1) == (300 + (1-1)*2, 80 + (-4-1)*2) == (300, 70)` (`round(0.5)=1`, `round(-3.5)=-4`).
E08 repetibilidad: dos runs de 600 pasos de `1/60` con buffer lleno -> hash FNV-1a de los bits de todas las particulas identico; con otra semilla, distinto.
**E07 (`test_polvo_pasos`, gl-free)**: `(5,5)->(6,5)` -> 1 emisor, `grupo.esquina == gridToScreen((5,5),tw,th) + (tw*0.5, th*0.9)`; 100 ticks en la misma celda -> ningun emisor nuevo; muro (misma celda) -> ninguno; primera llamada tras `monta` -> ninguno; salto `(5,5)->(40,40)` o centinela `(-1000,-1000)` -> `emisores()==0` y presupuesto 0; `reducido=true` -> 0 particulas nacidas; mismo `llavor` y misma ruta en dos instancias -> hash identico, otro `llavor` -> distinto; 20 pasos seguidos a 0,16 s -> `emisores <= 16` y `vivas <= 256`; 2 s sin pasos -> `emisores()==0`; 100 ciclos `monta`/pasos/`olvida` -> presupuesto 0.
Caso negativo obligatorio de la ficha: cada E trae al menos uno (arriba).

## 7. Ambito de ficheros congelado (propuesto)

```
NUEVOS (0 lineas hoy)
include/Game/Emisor/DefinicionEmisor.h       (E02)
src/Game/Emisor/DefinicionEmisor.cpp         (E02)
include/Game/Emisor/EmisionConResiduo.h      (E03)  <- unica definicion de kMaxPasoEmision
src/Game/Emisor/EmisionConResiduo.cpp        (E03)
include/Game/Emisor/Particula.h              (E04)
src/Game/Emisor/CrearParticula.cpp           (E04)
include/Game/Emisor/BufferParticulas.h       (E05)
include/Game/Emisor/Emisor.h                 (E05)  <- Emisor, GrupoDeDibujo, PresupuestoParticulas
src/Game/Emisor/BufferParticulas.cpp         (E05)
src/Game/Emisor/AvanzarEmisor.cpp            (E05)
include/Game/Emisor/ProyeccionParticulas.h   (E06)  <- usa Emisor.h, no lo edita
src/Game/Emisor/ProyeccionParticulas.cpp     (E06)
tests/Game/EmisorTests.cpp                   (E02-E06)  target: test_emisor
tests/Game/EmisorCargaTests.cpp              (E08)      target: test_emisor_carga
include/Game/Emisor/PasosDePolvo.h           (E07, gl-free)
src/Game/Emisor/PasosDePolvo.cpp             (E07, gl-free)
tests/Game/PolvoDePasosTests.cpp             (E07)      target: test_polvo_pasos (sin glad)
include/Render/PolvoDePasos.h                (E07, GL)
src/Render/PolvoDePasos.cpp                  (E07, GL)  -> 1 linea en CMakeLists.txt junto a :809
cmake/Emisor.cmake                           (E02, E07, E08) + 1 linea include(...) en CMakeLists.txt (cierra motor_level en :388)
EXISTENTES QUE TOCA E07 (tamano -> regla)
include/Render/VidaNivel.h                    61   -> +~10
src/Render/VidaNivel.cpp                     371   -> +~25 (solo polvo; sin tocar mascotas)
src/Engine/ApplicationHud.cpp                465   -> techo +34 (<500): la linea ponBlanco
src/Render/PintaDestellos.cpp                109   -> refactor a mundoDePixel
NO TOCAR: src/Engine/Application.cpp (1282), src/Engine/ApplicationNivel.cpp (506), CMakeLists.txt (1639; solo las
          dos lineas dichas), src/Engine/ApplicationPantallas.cpp (497), tests/Game/BrilloEquipoTests.cpp (716),
          src/Game/Personajes/BrilloEquipo.cpp (360), src/Game/Personajes/FamiliasDeParticulas.cpp (224),
          include/Render/EfectosDeCartas.h, include/Render/PergaminoGL.h/.cpp, src/Render/Player.cpp,
          include/Render/PergaminoGL 2.h y src/Engine/ApplicationPantallas 2.cpp (copias muertas)
```
Ningun fichero nuevo previsto >500 lineas ni cabecera >15 miembros. Comando de prueba: `cmake --build <build> --target test_emisor test_polvo_pasos && ctest --test-dir <build> -R "test_emisor|test_polvo_pasos|test_brillo_equipo"` (el ultimo, para no romper el sistema actual). Esta E01 **no lanza builds**.

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

**Decidida**: D-O1 = **B, polvo de pasos del jugador** (Oriol). Cartas/golpes no se migran.

| ID | Pregunta | Opciones | Recomendacion | Bloquea |
|---|---|---|---|---|
| D-O2 | Cifras de presupuesto (seccion 5: 256 por emisor, 64 emisores, 2048 globales, 8192 quads, 0,25 s de catch-up; **polvo: 16 emisores, 16 por puff, 256 vivas**), politica de rechazo (drop-new) y maquina de referencia | aceptar / dar otras / reducidas para el portatil | Aceptar como provisionales; E08 las valida con medidas, sin prometer FPS | no bloquea E02-E06; cierra en E08 |
| D-O3 | Movimiento reducido (`MOTOR_MOVIMIENTO_REDUCIDO`) para el **polvo de pasos** | **apagado** / tasa al 25% / sin cambio | **Apagado**: el polvo es decorativo, no informa | E07 |
| D-O4 | Aspecto del polvo: tamano (1 px de frame, canon §7.4 `CANON_PARTICULAS.md:460-461`), opacidad (0,7) y un solo polvo para todos los suelos | 1 px y un polvo / mas grande / polvo por tipo de suelo | **1 px, un polvo** en v1; ajustar al verlo (D-O5) | nada (valores iniciales ajustables) |
| D-O5 | Aprobacion visual/jugable del polvo (E07) y de la carga (E08) | observacion real de Oriol | no se declara por test | cierre de E07/E08 |
| D-O6 | Pausa del polvo (limitada a este efecto): sigue el reloj de `VidaNivel::avanza` (`deltaTime`, como las mascotas) | seguir a `VidaNivel` / congelar tambien en los 70 ms de pausa del critico (usar `mundo`) | **Seguir a `VidaNivel`**: no hay critico en exploracion | E07 |

Paridad con el canon: el emisor **no inventa** colores/formas; cada familia viene de `tipoDeParticula` (canon §2: suma vs tapa, contorno oscuro de 1 px, 8 movimientos). El polvo usa la familia `polvo` tal cual.

## 9. Riesgos y trampas

- **R1. Dos modelos conviven**: funcion cerrada (brillo del equipo, cartas) vs simulacion con estado (emisor). Se solapan en familias y pintura; no mezclar en un mismo `Destello` vector sin que cada uno haga `clear()` (patron de `AuraJugador.cpp:138-142`).
- **R2. `tipoDeParticula` nunca falla**: devuelve `chispas` si el nombre no existe (`FamiliasDeParticulas.cpp:78-82`); un typo en datos se veria "bien". Validar con `familiasDeParticulas()` (E02, caso negativo).
- **R3. Correlacion de semillas de `Xoroshiro128p`**: el constructor fija `s0=seed`, `s1=C^seed` sin calentamiento (`RandomEngine.h:48-51`) y el primer `next_uint64 = s0+s1` (`RandomEngine.h:75`). Como `s + (C^s) = C + 2*(s & ~C)`, cualquier semilla cuyos bits esten contenidos en `C=0x853C49E6748FEA9B` da **exactamente el mismo primer valor `C`**: semillas 1, 2 y 3 dan `next_double = 0.5204511821398793` **y** `roll_in_range(0,999999) = 949339` (calculado en esta sesion con replica del constructor; Codex y Haiku lo confirmaron). Es decir, **tambien `roll_in_range` coincide en el primer sorteo** (corrige la version anterior de este contrato). Tampoco difiere el **segundo** `next_double` (0.370477 para las semillas 1, 2, 3, 4 y 1000, replica en esta sesion); el tercero ya difiere (0.431092, 0.806107, 0.681122, 0.056138...), y el primer `roll_in_range` varia para la semilla 4 (949347) y 1000 (950043). Con 4 sorteos descartados queda fuera de la zona correlada. Por eso: `mezclarSemilla` (splitmix64) **y** descartar los primeros 4 sorteos; test de guarda en E04.
- **R4. Sin snapshot del RNG**: `Xoroshiro128p` tiene `m_seed` const y estado privado (`RandomEngine.h:66-68`): no se puede guardar/restaurar a mitad. La reproduccion es "misma semilla desde el principio" (una partida guardada no reanuda particulas; no se guardan: son efimeras, **inferido** aceptable).
- **R5. `roll_in_range` y aritmetica**: `b-a+1` en `int` (`RandomEngine.h:60`); con rangos pequenos no hay problema; no usar para 64 bits.
- **R6. Reloj de pared y pausa**: el sistema de cartas/golpes ignora la pausa (`PintaDestellos.cpp:88,94-103`); el emisor del polvo sigue a `VidaNivel` (D-O6); la migracion de cartas, si llega, cambia ese comportamiento visible (seccion 9bis). Las particulas de mascotas **si** avanzan con `dt` (`ApplicationBucle.cpp:176`) pero **no** respetan `MOTOR_MOVIMIENTO_REDUCIDO` ni la pausa de golpe (corre fuera de `mundo`, `:141`); el emisor nuevo debe consultar la reduccion (D-O3) y E07 no debe asumir que "ninguna particula la consulta" cuando se compare con las mascotas.
- **R7. Residuo en coma flotante**: 60 x (1/60) x 10 no suma exactamente 10; usar tolerancia `kTolEmision` y test explicito (leccion de `U3-07_tweens.md` sobre `kTolFin`).
- **R8. Catch-up tras un tirón** (ventana minimizada, breakpoint): sin tope saldrian miles de particulas; `kMaxPasoEmision` descarta el exceso.
- **R9. Limpieza por cambio de mapa**: ya no es un hueco: el polvo se limpia en `VidaNivel::monta` (`src/Engine/ApplicationNivel.cpp:359`), por salto de celda no adyacente y por el centinela de cinematica (E07). Para cartas (`EfectosDeCartas`) sigue sin gancho: `olvida()` solo al terminar el combate (`ApplicationHud.cpp:182`); es parte de la migracion futura.
- **R10. Estado compartido en tests**: nada `static` mutable en el emisor; las unicas `static` aceptables son tablas `const` (`FamiliasDeParticulas.cpp:14`, `ParticulasDeCartas.cpp:61`). `static std::minstd_rand temblor(7)` (`ApplicationCinematica.cpp:495`) es el antipatron a no copiar.
- **R11. Coste GL**: `pintaDestellos` hace un `submit` por pixel de forma y de contorno (`PintaDestellos.cpp:44`, `:58`) y `MezclaAditiva` fuerza dos `flush()` por llamada (`:18-26`, `:62`). Como se pinta **una llamada por `GrupoDeDibujo`** (E06/E07), N grupos = N pares de flush, igual que hoy una por `Activa` (`:101-105`). Mitigacion: el polvo agrupa puffs consecutivos con el mismo grupo y esta acotado a 16 emisores.
- **R12. Ficheros calientes**: `CMakeLists.txt` (1639) lo editan varios agentes; 1 linea y `.cmake` propio. `ApplicationPantallas.cpp` a 497 lineas: cualquier adicion lo pasa de 500.
- **R13. Copia muerta** `include/Render/PergaminoGL 2.h`: editar la equivocada compila igual (nadie la incluye) y no hace nada. Editar `PergaminoGL.h`.
- **R14. Datos en python**: `tools/particulas.py` y `tools/test_validar_sprites.py:399` fijan que las dos listas de familias son la misma; si el emisor anadiera familias propias, romperia esa prueba. El emisor **no anade familias**.
- **R16. Escala y posicion por sprite**: `pintaDestellos` aplica `tam/64` a la posicion **y** al tamano del pixel (`PintaDestellos.cpp:32-33,42-44,55-58`) y `Destello` no lleva escala (`BrilloEquipo.h:91-97`): mezclar emisores de grupos distintos en un `out` desplazaria posiciones (ej.: `x=110` con `tam.x=128` sale en 220). Solucion: `GrupoDeDibujo` por emisor (declarado en `Emisor.h`, E05), una llamada por grupo y `mundoDePixel` comun a pintura y test. Tamanos reales variables en cartas: `Entity.cpp:89-92` (`m_visualScale`), `ApplicationHud.cpp:193-209` (relevante para la migracion futura).
- **R17. Emisor no asignable por el RNG**: `Xoroshiro128p` tiene miembro `const` (`RandomEngine.h:68`), asi que se puede construir/mover pero **no asignar**: `std::vector<Emisor>` con `remove_if`+`erase` no compilaria si `Emisor` guardara el RNG por valor. Solucion v1: el `Emisor` guarda `std::unique_ptr<RPG::Xoroshiro128p>` (D-T10), sin tocar `RandomEngine.h`; `static_assert(std::is_move_assignable_v<Emisor>)` en el test.
- **R18. Efecto nuevo sin referencia visual**: el polvo no tiene paridad que comparar; el criterio de aceptacion es D-O5 (observacion de Oriol) y los invariantes de presupuesto/determinismo.
- **R15. Dependencias de paquete** (ficha: U3-07, PREP.E10): no verificadas aqui mas alla de que `Interpolacion.h` existe en la base. Anotadas.

## 9bis. Migracion futura de cartas y golpes (fuera de U4-07)

Diagnostico de paridad (adversarial), para quien abra ese paquete:
- **Poblacion en regimen desde t~0**: el sistema actual esta en estado estacionario desde el primer instante (`cuantas = int(e.particulas*vida*densidad)+1`, `BrilloEquipo.cpp:254`; cada particula con su `desfase` y `edad = fmod(segundos+desfase, vida)`, `:265-267`), por lo que una carta muestra ~14,4*vida*densidad particulas ya a edad variada en t=0; un emisor con tasa constante arranca vacio y llega al regimen despues de una `vida`.
- **Origen**: un pixel aleatorio del cuerpo (13x17 en `ParticulasDeCartas.cpp:92-94`; `pix[mezcla(...) % n]`, `BrilloEquipo.cpp:272`), no un punto; el emisor v1 tiene origen puntual.
- **Estado por indice**: balanceo con el indice `i` y el desfase (`sin(edad*3 + i)`, `BrilloEquipo.cpp:349`; `sin(edad*5 + i)`, `:317`): `Particula.k` (E05) lo cubre, pero hay que sortear el desfase inicial.
- **Envolvente**: fundidos por edad total de la carta y corte a `kDuracionDeCarta` (`ParticulasDeCartas.cpp:78`, `:98-102`): ya soportados por `cortar`/`entrada`/`salida` (E06).
- `DefinicionEmisor` ya tiene 15 campos (limite de 15 miembros): ese paquete debe partirla (p. ej. `EstiloEmisor` aparte) antes de anadir "origen en area" y "pre-calentamiento".
- Reloj: de pared a `dt` pausable (cambio visible de ~70 ms en el critico; antes D-O6).

## 10. Cierre de E01

- Comandos: `grep -rIlniE "particula|particle|chispa|emitter|emisor|polvo|humo|niebla|SpriteBatch|instanc" src include editor examples tests cmake CMakeLists.txt`; `grep -rnE "EfectosDeCartas|ParticulasDeCartas|destellosDeCarta|pintaDestellos|destellosDeBrillo|tipoDeParticula|pixelesDeForma|familiasDeParticulas|contornoDeForma|MezclaAditiva|salpica|lanza"` (consumidores); `grep -rlE "RandomEngine|Xoroshiro"` (50 ficheros); `grep -rnE "\brand\(|srand|mt19937|random_device|uniform_real"`; `wc -l` de cada fichero de integracion; lectura con `sed -n` de cada tramo citado. Sin build ni tests (documental; build de fondo en curso).
- Registro de citas: `out/fabrica/REPLICA-R1/U4-07.E01_citas.log` (primera y ultima linea de cada rango + asercion de contenido).
- Base `628f417`. Dependencias de paquete y de E (PREP.E10) anotadas, no verificadas.
- Pendiente de Oriol: D-O2, D-O3, D-O4, D-O5, D-O6 (D-O1 decidida: B, polvo de pasos). D-O4 no bloquea.
