# La Loba Herida — Esqueleto (§18a) y validación (§18c)

Aventura de ejemplo del sistema de mazos por actos (GDD §20), construida con los prompts
`Plantilla_Prompt_Contenido.md §18` sobre la trama existente `la_loba_herida`
(facción `manada_del_rei_llop`, los bosques del norte de Llerba). Tier 1-2.
Arquitectura: espina de pescado. Tamaño: 10 / 8 / 6.

---

## Hilos argumentales (mini-árboles)

| Hilo | Acto I | Acto II | Acto III |
|---|---|---|---|
| **La Loba** (principal) | A1-01 | B2-03 · B2-08 | C3-02 |
| **El Collar Real** | A1-04 | B2-06 | C3-04 |
| **La Puerta Norte** (Bruna) | A1-02 | B2-02 | — (balance) |
| **El Mercader** (Vidal) | A1-06 | B2-05 | — (balance) |
| **El Chamán** | A1-08 | B2-07 | C3-03 |
| **El Frío** | A1-07 · A1-09 | B2-04 | — (balance) |

## Matriz de Cruce

| Código | Título | Verde activa… | Roja activa… |
|---|---|---|---|
| A1-01 | La loba junto al molino | B2-03 entra (el rastro) | rama de B2-08 (la manada os caza) + final_caceria en C3-06 |
| A1-02 | La Puerta Norte | B2-02 entra (el atajo) | B2-02 descartada (sin escolta ni atajo) |
| A1-03 | La taberna del Cuerno | rama de C3-01 (conocéis la guarida) | rama de B2-01 (entráis a ciegas: emboscada completa) |
| A1-04 | El sello del collar | B2-06 entra (el grabador) | B2-06 descartada (el sello muere sin leerse) |
| A1-05 | Los tramperos | rama de C3-05 (el lobo joven os reconoce) | — (callejón sin salida asumido, 1 permitido) |
| A1-06 | El trato de Vidal | B2-05 entra (la caravana) | B2-05 descartada (Vidal no cruza el bosque) |
| A1-07 | La vieja senda | rama de B2-04 (refugio conocido) | rama de B2-04 (la tormenta os pilla al raso) |
| A1-08 | Aullidos en la muralla | rama de B2-07 (podéis negociar el ritual) | B2-07 sin opción de negociar |
| A1-09 | Provisiones de escarcha | rama de B2-04 (abrigo suficiente) | rama de B2-04 (Fatiga bajo la tormenta) |
| A1-10 | La oferta del alfa | rama de C3-05 (el alfa parlamenta primero) | rama de C3-05 (el alfa ataca sin cuartel) |
| B2-03 | El rastro de sangre | C3-02 entra (la loba os guía) | C3-02 descartada |
| B2-06 | El grabador de sellos | C3-04 entra (el sello habla) | C3-04 descartada |
| B2-07 | El círculo de huesos | rama de C3-03 (el chamán se aparta) | rama de C3-03 (el oso de tumba despierta) |

## Reparto por acto

**Acto I (10)** — todas Base (el Acto I nunca se filtra): A1-01 🔗 1/2 y A1-02 🔗 2/2
(mazo de aprendizaje, GDD §20.5) + A1-03…A1-10 barajadas.

**Acto II (8)** — Base: B2-01, B2-04 (inyectada), B2-07, B2-08 (inyectada) → 4/8 = 50% ✓.
Condicionales (1 requisito c/u): B2-02 (A1-02=Verde), B2-03 (A1-01=Verde),
B2-05 (A1-06=Verde), B2-06 (A1-04=Verde).

**Acto III (6)** — Base: C3-01 🔗 1/1 (siempre primera), C3-03 (inyectada), C3-05 (jefe),
C3-06 (balance_final) → 4/6 = 67% ✓. Condicionales: C3-02 (B2-03=Verde), C3-04 (B2-06=Verde).

---

## Informe de validación (§18c)

1. **FILTRO** — Ningún requisito apunta hacia delante ni al propio acto. Acto II solo usa A1;
   Acto III solo B2 (y ramas internas consultan A1, permitido: las ramas no filtran). Máx. 1
   requisito por carta. ✓
2. **MARIPOSA** — Las 13 cartas ignorables con efecto tienen fila en la matriz con consecuencia
   Verde y Roja reales. Excepción declarada: A1-05 Roja es callejón sin salida (1 permitido,
   patrón A1-10 del GDD §20.6). ✓
3. **VÁLVULA** — Base por acto: I 100% · II 50% · III 67% (≥40% ✓). Peor caso (todo Rojas):
   Acto II queda con B2-01, B2-04, B2-07, B2-08 = 4 cartas ✓; Acto III queda con C3-01, C3-03,
   C3-05, C3-06 = 4 cartas ✓. Mejor caso (todo Verdes): 8 y 6, mazos completos. ✓
4. **CADENA** — 🔗 A1-01/A1-02 numeradas 1/2-2/2; C3-01 1/1. Ninguna 🔗 lleva activation. ✓
5. **BALANCE FINAL** — Exactamente 1 (C3-06); 5 filas contables en mesa que cubren los 6 hilos
   (Loba, Collar, Bruna, Vidal, Chamán vía recuento global, Frío vía recuento del Acto II). ✓
6. **REFERENCIAS** — Ids verificados contra el catálogo: enemigos (lobo_famelico[,_veterano],
   rastreador_de_nieve, chaman_de_la_manada, carronero_del_deshielo, guardian_de_la_loba[,_alfa],
   oso_de_tumba), npcs (npc_capitana_bruna, npc_vidal_el_callado), loot
   (loot_manada_del_rei_llop_t1/t2/t3), historias (hist_la_loba_herida_* pasos 1-5). ✓
   Nota: el jefe es un elite t3 contra un grupo t1-2 — intencionado (clímax); las ramas de
   C3-02/C3-05 lo compensan si se jugó bien.
7. **AUTOCONTENCIÓN** — Cada scene plantea y cierra su mini-conflicto; ninguna exige haber
   leído otra carta (las dependencias van por fichas, no por texto). ✓

**Veredicto: APTA.**
