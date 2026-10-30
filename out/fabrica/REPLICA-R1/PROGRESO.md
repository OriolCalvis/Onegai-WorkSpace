# REPLICA-R1 · Progrés (Unity → MotorGraphico)

Actualitzat per Claude (Brain, Opus) després de cada pas. Última actualització: **30/10 · ronda 1 de les tres E01**.

## Com funciona cada E

1. **Sonnet** (subagent) programa la E al seu worktree, amb prova gl-free i mutació que la fa fallar.
2. **Codex** (revisió adversarial) sobre el hash congelat.
3. **Haiku** (subagent) revisa el codi (estil del repo, límits 500 línies / 15 membres, CMake, `require`).
4. **Opus** consolida: **tancada** o **torna al bucle** (màx. 2 rondes; si no, **bloquejada**).
5. Tancada una E → següent E del paquet; acabat el paquet → següent paquet de la llista.

Base: `claude/unificacion-28-10` @ `9d52ae8`. Sense merge ni push. Cap fitxer del teu checkout.

## Desglossament (files de `replica/features.csv` → paquets del Pla U)

| Fila de features.csv | Prioritat | Paquet(s) | Estat del carril |
|---|---|---|---|
| Triggers entrada/permanència/sortida | must | U3-02 (8 E) | **Carril A — en marxa** |
| Clips amb events de fotograma | must | U3-05 (8 E) | **Carril B — en marxa** |
| Corbes d'interpolació compartides | should | U3-07 (8 E) | **Carril C — en marxa** |
| Màquina d'estats d'animació | must | U3-06 (7 E) | cua (després d'U3-05) |
| Consola i inspector en execució | must | F-04 (8 E) | cua |
| Cinemàtiques amb pistes paral·leles | must | U4-02 (9 E) | cua |
| Prefabs de grups | must | U4-05 (9 E) | cua |
| Càmera virtual | should | U4-01 (7 E) | cua |
| Partícules | should | U4-07 (8 E) | cua |
| Navegació (kChase) | should | U1-03 (15 E) | cua (E14 és decisió teva) |
| Llum ambiental | should | U1-05 (8 E) | cua (E08 necessita el teu visto) |
| Gizmos de depuració | should | U4-06 (6 E) | cua |
| Project Settings / Build / mòduls / editor únic / plantilles | must/should | U2-02, G-08, U2-03, U2-05, U2-04 | cua (toquen arquitectura; amb decisions teves) |
| Play Mode | must | F-01 | **bloquejat**: espera DEC-06 |
| Scripting visual | must | U3-04, E-05, E-06 | cua |
| Audio / IDs estables / finestra projecte | must | U1-01, U1-02 (E05+), E-11 | cua (U1-02.E05 = decisió d'estratègia) |
| skip (Asset Store, 3D, consoles, núvol, C#) | could | — | fora d'abast |

Criteri d'ordre: primer lògica pura gl-free sense decisions teves; les E «Decidir…»/«visto» es paren i te les pregunto.

## E en curs i tancades

| E | Carril | Sonnet | Codex | Haiku | Opus | Hash | Nota |
|---|---|---|---|---|---|---|---|
| U3-02.E01 Auditar capa de regions | A | ✅ | ❌ 3 → corregit | ❌ 1+4 → corregit | ✅ **tancada** | b22c042 | Regions només model/editor; `MapEventRunner` no corre a `juego`. 7 dubtes (D1–D7), D1/D2 bloquegen E06/E08 |
| U3-05.E01 Auditar reproductors | B | ✅ | ❌ 4 → corregit | ❌ 1+3 → corregit | ✅ **tancada** | 4501acc | `AnimatedEntity` sempre en bucle i sense events; no hi ha format de clip. D1 (format) bloqueja E02 |
| U3-07.E01 Inventariar interpolacions | C | ✅ | ❌ 3 → corregit | ❌ 1+3 → corregit | ✅ **tancada** | 3c87672 | 1r consumidor: `Camera::transitionTo` (juego+editors). Corbes: lineal, quad/cubic in/out/inout. 6 dubtes (D1–D6) |
| U3-07.E02 Normalitzar progrés | C | ✅ verificat per Opus | ✅ approve | ✅ 0 bloquejants (2 menors) | ✅ **tancada** | 3f3a3da | `test_interpolacion` verd; mutació (sense branca durada 0) → vermell |
| U3-02.E02 Pertinença d'una casella | A | ✅ verificat per Opus | ✅ approve | ✅ 0 bloquejants (3 menors → a E03) | ✅ **tancada** | 3900b88 | `test_region_triggers` 9 casos verd; mutació (`<=`→`<`) → vermell |
| U3-05.E02 Llegir un clip en JSON | B | ✅ verificat per Opus | ✅ approve | ✅ 0 bloquejants (5 menors → a E03) | ✅ **tancada** | 80335bc | `test_clips_animacion` 11 casos verd; BUILD_TESTING=OFF compila; mutació → vermell |
| U3-07.E03 Easing en dades (7 corbes) | C | ✅ verificat per Opus | ✅ approve | ✅ 0 bloquejants | ✅ **tancada** | 9f4d774 | verd; mutació (cubic-out→in) → 3 fallades |
| U3-02.E03 Entrada i sortida (delta) | A | ✅ verificat per Opus | ✅ approve | ✅ 0 bloquejants (2 menors) | ✅ **tancada** | 3f46b76 | verd; mutació (ignorar capa) → vermell |
| U3-07.E04 Interpolar escalar | C | ✅ verificat per Opus | ⏳ | ⏳ | | 325e643 | verd; mutació (sense límits de p) → vermell |
| U3-05.E03 Indexar events de fotograma | B | ✅ verificat per Opus | ✅ approve | ✅ cap hallazgo | ✅ **tancada** | 197cf8a | verd; mutació (ordenar per id) → 2 fallades; inclou els menors d'E02 |
| U3-02.E04 Permanència sense inundar | A | ✅ verificat per Opus | ❌ Stay en sortir | ❌ prova de no-recuperació absent | 🔁 ronda 1 | ca698a5 | verd; mutació (recuperar dt/N) → vermell; interval per regió amb `StayOverride` |
| U3-05.E04 Avançar cursor de clip | B | ⏳ | | | | | |

## Decisions teves (30/10)

- U3-05 format de clip: durada per fotograma en ms (`duracionMs`).
- U3-02: aparèixer dins d'una regió dispara Entrada; Permanència cada N ms, sense recuperar, la pausa no compta.
- U3-07: corbes Lineal + quad/cubic × entrada/sortida/entrada-sortida.

Obertes (bloquegen la integració, no la part pura): U3-02 D1 (enllaç regió→event), D2 (com arriba `MapDocument` al joc), D3/DEC-10 (ordre); U3-05 D2 (API d'`AnimatedEntity`), D3 (primer actor); U3-07 visto visual d'E08.

## Incidències

- Ronda 1 de les tres E01: Codex i Haiku hi han trobat errors reals (comprovats per Opus al codi): cites falses, inventari incomplet, CMake dins `BUILD_TESTING`, identitat de regió ambigua entre capes, contracte de durada 0 contrari a la càmera.
