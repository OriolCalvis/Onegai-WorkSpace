#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Genera el markdown, el Gantt i la pàgina del pla a partir de dades.py."""
import html
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dades import (P, PHASES, DECISIONS, FORA, SCREEN_COVERAGE,
                   BASELINE_PACKAGES, BASELINE_EFFORT)  # noqa: E402
from seguiment import load_progress, markdown as progress_md, webpage as progress_html
from pathlib import Path

BY_ID = {p[0]: p for p in P}
PROGRESS = load_progress(BY_ID)
PHASE_TITLE = {c: t for c, t, _s, _d in PHASES}

# --- comprovacions: res a l'atzar vol dir que això ha de quadrar ---------
errors = []
if len(BY_ID) != len(P):
    errors.append("IDs de paquet duplicats")
PHASE_ORDER = [c for c, _t, _s, _d in PHASES]
for pid, title, ph, eff, deps, why, files, done, test in P:
    if ph not in PHASE_TITLE:
        errors.append(f"{pid}: fase desconeguda {ph}")
    for d in deps:
        if d not in BY_ID:
            errors.append(f"{pid}: depèn de {d}, que no existeix")
        elif (ph in PHASE_ORDER and BY_ID[d][2] in PHASE_ORDER
              and PHASE_ORDER.index(BY_ID[d][2]) > PHASE_ORDER.index(ph)):
            errors.append(f"{pid}: depèn d'una fase posterior ({d})")
    if eff <= 0:
        errors.append(f"{pid}: esforç no positiu")
    if not all((title, why, files, done, test)):
        errors.append(f"{pid}: falta descripció, fitxers, acceptació o prova")
for area, screen, current, scope, packages, acceptance in SCREEN_COVERAGE:
    if scope not in ("Corte B", "1.1") or not all((area, screen, current, acceptance)):
        errors.append(f"Pantalla incompleta: {screen}")
    if scope == "Corte B" and not packages:
        errors.append(f"Pantalla del corte sense paquet: {screen}")
    for pid in packages:
        if pid not in BY_ID:
            errors.append(f"Pantalla {screen}: paquet desconegut {pid}")
if errors:
    raise SystemExit("Pla incoherent:\n  " + "\n  ".join(errors))

# ordre topològic estable (per id dins de cada nivell)
order, pending = [], [p[0] for p in P]
while pending:
    ready = [i for i in pending if all(d in order for d in BY_ID[i][4])]
    if not ready:
        raise SystemExit("Cicle de dependències: " + ", ".join(pending))
    ready.sort()
    order += ready
    pending = [i for i in pending if i not in order]

# --- portes de fase -----------------------------------------------------
# La decisio va ser "consolidacio abans de res mes": les fases no s'encavalquen.
# Cada arrel espera TOTS els terminals de la fase anterior; l'últim ID en ordre
# topològic no és necessàriament el de durada més llarga. Es una politica, no una dependencia
# tecnica, i per aixo viu aqui i no a dades.py.
TERMINALS = {
    c: [pid for pid in order if BY_ID[pid][2] == c
        and not any(pid in p[4] for p in P if p[2] == c)]
    for c in PHASE_ORDER
}
GATE = {}
for idx, c in enumerate(PHASE_ORDER):
    if idx == 0:
        continue
    prev = PHASE_ORDER[idx - 1]
    for pid in [i for i in order if BY_ID[i][2] == c]:
        if not any(BY_ID[d][2] == c for d in BY_ID[pid][4]):
            GATE[pid] = TERMINALS[prev]

ALL_DEPS = {pid: list(dict.fromkeys(list(BY_ID[pid][4]) + GATE.get(pid, [])))
            for pid in BY_ID}

sched, pend = [], list(BY_ID)
while pend:
    ready = sorted(i for i in pend if all(d in sched for d in ALL_DEPS[i]))
    if not ready:
        raise SystemExit("Cicle amb portes de fase: " + ", ".join(pend))
    sched += ready
    pend = [i for i in pend if i not in sched]

