#!/usr/bin/env python3
"""AV18-05b - el botin de los TRES DIAS con los contenedores nativos del motor.

Sustituye al reparto estatico de AV18-05 (Qwen, da120b1), que escribia `loot: [...]`
y un `eventId` compartido ("botin_av18_05"). Ahora cada instancia es un prefab del
contrato de AV18-02/03 (FORMATO_CONTENEDORES.md, "Tablas publicadas y colocacion"):

    {"objectId": "botin_ciudad_cofre", "eventId": "botin_ciudad_centro_cofre_01",
     "position": {...}, "botin": {"dia": 1, "region": "ciudad",
                                  "canal": "cofres_visibles", "orden": 10}}

Lo que se conserva de Qwen: colocacion por REGLAS (no por coordenadas, la ciudad se
regenera), aprovechar lo que ya esta en el mapa para el canal "entorno", y no cerrar
nunca un paso. Lo que cambia:

1. Los tres dias, no solo el 1. El presupuesto se lee del bloque ```json economia``` de
   ECONOMIA.md (§8), por dia, region y canal; solo los tres canales estaticos
   (cofres_visibles, entorno, tesoros_ocultos): combates y misiones los paga el motor.
2. El valor de cada instancia es el `presupuesto` de su prefab (asi lo valida
   tools/validar_botin.py). Cuando el presupuesto que queda no cabe en ningun prefab
   publicado, se crea una VARIANTE del prefab con ese presupuesto
   (`botin_<region>_<tipo>_<n>`, misma tabla y requisitos) en botin_boundington.json.
3. Dia 2 y 3: el motor no lee `botin.dia`, asi que en los mapas que se pisan el dia 1
   la instancia lleva `"disponibleDia": 2|3` (lo lee GameSession, AV18-07/PO-52).
4. Entorno: el barril, saco o armario que YA esta en el mapa pasa a ser el prefab del
   mismo tipo en la misma casilla; `botin.original` guarda el objectId de antes para
   poder deshacerlo.
5. Idempotente: en cada pasada se deshace lo de la pasada anterior (y las etiquetas
   viejas de AV18-05) y se vuelve a repartir con la misma semilla.

Uso:  python3 tools/repartir_botin.py            # reparte y escribe
      python3 tools/repartir_botin.py --seco     # solo el recuento, no escribe
      --economia RUTA                            # otra copia de ECONOMIA.md
"""
import json
import os
import random
import re
import sys
from collections import deque

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D  # noqa: E402
from pathlib import Path
from validar_botin import lee_economia  # noqa: E402

BASE = D.BASE
SECO = "--seco" in sys.argv
SEMILLA = 1805
PREFIJO = "botin_"                     # eventId y objectId de todo lo que pone esta herramienta
VIEJO = "botin_av18_05"                # las etiquetas de AV18-05 (se deshacen)
CANAL = {"cofre": "cofres_visibles", "escondite": "tesoros_ocultos",
         "barril": "entorno", "saco": "entorno", "armario": "entorno"}
# Lo que ya esta en el mapa y se convierte en contenedor de entorno: prefijo -> tipo.
ENTORNO = (("barril", "barril"), ("barrica", "barril"), ("tonel", "barril"),
           ("saco", "saco"), ("fardo", "saco"), ("cesta", "saco"), ("caja", "saco"),
           ("armario", "armario"), ("estanteria", "armario"))
PREFABS = BASE + "assets/objects/botin_boundington.json"
# Mapas que se visitan por primera vez un dia concreto: ahi no hace falta disponibleDia.
PRIMER_DIA = {"cueva": 2, "ocaso": 3}


def ruta_economia():
    if "--economia" in sys.argv:
        return sys.argv[sys.argv.index("--economia") + 1]
    aqui = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(aqui, "..", "..", "..",
                                         "planificacion_alcance_y_vision", "ECONOMIA.md"))


def region_de(nombre):
    """En que region de ECONOMIA.md cae cada nivel del proyecto boundington."""
    if nombre.startswith("recuerdo_") or nombre == "prado_del_rio":
        return None                           # prologo: fuera de la tabla (§3)
    if nombre == "interior_iglesia_s1":
        return "cripta"
    if nombre.startswith("interior_") or nombre.startswith("mansion_") \
            or nombre == "tienda_abandonada":
        return "interiores"
    if nombre.startswith("afueras_") or nombre.startswith("montana_"):
        return "afueras"
    if nombre.startswith("cueva_"):
        return "cueva"
    if nombre == "ciudad_centro_ocaso":
        return "ciudad"
    if nombre.startswith("ciudad_") or nombre == "plaza_de_las_razas":
        return "ciudad"
    return None


