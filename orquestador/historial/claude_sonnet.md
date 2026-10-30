
---
# Archivado el 14/10/2026 (noche) por Claude (coordinador): fichero completo tal como estaba

# Claude Sonnet — tarea actual (15/10)
**Asignada:** 14/10 (21:30) por Claude, a petición de Oriol («feines que puguem fer sense abordar el que he de revisar»)
· **Modelo:** Sonnet, esfuerzo **medio** (medium) · **Repo:** `MotorGraphico-main` · Lee antes el PROTOCOLO de `orquestador/README.md`.

**Base:** tus ficheros de ahora en el Mac, **sin** los ocho paquetes de `REVISAR.md` (Oriol los revisa el 15/10).
Tu zona no toca ninguno de los 245 ficheros que llevan esos paquetes (salvo `CMakeLists.txt`, y solo por ancla).
Si ves que necesitas otro, **para** y escríbelo en el INFORME en vez de tocarlo. Entrega como siempre: paquete en `MotorGraphico/out/claude_<nombre>/`
con `aplicar.sh` que compruebe la preimagen, fichero de sesión, fila en el TABLERO. No commitees.

## AV12-05 — La deuda de contenido aguas arriba (medida y propuesta, sin aplicar)
El plan dice: 202 hechizos huérfanos, 432 monstruos con vida 0 y 59 clases sin hechizos. Nadie lo ha medido
desde entonces.
- **Zona:** NUEVOS `tools/medir_deuda_contenido.py` (solo lectura) y su prueba `tools/test_medir_deuda_contenido.py`,
  y el informe `planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md`. **No cambies** `assets/catalogs/`.
- **Qué mide**, sobre `assets/catalogs/` y lo que cargan `CatalogoCartas`/`EnemyCatalog` (léelos para saber qué es
  «huérfano» de verdad):
  1. hechizos y cartas que ninguna clase ni monstruo usa;
  2. monstruos con vida 0 o sin CA o sin ataque, por tier;
  3. clases sin hechizos o con menos cartas que el mínimo de su tier;
  4. referencias rotas (un id que no existe en ningún catálogo).
- **Qué propone**, por cada grupo:
  - lo que se puede rellenar con una fórmula (la vida de un monstruo por tier, AV6-06), con la fórmula y
    10 ejemplos;
  - lo que necesita a una persona (Oriol o Gemini, AV13-21/36).
- **Hecho cuando:** el script da los cuatro números hoy (aunque no coincidan con los del plan: di por qué) y la
  prueba cubre un catálogo de mentira con un caso de cada. Paquete `out/claude_deuda01/`.
- **Bonus si te sobra tiempo:** el script de la fórmula de vida, que escriba un catálogo propuesto en
  `out/claude_deuda01/propuesta/` sin tocar el de verdad.

---
## INFORME (15/10)
- **Estado:** hecho. AV12-05 medida y propuesta; no se ha aplicado nada a los catálogos.
- **Paquete:** `out/claude_deuda01/` (`aplicar.sh`, `nuevos.txt`, `deuda01.tgz`, `propuesta/`). Solo ficheros nuevos; no depende de ningún otro paquete.
- **Los cuatro números de hoy:** hechizos huérfanos **205 de 210** (el plan: 202; con `classTags` también sale 202) · monstruos con vida 0 **0 de 432** (el plan: 432; era anterior a D-03) · clases sin hechizos **125 de 127**, las 70 jugables (el plan: 59 de 61; el catálogo se duplicó) · referencias rotas **187 de 7.994** (primera medida; 169 son de 10 deidades que no existen).
- **Qué ha cambiado:**
  - `tools/medir_deuda_contenido.py` (nuevo): mide con la regla del C++; `--md`, `--json`, `--proponer DIR`.
  - `tools/test_medir_deuda_contenido.py` (nuevo): 42 comprobaciones sobre un catálogo de mentira, un caso de cada medida.
  - `planificacion_alcance_y_vision/DEUDA_DE_CONTENIDO.md` (nuevo): el informe, con qué arregla una fórmula y qué necesita a una persona.
  - `out/claude_deuda01/propuesta/` (bonus): catálogo de monstruos propuesto (solo 2 cifras cambiadas) y 5 CSV.
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
  ```
  cd ~/Software/MotorGraphico-main/MotorGraphico   # o donde esté tu copia
  bash out/claude_deuda01/aplicar.sh               # aplica 2 ficheros y corre la prueba (42/42)
  python3 tools/medir_deuda_contenido.py           # los cuatro números
  python3 tools/medir_deuda_contenido.py --md      # tablas
  ```
