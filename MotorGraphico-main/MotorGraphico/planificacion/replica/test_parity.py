"""Regresiones de puntuación e interfaz; no requieren el motor ni OpenGL."""
import contextlib
import csv
import io
import json
from pathlib import Path
import tempfile
import unittest

import parity


class ParityTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.matrix = Path(self.tmp.name) / "features.csv"

    def write_rows(self, rows):
        with self.matrix.open("w", encoding="utf-8", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["feature", "area", "priority", "original", "clone", "notes"])
            writer.writerows(rows)

    def run_cli(self, *args):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            code = parity.main([str(self.matrix), *args])
        return code, out.getvalue(), err.getvalue()

    def test_weighted_score_excludes_skips_and_own_features(self):
        self.write_rows([
            ["Audio", "audio", "must", "yes", "yes", "evidence"],
            ["Luz", "render", "should", "yes", "partial", "pending"],
            ["Extra", "editor", "could", "yes", "no", ""],
            ["3D", "render", "must", "yes", "skip", "outside scope"],
            ["Save", "save", "must", "no", "yes", "own"],
        ])
        result = parity.score(parity.load(self.matrix))
        self.assertAlmostEqual(result["feature_score_raw"], 100 * 4 / 6)
        self.assertEqual((result["counted"], result["must_have_done"],
                          result["must_have_total"]), (3, 1, 1))
        self.assertEqual(len(result["skipped"]), 1)
        self.assertEqual(len(result["extras"]), 1)

    def test_invalid_states_never_produce_success(self):
        for field, value in ((2, "urgent"), (3, "yess"), (4, "complete")):
            with self.subTest(field=field):
                row = ["Audio", "audio", "must", "yes", "no", ""]
                row[field] = value
                self.write_rows([row])
                code, _, err = self.run_cli()
                self.assertEqual(code, 2)
                self.assertIn("line 2", err)

    def test_skip_requires_explanation(self):
        self.write_rows([["3D", "render", "could", "yes", "skip", ""]])
        self.assertEqual(self.run_cli()[0], 2)

    def test_malformed_csv_returns_diagnostic_without_traceback(self):
        for content in ("feature,priority,clone\na,must,no,extra\n",
                        "feature,priority,clone\na,must\n",
                        "feature,priority,clone,clone\na,must,no,no\n"):
            with self.subTest(content=content):
                self.matrix.write_text(content)
                code, _, err = self.run_cli()
                self.assertEqual(code, 2)
                self.assertNotIn("Traceback", err)

    def test_empty_matrix_is_error(self):
        for content in ("", "feature,priority,clone\n"):
            self.matrix.write_text(content)
            self.assertEqual(self.run_cli()[0], 2)

    def test_threshold_uses_unrounded_score(self):
        self.write_rows([[str(i), "area", "could", "yes", "yes" if i < 2 else "no", ""]
                         for i in range(3)])
        self.assertEqual(self.run_cli("--fail-under", "66.7")[0], 1)
        self.assertEqual(self.run_cli("--fail-under", "66.6")[0], 0)

    def test_threshold_rejects_nonfinite_and_out_of_range(self):
        self.write_rows([["A", "a", "must", "yes", "yes", ""]])
        for value in ("nan", "inf", "-1", "101"):
            with self.subTest(value=value), contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as exc:
                    self.run_cli("--fail-under", value)
                self.assertEqual(exc.exception.code, 2)

    def test_quoted_unicode_and_json_roundtrip(self):
        self.write_rows([["Cámara, sacudida", "cámara", "must", "yes", "partial", "ñ"]])
        code, out, _ = self.run_cli("--json")
        self.assertEqual(code, 0)
        self.assertEqual(json.loads(out)["feature_score"], 50)
        self.assertIn("Cámara, sacudida", out)

    def test_imported_snapshot_is_reproducible(self):
        imported = Path(__file__).with_name("features.csv")
        result = parity.score(parity.load(imported))
        self.assertEqual((result["feature_score"], result["counted"],
                          result["must_have_total"]), (10, 25, 15))
        self.assertEqual((len(result["skipped"]), len(result["extras"])), (5, 5))

    def test_all_complete_and_no_counted_features(self):
        self.write_rows([["A", "a", "must", "yes", "yes", "evidence"]])
        self.assertEqual(json.loads(self.run_cli("--json")[1])["overall"], 100)
        self.write_rows([["Own", "a", "must", "no", "yes", "evidence"]])
        self.assertEqual(self.run_cli("--fail-under", "1")[0], 1)


if __name__ == "__main__":
    unittest.main()
