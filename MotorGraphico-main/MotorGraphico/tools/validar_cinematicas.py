#!/usr/bin/env python3
"""PROLOGO-02 — Comprueba assets/cinematicas/cinematicas.json contra lo demás:

  - los niveles que nombra existen;
  - quien anda, se coloca o mira está en el nivel donde lo hace (o es «jugador»);
  - las celdas caen dentro del mapa;
  - cada «beat» lo pide algún beat de las aventuras (si no, la cinemática se
    para a esperar un diálogo que nunca sale: no se atasca, pero no dice nada);
  - lo que «requiere» una cinemática lo enciende alguien (un beat, una opción u
    otra cinemática);
  - los objetos de las apariciones y de los espectros existen en los catálogos;
  - una aparición con «nivel» apunta a un nivel que existe y donde el objeto está colocado;
  - E-DISPARADORES-1 (D0026): el «disparador» de una cinemática. El foco (lleva.a o el centro de la zona) y el
    destino de «lleva» están dentro del nivel y son transitables (ni tile con colisión ni objeto que bloquee); una
    sola «accion» (cinematica | beat | marca | combate: un enemigo del catálogo) que existe; «radio» 0..20; «ambito» del nivel válido;
    AVISO si menos del 50 % de las casillas transitables de la zona ven el foco (la zona sigue valiendo, pero el
    radio no cubre a quien llega por ahí); AVISO si el centro de una zona antigua no es transitable (el arnés y
    el explorador lo pisan); AVISO si «once» es false (E5 aún no rearma).

    python3 tools/validar_cinematicas.py      # 0 errores -> sale con 0
"""
import glob
import json
import os
import re
import sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def lee(rel):
    return json.load(open(os.path.join(RAIZ, rel), encoding="utf-8"))


def lista(v):
    return [v] if isinstance(v, str) else list(v or [])


# ---- E-DISPARADORES-1: el nivel como lo ve el motor (colision de tiles, objetos que bloquean o tapan la vista)

AMBITOS = ("interior_pequeno", "interior_grande", "exterior")
RADIO_AMBITO = {"interior_pequeno": None, "interior_grande": 5, "exterior": 10}  # None: al entrar
PREFIJOS_INTERIOR = ("interior_", "mansion_", "casa_", "tienda_", "dependencias_", "mina", "cueva", "galeria",
                     "camara_", "grieta", "santuario", "mazmorra", "antecova")


def ambito_deducido(nivel_ruta, ancho, alto):
    """Misma regla que include/Level/AmbitoNivel.h (D0026.7): nombre del fichero y, en un interior, tamaño (< 200 = pequeño)."""
    n = os.path.basename(nivel_ruta)
    n = n[:-5] if n.endswith(".json") else n
    interior = n.startswith(PREFIJOS_INTERIOR) or bool(re.fullmatch(r"aldea_.*_p\d+", n))
    if not interior:
        return "exterior"
    return "interior_pequeno" if ancho * alto < 200 else "interior_grande"


