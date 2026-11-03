#!/usr/bin/env python3
"""AV18-04b. Sintetiza las rutas de tools/rutas_economia.json a partir de las aventuras.

El simulador (simular_economia.py) recorre las aventuras con recorrer_recompensas
siguiendo una lista de acciones por perfil. Escribirlas a mano deja fuera la mayoria
de los premios (los beats `auto` que exigen banderas de combates, pruebas y charlas
anteriores). Aqui se calcula, para cada premio (grantGold > 0 o darObjeto), la cadena
de beats que lo desbloquea, y de ahi las acciones: entrar en un nivel, hablar con un
PNJ y que opcion preferir. Los combates se ganan y las pruebas salen con exito
(`victoria`/`grado` de la ruta), como ya suponia Codex.

UNI-02-11 E5: cuatro dias (Gorran es el dia 2), cinematicas y eventos del mundo como proveedores de banderas, y
las acciones «pisar <zona>» y «poner <bandera|variable=n>» (ver recorrer_recompensas.cpp). El esqueleto de cada dia
esta en rutas_economia_base.json; la accion «premios» marca donde se insertan las cadenas (dias 2-4).

Cobertura por perfil (ECONOMIA.md §6, por VALOR): historia 75 % de misiones y 65 %
de combates; explora_todo y optimiza, todo. Un premio que sale de un combate (su
cadena pasa por un flagVictory) cuenta en el canal de combates.

Uso:  python3 tools/sintetizar_rutas.py            # reescribe rutas_economia.json
      python3 tools/sintetizar_rutas.py --seco     # solo cuenta
"""
import json
import os
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from validar_botin import lee_economia  # noqa: E402

MOTOR = Path(__file__).resolve().parents[1]
# UNI-02-11 E5 (decision de Oriol, 03/11): la campana tiene CUATRO dias, en el orden del juego
# (Application.cpp, m_dia2 = el antiguo segundo_dia, que ahora es el DIA 3): 1 primer_dia, 2 la
# montana de Gorran, 3 segundo_dia (la Barriada y la fiesta), 4 el Ocaso.
AVENTURAS = {1: "boundington_primer_dia.json", 2: "boundington_dia_gorran.json",
             3: "boundington_segundo_dia.json", 4: "boundington_ocaso.json"}
DIAS = tuple(sorted(AVENTURAS))
ULTIMO = DIAS[-1]
# La bandera con que cierra cada dia (la pone el cierre del esqueleto de rutas_economia_base.json): para los
# dias siguientes ya esta puesta y no se intenta cerrar el dia en mitad de otro.
CIERRES = {1: "bnd_dia_cerrado", 2: "bndg_dia_cerrado", 3: "bnd2_dia_cerrado"}
# Variables que el recorrido NO juega: la demo de la campana (examples/demo_boundington_partida.h,
# laEspadaDeRosita) las da por hechas. gorran_misiones = los cuatro «lios» de la aldea de Gorran (Cal Serra,
# la Veta Honda, las Galerias, la mina vieja): la subasta exige tres cerrados. Se pone con la accion «poner»
# y los premios de esos lios quedan sin recorrer (ver LIOS_DE_GORRAN y la nota de rutas_economia.json).
PONER_VARIABLES = {"gorran_misiones": 3}
LIOS_DE_GORRAN = ("bga_", "bgb_", "bgc_", "bgd_")
# Zonas a las que se va ANTES que a las demas (el arnés de las demos: zonasPrimero). Edul se libera en la ciudad
# antes de la salida del Ocaso; despues la salida lo traslada (bnd3_edul_trasladado) y su zona ya no arranca.
PRIMERO = [(ULTIMO, "edul_casco_retenido")]
# Banderas que no pone ninguna aventura de los cuatro dias: vienen del prologo/precuela o
# de la creacion del personaje. Se dan por puestas al empezar (flags_iniciales).
INICIALES = ["bnd_prologo_terminado", "heroe_pueblo_humano", "heroe_pueblo_de_aegroum",
             "heroe_rol_tank"]
