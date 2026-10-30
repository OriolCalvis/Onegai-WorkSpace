#!/usr/bin/env python3
"""INT-01 — Els interiors de Boundington amb el kit: sales d'Habbo (les dues
parets del fons, el davant obert) amb el terra i els mobles del kit, de la
mida que toca a cada edifici segons la seva importància; els grans, amb més
d'una planta (cada planta és un nivell i les escales les uneixen).

Per a cada edifici hi ha un pla per planta (tools/interiors_kit/plans*.py): la
mida, l'estil de paret, el terra, les catifes, què hi ha a cada cel·la de
paret, la porta de sortida, els mobles, on es posa cada PNJ i les escales.
El generador:

  - escriu assets/maps/<nivell>.tmx i assets/levels/<nivell>.json (la planta
    baixa conserva el nom de sempre, interior_<edifici>; les altres,
    interior_<edifici>_p1, _p2 o _s1 si és un soterrani);
  - conserva els objectes que ja hi havia (els PNJ, els de les aventures, la
    sortida amb el seu enllaç) i els posa on diu el pla o a prop d'on eren;
  - torna a apuntar les portes de la ciutat a la nova entrada (la celda de
    l'estora de la sortida);
  - dona d'alta les plantes noves al projecte, el catàleg de les escales
    (assets/objects/interiores_kit.json) i el dibuix de les sortides i les
    escales (àlies de sprites.json);
  - comprova que de l'entrada s'arriba a tot arreu (PNJ, escales, sortida).

    python3 tools/gen_interiores_kit.py            # tots els que tenen pla
    python3 tools/gen_interiores_kit.py taberna    # només un
    python3 tools/gen_interiores_kit.py en         # INT-02: els 33 de l'est (en_<assentament>_<local>)
    python3 tools/gen_interiores_kit.py en_grytoz  # els d'un assentament
"""
import glob
import json
import os
import re
import sys
from collections import deque

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"
sys.path.insert(0, BASE + "tools/iso")
sys.path.insert(0, BASE + "tools")
from peces_grans import PECES  # noqa: E402
from sales import ALT, ALT_SALA, ids_parets  # noqa: E402

_IDX = json.load(open(BASE + "assets/textures/terreno_iso.json"))
T = {t["nombre"]: t["gid"] for t in _IDX["tiles"]}
COLISION = set(_IDX["colisionGids"])
PARETS = ids_parets()
GIDS_PECA = {T[f"int_peca_{n}"] for n in PECES if f"int_peca_{n}" in T}
CATALEG_KIT = json.load(open(BASE + "assets/objects/mobles_kit.json"))["objects"]
NOSA_KIT = {o["id"]: o["blocksMovement"] for o in CATALEG_KIT}
CATALEG = "interiores_kit.json"
NIVELLS = BASE + "assets/levels/"
VORES = {"c": (), "xm": ("xm",), "xp": ("xp",), "ym": ("ym",), "yp": ("yp",)}


def edifici_de(nivell):
    """interior_castillo_p1 -> castillo."""
    return re.sub(r"_[ps]\d+$", "", nivell).replace("interior_", "")


def id_escala(nivell, sentit):
    """escalera_<planta on és>_<sube|baja>: escalera_castillo_p0_sube, escalera_castillo_p1_baja..."""
    planta = nivell.replace("interior_", "")
    return f"escalera_{planta}" + ("" if re.search(r"_[ps]\d+$", planta) else "_p0") + f"_{sentit}"


def es_escala_de(oid, edificis):
    """Si oid és una escala (amb les ids d'ara o les d'abans) d'algun d'aquests edificis."""
    return any(re.fullmatch(rf"escalera_{e}_[ps]\d+(_sube|_baja)?", oid) for e in edificis)


def _catalegs_npc():
    """Els ids que són PNJ (per saber què es conserva i què fa nosa)."""
    out = {}
    projecte = json.load(open(BASE + "assets/proyectos/boundington.json"))
    for c in projecte["catalogs"]:
        ruta = BASE + "assets/objects/" + c
        if os.path.exists(ruta):
            for o in json.load(open(ruta)).get("objects", []):
                out[o["id"]] = o
    return out