def primer_dia(nombre):
    if nombre.startswith("cueva_") or nombre.startswith("montana_"):
        return 2
    if nombre == "ciudad_centro_ocaso":
        return 3
    return 1


def niveles():
    man = json.load(open(BASE + "assets/proyectos/boundington.json", encoding="utf-8"))
    return [os.path.splitext(n)[0] for n in man["levels"]]


class Prefabs:
    """Los contenedores publicados (botin_boundington.json) y las variantes que hagan falta."""

    def __init__(self, limites):
        self.doc = json.load(open(PREFABS, encoding="utf-8"))
        self.por_id = {o["id"]: o for o in self.doc["objects"]}
        self.limites = limites
        self.nuevos = []

    def base(self, region, tipo):
        return self.por_id.get(f"botin_{region}_{tipo}")

    def tamanos(self, region, tipo):
        """Presupuestos disponibles para region/tipo, de mayor a menor."""
        out = {}
        for oid, o in self.por_id.items():
            c = o.get("contenedor")
            if c and c["tipo"] == tipo and c["tabla"] == f"botin_{region}_{tipo}":
                out[c["presupuesto"]] = oid
        return dict(sorted(out.items(), reverse=True))

    def para(self, region, tipo, resto):
        """El prefab mas grande que cabe en `resto`; si ninguno cabe pero `resto` llega al
        minimo del tipo, una variante con ese presupuesto. None si no cabe nada."""
        minimo, maximo = self.limites[tipo]
        if resto < max(minimo, 1):
            return None
        for presupuesto, oid in self.tamanos(region, tipo).items():
            if presupuesto <= resto:
                return oid, presupuesto
        base = self.base(region, tipo)
        if base is None:
            return None
        presupuesto = min(resto, maximo)
        oid = f"botin_{region}_{tipo}_{presupuesto}"
        if oid not in self.por_id:
            o = json.loads(json.dumps(base))
            o["id"] = oid
            o["name"] = base.get("name", oid)
            o["contenedor"]["presupuesto"] = presupuesto
            self.doc["objects"].append(o)
            self.por_id[oid] = o
            self.nuevos.append(oid)
        return oid, presupuesto

    def guardar(self):
        if self.nuevos and not SECO:
            open(PREFABS, "w", encoding="utf-8").write(
                json.dumps(self.doc, indent=2, ensure_ascii=False) + "\n")


