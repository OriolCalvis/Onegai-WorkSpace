#!/usr/bin/env python3
"""Pruebas de tools/medir_objetivos.py (solo python3: python3 tools/test_medir_objetivos.py).

Montan un mundo de mentira en una carpeta temporal: un Casco Antiguo con dos
puertas (taberna y herreria), una cueva a la que no lleva ninguna puerta, un
prado sin puertas (una escena aparte) y tres aventuras/fichas.

Con MEDIR_EXIGE_REAL=1 comprueba ademas el proyecto de verdad: ningun objetivo
de Boundington sin destino o que no se resuelva.
"""
import contextlib
import io
import json
import os
import shutil
import sys
import tempfile
import unittest

AQUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, AQUI)
import medir_objetivos as m  # noqa: E402


def escribe(raiz, ruta, datos):
    destino = os.path.join(raiz, *ruta.split("/"))
    os.makedirs(os.path.dirname(destino), exist_ok=True)
    with open(destino, "w", encoding="utf-8") as f:
        json.dump(datos, f, ensure_ascii=False)


def nivel(*objetos):
    """Un nivel con objetos: ("id",) o ("id", "assets/levels/otro.json") para una puerta."""
    lista = []
    for o in objetos:
        e = {"objectId": o[0], "position": {"x": 0, "y": 0}}
        if len(o) > 1:
            e["targetLevel"] = o[1]
        lista.append(e)
    return {"name": "n", "objects": lista}


def mundo():
    raiz = tempfile.mkdtemp(prefix="medir_objetivos_")
    niv = "assets/levels/"
    escribe(raiz, niv + "ciudad_centro.json", nivel(("puerta_taberna", niv + "interior_taberna.json"),
                                                      ("puerta_herreria", niv + "interior_herreria.json")))
    escribe(raiz, niv + "interior_taberna.json", nivel(("salida", niv + "ciudad_centro.json"), ("skilla",)))
    escribe(raiz, niv + "interior_herreria.json", nivel(("salida", niv + "ciudad_centro.json"), ("aigren",)))
    escribe(raiz, niv + "cueva.json", nivel(("gorran",)))
    escribe(raiz, niv + "ejemplo.json", nivel(("puerta_cueva", niv + "cueva.json")))
    escribe(raiz, niv + "prado.json", nivel(("sho_noco",), ("cofre",)))
    escribe(raiz, "assets/adventures/a.json", {"id": "adv_boundington_a", "objectives": [
        {"id": "sin", "text": "Sin sitio", "doneFlag": "f1"},
        {"id": "nivel_ok", "text": "Ve a la taberna", "doneFlag": "f2", "donde": niv + "interior_taberna.json"},
        {"id": "persona", "text": "Habla con Aigren", "doneFlag": "f3", "donde": [niv + "interior_herreria.json", "aigren"]},
        {"id": "suelto", "text": "Habla con Rosita", "doneFlag": "f4", "donde": ["skilla"]},
        {"id": "nivel_roto", "text": "Roto", "doneFlag": "f5", "donde": niv + "no_existe.json"},
        {"id": "objeto_roto", "text": "Roto", "doneFlag": "f6", "donde": [niv + "interior_taberna.json", "nadie"]},
        {"id": "cueva", "text": "A la cueva", "doneFlag": "f7", "donde": niv + "cueva.json"},
        {"id": "cueva_y_taberna", "text": "Cueva o taberna", "doneFlag": "f8",
         "donde": [niv + "cueva.json", niv + "interior_taberna.json"]},
        {"id": "escena", "text": "El cofre", "doneFlag": "f9", "donde": [niv + "prado.json", "cofre"]},
    ]})
    escribe(raiz, "assets/adventures/b.json", {"id": "adv_fuera", "objectives": [
        {"id": "fuera_sin", "text": "Sin sitio", "doneFlag": "g1"}]})
    escribe(raiz, "assets/catalogs/quests.json", {"schemaVersion": 1, "entries": [
        {"id": "bnd_ficha", "objectives": [
            {"id": "op", "description": "Opcional", "optional": True, "doneFlag": "h1"},
            {"id": "op2", "description": "Opcional con sitio", "optional": True, "doneFlag": "h2",
             "donde": [niv + "interior_herreria.json", "aigren"]}]},
        {"id": "mision_llave", "objectives": [{"id": "llave", "description": "Prueba", "doneFlag": "h3"}]},
        {"id": "bnd_prologo", "name": "sin objetivos propios"}]})
    return raiz


