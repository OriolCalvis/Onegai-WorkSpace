#!/usr/bin/env python3
import json
import random
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import explorar_aventura as ea  # noqa: E402


def E(flags=(), vars_=None):
    return {"flags": set(flags), "vars": dict(vars_ or {})}


class Carga(unittest.TestCase):
    def test_precuela_tiene_10_beats(self):
        ruta = ea.MOTOR / "assets" / "adventures" / "boundington_precuela_taberna.json"
        if not ruta.is_file():
            self.skipTest("sin aventura")
        beats, done = ea.carga(str(ruta))
        self.assertEqual(len(beats), 10)
        self.assertEqual(done, "pre_bnd_precuela_cerrada")


class Disponibles(unittest.TestCase):
    def test_cada_clase_de_condicion(self):
        beats = [
            {"id": "all", "requires": {"allFlags": ["a", "b"]}},
            {"id": "not", "requires": {"notFlags": ["x"]}},
            {"id": "any", "requires": {"anyFlags": ["p", "q"]}},
            {"id": "var", "requires": {"variables": {"v": {"min": 2}, "w": 1}}},
            {"id": "libre"},
        ]
        ids = lambda e: [b["id"] for b in ea.disponibles(e, beats)]  # noqa: E731
        self.assertEqual(ids(E()), ["not", "libre"])
        self.assertEqual(ids(E(["a", "b", "x", "q"], {"v": 2, "w": 1})), ["all", "any", "var", "libre"])
        self.assertEqual(ids(E(["a"], {"v": 1, "w": 1})), ["not", "libre"])


class Aplica(unittest.TestCase):
    EFECTOS = [
        {"type": "setFlag", "arg": "f"}, {"type": "setFlag", "arg": "g"}, {"type": "clearFlag", "arg": "g"},
        {"type": "setVariable", "arg": "v", "value": 3}, {"type": "addVariable", "arg": "v", "value": 5, "max": 6},
        {"type": "skillCheck", "skillCheck": {"flagSuccess": "ok", "flagBotch": "mal"}},
        {"type": "startBattle", "startBattle": {"flagVictory": "gana", "flagDefeat": "pierde"}},
        {"type": "log", "arg": "ignorado"},
    ]

    def test_efectos(self):
        e = E()
        ea.aplica(e, self.EFECTOS, random.Random(1))
        self.assertIn("f", e["flags"])
        self.assertNotIn("g", e["flags"])
        self.assertEqual(e["vars"]["v"], 6)
        self.assertEqual(len(e["flags"] & {"ok", "mal"}), 1)
        self.assertEqual(len(e["flags"] & {"gana", "pierde"}), 1)

    def test_misma_semilla_mismo_estado(self):
        a, b = E(), E()
        ea.aplica(a, self.EFECTOS, random.Random(7))
        ea.aplica(b, self.EFECTOS, random.Random(7))
        self.assertEqual(a, b)


class Explora(unittest.TestCase):
    def test_atasco_a_proposito(self):
        aventura = {"objectives": [{"doneFlag": "fin"}], "beats": [
            {"id": "b1", "effects": [{"type": "setFlag", "arg": "uno"}]},
            {"id": "b2", "requires": {"allFlags": ["uno"]}, "effects": [{"type": "setFlag", "arg": "dos"}]},
            {"id": "b3", "requires": {"allFlags": ["tres"]}, "effects": [{"type": "setFlag", "arg": "fin"}]},
        ]}
        with tempfile.TemporaryDirectory() as d:
            ruta = Path(d) / "a.json"
            ruta.write_text(json.dumps(aventura), encoding="utf-8")
            r = ea.explora(str(ruta), 10, 1)
        self.assertEqual(r["atascos"], 10)
        self.assertEqual(r["finales"], 0)
        self.assertEqual(r["beats_atasco"], {"b2": 10})
        self.assertEqual(r["no_visitados"], ["b3"])


if __name__ == "__main__":
    unittest.main()
