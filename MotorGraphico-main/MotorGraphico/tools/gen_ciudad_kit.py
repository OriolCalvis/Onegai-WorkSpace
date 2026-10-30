"""ISO-02 — Boundington amb les peces del kit isometric (tools/iso): converteix
les cases i els locals que ha pintat el generador en peces orientades com els
mobles d'Habbo. La rajola de cada empremta diu la familia (o l'ofici) i cap on
mira la porta; el motor hi posa la peça que toca per mida (tools/iso/cataleg_kit.py).

  - Locals: la porta (objecte puerta_*) diu l'ofici; s'escull l'orientacio amb
    carrer davant de la porta (preferint la facana que es veu), el llindar es
    mou a la cel·la de la porta i l'objecte puerta_* amb ell.
  - Cases: cada empremta es redueix al rectangle mes gran que el cataleg sap
    dibuixar, sense tocar cap altra peça (les cel·les que sobren passen a pati),
    i es gira cap al costat amb mes carrer.
"""
import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "iso"))
from cataleg_kit import (FAMILIES, OFICIS, PORTA_OFICI, cella_llindar, costat_porta, mides_ofici,  # noqa: E402
                         rajola_familia, rajola_ofici)

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
_T = {t["nombre"]: t["gid"] for t in json.load(open(BASE + "assets/textures/terreno_iso.json"))["tiles"]}
FAM = {fam: [_T[rajola_familia(fam, o)] for o in range(4)] for fam in FAMILIES}
OFI = {ofi: [_T[rajola_ofici(ofi, o)] for o in range(4)] for ofi in OFICIS}
KIT = {g for v in list(FAM.values()) + list(OFI.values()) for g in v}
FAM_DE_GID = {g: fam for fam, v in FAM.items() for g in v}
OFI_DE_GID = {g: ofi for ofi, v in OFI.items() for g in v}
PREFERENCIA = {0: 0.6, 3: 0.5, 1: 0.1, 2: 0.0}      # a igualtat de carrer, la facana que es veu


def canonica(w, h, o):
    return (w, h) if o % 2 == 0 else (h, w)


def components(m, gids):
    vistos, out = set(), []
    for y in range(m.h):
        for x in range(m.w):
            if (x, y) in vistos or m.g[y][x] not in gids:
                continue
            gid = m.g[y][x]
            pila, cel = [(x, y)], []
            vistos.add((x, y))
            while pila:
                cx, cy = pila.pop()
                cel.append((cx, cy))
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    n = (cx + dx, cy + dy)
                    if n not in vistos and m.dentro(*n) and m.g[n[1]][n[0]] == gid:
                        vistos.add(n)
                        pila.append(n)
            out.append((gid, cel))
    return out


def caixa(cel):
    xs, ys = [c[0] for c in cel], [c[1] for c in cel]
    return min(xs), min(ys), max(xs) - min(xs) + 1, max(ys) - min(ys) + 1


def costat(x0, y0, w, h, o):
    return {0: [(x, y0 + h) for x in range(x0, x0 + w)], 3: [(x0 + w, y) for y in range(y0, y0 + h)],
            1: [(x0 - 1, y) for y in range(y0, y0 + h)], 2: [(x, y0 - 1) for x in range(x0, x0 + w)]}[o]


def veins(cel):
    out = set()
    for (x, y) in cel:
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            out.add((x + dx, y + dy))
    return out - set(cel)


def sol_del_voltant(m, cel, per_defecte):
    """El terra transitable mes habitual al voltant (pati que queda d'una casa reduida)."""
    from collections import Counter
    c = Counter(m.g[y][x] for (x, y) in veins(cel) if m.dentro(x, y) and m.libre(x, y))
    return c.most_common(1)[0][0] if c else per_defecte


def rect_maxim(cel, mides):
    """El rectangle mes gran de mides permeses dins de les cel·les; None si no n'hi cap."""
    cs = set(cel)
    x0, y0, w, h = caixa(cel)
    for (mw, mh) in sorted(mides, key=lambda s: (-s[0] * s[1], -min(s))):
        for y in range(y0, y0 + h - mh + 1):
            for x in range(x0, x0 + w - mw + 1):
                if all((xx, yy) in cs for yy in range(y, y + mh) for xx in range(x, x + mw)):
                    return x, y, mw, mh
    return None


def mides_mapa(fam):
    out = set()
    for (a, b) in FAMILIES[fam]["mides"]:
        out.add((a, b))
        out.add((b, a))
    return out


