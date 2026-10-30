#!/usr/bin/env python3
"""validar_puertas.py — toda puerta de los niveles lleva a un sitio al que se puede llegar (AV14-20).

Recorre assets/levels/*.json. Cada objeto con "targetLevel" es una puerta, y comprueba:

  1. El nivel destino existe y se puede leer.
  2. La casilla de llegada cae dentro del mapa. Es el "targetPosition" del objeto;
     si no lo trae, el "playerStart" del destino (ver ObjectSpawn::targetPosition
     en include/Level/LevelDefinition.h y la nota de FORMATO_NIVELES.md).
  3. La casilla de llegada se puede pisar. Mismo criterio que GameSession:
       - tileBlocks(): ninguna capa del TMX la marca con colision;
       - tryMovePlayer(): ningun objeto de esa casilla tiene blocksMovement;
       - ningun enemigo ocupa la casilla (pisarlo empieza un combate).
     Los objetos se miran con el catalogo de los proyectos que listan el nivel
     (si ninguno lo lista, con todos los catalogos de assets/objects/).
  4. El destino tiene una puerta de vuelta: algun objeto suyo lleva "targetLevel"
     al nivel de origen.

Solo stdlib. Sin argumentos valida el repo entero; "--raiz <dir>" sirve para las pruebas.
Salida: una linea por problema, con nivel, objeto, campo y motivo. Exit 1 si hay problemas.
validar_todo.py lo llama como AVISO, no como error.

Limites heredados del motor (no inventados aqui):
  - TMX con <tileset> embebido y <data encoding="csv"> (FORMATO_NIVELES.md, seccion 3).
    Un .tsx externo se marca como problema: el motor no lee su colision.
  - Un objeto cuyo objectId no esta en ningun catalogo no bloquea: no se carga en el mundo.
  - Un objeto de categoria "enemy" en "objects" cuenta como enemigo (empieza combate).
"""
from __future__ import annotations

import json
import os
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
MOTOR = TOOLS.parent
DIR_NIVELES = "assets/levels"


def _json(ruta: Path):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


def _celda(valor):
    """(x, y) de un {"x":..,"y":..}, o None si no es un objeto con coordenadas."""
    if not isinstance(valor, dict):
        return None
    try:
        return int(valor.get("x", 0)), int(valor.get("y", 0))
    except (TypeError, ValueError):
        return None


def _norma(ruta: str) -> str:
    return os.path.normpath(ruta).replace("\\", "/")


class Mapa:
    def __init__(self, ancho, alto, colision, capas):
        self.ancho, self.alto = ancho, alto
        self.colision = colision  # set de GIDs con collision=true
        self.capas = capas        # [capa][y][x] = GID

    def gids_en(self, x, y):
        out = []
        for capa in self.capas:
            if 0 <= y < len(capa) and 0 <= x < len(capa[y]):
                out.append(capa[y][x])
        return out


def leer_tmx(ruta: Path) -> Mapa:
    raiz = ET.parse(ruta).getroot()
    if raiz.tag != "map":
        raise ValueError("la raiz no es <map>")
    ancho, alto = int(raiz.get("width")), int(raiz.get("height"))
    colision = set()
    for ts in raiz.findall("tileset"):
        if ts.get("source") is not None:
            raise ValueError("tileset externo (.tsx): el motor no lee su colision")
        primero = int(ts.get("firstgid"))
        for tile in ts.findall("tile"):
            for prop in tile.findall("properties/property"):
                if prop.get("name") == "collision" and prop.get("value", "false").lower() == "true":
                    colision.add(primero + int(tile.get("id")))
    capas = []
    for capa in raiz.findall("layer"):
        datos = capa.find("data")
        if datos is None or datos.get("encoding") != "csv":
            raise ValueError(f"la capa '{capa.get('name')}' no es CSV")
        valores = [int(v) & 0x1FFFFFFF for v in (datos.text or "").replace("\n", "").split(",") if v.strip()]
        ancho_capa = int(capa.get("width", ancho))
        alto_capa = int(capa.get("height", alto))
        capas.append([valores[y * ancho_capa:(y + 1) * ancho_capa] for y in range(alto_capa)])
    return Mapa(ancho, alto, colision, capas)


