#!/usr/bin/env python3
"""Pruebas de tools/laminas_desde_capas.py (CAR-4). Sin ventana, sobre una copia temporal y solo con la
lamina razas_andando: python3 tools/test_laminas_desde_capas.py   (~2 min)"""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

AQUI = Path(__file__).resolve().parent
sys.path.insert(0, str(AQUI))
import laminas_desde_capas as LC  # noqa: E402

TEX = AQUI.parent / "assets" / "textures"
HOJA = "razas_andando"
FILA_MUTADA = 54  # aarakocra_del_ejercito_verde


def correr(tex, *args):
    r = subprocess.run([sys.executable, str(AQUI / "laminas_desde_capas.py"), "--tex", str(tex), "--hoja", HOJA, *args],
                       capture_output=True, text=True)
    return r.returncode, r.stdout.split(), r.stdout + r.stderr


def mutar_cuerpo(tex):
    p = Path(tex) / "personajes" / "razas" / "cuerpo.png"
    a = np.array(Image.open(p).convert("RGBA"))
    y = (FILA_MUTADA - 1) * 64
    a[y + 10:y + 20, 10:20] = [1, 2, 3, 255]
    Image.fromarray(a, "RGBA").save(p)


class PruebaLaminas(unittest.TestCase):
    def test_flujo_completo(self):
        with tempfile.TemporaryDirectory() as tmp:
            tex = Path(tmp) / "textures"
            tex.mkdir()
            for j in TEX.glob("*.json"):
                shutil.copy(j, tex)
            shutil.copytree(TEX / "personajes", tex / "personajes")
            shutil.copy(TEX / f"{HOJA}.png", tex)
            fila = f"{HOJA}:aarakocra_del_ejercito_verde"

            # (a) la copia limpia esta al dia, tambien por el camino rapido
            rc, _, txt = correr(tex, "--comprobar", "--rapido")
            self.assertEqual(rc, 0, txt)
            self.assertIn("sin cambios", txt)

            # (a2) lamina tocada a mano: un pixel de razas_andando.png cambia y el camino rapido ya no se fia
            png = tex / f"{HOJA}.png"
            original = png.read_bytes()
            im = np.array(Image.open(png).convert("RGBA"))
            im[5, 5] = [250, 3, 4, 255] if tuple(im[5, 5]) != (250, 3, 4, 255) else [1, 1, 1, 255]
            Image.fromarray(im, "RGBA").save(png)
            rc, _, txt = correr(tex, "--comprobar", "--rapido")
            self.assertEqual(rc, 1, txt)
            self.assertNotIn("sin cambios", txt)
            self.assertIn(f"{HOJA}:(PNG distinto", txt)
            png.write_bytes(original)
            rc, _, txt = correr(tex, "--comprobar", "--rapido")
            self.assertEqual(rc, 0, txt)

            # (b) una capa cambiada: el camino rapido no se fia y la comprobacion nombra solo esa fila
            mutar_cuerpo(tex)
            rc, _, txt = correr(tex, "--comprobar", "--rapido")
            self.assertEqual(rc, 1, txt)
            self.assertEqual(txt.split("\n")[0], fila, txt)
            self.assertNotIn("sin cambios", txt)
            filas = [l for l in txt.splitlines() if l.startswith(HOJA + ":")]
            self.assertEqual(filas, [fila], txt)

            # (c) --reconstruir la rehace, y la fila coincide con el montaje recalculado
            rc, _, txt = correr(tex, "--reconstruir")
            self.assertEqual(rc, 0, txt)
            rc, _, txt = correr(tex, "--comprobar")
            self.assertEqual(rc, 0, txt)
            rc, _, txt = correr(tex, "--comprobar", "--rapido")  # --reconstruir --hoja deja el camino rapido al dia
            self.assertEqual(rc, 0, txt)
            self.assertIn("sin cambios", txt)
            j = json.load(open(tex / f"{HOJA}.json"))
            a = next(x for x in j["actores"] if x["id"] == "aarakocra_del_ejercito_verde")
            img = np.array(Image.open(tex / f"{HOJA}.png").convert("RGBA"))
            y0 = (a["fila"] - 1) * 64
            esperada = LC.Montaje(tex).montar(a["composicion"])
            self.assertTrue((img[y0:y0 + 64] == esperada).all())

            # (d) mutacion: sin el hash del contenido de las capas en la huella, el cambio pasaria desapercibido
            limpio = Path(tmp) / "limpio"
            shutil.copytree(tex, limpio)
            shutil.copy(TEX / "personajes" / "razas" / "cuerpo.png", limpio / "personajes" / "razas" / "cuerpo.png")
            M0, M1 = LC.Montaje(limpio), LC.Montaje(tex)
            comp = a["composicion"]
            self.assertNotEqual(LC.huella(M0, comp, {}), LC.huella(M1, comp, {}))
            ciega = {(x["atlas"], x["fila"]): "x" for x in M0.capas(comp)}
            self.assertEqual(LC.huella(M0, comp, dict(ciega)), LC.huella(M1, comp, dict(ciega)))

            # (e) tinte y orden de dibujo forman parte de la huella (si no, un cambio de color o de orden
            # dejaria la lamina vieja sin aviso)
            base = LC.huella(M1, comp, {})
            tinte = json.loads(json.dumps(comp))
            tinte["equipo"]["torso"] = "cuero:acero>acero_escarcha"
            self.assertNotEqual(base, LC.huella(M1, tinte, {}), "el tinte no cambia la huella")
            orden = LC.Montaje(tex)
            orden.orden["torso"] = dict(orden.orden["torso"], abajo=orden.orden["torso"].get("abajo", 0) + 7)
            self.assertNotEqual(base, LC.huella(orden, comp, {}), "el orden de dibujo no cambia la huella")


if __name__ == "__main__":
    unittest.main()
