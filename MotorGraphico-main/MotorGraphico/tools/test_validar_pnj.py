#!/usr/bin/env python3
"""Pruebas del alias explicito de validar_pnj.comprobar_alias.

    python3 tools/test_validar_pnj.py
"""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import validar_pnj as vp  # noqa: E402

ACTORES = {"vecina_ventana": ("pnj_boundington_3", 1, [], {}), "xila": ("pnj_boundington", 2, [], {}),
           "caballero": ("personajes_historia", 3, [], {}), "otra": ("pnj_mundo_1", 4, [], {})}


class AliasPnj(unittest.TestCase):
    def test_alias_a_fila_de_relleno_es_valido(self):
        self.assertIsNone(vp.comprobar_alias("v", "vecina_ventana", ACTORES, {"v": "vecina_ventana"}))

    def test_alias_a_fila_inexistente_es_error(self):
        self.assertIn("no tiene fila", vp.comprobar_alias("v", "nadie", ACTORES, {"v": "nadie"}))

    def test_alias_a_actor_de_trama_es_error(self):
        self.assertIn("trama", vp.comprobar_alias("v", "xila", ACTORES, {"v": "xila"}))

    def test_alias_a_hoja_que_no_es_pnj_es_error(self):
        self.assertIn("pnj_*", vp.comprobar_alias("v", "caballero", ACTORES, {"v": "caballero"}, trama=set()))

    def test_alias_de_alias_es_error(self):
        self.assertIn("alias", vp.comprobar_alias("v", "otra", ACTORES, {"v": "otra", "otra": "vecina_ventana"}))


if __name__ == "__main__":
    unittest.main()