- **Qué tengo que mirar yo (Oriol):**
  - Lo más grande: 200 de los 210 hechizos son plegarias de deidad, no cartas de clase. Repartirlos entre las 6 clases del método no encaja con la carta («cuenta para el límite de 3»). Recomiendo la vía de la deidad.
  - Faltan 10 deidades (arreglan 169 referencias y 100 hechizos). Lo escribes tú o Gemini.
  - La vida ya está rellena (por rango y tier, `8+4t` / `16+6t` / `30+10t`, 430 de 432). Lo abierto: `derived.vida` es la mitad de `maxHealth` en las 432; 36 monstruos superan el `healthCap`; los 432 no tienen cartas (`skillIds` vacío); los 900 ataques no tienen cifra de daño.
  - Todas las clases son de tier 1: las 125 cartas de tier 3 a 5 no las tiene nadie.
  - Duda de método: tomé el `handLimit` del tier como «mínimo de cartas». Las 70 jugables lo cumplen (10 de 10); las 57 no jugables no.
  - Las cinco preguntas están al final de `DEUDA_DE_CONTENIDO.md` (§8) y de la sesión, para que Claude Cowork las pase a la BANDEJA. Yo no la he tocado.
- **Fichero de sesión:** `sesiones/2026-10-15-claude-sonnet-deuda.md`

---
# Historial (tareas anteriores)

**Asignada:** 14/10 (12:31) por Claude, a petición de Oriol · **Modelo:** Sonnet, esfuerzo **medio** (medium)
· **Repo:** `MotorGraphico-main`. Lee antes el PROTOCOLO de `orquestador/README.md`.

## EH-14 y EH-15 — Los iconos pequeños del combate
De tu evaluación (`EVALUACION_HEURISTICA.md`) y de tu guía (F2):

- **EH-14 — Huir:** lleva la X de `cerrar` y se lee «cerrar».
  - Ponle un icono propio que diga «huir».
  - Si el atlas `assets/textures/iconos.png` no tiene uno que sirva, deja escrito el encargo para Pixel en
    `orquestador/pixel.md`: un icono de 16 px, «correr / salir por piernas», con el estilo de los demás.
  - Mientras tanto, usa el más claro de los que hay y dilo en el INFORME.
- **EH-15 — La leyenda de los tipos («●ACCIÓN ▲MOVIMIENTO ●CANALIZA ◆REACCIÓN»):**
  - Una forma distinta por tipo, la misma en la carta (arriba a la izquierda) y en la leyenda.
  - «Canaliza» se explica sola o se cambia por una palabra que se entienda.
  - Propón la palabra en el INFORME; si cambia un término del juego, pregúntalo en la BANDEJA.
  - Ojo: la fuente pixel no tiene «◇» (sale «◆» gris en su lugar). Mira qué glifos tiene antes de elegir.

**Zona:**
- `src/HUD/Pergamino/MesaCombateHud.cpp`: `iconoDeCarta`, `tipoDeCarta` y la leyenda de `pintaTurno`.
- `MesaCombateInterno.h`, si hace falta.
- `orquestador/pixel.md`.

Claude (Cowork) no va a tocar esos ficheros mientras dure tu tarea. **No toques** nada más del combate.

- **Entrega:** como hasta ahora, un paquete con preimagen en `MotorGraphico/out/claude_iconos01/` que vaya
  **después** de `claude_dialogos04`, `claude_diario01`, `claude_cicatrices01` y `claude_reputacion01`
  (en ese orden: es el orden de `Software/REVISAR.md`). Para los ficheros que también tocan esos paquetes,
  edita **por ancla** (como `aplica_ediciones.py` de `claude_reputacion01`), nunca el fichero entero.
  Prueba el paquete sobre una copia con los cuatro aplicados antes.
