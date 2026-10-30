# Recompensas y alternativas de escenas — Boundington (AV18-08)

Cierra la pieza «Recompensas de escenas» que `ECONOMIA.md` §7 dejaba pendiente para Z.AI. Las tres aventuras de Boundington (`boundington_primer_dia`, `_segundo_dia`, `_ocaso`) pagan ahora oro y objetos por sus decisiones; las vías excluyentes compensan lo que el jugador pierde al elegir.

## Totales (valor = oro + precio × cantidad; de cada grupo excluyente cuenta la vía más alta)

| Día | Misiones | Presupuesto (`ECONOMIA.md`) | Desvío | Combates con premio narrativo |
|---|---|---|---|---|
| 1 | 259 | 280 | −7,5 % | 90 de 140 |
| 2 | 498 | 480 | +3,8 % | 0 de 400 |
| 3 (Ocaso) | 421 | 400 | +5,2 % | 70 de 300 |

## Grupos excluyentes (resumen)

- **Día 1:** Luisarda 60 · Pulga 10 · fuego (cobrar 10 *o* dos pasteles 28) · Xila sobrio 36 (dos vendas) · Verdaderos 20 · guarida 40 · Venides 40 (informe completo; con rumores, 20) · camorrista 25.
- **Día 2:** daga de Aigren 90 · amuleto del ritual 150 · fiesta (robo 40 / chantaje 30) · Rosita 50 · Gorran (perdonar 20 + aliado en el Ocaso / matar 40) · Zarela 18 · capitán 20 · disfraz 40 (flauta, entrada de ópera o bolas de malabares) · Venides 50.
- **Día 3:** Aigren (escudo) 90 · Xila 36 · Edul y Verina 45 · Rosita 40 · atrapados 40 · Gorran 30 · Sandes 80 (aceptar o entregar a Verina) o 70 si se le vence · huida 60.

Detalle por premio: `tools/recompensas_escenas.json` (61 premios). Beat nuevo: `rec_fuego_regalo`.

## Cómo se comprueba

`python3 tools/auditar_recompensas_escenas.py` (también dentro de `validar_todo.py`) y `python3 tools/test_auditar_recompensas_escenas.py`. Fallan: premio sin registrar, cobro repetible, oro sin decir en la escena, objeto sin precio, día fuera de ±15 %.

## Avisos para otras zonas

1. **Ocaso paga tarde.** Los premios del día 3 llegan cuando las tiendas ya cerraron; valen como oro y objetos de uso, no como compra. Si el equipo quiere que cuenten del todo, habría que dar el Ocaso con tiendas abiertas (zona de diseño, no mía).
2. **Furia de Alaro** (precio 400 en el catálogo) se puede conservar mintiendo a Rosita y venderla; conviene marcarla como no vendible (catálogos, no tocado).
3. `rewardGold` de `quests.json` se queda a 0 a propósito (el motor no lo aplica; evita doble pago el día que lo haga).
4. `recorrer_recompensas` (C++) no se pudo reejecutar: la VM no tiene cmake. Queda para quien integre.
5. La prueba nueva no está registrada en CMake (lo hace el integrador); se ejecuta con python directamente.
