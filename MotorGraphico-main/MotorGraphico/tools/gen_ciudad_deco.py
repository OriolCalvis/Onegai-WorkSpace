"""DECO-01 — El decorat del kit al generador (tools/iso/decorats.py, fulla
kit_decorats): l'ultim pas abans d'escriure el nivell.

  - Els decorats amb cara de davant (bancs, parades, carros, tanques, pou) es
    giren cap a on miren: el punt que ha dit el barri (`_mira` a l'objecte: el
    centre de la plaça, el riu) o, si no n'ha dit cap, el costat amb mes carrer
    lliure davant. L'id passa a <id>_o1.._o3 (o0, de cara a +y, es l'id de sempre).
  - La roba estesa en cordes de diverses cel·les seguides (al llarg de x) nomes
    porta pal als extrems: tendedero_ini, _mig/_mig2 i _fi; una cel·la sola es
    el tendedero de sempre, amb els dos pals.
  - VEG-01: els arbres, les mates i les flors alternen el dibuix (<id>_b) segons
    la cel·la, perque un jardi no sigui el mateix arbre repetit.
"""
import json
import os
import random

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
_M = json.load(open(BASE + "assets/textures/kit_decorats.json"))
ORIENTABLES = set(_M["orientables"])
TRAMS = {k: set(v) for k, v in _M["tramos"].items()}
VARIANTS = {k: list(v) for k, v in json.load(open(BASE + "assets/textures/kit_vegetacio.json"))["variantes"].items()}
CAP = {0: (0, 1), 1: (-1, 0), 2: (0, -1), 3: (1, 0)}    # cap on mira la cara de davant a cada orientacio
PREFERENCIA = {0: 0.3, 3: 0.2, 1: 0.1, 2: 0.0}            # a igualtat, la cara que es veu


def variant_de(oid):
    """L'id de base d'una variant del kit (banco_plaza_o2 -> banco_plaza), o None."""
    base, _, sufix = oid.rpartition("_")
    if base in ORIENTABLES and sufix in ("o1", "o2", "o3"):
        return base
    if base in TRAMS and sufix in TRAMS[base]:
        return base
    if base in VARIANTS and sufix in VARIANTS[base]:
        return base
    return None


def orientacio_cap(x, y, tx, ty):
    """L'orientacio que mira mes de dret cap a (tx, ty)."""
    return max(CAP, key=lambda o: (CAP[o][0] * (tx - x) + CAP[o][1] * (ty - y), PREFERENCIA[o]))


def orientacio_oberta(m, x, y, ocupades):
    """L'orientacio amb mes carrer lliure davant (tres cel·les de fons, tres
    d'ample, les de prop compten mes); la cel·la de davant ha de ser lliure."""
    def lliure(cx, cy):
        return m.libre(cx, cy) and (cx, cy) not in ocupades

    def punts(o):
        dx, dy = CAP[o]
        if not lliure(x + dx, y + dy):
            return -1.0
        return sum((4 - k) * (1.0 if s == 0 else 0.5)
                   for k in (1, 2, 3) for s in (-1, 0, 1)
                   if lliure(x + k * dx + s * dy, y + k * dy + s * dx)) + PREFERENCIA[o]
    return max(CAP, key=punts)


def orientar(m):
    """Gira cada decorat orientable del nivell (i treu la pista `_mira`)."""
    ocupades = {(o["position"]["x"], o["position"]["y"]) for o in m.objetos}
    girats = 0
    for o in m.objetos:
        mira = o.pop("_mira", None)
        if o["objectId"] not in ORIENTABLES:
            continue
        x, y = o["position"]["x"], o["position"]["y"]
        ori = orientacio_cap(x, y, *mira) if mira else orientacio_oberta(m, x, y, ocupades - {(x, y)})
        if ori:
            o["objectId"] = f"{o['objectId']}_o{ori}"
            girats += 1
    return girats


def variar(m):
    """Cada arbre, mata o clap de flors pren una de les seves variants segons la
    cel·la (sempre la mateixa per a la mateixa cel·la)."""
    canviats = 0
    for o in m.objetos:
        base = o["objectId"]
        if base not in VARIANTS:
            continue
        x, y = o["position"]["x"], o["position"]["y"]
        tria = ([base] + [f"{base}_{v}" for v in VARIANTS[base]])[((x * 73856093) ^ (y * 19349663)) % (1 + len(VARIANTS[base]))]
        if tria != base:
            o["objectId"] = tria
            canviats += 1
    return canviats