EARLIEST_END = {}
for pid in sched:
    EARLIEST_END[pid] = max((EARLIEST_END[d] for d in ALL_DEPS[pid]), default=0) + BY_ID[pid][3]

TOTAL = sum(p[3] for p in P)
PER_PHASE = {c: sum(p[3] for p in P if p[2] == c) for c, _t, _s, _d in PHASES}
RITME = 1.5  # jornades d'esforç per setmana natural, ritme real de ratos lliures


def setmanes(j):
    return round(j / RITME)


# ------------------------------------------------------------------ markdown
def md():
    L = []
    A = L.append
    A("# Pla definitiu — MotorGraphico Creator, corte B\n")
    A("Generat des de `planificacion/pla/dades.py` i `planificacion/seguimiento/*.json`. Aquest fitxer no s'edita a mà: "
      "es regenera, i així el markdown, el Gantt i la pàgina no poden divergir.\n")

    A("## 1. Les decisions que fixen el pla\n")
    for tema, tria, per in DECISIONS:
        A(f"**{tema} → {tria}.** {per}\n")

    A("## 2. El número\n")
    A(f"| Fase | Paquets | Jornades | Setmanes a ritme real |")
    A("| --- | ---: | ---: | ---: |")
    for c, t, _s, _d in PHASES:
        n = len([p for p in P if p[2] == c])
        A(f"| {c} — {t} | {n} | {PER_PHASE[c]} | {setmanes(PER_PHASE[c])} |")
    A(f"| **Total** | **{len(P)}** | **{TOTAL}** | **{setmanes(TOTAL)}** |\n")
    A("Una jornada és ~6 h de treball enfocat. A ritme de ratos lliures i caps de "
      f"setmana (~1,5 jornades per setmana), les {TOTAL} jornades són unes "
      f"{setmanes(TOTAL)} setmanes naturals.\n")
    A(f"**Revisió de pantalles (03/09):** la base era de {BASELINE_PACKAGES} paquets "
      f"i {BASELINE_EFFORT} jornades. S'hi afegeixen {len(P)-BASELINE_PACKAGES} paquets "
      f"i {TOTAL-BASELINE_EFFORT} jornades per tancar les pantalles i recorreguts. "
      "És una estimació completa d'abast, no de feina restant ni una data de "
      "lliurament. No es resten paquets parcialment verificats ni es compten "
      "caselles locals com a tancaments de CI. El calendari del Gantt és una "
      "escala d'esforç amb dependències, no una previsió de dates naturals.\n")

    A("## 3. Què vol dir «fet» a cada fase\n")
    for c, t, sub, done in PHASES:
        A(f"### Fase {c} — {t}\n")
        A(f"{sub}\n")
        A(f"**Tancada quan:** {done}\n")

    A("## 4. Els paquets\n")
    A("Cada paquet porta el seu criteri d'acceptació i la prova que el tanca. Si "
      "una d'aquestes dues coses no es pot escriure, el paquet no està prou definit "
      "per començar-lo.\n")
    for c, t, _s, _d in PHASES:
        A(f"### Fase {c} — {t}\n")
        for pid in order:
            p = BY_ID[pid]
            if p[2] != c:
                continue
            pid, title, ph, eff, deps, why, files, done, test = p
            dep = ", ".join(deps) if deps else "cap"
            A(f"#### {pid} — {title}\n")
            A(f"`{eff} j` · depèn de: {dep}\n")
            A(f"**Per què.** {strip(why)}\n")
            A("**Fitxers.** " + ", ".join(f"`{f}`" for f in files) + "\n")
            A(f"**Fet quan.** {strip(done)}\n")
            A(f"**Ho prova.** {strip(test)}\n")
            if pid in PROGRESS:
                A(progress_md(PROGRESS[pid]))

    A("## 5. Fora d'abast, per escrit\n")
    A("El corte B es defineix tant pel que inclou com pel que no. Això queda per a "
      "1.1, i està aquí perquè ningú (jo inclòs) ho coli sense adonar-se'n.\n")
    A("| Queda fora | Paquets del Gantt vell | Motiu |")
    A("| --- | --- | --- |")
    for què, paq, motiu in FORA:
        A(f"| {què} | {paq} | {motiu} |")
    A("")

    A("## 6. Regles de treball\n")
    A("- **Un paquet no es comença sense les seves dependències tancades.** L'ordre "
      "topològic del punt 4 ja el respecta.\n"
      "- **Un paquet es tanca quan la seva prova passa a CI**, no quan el codi "
      "compila a la màquina de qui l'ha escrit.\n"
      "- **Cap paquet nou entra al pla sense criteri d'acceptació i prova.** Si no "
      "es poden escriure, el que falta és entendre el problema, no programar-lo.\n"
      "- **Si un paquet supera el doble de la seva estimació, es para i es "
      "replanteja**, en comptes d'anar-hi afegint jornades.\n"
      "- **Les fases no s'encavalquen.** Cada fase s'obre quan la anterior ha "
      "tancat: és el que vol dir «consolidació abans de res més». Al Gantt això "
      "són les portes de fase.\n"
      "- **La fase C no admet funcions noves.** Qualsevol idea que aparegui durant "
      "la consolidació es registra per després del seu tancament. Les pantalles "
      "autoritzades en aquesta revisió viuen a E/G; la resta segueix a 1.1.\n"
      "- **Una pantalla no es tanca només amb tests del model.** Cal exercitar "
      "el control visible, comprovar l'efecte real i registrar el recorregut "
      "de retorn/error. G-16 és la porta final d'acceptació amb persones noves.\n")
    A("## 7. Cobertura de pantalles i funcionalitats\n")
    A("Fotografia de la revisió del 03/09/2026; no és un percentatge ni una "
      "declaració de feina acabada. Els IDs són compromisos d'execució. "
      "El seguiment de cada paquet continua al seu registre.\n")
    A("| Àmbit / pantalla | Fotografia de partida | Abast / paquets | Recorregut que ho tanca |")
    A("| --- | --- | --- | --- |")
    for area, screen, current, scope, packages, acceptance in SCREEN_COVERAGE:
        refs = ", ".join(packages) or "fora del corte"
        A(f"| {area} · {screen} | {current} | {scope}: {refs} | {acceptance} |")
    A("\n## 8. Ordre d'execució i correspondència amb plans previs\n")
    A("C → D → E → F → G. Cada arrel de fase espera tots els terminals de "
      "l'anterior, no només l'últim ID. La revisió manté els IDs existents. "
      "E-10..14 concreten ADR-001/EDT-01..06 i els inspectors; E-15 connecta "
      "diagnòstics; E-16 avança el mínim de GDB a dades locals de projecte. "
      "G-08..14 connecten pantalles del jugador; G-15 embolcalla G-06; G-16 "
      "tanca el recorregut complet.\n")
    A("La pantalla F1 de temporitzadors del TABLERO conserva el seu propietari: "
      "G-14 integra el resultat, no crea una tasca rival. Els roadmaps "
      "F0–F8/EDT/GDB i les auditories anteriors són referències de procedència; "
      "no s'hi sumen jornades per segona vegada. Aquesta revisió només actualitza "
      "el Corte B; no resol d'amagat la numeració històrica pendent al TABLERO.\n")
    return "\n".join(L)


