# Claude Haiku — tarea actual (15/10)
**Asignada:** 14/10 (noche) por Claude (coordinador) · **Modelo:** Haiku, esfuerzo **medio** (medium) · **Repo:** `MotorGraphico-main`
· Lee antes el PROTOCOLO de `orquestador/README.md`. La tarea anterior (AV14-24 + AV14-20, paquete `claude_puertas02`)
y su informe están en `orquestador/historial/claude_haiku.md`.

**Base:** el árbol de ahora en el Mac (lleva aplicados los once paquetes de `Software/REVISAR.md`). Entrega en paquete
`out/claude_alcance01/` con `aplicar.sh` que compruebe la preimagen. **Antes de escribir en una carpeta de `out/`,
mira que no exista** (la última vez se pisaron ficheros de `out/claude_puertas01/`).

## Juicio de clases (b) — `tools/medir_alcance.py`: sus pruebas y la tercera regla
De «Pendiente» del TABLERO («Juicio de clases, lo que falta»). (Era la tarea de trae, que no se ha lanzado: PO-02.)
Lo que ya hay: el commit `5504b63` (29/09) metió las dos primeras reglas en `tools/medir_alcance.py`
(`es_paraguas`, línea ~194, y las especializaciones `template<>`, líneas ~304–310), **sin pruebas**.
Falta (ver `planificacion/juicio_clases/00-INDEX.md` §6.3 y §3.20):
1. **Pruebas:** NUEVO `tools/test_medir_alcance.py` con un árbol de mentira: un paraguas que antes salía
   «no alcanzado» y ahora sí; una cabecera usada solo por `template<> Clase<T>::metodo` que ahora sale alcanzada.
   Comprueba que fallan si quitas cada regla.
2. **La tercera regla:** las fuentes añadidas con `target_sources(<ejecutable> ...)` (en `cmake/*.cmake`) son del
   ejecutable. Hoy el script solo mira `add_executable`/`add_library` (línea ~272). Con su prueba.
- **Zona (solo esto):** `tools/medir_alcance.py` y NUEVO `tools/test_medir_alcance.py`. **No regeneres**
  `planificacion/alcance_real.json` ni ningún fichero compartido.
- **Hecho cuando:** las pruebas pasan; y en el INFORME, cuántos ficheros cambian de estado al ejecutarlo sobre el
  árbol de ahora (antes → después), escribiendo la salida en `out/claude_alcance01/`, no en `planificacion/`.
  Fila en el TABLERO y fichero de sesión.

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits / paquete:**
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
