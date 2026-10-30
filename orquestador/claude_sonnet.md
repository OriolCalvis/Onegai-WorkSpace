# Claude Sonnet — tarea actual (15/10)
**Asignada:** 14/10 (noche) por Claude (coordinador) · **Modelo:** Sonnet, esfuerzo **medio** (medium)
· **Repo:** `dndWeebCC-master` (solo lectura en el paso 1) · Lee antes el PROTOCOLO de `orquestador/README.md`.
La tarea anterior (AV12-05, paquete `claude_deuda01`) y su informe están en `orquestador/historial/claude_sonnet.md`.

## AV13-09 + AV13-21 — Alcance, área y tipo de daño de cada carta (datos, aguas arriba) — SOLO EL PASO 1
De «Pendiente» del TABLERO. (Era la SIGUIENTE de gemini, que no se ha lanzado: PO-02.)
Plan: `planificacion_alcance_y_vision/PLAN_ALCANCE_Y_VISION.md` §8 y `COMBATE_BG3.md` §5 y §8. El flujo es
canon → cartas → motor: esto va en las cartas (`dndWeebCC-master/`), **no en el motor**.
- **Paso 1 (haz esto y para):** una propuesta de esquema, sin cambiar ningún fichero de `dndWeebCC-master/`:
  - `alcance`: `cuerpo_a_cuerpo` / `alcance:N` / `distancia:N` / `sobre_si`, con las distancias ya decididas
    (D0014 §8: al lado 1, corto 4, medio 8, largo 12 casillas);
  - `area`: `circulo:N` / `cono:N` / `linea:N` / `cuadrado:N` (las formas de `include/Game/PlantillaArea.h`);
  - `tipoDano`: cortante, contundente, perforante, fuego, frio, rayo, acido, veneno, sagrado, profano, psiquico;
  - en los monstruos: `resistencias`, `vulnerabilidades`, `inmunidades`.
  Dónde va cada campo en los ficheros de verdad de las cartas (dilo con fichero y un ejemplo de 3 cartas reales
  ya rellenas a mano en la propuesta), y cuántas cartas jugables hay que rellenar (las 4 clases de CLASES-03,
  el Señor de las armas y las SP; usa lo que ya mide tu `tools/medir_deuda_contenido.py`).
- **Salida:** `sesiones/2026-10-15-claude-sonnet-esquema-cartas.md` con la propuesta, y el resumen en el INFORME.
  Si hay una duda de diseño, a la BANDEJA (regla 7).
- **Hecho cuando:** la propuesta está escrita y aprobada por Oriol o Claude Cowork. **El paso 2** (rellenar y el
  validador que dé 0 cartas jugables sin alcance ni tipo) se asigna después, en otra tarea.
- **No toques** `MotorGraphico/` ni las deidades (PO-45 sigue abierta).

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits / paquete:**
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
