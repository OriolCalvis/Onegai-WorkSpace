#!/usr/bin/env python3
"""Ensancha los pasos entre zonas a todo el hueco (Oriol, 16/10: «hay que expandir las
casillas que te permiten ir a otra zona»).

Hoy cada paso entre barrios (`paso_centro_a_este_norte`, `camino_a_egaroth`, las salidas de
las afueras y la montana...) es UNA casilla, y el hueco de la muralla o del camino tiene tres
o cinco. Si no pisas justo esa, no cambias de zona. Esto pone el mismo objeto de paso en
todas las casillas abiertas del hueco (hasta 5, contiguas a la original), y el destino se
desplaza igual si en el otro nivel esa casilla tambien esta abierta; si no, se llega a la
casilla de siempre.

Reglas:
  - Solo salidas de ZONA a ZONA (ni el nivel ni el destino son un interior, una mansion o una
    tienda: la salida de una tienda es su puerta) y que
    estan en un borde del mapa (x <= 1, x >= W-2, y <= 1, y >= H-2). Se ensancha A LO ANCHO
    del paso: por el borde este/oeste, a lo largo de y; por el norte/sur, a lo largo de x.
  - Idempotente: los clones son los spawns con el mismo objectId y targetLevel que un
    primario y pegados a el en el eje del borde; se quitan y se vuelven a poner.
  - No toca puertas de interiores, ni el motor, ni los catalogos (los clones llevan el
    mismo objectId, asi que las fichas de boundington_pasos.json valen).
Uso:  python3 tools/ensanchar_pasos.py            # escribe
      python3 tools/ensanchar_pasos.py --seco     # solo informa
"""
import json
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
NIVELES = os.path.join(RAIZ, "assets", "levels")
SECO = "--seco" in sys.argv
MAX_ANCHO = 5
NO_ZONA = ("interior_", "mansion_", "tienda_", "recuerdo_", "mazmorra", "mundi_landmass")


def rejilla(nivel):
    p = os.path.join(RAIZ, nivel["map"])
    t = re.sub(r"<!--.*?-->", "", open(p, encoding="utf-8").read(), flags=re.S)
    W = int(re.search(r'<map[^>]*?\swidth="(\d+)"', t).group(1))
    H = int(re.search(r'<map[^>]*?\sheight="(\d+)"', t).group(1))
    col = {int(m) + 1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"[^/]*/>', t)}
    g = [int(v) for v in re.search(r'<data encoding="csv">(.*?)</data>', t, re.S)
         .group(1).replace("\n", "").split(",") if v.strip()]
    return W, H, g, col


class Mapa:
    def __init__(self, nombre):
        self.nombre = nombre
        self.ruta = os.path.join(NIVELES, nombre + ".json")
        self.texto = open(self.ruta, encoding="utf-8").read()
        self.lvl = json.loads(self.texto)
        self.W, self.H, self.g, self.col = rejilla(self.lvl)
        self.cambiado = False

    def andable(self, x, y):
        return 0 <= x < self.W and 0 <= y < self.H and self.g[y * self.W + x] not in self.col

    def ocupadas(self):
        return {(o["position"]["x"], o["position"]["y"]) for o in self.lvl["objects"]}

    def borde(self, x, y):
        if x <= 1: return "O"
        if x >= self.W - 2: return "E"
        if y <= 1: return "N"
        if y >= self.H - 2: return "S"
        return None

    def escribir(self):
        if not self.cambiado or SECO:
            return
        m = re.match(r'\{\n( +)\S', self.texto)
        open(self.ruta, "w", encoding="utf-8").write(
            json.dumps(self.lvl, indent=len(m.group(1)) if m else 1, ensure_ascii=False) + "\n")


def es_zona(target):
    return target and not any(k in target for k in NO_ZONA)


def main():
    mapas = {}
    def mapa(nombre):
        if nombre not in mapas:
            mapas[nombre] = Mapa(nombre)
        return mapas[nombre]

    manifiesto = json.load(open(os.path.join(RAIZ, "assets/proyectos/boundington.json"), encoding="utf-8"))
    niveles = [os.path.splitext(n)[0] for n in manifiesto["levels"]]
    total = 0
    informe = []
    for nombre in niveles:
        if not es_zona(nombre):
            continue                            # la salida de una tienda es su puerta, no un hueco
        try:
            m = mapa(nombre)
        except (OSError, ValueError, AttributeError):
            continue
        objetos = m.lvl["objects"]
        # 1) quitar los clones de pasadas anteriores: mismo objectId+targetLevel que un primario
        #    (el primero del fichero) y pegado a el en el eje del borde.
        primarios = {}
        limpios = []
        for o in objetos:
            t = o.get("targetLevel", "")
            if not es_zona(t):
                limpios.append(o); continue
            clave = (o["objectId"], t)
            x, y = o["position"]["x"], o["position"]["y"]
            if clave not in primarios:
                primarios[clave] = o; limpios.append(o); continue
            p = primarios[clave]
            px, py = p["position"]["x"], p["position"]["y"]
            b = m.borde(px, py)
            pegado = (b in ("E", "O") and x == px and abs(y - py) < MAX_ANCHO) or \
                     (b in ("N", "S") and y == py and abs(x - px) < MAX_ANCHO)
            if not pegado:
                limpios.append(o)          # otro paso con el mismo nombre, lejos: no es un clon
        if len(limpios) != len(objetos):
            m.lvl["objects"] = objetos = limpios; m.cambiado = True

        # 2) clonar cada primario a lo ancho del hueco
        for (oid, t), p in primarios.items():
            px, py = p["position"]["x"], p["position"]["y"]
            b = m.borde(px, py)
            if b is None:
                informe.append(f"  {nombre}: {oid} en ({px},{py}) no esta en un borde: se deja como esta")
                continue
            destino_nombre = os.path.splitext(os.path.basename(t))[0]
            try:
                md = mapa(destino_nombre)
            except (OSError, ValueError, AttributeError):
                md = None
            tp = p.get("targetPosition")
            eje = (0, 1) if b in ("E", "O") else (1, 0)
            ocup = m.ocupadas()
            nuevos = []
            for signo in (-1, 1):
                for k in range(1, MAX_ANCHO):
                    x, y = px + eje[0] * k * signo, py + eje[1] * k * signo
                    if not m.andable(x, y) or (x, y) in ocup:
                        break                      # el hueco se acaba (o hay algo): contiguo o nada
                    if len(nuevos) + 1 >= MAX_ANCHO:
                        break
                    c = {"objectId": oid, "position": {"x": x, "y": y}, "targetLevel": t}
                    if tp is not None:
                        dx, dy = eje[0] * k * signo, eje[1] * k * signo
                        if md is not None and md.andable(tp["x"] + dx, tp["y"] + dy) and \
                                (tp["x"] + dx, tp["y"] + dy) not in md.ocupadas() - {(tp["x"], tp["y"])}:
                            c["targetPosition"] = {"x": tp["x"] + dx, "y": tp["y"] + dy}
                        else:
                            c["targetPosition"] = dict(tp)
                    for k2 in ("displayName",):
                        if k2 in p: c[k2] = p[k2]
                    nuevos.append(c)
            if nuevos:
                objetos.extend(nuevos); m.cambiado = True; total += len(nuevos)
                informe.append(f"  {nombre:22} {oid:30} ({px},{py}) +{len(nuevos)} -> {destino_nombre}")
    for linea in informe:
        print(linea)
    for m in mapas.values():
        m.escribir()
    print(f"  {total} casillas de paso anadidas" + (" (en seco: nada escrito)" if SECO else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
