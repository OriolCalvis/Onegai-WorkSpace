#!/usr/bin/env python3
"""AV19 - los niveles nuevos de Boundington y su alrededor (solo datos, sin C++).

Construye los niveles que pide el PLAN_15-10 §5 con el convenio de los interiores
que hay en HEAD (interior_banco, interior_castillo_p1 y compania): el suelo va en
el TMX con el tileset `terreno_mundo` y SIN tiles de colision; las paredes son
OBJETOS `paret_*` (bloquean) sobre celdas vacias (gid 0), y por eso solo se
levantan la fila 0 y la columna 0 (las dos caras que ve la isometrica) mas los
tabiques interiores. Las alfombras son tiles `int_catifa_*`, no objetos. Las
puertas de transicion son objetos `salida_*`/`escalera_*_sube|baja` con
`blocksMovement=false`, y su marco decorativo es `paret_<familia>_porta`.

Niveles que salen de aqui (AV19-05 primero; el resto, segun avance la cola):
  tienda_abandonada        la tienda del callejon del Comercial (AV17-06)
  mansion_barrio_alto      salon + jardin con ventana accesible (AV17-06)
  mansion_barrio_alto_p1   el piso de arriba de la mansion

Determinista y reejecutable: las puertas que planta en las ciudades se
reemplazan por su id en cada pasada, nunca se acumulan. Las celdas de conexion
se comprueban andables y libres en el nivel vivo: si un regenerado de la
ciudad las mueve, esto falla en vez de dejar una puerta dentro de una casa.

Uso:  python3 tools/gen_niveles_av19.py            # construye y escribe
      python3 tools/gen_niveles_av19.py --seco     # solo comprueba, no escribe
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import despejar_calles as D          # mismo cargador de nivel, TMX y catalogo que los validadores

BASE = D.BASE
SECO = "--seco" in sys.argv

# Suelos por nombre del tileset terreno_mundo (assets/textures/terreno_iso.json).
_T = {t["nombre"]: t["gid"] for t in json.load(open(BASE + "assets/textures/terreno_iso.json"))["tiles"]}
BUIT = 0                              # celda vacia: la pared la pone el objeto paret_*
FUSTA, MARBRE = _T["int_taulons"], _T["int_marbre"]
CESPED, FLORES = _T["pradera"], _T["hierba_flores"]
# Alfombra roja por piezas, como la azul de interior_banco.tmx (c=179 -> 170).
CAT_C, CAT_XM, CAT_XP, CAT_YM, CAT_YP = 170, 171, 172, 173, 174
CAT_XX_YY, CAT_XP_YY, CAT_XX_Yp, CAT_XP_Yp = 175, 177, 176, 178

# Conexiones con las ciudades (AV19-05). Se comprueban andables y libres al
# generar; si un regenerado de la ciudad las come, esto falla con el motivo.
CONEXIONES = {
    "ciudad_centro": [
        # la trasera de la tienda, en el callejon del Comercial (x 9-10, filas 22-24)
        {"objectId": "puerta_tienda_abandonada", "position": (10, 23),
         "targetLevel": "assets/levels/tienda_abandonada.json", "entra_en": (1, 5)},
        # la entrada principal, cerrada con tabla: se ve desde la calle, no se entra
        {"objectId": "puerta_cerrada_tienda", "position": (10, 25), "props": {
            "displayName": "Puerta de la tienda (cerrada con tabla)", "eventId": "tienda_puerta_cerrada"}},
    ],
    "ciudad_este": [
        # la puerta de la mansion, en el portillo del seto de su parcela
        {"objectId": "puerta_mansion_barrio_alto", "position": (24, 21),
         "targetLevel": "assets/levels/mansion_barrio_alto.json", "entra_en": (1, 14)},
        # la ventana del jardin, por donde se cuela quien no llama a la puerta
        {"objectId": "ventana_mansion_barrio_alto", "position": (17, 16),
         "targetLevel": "assets/levels/mansion_barrio_alto.json", "entra_en": (14, 10)},
    ],
    "ciudad_surysal": [
        # la senda que sale de los campamentos del vado hacia las afueras (AV19-01)
        {"objectId": "puerta_afueras_boundington", "position": (15, 46),
         "targetLevel": "assets/levels/afueras_boundington.json", "entra_en": (20, 2)},
    ],
    "afueras_boundington": [
        # AV19-03: el sendero a la montana deja de ser un marcador y se vuelve puerta.
        # Se llega a la casilla del inicio de la montana, no encima de su salida, para
        # no rebotar (el convenio de las otras puertas: llegar al lado, no encima).
        {"objectId": "sendero_a_montana_gorran", "position": (20, 30),
         "targetLevel": "assets/levels/montana_gorran.json", "entra_en": (13, 33)},
    ],
}


class Nivel:
    def __init__(self, nombre, titulo, w, h, suelo=FUSTA, colision=()):
        self.nombre, self.titulo = nombre, titulo
        self.w, self.h = w, h
        self.g = [[suelo] * w for _ in range(h)]
        self.objetos = []
        self.start = (1, 1)
        # Gids que bloquean el paso, por nombre de tile: en exteriores el agua,
        # la roca y las casas son colision de tile (en interiores la ponen los
        # objetos paret_*). Se declaran en el TMX, como hace gen_ciudad.py.
        self.colision = {_T[n] for n in colision}
        self.copiar_tmx = None        # si se pone un nombre, el TMX se copia de ese nivel
        self.alturas = []             # AV19-02/AV19-03: regiones de altura del terreno

    def altura(self, x, y, w, h, nivel, hasta=None, eje=None):
        """Una region de altura (FORMATO_NIVELES, "Altura del terreno", AV19-02 de
        Claude). Sin `hasta` es una meseta plana; con `hasta` y `eje` es una rampa que
        va de `nivel` a `hasta` a lo largo de ese eje. Las rampas de esta casa son
        siempre de dos casillas: con mas, el reparto intermedio depende del redondeo
        del motor y la guarda no podria reproducirlo."""
        r = {"x": x, "y": y, "w": w, "h": h, "nivel": nivel}
        if hasta is not None:
            assert abs(nivel - hasta) == 1 and (h if eje == "y" else w) == 2, \
                f"rampa {r} hasta={hasta}: solo de un nivel y de dos casillas"
            r["hasta"], r["eje"] = hasta, eje
        self.alturas.append(r)
        return r

    def rect(self, x, y, w, h, t):
        for yy in range(max(0, y), min(y + h, self.h)):
            for xx in range(max(0, x), min(x + w, self.w)):
                self.g[yy][xx] = t

    def catifa(self, x, y):
        """Alfombra roja de 3x3 por piezas, como la azul de interior_banco.tmx."""
        for i, fila in enumerate(([CAT_XX_YY, CAT_YM, CAT_XP_YY],
                                  [CAT_XM, CAT_C, CAT_XP],
                                  [CAT_XX_Yp, CAT_YP, CAT_XP_Yp])):
            for j, t in enumerate(fila):
                self.g[y + i][x + j] = t

    def ocupades(self):
        return {(o["position"]["x"], o["position"]["y"]) for o in self.objetos}

    def paret(self, oid, celdas, variantes=()):
        """Pared decorada sobre celdas vacias; salta las ocupadas (puertas,
        escaleras) y aplica las variantes puntuales (quadre, finestra...)."""
        por_celda = {(x, y): v for (x, y, v) in variantes}
        ocup = self.ocupades()
        for (x, y) in celdas:
            if self.g[y][x] == BUIT and (x, y) not in ocup:
                self.obj(por_celda.get((x, y), oid), x, y)

    def murs(self, oid, w, h, variantes=()):
        """Las dos caras que ve la isometrica: fila 0 y columna 0."""
        self.paret(oid, [(x, 0) for x in range(w)] + [(0, y) for y in range(1, h)], variantes)

    def obj(self, oid, x, y, **campos):
        o = {"objectId": oid, "position": {"x": x, "y": y}}
        o.update(campos)
        self.objetos.append(o)
        return o

    def salida(self, oid, x, y, nivel, pos):
        """Una puerta de salida. La casilla de llegada se comprueba AQUI, no despues:
        aterrizar sobre roca o sobre un objeto que bloquea es meter al jugador dentro
        de una pared, y validar_puertas solo lo dice cuando el nivel ya esta escrito."""
        ruta = BASE + f"assets/levels/{nivel}.json"
        if os.path.exists(ruta):
            dlvl, dg, dcol, _ = D.cargar_nivel(nivel)
            assert dg[pos[1]][pos[0]] not in dcol, \
                f"{self.nombre}: {oid} aterriza en colision de {nivel} en {pos}"
            cat = D.catalogo()
            tapado = [o["objectId"] for o in dlvl["objects"]
                      if (o["position"]["x"], o["position"]["y"]) == tuple(pos)
                      and cat.get(o["objectId"], {}).get("blocksMovement")
                      and not cat.get(o["objectId"], {}).get("flotante")]
            assert not tapado, \
                f"{self.nombre}: {oid} aterriza en {nivel} {pos}, tapado por {tapado}"
        return self.obj(oid, x, y, targetLevel=f"assets/levels/{nivel}.json",
                        targetPosition={"x": pos[0], "y": pos[1]})

    def escribir(self):
        assert self.titulo.isascii(), f"{self.nombre}: el titulo tiene caracteres no ASCII"
        assert self.g[self.start[1]][self.start[0]] != BUIT, \
            f"{self.nombre}: el inicio ({self.start}) cae en una celda vacia"
        csv = ",\n".join(",".join(str(v) for v in fila) for fila in self.g)
        # Cada bloque lleva su salto de linea y, si no hay colision, el resultado
        # es vacio: con un `"\n".join` la plantilla dejaba una linea en blanco
        # dentro de <tileset> y cada regenerado ensuciaba el diff de los
        # interiores (que no tienen tiles de colision).
        tiles = "".join(
            f'  <tile id="{gid - 1}">\n   <properties>\n'
            f'    <property name="collision" type="bool" value="true"/>\n'
            f'   </properties>\n  </tile>\n' for gid in sorted(self.colision))
        tmx = f'''<?xml version="1.0" encoding="UTF-8"?>
<!-- GENERADO por tools/gen_niveles_av19.py - no editar a mano. {self.titulo}.
     La fila 0 y la columna 0 son las paredes (objetos paret_*): sin baldosa. -->
<map version="1.10" orientation="orthogonal" renderorder="right-down" width="{self.w}" height="{self.h}" tilewidth="64" tileheight="32" infinite="0" nextlayerid="2" nextobjectid="1">
 <tileset firstgid="1" name="terreno_mundo" tilewidth="128" tileheight="64" tile_count="203" columns="8">
  <image source="../textures/../textures/terreno_mundo.png" width="1024" height="1664"/>
{tiles} </tileset>
 <layer id="1" name="suelo" width="{self.w}" height="{self.h}">
  <data encoding="csv">
{csv}
</data>
 </layer>
</map>
'''
        nivel = {"name": self.titulo, "map": f"assets/maps/{self.nombre}.tmx",
                 "playerStart": {"x": self.start[0], "y": self.start[1]}, "objects": self.objetos}
        if self.alturas:
            # Campo opcional de AV19-02: sin el, todo plano. El motor de hoy lo ignora
            # (comprobado con el binario: el nivel carga y juega igual), y cuando la
            # altura entre, el sendero sube solo.
            nivel["alturas"] = self.alturas
        if SECO:
            print(f"  {self.nombre}: {self.w}x{self.h}, {len(self.objetos)} objetos"
                  + (f", {len(self.alturas)} regiones de altura" if self.alturas else "") + " (en seco)")
            return
        if self.copiar_tmx:
            import shutil
            shutil.copyfile(BASE + f"assets/maps/{self.copiar_tmx}.tmx", BASE + f"assets/maps/{self.nombre}.tmx")
        else:
            open(BASE + f"assets/maps/{self.nombre}.tmx", "w").write(tmx)
        open(BASE + f"assets/levels/{self.nombre}.json", "w").write(
            json.dumps(nivel, indent=2, ensure_ascii=False) + "\n")
        print(f"  {self.nombre}: {self.w}x{self.h}, {len(self.objetos)} objetos")


def catalogo_union():
    cat = {}
    for nombre in os.listdir(BASE + "assets/objects"):
        if not nombre.endswith(".json"):
            continue
        try:
            for o in json.load(open(BASE + "assets/objects/" + nombre)).get("objects", []):
                cat[o["id"]] = o
        except (OSError, ValueError):
            pass
    return cat


def conectar_ciudades(cat):
    """Planta en cada ciudad las puertas de ida a los niveles nuevos, y comprueba
    que la celda elegida es andable y libre en el nivel vivo."""
    for ciudad, puertas in CONEXIONES.items():
        ruta = BASE + f"assets/levels/{ciudad}.json"
        lvl = json.load(open(ruta))
        _, g, col, _ = D.cargar_nivel(ciudad)
        ids = {p["objectId"] for p in puertas}
        lvl["objects"] = [o for o in lvl["objects"] if o["objectId"] not in ids]
        for p in puertas:
            x, y = p["position"]
            assert g[y][x] not in col, f"{ciudad}: la celda ({x}, {y}) de {p['objectId']} es colision"
            assert not any(o["position"] == {"x": x, "y": y} for o in lvl["objects"]), \
                f"{ciudad}: la celda ({x}, {y}) de {p['objectId']} ya esta ocupada"
            o = {"objectId": p["objectId"], "position": {"x": x, "y": y}}
            if "targetLevel" in p:
                o["targetLevel"] = p["targetLevel"]
                o["targetPosition"] = {"x": p["entra_en"][0], "y": p["entra_en"][1]}
            o.update(p.get("props", {}))
            lvl["objects"].append(o)
        if not SECO:
            open(ruta, "w").write(json.dumps(lvl, indent=2, ensure_ascii=False) + "\n")
        print(f"  {ciudad}: {len(puertas)} puertas a los niveles nuevos")


def revisar_ids(niveles, cat):
    """Ningun objectId inventado: un id que no esta en los catalogos del proyecto
    no da error de carga, simplemente no se dibuja (FORMATO_NIVELES, error 2).
    `cat` son los catalogos de boundington, no los de todo assets/objects/."""
    fallos = []
    for n in niveles:
        for o in n.objetos:
            if o["objectId"] not in cat:
                fallos.append(f"{n.nombre}: {o['objectId']} no esta en ningun catalogo")
    for ciudad, puertas in CONEXIONES.items():
        for p in puertas:
            if p["objectId"] not in cat:
                fallos.append(f"{ciudad}: {p['objectId']} no esta en ningun catalogo")
    return fallos


def manifiesto(nombres):
    ruta = BASE + "assets/proyectos/boundington.json"
    man = json.load(open(ruta))
    for n in nombres:
        f = f"{n}.json"
        if f not in man["levels"]:
            man["levels"].append(f)
    if "boundington_av19_objetos.json" not in man["catalogs"]:
        man["catalogs"].append("boundington_av19_objetos.json")
    if not SECO:
        # Sangria de 2 y un salto final: es la forma canonica del motor, y
        # `test_project_manifests` compara el fichero BYTE a byte con lo que
        # reescribe el motor (tests/Project/ManifestTests.cpp). Con indent=1 el
        # test ponia "canonical byte round-trip changed manifest".
        texto = json.dumps(man, indent=2, ensure_ascii=False) + "\n"
        open(ruta, "w").write(texto)
        assert texto == json.dumps(json.load(open(ruta)), indent=2, ensure_ascii=False) + "\n", \
            "manifiesto: el fichero escrito no vuelve a salir igual (no es canonico)"
    print(f"  manifiesto: {len(man['levels'])} niveles, {len(man['catalogs'])} catalogos")


# Los niveles, en funciones separadas: cada tarea de la cola anade la suya.
if __name__ == "__main__":
    from av19_niveles import NIVELES_AV1901, NIVELES_AV1903, NIVELES_AV1904, NIVELES_AV1905, NIVELES_AV1906
    cat = D.catalogo()          # solo los catalogos del proyecto boundington
    niveles = NIVELES_AV1905() + NIVELES_AV1901() + NIVELES_AV1906() + NIVELES_AV1904() + NIVELES_AV1903()
    fallos = revisar_ids(niveles, cat)
    assert not fallos, "ids que no existen en ningun catalogo:\n  " + "\n  ".join(fallos)
    for n in niveles:
        n.escribir()
    conectar_ciudades(cat)
    manifiesto([n.nombre for n in niveles])
    print("  (en seco: nada escrito)" if SECO else "  escrito")
