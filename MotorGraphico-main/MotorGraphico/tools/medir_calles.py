#!/usr/bin/env python3
"""AV15-03 - medir el ruido de una ciudad (solo lectura, no toca nada).

Uso:  MG=<raiz de MotorGraphico> python3 medir_calles.py [nivel ...] [--json salida.json] [--sin-voraz] [--comprobar]
      --comprobar: sale con 1 si hay una puerta tapada o inalcanzable o una calle principal sin carril recto.

Reproduce la regla de colision del juego (GameSession::tryMovePlayer):
  - un tile con collision=true bloquea;
  - un objeto cuyo catalogo dice blocksMovement=true bloquea (NPC incluidos);
  - se anda a 4 direcciones, un paso cada vez.

Mide, por cada origen (inicio del jugador y plaza del Casco Antiguo) y cada
puerta de interior:
  ruta_total   pasos de la ruta mas corta con tiles y objetos (lo que anda el jugador)
  ruta_tiles   lo mismo con solo los tiles (lo que daria la ciudad sin objetos)
  coste_obj    ruta_total - ruta_tiles: el desvio que ponen los OBJETOS
  manhattan    distancia en linea recta a 4 direcciones
Ademas: puertas con la entrada tapada, objetos que estrechan una calle a una
sola celda, y que objetos de decorado (no NPC) quitar para bajar mas el coste.
"""
import json
import os
import sys
from collections import Counter, deque

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D          # mismo cargador, mismo catalogo y mismas calles principales

VECINOS = D.VECINOS
CALLES = D.CALLES