def _entradas(ruta: Path) -> list[dict]:
    d = _json(ruta)
    if isinstance(d, list):
        lista = d
    elif isinstance(d, dict):
        lista = next((d[k] for k in ("entries", "objects") if isinstance(d.get(k), list)), [])
    else:
        lista = []
    return [e for e in lista if isinstance(e, dict) and isinstance(e.get("id"), str)]


def _defs(rutas) -> dict:
    """id -> {blocks, category}. Si dos catalogos definen el mismo id, bloquea si alguno bloquea."""
    out = {}
    for ruta in rutas:
        if not Path(ruta).is_file():
            continue
        for e in _entradas(Path(ruta)):
            d = out.setdefault(e["id"], {"blocks": False, "category": e.get("category", "?")})
            d["blocks"] = d["blocks"] or bool(e.get("blocksMovement", False))
            if e.get("category") == "enemy":
                d["category"] = "enemy"
    return out


class Contexto:
    def __init__(self, raiz: Path):
        self.raiz = raiz
        self._niveles, self._mapas, self._defs = {}, {}, {}
        self._catalogos_por_nivel = self._leer_proyectos()
        self._todos = sorted((raiz / "assets" / "objects").glob("*.json"))

    def _leer_proyectos(self) -> dict[str, list[Path]]:
        out: dict[str, list[Path]] = {}
        try:
            indice = _json(self.raiz / "assets" / "proyectos" / "index.json")
        except (OSError, ValueError):
            return out
        for pid in indice.get("proyectos", []) if isinstance(indice, dict) else []:
            try:
                man = _json(self.raiz / "assets" / "proyectos" / f"{pid}.json")
            except (OSError, ValueError):
                continue
            carpeta = (man.get("settings") or {}).get("catalogsDirectory", "objects")
            cats = [self.raiz / "assets" / carpeta / c for c in man.get("catalogs", []) if isinstance(c, str)]
            for niv in man.get("levels", []):
                if isinstance(niv, str):
                    out.setdefault(niv, []).extend(cats)
        return out

    def nivel(self, ruta_rel: str):
        if ruta_rel not in self._niveles:
            self._niveles[ruta_rel] = _json(self.raiz / ruta_rel)
        return self._niveles[ruta_rel]

    def mapa(self, ruta_rel: str) -> Mapa:
        if ruta_rel not in self._mapas:
            self._mapas[ruta_rel] = leer_tmx(self.raiz / ruta_rel)
        return self._mapas[ruta_rel]

    def defs_de(self, nombre_nivel: str) -> dict:
        if nombre_nivel not in self._defs:
            rutas = self._catalogos_por_nivel.get(nombre_nivel) or self._todos
            self._defs[nombre_nivel] = _defs(rutas)
        return self._defs[nombre_nivel]


def _motivo_bloqueo(ctx: Contexto, dnivel: dict, nombre_destino: str, mapa: Mapa, x: int, y: int):
    """Por que no se puede pisar (x, y) del destino, o None si se puede."""
    colisiones = [g for g in mapa.gids_en(x, y) if g in mapa.colision]
    if colisiones:
        return f"celda con colision en el TMX (GID {colisiones[0]})"
    for s in dnivel.get("enemies") or []:
        if isinstance(s, dict) and _celda(s.get("position")) == (x, y):
            return f"hay un enemigo ({s.get('type', '?')}): pisarlo empieza un combate"
    defs = ctx.defs_de(nombre_destino)
    for s in dnivel.get("objects") or []:
        if not isinstance(s, dict) or _celda(s.get("position")) != (x, y):
            continue
        oid = s.get("objectId")
        d = defs.get(oid)
        if d is None:
            continue  # sin ficha no se carga: no bloquea
        if d["category"] == "enemy":
            return f"hay un enemigo ({oid}): pisarlo empieza un combate"
        if d["blocks"]:
            return f"hay un objeto que bloquea ({oid})"
    return None


