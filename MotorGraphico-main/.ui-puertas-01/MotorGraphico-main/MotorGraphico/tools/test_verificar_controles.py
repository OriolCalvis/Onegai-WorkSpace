#!/usr/bin/env python3
"""Regresiones del guardian; fuentes sinteticas, sin modificar el editor."""
import contextlib
import io
import unittest
from pathlib import Path
from unittest.mock import patch

import verificar_controles as controles


ENUM = "enum class EditorTool { PaintTile, EraseTile };"
FUENTE = '''// Controles:
//   1..2 = herramienta
//   G = guardar (F5)
const char* ayuda = "1-2 HERRAM  G GUARDAR";
auto menu = {"1 " + std::string(toolName(EditorTool::PaintTile)),
             "2 " + std::string(toolName(EditorTool::EraseTile))};
if (keys.pressed(win, GLFW_KEY_1)) editor.setTool(EditorTool::PaintTile);
if (keys.pressed(win, GLFW_KEY_2)) editor.setTool(EditorTool::EraseTile);
if (pedida(GLFW_KEY_G, GLFW_KEY_F5)) guardar();
'''


class ControlesTests(unittest.TestCase):
    def comprobar(self, src=FUENTE, enum=ENUM):
        salida = io.StringIO()
        with contextlib.redirect_stdout(salida):
            codigo = controles.comprobar(src, enum)
        return codigo, salida.getvalue()

    def test_fixture_valida(self):
        self.assertEqual(self.comprobar()[0], 0)

    def test_editor_actual_y_ruta_migrada(self):
        self.assertEqual(Path(controles.FUENTE).parts[-3:],
                         ("editor", "src", "level_editor.cpp"))
        codigo, salida = self.comprobar(Path(controles.FUENTE).read_text(),
                                       Path(controles.ENUM).read_text())
        self.assertEqual(codigo, 0, salida)

    def test_banner_del_mapa_no_oculta_la_cabecera_de_controles(self):
        self.assertEqual(self.comprobar("// PAQUETES DEL PLAN\n// E-10\n\n" + FUENTE)[0], 0)

    def test_tecla_funcion_fantasma(self):
        codigo, salida = self.comprobar(FUENTE.replace("GLFW_KEY_F5", "GLFW_KEY_G"))
        self.assertEqual(codigo, 1)
        self.assertIn("F5", salida)

    def test_tab_fantasma_no_se_enlaza_desde_comentario_o_literal(self):
        src = FUENTE.replace("// Controles:", "// TAB = familias")
        src += '\n// GLFW_KEY_TAB\nconst char* x = "GLFW_KEY_TAB";\n'
        codigo, salida = self.comprobar(src)
        self.assertEqual(codigo, 1)
        self.assertIn("TAB", salida)

    def test_letra_anunciada_sin_enlace(self):
        codigo, salida = self.comprobar(FUENTE.replace("GLFW_KEY_G", "GLFW_KEY_F5"))
        self.assertEqual(codigo, 1)
        self.assertIn("      G", salida)

    def test_tecla_silenciosa(self):
        codigo, salida = self.comprobar(FUENTE + "keys.pressed(win, GLFW_KEY_SPACE);")
        self.assertEqual(codigo, 1)
        self.assertIn("SPACE", salida)

    def test_mismo_numero_de_herramientas_con_accion_equivocada(self):
        src = FUENTE.replace("editor.setTool(EditorTool::EraseTile)",
                             "editor.setTool(EditorTool::PaintTile)")
        codigo, salida = self.comprobar(src)
        self.assertEqual(codigo, 1)
        self.assertIn("LAS HERRAMIENTAS NO CUADRAN", salida)

    def test_menu_con_identidad_equivocada(self):
        src = FUENTE.replace("toolName(EditorTool::EraseTile)",
                             "toolName(EditorTool::PaintTile)")
        self.assertEqual(self.comprobar(src)[0], 1)

    def test_enum_ausente_o_nuevo_miembro(self):
        for enum in ("", "enum class EditorTool { PaintTile, EraseTile, FillTiles };"):
            with self.subTest(enum=enum):
                self.assertEqual(self.comprobar(enum=enum)[0], 1)

    def test_rangos_de_ayuda_desfasados(self):
        for antes, despues in (("1-2 HERRAM", "1-3 HERRAM"),
                               ("1..2 = herramienta", "1..3 = herramienta")):
            with self.subTest(antes=antes):
                self.assertEqual(self.comprobar(FUENTE.replace(antes, despues))[0], 1)

    def test_formato_cpp_y_comentarios_no_alteran_identidades(self):
        src = FUENTE.replace(")) editor.setTool", ")) { /* enlace */ editor.setTool")
        src = src.replace("editor.setTool(EditorTool::PaintTile);",
                          "editor.setTool(EditorTool::PaintTile); }")
        src = src.replace("editor.setTool(EditorTool::EraseTile);",
                          "editor.setTool(EditorTool::EraseTile); }")
        self.assertEqual(self.comprobar(src, ENUM.replace(
            "EraseTile", "/* segunda herramienta */ EraseTile"))[0], 0)

    def test_error_lectura_falla_sin_traceback(self):
        with patch("builtins.open", side_effect=FileNotFoundError("fuente ausente")):
            with contextlib.redirect_stderr(io.StringIO()) as salida:
                self.assertEqual(controles.main(), 1)
        self.assertIn("fuente ausente", salida.getvalue())

    def test_tokens_no_son_subcadenas_de_prosa(self):
        self.assertFalse(controles.aparece("G", "GUARDAR"))
        self.assertFalse(controles.aparece("TAB", "ESTABLE"))


class ShellControlesTests(unittest.TestCase):
    def setUp(self):
        self.src = Path(controles.SHELL).read_text()
        self.model = Path(controles.ACCIONES).read_text()
        self.header = Path(controles.ACCIONES_H).read_text()

    def check(self, src=None, model=None):
        with contextlib.redirect_stdout(io.StringIO()):
            return controles.comprobar_shell(self.src if src is None else src,
                self.model if model is None else model, self.header)

    def test_shell_actual(self):
        self.assertEqual(self.check(), 0)

    def test_accion_anunciada_sin_implementacion(self):
        self.assertEqual(self.check(model=self.model.replace("case EditorAction::Fill:", "case EditorAction::Paint:")), 1)

    def test_atajo_duplicado_o_tecla_desconocida(self):
        self.assertEqual(self.check(model=self.model.replace('ActionGroup::Tools, "G"', 'ActionGroup::Tools, "B"')), 1)
        self.assertEqual(self.check(model=self.model.replace('ActionGroup::Tools, "G"', 'ActionGroup::Tools, "Imaginary"')), 1)

    def test_tabla_no_conecta_el_teclado_por_un_comentario(self):
        src = self.src.replace("actions.shortcut(", "otro.shortcut(") + '\n// actions.shortcut(\nconst char* ayuda = "actions.shortcut(";\n'
        self.assertEqual(self.check(src=src), 1)

    def test_accion_directa_al_mapa_bypassea_el_despacho(self):
        self.assertEqual(self.check(src=self.src + "\nsession.fill(1, 1, 2);\n"), 1)


if __name__ == "__main__":
    unittest.main()
