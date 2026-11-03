#!/usr/bin/env python3
"""Pruebas de validar_opciones_beat.py (E-MATRIZ-02, A0) con aventuras de mentira.

    python3 tools/test_validar_opciones_beat.py

Incluye la MUTACION: un beat con 5 opciones debe dar error; con 4 no. Y una prueba de humo sobre el repo real.
"""
import json
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import validar_opciones_beat as vob  # noqa: E402


def beat_con(n, bid="b"):
    return {"id": bid, "trigger": {"type": "talk", "target": "x"},
            "opciones": [{"texto": "op%d" % i} for i in range(n)]}


def problemas_de(*beats, maximo=4):
    with tempfile.TemporaryDirectory() as tmp:
        carpeta = Path(tmp) / "assets" / "adventures"
        carpeta.mkdir(parents=True)
        (carpeta / "mentira.json").write_text(json.dumps({"beats": list(beats)}, ensure_ascii=False, indent=1), encoding="utf-8")
        return vob.comprueba(tmp, maximo)[0]


class OpcionesDeBeat(unittest.TestCase):
    def test_cuatro_opciones_valen(self):
        self.assertEqual(problemas_de(beat_con(4)), [])

    def test_cinco_opciones_es_error(self):  # la mutacion de E-MATRIZ-02: 5 opciones -> error
        p = problemas_de(beat_con(1, "ok"), beat_con(5, "cinco"))
        self.assertEqual([x[1] for x in p], ["cinco"])
        self.assertIn("5 opciones", p[0][3])
        self.assertIsNotNone(p[0][2])  # lleva numero de linea

    def test_sin_opciones_no_cuenta(self):
        self.assertEqual(problemas_de({"id": "a", "trigger": {"type": "auto"}}), [])

    def test_el_maximo_sale_del_motor(self):
        raiz = HERE.parent
        self.assertEqual(vob.maximo_de_opciones(str(raiz)), 4)
        self.assertEqual(vob.maximo_de_opciones("/no/existe"), vob.POR_DEFECTO)

    def test_el_repo_real_esta_limpio(self):
        p, revisados = vob.comprueba(str(HERE.parent))
        self.assertEqual(p, [], p)
        self.assertGreater(revisados, 100)


if __name__ == "__main__":
    unittest.main()
