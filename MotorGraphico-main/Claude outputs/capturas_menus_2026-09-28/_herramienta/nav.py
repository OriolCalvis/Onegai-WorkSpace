# Rutas por la rejilla de un nivel: colision del TMX + objetos que bloquean.
# Devuelve la lista de teclas W/A/S/D para ir de A a B (BFS).
import json, glob, os, re, xml.etree.ElementTree as ET
from collections import deque

ASSETS = "/home/claude/mg/assets"
_catalog = None

def catalog():
    global _catalog
    if _catalog is None:
        _catalog = {}
        for f in glob.glob(os.path.join(ASSETS, "objects", "*.json")):
            try:
                d = json.load(open(f))
            except Exception:
                continue
            objs = d.get("objects", []) if isinstance(d, dict) else []
            for o in objs:
                if isinstance(o, dict) and "id" in o:
                    _catalog.setdefault(o["id"], o)
    return _catalog

class Level:
    def __init__(self, level_rel):
        self.file = os.path.join(ASSETS, "..", level_rel) if level_rel.startswith("assets/") else level_rel
        self.data = json.load(open(self.file))
        mp = self.data["map"]
        tmx = os.path.join(ASSETS, "..", mp)
        root = ET.parse(tmx).getroot()
        self.w = int(root.get("width")); self.h = int(root.get("height"))
        coll = set()
        for ts in root.findall("tileset"):
            first = int(ts.get("firstgid"))
            for t in ts.findall("tile"):
                for p in t.iter("property"):
                    if p.get("name") == "collision" and p.get("value") == "true":
                        coll.add(first + int(t.get("id")))
        self.blocked = set()
        for layer in root.findall("layer"):
            data = layer.find("data").text.strip()
            vals = [int(v) for v in re.split(r"[,\s]+", data) if v]
            for i, g in enumerate(vals):
                x, y = i % self.w, i // self.w
                if g == 0 or (g & 0x1FFFFFFF) in coll:
                    self.blocked.add((x, y))
        self.objects = {}
        cat = catalog()
        for o in self.data.get("objects", []):
            p = o["position"]; c = (p["x"], p["y"])
            self.objects[c] = o["objectId"]
            d = cat.get(o["objectId"], {})
            if d.get("blocksMovement") or d.get("category") == "enemy":
                self.blocked.add(c)
        s = self.data.get("playerStart", {"x": 0, "y": 0})
        self.start = (s["x"], s["y"])

    def find(self, object_id):
        return [c for c, i in self.objects.items() if i == object_id]

    def path(self, a, b, avoid_doors=True):
        """BFS de a a b; evita pisar puertas intermedias si se puede."""
        doors = {c for c, i in self.objects.items()
                 if "targetLevel" in next((o for o in self.data["objects"] if (o["position"]["x"], o["position"]["y"]) == c), {})}
        def ok(c):
            x, y = c
            if not (0 <= x < self.w and 0 <= y < self.h):
                return False
            if c in self.blocked and c != b:
                return False
            return True
        for use_doors in ([False, True] if avoid_doors else [True]):
            prev = {a: None}; q = deque([a])
            while q:
                c = q.popleft()
                if c == b:
                    break
                for dx, dy, k in ((0, -1, "W"), (0, 1, "S"), (-1, 0, "A"), (1, 0, "D")):
                    n = (c[0] + dx, c[1] + dy)
                    if n in prev or not ok(n):
                        continue
                    if not use_doors and n in doors and n != b:
                        continue
                    prev[n] = (c, k); q.append(n)
            if b in prev:
                keys = []; c = b
                while prev[c] is not None:
                    c, k = prev[c][0], prev[c][1]
                    keys.append(k)
                return list(reversed(keys))
        return None

    def adjacent_free(self, target, prefer=("S", "W", "E", "N")):
        x, y = target
        opts = {"N": (x, y - 1), "S": (x, y + 1), "W": (x - 1, y), "E": (x + 1, y)}
        return [opts[p] for p in prefer if opts[p] not in self.blocked
                and 0 <= opts[p][0] < self.w and 0 <= opts[p][1] < self.h]

if __name__ == "__main__":
    L = Level("assets/levels/ciudad_centro.json")
    print(L.w, L.h, L.start, len(L.blocked))
    for t in ("puerta_sastreria", "cartel_sastreria", "anciana", "nino"):
        c = L.find(t)[0]
        adj = L.adjacent_free(c)
        print(t, c, adj, L.path(L.start, c if t.startswith("puerta") else adj[0]))
