#!/usr/bin/env python3
"""matriz_decisiones.py — E-MATRIZ-01: inventario de las decisiones importantes de un dia de Boundington.

Una DECISION IMPORTANTE es una opcion de dialogo (o el resultado de una tirada / combate que cuelga de ella) cuyas
flags o variables se LEEN despues del dia: en el `requires` de un beat de un dia posterior, en un evento del mundo
(assets/eventos/), en una cinematica posterior (assets/cinematicas/) o en el codigo (src/); o que mueve la moral
(`acto`), el oro o el nivel de forma notable. Se calcula a mano en el sentido de que no se mira el juego, sino los
JSON: nada de lo que dice aqui depende de ojo.

Alcance de la lectura "despues":
  * directa: la flag/variable que pone la opcion.
  * un salto: las flags que pone el `effects` de un beat del mismo dia cuyo `requires` positivo (allFlags/anyFlags)
    contiene una flag de la opcion («via b1_x»). Se marca como indirecta.
Lo que el script NO sabe: los bloqueos por AND con otras flags (un beat que lee la flag pero exige mas), asi que la
lectura es una cota superior de «puede influir»; la prueba de la fase B es la que dice si de verdad llega.

Uso (desde MotorGraphico/):
    python3 tools/matriz_decisiones.py [--dia primer_dia] [--md salida.md] [--json salida.json] [--top 8]

`--dia` es el sufijo de assets/adventures/boundington_<dia>.json: primer_dia, dia_gorran, segundo_dia, ocaso. Los dias
posteriores salen de ORDEN. ensure_ascii=False en todo lo que escribe. Solo biblioteca estandar.
"""
import argparse
import json
import os
import re
import sys
from collections import defaultdict

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ORDEN = ["primer_dia", "dia_gorran", "segundo_dia", "ocaso"]
# Prefijos de flag propios de cada dia (para decidir de que dia es una cinematica): el dia 1 y el dia 3 de Boundington
# comparten el prefijo de su fichero (bnd0_/bnd1_, bnd2_ = dia de Gorran y dia 3, bnd3_ = Ocaso).
PROPIOS = {
    "primer_dia": ("bnd0_", "bnd1_"),
    "dia_gorran": ("bndg_", "bga_", "bgb_", "bgc_", "bgd_", "bg_"),
    "segundo_dia": ("bnd2_",),
    "ocaso": ("bnd3_",),
}
# Marcas de «ya es otro dia» para cinematicas del dia 1 (cierran el dia o ya son del siguiente).
POSTERIORES = {
    "primer_dia": ("bnd2_", "bnd3_", "bndg_", "bga_", "bgb_", "bgc_", "bgd_", "bg_", "bnd_dia_cerrado", "bnd_venides_20"),
    # E-MATRIZ-02: las cinematicas del dia de Gorran leen bnd2_/bg*_ de su propio dia; las posteriores se reconocen por el Ocaso (bnd3_).
    "dia_gorran": ("bnd3_", "bnd2_dia_cerrado"),
}
# Variables y flags que lee todo el mundo (el reloj y el cierre del dia): no distinguen una decision de otra.
CONCENTRADORES = {"dia", "franja", "bnd_dia_cerrado"}
LIBRO = ("_en_charla", "_pag", "_abierta", "_visto")  # contabilidad de las charlas: no es una decision
PESO_GRAVEDAD = {"menor": 2, "medio": 5, "mayor": 12}


# ---------------------------------------------------------------------------------------------------------------
# JSON con numeros de linea: cada cadena (clave o valor) es un LStr con `.line`.
# ---------------------------------------------------------------------------------------------------------------
class LStr(str):
    line = 0