DEFS = _catalegs_npc()


class Sala:
    """Una planta: W x H cel·les; la fila y = 0 i la columna x = 0 són la paret."""

    def __init__(self, nivell, nom, W, H, estil, terra, alt=ALT):
        self.nivell, self.nom, self.W, self.H = nivell, nom, W, H
        self.estil, self.alt = estil, alt
        self.g = [[0] * W for _ in range(H)]
        for y in range(1, H):
            for x in range(1, W):
                self.g[y][x] = T[terra]
        self.obj = {}                       # (x, y) -> objecte del nivell
        self.decos_y, self.decos_x = {}, {}
        self.pnj = {}                       # id -> (x, y)
        self.entrada = None
        self.escales = []                   # (id, (x, y), nivell destí, celda destí)
        self.sortida = None                 # (costat, posició)
        self.lliures = set()                # cel·les que han de quedar lliures (passos)

    # ------------------------------------------------------------ el terra
    def terra(self, x0, y0, x1, y1, nom):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.g[y][x] = T[nom]

    def catifa(self, celles, color="vermella"):
        """Catifa sobre un conjunt de cel·les: cada cel·la pren la vora dels
        costats on la catifa s'acaba (nou peces)."""
        cs = set(celles)
        for (x, y) in cs:
            parts = []
            if (x - 1, y) not in cs:
                parts.append("xm")
            if (x + 1, y) not in cs:
                parts.append("xp")
            if (x, y - 1) not in cs:
                parts.append("ym")
            if (x, y + 1) not in cs:
                parts.append("yp")
            if len(parts) > 2 or ("xm" in parts and "xp" in parts) or ("ym" in parts and "yp" in parts):
                raise SystemExit(f"{self.nivell}: catifa massa estreta a {(x, y)} ({parts})")
            self.g[y][x] = T[f"int_catifa_{color}_{'_'.join(parts) or 'c'}"]

    def rect_catifa(self, x0, y0, x1, y1, color="vermella"):
        self.catifa([(x, y) for x in range(x0, x1 + 1) for y in range(y0, y1 + 1)], color)

    # ------------------------------------------------------------ objectes
    def posa(self, oid, x, y, o=0, **camps):
        if not (1 <= x < self.W and 1 <= y < self.H):
            raise SystemExit(f"{self.nivell}: {oid} fora de la sala a {(x, y)}")
        if (x, y) in self.obj:
            raise SystemExit(f"{self.nivell}: {oid} a {(x, y)}, que ja té {self.obj[(x, y)]['objectId']}")
        if self.g[y][x] in GIDS_PECA:
            raise SystemExit(f"{self.nivell}: {oid} a {(x, y)}, sobre una peça gran")
        nom = oid if o == 0 else f"{oid}_o{o}"
        self.obj[(x, y)] = {"objectId": nom, "position": {"x": x, "y": y}, **camps}
        return self.obj[(x, y)]

    def mobles(self, llista):
        for m in llista:
            self.posa(*m)

    def tira(self, base, x, y, n, o=0, variants=None, substituts=None):
        """n trams d'un moble llarg des de (x, y): al llarg de x (o0, o2) o de y
        (o1, o3); base_ini a la punta -X canònica (o0: x baixa, o1: y baixa,
        o2: x alta, o3: y alta), base_mig, base_fi; un de sol, base_sol.
        variants: {posició: altra base} (un tram amb gerres, amb ampolles...);
        substituts: {posició: id sencer} (un banc trencat enmig de la filera)."""
        dx, dy = (1, 0) if o in (0, 2) else (0, 1)
        cel = [(x + dx * k, y + dy * k) for k in range(n)]
        if o in (2, 3):
            cel = cel[::-1]
        for k, (cx, cy) in enumerate(cel):
            tram = "sol" if n == 1 else ("ini" if k == 0 else ("fi" if k == n - 1 else "mig"))
            if k in (substituts or {}):
                self.posa(substituts[k], cx, cy, o)
            else:
                self.posa(f"{(variants or {}).get(k, base)}_{tram}", cx, cy, o)

    def llit(self, base, x, y, o=0):
        """Un llit (o qualsevol moble de cap i peus): el capçal a (x, y) i els peus
        a la cel·la del davant segons l'orientació (o0 +y, o3 +x, o2 -y, o1 -x)."""
        d = {0: (0, 1), 3: (1, 0), 2: (0, -1), 1: (-1, 0)}[o]
        self.posa(f"{base}_cap", x, y, o)
        self.posa(f"{base}_peus", x + d[0], y + d[1], o)

    def peca(self, nom, x, y):
        """Una peça gran (tools/iso/peces_grans.py): la seva huella de rajoles
        int_peca_<nom>, que fa nosa, des de (x, y)."""
        (w, d, _), _ = PECES[nom]
        for yy in range(y, y + d):
            for xx in range(x, x + w):
                if not (1 <= xx < self.W and 1 <= yy < self.H) or (xx, yy) in self.obj:
                    raise SystemExit(f"{self.nivell}: la peça {nom} no cap a {(xx, yy)}")
                self.g[yy][xx] = T[f"int_peca_{nom}"]

    def paret_y(self, **decos):
        """Decoracions de la paret de la fila y = 0: x=deco (claus x1, x2...)."""
        self.decos_y.update({int(k[1:]): v for k, v in decos.items()})

    def paret_x(self, **decos):
        self.decos_x.update({int(k[1:]): v for k, v in decos.items()})

    def porta(self, costat, pos):
        """La porta de sortida: a la paret x (columna, pos = y) o y (fila, pos = x)."""
        self.sortida = (costat, pos)
        if costat == "x":
            self.decos_x[pos] = "porta"
            self.entrada = (1, pos)
        else:
            self.decos_y[pos] = "porta"
            self.entrada = (pos, 1)

    def escala(self, sentit, x, y, o, desti, celda_desti, pedra=False):
        """Escala que puja (sube) o baixa (baja): la cel·la on es pitja és (x, y);
        el tram de la paret és la del costat de la paret (o0: y - 1, o3: x - 1).
        pedra: l'escala que puja, de pedra (cripta, fossats, cambra cuirassada)."""
        dx, dy = (0, -1) if o == 0 else (-1, 0)
        # Una id per escala (la planta on és i cap on va): una planta del mig en té dues,
        # i cada una es dibuixa diferent (el peu de la que puja, el cap de la que baixa).
        oid = id_escala(self.nivell, sentit)
        base = "escalera_piedra" if (pedra and sentit == "sube") else f"escalera_{sentit}"
        self.escales.append((oid, (x, y), desti, celda_desti, sentit, o, base))
        alt = "alto" if sentit == "sube" else "fondo"
        self.posa(f"{base}_{alt}", x + dx, y + dy, o)
        self.posa(oid, x, y, targetLevel=f"assets/levels/{desti}.json",
                  targetPosition={"x": celda_desti[0], "y": celda_desti[1]})

    # ------------------------------------------------------------ parets
    def _parets(self):
        sufix = "_alta" if self.alt == ALT_SALA else ""
        base = f"paret_{self.estil}{sufix}"
        out = [{"objectId": f"{base}_cantonada", "position": {"x": 0, "y": 0}}]
        for x in range(1, self.W):
            pid = f"{base}_{self.decos_y.get(x) or 'llisa'}"
            assert pid in PARETS, f"{self.nivell}: no hi ha la paret {pid}"
            out.append({"objectId": pid, "position": {"x": x, "y": 0}})
        for y in range(1, self.H):
            pid = f"{base}_{self.decos_x.get(y) or 'llisa'}"
            assert pid in PARETS, f"{self.nivell}: no hi ha la paret {pid}"
            out.append({"objectId": f"{pid}_o3", "position": {"x": 0, "y": y}})
        return out

    # ------------------------------------------------------------ comprovacions
    def fa_nosa(self, o):
        oid = o["objectId"]
        if oid in NOSA_KIT:
            return NOSA_KIT[oid]
        d = DEFS.get(oid)
        if d is not None:
            return d.get("blocksMovement", False) or d.get("category") == "npc"
        return False

    def transitables(self):
        nosa = {p for p, o in self.obj.items() if self.fa_nosa(o)}
        return {(x, y) for y in range(1, self.H) for x in range(1, self.W)
                if self.g[y][x] not in COLISION and (x, y) not in nosa}

    def inici(self):
        """On apareix el jugador: l'estora de la sortida o, a les plantes de dalt, la primera escala."""
        return self.entrada or self.escales[0][1]

    def mapa(self, marques=()):
        """El pla en text (per als errors): # paret, X fa nosa, . lliure, ? marcada."""
        lliure = self.transitables()
        files = []
        for y in range(self.H):
            fila = ""
            for x in range(self.W):
                if (x, y) in marques:
                    fila += "?"
                elif x == 0 or y == 0:
                    fila += "#"
                elif (x, y) in lliure:
                    fila += "."
                elif (x, y) in self.obj:
                    fila += self.obj[(x, y)]["objectId"][0]
                else:
                    fila += "X"
            files.append(fila)
        return "\n".join(files)

    def comprova(self):
        lliure = self.transitables()
        inici = self.inici()
        assert inici in lliure, f"{self.nivell}: l'entrada {inici} no és transitable"
        vist = {inici}
        cua = deque([inici])
        while cua:
            x, y = cua.popleft()
            for v in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                if v in lliure and v not in vist:
                    vist.add(v)
                    cua.append(v)
        sols = lliure - vist
        assert not sols, f"{self.nivell}: {len(sols)} cel·les tancades {sorted(sols)[:8]}\n{self.mapa(sols)}"
        for oid, (x, y) in self.pnj.items():
            veins = {(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)}
            assert veins & vist, f"{self.nivell}: no s'arriba a parlar amb {oid} a {(x, y)}\n{self.mapa({(x, y)})}"
        for oid, cel, *_ in self.escales:
            assert cel in vist, f"{self.nivell}: no s'arriba a l'escala {oid}"
        return len(vist)

    # ------------------------------------------------------------ escriure
    def objectes(self):
        return self._parets() + [self.obj[p] for p in sorted(self.obj, key=lambda p: (p[1], p[0]))]

    def tmx(self):
        usats = sorted({v for fila in self.g for v in fila} & COLISION)
        tiles_col = "\n".join(f'  <tile id="{gid - 1}">\n   <properties>\n'
                              f'    <property name="collision" type="bool" value="true"/>\n'
                              f'   </properties>\n  </tile>' for gid in usats)
        csv = ",\n".join(",".join(str(v) for v in fila) for fila in self.g)
        return f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por tools/gen_interiores_kit.py (INT-01) - no editar a mano.
     {self.nom}. La fila 0 y la columna 0 son las paredes (objetos paret_*): sin baldosa. -->