class Nivel:
    """Un nivel cargado, con sus celdas, sus objetos y lo que no se puede tocar."""

    def __init__(self, nombre, nombres_tile):
        self.nombre = nombre
        self.lvl, self.g, self.col, _ = D.cargar_nivel(nombre)
        self.h, self.w = len(self.g), len(self.g[0])
        self.objetos = []
        for o in self.lvl["objects"]:
            o = dict(o)
            ev = str(o.get("eventId", ""))
            if ev.startswith(VIEJO):           # AV18-05: etiqueta o contenedor viejo
                if ev.endswith("_nuevo"):
                    continue
                o.pop("eventId"); o.pop("loot", None)
            elif ev.startswith(PREFIJO) and str(o.get("objectId", "")).startswith(PREFIJO):
                original = (o.get("botin") or {}).get("original")
                if not original:
                    continue                   # lo puse yo: fuera, se vuelve a poner
                o = {"objectId": original, "position": o["position"]}  # era del nivel
            self.objetos.append(o)
        self.ocupadas = {(o["position"]["x"], o["position"]["y"]) for o in self.objetos}
        self.puertas = [(o["position"]["x"], o["position"]["y"]) for o in self.objetos
                        if o.get("targetLevel")]
        self.intocables = set(self.puertas)
        for (x, y) in self.puertas:
            for dx, dy in D.VECINOS:
                self.intocables.add((x + dx, y + dy))
        s = self.lvl["playerStart"]
        self.intocables.add((s["x"], s["y"]))
        self.carriles = {(x, y) for (x, y) in self.lvl.get("carriles", [])}
        self.nombres = nombres_tile
        self.dist = self.distancias()
        self.contador = {}

    def tile(self, c):
        return self.nombres.get(self.g[c[1]][c[0]], "")

    def andable(self, c):
        x, y = c
        return 0 <= x < self.w and 0 <= y < self.h and self.g[y][x] not in self.col

    def libre(self, c):
        return self.andable(c) and c not in self.ocupadas and c not in self.intocables \
            and c not in self.carriles

    def distancias(self):
        """Casillas hasta la entrada mas cercana (puertas o inicio): "en el camino" o no."""
        s = self.lvl["playerStart"]
        semillas = set(self.puertas) | {(s["x"], s["y"])}
        dist = {c: 0 for c in semillas}
        q = deque(sorted(semillas))
        while q:
            c = q.popleft()
            for dx, dy in D.VECINOS:
                v = (c[0] + dx, c[1] + dy)
                if v not in dist and self.andable(v):
                    dist[v] = dist[c] + 1
                    q.append(v)
        return dist

    def candidatos(self, tipo):
        """Celdas libres para un contenedor nuevo. Cofres: cerca de una entrada y, en la
        calle, a la vista. Escondites: lejos de todas las entradas."""
        for y in range(self.h):
            for x in range(self.w):
                c = (x, y)
                if not self.libre(c):
                    continue
                d = self.dist.get(c, 99)
                if tipo == "escondite":
                    if d < 6:
                        continue
                else:
                    if d > 4:
                        continue
                    cerca = any((x + dx, y + dy) in self.carriles for dx, dy in D.VECINOS)
                    if self.carriles and not cerca and self.tile(c) not in D.CALLES:
                        continue
                yield c

    def event_id(self, tipo):
        n = self.contador.get(tipo, 0) + 1
        self.contador[tipo] = n
        return f"botin_{self.nombre}_{tipo}_{n:02d}"

    def spawn(self, oid, c, tipo, region, dia, orden, original=None):
        o = {"objectId": oid, "eventId": self.event_id(tipo),
             "position": {"x": c[0], "y": c[1]},
             "botin": {"dia": dia, "region": region, "canal": CANAL[tipo], "orden": orden}}
        if original:
            o["botin"]["original"] = original
        if dia > primer_dia(self.nombre):
            o["disponibleDia"] = dia
        return o

    def poner(self, oid, c, tipo, region, dia, orden):
        o = self.spawn(oid, c, tipo, region, dia, orden)
        self.objetos.append(o)
        self.ocupadas.add(c)
        return o

    def convertir(self, viejo, oid, tipo, region, dia, orden):
        """El barril/saco/armario del nivel pasa a ser el prefab, en su casilla."""
        c = (viejo["position"]["x"], viejo["position"]["y"])
        nuevo = self.spawn(oid, c, tipo, region, dia, orden, original=viejo["objectId"])
        self.objetos[self.objetos.index(viejo)] = nuevo
        return nuevo

    def alcanzables(self, catalogos):
        """Flood fill contando los objetos que bloquean, que es lo que hace el motor."""
        bloquea = lambda oid: bool(catalogos.get(oid, {}).get("blocksMovement")) \
            and not catalogos.get(oid, {}).get("flotante")
        muro = {(o["position"]["x"], o["position"]["y"]) for o in self.objetos
                if bloquea(o["objectId"])}
        s = self.lvl["playerStart"]
        vis = {(s["x"], s["y"])}
        pila = [(s["x"], s["y"])]
        while pila:
            x, y = pila.pop()
            for dx, dy in D.VECINOS:
                c = (x + dx, y + dy)
                if self.andable(c) and c not in muro and c not in vis:
                    vis.add(c)
                    pila.append(c)
        return vis

    def escribir(self):
        self.lvl["objects"] = self.objetos
        base = BASE + f"assets/levels/{self.nombre}.json"
        actual = open(base, encoding="utf-8").read()
        m = re.match(r'\{\n( +)\S', actual)
        open(base, "w", encoding="utf-8").write(
            json.dumps(self.lvl, indent=len(m.group(1)) if m else 1, ensure_ascii=False) + "\n")


def presupuestos(eco):
    """{(dia, region): {canal: valor}} del bloque JSON de ECONOMIA.md."""
    out = {}
    for p in eco["presupuestos"]:
        out[p["dia"], p["region"]] = dict(zip(eco["canales"], p["valor"]))
    return out


