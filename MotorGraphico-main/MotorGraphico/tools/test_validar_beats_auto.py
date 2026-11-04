#!/usr/bin/env python3
"""Pruebas de validar_beats_auto.py (UNI-02-11, B1) con aventuras de mentira.

    python3 tools/test_validar_beats_auto.py

Cada caso escribe una aventura minima en un directorio temporal y comprueba si el beat auto
se marca como pegajoso (B1) o si un beat talk/enter queda tapado por otro anterior (B2). Al
final hay una prueba de humo sobre el repo real: ninguna aventura de assets/adventures tiene un
auto pegajoso ni un beat tapado.
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


def talk(bid, target="pnj", requires=None, effects=None, **extra):
    """Un beat talk con id `bid`."""
    beat = {"id": bid, "trigger": {"type": "talk", "target": target}, "requires": requires or {},
            "effects": effects or []}
    beat.update(extra)
    return beat


def tapados(*beats, nombre="mentira.json"):
    """[(tapado, tapador)] segun el validador, en una aventura con esos beats."""
    with tempfile.TemporaryDirectory() as tmp:
        carpeta = Path(tmp) / "assets" / "adventures"
        carpeta.mkdir(parents=True)
        (carpeta / nombre).write_text(json.dumps({"beats": list(beats)}, ensure_ascii=False), encoding="utf-8")
        problemas, _ = vba.comprueba(tmp)
        return [(bid, motivo.split("'")[1]) for _, bid, _, motivo in problemas if "tapado" in motivo]


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


SECTARIOS_RESUELTOS = {"allFlags": ["resueltos"]}
SECTARIOS_SEGUIR = {"allFlags": ["resueltos"], "notFlags": ["visto"]}


class BeatsTapadosTests(unittest.TestCase):
    def test_patron_sectarios_tapa_al_siguiente(self):
        # A («Ya hemos hablado», sin efectos) va antes que B (persecucion, mismos requisitos + guardas).
        despues = talk("despues", "sectario", SECTARIOS_RESUELTOS)
        seguir = talk("seguir", "sectario", SECTARIOS_SEGUIR, [set_flag("visto")])
        self.assertEqual(tapados(despues, seguir), [("seguir", "despues")])

    def test_b_antes_que_a_no_esta_tapado(self):
        despues = talk("despues", "sectario", SECTARIOS_RESUELTOS)
        seguir = talk("seguir", "sectario", SECTARIOS_SEGUIR, [set_flag("visto")])
        self.assertEqual(tapados(seguir, despues), [])

    def test_a_con_once_no_tapa(self):
        despues = talk("despues", "sectario", SECTARIOS_RESUELTOS, once=True)
        seguir = talk("seguir", "sectario", SECTARIOS_SEGUIR, [set_flag("visto")])
        self.assertEqual(tapados(despues, seguir), [])

    def test_a_con_guarda_propia_que_b_no_tiene_no_tapa(self):
        # A se apaga con su propia flag; sin esa flag en B, B puede salir cuando A ya se apago.
        a = talk("a", requires={"allFlags": ["x"], "notFlags": ["a_visto"]}, effects=[set_flag("a_visto")])
        b = talk("b", requires={"allFlags": ["x"]})
        self.assertEqual(tapados(a, b), [])

    def test_otro_target_o_otro_tipo_no_tapa(self):
        a = talk("a", "uno", {"allFlags": ["x"]})
        b = talk("b", "dos", {"allFlags": ["x"], "notFlags": ["y"]})
        c = dict(talk("c", "uno", {"allFlags": ["x"], "notFlags": ["y"]}), trigger={"type": "enter", "target": "uno"})
        self.assertEqual(tapados(a, b, c), [])

    def test_b_con_menos_requisitos_que_a_no_esta_tapado(self):
        a = talk("a", requires={"allFlags": ["x", "y"]})
        b = talk("b", requires={"allFlags": ["x"]})
        self.assertEqual(tapados(a, b), [])

    def test_a_sin_requisitos_tapa_a_todo_lo_posterior(self):
        a = talk("a")
        b = talk("b", requires={"allFlags": ["x"]})
        self.assertEqual(tapados(a, b), [("b", "a")])

    def test_lugar_de_a_distinto_o_mas_estrecho_no_tapa(self):
        a = talk("a", "uno", {"allFlags": ["x"], "lugar": "taberna.json"})
        sin_lugar = talk("b", "dos", {"allFlags": ["x"]})
        otro = talk("c", "tres", {"allFlags": ["x"], "lugar": "calle.json"})
        mismo = talk("d", "cuatro", {"allFlags": ["x"], "lugar": "taberna.json"})
        for solo in (sin_lugar, otro):
            self.assertEqual(tapados(a, dict(solo, trigger=a["trigger"])), [])
        self.assertEqual(tapados(a, dict(mismo, trigger=a["trigger"])), [("d", "a")])

    def test_a_sin_lugar_tapa_aunque_b_tenga_lugar(self):
        a = talk("a", requires={"allFlags": ["x"]})
        b = talk("b", requires={"allFlags": ["x"], "lugar": "taberna.json"})
        self.assertEqual(tapados(a, b), [("b", "a")])

    def test_variables_y_anyflags(self):
        a = talk("a", requires={"variables": {"n": {"min": 0, "max": 5}}, "anyFlags": ["p", "q"]})
        dentro = talk("b", requires={"variables": {"n": 3}, "allFlags": ["p"]})
        fuera = talk("c", requires={"variables": {"n": 9}, "allFlags": ["p"]})
        sin_var = talk("d", requires={"allFlags": ["p"]})
        self.assertEqual(tapados(a, dentro, fuera, sin_var), [("b", "a")])

    def test_los_auto_no_cuentan_como_tapados(self):
        a = auto({"allFlags": ["x"]}, [set_flag("x_visto")], once=True)
        b = dict(auto({"allFlags": ["x"], "notFlags": ["z"]}, [set_flag("z")]), id="b")
        self.assertEqual(tapados(a, b), [])

    def test_excepcion_explicita_se_acepta(self):
        despues = talk("despues", "sectario", SECTARIOS_RESUELTOS)
        seguir = talk("seguir", "sectario", SECTARIOS_SEGUIR, [set_flag("visto")])
        antes = set(vba.EXCEPCIONES)
        vba.EXCEPCIONES.add("mentira.json:seguir")
        try:
            self.assertEqual(tapados(despues, seguir), [])
        finally:
            vba.EXCEPCIONES.clear()
            vba.EXCEPCIONES.update(antes)

    def test_main_sale_con_1_si_hay_tapado(self):
        with tempfile.TemporaryDirectory() as tmp:
            carpeta = Path(tmp) / "assets" / "adventures"
            carpeta.mkdir(parents=True)
            beats = [talk("despues", "s", SECTARIOS_RESUELTOS), talk("seguir", "s", SECTARIOS_SEGUIR)]
            (carpeta / "m.json").write_text(json.dumps({"beats": beats}), encoding="utf-8")
            self.assertEqual(vba.main(["--raiz", tmp]), 1)
            (carpeta / "m.json").write_text(json.dumps({"beats": beats[::-1]}), encoding="utf-8")
            self.assertEqual(vba.main(["--raiz", tmp]), 0)


@unittest.skipUnless((HERE.parent / "assets" / "adventures").is_dir(), "no hay assets/adventures")
class RepoTests(unittest.TestCase):
    def test_ninguna_aventura_real_tiene_un_auto_pegajoso(self):
        problemas, revisados = vba.comprueba(str(HERE.parent))
        self.assertGreater(revisados, 0)
        self.assertEqual([f"{f}:{b}" for f, b, _, _ in problemas], [])

    def test_los_sectarios_persiguen_antes_de_decir_ya_hemos_hablado(self):
        # Regresion UNI-02-11 B2: b1_sectarios_seguir iba DESPUES de b1_sectarios_despues y no salia nunca.
        ruta = HERE.parent / "assets" / "adventures" / "boundington_primer_dia.json"
        ids = [b["id"] for b in json.loads(ruta.read_text(encoding="utf-8"))["beats"]]
        self.assertLess(ids.index("b1_sectarios_seguir"), ids.index("b1_sectarios_despues"))

    def test_hablar_con_el_sectario_da_primero_seguir_y_luego_despues(self):
        # Simula el «primer beat que encaja» del motor sobre la aventura real: con la flag de
        # sectarios resueltos sale la persecucion UNA vez (ella misma pone seguir_visto) y despues
        # «Ya hemos hablado». La opcion «Dejarles ir» no toca flags, asi que el estado sigue igual.
        ruta = HERE.parent / "assets" / "adventures" / "boundington_primer_dia.json"
        beats = json.loads(ruta.read_text(encoding="utf-8"))["beats"]
        flags = frozenset({"bnd1_sectarios_vistos", "bnd1_sectarios_resueltos"})  # tras la charla (b1_secta_s3)
        vistos = []
        for _ in range(3):
            elegido = next(b for b in beats if b["trigger"] == {"type": "talk", "target": "sectario_perdido"}
                           and vba.cumple(vba.requisitos(b), flags, frozenset()))
            vistos.append(elegido["id"])
            flags = vba.aplica(elegido.get("effects", []), [(flags, frozenset())])[0][0]
        self.assertEqual(vistos, ["b1_sectarios_seguir", "b1_sectarios_despues", "b1_sectarios_despues"])


if __name__ == "__main__":
    unittest.main()
