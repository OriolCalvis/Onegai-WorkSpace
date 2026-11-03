#!/usr/bin/env python3
"""Pruebas de validar_beats_auto.py (UNI-02-11, B1) con aventuras de mentira.

    python3 tools/test_validar_beats_auto.py

Cada caso escribe una aventura minima en un directorio temporal y comprueba si el beat auto
se marca como pegajoso. Al final hay una prueba de humo sobre el repo real: ninguna
aventura de assets/adventures tiene un auto pegajoso.
"""
import json
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import validar_beats_auto as vba  # noqa: E402


def auto(requires=None, effects=None, **extra):
    """Un beat auto 'b' con los requisitos y efectos dados."""
    beat = {"id": "b", "trigger": {"type": "auto"}, "requires": requires or {}, "effects": effects or []}
    beat.update(extra)
    return beat


def pegajosos(*beats, nombre="mentira.json"):
    """Ids de los beats que el validador marca como pegajosos en una aventura con esos beats."""
    with tempfile.TemporaryDirectory() as tmp:
        carpeta = Path(tmp) / "assets" / "adventures"
        carpeta.mkdir(parents=True)
        (carpeta / nombre).write_text(json.dumps({"beats": list(beats)}, ensure_ascii=False), encoding="utf-8")
        problemas, _ = vba.comprueba(tmp)
        return [bid for _, bid, _, _ in problemas]


def set_flag(f):
    return {"type": "setFlag", "arg": f}


class BeatsAutoTests(unittest.TestCase):
    def test_auto_sin_once_con_solo_log_es_pegajoso(self):
        malos = pegajosos(auto({"allFlags": ["a"]}, [{"type": "log", "arg": "texto"}]))
        self.assertEqual(malos, ["b"])

    def test_auto_sin_efectos_ni_requisitos_es_pegajoso(self):
        self.assertEqual(pegajosos(auto()), ["b"])

    def test_con_once_no_se_mira(self):
        self.assertEqual(pegajosos(auto({"allFlags": ["a"]}, [{"type": "log", "arg": "t"}], once=True)), [])

    def test_notflags_que_pone_su_propio_setflag_no_es_pegajoso(self):
        beat = auto({"allFlags": ["a"], "notFlags": ["visto"]}, [set_flag("visto")])
        self.assertEqual(pegajosos(beat), [])

    def test_notflags_que_pone_otro_beat_si_es_pegajoso(self):
        # La guarda tiene que ponerla el propio beat: que la ponga otro, que quiza no llega, no vale.
        beat = auto({"allFlags": ["a"], "notFlags": ["visto"]}, [{"type": "log", "arg": "t"}])
        otro = dict(auto(effects=[set_flag("visto")]), id="otro", once=True)
        self.assertEqual(pegajosos(beat, otro), ["b"])

    def test_clearflag_de_una_de_sus_allflags_no_es_pegajoso(self):
        beat = auto({"allFlags": ["a", "b"]}, [{"type": "clearFlag", "arg": "b"}])
        self.assertEqual(pegajosos(beat), [])

    def test_setvariable_que_rompe_su_igualdad_no_es_pegajoso(self):
        beat = auto({"variables": {"fase": 1}}, [{"type": "setVariable", "arg": "fase", "value": 2}])
        self.assertEqual(pegajosos(beat), [])

    def test_setvariable_que_mantiene_su_igualdad_es_pegajoso(self):
        beat = auto({"variables": {"fase": 1}}, [{"type": "setVariable", "arg": "fase", "value": 1}])
        self.assertEqual(pegajosos(beat), ["b"])

    def test_addvariable_que_sale_del_rango_no_es_pegajoso(self):
        beat = auto({"variables": {"n": {"min": 0, "max": 3}}}, [{"type": "addVariable", "arg": "n", "value": 1}])
        self.assertEqual(pegajosos(beat), [])

    def test_azar_con_una_rama_que_sigue_cumpliendo_es_pegajoso(self):
        # skillCheck: si UNA rama deja la flag que pide, ese camino se repite sin fin.
        beat = auto({"allFlags": ["ok"]}, [{"type": "skillCheck",
                                            "skillCheck": {"flagSuccess": "ok", "flagBotch": "mal"}}])
        self.assertEqual(pegajosos(beat), ["b"])

    def test_opcion_que_apaga_el_beat_lo_salva(self):
        # La eleccion es del jugador: si una opcion siempre apaga el beat, no es un bucle del motor.
        beat = auto({"allFlags": ["a"]}, [], opciones=[{"id": "o", "effects": [{"type": "clearFlag", "arg": "a"}]}])
        self.assertEqual(pegajosos(beat), [])

    def test_opcion_que_no_lo_apaga_no_lo_salva(self):
        beat = auto({"allFlags": ["a"]}, [], opciones=[{"id": "o", "effects": [{"type": "log", "arg": "t"}]}])
        self.assertEqual(pegajosos(beat), ["b"])

    def test_trigger_talk_no_se_mira(self):
        beat = auto({"allFlags": ["a"]}, [{"type": "log", "arg": "t"}])
        beat["trigger"] = {"type": "talk", "target": "pnj"}
        self.assertEqual(pegajosos(beat), [])

    def test_excepcion_explicita_se_acepta(self):
        beat = auto({"allFlags": ["a"]}, [{"type": "log", "arg": "t"}])
        antes = set(vba.EXCEPCIONES)
        vba.EXCEPCIONES.add("mentira.json:b")
        try:
            self.assertEqual(pegajosos(beat), [])
        finally:
            vba.EXCEPCIONES.clear()
            vba.EXCEPCIONES.update(antes)

    def test_excepciones_vacias_por_defecto(self):
        self.assertEqual(vba.EXCEPCIONES, set())

    def test_main_sale_con_1_si_hay_pegajoso_y_con_0_si_no(self):
        with tempfile.TemporaryDirectory() as tmp:
            carpeta = Path(tmp) / "assets" / "adventures"
            carpeta.mkdir(parents=True)
            ruta = carpeta / "m.json"
            ruta.write_text(json.dumps({"beats": [auto({"allFlags": ["a"]})]}), encoding="utf-8")
            self.assertEqual(vba.main(["--raiz", tmp]), 1)
            ruta.write_text(json.dumps({"beats": [auto({"allFlags": ["a"]}, once=True)]}), encoding="utf-8")
            self.assertEqual(vba.main(["--raiz", tmp]), 0)


@unittest.skipUnless((HERE.parent / "assets" / "adventures").is_dir(), "no hay assets/adventures")
class RepoTests(unittest.TestCase):
    def test_ninguna_aventura_real_tiene_un_auto_pegajoso(self):
        problemas, revisados = vba.comprueba(str(HERE.parent))
        self.assertGreater(revisados, 0)
        self.assertEqual([f"{f}:{b}" for f, b, _, _ in problemas], [])


if __name__ == "__main__":
    unittest.main()
