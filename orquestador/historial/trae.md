
---
# Archivado el 14/10/2026 (noche) por Claude (coordinador): fichero completo tal como estaba

# trae — tarea actual
**Asignada:** 14/10 por Claude · **Repo:** `MotorGraphico-main` · **Tipo:** trabajo nuevo → rama propia (regla 2)

## Juicio de clases (b) — arreglar `tools/medir_alcance.py`
Está en «Pendiente» del TABLERO. Hay que hacerlo **antes** de recongelar la línea base (`decisiones/D0007`). Dos reglas nuevas:
1. Un **paraguas** (cabecera que solo incluye otras) está alcanzado si lo está cualquiera de las cabeceras que incluye.
2. `template<> Clase<T>::metodo` cuenta como **uso** de la cabecera que declara `Clase`.

Solo `tools/medir_alcance.py` y sus pruebas (si no hay, créalas junto a él). **No regeneres** `alcance_real.json` ni ningún fichero compartido.

**Hecho cuando:** pruebas de las dos reglas (un caso que antes salía «no alcanzado» y ahora sí, para cada regla); en el informe, cuántos ficheros cambian de estado al ejecutarlo sobre HEAD (antes → después).

---
## SIGUIENTE (solo si Oriol te lo dice al pegar el mensaje) — AV14-20 — Validador de puertas de los niveles
(La que tenías antes, AV13-07, la ruta por casillas, la ha hecho Claude el 14/10: el movimiento en combate la necesitaba ya.)
Plan: `planificacion_alcance_y_vision/` (AV14, carril paralelo).
- **Zona (solo esto):** NUEVOS `tools/validar_puertas.py` y `tools/test_validar_puertas.py`; una llamada en `tools/validar_todo.py`. No toques assets ni el motor.
- **Qué:** recorre `assets/levels/*.json`: cada objeto con `targetLevel` (una puerta) lleva a un nivel que existe; la casilla de llegada (`targetSpawn` o el `playerStart` del destino) se puede pisar (sin colisión en ninguna capa ni objeto que bloquee); y desde el destino hay una puerta de vuelta. Lista lo roto, con el nivel, la casilla y por qué.
- **Hecho cuando:** pruebas con niveles de mentira (una puerta rota, una llegada en pared, un nivel sin vuelta) y, sobre los niveles de verdad, el número de avisos en el INFORME. En `validar_todo` entra como aviso, no como error. Fila en el TABLERO.

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:** `agente/...` · hashes
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