class _Parser:
    def __init__(self, texto):
        self.t = texto
        self.i = 0
        self.linea = 1

    def _salta(self):
        t = self.t
        while self.i < len(t) and t[self.i] in " \t\r\n":
            if t[self.i] == "\n":
                self.linea += 1
            self.i += 1

    def valor(self):
        self._salta()
        c = self.t[self.i]
        if c == "{":
            return self._objeto()
        if c == "[":
            return self._lista()
        if c == '"':
            return self._cadena()
        m = re.compile(r"-?\d+(\.\d+)?([eE][+-]?\d+)?|true|false|null").match(self.t, self.i)
        if not m:
            raise ValueError("JSON invalido en la linea %d" % self.linea)
        self.i = m.end()
        s = m.group(0)
        if s in ("true", "false", "null"):
            return {"true": True, "false": False, "null": None}[s]
        return float(s) if ("." in s or "e" in s.lower()) else int(s)

    def _cadena(self):
        linea = self.linea
        j = self.i + 1
        t = self.t
        while t[j] != '"':
            j += 2 if t[j] == "\\" else 1
        crudo = t[self.i:j + 1]
        self.i = j + 1
        s = LStr(json.loads(crudo))
        s.line = linea
        return s

    def _objeto(self):
        self.i += 1
        d = {}
        self._salta()
        if self.t[self.i] == "}":
            self.i += 1
            return d
        while True:
            self._salta()
            k = self._cadena()
            self._salta()
            self.i += 1  # :
            d[k] = self.valor()
            self._salta()
            c = self.t[self.i]
            self.i += 1
            if c == "}":
                return d

    def _lista(self):
        self.i += 1
        l = []
        self._salta()
        if self.t[self.i] == "]":
            self.i += 1
            return l
        while True:
            l.append(self.valor())
            self._salta()
            c = self.t[self.i]
            self.i += 1
            if c == "]":
                return l


def carga(ruta):
    with open(ruta, encoding="utf-8") as f:
        return _Parser(f.read()).valor()


def linea(x, por_defecto=0):
    return getattr(x, "line", por_defecto)


def rel(ruta):
    return os.path.relpath(ruta, RAIZ)


# ---------------------------------------------------------------------------------------------------------------
# Lo que lee y lo que pone un beat
# ---------------------------------------------------------------------------------------------------------------
def lecturas_de_requires(req):
    """[(nombre, 'flag'|'noflag'|'var', linea)] de un bloque requires/condiciones."""
    out = []
    if not isinstance(req, dict):
        return out
    for clave, rol in (("allFlags", "flag"), ("anyFlags", "flag"), ("notFlags", "noflag"),
                       ("todas", "flag"), ("alguna", "flag"), ("ninguna", "noflag")):
        for f in req.get(clave, []) or []:
            out.append((str(f), rol, linea(f)))
    v = req.get("variables")
    if isinstance(v, dict):
        for n in v:
            out.append((str(n), "var", linea(n)))
    return out


def efectos_de(lista):
    """Normaliza los efectos de una opcion/beat a un dict de lo que hace."""
    r = {"set": [], "clear": [], "vars": [], "tiradas": [], "combates": [], "actos": [], "oro": 0, "nivel": 0.0,
         "objetos": [], "heridas": [], "otros": []}
    for e in lista or []:
        t = e.get("type")
        a = str(e.get("arg", ""))
        if t == "setFlag":
            r["set"].append(a)
        elif t == "clearFlag":
            r["clear"].append(a)
        elif t in ("setVariable", "addVariable"):
            r["vars"].append((a, "=" if t == "setVariable" else "+", e.get("value", 0)))
        elif t == "skillCheck":
            sc = e.get("skillCheck", {})
            r["tiradas"].append({"skill": str(sc.get("skillId", "")),
                                 "flags": {k[4:].lower(): str(v) for k, v in sc.items() if k.startswith("flag")}})
        elif t == "startBattle":
            sb = e.get("startBattle", {})
            r["combates"].append({"enemigos": [("%s x%s" % (x.get("id"), x.get("cantidad", 1))) for x in sb.get("enemigos", [])]
                                  or [str(sb.get("monsterId", ""))],
                                  "flags": {k[4:].lower(): str(v) for k, v in sb.items() if k.startswith("flag")}})
        elif t == "acto":
            r["actos"].append(a)
        elif t == "grantGold":
            r["oro"] += int(e.get("value", 0))
        elif t == "grantLevel":
            r["nivel"] += float(e.get("value", 0))
        elif t == "darObjeto":
            r["objetos"].append("%s x%s" % (a, e.get("value", 1)))
        elif t == "herida":
            r["heridas"].append("%s (%s)" % (a, e.get("value", "")))
        elif t in ("log",):
            pass
        else:
            r["otros"].append(t)
    return r


def flags_que_pone(ef):
    """Todas las flags que enciende un efecto (set + salidas de tiradas y combates)."""
    out = list(ef["set"])
    for t in ef["tiradas"]:
        out += list(t["flags"].values())
    for c in ef["combates"]:
        out += list(c["flags"].values())
    return out