<map version="1.10" orientation="orthogonal" renderorder="right-down" width="{self.W}" height="{self.H}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="{_IDX['nombre']}" tilewidth="{_IDX['celda']['ancho']}" tileheight="{_IDX['celda']['alto']}" tilecount="{_IDX['tileCountTotal']}" columns="{_IDX['columnas']}">
  <image source="../textures/{_IDX['imagen']}" width="{_IDX['anchoTotal']}" height="{_IDX['altoTotal']}"/>
{tiles_col}
 </tileset>
 <layer id="1" name="suelo" width="{self.W}" height="{self.H}">
  <data encoding="csv">
{csv}
</data>
 </layer>
</map>
'''


# ---------------------------------------------------------------- l'edifici sencer
def objectes_vells(nivell):
    ruta = NIVELLS + nivell + ".json"
    if not os.path.exists(ruta):
        return []
    return json.load(open(ruta)).get("objects", [])


def es_generat(o):
    """El que posa el generador (parets, mobles, escales) es torna a fer cada cop;
    es conserven els PNJ, els enemics, els objectes per agafar i les sortides
    (abans d'INT-01 els interiors no tenien res més)."""
    oid = o["objectId"]
    if oid.startswith("salida_"):
        return False
    if oid.startswith("escalera_") or oid in NOSA_KIT:
        return True
    return DEFS.get(oid, {}).get("category") not in ("npc", "enemy", "pickup")


def munta(edifici, plantes):
    """plantes: [(nivell, fer_sala)]; cada fer_sala() -> Sala amb el pla fet.
    Retorna les sales, amb els objectes vells ja recol·locats."""
    sales = []
    for nivell, fer in plantes:
        s = fer()
        vells = [o for o in objectes_vells(nivell) if not es_generat(o)]
        for o in vells:
            oid = o["objectId"]
            if oid.startswith("salida_"):
                if s.entrada is None:
                    raise SystemExit(f"{nivell}: hi havia la sortida {oid} però el pla no té porta")
                s.posa(oid, *s.entrada, **{k: v for k, v in o.items() if k not in ("objectId", "position")})
                continue
            extra = {k: v for k, v in o.items() if k not in ("objectId", "position")}
            if oid in s.pnj:
                s.posa(oid, *s.pnj[oid], **extra)
                continue
            # Sense lloc al pla: on era, si hi cap; si no, la cel·la lliure més propera al centre.
            lliures = s.transitables() - {s.inici()} - {c for _, c, *_ in s.escales}
            x, y = o["position"]["x"], o["position"]["y"]
            if (x, y) not in lliures:
                cx, cy = s.W // 2, s.H // 2
                x, y = min(lliures, key=lambda c: (abs(c[0] - cx) + abs(c[1] - cy), c))
            s.posa(oid, x, y, **extra)
            s.pnj[oid] = (x, y)
        for oid, cel in s.pnj.items():
            if s.obj.get(cel, {}).get("objectId") != oid:
                print(f"  avís: {nivell}: el pla posa {oid}, que no hi era (no l'afegeixo)")
        if s.entrada and not any(o["objectId"].startswith("salida_") for o in s.obj.values()):
            raise SystemExit(f"{nivell}: el pla té porta però no hi havia cap sortida")
        sales.append(s)
    return sales


def escriu(sala):
    sense = sorted({o["objectId"] for o in sala.objectes()} - set(NOSA_KIT) - set(DEFS)
                   - {e[0] for e in sala.escales})
    if sense:
        raise SystemExit(f"{sala.nivell}: objectes que no són a cap catàleg (cal tornar a exportar els mobles? "
                         f"python3 tools/iso/fer_interiors.py): {sense[:8]}")
    open(BASE + f"assets/maps/{sala.nivell}.tmx", "w").write(sala.tmx())
    inici = sala.inici()
    nivell = {"name": sala.nom, "map": f"assets/maps/{sala.nivell}.tmx",
              "playerStart": {"x": inici[0], "y": inici[1]}, "objects": sala.objectes()}
    open(NIVELLS + f"{sala.nivell}.json", "w").write(json.dumps(nivell, indent=1, ensure_ascii=False) + "\n")


def reapunta_portes(interior, entrada):
    """Les portes de fora (la ciutat) que porten a la planta baixa, a l'estora nova."""
    desti = f"assets/levels/{interior}.json"
    canvis = 0
    for ruta in sorted(glob.glob(NIVELLS + "*.json")):
        if os.path.basename(ruta).startswith(interior):
            continue
        text = open(ruta).read()
        if desti not in text:
            continue
        nivell = json.loads(text)
        tocat = False
        for o in nivell.get("objects", []):
            if o.get("targetLevel") == desti and o.get("targetPosition") != {"x": entrada[0], "y": entrada[1]}:
                o["targetPosition"] = {"x": entrada[0], "y": entrada[1]}
                tocat = True
        if tocat:
            open(ruta, "w").write(json.dumps(nivell, indent=1, ensure_ascii=False) + "\n")
            canvis += 1
    return canvis


def dona_d_alta(sales):
    """Les plantes noves a les llistes del projecte (edició de text, al costat de la planta baixa)."""
    ruta = BASE + "assets/proyectos/boundington.json"
    text = open(ruta).read()
    projecte = json.loads(text)
    for s in sales:
        base = re.sub(r"_[ps]\d+$", "", s.nivell)
        for llista, ext in (("levels", "json"), ("maps", "tmx")):
            nom = f"{s.nivell}.{ext}"
            if nom in projecte[llista]:
                continue
            # després de la darrera línia de la llista que comença pel nom de l'edifici
            # (la planta baixa o una planta que ja hi era): les plantes queden en ordre
            linies = text.split("\n")
            fet = [i for i, l in enumerate(linies) if l.startswith(f'    "{base}') and l.rstrip(",").endswith(f'.{ext}"')]
            assert fet, f"boundington.json: no trobo {base}.{ext}"
            linies.insert(fet[-1] + 1, f'    "{nom}",')
            text = "\n".join(linies)
            projecte[llista].append(nom)
    open(ruta, "w").write(text)


def cataleg_i_alies(totes):
    """El catàleg de les escales i els àlies del dibuix (sortides i escales)."""
    objectes, alies = [], {}
    for s in totes:
        for (x, y), o in s.obj.items():
            oid = o["objectId"]
            if oid.startswith("salida_"):
                alies[oid] = "felpudo"
        for oid, cel, desti, _, sentit, o, base in s.escales:
            dibuix = f"{base}_{'base' if sentit == 'sube' else 'alto'}" + (f"_o{o}" if o else "")
            alies[oid] = dibuix
            objectes.append({"id": oid, "name": "Escalera " + ("arriba" if sentit == "sube" else "abajo"),
                             "category": "prop", "spriteId": 23, "blocksMovement": False, "interactable": True})
    # Les escales dels edificis que es tornen a fer surten de nou (les dels altres es queden).
    edificis = {edifici_de(s.nivell) for s in totes}
    ruta = BASE + "assets/objects/" + CATALEG
    vells = json.load(open(ruta))["objects"] if os.path.exists(ruta) else []
    per_id = {o["id"]: o for o in vells if not es_escala_de(o["id"], edificis)}
    per_id.update({o["id"]: o for o in objectes})
    open(ruta, "w").write(json.dumps({
        "_fuente": "INT-01 — las escaleras entre las plantas de los interiores (tools/gen_interiores_kit.py, no editar "
                   "a mano): escalera_<planta donde esta>_<sube|baja>, con targetLevel en el nivel. El dibujo, por "
                   "alias en sprites.json.",
        "objects": [per_id[k] for k in sorted(per_id)]}, ensure_ascii=False, indent=1) + "\n")
    texto = open(BASE + "assets/proyectos/boundington.json").read()
    if CATALEG not in json.loads(texto)["catalogs"]:
        marca = '    "mobles_kit.json",\n'
        assert marca in texto, "boundington.json: no trobo mobles_kit.json a la llista de catàlegs"
        open(BASE + "assets/proyectos/boundington.json", "w").write(texto.replace(marca, marca + f'    "{CATALEG}",\n', 1))
    registre = json.load(open(BASE + "assets/textures/sprites.json"))
    nou = {k: v for k, v in registre["alias"].items() if not es_escala_de(k, edificis)}
    nou.update(alies)
    if nou != registre["alias"]:
        registre["alias"] = nou
        open(BASE + "assets/textures/sprites.json", "w").write(json.dumps(registre, ensure_ascii=False, indent=2) + "\n")
    return len(objectes), len(alies)


def main(filtre=None):
    from interiors_kit import EDIFICIS
    totes = []
    for edifici, plantes in EDIFICIS.items():
        if filtre and not any(edifici == f or edifici.startswith(f + "_") for f in filtre):
            continue
        sales = munta(edifici, plantes)
        for s in sales:
            n = s.comprova()
            escriu(s)
            print(f"  {s.nivell:28} {s.W - 1}x{s.H - 1} {s.estil:12} {len(s.obj):3} objectes  {n} cel·les a l'abast")
        portes = reapunta_portes(sales[0].nivell, sales[0].entrada)
        if portes:
            print(f"  {sales[0].nivell}: {portes} nivells amb la porta reapuntada a {sales[0].entrada}")
        dona_d_alta(sales[1:])
        totes += sales
    escales, alies = cataleg_i_alies(totes)
    print(f"{len(totes)} plantes; {escales} escales al catàleg {CATALEG}; {alies} àlies de dibuix")


if __name__ == "__main__":
    main(set(sys.argv[1:]) or None)