def strip(s):
    return re.sub(r"</?code>", "`", s)


# ------------------------------------------------------------------ gantt
def puml():
    L = ["@startgantt", "",
         "title MotorGraphico Creator — corte B (cicle d'autor complet)", "",
         "caption Esquema de dependències amb tasques en paral·lel. Les dates no són compromisos de lliurament ni treball restant.", "",
         "' Generat des de planificacion/pla/dades.py. No s'edita a ma.",
         "' 1 dia del Gantt = 1 JORNADA d'esforc (~6 h), no un dia natural.",
         f"' Total {TOTAL} jornades ≈ {setmanes(TOTAL)} setmanes a ~1,5 j/setmana.", "",
         "Project starts 2026-09-07", "saturday are closed", "sunday are closed", ""]
    colors = {"C": "#C0503C", "D": "#E3A82B", "E": "#5BA85B", "F": "#4E86B4", "G": "#8A6BA8"}
    # Els noms porten comes i apostrofs, i PlantUML no els resol be dins d'un
    # enllac: cada tasca rep un alias curt i totes les fletxes hi apunten.
    alias = {pid: pid.replace("-", "") for pid in BY_ID}
    for c, t, _s, _d in PHASES:
        L.append(f"-- Fase {c} — {t} ({PER_PHASE[c]} j) --")
        L.append("")
        for pid in order:
            p = BY_ID[pid]
            if p[2] != c:
                continue
            a = alias[pid]
            L.append(f"[{p[0]} {p[1]}] as [{a}] lasts {p[3]} days")
            L.append(f"[{a}] is colored in {colors[c]}")
        L.append("")
    L.append("' --- dependencies (tecniques i portes de fase) ---")
    L.append("' En ordre topologic: PlantUML col·loca cada tasca quan llegeix el")
    L.append("' seu enllac i no reajusta les que ja ha situat.")
    for pid in sched:
        # L'últim enllaç llegit ha de ser el predecessor que acaba més tard.
        for d in sorted(ALL_DEPS[pid], key=lambda dep: EARLIEST_END[dep]):
            if d not in BY_ID[pid][4]:
                L.append(f"' porta de fase {BY_ID[d][2]} -> {BY_ID[pid][2]}")
            L.append(f"[{alias[d]}] -> [{alias[pid]}]")
    L.append("")
    L.append("' --- fites ---")
    for c, t, _s, _d in PHASES:
        last = max(TERMINALS[c], key=lambda pid: EARLIEST_END[pid])
        L.append(f"[Fase {c} tancada] happens at [{last.replace('-', '')}]'s end")
    L.append("")
    for pid, record in PROGRESS.items():
        L.append(f"\' Seguiment {pid}: {record['date']} {record['author']} — {record['status']}")
    L.append("@endgantt")
    return "\n".join(L)