# Opciones que cierran el juego de otra manera o cuestan mas de lo que dan: no se eligen
# aunque desbloqueen algo (se puede ampliar).
# «Os he salvado la vida. Eso vale algo.» (10 de oro, b1_fuego_s3_a) cierra la charla de la familia y excluye
# «Os llevo a la Barriada de Surysal» (b1_fuego_s2_a), de la que cuelgan las cinematicas y los premios de la
# Barriada del dia 3 (bnd2_familia_llevada): se prefiere la Barriada y ese premio queda sin cadena.
EVITAR_OPCIONES = {"Unirte de verdad", "Os he salvado la vida. Eso vale algo."}
# Beats cuyo premio exige irse sin ayudar a la familia (rec_fuego_regalo: notFlags bnd1_familia_en_barriada): mismo
# motivo, se prefiere la Barriada y ese premio queda sin cadena.
EVITAR_BEATS = {"rec_fuego_regalo"}
# ... ni las que dejan al heroe unido a la secta: en el dia 3 la Barriada lo rechaza (b2_po54_barriada, que pone
# bnd2_barriada_rechaza_culto) y las cinematicas de la Barriada ya no arrancan.
EVITAR_BANDERAS = {"bnd1_unido_al_culto", "bnd1_verdaderos_unido"}


def carga(dia):
    return json.load(open(MOTOR / "assets/adventures" / AVENTURAS[dia], encoding="utf-8"))


def precio(objetos, equipo, i):
    e = equipo.get(i) or equipo.get(i.removeprefix("item_"))
    return e["price"] if e else objetos.get(i, {}).get("price", 0)


class Aventura:
    def __init__(self, dia, doc, precio_de):
        self.dia = dia
        self.archivo = AVENTURAS[dia]
        self.beats = {b["id"]: b for b in doc["beats"]}
        self.precio_de = precio_de
        # flag -> [(beat_id, opcion_idx|None, tipo)]  tipo: setFlag | victoria | exito
        self.pone = defaultdict(list)
        for b in doc["beats"]:
            for idx, efectos in self.efectos(b):
                for e in efectos:
                    if e["type"] == "setFlag":
                        self.pone[e["arg"]].append((b["id"], idx, "setFlag"))
                    elif e["type"] == "startBattle":
                        f = e["startBattle"].get("flagVictory")
                        if f:
                            self.pone[f].append((b["id"], idx, "victoria"))
                    elif e["type"] == "skillCheck":
                        for k in ("flagSuccess", "flagCritical"):
                            f = e["skillCheck"].get(k)
                            if f:
                                self.pone[f].append((b["id"], idx, "exito"))

    @staticmethod
    def efectos(b):
        yield None, b.get("effects", [])
        for i, o in enumerate(b.get("opciones", [])):
            yield i, o.get("effects", [])

    def premios(self):
        """[(beat_id, opcion_idx|None, oro, valor_objetos)]"""
        out = []
        for b in self.beats.values():
            for idx, efectos in self.efectos(b):
                oro = sum(e["value"] for e in efectos if e["type"] == "grantGold" and e["value"] > 0)
                obj = sum(self.precio_de(e["arg"]) * e.get("value", 1) for e in efectos if e["type"] == "darObjeto")
                if oro or obj:
                    out.append((b["id"], idx, oro, obj))
        return out


def nivel_de(ruta):
    """'assets/levels/ciudad_centro.json' -> 'ciudad_centro' (como lo espera el recorrido)."""
    return os.path.splitext(os.path.basename(ruta))[0]


class Mundo:
    """Lo que enciende banderas sin ser un beat: las cinematicas (assets/cinematicas/cinematicas.json) y los
    eventos del mundo (assets/eventos/boundington.json), que el recorrido ejecuta como el juego."""

    def __init__(self):
        self.cines = {}
        cat = json.load(open(MOTOR / "assets/cinematicas/cinematicas.json", encoding="utf-8"))
        for c in cat["cinematicas"]:
            self.cines[c["id"]] = c
        self.eventos = {}
        ruta = MOTOR / "assets/eventos/boundington.json"
        if ruta.exists():
            for e in json.load(open(ruta, encoding="utf-8"))["eventos"]:
                self.eventos[e["id"]] = e
        # flag -> [("cine"|"evento", id)]
        self.pone = defaultdict(list)
        for c in self.cines.values():
            for f in self.flags_cine(c):
                self.pone[f].append(("cine", c["id"]))
        for e in self.eventos.values():
            for f in self.flags_evento(e):
                self.pone[f].append(("evento", e["id"]))

    @staticmethod
    def flags_cine(c):
        # Como Game::Cinematica en el recorrido: los pasos beat/flag, la «vista» y alTerminar.
        fl = [p[k] for p in c.get("pasos", []) for k in ("beat", "flag") if k in p]
        return fl + [f"cine_{c['id']}_vista"] + list(c.get("alTerminar", []))

    @staticmethod
    def flags_evento(e):
        fl = [f"evt_{e['id']}"]
        efectos = list(e.get("efectos", []))
        for fase in e.get("fases", []):
            fl.append(f"evt_{e['id']}__{fase['id']}")
            efectos += fase.get("efectos", [])
        return fl + [x["arg"] for x in efectos if x["type"] == "setFlag"]