- **Reglas del motor:** ningún fichero de `src/` o `include/` por encima de 500 líneas; ninguna cabecera
  con más de 15 miembros `m_`; un `.cpp` sin GL no va en `motor_core`. `python3 tools/linea_base_core.py`
  debe decir «0 de deuda nueva» (en un árbol con `.git`; fuera de él cuenta mal).
- Tu fila en el TABLERO, tu fichero de sesión y el INFORME. Preguntas para Oriol, a la BANDEJA (regla 7).

**Hecho cuando:**
- hay captura del combate con Huir y la leyenda nuevas;
- los textos pasan tu propia guía (16 px como mínimo, contraste).

---
## INFORME
(rellénalo al terminar)

---
# Historial (tareas anteriores, ya hechas)

# (anterior)
**Asignada:** 14/10 (10:16) por Claude, a petición de Oriol
· **Modelo:** Sonnet, esfuerzo **medio** (medium) · **Repo:** carpeta `Software/` · **Tipo:** documentos, sin
código. Lee antes el PROTOCOLO de `orquestador/README.md`.

## AV15-06 — La guía de estilo de Onegai
Base: `planificacion_alcance_y_vision/USABILIDAD_UOC.md` §2 (los apuntes de usabilidad de Oriol) y lo que
ya hay en el juego.

- **Entrada:** `MotorGraphico/include/HUD/Pergamino/Tema.h` (colores), las letras del tema pergamino, y las
  capturas que ya existen en `MotorGraphico/out/claude_*/*.png` (combate, tienda, título, cuaderno).
- **Salida:** NUEVO `planificacion_alcance_y_vision/GUIA_DE_ESTILO.md`:
  - la paleta, con el nombre de cada color en `Tema.h` y para qué se usa (y para qué no);
  - las letras y sus tamaños;
  - la retícula de 8 px y los márgenes;
  - los botones con sus 5 estados (normal, encima, pulsado, desactivado, foco);
  - los iconos: objetos reconocibles en 5 s, nada abstracto;
  - la voz: cómo habla el juego (castellano, tuteo, sin chistes modernos).
  Cada regla, con la captura donde se cumple o donde no.
- **No toques código.** Si ves algo que hay que cambiar, va en una tabla «Pantallas que no cumplen» al
  final, con la captura y la regla.
- **Hecho cuando:** la guía existe y la tabla de pantallas que no cumplen tiene al menos las pantallas del
  combate, la tienda, el título y el cuaderno. Tu fila en el TABLERO y tu fichero de sesión.

---
## SIGUIENTE (solo si Oriol te lo dice) — AV15-01 — Evaluación heurística
- **Método:** las 10 heurísticas de Nielsen y los principios de Norman (`USABILIDAD_UOC.md` §3).
- **Qué:** recorrido cognitivo de las cuatro tareas del jugador nuevo, tal como están en
  `USABILIDAD_UOC.md`.
- **Salida:** `planificacion_alcance_y_vision/EVALUACION_HEURISTICA.md`, una fila por problema:
  pantalla, captura, principio incumplido, gravedad 0–4 y propuesta. Sin código.
- **Capturas:** si te faltan, pide a Oriol que juegue en Release y las haga.

---
## INFORME
**AV15-06 terminada (14/10).** Guía en `planificacion_alcance_y_vision/GUIA_DE_ESTILO.md`; capturas en `guia_de_estilo_capturas/`; fila en el TABLERO y sesión `sesiones/2026-10-14-claude-sonnet-guia-estilo.md`. No se tocó código.
- Tabla «Pantallas que no cumplen»: combate (F1–F4), tienda (F5–F7), título (F8–F9), cuaderno (F10–F12), negocio (F13) y cinco reglas globales (F14–F18).
- Hallazgos principales: contraste sobre el papel real de las capturas (solo `kTinta`/`kTinta2` pasan 4,5:1 en hoja; por confirmar en Release); iconos repetidos (`dote` en las siete cartas, `cerrar` para Huir, mismo frasco en tres objetos); 38 `rgb(` sueltos; 17 medidas fuera de la retícula; botón principal con dos contornos.
- Sin comprobar (§10): cuaderno post TINTAS-01, botones encima/pulsado/foco, termas, prueba de 5 s.
- Pregunta para Oriol: ¿adoptas las reglas marcadas [propuesto] (mínimo 16 px, estados de botón, regla provisional de tintas sobre la hoja)?

