#!/usr/bin/env python3
"""Pruebas de tools/validar_oficios.py. Sin dependencias: python3 tools/test_validar_oficios.py"""
import copy
import json
import os
import sys
import tempfile
import unittest

AQUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, AQUI)
import validar_oficios as V  # noqa: E402

RAIZ = os.path.abspath(os.path.join(AQUI, ".."))


def _escribe(ruta, dato):
    os.makedirs(os.path.dirname(ruta), exist_ok=True)
    with open(ruta, "w", encoding="utf-8") as f:
        json.dump(dato, f, ensure_ascii=False)


def _mundo(tmp):
    a = os.path.join(tmp, "assets")
    _escribe(a + "/objects/o.json", {"objects": [
        {"id": "barra"}, {"id": "ponton"}, {"id": "cana"}, {"id": "daga"}, {"id": "huerto_futuro_no"}]})
    _escribe(a + "/levels/n.json", {"objects": [
        {"objectId": "barra", "position": {"x": 1, "y": 2}},
        {"objectId": "ponton", "position": {"x": 5, "y": 6}},
        {"objectId": "daga", "position": {"x": 0, "y": 0}}]})
    _escribe(a + "/catalogs/skills.json", {"entries": [
        {"id": "persuasion", "castingStat": "CAR", "tags": ["narrativa"]},
        {"id": "percepcion", "castingStat": "DES", "tags": ["narrativa"]},
        {"id": "bola_de_fuego", "castingStat": "INT", "tags": ["fuego"]}]})
    _escribe(a + "/catalogs/mounts.json", {"entries": [{"id": "caballo"}]})
    _escribe(a + "/rules/facciones.json", {"facciones": [{"id": "aegroum", "tiendas": ["skilla"]}]})


def _oficio(**kw):
    o = {"nombre": "Servir", "verbo": "Servir", "objetos": ["barra"],
         "tirada": {"caracteristica": "CAR", "competencia": "persuasion", "rival": 2},
         "duracion": 1, "descanso": 120, "coste": {"oro": 0},
         "paga": {"roce": {"oro": 4, "exp": 1, "reputacion": 0}, "exito": {"oro": 8, "exp": 2, "reputacion": 1},
                  "critico": {"oro": 12, "exp": 3, "reputacion": 2}, "pifia": {"oro": -2}},
         "faccion": "", "efecto": {"pv": 0, "fatiga": 0},
         "objeto": {"id": "", "cantidad": 0, "calidadPorGrado": False},
         "textos": {"aceptar": "a", "exito": "b", "fallo": "c", "critico": "d", "pifia": "e"},
         "_tanda": 1}
    o.update(kw)
    return o


def _tablas():
    def fr(c, r):
        return {"comunes": c, "raros": r}
    return {"version": 1, "pesca": {"franjas": ["manana", "tarde", "noche"],
            "peces": {"c": {"nombre": "C", "precio": 8, "venta": 2, "rareza": "comun", "gradoMinimo": "roce"},
                      "r": {"nombre": "R", "precio": 40, "venta": 10, "rareza": "raro", "gradoMinimo": "exito"},
                      "l": {"nombre": "L", "precio": 200, "venta": 50, "rareza": "legendario", "gradoMinimo": "critico"}},
            "sitios": {"m": {"nombre": "M", "donde": [{"nivel": "assets/levels/n.json", "objectId": "ponton", "x": 5, "y": 6}],
                             "franjas": {"manana": fr(["c"], ["r"]), "tarde": fr(["c"], ["r"]), "noche": fr(["c"], ["r"])},
                             "legendario": {"pez": "l", "franjas": ["noche"]}}}}}


class Base(unittest.TestCase):
    def setUp(self):
        self.td = tempfile.TemporaryDirectory()
        _mundo(self.td.name)
        self.datos = V.Datos(self.td.name)
        self.prop = {"objects": [{"id": k, "name": v[0], "price": v[1]} for k, v in
                                  {"c": ("C", 8), "r": ("R", 40), "l": ("L", 200)}.items()]}

    def tearDown(self):
        self.td.cleanup()

    def val(self, oficios, tablas=None):
        cat = {"version": 1, "oficios": oficios}
        if tablas is None and "pescar" in oficios:
            tablas = _tablas()
        return V.validar_oficios(cat, tablas, self.datos, self.prop)

    def pescar(self, **kw):
        base = _oficio(objetos=["ponton"], tirada={"caracteristica": "DES", "competencia": "percepcion", "rival": 2},
                       objeto={"id": "", "cantidad": 1, "calidadPorGrado": True, "tabla": "pesca"}, herramienta="cana")
        base.update(kw)
        return base


