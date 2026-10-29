#!/usr/bin/env python3
"""Valida assets/catalogs/oficios.json y oficios_tablas.json (D0017, claude_oficios_datos01).

Comprueba: el contrato de D0017, los rangos, que los objectId existan (en los
catálogos de objetos) y estén puestos en algún nivel, que las competencias
existan (skills.json, etiqueta «narrativa») con su característica, que las
facciones existan (rules/facciones.json), los textos (<= 90 caracteres), el
objeto que se da y la tabla de pesca.

Uso:  python3 tools/validar_oficios.py [--raiz CARPETA_DEL_MOTOR]
Salida: 0 sin errores; 1 con errores. Los avisos no cambian la salida.
"""
import argparse
import glob
import json
import os
import sys

GRADOS_PAGA = ("roce", "exito", "critico")
TEXTOS = ("aceptar", "exito", "fallo", "critico", "pifia")
MAX_TEXTO = 90
CARACTERISTICAS = ("CON", "DES", "INT", "CAR")
FRANJAS = ("manana", "tarde", "noche")
RAREZAS = {"comun": "roce", "raro": "exito", "legendario": "critico"}
# Números modestos: tope de lo que paga un grado (coherente con ACTUAR-01 y las tiendas).
TOPE_ORO, TOPE_EXP, TOPE_REP = 60, 10, 5


def _json(ruta):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


def _entero(v):
    return isinstance(v, int) and not isinstance(v, bool)


class Datos:
    """Lo que hay en el motor y se cita desde los oficios."""

    def __init__(self, raiz):
        self.raiz = raiz
        a = os.path.join(raiz, "assets")
        self.objetos = {}  # id -> ficha
        for f in sorted(glob.glob(os.path.join(a, "objects", "*.json"))):
            if os.path.basename(f) == "previews_pendientes.json":
                continue
            try:
                d = _json(f)
            except (OSError, ValueError):
                continue
            lista = d.get("objects", []) if isinstance(d, dict) else d
            for o in lista:
                if isinstance(o, dict) and "id" in o:
                    self.objetos.setdefault(o["id"], o)
        self.colocados = {}  # objectId -> {(nivel, x, y)}
        for f in sorted(glob.glob(os.path.join(a, "levels", "*.json"))):
            try:
                d = _json(f)
            except (OSError, ValueError):
                continue
            nivel = "assets/levels/" + os.path.basename(f)
            for o in d.get("objects", []) if isinstance(d, dict) else []:
                p = o.get("position", {})
                self.colocados.setdefault(o.get("objectId"), set()).add((nivel, p.get("x"), p.get("y")))
        self.competencias = {}  # id -> característica
        ruta = os.path.join(a, "catalogs", "skills.json")
        if os.path.exists(ruta):
            for s in _json(ruta).get("entries", []):
                if "narrativa" in s.get("tags", []):
                    self.competencias[s["id"]] = s.get("castingStat")
        self.facciones = {}  # id -> {tiendas}
        ruta = os.path.join(a, "rules", "facciones.json")
        if os.path.exists(ruta):
            for fa in _json(ruta).get("facciones", []):
                self.facciones[fa["id"]] = set(fa.get("tiendas", []))
        self.monturas = set()
        ruta = os.path.join(a, "catalogs", "mounts.json")
        if os.path.exists(ruta):
            self.monturas = {m["id"] for m in _json(ruta).get("entries", [])}


