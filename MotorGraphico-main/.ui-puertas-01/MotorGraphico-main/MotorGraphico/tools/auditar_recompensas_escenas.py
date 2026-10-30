#!/usr/bin/env python3
"""auditar_recompensas_escenas.py — premios de las escenas de Boundington contra ECONOMIA.md (AV18-08).

El libro tools/recompensas_escenas.json registra cada grantGold / darObjeto positivo de las
aventuras de Boundington con su dia, canal y grupo de alternativas excluyentes. Aqui se comprueba:

  1. Cada premio del libro existe donde dice (aventura, beat, opcion) y paga lo que dice.
  2. Todo premio positivo de las aventuras esta en el libro (nada sin contar).
  3. Un premio de oro lleva su linea de log con la misma cifra.
  4. El premio no se puede cobrar dos veces: el beat o la opcion pone una bandera que el mismo
     beat u opcion exige ausente (requires.notFlags).
  5. Los objetos existen en los catalogos y tienen precio.
  6. Valor (oro + precio x cantidad) del canal "misiones" por dia: de cada grupo de alternativas
     excluyentes cuenta la mas alta; el total ha de quedar a +-tolerancia del presupuesto de
     ECONOMIA.md (suma de los canales "misiones" del dia). Los combates se informan aparte.

Solo stdlib. Sin argumentos audita el repo; "--raiz <MotorGraphico>" para pruebas.
Exit 1 si hay errores. validar_todo.py lo llama: errores -> error, notas -> aviso.
"""
from __future__ import annotations
import json, re, sys
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
MOTOR_DEF = TOOLS.parent


def _cargar(p: Path):
    return json.loads(p.read_text(encoding="utf-8"))


def _economia(motor: Path, ruta_rel: str):
    for base in (motor, *motor.parents):
        p = base / ruta_rel
        if p.exists():
            m = re.search(r"```json economia\n(.*?)```", p.read_text(encoding="utf-8"), re.S)
            if not m:
                raise ValueError(f"{ruta_rel}: sin bloque ```json economia")
            return json.loads(m.group(1))
    raise FileNotFoundError(ruta_rel)


def _precios(motor: Path) -> dict:
    out = {}
    for f in sorted((motor / "assets" / "objects").glob("*.json")):
        try:
            d = _cargar(f)
        except Exception:
            continue
        lst = d.get("objects", []) if isinstance(d, dict) else d
        for o in lst if isinstance(lst, list) else []:
            if isinstance(o, dict) and "id" in o and isinstance(o.get("price"), (int, float)):
                out.setdefault(o["id"], o["price"])
    return out


_LETRAS = {10: "diez", 15: "quince", 20: "veinte", 25: "veinticinco", 30: "treinta", 40: "cuarenta",
           50: "cincuenta", 60: "sesenta", 70: "setenta", 80: "ochenta", 90: "noventa", 100: "cien"}


def _dice(texto: str, n: int) -> bool:
    return bool(re.search(rf"\b{n}\b", texto)) or (n in _LETRAS and _LETRAS[n] in texto.lower())


def _premios_de(efectos):
    """[(tipo, arg, cantidad)] de los efectos positivos."""
    r = []
    for e in efectos or []:
        t = e.get("type")
        if t == "grantGold" and e.get("value", 0) > 0:
            r.append(("oro", "", int(e["value"])))
        elif t == "darObjeto" and e.get("value", 1) > 0:
            r.append(("obj", e.get("arg", ""), int(e.get("value", 1))))
    return r