def revisa_puerta(ctx: Contexto, nombre: str, o: dict, problemas: list) -> None:
    oid = str(o.get("objectId", "?"))
    origen = f"{DIR_NIVELES}/{nombre}"

    def P(campo, texto):
        problemas.append((f"{DIR_NIVELES}/{nombre}", oid, campo, texto))

    destino = o["targetLevel"]
    if not isinstance(destino, str) or not (ctx.raiz / destino).is_file():
        P("targetLevel", f"el nivel '{destino}' no existe")
        return
    try:
        dnivel = ctx.nivel(destino)
    except (OSError, ValueError) as e:
        P("targetLevel", f"el nivel '{destino}' no se lee: {e}")
        return
    if not isinstance(dnivel, dict) or "map" not in dnivel:
        P("targetLevel", f"'{destino}' no es un nivel (falta \"map\")")
        return
    try:
        mapa = ctx.mapa(dnivel["map"])
    except (OSError, ValueError, ET.ParseError) as e:
        P("targetLevel", f"el mapa '{dnivel['map']}' de {destino} no se lee: {e}")
        return

    if isinstance(o.get("targetPosition"), dict):
        x, y = _celda(o["targetPosition"])
    else:
        x, y = _celda(dnivel.get("playerStart")) or (0, 0)

    if not (0 <= x < mapa.ancho and 0 <= y < mapa.alto):
        P("targetPosition", f"la llegada ({x},{y}) en {destino} cae fuera del mapa {mapa.ancho}x{mapa.alto}")
    else:
        motivo = _motivo_bloqueo(ctx, dnivel, os.path.basename(destino), mapa, x, y)
        if motivo:
            P("targetPosition", f"llegada ({x},{y}) en {destino}: {motivo}")

    if not any(isinstance(d, dict) and isinstance(d.get("targetLevel"), str)
               and _norma(d["targetLevel"]) == _norma(origen)
               for d in dnivel.get("objects") or []):
        P("targetLevel", f"{destino} no tiene puerta de vuelta a {origen}")


def comprueba(raiz: Path = MOTOR):
    """(niveles, puertas, problemas). Cada problema: (nivel, objectId, campo, texto)."""
    raiz = Path(raiz)
    ctx = Contexto(raiz)
    problemas: list = []
    niveles = puertas = 0
    for ruta in sorted((raiz / DIR_NIVELES).glob("*.json")):
        rel = f"{DIR_NIVELES}/{ruta.name}"
        try:
            nivel = ctx.nivel(rel)
        except (OSError, ValueError) as e:
            problemas.append((rel, "-", "json", f"no se lee: {e}"))
            continue
        if not isinstance(nivel, dict) or "map" not in nivel:
            continue  # JSON de la carpeta que no es un nivel
        niveles += 1
        for o in nivel.get("objects") or []:
            if not isinstance(o, dict) or not o.get("targetLevel"):
                continue
            puertas += 1
            try:
                revisa_puerta(ctx, ruta.name, o, problemas)
            except Exception as e:  # una puerta rara no tumba el resto
                problemas.append((rel, str(o.get("objectId", "?")), "targetLevel", f"no se pudo comprobar: {e}"))
    return niveles, puertas, problemas


def resum(probs: list[tuple]) -> dict[str, int]:
    """Cuenta los problemas de comprueba() por tipo (plan E-030)."""
    out = {"icloud": 0, "sin_vuelta": 0, "destino_inexistente": 0, "casilla_no_pisable": 0, "otro": 0}
    for p in probs:
        msg = str(p[3])
        if "Errno 35" in msg:
            tipo = "icloud"
        elif "no tiene puerta de vuelta" in msg:
            tipo = "sin_vuelta"
        elif "no existe" in msg:
            tipo = "destino_inexistente"
        elif "pisable" in msg:
            tipo = "casilla_no_pisable"
        else:
            tipo = "otro"
        out[tipo] += 1
    return out


def main(argv=None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    raiz = MOTOR
    if "--raiz" in argv:
        i = argv.index("--raiz")
        raiz = Path(argv[i + 1]).resolve()
    niveles, puertas, problemas = comprueba(raiz)
    for nivel, oid, campo, texto in problemas:
        print(f"[AVISO] {nivel} :: {oid} :: {campo} :: {texto}")
    print(f"{niveles} niveles, {puertas} puertas comprobadas, {len(problemas)} problema(s)")
    return 1 if problemas else 0


if __name__ == "__main__":
    sys.exit(main())
