#!/usr/bin/env python3
"""AV18-04b. Sintetiza las rutas de tools/rutas_economia.json a partir de las aventuras.

El simulador (simular_economia.py) recorre las aventuras con recorrer_recompensas
siguiendo una lista de acciones por perfil. Escribirlas a mano deja fuera la mayoria
de los premios (los beats `auto` que exigen banderas de combates, pruebas y charlas
anteriores). Aqui se calcula, para cada premio (grantGold > 0 o darObjeto), la cadena
de beats que lo desbloquea, y de ahi las acciones: entrar en un nivel, hablar con un
PNJ y que opcion preferir. Los combates se ganan y las pruebas salen con exito
(`victoria`/`grado` de la ruta), como ya suponia Codex.

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
AVENTURAS = {1: "boundington_primer_dia.json", 2: "boundington_segundo_dia.json",
             3: "boundington_ocaso.json"}
# Banderas que no pone ninguna aventura de los tres dias: vienen del prologo/precuela o
# de la creacion del personaje. Se dan por puestas al empezar (flags_iniciales).
INICIALES = ["bnd_prologo_terminado", "heroe_pueblo_humano", "heroe_pueblo_de_aegroum",
             "heroe_rol_tank"]
# Opciones que cierran el juego de otra manera o cuestan mas de lo que dan: no se eligen
# aunque desbloqueen algo (se puede ampliar).
EVITAR_OPCIONES = {"Unirte de verdad"}


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


class Sintetizador:
    def __init__(self, aventuras):
        self.av = aventuras                      # dia -> Aventura
        self.acciones = {}                       # dia -> [accion]
        self.generales = defaultdict(list)       # dia -> preferencias
        self.hechos = set()                      # (dia, beat, idx)
        self.combate = False

    def quien_pone(self, dia, flag):
        for d in sorted(self.av):
            if d > dia:
                break
            if flag in self.av[d].pone:
                return d, self.av[d].pone[flag]
        return None, []

    def resolver_flag(self, dia, flag, pila):
        if flag in INICIALES or flag in pila:
            return True
        d, proveedores = self.quien_pone(dia, flag)
        if not proveedores:
            return False
        # Preferir lo que no exige opcion (beat entero) y lo que exige menos banderas.
        def coste(p):
            b = self.av[d].beats[p[0]]
            req = b.get("requires") or {}
            return (p[1] is not None, len(req.get("allFlags", [])) + len(req.get("anyFlags", [])))
        for beat_id, idx, tipo in sorted(proveedores, key=coste):
            if tipo == "victoria":
                self.combate = True
            if self.disparar(d, beat_id, idx, pila | {flag}):
                return True
        return False

    def disparar(self, dia, beat_id, idx, pila=frozenset()):
        """Anade las acciones para que el beat (y su opcion) se dispare. True si se pudo."""
        clave = (dia, beat_id, idx)
        if clave in self.hechos:
            return True
        av = self.av[dia]
        b = av.beats[beat_id]
        if idx is not None:
            o = b["opciones"][idx]
            if o["texto"] in EVITAR_OPCIONES:
                return False
        req = b.get("requires") or {}
        for f in req.get("allFlags", []):
            if not self.resolver_flag(dia, f, pila):
                return False
        if req.get("anyFlags") and not any(self.resolver_flag(dia, f, pila) for f in req["anyFlags"]):
            return False
        if idx is not None:
            oreq = b["opciones"][idx].get("requires") or {}
            for f in oreq.get("allFlags", []):
                if not self.resolver_flag(dia, f, pila):
                    return False
            if oreq.get("anyFlags") and not any(self.resolver_flag(dia, f, pila) for f in oreq["anyFlags"]):
                return False
        # Las banderas del beat anterior ya estan: ahora la accion que lo dispara.
        self.hechos.add(clave)
        t = b.get("trigger", {})
        acciones = self.acciones.setdefault(dia, [])
        pref = [b["opciones"][idx]["texto"]] if idx is not None else []
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

    ruta_json = MOTOR / "tools/rutas_economia.json"
    # El esqueleto de cada dia (empezar en la taberna, cerrar la noche con Skilla y
    # Venides) es el de Codex (rutas_economia_base.json): las cadenas de premios se
    # insertan entre el arranque y el cierre de cada dia.
    base = {r["perfil"]: r for r in json.load(open(MOTOR / "tools/rutas_economia_base.json", encoding="utf-8"))["rutas"]}
    doc = json.load(open(ruta_json, encoding="utf-8"))
    nuevas = []
    for perfil in economia["perfiles"]:
        s = Sintetizador(aventuras)
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
            if not s.resolver_flag(3, f, frozenset()):
                print(f"  AVISO: no hay cadena hasta {f}")
        acciones = []
        esqueleto = base[perfil]["acciones"]
        for dia in (1, 2, 3):
            codex = [dict(a) for a in esqueleto if a["dia"] == dia]
            cierre_desde = next((i for i, a in enumerate(codex)
                                 if "Esperar a que anochezca" in a.get("preferencias", [])), len(codex))
            arranque, cierre = codex[:cierre_desde], codex[cierre_desde:]
            if dia == 1:
                arranque = codex[:2]  # entrar en la taberna y hablar con Skilla
                cierre = codex[cierre_desde:] if cierre_desde < len(codex) else codex[2:]
            if dia == 2:
                arranque = codex[:4]
            lista = arranque + [a for a in s.acciones.get(dia, []) if not a.get("accion") == "auto" or not arranque] + cierre
            for i, a in enumerate(lista):
                a = dict(a); a["orden"] = 2 + 2 * i
                if not a.get("preferencias"):
                    a.pop("preferencias", None)
                acciones.append(a)
        generales = list(base[perfil]["preferencias"])
        for dia in (1, 2, 3):
            for t in s.generales[dia]:
                if t not in generales:
                    generales.append(t)
        for t in ["Esperar a que anochezca", "Pagar la cama", "Subir a dormir", "Nada más", "Volver",
                  "Dejarlo estar", "Contárselo todo", "Contarle lo que tienes", "Aceptar el encargo"]:
            if t not in generales:
                generales.append(t)
        # El recorrido exige una preferencia para CADA beat con opciones que encuentre: de
        # respaldo, la primera opcion de cada beat (las elegidas arriba van antes y mandan).
        for dia in (1, 2, 3):
            for b in aventuras[dia].beats.values():
                for o in b.get("opciones", []):
                    if o["texto"] not in EVITAR_OPCIONES:
                        if o["texto"] not in generales:
                            generales.append(o["texto"])
                        break
        nuevas.append(dict(perfil=perfil, grado="exito", victoria=True, cara_tabla=1,
                           flags_iniciales=INICIALES,
                           flags_finales=["bnd3_fuera", "bnd3_epilogo"],
                           preferencias=generales, acciones=acciones))
        print(f"{perfil:13} premios cubiertos {logrados:>3}, sin cadena {fallidos:>3}, acciones {len(acciones):>3}, preferencias {len(generales)}")
    doc["rutas"] = nuevas
    doc["nota"] = ("Rutas sintetizadas por tools/sintetizar_rutas.py a partir de las aventuras (AV18-04b): "
                   "cada premio con su cadena de beats. Éxitos y victorias impuestos, moral neutral, sin tiempo.")
    if not seco:
        open(ruta_json, "w", encoding="utf-8").write(json.dumps(doc, ensure_ascii=False, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