def rejilla_tmx(texto):
    """(ancho, alto, set de (x, y) con colisión en alguna capa) de un TMX con tilesets embebidos (como lee el motor)."""
    m = re.search(r'<map [^>]*width="(\d+)" height="(\d+)"', texto)
    w, h = int(m.group(1)), int(m.group(2))
    texto = re.sub(r"<!--.*?-->", "", texto, flags=re.S)
    colision = set()
    for ts in re.finditer(r'<tileset firstgid="(\d+)"[^>]*>(.*?)</tileset>', texto, re.S):
        base = int(ts.group(1))
        colision.update(base + int(i) for i in re.findall(
            r'<tile id="(\d+)">\s*<properties>\s*<property name="collision"[^/]*value="true"', ts.group(2)))
    celdas = set()
    for datos in re.findall(r'<data encoding="csv">(.*?)</data>', texto, re.S):
        g = [int(v) & 0x0FFFFFFF for v in datos.replace("\n", "").split(",") if v.strip()]
        for i, gid in enumerate(g):
            if gid in colision:
                celdas.add((i % w, i // w))
    return w, h, celdas


def linea_de_vision(a, b, tapa):
    """Bresenham de centro a centro, extremos fuera (Game/LineaDeVision.h)."""
    x, y = a
    dx, dy = abs(b[0] - a[0]), -abs(b[1] - a[1])
    sx, sy = (1 if a[0] < b[0] else -1), (1 if a[1] < b[1] else -1)
    err = dx + dy
    while (x, y) != tuple(b):
        e2 = 2 * err
        if e2 >= dy:
            err += dy
            x += sx
        if e2 <= dx:
            err += dx
            y += sy
        if (x, y) == tuple(b):
            break
        if tapa(x, y):
            return False
    return True


def punto_de_entrada(c):
    """Cinematica::puntoDeEntrada sin 'transitable': lleva.a o el centro de la zona; None si no hay."""
    d = c.get("disparador") or {}
    if "lleva" in d and "a" in d["lleva"]:
        return tuple(d["lleva"]["a"])
    if "zona" in c:
        x0, y0, x1, y1 = c["zona"]
        x0, x1, y0, y1 = min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1)
        return ((x0 + x1) // 2, (y0 + y1) // 2)
    return None


def main():
    ruta = "assets/cinematicas/cinematicas.json"
    if not os.path.exists(os.path.join(RAIZ, ruta)):
        print("validar_cinematicas: no hay cinematicas")
        return 0
    cat = lee(ruta)
    errores, avisos = [], []

    # lo que piden y lo que encienden las aventuras
    pedidas, encendidas = set(), set()
    for aventura in glob.glob(os.path.join(RAIZ, "assets/adventures/*.json")):
        d = json.load(open(aventura, encoding="utf-8"))
        for b in d.get("beats", []):
            pedidas.update((b.get("requires") or {}).get("allFlags", []))
            efs = list(b.get("effects", []))
            for o in b.get("opciones", []):
                efs += o.get("effects", [])
            for e in efs:
                if e.get("type") == "setFlag":
                    encendidas.add(e.get("arg"))
                sb = e.get("startBattle") or {}
                encendidas.update(v for k, v in sb.items() if k in ("flagVictory", "flagDefeat", "flagHuida") and isinstance(v, str))
                sc = e.get("skillCheck") or {}
                encendidas.update(v for k, v in sc.items() if k.startswith("flag"))
    # C8-04a: los eventos del mundo también encienden banderas (sus efectos)
    for fev in glob.glob(os.path.join(RAIZ, "assets/eventos/*.json")):
        for ev in json.load(open(fev, encoding="utf-8")).get("eventos", []):
            for e in ev.get("efectos", []) + ev.get("alCaducar", []):
                if e.get("type") == "setFlag":
                    encendidas.add(e.get("arg"))
            for fase in ev.get("fases", []):
                for e in fase.get("efectos", []):
                    if e.get("type") == "setFlag":
                        encendidas.add(e.get("arg"))
    for c in cat.get("cinematicas", []):
        encendidas.add(f"cine_{c['id']}_vista")
        encendidas.update({f"cine_{c['id']}_ganado", f"cine_{c['id']}_perdido"})  # E4: las del disparador combate
        encendidas.update(lista(c.get("alTerminar")))
        for p in c.get("pasos", []):
            if "flag" in p:
                encendidas.add(p["flag"])
            if "beat" in p:
                encendidas.add(p["beat"])
    # el juego enciende estas (el cofre, los temporizadores)
    encendidas.update({"kit_basico_recogido", "kit_basico_puesto", "bnd_prologo_kit_listo"})

    proyecto = lee("assets/proyectos/boundington.json")
    ids, defs = set(), {}
    for c in proyecto["catalogs"]:
        p = os.path.join(RAIZ, "assets/objects", c)
        if os.path.exists(p):
            for o in json.load(open(p, encoding="utf-8")).get("objects", []):
                ids.add(o["id"])
                defs[o["id"]] = o
                if o.get("tapaVista") and not (o.get("blocksMovement") and o.get("category") == "prop"):
                    errores.append(f"objeto «{o['id']}»: tapaVista solo vale en un prop que bloquea el paso")

    def nivel(rel):
        p = os.path.join(RAIZ, rel)
        if not os.path.exists(p):
            return None
        d = json.load(open(p, encoding="utf-8"))
        tmx = open(os.path.join(RAIZ, d["map"]), encoding="utf-8").read()
        w, h, colision = rejilla_tmx(tmx)
        bloquea, tapa = set(), set()  # E-DISPARADORES-1: casillas con un objeto que bloquea el paso / tapa la vista
        for o in d.get("objects", []):
            pos = o.get("position") or {}
            celda = (pos.get("x", -1), pos.get("y", -1))
            df = defs.get(o["objectId"], {})
            if df.get("blocksMovement"):
                bloquea.add(celda)
            if df.get("tapaVista"):
                tapa.add(celda)
        return {"objetos": {o["objectId"] for o in d.get("objects", [])}, "w": w, "h": h, "colision": colision,
                "bloquea": bloquea, "tapa": tapa, "ambito": d.get("ambito"), "ruta": rel}

    for c in cat.get("cinematicas", []):
        donde = f"cinematica «{c['id']}»"
        actual = nivel(c["nivel"]) if c.get("nivel") else None
        if c.get("nivel") and actual is None:
            errores.append(f"{donde}: no existe el nivel {c['nivel']}")
        for f in lista(c.get("requiere")):
            if f not in encendidas:
                errores.append(f"{donde}: pide «{f}» y no la enciende nadie")
        for k, p in enumerate(c.get("pasos", [])):
            d = f"{donde}, paso {k}"
            if "nivel" in p:
                actual = nivel(p["nivel"])
                if actual is None:
                    errores.append(f"{d}: no existe el nivel {p['nivel']}")
            quien = p.get("anda") or p.get("coloca") or p.get("mira")
            if quien and quien != "jugador" and actual is not None and quien not in actual["objetos"]:
                errores.append(f"{d}: «{quien}» no está en ese nivel")
            celdas = ([p["a"]] if "a" in p else []) + list(p.get("por", [])) + ([p["en"]] if "en" in p else [])
            for x, y in celdas:
                if actual is not None and not (0 <= x < actual["w"] and 0 <= y < actual["h"]):
                    errores.append(f"{d}: la celda {(x, y)} cae fuera del mapa")
            if "beat" in p and p["beat"] not in pedidas:
                avisos.append(f"{d}: ningún beat pide «{p['beat']}» (no dirá nada)")
    # ---- E-DISPARADORES-1 (D0026): el disparador, y el centro de las zonas de siempre
    ids_cine = {c["id"] for c in cat.get("cinematicas", [])}
    ambitos_avisados = set()
    for nivel_json in glob.glob(os.path.join(RAIZ, "assets/levels/*.json")):
        try:
            with open(nivel_json, encoding="utf-8") as f:
                declarado = json.load(f).get("ambito")
        except (OSError, ValueError):
            continue
        if declarado is not None and declarado not in AMBITOS:
            errores.append(f"nivel {os.path.basename(nivel_json)}: ambito «{declarado}» no es {'/'.join(AMBITOS)}")
    for c in cat.get("cinematicas", []):
        donde = f"cinematica «{c['id']}»"
        lv = nivel(c["nivel"]) if c.get("nivel") else None
        d = c.get("disparador")
        foco = punto_de_entrada(c)

        def dentro(cel):
            return lv is not None and 0 <= cel[0] < lv["w"] and 0 <= cel[1] < lv["h"]

        def transitable(cel):  # ni tile con colision ni objeto que bloquee (lo que pisa GameSession::rutaAndando)
            return dentro(cel) and tuple(cel) not in lv["colision"] and tuple(cel) not in lv["bloquea"]

        def tapa(x, y):
            return (x, y) in lv["colision"] or (x, y) in lv["tapa"]

        if d is None:  # una zona de siempre: solo se avisa (no cambia nada)
            if lv is not None and "zona" in c and foco is not None and dentro(foco) and not transitable(foco):
                avisos.append(f"{donde}: el centro de la zona {foco} no es transitable (el arnés y el explorador lo pisan "
                              "igual); declara un disparador.lleva.a transitable")
            continue
        if not isinstance(d, dict):
            errores.append(f"{donde}: «disparador» tiene que ser un objeto")
            continue
        for k in d:
            if k not in ("lleva", "accion", "once", "radio", "vision"):
                errores.append(f"{donde}: disparador.{k} no existe (lleva, accion, once, radio, vision)")
        lleva = d.get("lleva")
        if lleva is not None:
            a_ok = isinstance(lleva, dict) and isinstance(lleva.get("a"), list) and len(lleva["a"]) == 2
            if not a_ok:
                errores.append(f"{donde}: disparador.lleva necesita «a»: [x, y]")
            else:
                if lleva.get("como", "andando") not in ("andando", "fundido"):
                    errores.append(f"{donde}: disparador.lleva.como es andando o fundido")
                if lleva.get("mira", "arriba") not in ("arriba", "abajo", "izquierda", "derecha"):
                    errores.append(f"{donde}: disparador.lleva.mira es arriba, abajo, izquierda o derecha")
        acc = d.get("accion")
        if acc is not None:
            if not isinstance(acc, dict) or len(acc) != 1:
                errores.append(f"{donde}: disparador.accion lleva exactamente una clave (cinematica, beat, marca o combate)")
            else:
                clave, valor = next(iter(acc.items()))
                if clave not in ("cinematica", "beat", "marca", "combate"):
                    errores.append(f"{donde}: la accion «{clave}» no existe (cinematica, beat, marca o combate)")
                elif not isinstance(valor, str) or not valor:
                    errores.append(f"{donde}: disparador.accion.{clave} necesita un id")
                elif clave == "combate" and defs.get(valor, {}).get("category") != "enemy":
                    errores.append(f"{donde}: la accion combate «{valor}» no es un enemigo del catalogo")
                elif clave == "cinematica" and valor not in ids_cine:
                    errores.append(f"{donde}: la accion apunta a la cinematica «{valor}», que no existe")
                elif clave == "beat" and valor not in pedidas:
                    avisos.append(f"{donde}: ningún beat pide «{valor}» (no dirá nada)")
                if clave in ("beat", "marca") and c.get("pasos"):
                    avisos.append(f"{donde}: su accion es {clave}: los «pasos» propios no se juegan")
        if d.get("once", True) not in (True, False):
            errores.append(f"{donde}: disparador.once es true o false")
        radio = d.get("radio", "nivel")
        if radio != "nivel" and not (isinstance(radio, int) and not isinstance(radio, bool) and 0 <= radio <= 20):
            errores.append(f"{donde}: disparador.radio es un entero 0..20 o \"nivel\"")
        if foco is None and ("radio" in d or "vision" in d):
            errores.append(f"{donde}: tiene radio/vision pero ni zona ni lleva.a: no hay foco")
        if lv is None:
            if foco is not None or "zona" in c:
                avisos.append(f"{donde}: sin «nivel» no se puede comprobar el foco ni la zona")
            continue
        if "zona" in c:
            x0, y0, x1, y1 = c["zona"]
            for cel in ((x0, y0), (x1, y1)):
                if not dentro(cel):
                    errores.append(f"{donde}: la zona {c['zona']} sale del nivel ({lv['w']}x{lv['h']})")
                    break
        if lleva is not None and isinstance(lleva.get("a"), list) and len(lleva["a"]) == 2:
            a_cel = tuple(lleva["a"])
            if not dentro(a_cel):
                errores.append(f"{donde}: disparador.lleva.a {a_cel} cae fuera del nivel ({lv['w']}x{lv['h']})")
            elif not transitable(a_cel):
                errores.append(f"{donde}: disparador.lleva.a {a_cel} no es transitable (muro o objeto que bloquea)")
        if foco is not None and foco != (tuple(lleva["a"]) if lleva and isinstance(lleva.get("a"), list) else None):
            if not dentro(foco):
                errores.append(f"{donde}: el foco {foco} (centro de la zona) cae fuera del nivel")
            elif not transitable(foco):
                errores.append(f"{donde}: el foco {foco} (centro de la zona) no es transitable; declara disparador.lleva.a")
        # el ambito del nivel: declarado, o deducido (se avisa una vez por nivel)
        if lv["ambito"] is None and lv["ruta"] not in ambitos_avisados:
            ambitos_avisados.add(lv["ruta"])
            avisos.append(f"nivel {lv['ruta']}: sin «ambito»; se deduce {ambito_deducido(lv['ruta'], lv['w'], lv['h'])} "
                          f"({lv['w']}x{lv['h']}) para «{c['id']}»")
        # la zona sigue valiendo, pero el radio solo cubre a quien ve el foco: cuantas casillas de la zona lo ven
        if "zona" in c and foco is not None and dentro(foco):
            x0, y0, x1, y1 = c["zona"]
            pisables = [(x, y) for x in range(min(x0, x1), max(x0, x1) + 1) for y in range(min(y0, y1), max(y0, y1) + 1)
                        if transitable((x, y))]
            if pisables:
                ven = sum(1 for cel in pisables if linea_de_vision(cel, foco, tapa))
                if ven * 2 < len(pisables):
                    avisos.append(f"{donde}: solo {ven} de {len(pisables)} casillas transitables de la zona ven el foco {foco} "
                                  f"({100 * ven // len(pisables)} %): por el radio no disparará a quien llegue por las demás")
    for a in cat.get("apariciones", []):
        if a["objeto"] not in ids:
            errores.append(f"aparición: «{a['objeto']}» no está en ningún catálogo")
        for n in lista(a.get("nivel")):
            lv = nivel(n)
            if lv is None:
                errores.append(f"aparición «{a['objeto']}»: no existe el nivel {n}")
            elif a["objeto"] not in lv["objetos"]:
                errores.append(f"aparición «{a['objeto']}»: no está colocado en {n}")
    for o in cat.get("espectro", {}).get("objetos", []):
        if o not in ids:
            errores.append(f"espectro: «{o}» no está en ningún catálogo")

    for e in errores:
        print("ERROR  ", e)
    for a in avisos:
        print("aviso  ", a)
    print(f"validar_cinematicas: {len(cat.get('cinematicas', []))} cinematicas, {len(errores)} errores, {len(avisos)} avisos")
    return 1 if errores else 0


if __name__ == "__main__":
    sys.exit(main())