def cordes(m, oid="tendedero"):
    """Les cordes de roba de diverses cel·les: pal nomes als extrems."""
    fila = {}
    for o in m.objetos:
        if o["objectId"] == oid:
            fila.setdefault(o["position"]["y"], {})[o["position"]["x"]] = o
    trams = 0
    for cel in fila.values():
        for x, o in cel.items():
            abans, despres = (x - 1) in cel, (x + 1) in cel
            if abans and despres:
                o["objectId"] = f"{oid}_mig" if x % 2 == 0 else f"{oid}_mig2"
            elif despres:
                o["objectId"] = f"{oid}_ini"
            elif abans:
                o["objectId"] = f"{oid}_fi"
            else:
                continue
            trams += 1
    return trams


# ---------- decorats que es posen quan el mapa ja és el definitiu ----------
# (després del kit: les cases ja tenen la mida final i les portes dels locals,
# el seu lloc; ho criden els barris amb m.al_final)

_BLOC = None


def bloquejants():
    """Els ids que fan nosa per sempre (catàlegs del projecte); els PNJ i les
    mascotes no compten: es mouen."""
    global _BLOC
    if _BLOC is None:
        _BLOC = set()
        for c in json.load(open(BASE + "assets/proyectos/boundington.json"))["catalogs"]:
            ruta = BASE + "assets/objects/" + c
            if os.path.exists(ruta):
                for o in json.load(open(ruta)).get("objects", []):
                    if o.get("blocksMovement") and o.get("category") not in ("npc", "enemy"):
                        _BLOC.add(o["id"])
    return _BLOC


def _tapades(m):
    return {(o["position"]["x"], o["position"]["y"]) for o in m.objetos if o["objectId"] in bloquejants()}


def _ocupades(m):
    return {(o["position"]["x"], o["position"]["y"]) for o in m.objetos}


def simple(m, x, y, tapades):
    """Posar-hi una cosa que fa nosa no talla cap pas: les cel·les veïnes on es
    pot anar queden unides pel voltant (la prova local de punt simple)."""
    anell = [(-1, -1), (0, -1), (1, -1), (1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0)]
    obert = {(dx, dy) for dx, dy in anell if m.libre(x + dx, y + dy) and (x + dx, y + dy) not in tapades}
    veines = [c for c in ((0, -1), (1, 0), (0, 1), (-1, 0)) if c in obert]
    if not veines:
        return False
    vist, pila = {veines[0]}, [veines[0]]
    while pila:
        cx, cy = pila.pop()
        for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
            if (nx, ny) in obert and (nx, ny) not in vist:
                vist.add((nx, ny))
                pila.append((nx, ny))
    return all(c in vist for c in veines)


def al_carrer(m, zona, oid, quants, separacio, llavor, sols, umbral, muralla=None, prefixos=("farola", "fanal"), lluny_porta=0):
    """Fanals (o el que sigui) arrambats a la paret d'una casa o a l'aigua (no a
    la muralla: el carrer de ronda no el veu ningú), al llarg del carrer: la
    cel·la de davant i les dels costats queden lliures, mai a tocar d'un
    llindar, i a `separacio` (Manhattan) dels altres llums. AV15-03b: amb `lluny_porta` > 0, tampoc a aquesta
    distancia (Manhattan) o menys d'una porta: l'entrada es veu i s'hi arriba recte."""
    rnd = random.Random(llavor)
    x0, y0, x1, y1 = zona
    ocupades, tapades = _ocupades(m), _tapades(m)
    llums = [(o["position"]["x"], o["position"]["y"]) for o in m.objetos if o["objectId"].startswith(prefixos)]
    portes = [(o["position"]["x"], o["position"]["y"]) for o in m.objetos if o.get("targetLevel")] if lluny_porta else []

    def lliure(cx, cy):
        return m.libre(cx, cy) and (cx, cy) not in tapades

    cands = []
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if (x, y) in ocupades or not m.libre(x, y) or m.g[y][x] not in sols:
                continue
            if any(abs(x - a) + abs(y - b) <= lluny_porta for a, b in portes):
                continue
            if any(m.dentro(x + dx, y + dy) and m.g[y + dy][x + dx] == umbral for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
                continue
            murs = [(dx, dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)) if not m.libre(x + dx, y + dy)]
            if len(murs) != 1:
                continue
            dx, dy = murs[0]
            if muralla is not None and m.g[y + dy][x + dx] == muralla:
                continue
            if lliure(x - dx, y - dy) and lliure(x + dy, y + dx) and lliure(x - dy, y - dx):
                cands.append((x, y))
    rnd.shuffle(cands)
    posats = []
    for (x, y) in cands:
        if len(posats) >= quants:
            break
        if any(abs(x - a) + abs(y - b) < separacio for a, b in llums) or not simple(m, x, y, tapades):
            continue
        m.objeto(oid, x, y)
        tapades.add((x, y))
        llums.append((x, y))
        posats.append((x, y))
    return posats


