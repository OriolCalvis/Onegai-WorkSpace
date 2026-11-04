#!/usr/bin/env python3
"""Prueba de tools/matriz_misiones.py (E-MISIONES-01): todos los beats del dia 1 caen en UNA mision, el inventario sale estable,
las misiones conocidas traen su cierre, y el analisis de alcanzabilidad detecta un cierre al que nadie puede llegar (mutacion).
Uso: python3 -I tools/test_matriz_misiones.py (desde MotorGraphico/). Solo biblioteca estandar."""
import json
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matriz_misiones as mm  # noqa: E402


def comprueba(cond, texto):
    if not cond:
        print("FALLO: " + texto)
        sys.exit(1)


inv = mm.inventario("primer_dia")
comprueba(inv["n_beats"] == 393, "el dia 1 tiene 393 beats (hay %d)" % inv["n_beats"])
comprueba(inv["sin_mision"] == [], "ningun beat sin mision: %s" % inv["sin_mision"][:5])
comprueba(inv["dobles"] == [], "ningun beat en dos misiones: %s" % inv["dobles"][:3])
ms = {m["id"]: m for m in inv["misiones"]}
comprueba(len(ms) >= 20, "al menos 20 misiones (hay %d)" % len(ms))
comprueba(sum(m["n"] for m in ms.values()) == 393, "las misiones suman los 393 beats")
comprueba(all(m["cerrable"]["ok"] for m in ms.values()), "todas las misiones del dia 1 se pueden cerrar (estatico)")
comprueba(all(m["cierre_beats"] for m in ms.values()), "toda mision tiene un beat que enciende su cierre")
# Misiones conocidas: el gancho, el cierre y el diario.
comprueba(ms["aigren"]["abre"]["beat"] == "b1_aigren_charla" and "bnd1_aigren_resuelto" in ms["aigren"]["cierre_flags"], "Aigren se abre con b1_aigren_charla y cierra con bnd1_aigren_resuelto")
comprueba(any(e["id"] == "obj_xila" for e in ms["xila"]["diario"]), "Xila tiene su entrada en el diario (obj_xila)")
comprueba(any(e["id"] == "obj_pruebas" for e in ms["iglesia"]["diario"]), "la iglesia cierra obj_pruebas")
comprueba(ms["presentaciones"]["recompensa"]["niveles"] > 1.0, "las presentaciones dan nivel")
comprueba(ms["verina_edul"]["recompensa"]["oro_pos"] >= 100, "la guarida paga oro (Verina/Edul)")
comprueba(any(s["tipo"] == "combate" for s in ms["familia"]["salidas"]), "la familia tiene un combate (los Verdaderos)")
comprueba(any(s["tipo"] == "tirada" for s in ms["descubierto"]["salidas"]), "descubierto tiene tiradas")
# Determinista.
with tempfile.TemporaryDirectory() as d:
    for n in (1, 2):
        mm.main(["--dia", "primer_dia", "--md", os.path.join(d, "a%d.md" % n), "--json", os.path.join(d, "a%d.json" % n)])
    for ext in ("md", "json"):
        comprueba(open(os.path.join(d, "a1." + ext), encoding="utf-8").read() == open(os.path.join(d, "a2." + ext), encoding="utf-8").read(),
                  "la salida %s es estable" % ext)
    # Mutacion: los dos beats que encienden bnd0_dormido piden ahora una flag que nadie pone -> la primera noche ya no se puede cerrar.
    ruta = os.path.join(mm.RAIZ, "assets/adventures/boundington_primer_dia.json")
    adv = json.load(open(ruta, encoding="utf-8"))
    tocados = 0
    for b in adv["beats"]:
        if b["id"] in ("b0_subir", "b0_subir_oido"):
            b["requires"].setdefault("allFlags", []).append("zz_flag_que_nadie_pone")
            tocados += 1
    comprueba(tocados == 2, "la mutacion toca los dos beats de dormir")
    tmp = os.path.join(d, "mut.json")
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(adv, f, ensure_ascii=False)
    mut = mm.inventario("primer_dia", tmp)
    m2 = {m["id"]: m for m in mut["misiones"]}
    comprueba(not m2["llegada_cama"]["cerrable"]["ok"], "mutada: la primera noche ya no se puede cerrar")
    comprueba("zz_flag_que_nadie_pone" in " ".join(c["motivo"] for c in m2["llegada_cama"]["cerrable"]["cierres"]), "...y el motivo nombra la flag")
    comprueba(m2["aigren"]["cerrable"]["ok"], "mutada: las demas misiones siguen cerrando")
    # Mutacion 2: el PNJ de un cierre se esconde con lo que el propio beat exige (cinematicas.json, `apariciones`).
    adv = json.load(open(ruta, encoding="utf-8"))
    for b in adv["beats"]:
        if b["id"] == "b1_aigren_envio":
            b["trigger"]["target"] = "pnj_que_no_esta_en_ningun_nivel"
    tmp2 = os.path.join(d, "mut2.json")
    with open(tmp2, "w", encoding="utf-8") as f:
        json.dump(adv, f, ensure_ascii=False)
    mut2 = mm.inventario("primer_dia", tmp2)
    comprueba(any(not c["ok"] and "PNJ" in c["motivo"] for m in mut2["misiones"] if m["id"] == "aigren" for c in m["cerrable"]["cierres"]), "mutada: un cierre con un PNJ sin nivel sale ⚠")
# Cobertura del explorador (si el log esta): descubierto y la entrada a la guarida casi no se ven.
if inv["log"]:
    comprueba(ms["descubierto"]["cobertura"]["vistos"] <= 2 and ms["entrada_guarida"]["cobertura"]["vistos"] < ms["entrada_guarida"]["cobertura"]["total"], "el explorador deja sin ver parte de la entrada a la guarida")
print("OK: %d misiones, %d cerrables" % (len(ms), sum(1 for m in ms.values() if m["cerrable"]["ok"])))
