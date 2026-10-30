"""AV16-08 — nivel_a_excel.py / excel_a_nivel.py: ida y vuelta sin perder nada, y los errores no escriben."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

from openpyxl import load_workbook
from openpyxl.styles import PatternFill

REAL = Path(__file__).resolve().parents[2]
NIVEL = "ciudad_centro"


class NivelExcelTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory(prefix="nivel excel ")
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name) / "MotorGraphico"
        for t in ("nivel_a_excel.py", "excel_a_nivel.py", "nivel_excel_comun.py"):
            self._copia(f"tools/{t}")
        self._copia(f"assets/levels/{NIVEL}.json")
        self._copia(f"assets/maps/{NIVEL}.tmx")
        self._copia("assets/textures/terreno_iso.json")
        for f in (REAL / "assets/objects").glob("*.json"):
            self._copia(f"assets/objects/{f.name}")
        self.excel = self.root / f"{NIVEL}.xlsx"
        self.nivel = self.root / f"assets/levels/{NIVEL}.json"
        self.tmx = self.root / f"assets/maps/{NIVEL}.tmx"
        self.assertEqual(self._corre("nivel_a_excel.py", NIVEL, "-o", str(self.excel)).returncode, 0)

    def _copia(self, rel):
        destino = self.root / rel
        destino.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(REAL / rel, destino)

    def _corre(self, script, *args):
        return subprocess.run([sys.executable, str(self.root / "tools" / script), *args], capture_output=True, text=True)

    def _objetos(self):
        return sorted(json.dumps(o, sort_keys=True) for o in json.loads(self.nivel.read_text(encoding="utf-8"))["objects"])

    def test_ida_y_vuelta_no_cambia_nada(self):
        antes_objetos, antes_tmx = self._objetos(), self.tmx.read_text(encoding="utf-8")
        r = self._corre("excel_a_nivel.py", str(self.excel))
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(self._objetos(), antes_objetos)
        self.assertEqual(self.tmx.read_text(encoding="utf-8"), antes_tmx)

    def test_mover_borrar_y_poner(self):
        wb = load_workbook(self.excel)
        h = wb["Mapa"]
        # la primera puerta se mueve 1 casilla a la derecha (con su comentario), un banco se borra, un sauce nuevo
        puerta = next(c for fila in h.iter_rows(min_row=2, min_col=2) for c in fila if c.value == "P")
        x, y = puerta.column - 2, puerta.row - 2
        destino = h.cell(puerta.row, puerta.column + 1)
        destino.value, destino.comment = puerta.value, puerta.comment
        puerta.value, puerta.comment = None, None
        libre = next(c for fila in h.iter_rows(min_row=2, min_col=2) for c in fila if c.value is None and c is not destino)
        libre.value = "A:sauce"
        wb.save(self.excel)
        r = self._corre("excel_a_nivel.py", str(self.excel))
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        objs = json.loads(self.nivel.read_text(encoding="utf-8"))["objects"]
        movida = [o for o in objs if o["position"] == {"x": x + 1, "y": y} and o.get("targetLevel")]
        self.assertEqual(len(movida), 1)  # conserva destino y todo
        self.assertTrue(any(o["objectId"] == "sauce" and o["position"] == {"x": libre.column - 2, "y": libre.row - 2} for o in objs))

    def test_un_error_no_escribe_nada(self):
        antes = self.nivel.read_text(encoding="utf-8")
        wb = load_workbook(self.excel)
        h = wb["Mapa"]
        libre = next(c for fila in h.iter_rows(min_row=2, min_col=2) for c in fila if c.value is None)
        libre.value = "Z"  # letra que no existe
        wb.save(self.excel)
        r = self._corre("excel_a_nivel.py", str(self.excel))
        self.assertEqual(r.returncode, 1)
        self.assertIn("ERROR", r.stderr)
        self.assertEqual(self.nivel.read_text(encoding="utf-8"), antes)
        self.assertFalse(self.nivel.with_suffix(".json.bak").exists())

    def test_cambiar_el_suelo(self):
        wb = load_workbook(self.excel)
        h = wb["Mapa"]
        leyenda = {wb["Leyenda"].cell(r, 2).value: wb["Leyenda"].cell(r, 1).fill.fgColor.rgb  # toda la leyenda: ha crecido
                   for r in range(1, wb["Leyenda"].max_row + 1)}
        celda = h.cell(4, 4)
        celda.fill = PatternFill("solid", fgColor=leyenda["Mármol (plaza)"])
        wb.save(self.excel)
        r = self._corre("excel_a_nivel.py", str(self.excel))
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        gids = [int(v) for v in self.tmx.read_text(encoding="utf-8").split('<data encoding="csv">')[1].split("</data>")[0].replace("\n", "").split(",") if v]
        marmol = json.loads((self.root / "assets/textures/terreno_iso.json").read_text(encoding="utf-8"))["tiles"]
        self.assertEqual(gids[2 * 64 + 2], next(t["gid"] for t in marmol if t["nombre"] == "marmol_real"))

    def test_alturas_ida_y_vuelta(self):
        nivel = json.loads(self.nivel.read_text(encoding="utf-8"))
        nivel["alturas"] = [{"x": 4, "y": 4, "w": 3, "h": 3, "nivel": 2},
                            {"x": 4, "y": 7, "w": 3, "h": 3, "nivel": 0, "hasta": 2, "eje": "y"}]
        self.nivel.write_text(json.dumps(nivel), encoding="utf-8")
        self.assertEqual(self._corre("nivel_a_excel.py", NIVEL, "-o", str(self.excel)).returncode, 0)
        wb = load_workbook(self.excel)
        self.assertEqual(wb["Alturas"].cell(4 + 2, 4 + 2).value, 2)       # la meseta
        self.assertEqual(wb["Alturas"].cell(8 + 2, 5 + 2).value, 1)       # el medio de la rampa
        self.assertIsNone(wb["Alturas"].cell(2, 2).value)                 # lo plano, vacío
        # se edita: una casilla más a nivel 3
        wb["Alturas"].cell(4 + 2, 8 + 2).value = 3
        wb.save(self.excel)
        r = self._corre("excel_a_nivel.py", str(self.excel))
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        regiones = json.loads(self.nivel.read_text(encoding="utf-8"))["alturas"]
        sys.path.insert(0, str(self.root / "tools"))
        import nivel_excel_comun as C
        g = C.rejilla_de_alturas(regiones, 64, 64)
        self.assertEqual(g[4][8], 3)
        self.assertEqual(g[4][4], 2)
        self.assertEqual(g[8][5], 1)
        # un nivel fuera de rango es un error y no escribe
        antes = self.nivel.read_text(encoding="utf-8")
        wb = load_workbook(self.excel)
        wb["Alturas"].cell(2, 2).value = 12
        wb.save(self.excel)
        self.assertEqual(self._corre("excel_a_nivel.py", str(self.excel)).returncode, 1)
        self.assertEqual(self.nivel.read_text(encoding="utf-8"), antes)


if __name__ == "__main__":
    unittest.main()