def es_libro(nombre):
    return nombre.endswith(LIBRO) or "_n_" in nombre or nombre.endswith("_visto")


# ---------------------------------------------------------------------------------------------------------------
# Catalogo de actos (moral)
# ---------------------------------------------------------------------------------------------------------------
def actos():
    d = json.load(open(os.path.join(RAIZ, "assets/rules/actos.json"), encoding="utf-8"))
    return {a["id"]: a for a in d["actos"]}


# ---------------------------------------------------------------------------------------------------------------
# Donde se lee cada nombre despues
# ---------------------------------------------------------------------------------------------------------------
def lecturas_posteriores(dia):
    """{nombre: [dict(donde='fichero:linea', cual='beat/evento/cinematica', rol)]} de todo lo que se lee despues."""
    lect = defaultdict(list)
    i = ORDEN.index(dia)
    for otro in ORDEN[i + 1:]:
        ruta = os.path.join(RAIZ, "assets/adventures/boundington_%s.json" % otro)
        for b in carga(ruta)["beats"]:
            for nombre, rol, ln in lecturas_de_requires(b.get("requires")):
                lect[nombre].append({"donde": "%s:%d" % (rel(ruta), ln), "cual": str(b["id"]), "tipo": "beat", "rol": rol, "dia": otro})
            for o in b.get("opciones", []):
                for nombre, rol, ln in lecturas_de_requires(o.get("requires")):
                    lect[nombre].append({"donde": "%s:%d" % (rel(ruta), ln), "cual": "%s/%s" % (b["id"], str(o.get("texto", ""))[:30]),
                                         "tipo": "opcion", "rol": rol, "dia": otro})
    # Eventos del mundo
    ruta = os.path.join(RAIZ, "assets/eventos/boundington.json")
    for ev in carga(ruta)["eventos"]:
        for nombre, rol, ln in lecturas_de_requires(ev.get("condiciones")):
            lect[nombre].append({"donde": "%s:%d" % (rel(ruta), ln), "cual": str(ev["id"]), "tipo": "evento", "rol": rol, "dia": "ocaso"})
        for dsp in ev.get("disparadores", []):
            if "senal" in dsp:
                lect[str(dsp["senal"])].append({"donde": "%s:%d" % (rel(ruta), linea(dsp["senal"])), "cual": str(ev["id"]),
                                                "tipo": "evento", "rol": "senal", "dia": "ocaso"})
    # El diario (E-MATRIZ-02): los objetivos (`doneFlag`) de los dias posteriores y las misiones de assets/catalogs/quests.json.
    # Cuentan como lector de DIARIO (tipo "diario"): la flag se apunta en el cuaderno pero no cambia nada del juego.
    for otro in ORDEN[i + 1:]:
        ruta = os.path.join(RAIZ, "assets/adventures/boundington_%s.json" % otro)
        for ob in carga(ruta).get("objectives", []):
            f = ob.get("doneFlag")
            if f:
                lect[str(f)].append({"donde": "%s:%d" % (rel(ruta), linea(f)), "cual": str(ob.get("id", "")), "tipo": "diario", "rol": "flag", "dia": otro})
    ruta = os.path.join(RAIZ, "assets/catalogs/quests.json")
    if os.path.exists(ruta):
        pila = [carga(ruta)]
        while pila:
            x = pila.pop()
            if isinstance(x, dict):
                for k, v in x.items():
                    if k in ("doneFlag", "completionFlag") and isinstance(v, str) and v:
                        lect[str(v)].append({"donde": "%s:%d" % (rel(ruta), linea(v)), "cual": str(x.get("id", "")), "tipo": "diario", "rol": "flag", "dia": "diario"})
                    else:
                        pila.append(v)
            elif isinstance(x, list):
                pila.extend(x)
    # Cinematicas que ya no son de este dia
    post = POSTERIORES.get(dia, ())
    propios = PROPIOS.get(dia, ())
    ruta = os.path.join(RAIZ, "assets/cinematicas/cinematicas.json")
    for c in carga(ruta)["cinematicas"]:
        fl = [str(x) for x in (c.get("requiere") or [])] + [str(x) for x in (c.get("sin") or [])]
        if not any(f.startswith(post) for f in fl):
            continue  # es de este dia (o anterior): no cuenta como lectura posterior
        for campo, rol in (("requiere", "flag"), ("sin", "noflag")):
            for f in c.get(campo) or []:
                lect[str(f)].append({"donde": "%s:%d" % (rel(ruta), linea(f)), "cual": str(c["id"]), "tipo": "cinematica", "rol": rol, "dia": "posterior"})
    # Codigo
    for raiz, _, fs in os.walk(os.path.join(RAIZ, "src")):
        for f in fs:
            if not f.endswith((".cpp", ".h")):
                continue
            ruta = os.path.join(raiz, f)
            for n, texto in enumerate(open(ruta, encoding="utf-8", errors="replace"), 1):
                if "setFlag" in texto and not ("hasFlag" in texto):
                    continue  # lo pone, no lo lee
                for m in re.finditer(r'"((?:bnd|bg)[a-z0-9]*_[A-Za-z0-9_]+)"', texto):
                    lect[m.group(1)].append({"donde": "%s:%d" % (rel(ruta), n), "cual": "codigo", "tipo": "codigo", "rol": "flag", "dia": "motor"})
    return lect