class Ciudad:
    def __init__(self, nombre):
        self.nombre = nombre
        self.cat = D.catalogo()
        self.lvl, self.g, self.col, self.nom = D.cargar_nivel(nombre)
        self.h, self.w = len(self.g), len(self.g[0])
        self.objs = self.lvl["objects"]
        self.quitados = set()      # (x, y) de objetos que se simulan quitados

    def def_(self, o):
        return self.cat.get(o["objectId"], {})

    def bloquea(self, o):
        d = self.def_(o)
        return bool(d.get("blocksMovement")) and not d.get("flotante")

    def es_npc(self, o):
        return self.def_(o).get("category") == "npc"

    def ocupadas(self):
        return {(o["position"]["x"], o["position"]["y"]): o for o in self.objs
                if self.bloquea(o) and (o["position"]["x"], o["position"]["y"]) not in self.quitados}

    def tile_bloquea(self, x, y):
        return not (0 <= x < self.w and 0 <= y < self.h) or self.g[y][x] in self.col

    def bfs(self, origen, con_objetos=True):
        occ = self.ocupadas() if con_objetos else {}
        dist, q = {origen: 0}, deque([origen])
        while q:
            x, y = q.popleft()
            for dx, dy in VECINOS:
                n = (x + dx, y + dy)
                if n in dist or self.tile_bloquea(*n) or n in occ:
                    continue
                dist[n] = dist[(x, y)] + 1
                q.append(n)
        return dist

    def cerca_libre(self, x, y):
        occ = self.ocupadas()
        for r in range(40):
            for dx in range(-r, r + 1):
                for dy in range(-r, r + 1):
                    c = (x + dx, y + dy)
                    if max(abs(dx), abs(dy)) == r and not self.tile_bloquea(*c) and c not in occ:
                        return c

    def puertas(self):
        return [(o["objectId"], (o["position"]["x"], o["position"]["y"])) for o in self.objs
                if o["objectId"].startswith("puerta_") and "/interior_" in o.get("targetLevel", "")]

    def centro_de_la_plaza(self):
        """La plaza del Casco Antiguo (gen_ciudad.centro(): x 23-39, y 43-55, pozo en 31,49): la celda
        libre mas cercana al pozo que tenga al menos tres vecinas libres (no un hueco entre PNJ)."""
        occ = self.ocupadas()
        libre = lambda c: not self.tile_bloquea(*c) and c not in occ
        mejor = None
        for y in range(43, 56):
            for x in range(23, 40):
                if libre((x, y)) and sum(libre((x + dx, y + dy)) for dx, dy in VECINOS) >= 3:
                    k = (abs(x - 31) + abs(y - 49), y, x)
                    if mejor is None or k < mejor[0]:
                        mejor = (k, (x, y))
        return mejor[1]

    def origenes(self):
        ps = (self.lvl["playerStart"]["x"], self.lvl["playerStart"]["y"])
        r = {"inicio del jugador": ps}
        if self.nombre == "ciudad_centro":
            r["plaza del Casco Antiguo"] = self.centro_de_la_plaza()
        return r

    def coste(self):
        """{(origen, puerta): (total, solo_tiles, manhattan)} o None si no se llega."""
        res = {}
        for nom, o in self.origenes().items():
            full, tiles = self.bfs(o), self.bfs(o, False)
            for pid, p in self.puertas():
                m = abs(p[0] - o[0]) + abs(p[1] - o[1])
                res[(nom, pid)] = (full.get(p), tiles.get(p), m)
        return res

    def puertas_tapadas(self):
        """Puertas cuyas celdas de acceso (vecinas andables) estan todas ocupadas por un objeto."""
        occ = self.ocupadas()
        out = []
        for pid, (px, py) in self.puertas():
            acc = [(px + dx, py + dy) for dx, dy in VECINOS if not self.tile_bloquea(px + dx, py + dy)]
            libres = [c for c in acc if c not in occ]
            if not libres:
                out.append({"puerta": pid, "pos": (px, py), "accesos": len(acc),
                            "tapado_por": [f"{occ[c]['objectId']}@{c}" for c in acc if c in occ]})
        return out

    def accesos_con_objeto(self):
        """Puertas con algun objeto que bloquea en una celda de acceso (aunque quede otra libre)."""
        occ = self.ocupadas()
        out = []
        for pid, (px, py) in self.puertas():
            acc = [(px + dx, py + dy) for dx, dy in VECINOS if not self.tile_bloquea(px + dx, py + dy)]
            tapa = [f"{occ[c]['objectId']}@{c}" for c in acc if c in occ]
            if tapa:
                out.append({"puerta": pid, "pos": (px, py), "accesos": len(acc), "tapado_por": tapa})
        return out

    def carriles(self):
        """Por cada calle principal (D.BANDAS): el mejor carril recto de punta a punta."""
        occ = self.ocupadas()
        res = []
        for (bn, ori, a0, a1, e0, e1) in D.BANDAS.get(self.nombre, []):
            mejor = None
            for a in range(a0, a1 + 1):
                celdas = [(a, e) for e in range(e0, e1 + 1)] if ori == "v" else [(e, a) for e in range(e0, e1 + 1)]
                if any(self.tile_bloquea(*c) for c in celdas):
                    continue
                tapa = [f"{occ[c]['objectId']}@{c}" for c in celdas if c in occ]
                if mejor is None or len(tapa) < len(mejor[1]):
                    mejor = (a, tapa)
            res.append({"banda": bn, "carril": None if mejor is None else mejor[0],
                        "recto": mejor is not None and not mejor[1], "estorbos": [] if mejor is None else mejor[1]})
        return res

    def estrechos(self):
        """Objetos que dejan una calle en UNA celda: una celda libre de calle con dos vecinas
        opuestas bloqueadas, y al menos una de ellas es un objeto."""
        occ = self.ocupadas()
        c = Counter()
        vistos = set()
        for y in range(self.h):
            for x in range(self.w):
                if self.tile_bloquea(x, y) or (x, y) in occ or self.nom.get(self.g[y][x]) not in CALLES:
                    continue
                for (a, b) in (((x - 1, y), (x + 1, y)), ((x, y - 1), (x, y + 1))):
                    ba = self.tile_bloquea(*a) or a in occ
                    bb = self.tile_bloquea(*b) or b in occ
                    if ba and bb:
                        for v in (a, b):
                            if v in occ and v not in vistos:
                                vistos.add(v)
                                c[occ[v]["objectId"]] += 1
        return c

    def que_quitar(self, maximo=25):
        """Voraz: de los objetos de decorado (bloquean, no son NPC ni puertas), el que mas pasos
        ahorra a la suma de rutas; hasta que ninguno ahorre nada."""
        orig = list(self.origenes().values())
        puertas = [p for _, p in self.puertas()]

        def suma():
            t = 0
            for o in orig:
                d = self.bfs(o)
                t += sum(d.get(p, 200) for p in puertas)
            return t
        base, plan = suma(), []
        cand = [o for o in self.objs if self.bloquea(o) and not self.es_npc(o) and not o["objectId"].startswith("puerta_")]
        for _ in range(maximo):
            mejor = None
            for o in cand:
                k = (o["position"]["x"], o["position"]["y"])
                if k in self.quitados:
                    continue
                self.quitados.add(k)
                s = suma()
                self.quitados.discard(k)
                if s < base and (mejor is None or s < mejor[0]):
                    mejor = (s, o)
            if mejor is None:
                break
            plan.append({"objeto": mejor[1]["objectId"], "pos": (mejor[1]["position"]["x"], mejor[1]["position"]["y"]),
                         "ahorra": base - mejor[0]})
            self.quitados.add(plan[-1]["pos"])
            base = mejor[0]
        self.quitados.clear()
        return plan


def informe(nombre, voraz=True):
    c = Ciudad(nombre)
    inf = {"nivel": nombre, "origenes": {k: v for k, v in c.origenes().items()}}
    filas = []
    for (nom, pid), (tot, til, man) in c.coste().items():
        filas.append({"origen": nom, "puerta": pid, "total": tot, "tiles": til, "manhattan": man})
    inf["rutas"] = filas
    occ = c.ocupadas()
    calle = lambda o: c.nom.get(c.g[o["position"]["y"]][o["position"]["x"]]) in CALLES
    inv = {}
    for o in c.objs:
        if c.bloquea(o):
            i = inv.setdefault(o["objectId"], {"n": 0, "en_calle": 0, "npc": c.es_npc(o)})
            i["n"] += 1
            i["en_calle"] += calle(o)
    inf["objetos_que_bloquean"] = inv
    inf["puertas_tapadas"] = c.puertas_tapadas()
    inf["accesos_con_objeto"] = c.accesos_con_objeto()
    inf["carriles"] = c.carriles()
    inf["estrechan_una_calle"] = dict(c.estrechos())
    inf["celdas_de_calle"] = sum(1 for y in range(c.h) for x in range(c.w) if c.nom.get(c.g[y][x]) in CALLES)
    inf["celdas_de_calle_con_objeto"] = sum(1 for o in occ.values() if calle(o))
    if voraz:
        inf["quitar_para_ahorrar"] = c.que_quitar()
    return inf


