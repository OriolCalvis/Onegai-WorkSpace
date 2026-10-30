#!/usr/bin/env python3
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import validar_audio as va  # noqa: E402


class Audio(unittest.TestCase):
    def test_bien_sin_licencia_y_falta(self):
        with tempfile.TemporaryDirectory() as d:
            m = Path(d)
            (m / "src/Game").mkdir(parents=True)
            (m / "src/Game/AudioDirector.cpp").write_text(
                'musica("a", "A", "musica/bien.ogg"); musica("b", "B", "musica/sinlic.ogg");\n'
                'efecto("c", "C", "sfx/falta.wav");', encoding="utf-8")
            (m / "assets/audio/musica").mkdir(parents=True)
            (m / "assets/audio/musica/bien.ogg").write_bytes(b"x")
            (m / "assets/audio/musica/sinlic.ogg").write_bytes(b"x")
            (m / "assets/audio/LICENCIAS.md").write_text(
                "musica/bien.ogg · autor · https://x · CC0 · 2026-10-28\n"
                "sfx/falta.wav · autor · https://x · CC0 · 2026-10-28\n", encoding="utf-8")
            p = va.comprueba(m)
        self.assertEqual(sorted(p), [
            "musica/sinlic.ogg: sin fila en assets/audio/LICENCIAS.md",
            "sfx/falta.wav: no existe en assets/audio/",
        ])

    def test_todo_bien(self):
        with tempfile.TemporaryDirectory() as d:
            m = Path(d)
            (m / "src/Game").mkdir(parents=True)
            (m / "src/Game/AudioDirector.cpp").write_text('x("sfx/ok.wav")', encoding="utf-8")
            (m / "assets/audio/sfx").mkdir(parents=True)
            (m / "assets/audio/sfx/ok.wav").write_bytes(b"x")
            (m / "assets/audio/LICENCIAS.md").write_text("sfx/ok.wav · a · https://u · CC0 · 2026-10-28\n", encoding="utf-8")
            self.assertEqual(va.comprueba(m), [])


class Categorias(unittest.TestCase):
    def montar(self, d, filas, texto_aut=None):
        m = Path(d)
        (m / "src/Game").mkdir(parents=True)
        (m / "src/Game/AudioDirector.cpp").write_text('x("sfx/a.wav") y("musica/b.ogg")', encoding="utf-8")
        (m / "assets/audio/sfx").mkdir(parents=True)
        (m / "assets/audio/musica").mkdir(parents=True)
        (m / "assets/audio/sfx/a.wav").write_bytes(b"x")
        (m / "assets/audio/musica/b.ogg").write_bytes(b"x")
        (m / "assets/audio/LICENCIAS.md").write_text(filas, encoding="utf-8")
        if texto_aut is not None:
            (m / "assets/audio/licencias").mkdir()
            (m / "assets/audio/licencias/erang.md").write_text(texto_aut, encoding="utf-8")
        return m

    TEXTO = "Autor: Erang\nURL de origen: https://e\nFecha de consulta: 2026-10-28\n"

    def test_cc0_y_autorizacion_conviven(self):
        with tempfile.TemporaryDirectory() as d:
            m = self.montar(d, "sfx/a.wav · A · https://x · CC0 · 2026-10-28\n"
                               "musica/b.ogg · Erang · https://e · AUT:erang · 2026-10-28\n", self.TEXTO)
            self.assertEqual(va.comprueba(m), [])

    def test_autorizacion_sin_texto_es_problema(self):
        with tempfile.TemporaryDirectory() as d:
            m = self.montar(d, "sfx/a.wav · A · https://x · CC0 · 2026-10-28\n"
                               "musica/b.ogg · Erang · https://e · AUT:erang · 2026-10-28\n")
            p = va.comprueba(m)
        self.assertEqual(len(p), 1)
        self.assertIn("falta el texto de la autorización", p[0])

    def test_licencia_desconocida_es_problema(self):
        with tempfile.TemporaryDirectory() as d:
            m = self.montar(d, "sfx/a.wav · A · https://x · CC-BY · 2026-10-28\n"
                               "musica/b.ogg · A · https://x · CC0 · 2026-10-28\n")
            p = va.comprueba(m)
        self.assertEqual(len(p), 1)
        self.assertIn("no reconocida", p[0])

    def test_fila_sin_url_o_fecha_es_problema(self):
        with tempfile.TemporaryDirectory() as d:
            m = self.montar(d, "sfx/a.wav · A · sin url · CC0 · ayer\n"
                               "musica/b.ogg · A · https://x · CC0 · 2026-10-28\n")
            p = va.comprueba(m)
        self.assertEqual(len(p), 2)


if __name__ == "__main__":
    unittest.main()