def conocidos_en(dia_y_siguientes):
    """Todo nombre que se pone (setFlag, tiradas, combates...) en esos dias: para encontrar lecturas huerfanas."""
    puestos = set()
    for d in dia_y_siguientes:
        for b in carga(os.path.join(RAIZ, "assets/adventures/boundington_%s.json" % d))["beats"]:
            for e in [b.get("effects", [])] + [o.get("effects", []) for o in b.get("opciones", [])]:
                ef = efectos_de(e)
                puestos.update(flags_que_pone(ef))
                puestos.update(ef["clear"])
                puestos.update(v[0] for v in ef["vars"])
    return puestos


# ---------------------------------------------------------------------------------------------------------------
# El inventario
# ---------------------------------------------------------------------------------------------------------------
def inventario(dia, ruta_dia=None):
    ruta = ruta_dia or os.path.join(RAIZ, "assets/adventures/boundington_%s.json" % dia)
    beats = carga(ruta)["beats"]
    catalogo_actos = actos()
    lect = lecturas_posteriores(dia)
    # flag -> beats del mismo dia que la leen en positivo y las flags que ponen (un salto)
    lee_positivo = defaultdict(list)
    for b in beats:
        for nombre, rol, _ in lecturas_de_requires(b.get("requires")):
            if rol == "flag":
                lee_positivo[nombre].append(b)

    def lecturas_de(nombre):
        return [] if nombre in CONCENTRADORES else lect.get(nombre, [])

    filas = []
    for b in beats:
        ops = b.get("opciones") or []
        if not ops:
            continue
        opciones = []
        for o in ops:
            ef = efectos_de(o.get("effects"))
            puestas = flags_que_pone(ef)
            directas = {}
            for f in puestas + list(ef["clear"]):
                if es_libro(f):
                    continue
                if lecturas_de(f):
                    directas[f] = lecturas_de(f)
            for v, _, _ in ef["vars"]:
                if lecturas_de(v):
                    directas[v] = lecturas_de(v)
            # un salto: beats del mismo dia que se activan con las flags de la opcion
            indirectas = {}
            for f in puestas:
                for b2 in lee_positivo.get(f, []):
                    if b2["id"] == b["id"]:
                        continue
                    ef2 = efectos_de(b2.get("effects"))
                    for g in flags_que_pone(ef2) + [v[0] for v in ef2["vars"]]:
                        if es_libro(g) or g in directas:
                            continue
                        if lecturas_de(g):
                            indirectas.setdefault(g, {"via": str(b2["id"]), "lecturas": lecturas_de(g)})
            moral = []
            for a in ef["actos"]:
                x = catalogo_actos.get(a, {})
                moral.append({"acto": a, "gravedad": x.get("gravedad", "?"), "signo": x.get("signo", 0), "reputacion": x.get("reputacion", {})})
            opciones.append({
                "texto": str(o.get("texto", "")), "linea": linea(o.get("texto")), "despedida": bool(o.get("despedida")),
                "pone": [f for f in puestas if not es_libro(f)], "quita": [f for f in ef["clear"] if not es_libro(f)],
                "variables": ["%s%s%s" % (n, op, v) for n, op, v in ef["vars"] if not n.endswith("_pag")],
                "tiradas": ef["tiradas"], "combates": ef["combates"], "moral": moral, "oro": ef["oro"],
                "nivel": round(ef["nivel"], 2), "objetos": ef["objetos"], "heridas": ef["heridas"],
                "directas": {k: [{kk: vv for kk, vv in x.items()} for x in v] for k, v in directas.items()},
                "indirectas": indirectas, "requires": [n for n, _, _ in lecturas_de_requires(o.get("requires"))],
            })
        filas.append({"beat": str(b["id"]), "linea": linea(b["id"]), "trigger": b.get("trigger", {}), "speaker": str(b.get("speaker", "")),
                      "requires": [n for n, _, _ in lecturas_de_requires(b.get("requires"))], "opciones": opciones})
    return filas, beats, ruta, lect