# ------------------------------------------------------------------ html
def page():
    with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "plantilla.html"),
              encoding="utf-8") as fh:
        tpl = fh.read()

    dec = "\n".join(
        f'<div class="decision"><p class="dec-topic">{html.escape(t)}</p>'
        f'<p class="dec-choice">{html.escape(c)}</p>'
        f'<p class="dec-why">{html.escape(w)}</p></div>'
        for t, c, w in DECISIONS)

    rows = "\n".join(
        f'<tr><td>{c} — {html.escape(t)}</td>'
        f'<td class="num">{len([p for p in P if p[2] == c])}</td>'
        f'<td class="num">{PER_PHASE[c]}</td>'
        f'<td class="num">{setmanes(PER_PHASE[c])}</td></tr>'
        for c, t, _s, _d in PHASES)
    rows += (f'<tr class="tot"><td>Total</td><td class="num">{len(P)}</td>'
             f'<td class="num">{TOTAL}</td><td class="num">{setmanes(TOTAL)}</td></tr>')

    phases_html = []
    nav = []
    for c, t, sub, done in PHASES:
        pkgs = [BY_ID[i] for i in order if BY_ID[i][2] == c]
        nav.append(f'<a href="#fase-{c}" class="navlink" data-target="fase-{c}">'
                   f'<span class="nav-c">{c}</span>{html.escape(t)}'
                   f'<span class="nav-j">{PER_PHASE[c]} j</span></a>')
        cards = []
        for pid, title, ph, eff, deps, why, files, dn, test in pkgs:
            dep = ("".join(f'<a href="#{d}">{d}</a>' for d in deps)
                   if deps else '<span class="muted">cap</span>')
            cards.append(
                f'<article class="pkg" id="{pid}" data-phase="{c}">'
                f'<div class="pkg-top">'
                f'<label class="tick"><input type="checkbox" data-id="{pid}">'
                f'<span class="pid">{pid}</span></label>'
                f'<h3>{html.escape(title)}</h3>'
                f'<span class="eff">{eff} j</span></div>'
                f'<p class="why">{why}</p>'
                f'<dl class="meta">'
                f'<dt>Fet quan</dt><dd>{dn}</dd>'
                f'<dt>Ho prova</dt><dd>{test}</dd>'
                f'<dt>Fitxers</dt><dd class="files">'
                + " ".join(f"<code>{html.escape(f)}</code>" for f in files) +
                f'</dd><dt>Depèn de</dt><dd class="deps">{dep}</dd></dl>'
                f'{progress_html(PROGRESS.get(pid))}</article>')
        phases_html.append(
            f'<section id="fase-{c}" class="phase">'
            f'<header class="phase-head">'
            f'<div class="phase-id">{c}</div>'
            f'<div><h2>{html.escape(t)}</h2>'
            f'<p class="phase-sub">{html.escape(sub)}</p>'
            f'<p class="phase-done"><strong>Tancada quan:</strong> {html.escape(done)}</p></div>'
            f'<div class="phase-stat"><b>{PER_PHASE[c]}</b><span>jornades</span>'
            f'<div class="bar"><i data-phase="{c}" style="width:0%"></i></div>'
            f'<em data-count="{c}">0 de {len(pkgs)}</em></div>'
            f'</header>{"".join(cards)}</section>')

    fora = "\n".join(
        f'<tr><td>{html.escape(q)}</td><td><code>{html.escape(p)}</code></td>'
        f'<td>{html.escape(m)}</td></tr>' for q, p, m in FORA)

    coverage = "\n".join(
        f'<tr><td>{html.escape(area)} · <strong>{html.escape(screen)}</strong></td>'
        f'<td>{html.escape(current)}</td><td>{html.escape(scope)}<br>'
        + (", ".join(f'<a href="#{pid}">{pid}</a>' for pid in packages) or "Fora del corte")
        + f'</td><td>{html.escape(acceptance)}</td></tr>'
        for area, screen, current, scope, packages, acceptance in SCREEN_COVERAGE)

    return (tpl
            .replace("<!--DECISIONS-->", dec)
            .replace("<!--ROWS-->", rows)
            .replace("<!--NAV-->", "\n".join(nav))
            .replace("<!--PHASES-->", "\n".join(phases_html))
            .replace("<!--FORA-->", fora)
            .replace("<!--COVERAGE-->", coverage)
            .replace("{{TOTAL}}", str(TOTAL))
            .replace("{{N}}", str(len(P)))
            .replace("{{SET}}", str(setmanes(TOTAL)))
            .replace("{{BASE_N}}", str(BASELINE_PACKAGES))
            .replace("{{BASE_J}}", str(BASELINE_EFFORT))
            .replace("{{ADDED_N}}", str(len(P) - BASELINE_PACKAGES))
            .replace("{{ADDED_J}}", str(TOTAL - BASELINE_EFFORT))
            .replace("{{NC}}", str(PER_PHASE["C"])))


if __name__ == "__main__":
    out = Path(__file__).resolve().parents[1]
    (out / "render").mkdir(parents=True, exist_ok=True)
    (out / "PLA_DEFINITIU.md").write_text(md(), encoding="utf-8")
    (out / "gantt_corte_b.puml").write_text(puml(), encoding="utf-8")
    (out / "render" / "pla.html").write_text(page(), encoding="utf-8")
    print(f"paquets {len(P)} · jornades {TOTAL} · setmanes {setmanes(TOTAL)}")
    print("per fase:", {c: PER_PHASE[c] for c, _t, _s, _d in PHASES})
    print("ordre:", " ".join(order[:8]), "…")
