"""Regresiones del enlace de Boundington, sin escribir en los assets reales."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parents[2] / "tools"


class BoundingtonRegenerationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="boundington regeneration ")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / "MotorGraphico-main" / "MotorGraphico"
        for relative in ("tools", "assets/levels", "assets/maps", "assets/objects"):
            (self.root / relative).mkdir(parents=True, exist_ok=True)
        for script in ("regenerar_ciudad.sh", "enganchar_boundington.py"):
            shutil.copyfile(TOOLS / script, self.root / "tools" / script)
        self.city = {
            "map": "assets/maps/ciudad_centro.tmx",
            "playerStart": {"x": 2, "y": 2},
            "objects": [{"objectId": "cartel", "position": {"x": 2, "y": 1}}],
        }
        self.landmass = {
            "objects": [{"objectId": "ciudad_mrnu1agq8t6u", "position": {"x": 4, "y": 5}}],
        }
        self.write_json("assets/levels/ciudad_centro.json", self.city)
        self.write_json("assets/levels/mundi_landmass_1.json", self.landmass)
        self.write_json("assets/objects/mapamundi_objetos.json", {"objects": []})
        self.write_grid()

    def write_json(self, relative, data):
        (self.root / relative).write_text(json.dumps(data, ensure_ascii=False), encoding="utf-8")

    def read_json(self, relative):
        return json.loads((self.root / relative).read_text(encoding="utf-8"))

    def write_grid(self, blocked=False):
        gids = [2 if blocked else 1] * 25
        gids[6] = 2  # La celda mas cercana (1,1) es pared.
        (self.root / "assets/maps/ciudad_centro.tmx").write_text(
            '<map width="5" height="5"><tileset>'
            '<tile id="1"><properties><property name="collision" value="true"/>'
            '</properties></tile></tileset><layer><data encoding="csv">'
            + ",".join(map(str, gids)) + '</data></layer></map>', encoding="utf-8")

    def run_linker(self):
        return subprocess.run(
            [sys.executable, str(self.root / "tools/enganchar_boundington.py")],
            cwd=self.temp.name, capture_output=True, text=True, timeout=10)

    def install_generation_stubs(self):
        # Los generadores reemplazan el contenido en su orden real; el script
        # de enganche es el de produccion y los validadores exigen ambas puertas.
        preamble = '''import json
from pathlib import Path
root = Path.cwd()
with (root / "calls.txt").open("a", encoding="utf-8") as calls:
    calls.write(Path(__file__).name + "\\n")
city_path = root / "assets/levels/ciudad_centro.json"
'''
        steps = {
            "tools/gen_tileset.py": "",
            "tools/gen_ciudad.py":
                'city_path.write_text((root / "city_fixture.json").read_text(encoding="utf-8"), encoding="utf-8")\n',
            "tools/gen_interiores.py": "",
            "tools/gen_carteles.py": "",
            "../../Onegai-Core/bridge/ambientar_boundington.py": '''city = json.loads(city_path.read_text(encoding="utf-8"))
city["objects"].append({"objectId": "pnj", "position": {"x": 3, "y": 1}})
city_path.write_text(json.dumps(city, ensure_ascii=False), encoding="utf-8")
''',
            "tools/conectividad.py": '''city = json.loads(city_path.read_text(encoding="utf-8"))
door = next(o for o in city["objects"] if o["objectId"] == "camino_a_egaroth")
if door["position"] in [o["position"] for o in city["objects"] if o is not door]:
    raise SystemExit("la puerta pisa un objeto")
if door["position"] == city["playerStart"]:
    raise SystemExit("la puerta pisa el inicio")
print("ciudad_centro 100%")
''',
            "tools/validar_enlaces.py": '''city = json.loads(city_path.read_text(encoding="utf-8"))
landmass = json.loads((root / "assets/levels/mundi_landmass_1.json").read_text(encoding="utf-8"))
door = next(o for o in city["objects"] if o["objectId"] == "camino_a_egaroth")
marker = landmass["objects"][0]
if (door["targetLevel"] != "assets/levels/mundi_landmass_1.json"
        or marker["targetLevel"] != "assets/levels/ciudad_centro.json"
        or marker["targetPosition"] != door["position"]
        or door["targetPosition"] != marker["position"]):
    raise SystemExit("enlace de ida o vuelta roto")
print("enlaces OK")
''',
        }
        self.write_json("city_fixture.json", self.city)
        for relative, body in steps.items():
            path = self.root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(preamble + body, encoding="utf-8")

    def run_regeneration(self):
        env = os.environ.copy()
        env["PATH"] = str(Path(sys.executable).parent) + os.pathsep + env.get("PATH", "")
        return subprocess.run(["bash", str(self.root / "tools/regenerar_ciudad.sh")],
                              cwd=self.temp.name, env=env, capture_output=True,
                              text=True, timeout=10)

    def test_regeneration_restores_both_links_after_population(self):
        self.install_generation_stubs()
        result = self.run_regeneration()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("enlaces OK", result.stdout)
        self.assertEqual((self.root / "calls.txt").read_text().splitlines(), [
            "gen_tileset.py", "gen_ciudad.py", "gen_interiores.py", "gen_carteles.py",
            "ambientar_boundington.py", "conectividad.py", "validar_enlaces.py"])
        city = self.read_json("assets/levels/ciudad_centro.json")
        door = next(o for o in city["objects"] if o["objectId"] == "camino_a_egaroth")
        self.assertEqual(door["position"], {"x": 1, "y": 2})

    def test_linker_moves_old_start_door_once_and_is_idempotent(self):
        old_door = {"objectId": "camino_a_egaroth", "position": self.city["playerStart"],
                    "targetLevel": "assets/levels/interior_armeria.json"}
        self.city["objects"].extend([old_door, old_door.copy()])
        self.write_json("assets/levels/ciudad_centro.json", self.city)
        result = self.run_linker()
        self.assertEqual(result.returncode, 0, result.stderr)
        city = self.read_json("assets/levels/ciudad_centro.json")
        doors = [o for o in city["objects"] if o["objectId"] == "camino_a_egaroth"]
        self.assertEqual(len(doors), 1)
        self.assertEqual(doors[0]["position"], {"x": 3, "y": 1})
        self.assertEqual(doors[0]["targetLevel"], "assets/levels/mundi_landmass_1.json")
        paths = list((self.root / "assets").rglob("*.json"))
        before = {p: p.read_bytes() for p in paths}
        result = self.run_linker()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(before, {p: p.read_bytes() for p in paths})
        catalog = self.read_json("assets/objects/mapamundi_objetos.json")
        self.assertEqual([o["id"] for o in catalog["objects"]], ["camino_a_egaroth"])

    def test_no_free_cell_fails_without_writing_links(self):
        self.write_grid(blocked=True)
        paths = list((self.root / "assets").rglob("*.json"))
        before = {p: p.read_bytes() for p in paths}
        result = self.run_linker()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("no hay hueco libre", result.stderr)
        self.assertEqual(before, {p: p.read_bytes() for p in paths})

    def test_regeneration_stops_before_validation_when_linking_fails(self):
        self.install_generation_stubs()
        self.write_grid(blocked=True)
        result = self.run_regeneration()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("no hay hueco libre", result.stderr)
        calls = (self.root / "calls.txt").read_text().splitlines()
        self.assertNotIn("conectividad.py", calls)
        self.assertNotIn("validar_enlaces.py", calls)


if __name__ == "__main__":
    unittest.main()