class MedirObjetivos(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raiz = mundo()
        cls.por_id = {"%s/%s" % (o["mision"], o["id"]): o for o in m.medir(cls.raiz)}

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.raiz, ignore_errors=True)

    def estado(self, clave):
        return self.por_id[clave]["estado"]

    def test_nombre_y_donde(self):
        self.assertEqual(m.nombre("assets/levels/x.json"), "x.json")
        self.assertEqual(m.nombre("assets\\levels\\x.json"), "x.json")
        self.assertEqual(m.lista_donde("a"), ["a"])
        self.assertEqual(m.lista_donde(["a", "b"]), ["a", "b"])
        self.assertEqual(m.lista_donde(None), [])
        self.assertEqual(m.lista_donde(""), [])
        self.assertTrue(m.es_nivel("assets/levels/x.json"))
        self.assertTrue(m.es_nivel("x.json"))
        self.assertFalse(m.es_nivel("sho_noco"))

    def test_cuenta_los_de_aventuras_y_los_de_fichas(self):
        self.assertEqual(len(self.por_id), 9 + 1 + 2 + 1)

    def test_sin_destino(self):
        self.assertEqual(self.estado("adv_boundington_a/sin"), "SIN_DESTINO")
        self.assertEqual(self.estado("adv_fuera/fuera_sin"), "SIN_DESTINO")
        self.assertEqual(self.estado("bnd_ficha/op"), "SIN_DESTINO")

    def test_nivel_u_objeto_que_se_siguen(self):
        self.assertEqual(self.estado("adv_boundington_a/nivel_ok"), "OK")
        self.assertEqual(self.estado("adv_boundington_a/persona"), "OK")
        self.assertEqual(self.estado("adv_boundington_a/suelto"), "OK")
        self.assertEqual(self.estado("bnd_ficha/op2"), "OK")  # una ficha de mision (clave `description`)

    def test_lo_que_no_se_resuelve(self):
        self.assertEqual(self.estado("adv_boundington_a/nivel_roto"), "NO_RESUELVE")
        self.assertEqual(self.estado("adv_boundington_a/objeto_roto"), "NO_RESUELVE")
        # La cueva existe pero ninguna puerta alcanzable lleva hasta ella.
        self.assertEqual(self.estado("adv_boundington_a/cueva"), "NO_RESUELVE")
        self.assertTrue(any("sin camino" in n for n in self.por_id["adv_boundington_a/cueva"]["notas"]))

    def test_una_alternativa_con_camino_basta(self):
        o = self.por_id["adv_boundington_a/cueva_y_taberna"]
        self.assertEqual(o["estado"], "OK")
        self.assertTrue(any("cueva.json" in n for n in o["notas"]))

    def test_una_escena_sin_puertas_vale(self):
        self.assertEqual(self.estado("adv_boundington_a/escena"), "OK_ESCENA")

    def test_pruebas_y_ambito(self):
        self.assertTrue(self.por_id["mision_llave/llave"]["prueba"])
        self.assertFalse(self.por_id["bnd_ficha/op"]["prueba"])
        self.assertTrue(self.por_id["bnd_ficha/op"]["boundington"])
        self.assertFalse(self.por_id["adv_fuera/fuera_sin"]["boundington"])

    def test_resumen_corto(self):
        lineas = m.resumen(list(self.por_id.values()))
        texto = "\n".join(lineas)
        self.assertIn("De prueba (no cuentan): 1", texto)
        self.assertIn("Boundington: 11 objetivos", texto)  # 9 + 2 de la ficha, sin la de prueba
        self.assertIn("Fuera de Boundington: 1 objetivos", texto)
        self.assertLess(len(lineas), 40)

    def test_detalle_dice_el_texto_y_el_donde(self):
        texto = "\n".join(m.detalle(list(self.por_id.values())))
        self.assertIn("estado: NO_RESUELVE", texto)
        self.assertIn("texto:  Habla con Aigren", texto)
        self.assertIn('donde:  ["assets/levels/interior_herreria.json", "aigren"]', texto)

    def test_la_linea_de_ordenes(self):
        with contextlib.redirect_stdout(io.StringIO()) as salida:
            self.assertEqual(m.principal(["--raiz", self.raiz]), 0)
            self.assertEqual(m.principal(["--raiz", self.raiz, "--exigir"]), 1)  # hay objetivos mal
        self.assertIn("sin destino", salida.getvalue())
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(m.principal(["--raiz", os.path.join(self.raiz, "nada")]), 2)


@unittest.skipUnless(os.environ.get("MEDIR_EXIGE_REAL") == "1", "con MEDIR_EXIGE_REAL=1 mide el proyecto de verdad")
class ProyectoReal(unittest.TestCase):
    def test_boundington_todo_con_destino(self):
        raiz = os.path.abspath(os.path.join(AQUI, ".."))
        mal = [o for o in m.medir(raiz) if o["boundington"] and not o["prueba"]
               and o["estado"] in ("SIN_DESTINO", "NO_RESUELVE")]
        self.assertEqual([], ["%s/%s" % (o["mision"], o["id"]) for o in mal])


if __name__ == "__main__":
    unittest.main(verbosity=1)