class Sintetizador:
    def __init__(self, aventuras, mundo=None):
        self.av = aventuras                      # dia -> Aventura
        self.mundo = mundo or Mundo()
        self.acciones = {}                       # dia -> [accion]
        self.generales = defaultdict(list)       # dia -> preferencias
        self.hechos = set()                      # (dia, beat, idx) | (dia, "cine"|"evento", id)
        self.puestas = {f: d for d, f in CIERRES.items()}  # bandera -> primer dia en que ya esta puesta
        self.combate = False
        self.variables = set()                   # variables ya puestas con «poner»

    def quien_pone(self, dia, flag):
        """[(dia, proveedores)] por orden de preferencia: el dia mismo y despues los anteriores, el mas reciente primero."""
        out = []
        for d in sorted((d for d in self.av if d <= dia), reverse=True):
            if flag in self.av[d].pone:
                out.append((d, self.av[d].pone[flag]))
        return out

    def marca(self, dia, b, idx):
        """Las banderas que deja un beat (y la opcion elegida) ya estan puestas desde este dia."""
        efectos = list(b.get("effects", []))
        if idx is not None:
            efectos += b["opciones"][idx].get("effects", [])
        for e in efectos:
            if e["type"] == "clearFlag":
                self.puestas.pop(e.get("arg"), None)
                continue
            f = {"setFlag": e.get("arg"),
                 "startBattle": (e.get("startBattle") or {}).get("flagVictory"),
                 "skillCheck": (e.get("skillCheck") or {}).get("flagSuccess")}.get(e["type"])
            if f:
                self.puestas[f] = min(self.puestas.get(f, dia), dia)

    def resolver_flag(self, dia, flag, pila):
        if flag in INICIALES or flag in pila:
            return True
        if self.puestas.get(flag, dia + 1) <= dia:
            return True
        # Preferir lo que no exige opcion (beat entero) y lo que exige menos banderas.
        for d, proveedores in self.quien_pone(dia, flag):
            def coste(p, d=d):
                b = self.av[d].beats[p[0]]
                req = b.get("requires") or {}
                # Una opcion que cobra (grantGold < 0: pagar el paso, la cama) va la ultima si hay otra salida.
                cobra = p[1] is not None and any(e["type"] == "grantGold" and e["value"] < 0
                                                  for e in b["opciones"][p[1]].get("effects", []))
                return (cobra, p[1] is not None, len(req.get("allFlags", [])) + len(req.get("anyFlags", [])))
            for beat_id, idx, tipo in sorted(proveedores, key=coste):
                if tipo == "victoria":
                    self.combate = True
                if self.disparar(d, beat_id, idx, pila | {flag}):
                    return True
        for tipo, ident in self.mundo.pone.get(flag, []):
            if (self.cinematica if tipo == "cine" else self.evento)(dia, ident, pila | {flag}):
                return True
        return False

    def pon_variables(self, dia, cond):
        for var, rango in (cond.get("variables") or {}).items():
            if var in PONER_VARIABLES and isinstance(rango, dict) and rango.get("min", 0) > 0 and var not in self.variables:
                self.variables.add(var)
                self.acciones.setdefault(dia, []).append(
                    dict(dia=dia, aventura=self.av[dia].archivo, accion="poner", destino=f"{var}={PONER_VARIABLES[var]}"))

    def cumple(self, dia, cond, pila):
        cond = cond or {}
        self.pon_variables(dia, cond)
        return (all(self.resolver_flag(dia, f, pila) for f in cond.get("allFlags", [])) and
                (not cond.get("anyFlags") or any(self.resolver_flag(dia, f, pila) for f in cond["anyFlags"])))

    def cinematica(self, dia, ident, pila):
        """Una cinematica arranca sola en su nivel (entrar/hablar) o al pisar su zona (accion «pisar»)."""
        clave = (dia, "cine", ident)
        if any((d, "cine", ident) in self.hechos for d in DIAS if d <= dia):
            return True
        c = self.mundo.cines[ident]
        # Una cinematica no arranca si ya esta puesta alguna bandera de su «sin» (las salidas del Ocaso se excluyen).
        if any(self.puestas.get(f, ULTIMO + 1) <= dia and f not in INICIALES for f in c.get("sin", [])):
            return False
        if not all(self.resolver_flag(dia, f, pila) for f in c.get("requiere", [])):
            return False
        self.hechos.add(clave)
        acciones = self.acciones.setdefault(dia, [])
        av = self.av[dia].archivo
        if c.get("nivel"):
            acciones.append(dict(dia=dia, aventura=av, accion="entrar", destino=nivel_de(c["nivel"]), preferencias=[]))
        if "zona" in c:
            acciones.append(dict(dia=dia, aventura=av, accion="pisar", destino=ident))
        elif not c.get("nivel"):
            acciones.append(dict(dia=dia, aventura=av, accion="auto"))
        for f in Mundo.flags_cine(c):
            self.puestas[f] = min(self.puestas.get(f, dia), dia)
        return True

    def esqueleto(self, acciones):
        """Lo que el esqueleto de la ruta ya hace (pisar una zona) no se repite en las cadenas."""
        for a in acciones:
            if a["accion"] == "pisar":
                self.hechos.add((a["dia"], "cine", a["destino"]))
                for f in Mundo.flags_cine(self.mundo.cines[a["destino"]]):
                    self.puestas[f] = min(self.puestas.get(f, a["dia"]), a["dia"])

    def evento(self, dia, ident, pila):
        """Un evento del mundo salta solo cuando se cumplen sus condiciones y un disparador (senal o evento previo;
        el de reloj lo pone el esqueleto de la ruta)."""
        clave = (dia, "evento", ident)
        if any((d, "evento", ident) in self.hechos for d in DIAS if d <= dia):
            return True
        e = self.mundo.eventos[ident]
        if not self.cumple(dia, e.get("condiciones"), pila):
            return False
        for d in e.get("disparadores", []):
            if "senal" in d and self.resolver_flag(dia, d["senal"], pila):
                break
            if "eventoPrevio" in d and self.evento(dia, d["eventoPrevio"].split(".")[0], pila):
                break
        else:
            return False
        self.hechos.add(clave)
        self.acciones.setdefault(dia, []).append(dict(dia=dia, aventura=self.av[dia].archivo, accion="auto"))
        for f in Mundo.flags_evento(e):
            self.puestas[f] = min(self.puestas.get(f, dia), dia)
        return True

    def disparar(self, dia, beat_id, idx, pila=frozenset()):
        """Anade las acciones para que el beat (y su opcion) se dispare. True si se pudo."""
        clave = (dia, beat_id, idx)
        if clave in self.hechos:
            return True
        av = self.av[dia]
        b = av.beats[beat_id]
        if beat_id in EVITAR_BEATS:
            return False
        if idx is not None:
            o = b["opciones"][idx]
            if o["texto"] in EVITAR_OPCIONES or any(
                    e["type"] == "setFlag" and e["arg"] in EVITAR_BANDERAS for e in o.get("effects", [])):
                return False
        if not self.cumple(dia, b.get("requires"), pila):
            return False
        if idx is not None and not self.cumple(dia, b["opciones"][idx].get("requires"), pila):
            return False
        # Las banderas del beat anterior ya estan: ahora la accion que lo dispara.
        self.hechos.add(clave)
        self.marca(dia, b, idx)
        t = b.get("trigger", {})
        acciones = self.acciones.setdefault(dia, [])
        pref = [b["opciones"][idx]["texto"]] if idx is not None else []
        # Un beat con «lugar» solo salta en ese nivel: se entra antes (salvo si el propio trigger es entrar).
        lugar = (b.get("requires") or {}).get("lugar")
        if isinstance(lugar, list):
            lugar = lugar[0] if lugar else None
        if lugar and t.get("type") != "enter":
            acciones.append(dict(dia=dia, aventura=av.archivo, accion="entrar", destino=nivel_de(lugar), preferencias=[]))
        if t.get("type") == "enter":
            destino = os.path.splitext(os.path.basename(t.get("target", "")))[0]  # como lo espera el recorrido
            acciones.append(dict(dia=dia, aventura=av.archivo, accion="entrar", destino=destino, preferencias=pref))
        elif t.get("type") == "talk":
            acciones.append(dict(dia=dia, aventura=av.archivo, accion="hablar", destino=t.get("target", ""), preferencias=pref))
        else:  # auto: se dispara solo al drenar; la opcion, como preferencia general
            if pref:
                self.generales[dia].extend(pref)
            if not acciones:
                acciones.append(dict(dia=dia, aventura=av.archivo, accion="auto"))
        return True