def validar_oficios(cat, tablas, datos, propuesta_peces=None):
    """Devuelve (errores, avisos) como listas de textos."""
    E, W = [], []

    def err(m):
        E.append(m)

    def aviso(m):
        W.append(m)

    if not isinstance(cat, dict) or cat.get("version") != 1:
        err("oficios.json: falta 'version': 1")
        return E, W
    # --- rangos
    rangos = cat.get("rangos")
    if rangos is not None:
        if not isinstance(rangos, list) or not rangos:
            err("rangos: debe ser una lista no vacía")
        else:
            ant = None
            ids = set()
            for i, r in enumerate(rangos):
                for k in ("id", "nombre", "expDesde", "dados"):
                    if k not in r:
                        err("rangos[%d]: falta '%s'" % (i, k))
                if r.get("id") in ids:
                    err("rangos[%d]: id repetido '%s'" % (i, r.get("id")))
                ids.add(r.get("id"))
                if not _entero(r.get("expDesde")) or not _entero(r.get("dados")):
                    err("rangos[%d]: expDesde y dados deben ser enteros" % i)
                    continue
                if i == 0 and (r["expDesde"] != 0 or r["dados"] != 0):
                    err("rangos[0] debe empezar en expDesde 0 y 0 dados")
                if ant is not None and (r["expDesde"] <= ant["expDesde"] or r["dados"] <= ant["dados"]):
                    err("rangos[%d]: expDesde y dados deben subir" % i)
                if r["dados"] > 3:
                    err("rangos[%d]: más de 3 dados de oficio (tope de TopesDados)" % i)
                ant = r
    oficios = cat.get("oficios")
    if not isinstance(oficios, dict) or not oficios:
        err("oficios.json: falta 'oficios'")
        return E, W
    tabla_ids = set()
    if isinstance(tablas, dict):
        tabla_ids = {k for k in ("pesca", "herbolario", "mineria") if k in tablas}  # tanda 7: oficios de recolectar
    for oid, o in oficios.items():
        p = "oficios.%s" % oid
        for k in ("nombre", "verbo", "objetos", "tirada", "duracion", "descanso", "coste", "paga",
                  "faccion", "efecto", "objeto", "textos"):
            if k not in o:
                err("%s: falta '%s'" % (p, k))
        if any(k not in o for k in ("nombre", "verbo", "objetos", "tirada", "duracion", "descanso", "coste", "paga",
                                     "faccion", "efecto", "objeto", "textos")):
            continue
        tanda = o.get("_tanda")
        if not _entero(tanda) or not 1 <= tanda <= 7:
            err("%s: '_tanda' debe ser un entero de 1 a 7" % p)
        if not o["nombre"] or not o["verbo"]:
            err("%s: nombre y verbo no pueden estar vacíos" % p)
        # --- objetos (sitios)
        if not isinstance(o["objetos"], list):
            err("%s: 'objetos' debe ser una lista" % p)
        else:
            for ob in o["objetos"]:
                if ob not in datos.objetos:
                    err("%s: el objeto '%s' no existe en assets/objects" % (p, ob))
                elif ob not in datos.colocados:
                    err("%s: el objeto '%s' no está puesto en ningún nivel" % (p, ob))
            pend = o.get("_objetosPendientes", [])
            for ob in pend:
                if ob in datos.objetos:
                    aviso("%s: '%s' ya existe: pásalo de _objetosPendientes a objetos" % (p, ob))
            if not o["objetos"]:
                if not pend:
                    err("%s: sin objetos y sin _objetosPendientes" % p)
                elif tanda == 1:
                    err("%s: la tanda 1 necesita sitio ya existente" % p)
                else:
                    aviso("%s: sin sitio todavía (falta: %s)" % (p, ", ".join(pend)))
        # --- tirada
        t = o["tirada"]
        car, comp, riv = t.get("caracteristica"), t.get("competencia"), t.get("rival")
        if not _entero(riv) or not 0 <= riv <= 8:
            err("%s: tirada.rival debe ser un entero de 0 a 8" % p)
        elif riv == 0:
            if car or comp:
                err("%s: sin tirada (rival 0) pide caracteristica y competencia vacías" % p)
        else:
            if car not in CARACTERISTICAS:
                err("%s: caracteristica '%s' no existe (%s)" % (p, car, "/".join(CARACTERISTICAS)))
            if comp not in datos.competencias:
                err("%s: la competencia '%s' no existe en skills.json" % (p, comp))
            elif datos.competencias[comp] != car:
                err("%s: la competencia '%s' se tira con %s, no con %s" % (p, comp, datos.competencias[comp], car))
        # --- tiempos y coste
        if not _entero(o["duracion"]) or not 1 <= o["duracion"] <= 3:
            err("%s: duracion debe ser 1, 2 o 3 franjas" % p)
        if not _entero(o["descanso"]) or not 0 <= o["descanso"] <= 600:
            err("%s: descanso debe ser un entero de 0 a 600 pasos" % p)
        coste = o["coste"]
        if not isinstance(coste, dict) or not _entero(coste.get("oro")) or coste["oro"] < 0 or coste["oro"] > 60:
            err("%s: coste.oro debe ser un entero de 0 a 60" % p)
        # --- paga
        paga = o["paga"]
        previo = None
        for g in GRADOS_PAGA:
            pg = paga.get(g) if isinstance(paga, dict) else None
            if not isinstance(pg, dict):
                err("%s: falta paga.%s" % (p, g))
                previo = None
                continue
            ok = True
            for k, tope in (("oro", TOPE_ORO), ("exp", TOPE_EXP), ("reputacion", TOPE_REP)):
                v = pg.get(k)
                if not _entero(v) or v < 0 or v > tope:
                    err("%s: paga.%s.%s debe ser un entero de 0 a %d" % (p, g, k, tope))
                    ok = False
            if ok and previo is not None:
                for k in ("oro", "exp", "reputacion"):
                    if pg[k] < previo[k]:
                        err("%s: paga.%s.%s (%d) es menor que el grado anterior" % (p, g, k, pg[k]))
            previo = pg if ok else None
        if isinstance(paga, dict):
            for g in paga:
                if g not in GRADOS_PAGA + ("pifia", "pierde"):
                    err("%s: grado desconocido en paga: '%s'" % (p, g))
            for g in ("pifia", "pierde"):
                if g in paga:
                    oro = paga[g].get("oro", 0) if isinstance(paga[g], dict) else None
                    if not _entero(oro) or oro > 0 or oro < -20:
                        err("%s: paga.%s.oro debe ser un entero de -20 a 0" % (p, g))
            if o["tirada"].get("rival") and not any(paga.get(g, {}).get(k, 0) for g in GRADOS_PAGA for k in ("oro", "exp")) \
                    and not o["objeto"].get("cantidad"):
                aviso("%s: con tirada y sin paga ni objeto" % p)
        # --- faccion
        fa = o["faccion"]
        if fa != "" and fa not in datos.facciones:
            err("%s: la facción '%s' no existe en rules/facciones.json" % (p, fa))
        if fa == "" and any(paga.get(g, {}).get("reputacion", 0) for g in GRADOS_PAGA) and not o["objetos"]:
            aviso("%s: reputación con facción vacía y sin sitio todavía: indica la facción" % p)
        # --- efecto
        ef = o["efecto"]
        for k in ("pv", "fatiga"):
            if not _entero(ef.get(k)) or not -10 <= ef[k] <= 10:
                err("%s: efecto.%s debe ser un entero de -10 a 10" % (p, k))
        # --- objeto que da
        ob = o["objeto"]
        if not isinstance(ob.get("id"), str) or not _entero(ob.get("cantidad")) or ob["cantidad"] < 0 \
                or not isinstance(ob.get("calidadPorGrado"), bool):
            err("%s: objeto necesita id (texto), cantidad (>= 0) y calidadPorGrado (booleano)" % p)
        else:
            tabla = ob.get("tabla")
            if ob["id"]:
                if ob["id"] not in datos.objetos:
                    err("%s: el objeto que da, '%s', no existe en assets/objects" % (p, ob["id"]))
                if ob["cantidad"] < 1:
                    err("%s: da un objeto con cantidad 0" % p)
            elif tabla:
                if tabla not in tabla_ids:
                    err("%s: la tabla '%s' no está en oficios_tablas.json" % (p, tabla))
                if ob["cantidad"] < 1:
                    err("%s: da de una tabla con cantidad 0" % p)
            elif ob.get("receta") is True:
                pass  # tanda 7: lo que da lo dice recetas.json (tools/validar_recetas.py)
            elif ob["cantidad"]:
                err("%s: cantidad %d sin id ni tabla" % (p, ob["cantidad"]))
            tn = ob.get("tendero")
            if tn:
                if not any(tn in ts for ts in datos.facciones.values()):
                    err("%s: el tendero '%s' no es de ninguna facción" % (p, tn))
        # --- herramienta y montura
        h = o.get("herramienta", "")
        if h and h not in datos.objetos:
            err("%s: la herramienta '%s' no existe en assets/objects" % (p, h))
        m = o.get("montura")
        if m and datos.monturas and m not in datos.monturas:
            err("%s: la montura '%s' no existe en mounts.json" % (p, m))
        # --- textos
        tx = o["textos"]
        for k in TEXTOS:
            v = tx.get(k)
            if not isinstance(v, str) or not v.strip():
                err("%s: falta el texto '%s'" % (p, k))
            elif len(v) > MAX_TEXTO:
                err("%s: el texto '%s' tiene %d caracteres (máximo %d)" % (p, k, len(v), MAX_TEXTO))
        for k in tx:
            if k not in TEXTOS:
                aviso("%s: texto desconocido '%s'" % (p, k))
    # --- sitios compartidos por varios oficios: es normal, pero que se sepa
    # --- tabla de pesca
    if isinstance(tablas, dict):
        _validar_pesca(tablas, oficios, datos, propuesta_peces, err, aviso)
    elif any(o.get("objeto", {}).get("tabla") for o in oficios.values()):
        err("oficios_tablas.json: no se pudo leer")
    return E, W


