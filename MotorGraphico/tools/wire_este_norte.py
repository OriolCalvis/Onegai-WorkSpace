"""Enlaza el Este-Norte con el mapamundi (landmass 2/4/7/8). IDEMPOTENTE.

Dos direcciones por asentimiento:
  1. el objeto-ciudad del landmass gana targetLevel/targetPosition ->
     entra a mi nivel exterior (a su playerStart, siempre transitable).
  2. la salida_en_<slug> de mi nivel gana targetLevel/targetPosition ->
     vuelve al landmass, a la celda transitable mas cercana al marcador.

Reglas del experimento (biblia, Parte 0):
  - SOLO se tocan los objetos de mis naciones (Choubar, Gongorguma,
    Mistarium, Nocturnsea). Los del Este-Sur (landmass_2 compartido) ni
    se miran: si un objeto ya tiene targetLevel, se salta SIEMPRE, sea
    de quien sea.
  - Los 8 asentimientos sin marcador (Qethatos + 6 de Mistarium +
    Grytoz) se anaden como objetos nuevos con su id canon cuando
    existe (geografia.json) o el id del experimento (Grytoz).

Caminabilidad del landmass: gid SIN propiedad collision = transitable.
En estos mapas el mar (gid 1) es transitable y la tierra firme (gid 2)
colisiona: es el criterio del propio TMX, no el nuestro.
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import este_norte_config as C

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/"

# ids canonicos de los asentimientos SIN marcador en ningun landmass
ID_CANON_EXTRA = {
    "qethatos":   "ciudad_mrnu4v7et14b",
    "ciudad_m":   "ciudad_mrnu66rfewq8",
    "ciudad_m_i": "ciudad_mrnu6d8qbys8",
    "pueblo_m":   "ciudad_mrnu5oyu3o9y",
    "pueblo_m_i": "ciudad_mrnu5z9w1rsg",
    "pueblo_m_ii": "ciudad_mrnu6ognh3t5",
    "aldea_m":    "ciudad_mrnu5gfvjf64",
}


def cargar_tmx(path):
    """(w, h, gids, colision) del TMX; colision = set de GIDs bloqueados."""
    t = open(path).read()
    w = int(re.search(r'width="(\d+)"', t).group(1))
    h = int(re.search(r'height="(\d+)"', t).group(1))
    gids = [int(v) for v in
            re.search(r'<data encoding="csv">(.*?)</data>', t, re.S)
            .group(1).replace("\n", "").split(",") if v.strip()]
    col = {int(m) + 1 for m in re.findall(
        r'<tile id="(\d+)">\s*<properties>\s*'
        r'<property name="collision"[^/]*/>', t)}
    return w, h, gids, col


def transitable(w, h, gids, col, x, y):
    return 0 <= x < w and 0 <= y < h and gids[y * w + x] not in col


def snap(w, h, gids, col, x, y):
    """Celda transitable mas cercana (espiral). Si el landmass entero
    colisiona, revienta: mejor un fallo claro que un warp al vacio."""
    if transitable(w, h, gids, col, x, y):
        return x, y
    for r in range(1, max(w, h)):
        for dx in range(-r, r + 1):
            for dy in (-r, r):
                if transitable(w, h, gids, col, x + dx, y + dy):
                    return x + dx, y + dy
        for dy in range(-r + 1, r):
            for dx in (-r, r):
                if transitable(w, h, gids, col, x + dx, y + dy):
                    return x + dx, y + dy
    raise AssertionError(f"landmass sin celdas transitables (ancla {x},{y})")


def main():
    # agrupar por landmass y abrir una sola vez cada nivel
    por_lm = {}
    for slug, nacion, nombre, tier, lm, pos in C.ASENTAMIENTOS:
        por_lm.setdefault(lm, []).append((slug, nacion, nombre, tier, pos))

    for lm, items in sorted(por_lm.items()):
        lm_level = f"assets/levels/mundi_landmass_{lm}.json"
        lm_map = f"assets/maps/mundi_landmass_{lm}.tmx"
        w, h, gids, col = cargar_tmx(BASE + lm_map)
        datos = json.load(open(BASE + lm_level))
        tocados = 0

        for slug, nacion, nombre, tier, ancla in items:
            canon = C.ID_CANON.get(slug) or ID_CANON_EXTRA.get(slug)
            nivel = C.nivel_exterior(slug)
            nivel_path = f"assets/levels/{nivel}.json"
            mios = json.load(open(BASE + nivel_path))
            start = mios["playerStart"]

            # 1. objeto-ciudad en el landmass
            obj = next((o for o in datos["objects"]
                        if o.get("objectId") == canon), None) if canon else None
            if obj is None and canon is None:
                obj = next((o for o in datos["objects"]
                            if o.get("_nombre") == nombre
                            and o.get("_nacion") == C.NACIONES[nacion]["nombre"]),
                           None)
            if obj is None:
                nuevo_id = canon or f"ciudad_en_{slug}"
                obj = {"objectId": nuevo_id,
                       "position": {"x": ancla[0], "y": ancla[1]},
                       "_nombre": nombre,
                       "_nacion": C.NACIONES[nacion]["nombre"]}
                datos["objects"].append(obj)
                print(f"  + marcador nuevo {nuevo_id} en lm{lm} "
                      f"({ancla[0]},{ancla[1]}) [{nombre}]")
            if "targetLevel" not in obj:
                # se respetan los enlaces ya puestos (por una pasada anterior
                # o por otro agente en landmass compartido)
                obj["targetLevel"] = nivel_path
                obj["targetPosition"] = {"x": start["x"], "y": start["y"]}
                tocados += 1

            # 2. salida del asentimiento -> landmass, celda caminable cercana.
            #    SIEMPRE se repone: gen_este_norte.py regenera los niveles
            #    desde cero y las salidas vuelven a nacer sin target.
            mx, my = snap(w, h, gids, col,
                          obj["position"]["x"], obj["position"]["y"])
            salida = next(o for o in mios["objects"]
                          if o.get("objectId") == f"salida_en_{slug}")
            salida["targetLevel"] = lm_level
            salida["targetPosition"] = {"x": mx, "y": my}
            open(BASE + nivel_path, "w").write(
                json.dumps(mios, indent=1, ensure_ascii=False) + "\n")

        open(BASE + lm_level, "w").write(
            json.dumps(datos, indent=1, ensure_ascii=False) + "\n")
        print(f"landmass_{lm}: {tocados} warps de ida enlazados")

    print("hecho (idempotente: re-ejecutar no cambia nada)")


if __name__ == "__main__":
    main()