class Contrato(Base):
    def test_todo_correcto(self):
        E, W = self.val({"servir": _oficio(), "pescar": self.pescar()})
        self.assertEqual(E, [], E)

    def test_falta_un_campo(self):
        o = _oficio()
        del o["descanso"]
        E, _ = self.val({"servir": o})
        self.assertTrue(any("falta 'descanso'" in e for e in E))

    def test_objectid_inexistente(self):
        E, _ = self.val({"servir": _oficio(objetos=["no_hay"])})
        self.assertTrue(any("no existe" in e for e in E))

    def test_objeto_no_colocado_en_nivel(self):
        E, _ = self.val({"servir": _oficio(objetos=["cana"])})
        self.assertTrue(any("no está puesto en ningún nivel" in e for e in E))

    def test_competencia_inexistente(self):
        E, _ = self.val({"servir": _oficio(tirada={"caracteristica": "CAR", "competencia": "bola_de_fuego", "rival": 2})})
        self.assertTrue(any("no existe en skills.json" in e for e in E))

    def test_competencia_con_otra_caracteristica(self):
        E, _ = self.val({"servir": _oficio(tirada={"caracteristica": "INT", "competencia": "persuasion", "rival": 2})})
        self.assertTrue(any("se tira con CAR" in e for e in E))

    def test_sin_tirada_valido_y_con_restos_no(self):
        E, _ = self.val({"s": _oficio(tirada={"caracteristica": "", "competencia": "", "rival": 0})})
        self.assertEqual(E, [], E)
        E, _ = self.val({"s": _oficio(tirada={"caracteristica": "CAR", "competencia": "", "rival": 0})})
        self.assertTrue(E)

    def test_faccion_inexistente(self):
        E, _ = self.val({"servir": _oficio(faccion="los_nadie")})
        self.assertTrue(any("facción" in e for e in E))
        E, _ = self.val({"servir": _oficio(faccion="aegroum")})
        self.assertEqual(E, [])

    def test_texto_largo(self):
        o = _oficio()
        o["textos"]["exito"] = "x" * 91
        E, _ = self.val({"servir": o})
        self.assertTrue(any("91 caracteres" in e for e in E))
        o["textos"]["exito"] = "x" * 90
        E, _ = self.val({"servir": o})
        self.assertEqual(E, [])

    def test_texto_que_falta(self):
        o = _oficio()
        del o["textos"]["pifia"]
        E, _ = self.val({"servir": o})
        self.assertTrue(any("'pifia'" in e for e in E))

    def test_paga_no_sube_con_el_grado(self):
        o = _oficio()
        o["paga"]["exito"]["oro"] = 2
        E, _ = self.val({"servir": o})
        self.assertTrue(any("menor que el grado anterior" in e for e in E))

    def test_paga_fuera_de_tope_y_pifia_positiva(self):
        o = _oficio()
        o["paga"]["critico"]["oro"] = 999
        o["paga"]["pifia"] = {"oro": 3}
        E, _ = self.val({"servir": o})
        self.assertTrue(any("paga.critico.oro" in e for e in E))
        self.assertTrue(any("paga.pifia.oro" in e for e in E))

    def test_duracion_y_descanso(self):
        E, _ = self.val({"servir": _oficio(duracion=0, descanso=-1)})
        self.assertEqual(len([e for e in E if "duracion" in e or "descanso" in e]), 2)

    def test_objeto_inexistente_y_tabla_inexistente(self):
        E, _ = self.val({"s": _oficio(objeto={"id": "espada_fantasma", "cantidad": 1, "calidadPorGrado": False})})
        self.assertTrue(any("espada_fantasma" in e for e in E))
        E, _ = self.val({"s": _oficio(objeto={"id": "", "cantidad": 1, "calidadPorGrado": False, "tabla": "caza"})})
        self.assertTrue(any("'caza'" in e for e in E))

    def test_tendero_y_herramienta_y_montura(self):
        E, _ = self.val({"s": _oficio(objeto={"id": "daga", "cantidad": 1, "calidadPorGrado": True, "tendero": "skilla"})})
        self.assertEqual(E, [], E)
        E, _ = self.val({"s": _oficio(objeto={"id": "daga", "cantidad": 1, "calidadPorGrado": True, "tendero": "nadie"})})
        self.assertTrue(any("tendero" in e for e in E))
        E, _ = self.val({"s": _oficio(herramienta="martillo")})
        self.assertTrue(any("herramienta" in e for e in E))
        E, _ = self.val({"s": _oficio(montura="dragon")})
        self.assertTrue(any("montura" in e for e in E))

    def test_sitio_pendiente(self):
        E, W = self.val({"s": _oficio(objetos=[], _tanda=4, _objetosPendientes=["bancal"])})
        self.assertEqual(E, [], E)
        self.assertTrue(any("sin sitio" in w for w in W))
        E, _ = self.val({"s": _oficio(objetos=[], _tanda=1, _objetosPendientes=["bancal"])})
        self.assertTrue(any("tanda 1" in e for e in E))
        E, _ = self.val({"s": _oficio(objetos=[])})
        self.assertTrue(E)

    def test_tanda_fuera_de_rango(self):
        E, _ = self.val({"s": _oficio(_tanda=9)})
        self.assertTrue(any("_tanda" in e for e in E))


