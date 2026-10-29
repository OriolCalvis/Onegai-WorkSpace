#!/usr/bin/env python3
"""Pruebas de tools/recortar_colores_capa.py (NET-E1b), sin ventana y sobre atlas diminutos en una carpeta
temporal: python3 tools/test_recortar_colores_capa.py"""
import hashlib, json, sys, tempfile, unittest
from pathlib import Path

import numpy as np
from PIL import Image

AQUI = Path(__file__).resolve().parent
sys.path.insert(0, str(AQUI))
import recortar_colores_capa as R  # noqa: E402

PAL = {"pelo_castano": "#101010 #202020 #303030 #404040 #505050",
       "pelo_rubio": "#a01010 #b02020 #303030 #d04040 #e05050"}  # el tercer tono no cambia


def fila_castana(rng):
    f = np.zeros((R.L, 8, 4), np.uint8)
    tonos = [(0x10,) * 3, (0x20,) * 3, (0x30,) * 3, (0x40,) * 3, (0x50,) * 3]
    for y in range(R.L):
        for x in range(8):
            if rng.random() < .6:
                f[y, x, :3] = tonos[rng.integers(5)]
                f[y, x, 3] = 255
    return f


def montar(tmp, mutar=False):
    rng = np.random.default_rng(3)
    tex = Path(tmp)
    (tex / "personajes").mkdir()
    man = {"paletas": PAL}
    for g in R.GRUPOS:
        castana = fila_castana(rng)
        rubia = R.tenir(castana, R.pares(PAL, "pelo_rubio"))
        if mutar:
            rubia[3, 3] = [1, 2, 3, 255]
        propia = fila_castana(rng)
        atlas = np.concatenate([castana, rubia, propia])
        for t in ("medio", "bajo"):
            Image.fromarray(atlas, "RGBA").save(tex / "personajes" / f"{g}_{t}.png")
        man[g] = {"atlas": {t: f"personajes/{g}_{t}.png" for t in ("medio", "bajo")},
                  "filas": [{"id": "corto/pelo_castano", "estilo": "corto", "color": "pelo_castano", "fila": 1},
                            {"id": "corto/pelo_rubio", "estilo": "corto", "color": "pelo_rubio", "fila": 2},
                            {"id": "propio/x", "estilo": "", "color": "", "raza": "x", "fila": 3}]}
    (tex / "personajes.json").write_text(json.dumps(man, ensure_ascii=False, indent=1) + "\n")
    return tex


def sha(tex):
    return {p.name: hashlib.sha1(p.read_bytes()).hexdigest() for p in sorted(tex.rglob("*")) if p.is_file()}


class PruebaRecorte(unittest.TestCase):
    def test_recorta_renumera_e_idempotente(self):
        with tempfile.TemporaryDirectory() as tmp:
            tex = montar(tmp)
            antes = [np.asarray(Image.open(tex / "personajes" / "pelo_medio.png").convert("RGBA")).copy()]
            self.assertEqual(R.main(["--tex", str(tex)]), 0)
            man = json.load(open(tex / "personajes.json"))
            for g in R.GRUPOS:
                self.assertEqual([(f["id"], f["fila"]) for f in man[g]["filas"]],
                                 [("corto/pelo_castano", 1), ("propio/x", 2)])
                for t in man[g]["atlas"].values():
                    self.assertEqual(Image.open(tex / t).size[1], 2 * R.L)
            nuevo = np.asarray(Image.open(tex / "personajes" / "pelo_medio.png").convert("RGBA"))
            np.testing.assert_array_equal(nuevo[:R.L], antes[0][:R.L])        # la castana, igual
            np.testing.assert_array_equal(nuevo[R.L:], antes[0][2 * R.L:])    # la propia, igual
            quedo = sha(tex)
            self.assertEqual(R.main(["--tex", str(tex)]), 0)
            self.assertEqual(sha(tex), quedo)  # segunda pasada: no cambia nada

    def test_se_niega_si_una_fila_falla(self):
        with tempfile.TemporaryDirectory() as tmp:
            tex = montar(tmp, mutar=True)
            antes = sha(tex)
            self.assertEqual(R.main(["--tex", str(tex)]), 1)
            self.assertEqual(sha(tex), antes)  # ni un PNG ni el JSON tocados

    def test_comprobar_no_escribe(self):
        with tempfile.TemporaryDirectory() as tmp:
            tex = montar(tmp)
            antes = sha(tex)
            self.assertEqual(R.main(["--tex", str(tex), "--comprobar"]), 0)
            self.assertEqual(sha(tex), antes)


if __name__ == "__main__":
    unittest.main()
