#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Genera el plan, el Gantt y la pagina a partir de dades.py y workflows_juego/.

Salida (no se edita a mano, se regenera):
  PLAN_ALCANCE_Y_VISION.md      el plan legible, con los flujos y las decisiones
  gantt_alcance_y_vision.puml   el Gantt PlantUML (1 dia = 1 jornada de ~6 h)
  render/plan_alcance_y_vision.html  la pagina autocontenida para navegarlo

Hereda las convenciones de planificacion/pla/build.py del motor: comprobaciones
antes de generar (nada al azar), orden topologico estable, alias cortos en el
Gantt porque PlantUML no resuelve comas dentro de un enlace, y puertas de fase
solo donde son politica y no tecnica.

Las decisiones NO viven aqui: se extraen de workflows_juego/wf-*.md, que es
donde Oriol las lee. Si el recuento declarado en dades.py y el real divergen,
esto falla: es la unica forma de que la bandeja no mienta.
"""
import html
import math
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dades import PHASES, TASKS, WORKFLOWS, SECUNDARIOS, RESOLUCIONES  # noqa: E402
from pathlib import Path  # noqa: E402

AQUI = Path(__file__).resolve().parent
BY_ID = {t["id"]: t for t in TASKS}
PHASE_ORDER = [c for c, _t, _d, _o in PHASES]
PHASE_TITLE = {c: t for c, t, _d, _o in PHASES}
PHASE_DOC = {c: d for c, _t, d, _o in PHASES}
PHASE_ORIGEN = {c: o for c, _t, _d, o in PHASES}
RITME = 1.5  # jornades d'esforç per setmana natural (mitjana real del projecte)
INICI = "2026-09-29"
COLORES = ["#C0503C", "#E3A82B", "#5BA85B", "#4E86B4", "#8A6BA8", "#B4557E",
           "#3F9C8F", "#7A8B2E", "#A9673F", "#4A6FA5", "#9B5DE5", "#D08C34",
           "#6B7B8C"]


def setmanes(j):
    return round(j / RITME)


# --------------------------------------------------------------- comprovacions
def validar():
    errors, avisos = [], []
    if len(BY_ID) != len(TASKS):
        errors.append("hay IDs de tarea duplicados")
    for t in TASKS:
        if t["fase"] not in PHASE_TITLE:
            errors.append(f'{t["id"]}: fase desconocida {t["fase"]}')
        if t["j"] <= 0:
            errors.append(f'{t["id"]}: esfuerzo no positivo')
        if t.get("carril", "principal") not in ("principal", "paralelo"):
            errors.append(f'{t["id"]}: carril desconocido {t["carril"]}')
        if t.get("carril") == "paralelo" and not str(t.get("zona", "")).strip():
            errors.append(f'{t["id"]}: una tarea en paralelo necesita su zona')
        for campo in ("titulo", "doc", "existe", "cierre"):
            if not str(t.get(campo, "")).strip():
                errors.append(f'{t["id"]}: falta {campo}')
        for d in t["deps"]:
            if d not in BY_ID:
                errors.append(f'{t["id"]}: depende de {d}, que no existe')
            elif PHASE_ORDER.index(BY_ID[d]["fase"]) > PHASE_ORDER.index(t["fase"]):
                avisos.append(f'{t["id"]} ({t["fase"]}) depende de {d} ({BY_ID[d]["fase"]}): cruce de fase hacia delante')
    flujos = WORKFLOWS + SECUNDARIOS
    ids_flujo = [f["id"] for f in flujos]
    if len(set(ids_flujo)) != len(ids_flujo):
        errors.append("hay IDs de flujo duplicados")
    for f in flujos:
        if not (AQUI / f["fichero"]).exists():
            errors.append(f'{f["id"]}: falta el fichero {f["fichero"]}')
        for tid in f["tareas"]:
            if tid not in BY_ID:
                errors.append(f'{f["id"]}: cita la tarea {tid}, que no existe')
    decisiones = leer_decisiones()
    for w in WORKFLOWS:
        reales = len([d for d in decisiones if d["archivo"] == Path(w["fichero"]).name])
        if reales != w["decisiones"]:
            errors.append(f'{w["id"]}: dades.py declara {w["decisiones"]} decisiones y el documento tiene {reales}')
    ids = {d["id"] for d in decisiones}
    for r in RESOLUCIONES:
        if r[0] not in ids:
            errors.append(f'RESOLUCIONES cita {r[0]}, que no existe en los flujos')
        if r[1] not in ("cerrada", "asumida", "abierta"):
            errors.append(f'{r[0]}: estado desconocido {r[1]}')
        for tid in r[3]:
            if tid not in BY_ID:
                errors.append(f'{r[0]}: cita la tarea {tid}, que no existe')
    for d in ids - set(RES):
        errors.append(f'{d}: respondida o no, no tiene RESOLUCION en dades.py')
    for i, r in RES.items():
        tiene = i in RESPUESTAS
        if r["estado"] == "abierta" and tiene:
            errors.append(f'{i}: marcada como abierta pero Oriol respondio ({RESPUESTAS[i][1]})')
        if r["estado"] in ("cerrada", "asumida") and not tiene:
            errors.append(f'{i}: marcada como {r["estado"]} pero no hay respuesta literal en los documentos')
    return errors, avisos, decisiones


# --------------------------------------------------------------- decisiones
DEC_RE = re.compile(r"^\|\s*(D-[A-Z]+-[0-9]{2})\s*\|")


def leer_decisiones():
    """Extrae la bandeja de decisiones de los wf-*.md (apartado 12)."""
    out = []
    for ruta in sorted((AQUI / "workflows_juego").glob("wf-*.md")):
        for linea in ruta.read_text(encoding="utf-8").splitlines():
            m = DEC_RE.match(linea)
            if not m:
                continue
            celdas = [c.strip() for c in linea.strip().strip("|").split("|")]
            if len(celdas) < 5:
                continue
            out.append(dict(id=celdas[0], pregunta=celdas[1], opciones=celdas[2],
                            recomendacion=celdas[3], bloquea=celdas[4],
                            archivo=ruta.name))
    return out


# ------------------------------------------------- respuestas literales de Oriol
DISCREP = AQUI / "workflows_juego" / "DISCREPANCIAS_Y_DECISIONES.md"


def leer_respuestas():
    """Respuesta literal de Oriol por decision, con su fuente.

    Oriol respondio el 28/09 en tres sitios distintos y los tres cuentan:
      * la columna final (sin nombre) de la tabla del apartado 12 de cada wf-*.md;
      * la columna «USUARIO DICE» de las secciones A y B del registro;
      * (la columna «Solucion» de la seccion C va aparte, ver leer_deudas).
    El texto es SUYO y se reproduce literal: aqui no se reescribe nada.
    """
    out = {}
    for ruta in sorted((AQUI / "workflows_juego").glob("wf-*.md")):
        for linea in ruta.read_text(encoding="utf-8").splitlines():
            m = DEC_RE.match(linea)
            if not m:
                continue
            c = [x.strip() for x in linea.strip().strip("|").split("|")]
            if len(c) > 5 and c[5] and c[5] != "-":
                out[m.group(1)] = (c[5], f"{ruta.name} §12")
    for linea in DISCREP.read_text(encoding="utf-8").splitlines():
        if not re.match(r"^\|\s*[AB]\d+\s*\|", linea):
            continue
        c = [x.strip() for x in linea.strip().strip("|").split("|")]
        dicha = c[-1]
        ids = sorted({i for cell in c[:-1] for i in re.findall(r"D-[A-Z]+-\d{2}", cell)})
        if dicha and dicha != "-" and ids:
            for i in ids:
                out.setdefault(i, (dicha, f"DISCREPANCIAS {c[0]}"))
    return out


def leer_deudas():
    """Seccion C del registro: deuda de contenido y la solucion que dio Oriol."""
    filas, en_c = [], False
    for linea in DISCREP.read_text(encoding="utf-8").splitlines():
        if linea.startswith("## C."):
            en_c = True
            continue
        if en_c and linea.startswith("## "):
            break
        if en_c and linea.startswith("|") and not re.match(r"^\|\s*-+", linea):
            c = [x.strip() for x in linea.strip().strip("|").split("|")]
            if len(c) >= 4 and c[0] not in ("Deuda",):
                filas.append(c[:4])
    return filas


RESPUESTAS = leer_respuestas()
DEUDAS = leer_deudas()
RES = {r[0]: dict(estado=r[1], consecuencia=r[2], tareas=r[3]) for r in RESOLUCIONES}


# --------------------------------------------------------------- orden y esfuerzo
def ordenar():
    order, pending = [], [t["id"] for t in TASKS]
    while pending:
        ready = sorted(i for i in pending if all(d in order for d in BY_ID[i]["deps"]))
        if not ready:
            raise SystemExit("ciclo de dependencias: " + ", ".join(pending))
        order += ready
        pending = [i for i in pending if i not in order]
    fin = {}
    for pid in order:
        fin[pid] = max((fin[d] for d in BY_ID[pid]["deps"]), default=0) + BY_ID[pid]["j"]
    return order, fin


ORDER, FIN = ordenar()
INICI = {}
for _pid in ORDER:
    INICI[_pid] = FIN[_pid] - BY_ID[_pid]["j"]
SPAN = max(FIN.values())  # camí crític en jornades
TOTAL = sum(t["j"] for t in TASKS)
PER_PHASE = {c: sum(t["j"] for t in TASKS if t["fase"] == c) for c in PHASE_ORDER}
N_PER_PHASE = {c: len([t for t in TASKS if t["fase"] == c]) for c in PHASE_ORDER}
TERMINALS = {c: [p for p in ORDER if BY_ID[p]["fase"] == c
                 and not any(p in t["deps"] for t in TASKS if t["fase"] == c)]
             for c in PHASE_ORDER}
FLUJO_DE = {}
for f in WORKFLOWS + SECUNDARIOS:
    for tid in f["tareas"]:
        FLUJO_DE.setdefault(tid, []).append(f["id"])
TRANSVERSALES = [p for p in ORDER if p not in FLUJO_DE]


# --------------------------------------------------------------- markdown
def plano_texto(decisiones, avisos):
    L = []
    A = L.append
    A("# Plan derivado del Documento de Alcance y Vision\n")
    A("Generado por `planificacion_alcance_y_vision/build.py` desde `dades.py` y "
      "`workflows_juego/`. **No se edita a mano**: se regenera, y asi el plan, el "
      "Gantt y la pagina no pueden divergir.\n")
    A(f"Fuente de diseno: `DOCUMENTO_DE_ALCANCE_Y_VISION.md` (28/09/2026). "
      f"Autoridad: `canon (Onegai 2) -> cartas (dndWeebCC) -> motor` (BITACORA 2.1); "
      "donde la vision contradice al reglamento ya implementado, se corrige la "
      "vision (ver `workflows_juego/DISCREPANCIAS_Y_DECISIONES.md`).\n")

    A("## 1. El numero\n")
    A("| Fase | Titulo | Tareas | Jornadas | Semanas a ritmo real | Origen |")
    A("| --- | --- | ---: | ---: | ---: | --- |")
    for c in PHASE_ORDER:
        A(f"| {c} | {PHASE_TITLE[c]} | {N_PER_PHASE[c]} | {PER_PHASE[c]:g} | "
          f"{setmanes(PER_PHASE[c])} | {PHASE_ORIGEN[c]} |")
    A(f"| **Total** | | **{len(TASKS)}** | **{TOTAL:g}** | **{setmanes(TOTAL)}** | |")
    A("")
    A(f"Una jornada (j) son ~6 h de trabajo enfocado. A ritmo real de ratos libres "
      f"(~{RITME} j por semana), las {TOTAL:g} jornadas son unas {setmanes(TOTAL)} "
      "semanas naturales. **Es una estimacion de alcance completo, no de trabajo "
      "restante ni una fecha de entrega**: las fechas del Gantt son una escala de "
      "esfuerzo con dependencias.\n")

    A("## 2. Los flujos de diseno (la capa que faltaba)\n")
    A("Cada flujo dice que tiene que pasar, con que numeros, quien se entera, que "
      "existe YA en el motor (con fichero y linea) y que decision de Oriol lo "
      "bloquea. Plantilla de 13 apartados en `workflows_juego/INDICE.md`.\n")
    A("| WF | Flujo | Documento | Base real en el motor | Tareas | Decisiones |")
    A("| --- | --- | --- | --- | ---: | ---: |")
    for w in WORKFLOWS:
        A(f'| {w["id"]} | {w["titulo"]} | {w["doc"]} | {w["base"]} | '
          f'{len(w["tareas"])} | {w["decisiones"]} |')
    A("")
    A("Flujos secundarios (ficha corta en `CATALOGO_FLUJOS_SECUNDARIOS.md`):\n")
    A("| FS | Flujo | Documento | Prioridad del primer corte | Tareas |")
    A("| --- | --- | --- | --- | ---: |")
    for f in SECUNDARIOS:
        A(f'| {f["id"]} | {f["titulo"]} | {f["doc"]} | {f["prioridad"]} | {len(f["tareas"])} |')
    A("")

    A("## 3. Que significa «hecho» en cada fase\n")
    for c in PHASE_ORDER:
        A(f"### {c} — {PHASE_TITLE[c]}\n")
        A(f"Documento: {PHASE_DOC[c]} · Origen: {PHASE_ORIGEN[c]} · "
          f"{N_PER_PHASE[c]} tareas · {PER_PHASE[c]:g} j\n")
        for pid in ORDER:
            t = BY_ID[pid]
            if t["fase"] != c:
                continue
            dep = ", ".join(t["deps"]) if t["deps"] else "ninguna"
            flujo = ", ".join(FLUJO_DE.get(pid, [])) or "transversal"
            A(f"**{pid} — {t['titulo']}**  ")
            A(f"`{t['j']:g} j` · depende de: {dep} · flujo: {flujo}  ")
            A(f"Documento: {t['doc']}  ")
            A(f"Ya existe: {t['existe']}  ")
            if t.get("carril"):
                A(f"Carril: **{t['carril']}** ({t.get('quien', '?')}) · zona: {t.get('zona', '')}  ")
            A(f"Cierre: {t['cierre']}\n")

    A("## 4. Cruces de fase declarados\n")
    A("El orden de presentacion es el del documento; el orden real lo da el grafo. "
      "Estas dependencias apuntan a una fase posterior a proposito:\n")
    for a in avisos:
        A(f"- {a}")
    A("")

    A("## 5. Tareas transversales (no pertenecen a un flujo del jugador)\n")
    A("Contratos, editor, red y aceptacion: habilitan flujos pero no son un flujo. "
      f"Son {len(TRANSVERSALES)} de {len(TASKS)}.\n")
    for pid in TRANSVERSALES:
        A(f'- {pid} — {BY_ID[pid]["titulo"]}')
    A("")

    A("## 6. Orden de ejecucion\n")
    A("Orden topologico estable (por ID dentro de cada nivel de dependencias). "
      "Una tarea no se empieza con dependencias abiertas.\n")
    A("```")
    for i in range(0, len(ORDER), 6):
        A("  ".join(f"{p:<8}" for p in ORDER[i:i + 6]))
    A("```\n")

    A("## 7. Estado de las decisiones (Oriol respondio el 28/09)\n")
    cerr = len([d for d in decisiones if RES[d["id"]]["estado"] == "cerrada"])
    asu = len([d for d in decisiones if RES[d["id"]]["estado"] == "asumida"])
    abie = len([d for d in decisiones if RES[d["id"]]["estado"] == "abierta"])
    A(f"De las {len(decisiones)} decisiones: **{cerr} cerradas**, **{asu} asumidas** "
      "(Oriol dejo la pregunta en blanco y se aplica la recomendacion diciendolo en voz "
      f"alta) y **{abie} abiertas**. La respuesta literal, la consecuencia y las tareas "
      "que la recogen estan en `DECISIONES_RESUELTAS.md` (generado).\n")
    A("| ID | Estado | Consecuencia en el plan | Tareas |")
    A("| --- | --- | --- | --- |")
    for d in decisiones:
        r = RES[d["id"]]
        A(f'| {d["id"]} | {r["estado"]} | {r["consecuencia"]} | '
          f'{" ".join(r["tareas"])} |')
    A("")

    A("## 8. Dos carriles: lo de Claude y lo que pueden coger las otras IAs\n")
    A("Desde el 14/10 (D0014) las tareas nuevas dicen su carril. **Principal**: toca el "
      "juego (Application, GameSession, el HUD de combate, las aventuras) y lo lleva Claude. "
      "**Paralelo**: ficheros NUEVOS con su linea de CMake, o datos aguas arriba; lo puede "
      "coger otra IA sin pisar lo principal. Quien la tiene esta en `orquestador/<agente>.md` "
      "y se reclama con su fila en el TABLERO, con la zona de esta tabla.\n")
    A("| Tarea | Carril | Quien | Jornadas | Depende de | Zona |")
    A("| --- | --- | --- | ---: | --- | --- |")
    for pid in ORDER:
        t = BY_ID[pid]
        if not t.get("carril"):
            continue
        A(f'| {pid} — {t["titulo"]} | {t["carril"]} | {t.get("quien", "")} | {t["j"]:g} | '
          f'{", ".join(t["deps"]) or "ninguna"} | {t.get("zona", "")} |')
    A("")

    A("## 9. Reglas de trabajo\n")
    A("- **Ninguna tarea se empieza con sus dependencias abiertas**; el orden de la "
      "seccion 6 ya las respeta.")
    A("- **Ningun flujo se implementa sin su decision resuelta** cuando la decision "
      "bloquea esa tarea (columna «Bloquea» de la seccion 7).")
    A("- **Una tarea se cierra cuando llega al jugador**, no cuando compila: "
      "`tools/medir_alcance.py` por uso desde cada `main()` (AV12-01).")
    A("- **No se planifica dos veces lo mismo**: la columna «Ya existe» de cada tarea "
      "y el apartado 11 de cada flujo son la defensa contra reescribir el motor.")
    A("- **Las discrepancias se registran, no se resuelven en silencio** (BITACORA, "
      "regla 3): `workflows_juego/DISCREPANCIAS_Y_DECISIONES.md`.")
    A("- **Si una tarea supera el doble de su estimacion, se para y se replantea**, "
      "en vez de anadirle jornadas.")
    A("- **El flujo es en una sola direccion**: `canon -> cartas -> motor`. La deuda "
      "de contenido (202 hechizos huerfanos, 432 monstruos con vida 0, 59 clases "
      "sin hechizos) se reclama aguas arriba, no se inventa en el motor (AV12-05).")
    A("")
    return "\n".join(L)


# ------------------------------------------------- decisiones resueltas
def decisiones_md(decisiones):
    cerr = [d for d in decisiones if RES[d["id"]]["estado"] == "cerrada"]
    asu = [d for d in decisiones if RES[d["id"]]["estado"] == "asumida"]
    abie = [d for d in decisiones if RES[d["id"]]["estado"] == "abierta"]
    L = []
    A = L.append
    A("# Decisiones resueltas — respuestas de Oriol del 28/09/2026\n")
    A("Generado por `build.py`. **La respuesta literal es de Oriol** y se extrae de donde "
      "él la escribió (la columna final del apartado 12 de cada `wf-*.md` y la columna "
      "«USUARIO DICE» de `DISCREPANCIAS_Y_DECISIONES.md`); aquí no se reescribe. La columna "
      "«Consecuencia en el plan» es el análisis de qwen: qué cambia y qué tareas lo recogen.\n")
    A(f"**{len(cerr)} cerradas** (Oriol respondió por escrito) · **{len(asu)} asumidas** "
      "(dejó la pregunta en blanco y se aplica la recomendación diciéndolo en voz alta) · "
      f"**{len(abie)} abiertas** (sin respuesta: se planifica con la recomendación y siguen "
      "en su bandeja).\n")
    if asu or abie:
        A("## Lo que necesita confirmación o respuesta\n")
        A("| ID | Estado | Qué se ha asumido | Por qué |")
        A("| --- | --- | --- | --- |")
        for d in asu + abie:
            r = RES[d["id"]]
            lit = RESPUESTAS.get(d["id"], ("", ""))[1]
            if r["estado"] == "asumida":
                por = (f'Oriol respondi\u00f3 en {lit}, pero dej\u00f3 en blanco la aclaraci\u00f3n posterior '
                       "(casos l\u00edmite): se aplica la recomendaci\u00f3n y hay que confirmarla")
            else:
                por = "Sin respuesta en ninguna de las tres fuentes"
            A(f'| {d["id"]} | **{r["estado"]}** | {r["consecuencia"]} | {por} |')
        A("")
    A("## Las 56 decisiones, con su respuesta literal y su consecuencia\n")
    for w in WORKFLOWS:
        del_flujo = [d for d in decisiones if d["archivo"] == Path(w["fichero"]).name]
        A(f'### {w["id"]} — {w["titulo"]}\n')
        A("| ID | Estado | Respuesta literal de Oriol | Consecuencia en el plan | Tareas |")
        A("| --- | --- | --- | --- | --- |")
        for d in del_flujo:
            r = RES[d["id"]]
            lit, fuente = RESPUESTAS.get(d["id"], ("—(sin respuesta)—", ""))
            A(f'| {d["id"]} | {r["estado"]} | {lit} <br><sub>{fuente}</sub> | '
              f'{r["consecuencia"]} | {" ".join("`" + x + "`" for x in r["tareas"])} |')
        A("")
    if DEUDAS:
        A("## Deuda de contenido aguas arriba: la solución que dio Oriol\n")
        A("| Deuda | Cifra | Efecto en los flujos | Solución de Oriol |")
        A("| --- | --- | --- | --- |")
        for c in DEUDAS:
            A("| " + " | ".join(c) + " |")
        A("")
        A("Las seis clases canónicas que Oriol pegó en la sección C del registro "
          "(Monje del Despertar, Señor de las Armas, Bardo de las 4 estaciones, Apóstata, "
          "Furia de los Creadores y Mago chasqueador) son la referencia de contenido para "
          "dar valores a los hechizos huérfanos y adaptar las clases sin hechizos. Van por "
          "`canon → cartas → motor`, no se inventan en el motor (AV12-05).\n")
    A("## Registro formal\n")
    A("Estas respuestas se registran como `decisiones/D0008-respuestas-workflows-diseno.md` "
      "(estado `accepted`: las decidió Oriol). Las tres abiertas y las dos asumidas siguen "
      "en la bandeja «Esperando a Oriol» del `TABLERO.md`.\n")
    return "\n".join(L)


# --------------------------------------------------------------- gantt
def puml(avisos):
    alias = {pid: pid.replace("-", "") for pid in BY_ID}
    L = ["@startgantt", "",
         "title Onegai — Plan derivado del Documento de Alcance y Vision", "",
         "caption 1 dia = 1 JORNADA de esfuerzo (~6 h), no un dia natural. "
         f"Total {TOTAL:g} jornadas = {setmanes(TOTAL)} semanas a ~{RITME} j/semana. "
         "Las fechas son una escala de esfuerzo con dependencias, NO una prevision de entrega.", "",
         "' Generado por planificacion_alcance_y_vision/build.py desde dades.py.",
         "' No se edita a mano.",
         f"' {len(TASKS)} tareas en {len(PHASES)} fases; {len(WORKFLOWS)} flujos de diseno "
         f"y {len(SECUNDARIOS)} secundarios en workflows_juego/.", ""]
    for a in avisos:
        L.append(f"' cruce declarado: {a}")
    L += ["", f"Project starts {INICI}", "saturday are closed", "sunday are closed", ""]
    for i, c in enumerate(PHASE_ORDER):
        L.append(f"-- {c} — {PHASE_TITLE[c]} ({PER_PHASE[c]:g} j) --")
        L.append("")
        for pid in ORDER:
            t = BY_ID[pid]
            if t["fase"] != c:
                continue
            dias = max(1, math.ceil(t["j"]))
            flujo = ",".join(FLUJO_DE.get(pid, [])) or "transversal"
            L.append(f'[{t["id"]} ({t["j"]:g} j) {t["titulo"]}] as [{alias[pid]}] lasts {dias} days')
            color = COLORES[i % len(COLORES)]
            if t.get("carril") == "paralelo":
                color = f"#FFFFFF/{color}"
            L.append(f"[{alias[pid]}] is colored in {color}")
            L.append(f"' {t['id']} flujo: {flujo} | doc: {t['doc']} | existe: {t['existe']}"
                     + (f" | carril: {t['carril']} ({t.get('quien', '')})" if t.get("carril") else ""))
        L.append("")
    L.append("' --- dependencias (en orden topologico: PlantUML coloca cada tarea")
    L.append("' cuando lee su enlace y no reajusta las que ya ha situado) ---")
    for pid in ORDER:
        for d in sorted(BY_ID[pid]["deps"], key=lambda x: FIN[x]):
            L.append(f"[{alias[d]}] -> [{alias[pid]}]")
    L.append("")
    L.append("' --- hitos de fase ---")
    for c in PHASE_ORDER:
        if not TERMINALS[c]:
            continue
        ultimo = max(TERMINALS[c], key=lambda p: FIN[p])
        L.append(f"[{c} cerrada] happens at [{alias[ultimo]}]'s end")
    L.append("")
    L.append("@endgantt")
    return "\n".join(L)


# --------------------------------------------------------------- pagina
CSS = """
:root{--bg:#12151b;--panel:#1a1f28;--panel2:#212836;--tx:#e6e9ef;--mut:#98a2b3;
--acc:#e3a82b;--line:#2c3543;--ok:#5ba85b;--warn:#c0503c}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--tx);
font:15px/1.55 -apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif}
a{color:var(--acc);text-decoration:none}a:hover{text-decoration:underline}
header{padding:28px 32px;border-bottom:1px solid var(--line);background:var(--panel)}
h1{margin:0 0 6px;font-size:24px}
.sub{color:var(--mut);max-width:100ch}
.wrap{display:grid;grid-template-columns:250px 1fr;gap:0}
nav{position:sticky;top:0;align-self:start;padding:20px 16px;border-right:1px solid var(--line);
max-height:100vh;overflow:auto;background:var(--panel)}
nav a{display:block;padding:6px 8px;border-radius:6px;color:var(--tx);font-size:13px}
nav a:hover{background:var(--panel2);text-decoration:none}
nav .k{color:var(--acc);font-weight:600;display:inline-block;width:44px}
nav .j{float:right;color:var(--mut);font-size:12px}
main{padding:24px 32px 80px;min-width:0}
section{margin:0 0 44px}
h2{font-size:19px;border-bottom:1px solid var(--line);padding-bottom:8px;margin:0 0 14px}
h3{font-size:15px;margin:18px 0 8px;color:var(--acc)}
table{border-collapse:collapse;width:100%;font-size:13px}
th,td{border:1px solid var(--line);padding:6px 8px;text-align:left;vertical-align:top}
th{background:var(--panel2);color:var(--mut);font-weight:600}
td.n,th.n{text-align:right;font-variant-numeric:tabular-nums}
tr.tot td{background:var(--panel2);font-weight:700}
.kpis{display:flex;gap:12px;flex-wrap:wrap;margin:0 0 18px}
.kpi{background:var(--panel);border:1px solid var(--line);border-radius:10px;padding:12px 16px;min-width:130px}
.kpi b{display:block;font-size:26px;color:var(--acc)}
.kpi span{color:var(--mut);font-size:12px}
.card{background:var(--panel);border:1px solid var(--line);border-left:4px solid var(--acc);
border-radius:8px;padding:12px 14px;margin:0 0 10px}
.card .id{font-weight:700;color:var(--acc);font-size:13px}
.card h4{margin:4px 0 8px;font-size:14px}
.card dl{margin:0;font-size:13px}
.card dt{color:var(--mut);font-size:11px;text-transform:uppercase;letter-spacing:.04em;margin-top:6px}
.card dd{margin:0}
.badge{display:inline-block;background:var(--panel2);border:1px solid var(--line);
border-radius:20px;padding:1px 9px;font-size:11px;color:var(--mut);margin-left:6px}
.wf{border-left-color:var(--ok)}
.dec{border-left-color:var(--warn)}
input#q{width:100%;padding:9px 11px;background:var(--panel2);border:1px solid var(--line);
border-radius:8px;color:var(--tx);font-size:14px;margin-bottom:12px}
.note{background:var(--panel2);border-left:4px solid var(--acc);padding:10px 14px;
border-radius:6px;color:var(--mut);font-size:13px;margin:0 0 16px}
code{background:var(--panel2);padding:1px 5px;border-radius:4px;font-size:12px}
.gantt{background:var(--panel);border:1px solid var(--line);border-radius:10px;padding:14px 16px}
.gphase{margin:0 0 18px}
.gphase h3{margin:0 0 6px;font-size:13px}
.gphase h3 .st{color:var(--mut);font-weight:400;margin-left:8px;font-size:12px}
.grow{display:grid;grid-template-columns:74px 1fr 250px;gap:8px;align-items:center;
padding:2px 0;font-size:12px}
.grow .gid{color:var(--mut);font-variant-numeric:tabular-nums}
.grow .gmeta{color:var(--mut);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.gtrack{position:relative;height:13px;background:var(--panel2);border-radius:3px;overflow:hidden}
.gtrack i{position:absolute;top:2px;bottom:2px;border-radius:2px;display:block}
.gtrack b{position:absolute;top:-2px;bottom:-2px;width:2px;background:var(--acc);display:block}
.grow.hito .gid{color:var(--acc)}
@media(max-width:900px){.grow{grid-template-columns:64px 1fr}.grow .gmeta{display:none}}
.dec .resp{border-left:3px solid var(--ok);padding-left:8px}
.dec.est-asumida .resp{border-left-color:var(--acc)}
.dec.est-abierta .resp{border-left-color:var(--warn)}
.badge.estado{text-transform:uppercase;letter-spacing:.05em}
.est-cerrada .badge.estado{color:var(--ok);border-color:var(--ok)}
.est-asumida .badge.estado{color:var(--acc);border-color:var(--acc)}
.est-abierta .badge.estado{color:var(--warn);border-color:var(--warn)}
.mut{color:var(--mut)}
.filtros{display:flex;gap:8px;margin:0 0 14px;flex-wrap:wrap}
.filtros button{background:var(--panel2);border:1px solid var(--line);color:var(--mut);
border-radius:20px;padding:4px 12px;font-size:12px;cursor:pointer}
.filtros button.on{color:var(--tx);border-color:var(--acc)}
.fase-head{display:flex;justify-content:space-between;align-items:baseline;gap:12px;flex-wrap:wrap}
.fase-head .st{color:var(--mut);font-size:12px}
@media(max-width:900px){.wrap{grid-template-columns:1fr}nav{position:static;max-height:none}}
"""

JS = """
const q=document.getElementById('q');
if(q){q.addEventListener('input',()=>{
  const v=q.value.toLowerCase();
  document.querySelectorAll('#decisiones .card').forEach(c=>{
    aplicar();});}
let ESTADO='todas';
function aplicar(){
  const v=(q?q.value:'').toLowerCase();
  document.querySelectorAll('#decisiones .card').forEach(c=>{
    const okE=ESTADO==='todas'||c.dataset.estado===ESTADO;
    const okT=!v||c.textContent.toLowerCase().includes(v);
    c.style.display=(okE&&okT)?'':'none';});
}
document.querySelectorAll('.filtros button').forEach(b=>b.addEventListener('click',()=>{
  document.querySelectorAll('.filtros button').forEach(x=>x.classList.remove('on'));
  b.classList.add('on');ESTADO=b.dataset.f;aplicar();}));
document.querySelectorAll('nav a').forEach(a=>a.addEventListener('click',e=>{
  const t=document.querySelector(a.getAttribute('href'));
  if(t){e.preventDefault();t.scrollIntoView({behavior:'smooth'});history.replaceState(null,'',a.getAttribute('href'));}
}));
"""


def pagina(decisiones, avisos):
    e = html.escape
    nav = ['<a href="#numero"><span class="k">0</span>El numero</a>',
           '<a href="#flujos"><span class="k">WF</span>Flujos de diseno</a>']
    for c in PHASE_ORDER:
        nav.append(f'<a href="#fase-{c}"><span class="k">{c}</span>'
                   f'{e(PHASE_TITLE[c][:28])}<span class="j">{PER_PHASE[c]:g} j</span></a>')
    nav += ['<a href="#gantt"><span class="k">G</span>Gantt visual</a>',
            '<a href="#cruces"><span class="k">x</span>Cruces de fase</a>',
            '<a href="#transversales"><span class="k">T</span>Transversales</a>',
            f'<a href="#decisiones"><span class="k">D</span>Decisiones ({len(decisiones)})</a>',
            '<a href="#reglas"><span class="k">!</span>Reglas</a>']

    filas = "".join(
        f'<tr><td>{c}</td><td>{e(PHASE_TITLE[c])}</td>'
        f'<td class="n">{N_PER_PHASE[c]}</td><td class="n">{PER_PHASE[c]:g}</td>'
        f'<td class="n">{setmanes(PER_PHASE[c])}</td><td>{e(PHASE_ORIGEN[c])}</td></tr>'
        for c in PHASE_ORDER)
    filas += (f'<tr class="tot"><td colspan="2">Total</td><td class="n">{len(TASKS)}</td>'
              f'<td class="n">{TOTAL:g}</td><td class="n">{setmanes(TOTAL)}</td><td></td></tr>')

    wf_cards = "".join(
        f'<div class="card wf"><span class="id">{w["id"]}</span>'
        f'<span class="badge">{w["decisiones"]} decisiones</span>'
        f'<span class="badge">{len(w["tareas"])} tareas</span>'
        f'<h4>{e(w["titulo"])}</h4><dl>'
        f'<dt>Documento</dt><dd>{e(w["doc"])}</dd>'
        f'<dt>Base real en el motor</dt><dd>{e(w["base"])}</dd>'
        f'<dt>Fichero</dt><dd><code>{e(w["fichero"])}</code></dd>'
        f'<dt>Tareas</dt><dd>{" ".join(f"<code>{t}</code>" for t in w["tareas"])}</dd>'
        f"</dl></div>" for w in WORKFLOWS)
    fs_rows = "".join(
        f'<tr><td>{f["id"]}</td><td>{e(f["titulo"])}</td><td>{e(f["doc"])}</td>'
        f'<td>{e(f["prioridad"])}</td><td>{len(f["tareas"])}</td></tr>' for f in SECUNDARIOS)

    fases = []
    for i, c in enumerate(PHASE_ORDER):
        cards = []
        for pid in ORDER:
            t = BY_ID[pid]
            if t["fase"] != c:
                continue
            dep = ", ".join(f'<a href="#t-{d}">{d}</a>' for d in t["deps"]) or "ninguna"
            flu = ", ".join(FLUJO_DE.get(pid, [])) or "transversal"
            cards.append(
                f'<div class="card" id="t-{pid}" style="border-left-color:{COLORES[i % len(COLORES)]}">'
                f'<span class="id">{pid}</span><span class="badge">{t["j"]:g} j</span>'
                f'<span class="badge">{e(flu)}</span>'
                + (f'<span class="badge">{e(t["carril"])} · {e(t.get("quien", ""))}</span>' if t.get("carril") else "")
                + f'<h4>{e(t["titulo"])}</h4><dl>'
                f'<dt>Documento</dt><dd>{e(t["doc"])}</dd>'
                f'<dt>Ya existe</dt><dd>{e(t["existe"])}</dd>'
                + (f'<dt>Zona</dt><dd>{e(t.get("zona", ""))}</dd>' if t.get("carril") else "") +
                f'<dt>Cierre</dt><dd>{e(t["cierre"])}</dd>'
                f'<dt>Depende de</dt><dd>{dep}</dd></dl></div>')
        fases.append(
            f'<section id="fase-{c}"><div class="fase-head"><h2>{c} — {e(PHASE_TITLE[c])}</h2>'
            f'<span class="st">{N_PER_PHASE[c]} tareas · {PER_PHASE[c]:g} j · '
            f'{setmanes(PER_PHASE[c])} semanas · doc {e(PHASE_DOC[c])} · {e(PHASE_ORIGEN[c])}</span>'
            f'</div>{"".join(cards)}</section>')

    dec = []
    for d in decisiones:
        r = RES[d["id"]]
        lit, fuente = RESPUESTAS.get(d["id"], ("(sin respuesta literal)", ""))
        dec.append(
            f'<div class="card dec est-{r["estado"]}" data-estado="{r["estado"]}">'
            f'<span class="id">{d["id"]}</span>'
            f'<span class="badge estado">{e(r["estado"])}</span>'
            f'<span class="badge">{e(d["archivo"])}</span><h4>{e(d["pregunta"])}</h4><dl>'
            f'<dt>Oriol responde</dt><dd class="resp">{e(lit)} '
            f'<small>({e(fuente)})</small></dd>'
            f'<dt>Consecuencia en el plan</dt><dd>{e(r["consecuencia"])}</dd>'
            f'<dt>Tareas</dt><dd>{" ".join(f"<code>{x}</code>" for x in r["tareas"])}</dd>'
            f'<dt>Recomendacion que se hizo</dt><dd class="mut">{e(d["recomendacion"])}</dd>'
            f'</dl></div>')
    dec = "".join(dec)

    barras = []
    for i, c in enumerate(PHASE_ORDER):
        color = COLORES[i % len(COLORES)]
        filas_b = []
        for pid in ORDER:
            t = BY_ID[pid]
            if t["fase"] != c:
                continue
            left = INICI[pid] / SPAN * 100
            width = max(0.35, t["j"] / SPAN * 100)
            dep = ", ".join(t["deps"]) or "—"
            flu = ", ".join(FLUJO_DE.get(pid, [])) or "transversal"
            filas_b.append(
                f'<div class="grow"><a class="gid" href="#t-{pid}">{pid}</a>'
                f'<div class="gtrack"><i style="left:{left:.3f}%;width:{width:.3f}%;'
                f'background:{color}" title="{e(t["titulo"])}"></i></div>'
                f'<span class="gmeta">{t["j"]:g} j · {e(flu)} · deps {e(dep)}</span></div>')
        hito = max(TERMINALS[c], key=lambda x: FIN[x]) if TERMINALS[c] else None
        if hito:
            left = FIN[hito] / SPAN * 100
            filas_b.append(f'<div class="grow hito"><span class="gid">{c} cerrada</span>'
                           f'<div class="gtrack"><b style="left:{left:.3f}%"></b></div>'
                           f'<span class="gmeta">jornada {FIN[hito]:g} de {SPAN:g}</span></div>')
        barras.append(f'<div class="gphase"><h3 style="color:{color}">{c} — '
                      f'{e(PHASE_TITLE[c])}<span class="st">{PER_PHASE[c]:g} j</span></h3>'
                      + "".join(filas_b) + "</div>")
    gantt_visual = "".join(barras)

    cruces = "".join(f"<li>{e(a)}</li>" for a in avisos) or "<li>ninguno</li>"
    trans = "".join(f'<tr><td>{p}</td><td>{e(BY_ID[p]["titulo"])}</td>'
                    f'<td>{e(BY_ID[p]["fase"])}</td><td class="n">{BY_ID[p]["j"]:g}</td></tr>'
                    for p in TRANSVERSALES)

    return f"""<!doctype html>
<html lang="es"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Plan Alcance y Vision — Onegai</title><style>{CSS}</style></head><body>
<header>
<h1>Plan derivado del Documento de Alcance y Vision</h1>
<p class="sub">Universo de Onegai · generado por <code>build.py</code> desde <code>dades.py</code>
y <code>workflows_juego/</code>. {len(TASKS)} tareas en {len(PHASES)} fases, {TOTAL:g} jornadas
(~{setmanes(TOTAL)} semanas a {RITME} j/semana), {len(WORKFLOWS)} flujos nucleares +
{len(SECUNDARIOS)} secundarios y {len(decisiones)} decisiones abiertas.
Las fechas son una escala de esfuerzo, no una prevision de entrega.</p>
</header>
<div class="wrap"><nav>{''.join(nav)}</nav><main>
<section id="numero"><h2>0. El numero</h2>
<div class="kpis">
<div class="kpi"><b>{len(TASKS)}</b><span>tareas</span></div>
<div class="kpi"><b>{TOTAL:g}</b><span>jornadas (~6 h)</span></div>
<div class="kpi"><b>{setmanes(TOTAL)}</b><span>semanas a {RITME} j/sem</span></div>
<div class="kpi"><b>{len(decisiones)}</b><span>decisiones abiertas</span></div>
<div class="kpi"><b>{len(WORKFLOWS)+len(SECUNDARIOS)}</b><span>flujos de diseno</span></div>
</div>
<table><thead><tr><th>Fase</th><th>Titulo</th><th class="n">Tareas</th>
<th class="n">Jornadas</th><th class="n">Semanas</th><th>Origen</th></tr></thead>
<tbody>{filas}</tbody></table>
<p class="note">Una jornada son ~6 h de trabajo enfocado. Es una estimacion de
<strong>alcance completo</strong>, no de trabajo restante: muchas tareas continuan
paquetes ya vivos (fase H, EST, corte B, fase E) y la columna «Ya existe» de cada
una dice de donde parte.</p></section>
<section id="flujos"><h2>WF. Flujos de diseno</h2>
<p class="note">La capa entre la vision y las tareas: que tiene que pasar, con que
numeros, quien se entera, que existe YA en el motor (con fichero y linea) y que
decision lo bloquea. Plantilla de 13 apartados y matriz de cruces en
<code>workflows_juego/INDICE.md</code>.</p>
{wf_cards}
<h3>Flujos secundarios</h3>
<table><thead><tr><th>FS</th><th>Flujo</th><th>Documento</th><th>Prioridad</th>
<th class="n">Tareas</th></tr></thead><tbody>{fs_rows}</tbody></table></section>
{''.join(fases)}
<section id="gantt"><h2>Gantt visual (escala de esfuerzo)</h2>
<p class="note">La cadena de dependencias mas larga (camino critico) son
<strong>{SPAN:g} jornadas</strong>; el esfuerzo total es de {TOTAL:g}. Con una sola persona
el calendario lo marca el esfuerzo total: <strong>~{setmanes(TOTAL)} semanas</strong> a
{RITME} j/semana. El camino critico solo manda si hay muchos brazos en paralelo, y entonces
el minimo absoluto serian esas {SPAN:g} jornadas (~{setmanes(SPAN)} semanas). El grafo es
muy ancho a proposito: la mayoria de tareas de una fase no dependen entre ellas, y eso es
lo que permite repartirlas entre agentes. Cada barra empieza cuando terminan sus
dependencias. Fuente de verdad: <code>gantt_alcance_y_vision.puml</code>, generado desde el
mismo grafo.</p>
<div class="gantt">{gantt_visual}</div></section>
<section id="cruces"><h2>Cruces de fase declarados</h2>
<p class="note">El orden de presentacion es el del documento; el orden real lo da el
grafo. Estas dependencias apuntan a una fase posterior a proposito.</p>
<ul>{cruces}</ul></section>
<section id="transversales"><h2>Tareas transversales</h2>
<p class="note">{len(TRANSVERSALES)} de {len(TASKS)} tareas no pertenecen a un flujo del
jugador: son contratos, editor, red y aceptacion. Habilitan flujos.</p>
<table><thead><tr><th>ID</th><th>Titulo</th><th>Fase</th><th class="n">j</th></tr></thead>
<tbody>{trans}</tbody></table></section>
<section id="decisiones"><h2>Bandeja de decisiones ({len(decisiones)})</h2>
<p class="note">Extraidas del apartado 12 de cada flujo. Ninguna requiere codigo para
resolverse. Al responderlas se convierten en entradas <code>D00xx</code> de
<code>decisiones/</code> (formato MADR).</p>
<input id="q" type="search" placeholder="Filtrar decisiones (p. ej. COMB, moral, calendario)...">
<div class="filtros">
<button data-f="todas" class="on">todas ({len(decisiones)})</button>
<button data-f="cerrada">cerradas ({len([d for d in decisiones if RES[d["id"]]["estado"] == "cerrada"])})</button>
<button data-f="asumida">asumidas ({len([d for d in decisiones if RES[d["id"]]["estado"] == "asumida"])})</button>
<button data-f="abierta">abiertas ({len([d for d in decisiones if RES[d["id"]]["estado"] == "abierta"])})</button>
</div>
{dec}</section>
<section id="reglas"><h2>Reglas de trabajo</h2><ul>
<li>Ninguna tarea se empieza con sus dependencias abiertas.</li>
<li>Ningun flujo se implementa sin su decision resuelta cuando esa decision lo bloquea.</li>
<li>Una tarea se cierra cuando llega al jugador, no cuando compila (<code>medir_alcance.py</code>, AV12-01).</li>
<li>No se planifica dos veces lo mismo: la columna «Ya existe» y el apartado 11 de cada flujo son la defensa.</li>
<li>Las discrepancias se registran, no se resuelven en silencio.</li>
<li>Si una tarea supera el doble de su estimacion, se para y se replantea.</li>
<li>El flujo es en una sola direccion: <code>canon -&gt; cartas -&gt; motor</code>.</li>
</ul></section>
</main></div><script>{JS}</script></body></html>
"""


if __name__ == "__main__":
    errores, avisos, decisiones = validar()
    if errores:
        raise SystemExit("Plan incoherente, no se genera nada:\n  " + "\n  ".join(errores))
    (AQUI / "render").mkdir(exist_ok=True)
    for ruta, texto in ((AQUI / "DECISIONES_RESUELTAS.md", decisiones_md(decisiones)),
                        (AQUI / "PLAN_ALCANCE_Y_VISION.md", plano_texto(decisiones, avisos)),
                        (AQUI / "gantt_alcance_y_vision.puml", puml(avisos)),
                        (AQUI / "render" / "plan_alcance_y_vision.html", pagina(decisiones, avisos))):
        ruta.write_text(texto if texto.endswith("\n") else texto + "\n", encoding="utf-8")
    print(f"tareas {len(TASKS)} · jornadas {TOTAL:g} · semanas ~{setmanes(TOTAL)}")
    print(f"fases {len(PHASES)} · flujos {len(WORKFLOWS)}+{len(SECUNDARIOS)} · "
          f"decisiones {len(decisiones)} · transversales {len(TRANSVERSALES)}")
    for a in avisos:
        print("aviso:", a)
    estados = {e: len([d for d in decisiones if RES[d["id"]]["estado"] == e])
               for e in ("cerrada", "asumida", "abierta")}
    print("decisiones:", estados)
    print("escrito: DECISIONES_RESUELTAS.md, PLAN_ALCANCE_Y_VISION.md, "
          "gantt_alcance_y_vision.puml, render/plan_alcance_y_vision.html")