def locals_kit(m, umbral):
    """Els locals amb porta coneguda passen a local_<ofici>_oN amb el llindar a la porta."""
    ocupat, fets = set(), 0
    portes = [o for o in m.objetos if o["objectId"] in PORTA_OFICI]
    for porta in portes:
        ofici = PORTA_OFICI[porta["objectId"]]
        px, py = porta["position"]["x"], porta["position"]["y"]
        gid = next((m.g[py + dy][px + dx] for dx, dy in ((0, -1), (-1, 0), (1, 0), (0, 1))
                    if m.dentro(px + dx, py + dy) and m.g[py + dy][px + dx] not in (umbral,) and not m.libre(px + dx, py + dy)), None)
        if gid is None:
            continue
        cel = [c for _, cc in components(m, {gid}) if any(abs(c[0] - px) + abs(c[1] - py) == 1 for c in cc) for c in cc]
        if not cel:
            continue
        x0, y0, w, h = caixa(cel + [(px, py)])
        millor = None
        for o in range(4):
            if canonica(w, h, o) not in mides_ofici(ofici):
                continue
            dx, dy = cella_llindar(w, h, o)
            fx, fy = costat_porta(o)
            davant = (x0 + dx + fx, y0 + dy + fy)
            if not (m.dentro(*davant) and m.libre(*davant)):
                continue
            nota = sum(1 for c in costat(x0, y0, w, h, o) if m.dentro(*c) and m.libre(*c)) + PREFERENCIA[o] + (1 if o == 0 else 0)
            if millor is None or nota > millor[0]:
                millor = (nota, o, (x0 + dx, y0 + dy))
        if millor is None:
            print(f"    AVIS: {porta['objectId']} ({ofici} {w}x{h}) sense orientacio amb carrer davant")
            continue
        _, o, (ux, uy) = millor
        for yy in range(y0, y0 + h):
            for xx in range(x0, x0 + w):
                m.g[yy][xx] = OFI[ofici][o]
                ocupat.add((xx, yy))
        m.g[uy][ux] = umbral
        porta["position"] = {"x": ux, "y": uy}
        fets += 1
    return ocupat, fets


def cases_kit(m, familia_de, ocupat, per_defecte):
    """Cada casa (tiles vells de familia_de) passa a la seva familia del kit, reduida i girada."""
    fets, perduts = 0, 0
    gids = set(familia_de)
    for gid, cel in sorted(components(m, gids), key=lambda c: (min(p[1] for p in c[1]), min(p[0] for p in c[1]))):
        fam = familia_de[gid]
        prop = veins(ocupat) | ocupat
        lliures = [c for c in cel if c not in prop]
        r = rect_maxim(lliures, mides_mapa(fam)) if lliures else None
        sol = sol_del_voltant(m, cel, per_defecte)
        for (x, y) in cel:
            m.g[y][x] = sol
        if r is None:
            perduts += 1
            continue
        x0, y0, w, h = r
        opcions = [o for o in range(4) if canonica(w, h, o) in FAMILIES[fam]["mides"]]
        o = max(opcions, key=lambda o: sum(1 for c in costat(x0, y0, w, h, o) if m.dentro(*c) and m.libre(*c)) + PREFERENCIA[o])
        for yy in range(y0, y0 + h):
            for xx in range(x0, x0 + w):
                m.g[yy][xx] = FAM[fam][o]
                ocupat.add((xx, yy))
        fets += 1
    return fets, perduts


def kit_ciutat(m, familia_de, umbral, per_defecte):
    ocupat, locs = locals_kit(m, umbral)
    cases, perdudes = cases_kit(m, familia_de, ocupat, per_defecte)
    print(f"  kit: {locs} locals, {cases} cases" + (f" ({perdudes} massa petites, ara pati)" if perdudes else ""))


def revisa_kit(m, per_defecte=None):
    """Despres de conectar: cada empremta del kit ha de ser un rectangle que el cataleg sap
    dibuixar. Una casa que conectar ha foradat es torna a reduir (les cel·les que sobren,
    pati); un local trencat es retorna com a error."""
    for gid, cel in components(m, set(FAM_DE_GID)):
        fam = FAM_DE_GID[gid]
        x0, y0, w, h = caixa(cel)
        o = FAM[fam].index(gid)
        if canonica(w, h, o) in FAMILIES[fam]["mides"] and len(cel) == w * h:
            continue
        sol = sol_del_voltant(m, cel, per_defecte if per_defecte is not None else m.g[y0][x0])
        for (x, y) in cel:
            m.g[y][x] = sol
        r = rect_maxim(cel, mides_mapa(fam))
        if r is None:
            continue
        x0, y0, w, h = r
        opcions = [o for o in range(4) if canonica(w, h, o) in FAMILIES[fam]["mides"]]
        o = max(opcions, key=lambda o: sum(1 for c in costat(x0, y0, w, h, o) if m.dentro(*c) and m.libre(*c)) + PREFERENCIA[o])
        for yy in range(y0, y0 + h):
            for xx in range(x0, x0 + w):
                m.g[yy][xx] = FAM[fam][o]
    errors = []
    for gid, cel in components(m, KIT):
        x0, y0, w, h = caixa(cel)
        o = FAM[FAM_DE_GID[gid]].index(gid) if gid in FAM_DE_GID else OFI[OFI_DE_GID[gid]].index(gid)
        if gid in FAM_DE_GID:
            ok = canonica(w, h, o) in FAMILIES[FAM_DE_GID[gid]]["mides"] and len(cel) == w * h
        else:
            ok = canonica(w, h, o) in mides_ofici(OFI_DE_GID[gid]) and len(cel) >= w * h - 1
        if not ok:
            errors.append((FAM_DE_GID.get(gid) or OFI_DE_GID.get(gid), o, (x0, y0, w, h), len(cel)))
    return errors