---
## INFORME — AV15-01 (14/10)
**AV15-01 terminada.** Informe en `planificacion_alcance_y_vision/EVALUACION_HEURISTICA.md`; capturas en `evaluacion_heuristica_capturas/`; fila en el TABLERO y sesión `sesiones/2026-10-14-claude-sonnet-evaluacion-heuristica.md`. No se tocó código.
- 35 problemas (8 de gravedad 3, 17 de 2, 10 de 1), uno por fila con captura, principio, gravedad y propuesta; las cuatro tareas, paso a paso con las tres preguntas de Norman.
- Lo más grave: sitios sin nombre y minimapa que no distingue (EH-01, 02), ruido de la calle (EH-06), turno del rival con aspecto de jugable (EH-12), cartas iguales con nombres cortados (EH-13), salir sin confirmar (EH-22), comprar una casa sin resumen ni confirmación y sin poder revender (EH-29, 30).
- Límite: juicio de experto, sin jugadores. Sin medir tiempos, choques ni desvío de la Tarea 1. Faltan capturas (§5 del informe).
- Pregunta para Oriol: ¿el orden del §6 (confirmar lo irreversible, nombres, ruido, turno del rival, tiradas en palabras) es el que quieres para AV15-02 a AV15-05?

---
## INFORME — AV15-07, parte de contraste (14/10)
**Terminada.** `planificacion_alcance_y_vision/CONTRASTE_Y_TINTAS.md`; fila en el TABLERO y sesión `sesiones/2026-10-14-claude-sonnet-contraste-tintas.md`. No se tocó código ni `Tema.h`.
- La hoja de las capturas es el papel real (`hoja_a.png`, `#AE9F7D`), no un efecto del harness. Sobre ella solo `kTinta` (7,07:1) y `kTinta2` (5,25:1) pasan 4,5:1; `kTinta3` 4,05; estados y carmín, de 2,7 a 3,5. Fallan también el carmín sobre el papiro y las pestañas inactivas.
- Propuesta: `kTinta3` a `#3C2D1E`, estados como texto solo sobre chip claro, contorno del foco en `kCarmin2`, pestaña inactiva un 10 % más clara. Alternativa: hoja un 30 % más clara.
- Sin comprobar: verlo jugando con las tintas nuevas; cada texto uno a uno (medido por rol). Quedan sin hacer las otras partes de AV15-07 (líneas de 45–75, trama del suelo, tamaño en Opciones, temblor).
- Pregunta para Oriol: ¿Camino A (tintas, la hoja no se toca) o Camino B (hoja más clara)? Y ¿vale la regla «el color de estado no es texto sobre la hoja»?

---
## INFORME — AV15-08, cicatrices en el cuaderno (14/10)
**Hecho y probado en lo mío; falta un cable en vuestro carril.** Paquete `out/claude_cicatrices01/` (aplicar tras `claude_dialogos04` y `claude_diario01`); fila en el TABLERO y sesión `sesiones/2026-10-14-claude-sonnet-cicatrices-cuaderno.md`.
- La FICHA muestra «Cicatrices: N» si N ≥ 1 (`FuentesCuaderno::cicatrices`, `anotaCicatrices`). 2 tests nuevos; `test_pergamino` y `test_cicatrices` pasan.
- **Para Claude Cowork** (zona Application/InterfazPergamino, no tocada): `EstadoJuego::cicatrices`, pasarlo como 6.º campo en `InterfazPergamino.cpp` ~258 y ~306, y rellenarlo en `ApplicationHud.cpp` con `Game::Cicatrices::carga(ruta).de(quien)` (mismo `quien` y ruta que `ponCicatriz`). Código exacto en `LEEME_CLAUDE.md` del paquete.
- Sin comprobar: `linea_base_core.py`, `ctest -L gl-free` completo, captura.