def texto(inf):
    out = [f"== {inf['nivel']} =="]
    por_origen = {}
    for f in inf["rutas"]:
        por_origen.setdefault(f["origen"], []).append(f)
    for nom, fs in por_origen.items():
        ok = [f for f in fs if f["total"] is not None]
        out.append(f"\nDesde {nom} {inf['origenes'][nom]}: {len(ok)}/{len(fs)} puertas alcanzables")
        for f in sorted(fs, key=lambda f: -((f["total"] or 999) - (f["tiles"] or 0))):
            if f["total"] is None:
                out.append(f"  {f['puerta']:<22} NO SE LLEGA (sin objetos: {f['tiles']})")
                continue
            dm = 100 * (f["total"] - f["manhattan"]) / max(f["manhattan"], 1)
            do = f["total"] - f["tiles"]
            out.append(f"  {f['puerta']:<22} ruta {f['total']:>3}  sin objetos {f['tiles']:>3}  recta {f['manhattan']:>3}  "
                       f"desvio {dm:>5.1f}%  de los objetos +{do}")
        if ok:
            st, sm, so = sum(f["total"] for f in ok), sum(f["manhattan"] for f in ok), sum(f["tiles"] for f in ok)
            out.append(f"  TOTAL {st} pasos; recta {sm}; sin objetos {so}.  Desvio {100*(st-sm)/sm:.1f}%; "
                       f"los objetos ponen +{st-so} ({100*(st-so)/so:.1f}%).  <25%: "
                       f"{sum(1 for f in ok if 100*(f['total']-f['manhattan'])/max(f['manhattan'],1) < 25)}/{len(ok)}")
    out.append(f"\nPuertas con la entrada tapada por un objeto: {inf['puertas_tapadas'] or 'ninguna'}")
    out.append(f"Puertas con un objeto en una celda de acceso: {len(inf['accesos_con_objeto'])}"
               + "".join(f"\n    {a['puerta']:<20} {tuple(a['pos'])}  {', '.join(a['tapado_por'])}" for a in inf["accesos_con_objeto"]))
    if inf["carriles"]:
        out.append("Calles principales, carril recto de punta a punta:")
        for k in inf["carriles"]:
            out.append(f"    {k['banda']:<36} " + ("SI  carril %s" % k["carril"] if k["recto"] else
                       "NO  mejor carril %s con %d estorbos: %s" % (k["carril"], len(k["estorbos"]), ", ".join(k["estorbos"][:5]))))
    n = sum(v["n"] for v in inf["objetos_que_bloquean"].values())
    out.append(f"Objetos que bloquean: {n} ({sum(v['en_calle'] for v in inf['objetos_que_bloquean'].values())} sobre calle; "
               f"{inf['celdas_de_calle_con_objeto']} de {inf['celdas_de_calle']} celdas de calle ocupadas)")
    for k, v in inf["objetos_que_bloquean"].items():
        out.append(f"  {v['n']:>3} {v['en_calle']:>3}  {k}{'  (NPC)' if v['npc'] else ''}")
    out.append(f"Objetos que dejan una calle en una celda: {inf['estrechan_una_calle']}")
    if "quitar_para_ahorrar" in inf:
        out.append("Que quitar (voraz, solo decorado): " + (", ".join(f"{p['objeto']}@{tuple(p['pos'])} -{p['ahorra']}"
                                                                      for p in inf["quitar_para_ahorrar"]) or "nada ahorra pasos"))
    return "\n".join(out)


if __name__ == "__main__":
    args = [a for a in sys.argv[1:]]
    salida = args[args.index("--json") + 1] if "--json" in args else None
    if salida:
        args.remove(salida)
        args.remove("--json")
    todo = [informe(n, voraz="--sin-voraz" not in args) for n in (a for a in args if not a.startswith("--")) or ["ciudad_centro"]]
    if salida:
        json.dump(todo, open(salida, "w"), ensure_ascii=False, indent=1)
    print("\n\n".join(texto(i) for i in todo))
    if "--comprobar" in args:
        # Para validar_todo / CI: ninguna puerta tapada ni inalcanzable, y un carril recto por calle principal.
        mal = [i["nivel"] for i in todo if i["puertas_tapadas"] or i["accesos_con_objeto"]
               or any(not k["recto"] for k in i["carriles"]) or any(f["total"] is None for f in i["rutas"])]
        print("\nCOMPROBAR: " + ("FALLO en " + ", ".join(mal) if mal else "bien (puertas libres y alcanzables, carriles rectos)"))
        sys.exit(1 if mal else 0)
