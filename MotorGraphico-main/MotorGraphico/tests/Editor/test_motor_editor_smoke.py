"""E-10: ratón, Cancelar, Ctrl+S, reapertura y texto en un workspace desechable."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", required=True, type=Path)
    parser.add_argument("--assets", required=True, type=Path)
    parser.add_argument("--scale", default="1")
    parser.add_argument("--capture", type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="motor-editor-smoke-") as temp:
        root = Path(temp)
        assets = root / "assets"
        for folder in ("maps", "levels", "textures", "proyectos", "shaders", "objects"):
            (assets / folder).mkdir(parents=True)
        (root / ".motor-editor-smoke").touch()
        shutil.copy2(args.assets / "maps/ejemplo_mapa.tmx", assets / "maps/ejemplo_mapa.tmx")
        shutil.copy2(args.assets / "textures/test_checker.png", assets / "textures/test_checker.png")
        for suffix in ("vert", "frag"):
            shutil.copy2(args.assets / f"shaders/sprite.{suffix}", assets / f"shaders/sprite.{suffix}")
        (assets / "proyectos/index.json").write_text('{"proyectos": ["smoke"]}\n', encoding="utf-8")
        project = {"schemaVersion": 1, "id": "smoke", "name": "Prueba", "prefix": "smoke_",
                   "description": "Fixture", "epoch": "", "author": "Test",
                   "settings": {"schemaVersion": 1, "mapsDirectory": "maps", "levelsDirectory": "levels",
                                "adventuresDirectory": "adventures", "catalogsDirectory": "objects",
                                "initialMap": "ejemplo_mapa.tmx"},
                   "entry": "assets/levels/ejemplo.json", "levels": ["ejemplo.json"],
                   "maps": ["ejemplo_mapa.tmx"], "adventures": [], "catalogs": ["smoke.json"]}
        (assets / "proyectos/smoke.json").write_text(json.dumps(project), encoding="utf-8")
        catalog = {"objects": [{"id": "pocion", "name": "Pocion de prueba", "category": "pickup"},
                               {"id": "cofre", "name": "Cofre de prueba", "category": "prop"}]}
        catalog_path = assets / "objects/smoke.json"
        catalog_path.write_text(json.dumps(catalog), encoding="utf-8")
        original_catalog = catalog_path.read_bytes()
        level = {"name": "Prueba del editor", "map": "assets/maps/ejemplo_mapa.tmx",
                 "playerStart": {"x": 1, "y": 1}, "objects": []}
        (assets / "levels/ejemplo.json").write_text(json.dumps(level) + "\n", encoding="utf-8")
        shutil.copy2(args.assets / "maps/ejemplo_mapa.tmx", assets / "maps/sin_catalogo.tmx")
        (assets / "levels/sin_catalogo.json").write_text(json.dumps(dict(level, map="assets/maps/sin_catalogo.tmx")), encoding="utf-8")
        capture = args.capture.resolve() if args.capture else root / "editor.ppm"
        process = subprocess.run([str(args.binary.resolve()), "--load", str(assets / "levels/ejemplo.json"),
                                  "--frames", "160", "--smoke-check", "--ui-scale", args.scale,
                                  "--capture", str(capture)], cwd=root, capture_output=True, text=True, timeout=45)
        print(process.stdout, end="")
        print(process.stderr, end="")
        if process.returncode:
            raise SystemExit(process.returncode)
        assert "editar/cancelar/guardar/reabrir OK" in process.stdout
        assert "rellenar/borrar/rehacer/ayuda OK" in process.stdout
        assert "objetos/catalogo/historial OK" in process.stdout
        assert "capas/seleccion/bloqueo/confirmacion OK" in process.stdout
        assert "nuevo mapa/pintar/guardar/reabrir OK" in process.stdout
        assert catalog_path.read_bytes() == original_catalog
        assert capture.stat().st_size > 1280 * 720
        assert Path(str(capture) + ".objects.ppm").stat().st_size > 1280 * 720
        native = json.loads((assets / "maps/ejemplo_mapa.map.json").read_text(encoding="utf-8"))
        assert native["schemaVersion"] == 1
        saved = json.loads((assets / "levels/ejemplo.json").read_text(encoding="utf-8"))
        assert saved["mapDocument"] == "assets/maps/ejemplo_mapa.map.json"
        assert saved["playerStart"] == level["playerStart"]
        assert len(saved["objects"]) == 1
        assert saved["objects"][0]["objectId"] == "pocion"
        assert saved["objects"][0]["position"] == {"x": 2, "y": 1}
        layers = json.loads((assets / "maps/sin_catalogo.map.json").read_text(encoding="utf-8"))["layers"]
        tiles = [layer for layer in layers if layer["type"] == "tile"]
        assert len(tiles) == 2
        assert tiles[-1]["name"] == "Detalle ## alto"
        assert not tiles[-1]["visible"] and not tiles[-1]["locked"]
        # H4-02: el mapa creado desde el panel Proyecto esta en el manifiesto,
        # con sus tres ficheros y el nombre que se escribio.
        manifest = json.loads((assets / "proyectos/smoke.json").read_text(encoding="utf-8"))
        assert "smoke_cueva_del_eco.json" in manifest["levels"]
        assert "smoke_cueva_del_eco.tmx" in manifest["maps"]
        created = json.loads((assets / "levels/smoke_cueva_del_eco.json").read_text(encoding="utf-8"))
        assert created["name"] == "Cueva del eco"
        assert created["mapDocument"] == "assets/maps/smoke_cueva_del_eco.map.json"
        assert (assets / "maps/smoke_cueva_del_eco.tmx").is_file()
        print("[PASS] UI y persistencia verificadas; assets originales intactos")


if __name__ == "__main__":
    main()