def peso_opcion(o):
    """Cuanto pesa una opcion: lecturas posteriores (distintos ficheros:linea, mas el hecho de ser de beat/evento/cine),
    salto indirecto a mitad, moral por gravedad, oro y nivel."""
    lecturas = set()
    diario = set()
    for v in o["directas"].values():
        lecturas.update((x["dia"], x["cual"].split("/")[0]) for x in v if x["tipo"] != "diario")
        diario.update((x["dia"], x["cual"]) for x in v if x["tipo"] == "diario")
    indirectas = set()
    for v in o["indirectas"].values():
        indirectas.update((x["dia"], x["cual"].split("/")[0]) for x in v["lecturas"] if x["tipo"] != "diario")
    moral = sum(PESO_GRAVEDAD.get(m["gravedad"], 0) for m in o["moral"] if m["signo"] != 0)
    return len(lecturas) + 0.5 * len(indirectas - lecturas) + 0.5 * len(diario) + moral + abs(o["oro"]) // 10 + (3 if o["nivel"] >= 0.5 else 0)


def es_importante(o):
    return bool(o["directas"] or o["indirectas"] or any(m["signo"] != 0 and m["gravedad"] in ("medio", "mayor") for m in o["moral"])
                or abs(o["oro"]) >= 20 or o["nivel"] >= 0.5)


def es_contabilidad(nombre):
    """Flags de contabilidad de un beat o de una charla (tema ya tratado, tirada pedida, beat resuelto): nadie tiene por que leerlas fuera."""
    return "_t_" in nombre or nombre.endswith(("_tirada", "_resuelto", "_resuelta", "_cerrado", "_cerrada", "_ayudado", "_abierto", "_hecho", "_dicho"))


def es_variante(lector, setter):
    """¿`lector` es el propio beat `setter` o una de sus paginas (_s0, _p0_a...)? Su guarda («notFlags» de lo que el beat enciende) no es un lector."""
    return lector == setter or re.fullmatch(re.escape(setter) + r"_[sp]\d+(_[a-z])?", lector) is not None


