#!/usr/bin/env python3
"""E-DISPARADORES-1 E3 — Pruebas del "disparador" en validar_cinematicas.py con un nivel de mentira.

    python3 tools/test_validar_cinematicas.py

Cada regla tiene su caso que la hace saltar: el foco y el destino de «lleva» dentro del nivel y transitables, una sola
acción, la zona dentro del mapa, el radio, el ámbito, tapaVista, y el aviso si menos del 50 % de la zona ve el foco.
Al final, el repo de verdad: 0 errores.
"""
import contextlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import validar_cinematicas as vc  # noqa: E402

MURO = 2  # firstgid 1; el tile id 1 (GID 2) lleva collision
OBJETOS = [
    {"id": "pared", "category": "prop", "blocksMovement": True, "tapaVista": True},
    {"id": "silla", "category": "prop", "blocksMovement": True},
    {"id": "alfombra", "category": "prop"},
]


def tmx(ancho, alto, muros=()):
    filas = []
    for y in range(alto):
        filas.append(",".join(str(MURO if (x, y) in muros else 1) for x in range(ancho)))
    return (f'<map version="1.10" width="{ancho}" height="{alto}" tilewidth="16" tileheight="16">'
            '<tileset firstgid="1" name="t" tilewidth="16" tileheight="16" tilecount="4" columns="2">'
            '<tile id="1"><properties><property name="collision" type="bool" value="true"/></properties></tile></tileset>'
            f'<layer id="1" name="c" width="{ancho}" height="{alto}"><data encoding="csv">\n' + ",\n".join(filas) +
            "\n</data></layer></map>")


def cine(**extra):
    base = {"id": "x", "nivel": "assets/levels/n.json", "zona": [4, 4, 8, 8],
            "disparador": {"lleva": {"a": [6, 6], "mira": "arriba"}, "accion": {"marca": "paso"}}}
    for k, v in extra.items():
        if k == "disparador":
            base["disparador"] = {**base["disparador"], **v}
        else:
            base[k] = v
    return base


class Mundo:
    """Un repo minimo en un directorio temporal: un nivel 14x14, un catálogo y las cinemáticas que se pidan."""

    def __init__(self, cinematicas, muros=(), objetos=(), nivel_extra=None, catalogo=None, ancho=14, alto=14):
        self.dir = tempfile.TemporaryDirectory()
        r = Path(self.dir.name)
        for sub in ("assets/proyectos", "assets/objects", "assets/levels", "assets/maps", "assets/cinematicas"):
            (r / sub).mkdir(parents=True)
        (r / "assets/proyectos/boundington.json").write_text(json.dumps({"catalogs": ["o.json"]}))
        (r / "assets/objects/o.json").write_text(json.dumps({"objects": catalogo or OBJETOS}))
        (r / "assets/maps/n.tmx").write_text(tmx(ancho, alto, muros))
        nivel = {"name": "n", "map": "assets/maps/n.tmx", "playerStart": {"x": 0, "y": 0},
                 "objects": [{"objectId": o, "position": {"x": x, "y": y}} for o, x, y in objetos]}
        nivel.update(nivel_extra or {})
        (r / "assets/levels/n.json").write_text(json.dumps(nivel))
        (r / "assets/cinematicas/cinematicas.json").write_text(json.dumps({"cinematicas": cinematicas}))
        self.raiz = str(r)

    def valida(self):
        antes = vc.RAIZ
        vc.RAIZ = self.raiz
        salida = io.StringIO()
        try:
            with contextlib.redirect_stdout(salida):
                codigo = vc.main()
        finally:
            vc.RAIZ = antes
            self.dir.cleanup()
        return codigo, salida.getvalue()