def _validar_pesca(tablas, oficios, datos, propuesta, err, aviso):
    if tablas.get("version") != 1:
        err("oficios_tablas.json: falta 'version': 1")
    pesca = tablas.get("pesca")
    if not isinstance(pesca, dict):
        err("oficios_tablas.json: falta 'pesca'")
        return
    franjas = tuple(pesca.get("franjas", ()))
    if franjas != FRANJAS:
        err("pesca.franjas debe ser %s" % list(FRANJAS))
    peces = pesca.get("peces", {})
    usados = set()
    for pid, pz in peces.items():
        p = "pesca.peces.%s" % pid
        if pz.get("rareza") not in RAREZAS:
            err("%s: rareza '%s' desconocida" % (p, pz.get("rareza")))
            continue
        if pz.get("gradoMinimo") != RAREZAS[pz["rareza"]]:
            err("%s: gradoMinimo debe ser '%s' para un pez %s" % (p, RAREZAS[pz["rareza"]], pz["rareza"]))
        if not pz.get("nombre"):
            err("%s: sin nombre" % p)
        pr, ve = pz.get("precio"), pz.get("venta")
        if not _entero(pr) or not _entero(ve) or pr < 1 or ve < 1:
            err("%s: precio y venta deben ser enteros >= 1" % p)
        elif ve != max(1, pr * 25 // 100):
            err("%s: venta (%d) debe ser el 25 %% del precio (%d), división entera y mínimo 1 (ECONOMIA.md §2)" % (p, ve, pr))
        elif ve > 150:
            err("%s: venta %d demasiado alta" % (p, ve))
        if pid in datos.objetos:
            aviso("%s: ya existe como objeto: quita la propuesta" % p)
    sitios = pesca.get("sitios", {})
    if not sitios:
        err("pesca.sitios vacío")
    rangos = {"comun": 0, "raro": 1, "legendario": 2}
    maxventa = {"comun": 10, "raro": 40}
    leyendas = set()
    for sid, s in sitios.items():
        p = "pesca.sitios.%s" % sid
        if not s.get("nombre"):
            err("%s: sin nombre" % p)
        if not s.get("donde"):
            err("%s: sin 'donde'" % p)
        for d in s.get("donde", []):
            if d.get("objectId") not in datos.objetos:
                err("%s: objectId '%s' no existe" % (p, d.get("objectId")))
            elif (d.get("nivel"), d.get("x"), d.get("y")) not in datos.colocados.get(d["objectId"], set()):
                err("%s: '%s' no está en %s (%s,%s)" % (p, d["objectId"], d.get("nivel"), d.get("x"), d.get("y")))
            elif d["objectId"] not in oficios.get("pescar", {}).get("objetos", []):
                err("%s: '%s' no es un sitio del oficio 'pescar'" % (p, d["objectId"]))
        fr = s.get("franjas", {})
        if tuple(fr) != FRANJAS:
            err("%s: franjas debe tener %s" % (p, list(FRANJAS)))
        for fn, c in fr.items():
            if not c.get("comunes"):
                err("%s.%s: sin peces comunes" % (p, fn))
            if not c.get("raros"):
                err("%s.%s: sin peces raros" % (p, fn))
            for clave, rar in (("comunes", "comun"), ("raros", "raro")):
                for pid in c.get(clave, []):
                    usados.add(pid)
                    if pid not in peces:
                        err("%s.%s: el pez '%s' no está en pesca.peces" % (p, fn, pid))
                    elif peces[pid].get("rareza") != rar:
                        err("%s.%s: '%s' no es un pez %s" % (p, fn, pid, rar))
        lg = s.get("legendario", {})
        pid = lg.get("pez")
        if pid not in peces:
            err("%s: el legendario '%s' no está en pesca.peces" % (p, pid))
        else:
            usados.add(pid)
            leyendas.add(pid)
            if peces[pid].get("rareza") != "legendario":
                err("%s: '%s' no es legendario" % (p, pid))
        if not lg.get("franjas") or any(f not in FRANJAS for f in lg.get("franjas", [])):
            err("%s: legendario.franjas inválido" % p)
    for pid in peces:
        if pid not in usados:
            aviso("pesca.peces.%s: no sale en ningún sitio" % pid)
    if len(leyendas) != sum(1 for pz in peces.values() if pz.get("rareza") == "legendario"):
        aviso("hay legendarios que no son de ningún sitio")
    # --- propuesta de fichas
    if propuesta is not None:
        fichas = {o["id"]: o for o in propuesta.get("objects", [])}
        for pid, pz in peces.items():
            if pid in datos.objetos:
                continue
            f = fichas.get(pid)
            if f is None:
                err("pesca.peces.%s: ni existe como objeto ni hay ficha en propuesta/objetos_peces.json" % pid)
            elif f.get("price") != pz.get("precio"):
                err("pesca.peces.%s: precio %s en la tabla y %s en la ficha propuesta" % (pid, pz.get("precio"), f.get("price")))
            elif f.get("name") != pz.get("nombre"):
                err("pesca.peces.%s: nombre distinto en la tabla y en la ficha propuesta" % pid)
    else:
        for pid in peces:
            if pid not in datos.objetos:
                aviso("pesca.peces.%s: no existe como objeto (sin propuesta que lo cubra)" % pid)


def cargar_y_validar(raiz, ruta_propuesta=None):
    cat = _json(os.path.join(raiz, "assets", "catalogs", "oficios.json"))
    rt = os.path.join(raiz, "assets", "catalogs", "oficios_tablas.json")
    tablas = _json(rt) if os.path.exists(rt) else None
    prop = None
    if ruta_propuesta and os.path.exists(ruta_propuesta):
        prop = _json(ruta_propuesta)
    return validar_oficios(cat, tablas, Datos(raiz), prop)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--raiz", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    ap.add_argument("--propuesta", default=None,
                    help="propuesta/objetos_peces.json (por defecto, la del paquete si está al lado)")
    a = ap.parse_args(argv)
    prop = a.propuesta
    if prop is None:
        c = os.path.join(a.raiz, "out", "claude_oficios_datos01", "propuesta", "objetos_peces.json")
        prop = c if os.path.exists(c) else None
    try:
        E, W = cargar_y_validar(a.raiz, prop)
    except (OSError, ValueError) as ex:
        print("ERROR: no se pudo leer: %s" % ex)
        return 1
    for w in W:
        print("AVISO: " + w)
    for e in E:
        print("ERROR: " + e)
    n = len(_json(os.path.join(a.raiz, "assets", "catalogs", "oficios.json"))["oficios"])
    print("%d oficios; %d errores, %d avisos" % (n, len(E), len(W)))
    return 1 if E else 0


if __name__ == "__main__":
    sys.exit(main())
