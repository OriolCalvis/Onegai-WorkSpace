#!/usr/bin/env python3
"""Pruebas de validar_rutinas_pnj.py con catalogos de mentira.

    python3 tools/test_validar_rutinas_pnj.py

Cada regla del contrato tiene su caso que la hace fallar. Todo se escribe en un directorio
temporal: no depende de los niveles de verdad. Al final hay una prueba de humo sobre el repo,
que solo corre si existen assets/catalogs/rutinas_pnj.json y assets/levels.
"""
import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import validar_rutinas_pnj as vr  # noqa: E402

ICONOS = ["oro", "tienda", "lore", "curiosidad"]

OBJETOS = [
    {"id": "mercader", "category": "npc"},
    {"id": "guardia", "category": "npc"},
    {"id": "nino", "category": "npc"},
    {"id": "tendero", "category": "npc", "shop": {"items": ["pan"]}},
    {"id": "npc_race_a", "category": "npc"},
    {"id": "npc_race_b", "category": "npc"},
    {"id": "suelto", "category": "npc"},
    {"id": "barril", "category": "prop"},
]

BUENO = {
    "version": 1,
    "porDefecto": {"rutina": "deambular", "radio": 2, "pausa": [3, 8], "mira": "jugador"},
    "rutinas": {
        "mercader": {"rutina": "actividad", "radio": 1, "pausa": [4, 9], "mira": "calle",
                     "actividad": {"nombre": "pregonar", "movimiento": "vaiven", "icono": "oro", "cada": [6, 12]},
                     "frases": ["¡Fruta fresca!", "¡Mirad, mirad!"]},
        "guardia": {"rutina": "vigilar", "radio": 0, "pausa": [5, 10], "mira": "giro"},
        "nino": {"rutina": "deambular", "radio": 4, "pausa": [1, 3]},
        "tendero": {"rutina": "actividad", "radio": 0,
                    "actividad": {"nombre": "ordenar", "movimiento": "vaiven", "icono": "", "cada": [8, 14]}},
        "npc_race_*": {"rutina": "deambular", "radio": 3, "pausa": [3, 9]},
        "npc_race_b": {"rutina": "quieto", "radio": 0, "mira": "fijo:abajo"},
        "suelto": {"rutina": "ir_y_volver", "radio": 0, "destino": [3, 0], "pausa": [5, 9]},
    },
}


def nivel(*ids):
    return {"name": "n", "map": "x.tmx", "playerStart": {"x": 1, "y": 1},
            "objects": [{"objectId": i, "position": {"x": k, "y": 1}} for k, i in enumerate(ids)]}