def hallazgos(filas, beats, dia, lect, ruta):
    """Sospechas automaticas: flags que nadie lee, opciones que dan lo mismo, lecturas de flags que nadie pone."""
    avisos = []
    lee_dia = defaultdict(set)  # nombre -> ids de los beats del dia que lo leen
    for b in beats:
        for nombre, rol, _ in lecturas_de_requires(b.get("requires")):
            lee_dia[nombre].add(str(b["id"]))
        for o in b.get("opciones", []):
            for nombre, rol, _ in lecturas_de_requires(o.get("requires")):
                lee_dia[nombre].add(str(b["id"]))
    # 1. flags que una opcion de decision pone y nadie lee en ningun sitio (ni el propio beat como guarda), o solo lee el diario
    for f in filas:
        for o in f["opciones"]:
            for fl in o["pone"]:
                if fl.startswith("heroe_") or es_libro(fl) or es_contabilidad(fl):
                    continue
                lectores_dia = {x for x in lee_dia.get(fl, set()) if not es_variante(x, f["beat"])}
                lectores_post = lect.get(fl, [])
                if not lectores_dia and not lectores_post:
                    avisos.append({"tipo": "flag-sin-lector", "donde": "%s:%d" % (f["beat"], o["linea"]), "dato": fl, "opcion": o["texto"]})
                elif not lectores_dia and all(x["tipo"] == "diario" for x in lectores_post):
                    avisos.append({"tipo": "solo-diario", "donde": "%s:%d" % (f["beat"], o["linea"]), "dato": fl,
                                   "opcion": "%s; lo lee solo el diario: %s" % (o["texto"], lectores_post[0]["donde"])})
    # 2. opciones hermanas que dan lo mismo (mismas flags y variables, sin diferencia de moral/oro) y no son charla
    for f in filas:
        grupos = defaultdict(list)
        for o in f["opciones"]:
            clave = (tuple(sorted(o["pone"])), tuple(sorted(o["variables"])), json.dumps(o["moral"], sort_keys=True), o["oro"],
                     tuple(sorted(o["quita"])), json.dumps(o["tiradas"], sort_keys=True))
            if o["pone"] or o["variables"] or o["moral"]:
                grupos[clave].append(o)
        for clave, os_ in grupos.items():
            if len(os_) > 1:
                avisos.append({"tipo": "opciones-iguales", "donde": "%s:%d" % (f["beat"], os_[0]["linea"]),
                               "dato": " | ".join(o["texto"] for o in os_), "opcion": ",".join(clave[0])})
    # 3. lecturas posteriores de flags del prefijo de este dia que nadie pone (en ningun dia): errata
    puestas = conocidos_en(ORDEN)
    prefijos = PROPIOS.get(dia, ())
    for nombre, l in lect.items():
        if not nombre.startswith(prefijos) or not prefijos:
            continue
        if nombre in puestas or nombre in ("bnd1_dorm",) or nombre.startswith("heroe_"):
            continue
        # las flags de cinematica / harness / codigo (visto de cinematica, pasos) tampoco se ponen en un beat: solo cuenta
        # el aviso si la lee un beat/evento/opcion.
        if any(x["tipo"] in ("beat", "opcion", "evento") for x in l):
            avisos.append({"tipo": "lectura-sin-escritor", "donde": l[0]["donde"], "dato": nombre, "opcion": ""})
    return avisos


# ---------------------------------------------------------------------------------------------------------------
# Salida
# ---------------------------------------------------------------------------------------------------------------
def md_celda(s):
    return str(s).replace("|", "\\|").replace("\n", " ")


def lecturas_cortas(d, maximo=4):
    """'fichero:linea (beat)' recortado."""
    partes = []
    vistos = set()
    for x in d:
        clave = x["donde"]
        if clave in vistos:
            continue
        vistos.add(clave)
        partes.append("%s (%s)" % (clave.replace("assets/", "").replace("adventures/boundington_", "adv/"), x["cual"]))
    corte = partes[:maximo]
    if len(partes) > maximo:
        corte.append("+%d mas" % (len(partes) - maximo))
    return "; ".join(corte)


def describe_opcion(o):
    pone = []
    for f in o["pone"]:
        pone.append(f)
    for v in o["variables"]:
        pone.append(v)
    for m in o["moral"]:
        pone.append("acto:%s(%s%s)" % (m["acto"], m["gravedad"], "+" if m["signo"] > 0 else "-" if m["signo"] < 0 else "0"))
    if o["oro"]:
        pone.append("oro%+d" % o["oro"])
    if o["nivel"]:
        pone.append("nivel+%s" % o["nivel"])
    for t in o["tiradas"]:
        pone.append("tirada %s [%s]" % (t["skill"], ", ".join("%s>%s" % (k, v) for k, v in t["flags"].items())))
        for v in t["flags"].values():
            if v in pone:
                pone.remove(v)
    for c in o["combates"]:
        pone.append("combate %s [%s]" % ("+".join(c["enemigos"]), ", ".join("%s>%s" % (k, v) for k, v in c["flags"].items())))
        for v in c["flags"].values():
            if v in pone:
                pone.remove(v)
    if o["quita"]:
        pone.append("quita " + ", ".join(o["quita"]))
    return pone


