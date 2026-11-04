# Formato de los eventos del mundo (C8-03a)

Diseño aprobado por Oriol el 28/10/2026 a las 20:48 (`orquestador/lote_F_evidencia/C8-02.md` y `decisiones/D0021-eventos-del-mundo-y-escenas.md`). Lo comprueba `tools/validar_eventos.py`, que está dentro de `validar_todo`.

> **Estado:** el formato y el validador están hechos, pero **el motor todavía no lee estos ficheros**: eso es C8-03b (evaluador y guardado) y C8-03c (integración en el juego). El primer evento de verdad, el Ocaso, es C8-04.

## La idea

El mundo avanza aunque el jugador no esté. Un **evento** cambia el estado del mundo por la cronología, por una acción real o por otro evento, sin que importe dónde está el jugador. Lo que el jugador ve si está presente es una **escena**: beats y cinemáticas, que pueden depender de `lugar`. Si llega tarde, encuentra el **rastro**. Cargar un mapa muestra la situación; no la crea.

## Fichero

`assets/eventos/<campaña>.json`:

```json
{
  "momentos": {"ocaso_puesta": {"dia": 4, "franja": 2}},
  "banderasDeObservacion": ["bnd3_plaza_vista", "bnd3_arenga"],
  "eventos": [
    {
      "id": "comienza_ocaso",
      "ausencia": "avanza",
      "condiciones": {"notFlags": ["bnd3_campanas"]},
      "disparadores": [
        {"id": "aviso_venides", "senal": "bnd3_avisado"},
        {"id": "aviso_xila", "senal": "bnd3_xila_avisa", "condiciones": {"allFlags": ["bnd1_xila_honor"]}},
        {"id": "puesta_sol", "cruzaMomento": "ocaso_puesta"}
      ],
      "efectos": [{"type": "setFlag", "arg": "bnd3_campanas"}],
      "escenas": [{"cinematica": "base_campanas"}]
    }
  ]
}
```

## Campos de un evento

| Campo | Qué es |
|---|---|
| `id` | Único en toda la campaña: minúsculas ASCII, cifras y `_`. No es una bandera de «visto». |
| `disparadores` | Lista **O**: basta con uno. Cada disparador es exactamente uno de estos: `cruzaMomento` (un momento del calendario), `senal` (una bandera que enciende una acción real, por ejemplo hablar con Venides) o `eventoPrevio` (`"otro"` u `"otro.fase"`). Puede llevar sus propias `condiciones`. El orden de la lista es el desempate si coinciden varios: el evento ocurre **una sola vez**. |
| `condiciones` | **Y** común a todos los disparadores: `allFlags`, `anyFlags`, `notFlags` y `variables`, como en los beats. **Sin `lugar`.** |
| `ausencia` | Qué pasa si el jugador no está: `avanza` (ocurre igual), `espera` (necesita `motivo`: qué intervención real espera, nunca «que cargue un mapa») o `caduca` (necesita `ventana.hasta` y `alCaducar`). |
| `ventana` | `desde` y `hasta`, cada uno un momento; `desde` tiene que ser anterior a `hasta`. |
| `efectos` | Solo estado: `setFlag`, `clearFlag`, `setVariable` y `addVariable`. Nunca sobre `dia` ni `franja`: avisar antes de tiempo no cambia la hora (decisión de Oriol). |
| `alCaducar` | El estado que queda si caduca. Mismos efectos permitidos. Nunca una victoria ni una elección del jugador. |
| `fases` | Opcional: `[{id, disparadores, efectos}]`, por ejemplo un sermón activo → acabado. Otro evento puede esperar a `"sermon.acabado"`. |
| `escenas` / `rastros` | `{"beat": id}` o `{"cinematica": id}` que existan: lo que ve quien está y lo que encuentra quien llega tarde. Un rastro no repite los efectos. |
| `ubicaciones` | Referencias al catálogo de ubicaciones de C3-04c (pendiente; hasta entonces da aviso). |

Un **momento** es `{"dia": n, "franja": 0|1|2}` (mañana, tarde, noche) o el nombre de uno de `momentos`. Un nombre sin resolver es error: no se esconde un número en el código. `banderasDeObservacion` lista las banderas que solo dicen que el jugador vio algo. Esas banderas, igual que `cine_<id>_vista`, no pueden ser causa de un evento.

## Lo que el validador rechaza

- **Errores:** `lugar` o disparadores como entrar o zona; haber visto una escena como causa (también en `notFlags`); efectos de texto, diario, combate, premio u hora; momentos sin resolver o mal formados; ventanas al revés; `caduca` sin `hasta` o sin `alCaducar`; `espera` sin `motivo`; señales que no emite nadie; escenas, rastros, eventos o fases que no existen; ids repetidos; ciclos de `eventoPrevio`.
- **Avisos:** condiciones con banderas que solo se encienden estando en un sitio concreto (posible peaje) o que no enciende nadie; `ubicaciones` mientras no exista el catálogo.

Una `senal` local (hablar con alguien) **no** es peaje: es una acción del jugador.