def parades_porta(m, porta_id, oid="parada_mercado", lluny=2):
    """Dues parades a banda i banda de la porta d'un mercat, de cara al carrer;
    la cel·la de davant de la porta queda lliure."""
    porta = next((o for o in m.objetos if o["objectId"] == porta_id), None)
    if porta is None:
        return []
    px, py = porta["position"]["x"], porta["position"]["y"]
    tapades, ocupades = _tapades(m), _ocupades(m)
    davant = [(dx, dy) for dx, dy in ((0, 1), (1, 0), (0, -1), (-1, 0)) if m.libre(px + dx, py + dy)]
    if not davant:
        return []
    dx, dy = max(davant, key=lambda d: sum(m.libre(px + k * d[0], py + k * d[1]) for k in (1, 2, 3)))
    posades = []
    for s in (-1, 1):
        x, y = px + dx + s * lluny * dy, py + dy + s * lluny * dx
        if not m.libre(x, y) or (x, y) in ocupades or not simple(m, x, y, tapades):
            continue
        o = m.objeto(oid, x, y)
        o["_mira"] = (x + 4 * dx, y + 4 * dy)
        tapades.add((x, y))
        posades.append((x, y))
    return posades


def ropa_entre_casas(m, max_cordes, preferir=None, oid="tendedero", separacio=4):
    """AV15-03b (RUIDO_CIUDAD_2) — La roba, nomes de casa a casa i junta on el barri te anima.
    1. Es queden les cordes (cel·les seguides al llarg de x) amb els DOS extrems lligats a una paret (la cel·la
       d'abans i la de despres no son de pas); les que pengen enmig d'un pati o d'un carrer ample, sense res on
       lligar-les, eren soroll.
    2. Si en queden menys de `max_cordes`, se n'estenen de noves dins de `preferir` (x0, y0, x1, y1: el carrer
       amb anima) als passos estrets de 1 a 3 cel·les entre dues parets, lluny de les portes i a `separacio`
       (Manhattan) de les altres cordes.
    Torna (cordes que hi havia, cordes que queden, cel·les tretes, cel·les posades)."""
    fila = {}
    for o in m.objetos:
        if o["objectId"] == oid:
            fila.setdefault(o["position"]["y"], {})[o["position"]["x"]] = o
    cordes_ = []
    for y, cel in fila.items():
        for x in sorted(cel):
            if (x - 1) in cel:
                continue
            x1 = x
            while (x1 + 1) in cel:
                x1 += 1
            cordes_.append((x, x1, y))

    def dins(c):
        return preferir is not None and preferir[0] <= c[0] and c[1] <= preferir[2] and preferir[1] <= c[2] <= preferir[3]

    def lligada(c):
        return not m.libre(c[0] - 1, c[2]) and not m.libre(c[1] + 1, c[2])
    lligades = sorted((c for c in cordes_ if lligada(c)), key=lambda c: (not dins(c), c[2], c[0]))[:max_cordes]
    queden = {(x, c[2]) for c in lligades for x in range(c[0], c[1] + 1)}
    abans = len(m.objetos)
    m.objetos = [o for o in m.objetos if o["objectId"] != oid or (o["position"]["x"], o["position"]["y"]) in queden]
    tretes = abans - len(m.objetos)
    posades = 0
    if preferir is not None and len(lligades) < max_cordes:
        portes = [(o["position"]["x"], o["position"]["y"]) for o in m.objetos if o.get("targetLevel")]
        ocupades = _ocupades(m)
        x0, y0, x1, y1 = preferir
        cands = []
        for y in range(y0, y1 + 1):
            x = x0
            while x <= x1:
                if m.libre(x, y) and not m.libre(x - 1, y):
                    xf = x
                    while xf + 1 <= x1 and m.libre(xf + 1, y):
                        xf += 1
                    if xf - x < 3 and not m.libre(xf + 1, y):
                        cands.append((x, xf, y))
                    x = xf + 1
                else:
                    x += 1
        cands.sort(key=lambda c: ((c[0] * 7349 + c[2] * 3571) % 101, c[2], c[0]))   # repartides, sense atzar
        fetes = list(lligades)
        for c in cands:
            if len(fetes) >= max_cordes:
                break
            cel = [(x, c[2]) for x in range(c[0], c[1] + 1)]
            if any(p in ocupades for p in cel):
                continue
            if any(abs(px - x) + abs(py - y) <= 2 for px, py in portes for x, y in cel):
                continue
            if any(min(abs(x - fx) for fx in range(f[0], f[1] + 1)) + abs(c[2] - f[2]) < separacio for f in fetes for x, _ in cel):
                continue
            for x, y in cel:
                m.objeto(oid, x, y)
                ocupades.add((x, y))
                posades += 1
            fetes.append(c)
    return len(cordes_), min(len(lligades), max_cordes), tretes, posades