def escribe_md(dia, filas, avisos, top, ruta_md, ruta_json):
    importantes = []
    resto = []
    for f in filas:
        imp = [o for o in f["opciones"] if es_importante(o)]
        if not imp:
            continue
        peso = sum(peso_opcion(o) for o in imp)
        reg = dict(f)
        reg["importantes"] = imp
        reg["peso"] = peso
        importantes.append(reg)
    unicas = {}
    for r in importantes:  # los beats gemelos de pagina (_s0, _p0...) son la misma decision
        clave = json.dumps([(o["texto"], o["pone"], o["variables"], o["quita"]) for o in r["importantes"]], ensure_ascii=False, sort_keys=True)
        if clave in unicas:
            unicas[clave].setdefault("variantes", []).append("%s:%d" % (r["beat"], r["linea"]))
        else:
            unicas[clave] = r
    importantes = sorted(unicas.values(), key=lambda r: -r["peso"])
    for n, r in enumerate(importantes):
        r["top"] = n < top
    with open(ruta_md, "w", encoding="utf-8") as m:
        m.write("# MATRIZ_DIA — %s\n\n" % dia)
        m.write("Generado por `MotorGraphico/tools/matriz_decisiones.py --dia %s` (no editar a mano: se regenera). \n" % dia)
        m.write("Decisiones importantes: **%d** beats con elecciones que leen despues del dia (de %d beats con elecciones). "
                "Las `top %d` por peso llevan guardado y prueba (fase B).\n\n" % (len(importantes), len(filas), top))
        m.write("Peso = beats/eventos/cinematicas posteriores distintos que leen (+0,5 por los de un salto) + moral (menor 2, medio 5, mayor 12) + |oro|/10 + 3 si da nivel.\n\n")
        m.write("## Decisiones importantes (por peso)\n\n")
        m.write("| # | beat:linea | peso | opcion (texto:linea) | pone | se lee despues en | consecuencia |\n|---|---|---|---|---|---|---|\n")
        for n, r in enumerate(importantes, 1):
            for k, o in enumerate(r["importantes"]):
                pone = "; ".join(describe_opcion(o))
                lee = []
                for nombre, d in o["directas"].items():
                    lee.append("**%s**: %s" % (nombre, lecturas_cortas(d, 3)))
                for nombre, d in o["indirectas"].items():
                    lee.append("(via %s) %s: %s" % (d["via"], nombre, lecturas_cortas(d["lecturas"], 2)))
                cons = set()
                for d in list(o["directas"].values()) + [v["lecturas"] for v in o["indirectas"].values()]:
                    for x in d:
                        cons.add("%s %s" % (x["tipo"], x["cual"]))
                for mm in o["moral"]:
                    if mm["signo"]:
                        cons.add("moral %s%s" % (mm["acto"], (" rep " + json.dumps(mm["reputacion"], ensure_ascii=False)) if mm["reputacion"] else ""))
                m.write("| %s | %s | %s | %s:%d | %s | %s | %s |\n" % (
                    n if k == 0 else "", ("%s:%d" % (r["beat"], r["linea"]) + ((" (y " + ", ".join(r["variantes"]) + ")") if r.get("variantes") else "")) if k == 0 else "", "%.1f" % r["peso"] if k == 0 else "",
                    md_celda(o["texto"]), o["linea"], md_celda(pone), md_celda("; ".join(lee) or "(moral/oro/nivel)"), md_celda(", ".join(sorted(cons)[:6]))))
        m.write("\n## Avisos automaticos (marcar ⚠ tras revisar a mano)\n\n")
        for a in avisos:
            m.write("- `%s` %s — %s %s\n" % (a["tipo"], a["donde"], a["dato"], ("(" + a["opcion"] + ")") if a["opcion"] else ""))
    if ruta_json:
        salida = {"dia": dia, "decisiones": [{"beat": r["beat"], "linea": r["linea"], "peso": r["peso"], "top": r["top"],
                                              "opciones": r["importantes"]} for r in importantes], "avisos": avisos}
        with open(ruta_json, "w", encoding="utf-8") as j:
            json.dump(salida, j, ensure_ascii=False, indent=1)
            j.write("\n")
    return importantes


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--dia", default="primer_dia", choices=ORDEN)
    ap.add_argument("--md", default=None)
    ap.add_argument("--json", default=None)
    ap.add_argument("--top", type=int, default=8)
    ap.add_argument("--adventure", default=None, help="ruta de un JSON de aventura alternativo (para la prueba y las mutaciones)")
    a = ap.parse_args(argv)
    filas, beats, ruta, lect = inventario(a.dia, a.adventure)
    avisos = hallazgos(filas, beats, a.dia, lect, ruta)
    md = a.md or os.path.join(RAIZ, "MATRIZ_%s.md" % a.dia)
    importantes = escribe_md(a.dia, filas, avisos, a.top, md, a.json)
    print("%s: %d beats con elecciones, %d decisiones importantes, %d avisos -> %s" % (a.dia, len(filas), len(importantes), len(avisos), md))
    return 0


if __name__ == "__main__":
    sys.exit(main())
