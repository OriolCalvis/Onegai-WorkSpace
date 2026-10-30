import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from migrar_proyectos import migrate
from project_manifest import load_manifest, migrate_manifest, resolve_entry, serialize_manifest, validate_manifest


def legacy(pid="demo"):
    return {"id": pid, "nombre": 'Mapa "Niña"\nPrueba', "descripcion": "ruta\\texto\t",
            "epoca": "2000 b.f.", "autor": "Oriol", "prefijo": "d_", "entrada": "inicio.json",
            "niveles": ["inicio.json"], "mapas": ["inicio.tmx"],
            "aventuras": [], "catalogos": ["props.json"]}


class MigrationTests(unittest.TestCase):
    def test_empty_index_is_valid_and_nonobject_index_fails(self):
        with tempfile.TemporaryDirectory() as temp, contextlib.redirect_stdout(io.StringIO()):
            folder = Path(temp) / "proyectos"
            folder.mkdir()
            index = folder / "index.json"
            index.write_text('{"proyectos":[]}')
            self.assertEqual(migrate(temp, write=True), 0)
            index.write_text('[]')
            with self.assertRaises(ValueError):
                migrate(temp)

    def test_all_fields_survive_and_migration_is_idempotent(self):
        old = legacy()
        new = migrate_manifest(old)
        from project_manifest import LEGACY_FIELDS
        for before, after in LEGACY_FIELDS.items():
            self.assertEqual(old[before], new[after])
        self.assertEqual(new, migrate_manifest(new))
        self.assertEqual(new, json.loads(serialize_manifest(new)))

    def test_unknown_fields_versions_duplicates_and_paths_fail(self):
        for update in ({"unexpected": 1}, {"schemaVersion": 2}, {"niveles": ["a", "a"]},
                       {"niveles": ["../a"]}, {"entrada": "../../outside"}):
            with self.subTest(update=update), self.assertRaises(ValueError):
                migrate_manifest({**legacy(), **update})
        for version in (True, 1.5, 2, "1"):
            new = migrate_manifest(legacy())
            new["schemaVersion"] = version
            with self.subTest(version=version), self.assertRaises(ValueError):
                validate_manifest(new)
        new = migrate_manifest(legacy())
        new["customData"] = "must not disappear"
        with self.assertRaises(ValueError):
            migrate_manifest(new)

    def test_preview_writes_nothing_and_second_migration_changes_nothing(self):
        with tempfile.TemporaryDirectory() as temp, contextlib.redirect_stdout(io.StringIO()):
            root = Path(temp)
            folder = root / "proyectos"
            folder.mkdir()
            (folder / "index.json").write_text('{"proyectos":["demo"]}')
            path = folder / "demo.json"
            original = json.dumps(legacy(), ensure_ascii=False)
            path.write_text(original)
            self.assertEqual(migrate(root), 1)
            self.assertEqual(path.read_text(), original)
            self.assertEqual(migrate(root, write=True), 1)
            self.assertEqual(load_manifest(path)["name"], legacy()["nombre"])
            self.assertEqual(migrate(root, write=True), 0)

    def test_invalid_last_manifest_does_not_partially_migrate(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            folder = root / "proyectos"
            folder.mkdir()
            (folder / "index.json").write_text('{"proyectos":["demo","broken"]}')
            path = folder / "demo.json"
            original = json.dumps(legacy())
            path.write_text(original)
            (folder / "broken.json").write_text('{"id":"broken","unexpected":3}')
            with self.assertRaises(ValueError):
                migrate(root, write=True)
            self.assertEqual(path.read_text(), original)

    def test_entry_resolves_inside_copied_assets_root(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "levels").mkdir()
            target = root / "levels/inicio.json"
            target.write_text("{}")
            data = migrate_manifest(legacy())
            for entry in ("inicio.json", "levels/inicio.json", "assets/levels/inicio.json"):
                data["entry"] = entry
                self.assertEqual(resolve_entry(data, root), target)
            data["entry"] = "missing.json"
            self.assertIsNone(resolve_entry(data, root))


if __name__ == "__main__":
    unittest.main()