def objetivos(aventuras, economia, perfil):
    """Premios a cubrir por dia segun la cobertura del perfil (por valor)."""
    p = economia["perfiles"][perfil]["recoge_pct"]
    canales = economia["canales"]
    cob_mis, cob_com = p[canales.index("misiones")], p[canales.index("combates")]
    out = {}
    for dia, av in aventuras.items():
        premios = sorted(av.premios(), key=lambda x: -(x[2] + x[3]))
        total = sum(x[2] + x[3] for x in premios)
        # Los premios por combate llevan "vencid" en el beat (convencion de las aventuras).
        sel, acum_m, acum_c = [], 0, 0
        tot_c = sum(x[2] + x[3] for x in premios if "vencid" in x[0])
        tot_m = total - tot_c
        for beat_id, idx, oro, obj in premios:
            v = oro + obj
            if "vencid" in beat_id:
                if acum_c + v <= tot_c * cob_com / 100 + 1e-9 or cob_com == 100:
                    sel.append((beat_id, idx)); acum_c += v
            else:
                if acum_m + v <= tot_m * cob_mis / 100 + 1e-9 or cob_mis == 100:
                    sel.append((beat_id, idx)); acum_m += v
        out[dia] = sel
    return out


def main():
    seco = "--seco" in sys.argv
    economia = lee_economia(MOTOR.parents[1] / "planificacion_alcance_y_vision/ECONOMIA.md")
    objetos = {}
    for f in (MOTOR / "assets/objects").glob("*.json"):
        for o in json.load(open(f, encoding="utf-8")).get("objects", []):
            objetos[o["id"]] = o
    equipo = {e["id"]: e for e in json.load(open(MOTOR / "assets/catalogs/equipment.json"))["entries"]}
    precio_de = lambda i: precio(objetos, equipo, i)
    aventuras = {dia: Aventura(dia, carga(dia), precio_de) for dia in AVENTURAS}
    mundo = Mundo()

    ruta_json = MOTOR / "tools/rutas_economia.json"
    # El esqueleto de cada dia (empezar en la taberna, cerrar la noche con Skilla y
    # Venides) es el de Codex (rutas_economia_base.json): las cadenas de premios se
    # insertan entre el arranque y el cierre de cada dia.
    base = {r["perfil"]: r for r in json.load(open(MOTOR / "tools/rutas_economia_base.json", encoding="utf-8"))["rutas"]}
    doc = json.load(open(ruta_json, encoding="utf-8"))
    nuevas = []
    for perfil in economia["perfiles"]:
        s = Sintetizador(aventuras, mundo)
        s.esqueleto(base[perfil]["acciones"])
        for dia, zona in PRIMERO:
            s.cinematica(dia, zona, frozenset())
        metas = objetivos(aventuras, economia, perfil)
        logrados = fallidos = 0
        for dia in sorted(metas):
            for beat_id, idx in metas[dia]:
                if s.disparar(dia, beat_id, idx):
                    logrados += 1
                else:
                    fallidos += 1
        # Y el final de la campana: las banderas que el recorrido exige al terminar.
        for f in ("bnd3_fuera", "bnd3_epilogo"):
            if not s.resolver_flag(ULTIMO, f, frozenset()):
                print(f"  AVISO: no hay cadena hasta {f}")
        acciones = []
        esqueleto = base[perfil]["acciones"]
        for dia in DIAS:
            codex = [dict(a) for a in esqueleto if a["dia"] == dia]
            cadenas = [a for a in s.acciones.get(dia, []) if not a.get("accion") == "auto"]
            marca = next((i for i, a in enumerate(codex) if a.get("accion") == "premios"), None)
            if marca is not None:
                # Dias 2-4: el esqueleto trae su propio hueco («premios») donde van las cadenas.
                arranque, cierre = codex[:marca], codex[marca + 1:]
                lista = arranque + cadenas + cierre
            else:
                cierre_desde = next((i for i, a in enumerate(codex)
                                     if "Esperar a que anochezca" in a.get("preferencias", [])), len(codex))
                arranque, cierre = codex[:cierre_desde], codex[cierre_desde:]
                if dia == 1:
                    arranque = codex[:2]  # entrar en la taberna y hablar con Skilla
                    cierre = codex[cierre_desde:] if cierre_desde < len(codex) else codex[2:]
                lista = arranque + [a for a in s.acciones.get(dia, []) if not a.get("accion") == "auto" or not arranque] + cierre
            # `orden` (1..89) solo ordena el dia; si hay mas acciones que huecos se reparten sin retroceder.
            paso = 2 if len(lista) <= 44 else 86 / (len(lista) - 1)
            for i, a in enumerate(lista):
                a = dict(a); a["orden"] = 2 + int(paso * i)
                if not a.get("preferencias"):
                    a.pop("preferencias", None)
                acciones.append(a)
        generales = list(base[perfil]["preferencias"])
        for dia in DIAS:
            for t in s.generales[dia]:
                if t not in generales:
                    generales.append(t)
        for t in ["Esperar a que anochezca", "Pagar la cama", "Subir a dormir", "Nada más", "Volver",
                  "Dejarlo estar", "Contárselo todo", "Contarle lo que tienes", "Aceptar el encargo"]:
            if t not in generales:
                generales.append(t)
        # El recorrido exige una preferencia para CADA beat con opciones que encuentre: de
        # respaldo, la primera opcion de cada beat que siempre se puede elegir (sin requisitos) y que no
        # cuesta oro; si no hay, la primera (las elegidas arriba van antes y mandan).
        for dia in DIAS:
            for b in aventuras[dia].beats.values():
                libres = [o for o in b.get("opciones", []) if o["texto"] not in EVITAR_OPCIONES]
                def cobra(o):
                    return any(e["type"] == "grantGold" and e["value"] < 0 for e in o.get("effects", []))
                o = next((o for o in libres if not o.get("requires") and not cobra(o)), libres[0] if libres else None)
                if o is not None and o["texto"] not in generales:
                    generales.append(o["texto"])
        nuevas.append(dict(perfil=perfil, grado="exito", victoria=True, cara_tabla=1,
                           flags_iniciales=INICIALES,
                           flags_finales=["bnd3_fuera", "bnd3_epilogo"],
                           preferencias=generales, acciones=acciones))
        print(f"{perfil:13} premios cubiertos {logrados:>3}, sin cadena {fallidos:>3}, acciones {len(acciones):>3}, preferencias {len(generales)}")
    doc["rutas"] = nuevas
    doc["nota"] = ("Rutas sintetizadas por tools/sintetizar_rutas.py a partir de las aventuras (AV18-04b) y ampliadas a "
                   "CUATRO dias (UNI-02-11 E5: 1 primer_dia, 2 dia_gorran, 3 segundo_dia, 4 ocaso): cada premio con su cadena "
                   "de beats, cinematicas (accion «pisar») y eventos del mundo. Exitos y victorias impuestos, moral neutral, "
                   "sin tiempo. LIOS DE GORRAN: como la demo (laEspadaDeRosita), la accion «poner gorran_misiones=3» da por "
                   "hechos los cuatro lios de la aldea (Cal Serra, Veta Honda, Galerias, mina vieja); sus premios NO se "
                   "recorren. Sin cadena por diseno: 'Os he salvado la vida' y rec_fuego_regalo (excluyen la Barriada del "
                   "dia 3) y las opciones que unen al heroe a la secta (la Barriada lo rechaza).")
    if not seco:
        open(ruta_json, "w", encoding="utf-8").write(json.dumps(doc, ensure_ascii=False, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
