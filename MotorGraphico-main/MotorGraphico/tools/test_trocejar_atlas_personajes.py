"""PERF-3 — tools/trocejar_atlas_personajes.py: la fila reconstruida de un .filas es identica byte a
byte a la fila del PNG (un atlas de cuerpo, uno de pelo y uno de equipo); --comprobar detecta un
.filas ausente o desfasado; y es idempotente."""
import io
import struct
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import trocejar_atlas_personajes as t

ATLAS = ["personajes/cuerpos/cuerpos_pequeno.png",
         "personajes/pelo/pelo_pequeno.png",
         "personajes/equipo/torso_pequeno.png"]


class TrocejarTests(unittest.TestCase):
    def test_filas_identicas_a_las_del_png(self):
        with tempfile.TemporaryDirectory() as d:
            self.assertEqual(t.main(["--salida", d] + sum((["--solo", a] for a in ATLAS), [])), 0)
            for rel in ATLAS:
                png = t.TEXTURAS / rel
                dst = Path(d) / Path(rel).with_suffix(".filas")
                datos = dst.read_bytes()
                magic, ver, ancho, lado, n, crc, tam = t.CABECERA.unpack_from(datos)
                self.assertEqual((magic, ver, lado), (t.MAGIC, t.VERSION, 64))
                self.assertEqual((crc, tam), t.huella(png))
                orig = np.array(Image.open(png).convert("RGBA"))
                self.assertEqual((orig.shape[1], orig.shape[0] // 64), (ancho, n))
                for i in range(n):
                    off, ln = t.ENTRADA.unpack_from(datos, t.CABECERA.size + i * t.ENTRADA.size)
                    fila = np.array(Image.open(io.BytesIO(datos[off:off + ln])).convert("RGBA"))
                    self.assertTrue(np.array_equal(fila, orig[i * 64:(i + 1) * 64]), (rel, i))

    def test_comprobar_detecta_faltante_y_antiguo_y_es_idempotente(self):
        solo = ["--solo", ATLAS[0]]
        with tempfile.TemporaryDirectory() as d:
            self.assertEqual(t.main(["--salida", d, "--comprobar"] + solo), 1)  # falta
            self.assertEqual(t.main(["--salida", d] + solo), 0)
            self.assertEqual(t.main(["--salida", d, "--comprobar"] + solo), 0)
            dst = Path(d) / Path(ATLAS[0]).with_suffix(".filas")
            antes = dst.stat().st_mtime_ns
            self.assertEqual(t.main(["--salida", d] + solo), 0)  # al dia: no reescribe
            self.assertEqual(dst.stat().st_mtime_ns, antes)
            datos = bytearray(dst.read_bytes())  # un .filas de un PNG anterior: otra huella
            struct.pack_into("<I", datos, 20, struct.unpack_from("<I", datos, 20)[0] ^ 1)
            dst.write_bytes(bytes(datos))
            self.assertEqual(t.main(["--salida", d, "--comprobar"] + solo), 1)
            self.assertEqual(t.main(["--salida", d] + solo), 0)  # se regenera
            self.assertEqual(t.main(["--salida", d, "--comprobar"] + solo), 0)


if __name__ == "__main__":
    unittest.main()
