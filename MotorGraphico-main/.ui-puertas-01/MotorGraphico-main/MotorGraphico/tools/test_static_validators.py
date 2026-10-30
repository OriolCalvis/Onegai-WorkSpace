#!/usr/bin/env python3
"""Regresiones de declaraciones C++ y del alcance del mundo de Egaroth."""
import contextlib
import importlib.util
import io
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

TOOLS = Path(__file__).resolve().parent


class DeclarationTests(unittest.TestCase):
    def check_source(self, source):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "include/Editor").mkdir(parents=True)
            (root / "src/Editor").mkdir(parents=True)
            (root / "include/Editor/Actions.h").write_text(
                "namespace Editor { const std::array<Action, 14>& actions(); }\n")
            (root / "src/Editor/Actions.cpp").write_text(source)
            return subprocess.run(
                [sys.executable, str(TOOLS / "comprobar_declaraciones_rpg.py"), str(root)],
                capture_output=True, text=True, timeout=10)

    def test_template_return_with_comma_has_definition(self):
        result = self.check_source(
            "namespace Editor {\nconst std::array<Action, 14>& actions() { return data; }\n}\n")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_multiline_template_return_has_definition(self):
        result = self.check_source(
            "namespace Editor {\nconst std::array<Action,\n 14>&\nactions() { return data; }\n}\n")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_missing_definition_still_fails(self):
        result = self.check_source("namespace Editor { int another() { return 0; } }\n")
        self.assertEqual(result.returncode, 1)
        self.assertIn("Actions.h: actions", result.stdout)


class WorldScopeTests(unittest.TestCase):
    def setUp(self):
        spec = importlib.util.spec_from_file_location("world_validator", TOOLS / "validar_mundo_unido.py")
        self.world = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.world)

    def validate(self, extra_names, objects=()):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ["mapamundi.json", *extra_names]:
                (root / name).write_text("{}")
            world_level = {"objects": list(objects), "playerStart": {"x": 0, "y": 0}}
            levels = {"assets/levels/mapamundi.json": world_level}
            output = io.StringIO()
            with patch.object(self.world, "NIVELES", directory), \
                 patch.object(self.world, "carga_nivel", side_effect=levels.get), \
                 patch.object(self.world, "rejilla_de", return_value=None), \
                 contextlib.redirect_stdout(output):
                code = self.world.main()
            return code, output.getvalue()

    def test_example_gallery_is_deliberately_outside_world(self):
        code, output = self.validate(["ejemplos_objetos.json"])
        self.assertEqual(code, 0, output)

    def test_unknown_level_is_not_excluded_by_prefix(self):
        code, output = self.validate(["ejemplos_otro.json"])
        self.assertEqual(code, 1)
        self.assertIn("ejemplos_otro.json", output)

    def test_broken_portal_to_excluded_level_still_fails(self):
        code, output = self.validate(["ejemplos_objetos.json"], [{
            "objectId": "puerta", "targetLevel": "assets/levels/ejemplos_objetos.json"}])
        self.assertEqual(code, 1)
        self.assertIn("lleva a un nivel que no existe", output)


if __name__ == "__main__":
    unittest.main()