class Base(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.raiz = Path(self._tmp.name)
        self.addCleanup(self._tmp.cleanup)
        for sub in ("catalogs", "objects", "levels", "adventures", "textures"):
            (self.raiz / "assets" / sub).mkdir(parents=True)
        self.escribe("assets/textures/iconos.json", {"iconos": ICONOS})
        self.escribe("assets/objects/o.json", {"objects": OBJETOS})
        self.escribe("assets/levels/ciudad.json", nivel("mercader", "guardia", "nino", "npc_race_a", "npc_race_b",
                                                        "suelto", "barril"))
        self.escribe("assets/levels/interior_tienda.json", nivel("tendero"))

    def escribe(self, rel, datos):
        p = self.raiz / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(datos if isinstance(datos, str) else json.dumps(datos, ensure_ascii=False), encoding="utf-8")

    def corre(self, cat=None):
        if cat is not None:
            self.escribe("assets/catalogs/rutinas_pnj.json", cat)
        return vr.validar(self.raiz)

    def con(self, clave, **cambios):
        """Catalogo bueno con la rutina `clave` cambiada (valor None = borrar el campo)."""
        cat = copy.deepcopy(BUENO)
        for k, val in cambios.items():
            if val is None:
                cat["rutinas"][clave].pop(k, None)
            else:
                cat["rutinas"][clave][k] = val
        return cat

    def assertError(self, inf, *trozos):
        hay = [e for e in inf.errores if all(t in e for t in trozos)]
        self.assertTrue(hay, f"esperaba un ERROR con {trozos}; hay: {inf.errores}")

    def assertAviso(self, inf, *trozos):
        hay = [a for a in inf.avisos if all(t in a for t in trozos)]
        self.assertTrue(hay, f"esperaba un AVISO con {trozos}; hay: {inf.avisos}")


class Contrato(Base):
    def test_catalogo_bueno_no_da_ni_errores_ni_avisos(self):
        inf, resumen = self.corre(BUENO)
        self.assertEqual(inf.errores, [])
        self.assertEqual(inf.avisos, [])
        self.assertEqual(sum(sum(d.values()) for _, _, d in resumen.values()), 7)

    def test_nota_en_la_raiz_vale(self):
        cat = copy.deepcopy(BUENO)
        cat["_nota"] = "texto"
        self.assertEqual(self.corre(cat)[0].errores, [])

    def test_sin_fichero(self):
        inf, _ = vr.validar(self.raiz)
        self.assertError(inf, "no se pudo abrir")

    def test_json_mal_formado(self):
        inf, _ = self.corre("{ esto no es json")
        self.assertError(inf, "JSON mal formado")

    def test_version_distinta(self):
        cat = copy.deepcopy(BUENO)
        cat["version"] = 2
        self.assertError(self.corre(cat)[0], "version")

    def test_falta_por_defecto(self):
        cat = copy.deepcopy(BUENO)
        del cat["porDefecto"]
        self.assertError(self.corre(cat)[0], "porDefecto")

    def test_clave_de_raiz_desconocida(self):
        cat = copy.deepcopy(BUENO)
        cat["rutines"] = {}
        self.assertError(self.corre(cat)[0], "rutines", "clave desconocida")

    def test_rutina_desconocida(self):
        self.assertError(self.corre(self.con("guardia", rutina="patrullar"))[0], "guardia", "rutina")

    def test_clave_desconocida_en_la_rutina(self):
        self.assertError(self.corre(self.con("guardia", velocidad=2))[0], "guardia", "velocidad")

    def test_radio_fuera_de_rango_o_de_tipo(self):
        for malo in (-1, 7, 1.5, "2", True):
            with self.subTest(radio=malo):
                self.assertError(self.corre(self.con("nino", radio=malo))[0], "nino", "radio")

    def test_pausa_mal(self):
        for malo in ([3], [3, 8, 9], [9, 3], [0, 3], [3, 99], "3", [3, "8"]):
            with self.subTest(pausa=malo):
                self.assertError(self.corre(self.con("nino", pausa=malo))[0], "nino", "pausa")

    def test_mira_desconocida(self):
        self.assertError(self.corre(self.con("guardia", mira="techo"))[0], "guardia", "mira")
        self.assertError(self.corre(self.con("guardia", mira="fijo:diagonal"))[0], "guardia", "mira")

    def test_miras_validas(self):
        for m in vr.MIRAS:
            with self.subTest(mira=m):
                self.assertEqual(self.corre(self.con("guardia", mira=m))[0].errores, [])

    def test_actividad_necesaria_con_rutina_actividad(self):
        self.assertError(self.corre(self.con("mercader", actividad=None))[0], "mercader", "actividad")

    def test_movimiento_desconocido(self):
        a = dict(BUENO["rutinas"]["mercader"]["actividad"], movimiento="barrer")
        self.assertError(self.corre(self.con("mercader", actividad=a))[0], "movimiento")

    def test_movimientos_validos(self):
        for m in vr.MOVIMIENTOS:
            with self.subTest(movimiento=m):
                a = dict(BUENO["rutinas"]["mercader"]["actividad"], movimiento=m)
                self.assertEqual(self.corre(self.con("mercader", actividad=a))[0].errores, [])

    def test_icono_que_no_existe(self):
        a = dict(BUENO["rutinas"]["mercader"]["actividad"], icono="martillo")
        self.assertError(self.corre(self.con("mercader", actividad=a))[0], "icono", "martillo")

    def test_icono_vacio_vale(self):
        a = dict(BUENO["rutinas"]["mercader"]["actividad"], icono="")
        self.assertEqual(self.corre(self.con("mercader", actividad=a))[0].errores, [])

    def test_cada_mal_o_ausente(self):
        a = dict(BUENO["rutinas"]["mercader"]["actividad"], cada=[12, 6])
        self.assertError(self.corre(self.con("mercader", actividad=a))[0], "cada")
        a = dict(BUENO["rutinas"]["mercader"]["actividad"])
        del a["cada"]
        self.assertError(self.corre(self.con("mercader", actividad=a))[0], "cada")

    def test_clave_desconocida_en_la_actividad(self):
        a = dict(BUENO["rutinas"]["mercader"]["actividad"], duracion=3)
        self.assertError(self.corre(self.con("mercader", actividad=a))[0], "actividad.duracion")

    def test_nombre_de_actividad_vacio_o_largo(self):
        for n in ("", "x" * 40):
            with self.subTest(nombre=n):
                a = dict(BUENO["rutinas"]["mercader"]["actividad"], nombre=n)
                self.assertError(self.corre(self.con("mercader", actividad=a))[0], "nombre")

    def test_actividad_en_otra_rutina_avisa(self):
        a = BUENO["rutinas"]["mercader"]["actividad"]
        self.assertAviso(self.corre(self.con("guardia", actividad=a))[0], "guardia", "se ignora")

    def test_ir_y_volver_pide_destino(self):
        self.assertError(self.corre(self.con("suelto", destino=None))[0], "suelto", "destino")
        for malo in ([0, 0], [9, 0], [1], [1.5, 0], "3,0"):
            with self.subTest(destino=malo):
                self.assertError(self.corre(self.con("suelto", destino=malo))[0], "suelto", "destino")

    def test_destino_en_otra_rutina_avisa(self):
        self.assertAviso(self.corre(self.con("guardia", destino=[1, 0]))[0], "guardia", "destino")

    def test_frases_demasiado_largas_o_muchas(self):
        self.assertError(self.corre(self.con("mercader", frases=["x" * 41]))[0], "frases[0]", "41")
        self.assertError(self.corre(self.con("mercader", frases=["a", "b", "c", "d"]))[0], "frases", "maximo 3")
        self.assertError(self.corre(self.con("mercader", frases=[""]))[0], "frases[0]")
        self.assertError(self.corre(self.con("mercader", frases="hola"))[0], "frases")

    def test_frase_de_cuarenta_caracteres_vale(self):
        self.assertEqual(self.corre(self.con("mercader", frases=["ñ" * 40]))[0].errores, [])

    def test_radio_en_quieto_avisa(self):
        self.assertAviso(self.corre(self.con("npc_race_b", radio=2))[0], "npc_race_b", "se ignora")

    def test_claves_peligrosas(self):
        cat = copy.deepcopy(BUENO)
        cat["rutinas"]["*"] = {"rutina": "quieto"}
        cat["rutinas"]["npc_*_x"] = {"rutina": "quieto"}
        inf, _ = self.corre(cat)
        self.assertError(inf, "casaria con todos")
        self.assertError(inf, "npc_*_x", "al final")


class Referencias(Base):
    def test_clave_exacta_que_no_existe(self):
        cat = copy.deepcopy(BUENO)
        cat["rutinas"]["mercadre"] = {"rutina": "quieto"}
        self.assertError(self.corre(cat)[0], "mercadre", "errata")

    def test_clave_exacta_que_no_es_npc(self):
        cat = copy.deepcopy(BUENO)
        cat["rutinas"]["barril"] = {"rutina": "quieto"}
        self.assertError(self.corre(cat)[0], "barril", "no un npc")

    def test_prefijo_sin_objetos_avisa(self):
        cat = copy.deepcopy(BUENO)
        cat["rutinas"]["zzz_*"] = {"rutina": "quieto"}
        self.assertAviso(self.corre(cat)[0], "zzz_*", "ningun objeto")


class Resolucion(unittest.TestCase):
    R = {"npc_race_*": {}, "npc_*": {}, "npc_race_b": {}, "guardia": {}}

    def test_exacta_gana(self):
        self.assertEqual(vr.resolver("npc_race_b", self.R), ("exacta", "npc_race_b"))

    def test_gana_el_prefijo_mas_largo(self):
        self.assertEqual(vr.resolver("npc_race_a", self.R), ("prefijo", "npc_race_*"))
        self.assertEqual(vr.resolver("npc_otro", self.R), ("prefijo", "npc_*"))

    def test_sin_clave_va_a_por_defecto(self):
        self.assertEqual(vr.resolver("sastre", self.R), ("defecto", None))

    def test_el_prefijo_no_es_una_subcadena(self):
        self.assertEqual(vr.resolver("el_guardia", {"guardia*": {}}), ("defecto", None))


class AvisosDeNiveles(Base):
    def test_pnj_en_por_defecto_avisa_con_cuantos_y_donde(self):
        cat = copy.deepcopy(BUENO)
        del cat["rutinas"]["nino"]
        self.escribe("assets/levels/otro.json", nivel("nino", "nino"))
        inf, resumen = self.corre(cat)
        self.assertAviso(inf, "nino", "3 PNJ caen en porDefecto", "otro")
        self.assertEqual(resumen["nino"][0], "defecto")

    def test_por_prefijo_no_es_por_defecto(self):
        inf, resumen = self.corre(BUENO)
        self.assertEqual(resumen["npc_race_a"][:2], ("prefijo", "npc_race_*"))
        self.assertFalse([a for a in inf.avisos if "porDefecto" in a])

    def test_un_objeto_que_no_es_npc_no_cuenta(self):
        inf, resumen = self.corre(BUENO)
        self.assertNotIn("barril", resumen)

    def test_tendero_de_interior_que_se_pasea_avisa(self):
        for cambios in ({"rutina": "deambular", "actividad": None},
                        {"radio": 2}):
            with self.subTest(cambios=cambios):
                inf, _ = self.corre(self.con("tendero", **cambios))
                self.assertAviso(inf, "tendero", "mostrador")

    def test_tendero_con_radio_cero_no_avisa(self):
        self.assertEqual(self.corre(BUENO)[0].avisos, [])

    def test_pnj_de_historia_que_se_pasea_lejos_avisa(self):
        self.escribe("assets/adventures/a.json", {"beats": [{"id": "b", "trigger": {"type": "talk", "target": "nino"}}]})
        inf, _ = self.corre(BUENO)       # nino: deambular radio 4
        self.assertAviso(inf, "nino", "dialogo de historia")

    def test_pnj_de_historia_quieto_o_con_radio_1_no_avisa(self):
        self.escribe("assets/adventures/a.json", {"beats": [
            {"trigger": {"type": "talk", "target": "guardia"}},
            {"trigger": {"type": "talk", "target": "mercader"}}]})   # vigilar r0; actividad r1
        self.assertEqual(self.corre(BUENO)[0].avisos, [])

    def test_pnj_de_historia_que_charla_avisa(self):
        self.escribe("assets/adventures/a.json", {"beats": [{"trigger": {"type": "talk", "target": "nino"}}]})
        inf, _ = self.corre(self.con("nino", rutina="charlar", radio=1))
        self.assertAviso(inf, "nino", "charlar")

    def test_un_trigger_que_no_es_talk_no_cuenta(self):
        self.escribe("assets/adventures/a.json", {"beats": [{"trigger": {"type": "enter", "target": "nino"}}]})
        self.assertEqual(self.corre(BUENO)[0].avisos, [])


class Salida(Base):
    def run_main(self, *args):
        import contextlib
        import io
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            codigo = vr.main(["--raiz", str(self.raiz), *args])
        return codigo, buf.getvalue()

    def test_exit_0_sin_errores(self):
        self.escribe("assets/catalogs/rutinas_pnj.json", BUENO)
        codigo, texto = self.run_main()
        self.assertEqual(codigo, 0)
        self.assertIn("0 errores, 0 avisos", texto)

    def test_exit_1_con_errores(self):
        self.escribe("assets/catalogs/rutinas_pnj.json", self.con("guardia", rutina="x"))
        codigo, texto = self.run_main()
        self.assertEqual(codigo, 1)
        self.assertIn("ERROR", texto)

    def test_estricto_rompe_con_avisos(self):
        cat = copy.deepcopy(BUENO)
        del cat["rutinas"]["nino"]
        self.escribe("assets/catalogs/rutinas_pnj.json", cat)
        self.assertEqual(self.run_main()[0], 0)
        self.assertEqual(self.run_main("--estricto")[0], 1)

    def test_resumen_lista_los_tipos(self):
        self.escribe("assets/catalogs/rutinas_pnj.json", BUENO)
        _, texto = self.run_main("--resumen")
        self.assertIn("npc_race_a", texto)
        self.assertIn("prefijo", texto)


class PruebaDeHumo(unittest.TestCase):
    def test_el_catalogo_del_repo_no_tiene_errores(self):
        raiz = HERE.parent
        if not (raiz / vr.CATALOGO).exists() or not (raiz / "assets" / "levels").exists():
            self.skipTest("no hay catalogo o niveles en este repo")
        inf, resumen = vr.validar(raiz)
        self.assertEqual(inf.errores, [])
        self.assertTrue(resumen, "no se encontro ningun PNJ en los niveles")


if __name__ == "__main__":
    unittest.main(verbosity=1)
