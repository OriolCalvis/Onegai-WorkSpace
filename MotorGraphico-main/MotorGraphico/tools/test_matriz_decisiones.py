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
# E-MATRIZ-02 — el dia de Gorran, con el diario como lector y sin contar la guarda del propio beat.
gf, gb, gr, gl = m.inventario("dia_gorran")
g_por = {f["beat"]: f for f in gf}
vic = g_por["b2_gorran_victoria"]
perdona = [o for o in vic["opciones"] if o["texto"].startswith("Suelta a los presos")][0]
acaba = [o for o in vic["opciones"] if o["texto"].startswith("Acabar con")][0]
comprueba(any(x["dia"] == "ocaso" and x["cual"] == "b3_gorran" for x in perdona["directas"].get("bnd2_gorran_perdonado", [])), "perdonar a Gorran lo lee el Ocaso (b3_gorran)")
comprueba(any(x["cual"] == "b3_gorran_sin_jefe" for x in acaba["directas"].get("bnd2_gorran_muerto", [])), "acabar con Gorran lo lee el Ocaso (b3_gorran_sin_jefe)")
comprueba(any(x["tipo"] == "cinematica" and x["cual"] == "afueras_gorran_presente" for x in perdona["directas"].get("bnd2_gorran_perdonado", [])),
          "... y la cinematica afueras_gorran_presente")
comprueba(any(x["tipo"] == "diario" for x in perdona["directas"].get("bnd2_tiene_aliados", [])), "bnd2_tiene_aliados la lee el diario (objetivo obj2_aliados)")
comprueba(all(x["tipo"] == "diario" for x in perdona["directas"].get("bnd2_tiene_aliados", [])), "... y solo el diario")
comprueba(m.es_importante(acaba) and m.peso_opcion(acaba) > 5, "acabar con Gorran pesa")
gav = m.hallazgos(gf, gb, "dia_gorran", gl, gr)
sin = {a["dato"] for a in gav if a["tipo"] == "flag-sin-lector"}
comprueba("bnd2_gorran_tregua" not in sin, "la tregua con Gorran ya la lee la pagina que la cumple (b2_subasta_tregua, D0024)")
comprueba("bnd2_gorran_perdonado" not in sin, "perdonar a Gorran si se lee")
comprueba(any(a["tipo"] == "solo-diario" and a["dato"] == "bnd2_tiene_aliados" for a in gav), "bnd2_tiene_aliados solo la lee el diario")
with tempfile.TemporaryDirectory() as d:
    for n in (1, 2):
        m.main(["--dia", "dia_gorran", "--md", os.path.join(d, "g%d.md" % n), "--json", os.path.join(d, "g%d.json" % n)])
    for ext in ("md", "json"):
        comprueba(open(os.path.join(d, "g1." + ext), encoding="utf-8").read() == open(os.path.join(d, "g2." + ext), encoding="utf-8").read(),
                  "la salida del dia de Gorran (%s) es estable" % ext)
# E-MATRIZ-03 — el dia 3 (boundington_segundo_dia): inventario reproducible; el informe a Venides y la fiesta se leen en el Ocaso.
tf, tb, tr, tl = m.inventario("segundo_dia")
t_por = {f["beat"]: f for f in tf}
informe = t_por["b2_noche_rosita_informa"]
callar = [o for o in informe["opciones"] if o["texto"] == "No contarle nada"][0]
comprueba(any(x["dia"] == "ocaso" and x["cual"] == "b3_venides_retenido" for x in callar["directas"].get("bnd2_venides_sin_aviso", [])),
          "no contarle nada a Venides lo lee el Ocaso (b3_venides_retenido)")
comprueba(any(x["tipo"] == "evento" for x in callar["directas"].get("bnd2_venides_sin_aviso", [])), "... y el evento xila_avisa_a_venides")
comprueba(m.es_importante(callar), "no contarle nada es importante")
cocina = t_por["b2_fiesta_cocina"]
comprueba(any(o["texto"] == "Entrar por la cocina" for o in cocina["opciones"]), "la cocina es una opcion de la fiesta")
with tempfile.TemporaryDirectory() as d:
    for n in (1, 2):
        m.main(["--dia", "segundo_dia", "--md", os.path.join(d, "t%d.md" % n), "--json", os.path.join(d, "t%d.json" % n)])
    for ext in ("md", "json"):
        comprueba(open(os.path.join(d, "t1." + ext), encoding="utf-8").read() == open(os.path.join(d, "t2." + ext), encoding="utf-8").read(),
                  "la salida del dia 3 (%s) es estable" % ext)
print("OK matriz_decisiones: %d beats con alguna opcion importante en el dia 1" % sum(1 for f in filas if any(m.es_importante(o) for o in f["opciones"])))
