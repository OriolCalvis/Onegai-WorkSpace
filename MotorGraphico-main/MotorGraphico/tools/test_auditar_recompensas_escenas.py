#!/usr/bin/env python3
"""Pruebas de auditar_recompensas_escenas.py (AV18-08). Sin dependencias: python3 tools/test_auditar_recompensas_escenas.py"""
import json, shutil, sys, tempfile, unittest
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
import auditar_recompensas_escenas as A

MOTOR = TOOLS.parent


class Auditor(unittest.TestCase):
    def _copia(self):
        d = Path(tempfile.mkdtemp())
        self.addCleanup(shutil.rmtree, d, True)
        (d / "tools").mkdir()
        shutil.copy(TOOLS / "recompensas_escenas.json", d / "tools")
        shutil.copytree(MOTOR / "assets" / "adventures", d / "assets" / "adventures")
        shutil.copytree(MOTOR / "assets" / "objects", d / "assets" / "objects")
        eco = A._cargar(TOOLS / "recompensas_escenas.json")["economia"]
        for base in (MOTOR, *MOTOR.parents):
            if (base / eco).exists():
                (d / eco).parent.mkdir(parents=True, exist_ok=True)
                shutil.copy(base / eco, d / eco)
                break
        return d

    def test_repo_limpio(self):
        _, errs, _ = A.comprueba(MOTOR)
        self.assertEqual(errs, [])

    def test_premio_sin_registrar(self):
        d = self._copia()
        p = d / "assets/adventures/boundington_ocaso.json"
        j = json.loads(p.read_text(encoding="utf-8"))
        j["beats"][0].setdefault("effects", []).append({"type": "grantGold", "value": 500})
        p.write_text(json.dumps(j, ensure_ascii=False), encoding="utf-8")
        _, errs, _ = A.comprueba(d)
        self.assertTrue(any("no esta en el libro" in e for e in errs))

    def test_cobro_repetible(self):
        d = self._copia()
        p = d / "assets/adventures/boundington_primer_dia.json"
        j = json.loads(p.read_text(encoding="utf-8"))
        b = next(x for x in j["beats"] if x["id"] == "b1_guarida_ganada")
        b["requires"]["notFlags"] = []
        p.write_text(json.dumps(j, ensure_ascii=False), encoding="utf-8")
        _, errs, _ = A.comprueba(d)
        self.assertTrue(any("mas de una vez" in e for e in errs))

    def test_presupuesto_desbordado(self):
        d = self._copia()
        p = d / "assets/adventures/boundington_ocaso.json"
        j = json.loads(p.read_text(encoding="utf-8"))
        b = next(x for x in j["beats"] if x["id"] == "b3_gorran")
        for e in b["effects"]:
            if e["type"] == "grantGold":
                e["value"] = 400
            if e["type"] == "log":
                e["arg"] = "Recibes 400 de oro."
        p.write_text(json.dumps(j, ensure_ascii=False), encoding="utf-8")
        _, errs, _ = A.comprueba(d)
        self.assertTrue(any("fuera de" in e for e in errs))


if __name__ == "__main__":
    unittest.main()
