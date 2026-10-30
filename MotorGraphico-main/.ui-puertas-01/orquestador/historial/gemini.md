
---
# Archivado el 14/10/2026 (noche) por Claude (coordinador): fichero completo tal como estaba

# gemini — tarea actual
**Asignada:** 14/10 por Claude · **Repo:** `MotorGraphico-main` · **Tipo:** solo documentación → rama propia (regla 2)

## Juicio de clases (a) — `planificacion/juicio_clases/07-editor.md`
Está en «Pendiente» del TABLERO (zona libre). Lee `planificacion/juicio_clases/00-INDEX.md` (sección 6) y sigue el mismo formato que `01-render.md` … `06-mapa-nivel-proyecto.md`.
Cubre las **37 clases propias del editor + las 12 cabeceras de su capa de estado**, usando los dos mapas ya generados que cita el índice.
Solo lees código; **no modificas nada fuera de `07-editor.md`** (y una línea en `00-INDEX.md` si toca).

**Hecho cuando:** las 49 piezas tienen su juicio con referencias `fichero:línea` comprobables; las decisiones que no te corresponden van como preguntas para Oriol, no resueltas.

---
## SIGUIENTE (solo si Oriol te lo dice al pegar el mensaje) — AV13-09 + AV13-21 — Alcance, área y tipo de daño de cada carta (datos, aguas arriba)
Plan: `planificacion_alcance_y_vision/` (AV13, carril paralelo) y `COMBATE_BG3.md` §5 y §8. El flujo es canon → cartas → motor: esto va en las cartas (`dndWeebCC-master/`), **no en el motor**.
- **Paso 1 (para y entrega):** una propuesta de esquema (campos `alcance`: cuerpo_a_cuerpo / alcance:N / distancia:N / sobre_si; `area`: circulo:N / cono:N / linea:N / cuadrado:N; `tipoDano`: cortante, contundente, perforante, fuego, frio, rayo, acido, veneno, sagrado, profano, psiquico) en `sesiones/` y en el INFORME. Oriol o Claude la aprueban.
- **Paso 2:** rellenarlo en las cartas de las clases jugables (las 4 de CLASES-03, el Señor de las armas y las SP) y las resistencias/vulnerabilidades de los monstruos de Boundington, con un validador que liste lo que falta.
- **Hecho cuando:** el validador da 0 cartas jugables sin alcance ni tipo. No toques `MotorGraphico/`.

---
## DESPUÉS (solo si Oriol te lo dice) — AV13-36 y las deidades (D0015 §10 y §17)

1. **Las 950 cartas de ejemplo (Llerba/Tomba) y las clases escondidas, para monstruos y enemigos.**
   Cada carta útil, al catálogo de monstruos con su rol (bruto, tirador, lanzador, apoyo); las clases
   escondidas, como clases de enemigo. Una lista de lo que se descarta y por qué.
2. **Las deidades de clérigos y paladines son los dioses de Oriol** (Vida, Egos, Envidia, Chronos…: los
   del vault). Las del importador (Espiga Dorada, Caminante Gris, Padre de la Forja, Rey Lobo) se
   sustituyen en los datos de las clases. No se inventa ningún dios.

Zona: `dndWeebCC-master` (datos), NUNCA el motor. Con su fila en el TABLERO y su informe.

## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:** `agente/...` · hashes
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