class DisparadorTests(unittest.TestCase):
    def espera_error(self, mundo, texto):
        codigo, salida = mundo.valida()
        self.assertEqual(codigo, 1, salida)
        self.assertIn(texto, salida)

    def test_un_disparador_bueno_no_da_ni_errores_ni_avisos_de_zona(self):
        codigo, salida = Mundo([cine()]).valida()
        self.assertEqual(codigo, 0, salida)
        self.assertNotIn("ven el foco", salida)

    def test_lleva_a_en_un_muro_de_tiles(self):
        self.espera_error(Mundo([cine()], muros=[(6, 6)]), "lleva.a (6, 6) no es transitable")

    def test_lleva_a_donde_hay_un_objeto_que_bloquea(self):
        self.espera_error(Mundo([cine()], objetos=[("silla", 6, 6)]), "lleva.a (6, 6) no es transitable")

    def test_un_objeto_que_no_bloquea_no_estorba(self):
        codigo, salida = Mundo([cine()], objetos=[("alfombra", 6, 6)]).valida()
        self.assertEqual(codigo, 0, salida)

    def test_lleva_a_fuera_del_nivel(self):
        self.espera_error(Mundo([cine(disparador={"lleva": {"a": [30, 2]}})]), "cae fuera del nivel")

    def test_el_foco_sin_lleva_es_el_centro_de_la_zona_y_tiene_que_ser_transitable(self):
        c = cine()
        del c["disparador"]["lleva"]
        self.espera_error(Mundo([c], muros=[(6, 6)]), "el foco (6, 6) (centro de la zona) no es transitable")

    def test_la_zona_sale_del_nivel(self):
        self.espera_error(Mundo([cine(zona=[10, 10, 40, 12])]), "sale del nivel")

    def test_dos_acciones_o_ninguna(self):
        self.espera_error(Mundo([cine(disparador={"accion": {"beat": "a", "marca": "b"}})]), "exactamente una clave")
        self.espera_error(Mundo([cine(disparador={"accion": {}})]), "exactamente una clave")

    def test_combate_es_de_E4(self):
        self.espera_error(Mundo([cine(disparador={"accion": {"combate": "enc"}})]), "llega con E4")

    def test_la_accion_apunta_a_una_cinematica_que_existe(self):
        self.espera_error(Mundo([cine(disparador={"accion": {"cinematica": "no_existe"}})]), "no existe")
        codigo, salida = Mundo([cine(disparador={"accion": {"cinematica": "x"}})]).valida()
        self.assertEqual(codigo, 0, salida)

    def test_el_radio_va_de_0_a_20(self):
        self.espera_error(Mundo([cine(disparador={"radio": 99})]), "radio")
        codigo, salida = Mundo([cine(disparador={"radio": 20})]).valida()
        self.assertEqual(codigo, 0, salida)

    def test_radio_sin_foco_no_tiene_a_donde_mirar(self):
        c = {"id": "x", "nivel": "assets/levels/n.json", "disparador": {"radio": 3}}
        self.espera_error(Mundo([c]), "no hay foco")

    def test_el_ambito_del_nivel_tiene_que_ser_uno_de_los_tres(self):
        self.espera_error(Mundo([cine()], nivel_extra={"ambito": "subterraneo"}), "ambito «subterraneo»")
        codigo, salida = Mundo([cine()], nivel_extra={"ambito": "interior_grande"}).valida()
        self.assertEqual(codigo, 0, salida)
        self.assertNotIn("sin «ambito»", salida)

    def test_un_nivel_sin_ambito_avisa_de_cual_se_deduce(self):
        codigo, salida = Mundo([cine()]).valida()
        self.assertIn("sin «ambito»; se deduce exterior", salida)

    def test_tapaVista_solo_en_un_prop_que_bloquea(self):
        cat = OBJETOS + [{"id": "cartel", "category": "prop", "tapaVista": True}]
        self.espera_error(Mundo([cine()], catalogo=cat), "tapaVista solo vale en un prop que bloquea")

    def test_aviso_si_menos_de_la_mitad_de_la_zona_ve_el_foco(self):
        # Zona 5x5 con el foco en (6, 6); un muro de tiles en la columna 5 (filas 4 a 8) corta casi todo el lado izquierdo,
        # y otro en la 7 el derecho: solo ven la columna 6.
        muros = [(5, y) for y in range(4, 9)] + [(7, y) for y in range(4, 9)]
        codigo, salida = Mundo([cine()], muros=muros).valida()
        self.assertEqual(codigo, 0, salida)
        self.assertIn("ven el foco (6, 6)", salida)

    def test_sin_muros_no_hay_aviso_de_vision(self):
        _, salida = Mundo([cine()]).valida()
        self.assertNotIn("ven el foco", salida)

    def test_once_false_avisa_que_aun_no_rearma(self):
        codigo, salida = Mundo([cine(disparador={"once": False})]).valida()
        self.assertEqual(codigo, 0, salida)
        self.assertIn("E5 aún no rearma", salida)

    def test_las_zonas_de_siempre_solo_avisan_si_su_centro_no_se_pisa(self):
        c = {"id": "vieja", "nivel": "assets/levels/n.json", "zona": [4, 4, 8, 8]}
        codigo, salida = Mundo([c], muros=[(6, 6)]).valida()
        self.assertEqual(codigo, 0, salida)  # aviso, no error: nada de lo que funciona cambia
        self.assertIn("el centro de la zona (6, 6) no es transitable", salida)


class AmbitoTests(unittest.TestCase):
    def test_la_deduccion_es_la_de_AmbitoNivel_h(self):
        d = vc.ambito_deducido
        self.assertEqual(d("assets/levels/interior_taberna.json", 15, 12), "interior_pequeno")
        self.assertEqual(d("assets/levels/interior_iglesia.json", 21, 15), "interior_grande")
        self.assertEqual(d("assets/levels/interior_x.json", 20, 10), "interior_grande")  # 200 exactos
        self.assertEqual(d("assets/levels/interior_x.json", 19, 10), "interior_pequeno")
        self.assertEqual(d("assets/levels/mina2_gorran.json", 42, 42), "interior_grande")
        self.assertEqual(d("assets/levels/aldea_cojo_p0.json", 10, 10), "interior_pequeno")
        self.assertEqual(d("assets/levels/ciudad_centro.json", 64, 64), "exterior")
        self.assertEqual(d("assets/levels/ciudad_en_aldea_m.json", 8, 8), "exterior")

    def test_bresenham_como_en_el_motor(self):
        muro = lambda x, y: x == 6  # noqa: E731
        self.assertFalse(vc.linea_de_vision((2, 5), (9, 5), muro))
        self.assertTrue(vc.linea_de_vision((2, 5), (6, 5), muro))  # el extremo no cuenta
        self.assertTrue(vc.linea_de_vision((6, 5), (9, 5), muro))


class RepoDeVerdadTests(unittest.TestCase):
    def test_el_repo_no_tiene_errores(self):
        if not (HERE.parent / "assets/cinematicas/cinematicas.json").exists():
            self.skipTest("sin assets")
        salida = io.StringIO()
        with contextlib.redirect_stdout(salida):
            codigo = vc.main()
        self.assertEqual(codigo, 0, salida.getvalue())


if __name__ == "__main__":
    unittest.main()