class Rangos(Base):
    def val_r(self, rangos):
        cat = {"version": 1, "rangos": rangos, "oficios": {"s": _oficio()}}
        return V.validar_oficios(cat, None, self.datos, self.prop)[0]

    def test_rangos_buenos(self):
        r = [{"id": "a", "nombre": "A", "expDesde": 0, "dados": 0}, {"id": "b", "nombre": "B", "expDesde": 10, "dados": 1}]
        self.assertEqual(self.val_r(r), [])

    def test_rangos_malos(self):
        self.assertTrue(self.val_r([{"id": "a", "nombre": "A", "expDesde": 5, "dados": 0}]))
        self.assertTrue(self.val_r([{"id": "a", "nombre": "A", "expDesde": 0, "dados": 0},
                                    {"id": "b", "nombre": "B", "expDesde": 0, "dados": 1}]))
        self.assertTrue(self.val_r([{"id": "a", "nombre": "A", "expDesde": 0, "dados": 0},
                                    {"id": "b", "nombre": "B", "expDesde": 9, "dados": 9}]))
        self.assertTrue(self.val_r([]))


class Pesca(Base):
    def val_t(self, t):
        return V.validar_oficios({"version": 1, "oficios": {"pescar": self.pescar()}}, t, self.datos, self.prop)

    def test_tabla_buena(self):
        self.assertEqual(self.val_t(_tablas())[0], [])

    def test_grado_minimo_incoherente(self):
        t = _tablas()
        t["pesca"]["peces"]["r"]["gradoMinimo"] = "roce"
        self.assertTrue(any("gradoMinimo" in e for e in self.val_t(t)[0]))

    def test_venta_no_es_el_25(self):
        t = _tablas()
        t["pesca"]["peces"]["c"]["venta"] = 7
        self.assertTrue(any("25 %" in e for e in self.val_t(t)[0]))

    def test_pez_que_no_esta(self):
        t = _tablas()
        t["pesca"]["sitios"]["m"]["franjas"]["tarde"]["comunes"] = ["fantasma"]
        self.assertTrue(any("fantasma" in e for e in self.val_t(t)[0]))

    def test_pez_en_lista_equivocada(self):
        t = _tablas()
        t["pesca"]["sitios"]["m"]["franjas"]["tarde"]["raros"] = ["c"]
        self.assertTrue(any("no es un pez raro" in e for e in self.val_t(t)[0]))

    def test_franja_sin_raros(self):
        t = _tablas()
        t["pesca"]["sitios"]["m"]["franjas"]["noche"]["raros"] = []
        self.assertTrue(any("sin peces raros" in e for e in self.val_t(t)[0]))

    def test_legendario_no_legendario(self):
        t = _tablas()
        t["pesca"]["sitios"]["m"]["legendario"]["pez"] = "r"
        self.assertTrue(any("no es legendario" in e for e in self.val_t(t)[0]))

    def test_sitio_donde_no_hay_ponton(self):
        t = _tablas()
        t["pesca"]["sitios"]["m"]["donde"][0]["x"] = 99
        self.assertTrue(any("no está en" in e for e in self.val_t(t)[0]))

    def test_ficha_propuesta_que_falta_o_discrepa(self):
        self.prop["objects"] = [o for o in self.prop["objects"] if o["id"] != "l"]
        self.assertTrue(any("ni existe como objeto" in e for e in self.val_t(_tablas())[0]))
        self.prop["objects"].append({"id": "l", "name": "L", "price": 1})
        self.assertTrue(any("precio" in e for e in self.val_t(_tablas())[0]))


class DatosReales(unittest.TestCase):
    """Sobre los ficheros del motor, si están (en el repo, sí)."""

    def test_el_catalogo_de_verdad(self):
        if not os.path.exists(os.path.join(RAIZ, "assets", "catalogs", "oficios.json")) \
                or not os.path.isdir(os.path.join(RAIZ, "assets", "objects")):
            self.skipTest("no hay catálogo de oficios o assets del motor")
        prop = os.path.join(RAIZ, "out", "claude_oficios_datos01", "propuesta", "objetos_peces.json")
        E, _ = V.cargar_y_validar(RAIZ, prop if os.path.exists(prop) else None)
        self.assertEqual(E, [], "\n".join(E))
        with open(os.path.join(RAIZ, "assets", "catalogs", "oficios.json"), encoding="utf-8") as f:
            cat = json.load(f)["oficios"]
        for oid in ("servir_bar", "estudiar_biblioteca", "pescar"):
            self.assertEqual(cat[oid]["_tanda"], 1, oid)
        self.assertEqual(sorted(o["_tanda"] for o in cat.values())[0], 1)


if __name__ == "__main__":
    unittest.main(verbosity=1)