def repartir(eco, catalogos, prefabs):
    rng = random.Random(SEMILLA)
    tiles = {t["gid"]: t["nombre"] for t in
             json.load(open(BASE + "assets/textures/terreno_iso.json"))["tiles"]}
    pres = presupuestos(eco)
    por_region = {}
    for nombre in niveles():                      # el orden del manifiesto = la ruta
        region = region_de(nombre)
        if region:
            por_region.setdefault(region, []).append(Nivel(nombre, tiles))
    recuento = {}                                 # (dia, region, canal) -> colocado
    orden = {}                                    # region -> siguiente orden
    alcanzables = {}

    def siguiente(region, dia):
        # Antes de las compras del dia (orden 90 en ECONOMIA.md) y despues de las
        # escenas de la manana (orden < 10 en rutas_economia.json).
        orden[region, dia] = min(85, orden.get((region, dia), 8) + 2)
        return orden[region, dia]

    def coloca_entorno(region, dia, resto):
        """Barriles, sacos y armarios que ya estan en los mapas de la region."""
        gastado = 0
        candidatos = []
        for n in por_region[region]:
            if dia < primer_dia(n.nombre):
                continue
            for o in n.objetos:
                if o.get("eventId") or o.get("targetLevel"):
                    continue
                for prefijo, tipo in ENTORNO:
                    if o["objectId"].startswith(prefijo):
                        candidatos.append((n, o, tipo))
                        break
        rng.shuffle(candidatos)
        for n, o, tipo in candidatos:
            r = prefabs.para(region, tipo, resto - gastado)
            if r is None:
                continue
            oid, presupuesto = r
            if presupuesto > resto - gastado:
                continue
            n.convertir(o, oid, tipo, region, dia, siguiente(region, dia))
            gastado += presupuesto
        return gastado

    def coloca_nuevos(region, dia, tipo, resto):
        """Cofres y escondites nuevos, repartidos entre los mapas de la region."""
        gastado = 0
        mapas = [n for n in por_region[region] if dia >= primer_dia(n.nombre)]
        rng.shuffle(mapas)
        intentos = 0
        while mapas and gastado < resto and intentos < 200:
            intentos += 1
            r = prefabs.para(region, tipo, resto - gastado)
            if r is None:
                break
            oid, presupuesto = r
            n = mapas[intentos % len(mapas)]
            celdas = sorted(n.candidatos(tipo))
            if not celdas:
                mapas = [m for m in mapas if m is not n]
                continue
            c = rng.choice(celdas)
            if n.nombre not in alcanzables:
                alcanzables[n.nombre] = n.alcanzables(catalogos)
            o = n.poner(oid, c, tipo, region, dia, siguiente(region, dia))
            if catalogos.get(oid, {}).get("blocksMovement") and \
                    (alcanzables[n.nombre] - n.alcanzables(catalogos)) - {c}:
                n.objetos.remove(o); n.ocupadas.discard(c)
                n.contador[tipo] -= 1
                orden[region, dia] -= 2
                continue
            gastado += presupuesto
        return gastado

    for dia in (1, 2, 3):
        for region in sorted(por_region):
            p = pres[dia, region]
            recuento[dia, region, "cofres_visibles"] = coloca_nuevos(region, dia, "cofre", p["cofres_visibles"])
            recuento[dia, region, "entorno"] = coloca_entorno(region, dia, p["entorno"])
            recuento[dia, region, "tesoros_ocultos"] = coloca_nuevos(region, dia, "escondite", p["tesoros_ocultos"])
    return por_region, recuento, pres


def main():
    eco = lee_economia(Path(ruta_economia()))
    catalogos = D.catalogo()
    prefabs = Prefabs(eco["limites_contenedor"])
    por_region, recuento, pres = repartir(eco, catalogos, prefabs)
    # Los prefabs nuevos tienen que estar en el catalogo que carga el juego para que
    # blocksMovement y el flood fill los conozcan en la pasada siguiente.
    for oid in prefabs.nuevos:
        catalogos[oid] = prefabs.por_id[oid]

    print(f"  ECONOMIA.md: {ruta_economia()}")
    print("  presupuesto (cofres / entorno / ocultos) y lo colocado, por dia y region:\n")
    faltan = []
    for dia in (1, 2, 3):
        for region in ("ciudad", "interiores", "cripta", "afueras", "cueva"):
            p = pres[dia, region]
            if region not in por_region:
                if any(p[c] for c in ("cofres_visibles", "entorno", "tesoros_ocultos")):
                    faltan.append(f"dia {dia} {region}")
                continue
            g = [recuento[dia, region, c] for c in ("cofres_visibles", "entorno", "tesoros_ocultos")]
            q = [p["cofres_visibles"], p["entorno"], p["tesoros_ocultos"]]
            ok = "OK" if all(g[i] <= q[i] for i in range(3)) else "SE PASA"
            print(f"    dia {dia} {region:11} {g[0]:>4}/{q[0]:<4} {g[1]:>4}/{q[1]:<4} {g[2]:>4}/{q[2]:<4}  {ok}")
            assert ok == "OK"
    if faltan:
        print("\n  sin niveles todavia (se reparte cuando entren):", ", ".join(faltan))
    if prefabs.nuevos:
        print("\n  variantes de prefab creadas:", ", ".join(prefabs.nuevos))
    total = 0
    for region, mapas in sorted(por_region.items()):
        n = sum(1 for m in mapas for o in m.objetos if str(o.get("eventId", "")).startswith(PREFIJO))
        total += n
        print(f"  {region:11} {len(mapas):>2} niveles, {n:>3} contenedores")
    print(f"  total: {total} contenedores")
    if SECO:
        print("\n  (en seco: nada escrito)")
        return 0
    prefabs.guardar()
    for mapas in por_region.values():
        for m in mapas:
            m.escribir()
    return 0


if __name__ == "__main__":
    sys.exit(main())
