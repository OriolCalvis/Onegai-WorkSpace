#!/usr/bin/env python3
"""Prueba de tools/matriz_decisiones.py (E-MATRIZ-01): el inventario del dia 1 sale, es estable y trae las decisiones
que se sabe que pesan. Uso: python3 -I tools/test_matriz_decisiones.py (desde MotorGraphico/). Solo biblioteca estandar."""
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matriz_decisiones as m  # noqa: E402


def comprueba(cond, texto):
    if not cond:
        print("FALLO: " + texto)
        sys.exit(1)


filas, beats, ruta, lect = m.inventario("primer_dia")
comprueba(len(beats) == 393, "el dia 1 tiene 393 beats (hay %d)" % len(beats))
comprueba(len(filas) == 206, "206 beats con elecciones (hay %d)" % len(filas))
por_beat = {f["beat"]: f for f in filas}
edul = por_beat["b1_edul"]
unirte = [o for o in edul["opciones"] if o["texto"] == "Unirte de verdad"][0]
comprueba("bnd1_unido_al_culto" in unirte["pone"], "Unirte de verdad pone bnd1_unido_al_culto")
comprueba(any(x["dia"] == "dia_gorran" for x in unirte["directas"].get("bnd1_unido_al_culto", [])), "...y la lee el dia de Gorran")
comprueba(m.es_importante(unirte), "Unirte de verdad es importante")
dudas = [o for o in edul["opciones"] if o["texto"] == "Sembrar dudas en Verina"][0]
comprueba(any(x["dia"] == "ocaso" for x in dudas["directas"].get("bnd1_dudas_verina", [])), "Sembrar dudas lo lee el Ocaso")
comprueba(m.peso_opcion(unirte) > m.peso_opcion(dudas) or m.peso_opcion(dudas) > 0, "los pesos son positivos")
# Determinista: dos pasadas dan el mismo markdown y el mismo json.
with tempfile.TemporaryDirectory() as d:
    for n in (1, 2):
        m.main(["--dia", "primer_dia", "--md", os.path.join(d, "a%d.md" % n), "--json", os.path.join(d, "a%d.json" % n)])
    for ext in ("md", "json"):
        comprueba(open(os.path.join(d, "a1." + ext), encoding="utf-8").read() == open(os.path.join(d, "a2." + ext), encoding="utf-8").read(),
                  "la salida %s es estable" % ext)
    texto = open(os.path.join(d, "a1.md"), encoding="utf-8").read()
    comprueba("b1_edul" in texto, "el md nombra b1_edul")
print("OK matriz_decisiones: %d beats con alguna opcion importante en el dia 1" % sum(1 for f in filas if any(m.es_importante(o) for o in f["opciones"])))