def comprueba(motor: Path = MOTOR_DEF):
    """-> (resumen dict, errores [str], notas [str])"""
    errs, notas = [], []
    libro = _cargar(motor / "tools" / "recompensas_escenas.json")
    eco = _economia(motor, libro["economia"])
    precios = _precios(motor)
    tol = libro.get("tolerancia_pct", 15) / 100.0
    canales = eco["canales"]
    i_mis, i_com = canales.index("misiones"), canales.index("combates")
    presup = {d: [0, 0] for d in (1, 2, 3)}
    for p in eco["presupuestos"]:
        presup[p["dia"]][0] += p["valor"][i_mis]
        presup[p["dia"]][1] += p["valor"][i_com]

    avs = {}
    for a in libro["aventuras"]:
        avs[a] = {b["id"]: b for b in _cargar(motor / "assets" / "adventures" / f"{a}.json")["beats"]}

    registrados = set()
    grupos = {}  # (dia, canal, grupo) -> max valor
    oro_total = {1: 0, 2: 0, 3: 0}
    ids = set()
    for en in libro["premios"]:
        pid = en["id"]
        if pid in ids:
            errs.append(f"{pid}: id repetido")
        ids.add(pid)
        beat = avs.get(en["aventura"], {}).get(en["beat"])
        if beat is None:
            errs.append(f"{pid}: no existe el beat {en['aventura']}/{en['beat']}")
            continue
        op = en.get("opcion")
        if op is None:
            efectos, req = beat.get("effects", []), beat.get("requires", {})
            reqs_nf = set(req.get("notFlags", []))
        else:
            ops = beat.get("opciones", [])
            if op >= len(ops):
                errs.append(f"{pid}: {en['beat']} no tiene la opcion {op}")
                continue
            efectos = ops[op].get("effects", [])
            reqs_nf = set(beat.get("requires", {}).get("notFlags", [])) | set(ops[op].get("requires", {}).get("notFlags", []))
        registrados.add((en["aventura"], en["beat"], op))
        mision_obj = set(libro.get("objetos_de_mision", {}))
        prem = [q for q in _premios_de(efectos) if not (q[0] == "obj" and q[1] in mision_obj)]
        if not prem:
            errs.append(f"{pid}: {en['beat']} opcion {op} no paga nada")
            continue
        valor = 0
        for t, arg, n in prem:
            if t == "oro":
                valor += n
                textos = [e.get("arg", "") for e in efectos if e.get("type") == "log"]
                textos += (beat if op is None else beat["opciones"][op]).get("lines", [])
                if not any(_dice(t, n) for t in textos):
                    errs.append(f"{pid}: el oro ({n}) no aparece en ningun log ni linea del premio (en cifra o en letra)")
            else:
                if arg not in precios:
                    errs.append(f"{pid}: objeto '{arg}' sin precio en los catalogos")
                valor += precios.get(arg, 0) * n
        flags = {e.get("arg") for e in efectos + ([] if op is None else beat.get("effects", [])) if e.get("type") == "setFlag"}
        if not (flags & reqs_nf):
            errs.append(f"{pid}: se puede cobrar mas de una vez (ninguna bandera que pone esta en sus notFlags)")
        k = (en["dia"], en["canal"], en["grupo"])
        grupos[k] = max(grupos.get(k, 0), valor)

    # premios sin registrar
    for a, bs in avs.items():
        for bid, b in bs.items():
            sitios = [(None, b.get("effects", []))] + [(i, o.get("effects", [])) for i, o in enumerate(b.get("opciones", []))]
            for op, efs in sitios:
                prem = [p for p in _premios_de(efs) if not (p[0] == "obj" and p[1] in mision_obj)]
                if prem and (a, bid, op) not in registrados:
                    errs.append(f"{a}/{bid} opcion {op}: paga {prem} y no esta en el libro")

    for (dd, canal, _), v in grupos.items():
        if canal == "combates":
            oro_total[dd] += v
    resumen = {}
    if "--grupos" in sys.argv:
        for k, v in sorted(grupos.items()):
            print("grupo", k, v)
    for d in (1, 2, 3):
        mis = sum(v for (dd, c, _), v in grupos.items() if dd == d and c == "misiones")
        pm, pc = presup[d]
        dev = (mis - pm) / pm if pm else 0
        resumen[d] = {"misiones": mis, "presupuesto_misiones": pm, "desvio_pct": round(dev * 100, 1),
                      "combates_narrativos": oro_total[d], "presupuesto_combates": pc}
        if abs(dev) > tol:
            errs.append(f"dia {d}: misiones {mis} frente a {pm} ({dev*100:+.0f}%), fuera de +-{tol*100:.0f}%")
        if pc and oro_total[d] > pc * libro.get("tope_combates_narrativos_pct", 50) / 100:
            notas.append(f"dia {d}: los combates narrativos suman {oro_total[d]} de {pc} de presupuesto de combates")
    return resumen, errs, notas


def main(argv):
    motor = MOTOR_DEF
    if "--raiz" in argv:
        motor = Path(argv[argv.index("--raiz") + 1])
    resumen, errs, notas = comprueba(motor)
    for d, r in resumen.items():
        print(f"dia {d}: misiones {r['misiones']}/{r['presupuesto_misiones']} ({r['desvio_pct']:+}%), "
              f"combates narrativos {r['combates_narrativos']}/{r['presupuesto_combates']}")
    for n in notas:
        print("nota:", n)
    for e in errs:
        print("ERROR:", e)
    print(f"RESULTADO: {'OK' if not errs else 'FALLA'} — {len(errs)} error(es), {len(notas)} nota(s)")
    return 1 if errs else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
