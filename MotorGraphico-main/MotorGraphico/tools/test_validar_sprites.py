#!/usr/bin/env python3
"""Pruebas del validador de atlas y de los generadores de sprites (ARTE-01 y
ARTE-02: personajes por capas).

    python3 tools/test_validar_sprites.py

Cada regla del validador tiene su caso que la hace fallar (si una regla se
borra, su prueba se pone roja). Ademas se comprueba que los atlas del repo
cumplen el contrato, que el mapa de partes cubre el cuerpo base entero, que
los PNG versionados son exactamente lo que producen los generadores, que el
probador monta con los atlas de capas lo mismo que los atlas horneados y
las reglas de composicion (tallas, armas en la mano, capas, razas).
"""
import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import validar_sprites as vs  # noqa: E402

TEX = HERE.parent / "assets/textures"


def _atlas(carpeta, frames, tam=8, columnas=2, filas=1, nombre="a.png"):
    """PNG de columnas x filas celdas; 'frames' = {frame: (x0, y0, x1, y1) opaco}."""
    img = np.zeros((filas * tam, columnas * tam, 4), np.uint8)
    for f, (x0, y0, x1, y1) in frames.items():
        c, r = (f - 1) % columnas, (f - 1) // columnas
        img[r * tam + y0:r * tam + y1, c * tam + x0:c * tam + x1] = (200, 100, 50, 255)
    Image.fromarray(img, "RGBA").save(Path(carpeta) / nombre)


def _manifiesto(carpeta, datos, nombre="a.json"):
    ruta = Path(carpeta) / nombre
    ruta.write_text(json.dumps(datos), encoding="utf-8")
    return ruta


class ValidadorTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = self.tmp.name
        _atlas(self.dir, {1: (2, 2, 6, 7), 2: (1, 1, 7, 7)})
        self.base = {"image": "a.png", "frameSize": [8, 8], "columns": 2, "rows": 1,
                     "generador": "prueba", "frames": {"uno": 1, "dos": 2}}

    def tearDown(self):
        self.tmp.cleanup()

    def test_atlas_conforme_no_da_errores(self):
        self.assertEqual(vs.validar(_manifiesto(self.dir, self.base)), [])

    def test_frame_fuera_de_la_rejilla(self):
        self.base["frames"]["tres"] = 3
        errores = vs.validar(_manifiesto(self.dir, self.base))
        self.assertTrue(any("fuera de 1..2" in e for e in errores), errores)

    def test_frame_vacio(self):
        _atlas(self.dir, {1: (2, 2, 6, 7)})  # el 2 queda vacio
        errores = vs.validar(_manifiesto(self.dir, self.base))
        self.assertTrue(any("vacio" in e for e in errores), errores)

    def test_dibujo_que_toca_el_borde_en_atlas_generado(self):
        _atlas(self.dir, {1: (2, 2, 6, 7), 2: (0, 1, 7, 7)})
        errores = vs.validar(_manifiesto(self.dir, self.base))
        self.assertTrue(any("toca el borde" in e for e in errores), errores)

    def test_el_borde_solo_se_exige_a_los_atlas_generados(self):
        _atlas(self.dir, {1: (2, 2, 6, 7), 2: (0, 1, 7, 8)})
        del self.base["generador"]  # los pies de un retrato pueden llegar abajo
        self.assertEqual(vs.validar(_manifiesto(self.dir, self.base)), [])

    def test_imagen_que_no_mide_lo_declarado(self):
        self.base["columns"] = 3
        errores = vs.validar(_manifiesto(self.dir, self.base))
        self.assertTrue(any("de ancho" in e for e in errores), errores)

    def test_falta_la_imagen(self):
        self.base["image"] = "no_existe.png"
        errores = vs.validar(_manifiesto(self.dir, self.base))
        self.assertTrue(any("falta la imagen" in e for e in errores), errores)

    def test_ids_repetidos(self):
        self.base["objetos"] = [{"id": "x", "frame": 1}, {"id": "x", "frame": 2}]
        errores = vs.validar(_manifiesto(self.dir, self.base))
        self.assertTrue(any("repetidos" in e for e in errores), errores)

    def test_animacion_fuera_de_la_fila_del_actor(self):
        _atlas(self.dir, {1: (2, 2, 6, 7), 2: (2, 2, 6, 7), 3: (2, 2, 6, 7), 4: (2, 2, 6, 7)},
               filas=2)
        self.base["rows"] = 2
        self.base["actores"] = [{"id": "a", "fila": 1, "animaciones": {"walk": [1, 2, 3]}}]
        errores = vs.validar(_manifiesto(self.dir, self.base))
        self.assertTrue(any("fuera de su fila" in e for e in errores), errores)

    def test_formato_de_race_npc_idle(self):
        datos = {"image": "a.png", "frame": {"width": 8, "height": 8}, "columns": 2,
                 "frames": [{"raceId": "r1", "frame": 1}, {"raceId": "r2", "frame": 2}]}
        self.assertEqual(vs.validar(_manifiesto(self.dir, datos)), [])


class ValidadorPersonajesTests(unittest.TestCase):
    """personajes.json: un manifiesto minimo con una raza, un peinado y una pieza."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self.tmp.name)
        tam, cols = 8, 12

        def hoja(nombre, filas, llenas):
            img = np.zeros((filas * tam, cols * tam, 4), np.uint8)
            for r in llenas:
                img[r * tam + 2:r * tam + 6, 2:6] = (200, 100, 50, 255)
            (self.dir / nombre).parent.mkdir(parents=True, exist_ok=True)
            Image.fromarray(img, "RGBA").save(self.dir / nombre)
        self.hoja = hoja
        hoja("r/cuerpo.png", 1, [0])
        hoja("r/cola.png", 1, [])
        for n in ("c.png", "o.png", "p.png", "b.png", "e.png"):
            hoja(n, 1, [0])
        orden = {r: {d: 1 for d in vs.DIRECCIONES}
                 for r in ("cuerpo", "cola", "ojos", "pelo", "barba", "torso")}
        self.man = {
            "frameSize": [tam, tam], "columns": cols, "orden": orden,
            "razas": {"atlas": {"cuerpo": "r/cuerpo.png", "cola": "r/cola.png"}, "filas": 1,
                      "lista": [{"id": "humano", "fila": 1, "capas": ["cuerpo"]}]},
            "cuerpos": {"atlas": {"medio": "c.png"}, "filas": [{"id": "piel", "fila": 1}]},
            "ojos": {"atlas": {"medio": "o.png"}, "filas": [{"id": "azul", "fila": 1}]},
            "pelo": {"atlas": {"medio": "p.png"}, "filas": [{"id": "corto/negro", "fila": 1}]},
            "barba": {"atlas": {"medio": "b.png"}, "filas": [{"id": "BARBA/negro", "fila": 1}]},
            "equipo": {"torso": {"atlas": {"medio": "e.png"}, "piezas": [{"id": "tunica", "fila": 1}]}},
            "personajes": {"historia": {"x": {"raza": "humano", "equipo": {"torso": "tunica"}}}},
        }

    def tearDown(self):
        self.tmp.cleanup()

    def validar(self):
        return vs.validar(_manifiesto(self.dir, self.man, "personajes.json"))

    def test_manifiesto_conforme_no_da_errores(self):
        self.assertEqual(self.validar(), [])

    def test_cuerpo_casi_vacio_en_celdas_de_64(self):
        # CAR-3b: con celdas reales (64x64) un cuerpo de pocos pixeles es un error; uno sano, no
        self.man["frameSize"] = [64, 64]
        for n, px in (("r/cuerpo.png", 4), ("r/cola.png", 0)):
            img = np.zeros((64, 12 * 64, 4), np.uint8)
            for f in range(12):
                img[10:10 + px, f * 64 + 10:f * 64 + 10 + px] = (200, 100, 50, 255)
            Image.fromarray(img, "RGBA").save(self.dir / n)
        self.assertTrue(any("cuerpo con 16 px opacos" in e for e in self.validar()))
        img = np.zeros((64, 12 * 64, 4), np.uint8)
        for f in range(12):
            img[5:25, f * 64 + 5:f * 64 + 25] = (200, 100, 50, 255)
        Image.fromarray(img, "RGBA").save(self.dir / "r/cuerpo.png")
        self.assertFalse(any("cuerpo con" in e for e in self.validar()))

    def test_falta_un_atlas(self):
        (self.dir / "p.png").unlink()
        self.assertTrue(any("falta p.png" in e for e in self.validar()))

    def test_atlas_sin_la_fila_de_una_pieza(self):
        self.man["equipo"]["torso"]["piezas"].append({"id": "cota", "fila": 2})
        self.assertTrue(any("deberia medir" in e for e in self.validar()))

    def test_fila_vacia(self):
        self.hoja("e.png", 1, [])
        self.assertTrue(any("esta vacia" in e for e in self.validar()))

    def test_capa_de_raza_con_dibujo_que_la_raza_no_declara(self):
        self.hoja("r/cola.png", 1, [0])
        self.assertTrue(any("capa cola" in e for e in self.validar()))

    def test_dibujo_que_toca_el_borde(self):
        img = np.zeros((8, 96, 4), np.uint8)
        img[0:4, 2:6] = (200, 100, 50, 255)
        Image.fromarray(img, "RGBA").save(self.dir / "p.png")
        self.assertTrue(any("toca el borde" in e for e in self.validar()))

    def test_ranura_sin_orden_de_dibujo(self):
        del self.man["orden"]["pelo"]["izquierda"]
        self.assertTrue(any("ranura pelo" in e for e in self.validar()))

    def test_personaje_con_pieza_que_no_existe(self):
        self.man["personajes"]["historia"]["x"]["equipo"]["torso"] = "armadura_de_oro"
        self.assertTrue(any("armadura_de_oro" in e for e in self.validar()))

    def test_ids_repetidos(self):
        self.hoja("e.png", 2, [0, 1])
        self.man["equipo"]["torso"]["piezas"].append({"id": "tunica", "fila": 2})
        self.assertTrue(any("repetidos" in e for e in self.validar()))

    def test_anclas_de_las_tallas(self):
        # PART-02b: un punto "x,y" dentro del frame por frame, para la mano, la otra, el pecho y la cintura
        bien = " ".join(["3,5"] * 12)
        self.man["tallas"] = {"medio": {"nombre": "Normal",
                                        "anclas": {p: bien for p in ("mano", "otraMano", "pecho", "cintura")}}}
        self.assertEqual(self.validar(), [])
        self.man["tallas"]["medio"]["anclas"]["pecho"] = " ".join(["3,5"] * 11)  # le falta un frame
        self.man["tallas"]["medio"]["anclas"]["cintura"] = " ".join(["3,9"] * 12)  # fuera del frame de 8
        errores = self.validar()
        self.assertTrue(any("anclas.pecho" in e for e in errores), errores)
        self.assertTrue(any("anclas.cintura" in e for e in errores), errores)
        del self.man["tallas"]["medio"]["anclas"]["mano"]
        self.assertTrue(any("anclas.mano" in e for e in self.validar()))


@unittest.skipUnless((TEX / "actores_capas.json").is_file(), "sin los atlas de ARTE-01")
class AtlasDelRepoTests(unittest.TestCase):
    def test_los_atlas_del_repo_cumplen_su_manifiesto(self):
        errores = []
        for nombre in vs.POR_DEFECTO:
            if (TEX / nombre).is_file():
                errores += vs.validar(TEX / nombre)
        self.assertEqual(errores, [])

    def test_el_jugador_mira_a_camara_al_quedarse_quieto(self):
        # personaje.png: arriba 1-3, derecha 4-6, abajo 7-9, izquierda 10-12
        man = json.loads((TEX / "personaje.json").read_text(encoding="utf-8"))
        self.assertEqual(man["animaciones"]["idle"], [8])
        self.assertEqual(man["animaciones"]["walk_down"], [7, 8, 9, 8])
        self.assertEqual(man["animaciones"]["walk_left"], [10, 11, 12, 11])

    def test_mapa_de_partes_cubre_el_cuerpo_base(self):
        import actores_partes
        _frames, labs, errores = actores_partes.construir(TEX / "personaje.png")
        self.assertEqual(errores, [])
        self.assertEqual(sorted(labs), list(range(1, 13)))


def _md5(ruta):
    return hashlib.md5(Path(ruta).read_bytes()).hexdigest()


def _huella(ruta):
    """md5 del contenido: de un PNG, sus pixeles (otra version de zlib o de PIL
    comprime distinto los mismos pixeles); de lo demas, sus bytes."""
    ruta = Path(ruta)
    if ruta.suffix == ".png":
        px = np.array(Image.open(ruta).convert("RGBA"))
        return hashlib.md5(px.tobytes() + str(px.shape).encode()).hexdigest()
    return _md5(ruta)


SALIDAS_PERSONAJES = ["actores_capas.png", "actores_capas.json", "razas_andando.png",
                      "razas_andando.json", "personajes_historia.png", "personajes_historia.json",
                      "personajes.json", "personaje.json"]


@unittest.skipUnless((TEX / "personajes.json").is_file(), "sin los atlas de ARTE-02")
class GeneradoresTests(unittest.TestCase):
    """Lo versionado es exactamente lo que sale del generador: nadie lo retoca a mano."""

    def test_personajes_son_reproducibles(self):
        import gen_personajes
        with tempfile.TemporaryDirectory() as tmp:
            gen_personajes.generar(salida=tmp)
            nombres = SALIDAS_PERSONAJES + sorted(
                str(p.relative_to(TEX)) for p in (TEX / "personajes").rglob("*.png"))
            for nombre in nombres:
                self.assertEqual(_huella(Path(tmp) / nombre), _huella(TEX / nombre), nombre)

    def test_props_y_pickups_son_reproducibles(self):
        import gen_props_p0
        with tempfile.TemporaryDirectory() as tmp:
            gen_props_p0.generar(salida=tmp)
            for nombre in ("props_p0.png", "props_p0.json", "pickups_p0.png", "pickups_p0.json"):
                self.assertEqual(_huella(Path(tmp) / nombre), _huella(TEX / nombre), nombre)


@unittest.skipUnless((TEX / "personajes.json").is_file(), "sin los atlas de ARTE-02")
class ProbadorTests(unittest.TestCase):
    """Con solo los atlas de capas y personajes.json (lo que tendra el motor) se
    montan exactamente los personajes de los atlas horneados."""

    def test_el_probador_monta_lo_mismo_que_los_atlas_horneados(self):
        import probador_personaje
        self._comparar(probador_personaje.Probador(TEX))

    def test_el_probador_tine_la_castana_si_falta_la_fila_de_color(self):
        """NET-E1a: con pelo y barba sin las filas de color (solo la castana y los propios) el probador
        tine la castana con la paleta del color y sale exactamente lo mismo que la fila cuita."""
        import probador_personaje
        pr = probador_personaje.Probador(TEX)
        for grupo in ("pelo", "barba"):
            g = pr.man[grupo]
            filas = g["filas"]
            castanos = {f["estilo"]: f for f in filas if f["color"] == "pelo_castano"}
            muestra = [f for f in filas if f["estilo"] in castanos and f["color"] != "pelo_castano"]
            self.assertGreater(len(muestra), 0)
            for talla in ("medio", "pequeno"):
                if talla not in g["atlas"]:
                    continue
                # un estilo por color basta: la misma paleta tine todos los estilos
                vistos = set()
                for f in muestra:
                    if f["color"] in vistos:
                        continue
                    vistos.add(f["color"])
                    cuita = pr.fila(g["atlas"][talla], f["fila"])
                    fila, cambios = pr._fila_color({"filas": [x for x in filas if x["id"] != f["id"]]}, f["id"], grupo)
                    self.assertIsNotNone(cambios, f["id"])
                    tenida = pr.fila(g["atlas"][talla], fila, cambios)
                    for i, (a, b) in enumerate(zip(cuita, tenida)):
                        self.assertTrue(np.array_equal(a, b), f"{grupo}.{talla}.{f['id']} frame {i + 1}")
        with self.assertRaises(SystemExit):
            pr._fila_color(pr.man["pelo"], "corto/pelo_inventado", "peinado")
        self.assertEqual(pr._fila_color(pr.man["pelo"], "corto/pelo_castano", "peinado")[1], None)

    def test_la_lamina_tine_la_castana_si_falta_la_fila_de_color(self):
        """NET-E1a: pixellab_lamina.Atlas.pelo cae a la fila castana tenida (antes devolvia None)."""
        import pixellab_lamina
        at = pixellab_lamina.Atlas(TEX)
        g = at.man["pelo"]
        cuita = at.pelo("corto/pelo_rubio", "pequeno")
        g["filas"] = [f for f in g["filas"] if f["id"] != "corto/pelo_rubio"]
        tenida = at.pelo("corto/pelo_rubio", "pequeno")
        self.assertIsNotNone(tenida)
        self.assertTrue(all(np.array_equal(a, b) for a, b in zip(cuita, tenida)))
        self.assertIsNone(at.pelo("corto/pelo_inventado", "pequeno"))

    def _comparar(self, pr):
        for nombre in ("actores_capas", "razas_andando", "personajes_historia"):
            man = json.loads((TEX / f"{nombre}.json").read_text(encoding="utf-8"))
            img = np.array(Image.open(TEX / f"{nombre}.png").convert("RGBA"))
            for a in man["actores"]:
                frames = pr.personaje(a["composicion"])
                r = a["fila"] - 1
                for c in range(12):
                    ref = img[r * 64:(r + 1) * 64, c * 64:(c + 1) * 64]
                    self.assertTrue(np.array_equal(ref, frames[c]), f"{nombre}.{a['id']} frame {c + 1}")


class ParticulasTests(unittest.TestCase):
    """PART-01 (Oriol, 07/10): las particulas de los elementos y de los temas de las clases. La lista es
    tools/particulas.py; el motor pinta las mismas (Game::tipoDeParticula, src/Game/Personajes/
    FamiliasDeParticulas.cpp), con las mismas cifras."""

    def test_el_motor_tiene_las_mismas_familias(self):
        import re
        import particulas
        fuente = (HERE.parent / "src/Game/Personajes/FamiliasDeParticulas.cpp").read_text(encoding="utf-8")
        tabla = fuente[fuente.index("kTipos = {"):fuente.index("};", fuente.index("kTipos = {"))]
        fila = re.compile(r'\{"(\w+)", \{(-?[\d.]+)f, (-?[\d.]+)f, (-?[\d.]+)f, F::(\w+), (true|false), M::(\w+),\s*'
                          r'\{((?:\{0x\w\w, 0x\w\w, 0x\w\w\}(?:, )?)*)\}'
                          r'(?:,\s*([\d.]+)f)?(?:,\s*F::(\w+),\s*([\d.]+)f)?(?:,\s*(true|false))?\}\}')
        motor = {}
        for m in fila.finditer(tabla):
            colores = re.findall(r"0x(\w\w), 0x(\w\w), 0x(\w\w)", m.group(8))
            con_otra = float(m.group(11) or 0.0)
            motor[m.group(1)] = (float(m.group(2)), float(m.group(3)), float(m.group(4)), m.group(5).lower(),
                                 m.group(6) == "true", m.group(7).lower(), ["".join(c) for c in colores],
                                 float(m.group(9) or 1.0), m.group(10).lower() if con_otra > 0 else None, con_otra,
                                 m.group(12) == "true")
        self.assertEqual(sorted(motor), sorted(particulas.FAMILIAS))
        for nombre, f in particulas.FAMILIAS.items():
            forma = f["forma"].replace("_", "")
            otra = f["otra_forma"].replace("_", "") if f["otra_forma"] else None
            self.assertEqual(motor[nombre], (f["vy"], f["vx"], f["vida"], forma, f["suma"], f["movimiento"],
                                             f["colores"], f["densidad"], otra, f["con_otra_forma"], f["gira"]),
                             nombre)
            self.assertIn(f["forma"], particulas.FORMAS, nombre)
            self.assertIn(f["movimiento"], particulas.MOVIMIENTOS, nombre)

    def test_cada_clase_y_cada_elemento_tienen_sus_particulas(self):
        import json
        import elementos
        import particulas
        clases = json.loads((HERE.parent / "assets/catalogs/classes.json").read_text(encoding="utf-8"))["entries"]
        self.assertEqual(sorted(particulas.CLASES), sorted(c["id"] for c in clases))
        for nombre, (familias, por_que) in particulas.CLASES.items():
            self.assertTrue(familias and por_que, nombre)
            for f in familias:
                self.assertIn(f, particulas.FAMILIAS, nombre)
        for e, (_palabra, color, familia) in elementos.ELEMENTOS.items():
            self.assertIn(familia, particulas.FAMILIAS, e)
            self.assertEqual(particulas.FAMILIAS[familia]["de"], e, e)
            self.assertRegex(color, r"^[0-9a-f]{6}$")
        # Oriol: no las mismas en los puños de las Artes Misticas que en los del Puño del dragon, y el fuego del
        # Druida flamigero, mas puro
        primera = {k: v[0][0] for k, v in particulas.CLASES.items()}
        self.assertEqual(primera["puno_del_dragon"], "dragones")
        self.assertNotEqual(primera["artes_misticas"], primera["puno_del_dragon"])
        self.assertEqual(primera["druida_flamigero"], "llamas_puras")
        self.assertEqual(primera["el_apostata"], "sangre")
        self.assertEqual(primera["senor_de_las_armas"], "oro")
        self.assertEqual(primera["tempestario"], "rayos")

    def test_cada_carta_echa_una_de_las_de_su_clase(self):
        """La lista de cada clase es la de sus cartas: cada carta echa una de las de su clase y cada una de esas
        (menos la primera, que es la de su set y sus abalorios) la echa alguna carta."""
        import json
        from collections import Counter
        import particulas
        clases = json.loads((HERE.parent / "assets/catalogs/classes.json").read_text(encoding="utf-8"))["entries"]
        cartas = json.loads((HERE.parent / "assets/catalogs/skills.json").read_text(encoding="utf-8"))["entries"]
        familia = particulas.de_cada_carta(clases, cartas)
        uso = {}
        for c in cartas:
            for k in c.get("classTags") or []:
                if k in particulas.CLASES:
                    self.assertIn(familia[c["id"]], particulas.CLASES[k][0], (k, c["id"]))
                    uso.setdefault(k, Counter())[familia[c["id"]]] += 1
        for k, (familias, _por_que) in particulas.CLASES.items():
            for f in familias[1:]:
                self.assertIn(f, uso.get(k, {}), (k, f))
        # Los ejemplos de Oriol: el Apostata, sangre; el Puño del dragon, dragones; el Druida flamigero, fuego
        # puro; las Artes Misticas, petalos (no lo mismo que el Puño del dragon)
        for k, f in (("el_apostata", "sangre"), ("puno_del_dragon", "dragones"), ("druida_flamigero", "llamas_puras"),
                     ("artes_misticas", "petalos"), ("senor_de_las_armas", "oro"), ("tempestario", "rayos")):
            self.assertEqual(set(uso[k]), {f}, k)
        # el texto elige entre las de su clase, sin cambiar de elemento: la cancion del invierno del bardo de las
        # estaciones es escarcha; el halcon de la Espia, plumas; la «Tormenta» de hielo del Cazador, escarcha
        self.assertEqual(familia["cazador_tormenta"], "escarcha")
        self.assertEqual(familia["espia_vuelo_en_picado_de_halcon"], "plumas")
        self.assertEqual(familia["tempestario_saeta_de_rayo"], "rayos")

    def test_las_cartas_llevan_el_elemento_del_juego(self):
        """Oriol (07/10): «no estamos limitados a los 4 elementos del nostale aqui»: el elemento de una carta
        es uno de los del juego (o ninguno), y el rayo es Rayo."""
        import json
        import elementos
        cartas = json.loads((HERE.parent / "assets/catalogs/skills.json").read_text(encoding="utf-8"))["entries"]
        por_id = {c["id"]: c for c in cartas}
        for ident, elemento in (("tempestario_saeta_de_rayo", "Rayo"), ("tronador_electrochoque", "Rayo"),
                                ("mago_azul_lanza_de_hielo", "Hielo"), ("caballero_del_rey_lobo_ventisca", "Hielo"),
                                ("corsario_vampirico_mordisco", "Sangre"), ("guerrero_terremoto", "Tierra"),
                                ("druida_del_roble_raices_enredadoras", "Naturaleza")):
            self.assertIn(elemento, por_id[ident]["mechanicTags"], ident)
        for c in cartas:
            for t in c.get("mechanicTags") or []:
                if t in elementos.ELEMENTOS or t in elementos.ALIAS:
                    self.assertIsNotNone(elementos.de_la_etiqueta(t), (c["id"], t))
        self.assertNotIn("Luz", por_id["tempestario_saeta_de_rayo"]["mechanicTags"])

    def test_el_motor_sabe_que_echa_cada_carta(self):
        """PART-02d: assets/textures/particulas_cartas.json es lo que dice la regla (tools/particulas_cartas.py),
        y las familias de los elementos del motor (Game::familiaDeElemento, para las cartas sin clase) son las
        de tools/elementos.py."""
        import re
        import elementos
        import particulas
        import particulas_cartas
        guardado = json.loads((TEX / "particulas_cartas.json").read_text(encoding="utf-8"))
        ahora = particulas_cartas.datos()
        self.assertEqual(guardado["cartas"], ahora["cartas"], "regenera: python3 tools/particulas_cartas.py")
        self.assertEqual(guardado["colores"], ahora["colores"])
        self.assertTrue(set(guardado["cartas"].values()) <= set(particulas.FAMILIAS))
        fuente = (HERE.parent / "src/Game/ParticulasDeCartas.cpp").read_text(encoding="utf-8")
        tabla = fuente[fuente.index("kElementos = {"):fuente.index("};", fuente.index("kElementos = {"))]
        motor = dict(re.findall(r'\{"([^"]+)", "(\w+)"\}', tabla))
        esperado = {e: v[2] for e, v in elementos.ELEMENTOS.items()}
        esperado.update({a: elementos.ELEMENTOS[e][2] for a, e in elementos.ALIAS.items()})
        self.assertEqual(motor, esperado)


class AbaloriosT2Tests(unittest.TestCase):
    """PART-02c (Oriol: «podemos crear un anillo que de esas particulas aumentando la equipacion del tier
    2»): un anillo por elemento, cuatro de dos efectos y un collar o un cinturon por cada tema de clase
    (tools/abalorios_t2.py), en el catalogo, con su objeto puente y sus particulas."""

    @classmethod
    def setUpClass(cls):
        import abalorios_t2
        cls.ab = abalorios_t2
        cat = HERE.parent / "assets/catalogs"
        cls.equipo = {e["id"]: e for e in json.loads((cat / "equipment.json").read_text(encoding="utf-8"))["entries"]}
        obj = HERE.parent / "assets/objects"
        cls.puentes = {n: {o["id"] for o in json.loads((obj / n).read_text(encoding="utf-8"))["objects"]}
                       for n in ("libreria_armas.json", "libreria_completa.json")}
        cls.sin_pieza = {e["id"]: e for e in json.loads(
            (TEX / "personajes/equipo_catalogo.json").read_text(encoding="utf-8"))["sinPieza"]}

    def test_los_datos_cuadran(self):
        self.assertEqual(self.ab.comprobar(), [])
        slots = [a[2] for a in self.ab.abalorios()]
        self.assertEqual((slots.count("Ring"), slots.count("Neck"), slots.count("Belt")), (17, 16, 10))

    def test_en_el_catalogo_de_tier_2_con_su_puente(self):
        import niveles_equipo
        for a in self.ab.abalorios():
            e = self.equipo.get(a[0])
            self.assertIsNotNone(e, a[0])
            self.assertEqual((e["tier"], e["rarity"], e["slot"]), (2, "rare", a[2]))
            self.assertEqual(e["requiredLevel"], niveles_equipo.nivel_por_regla(e, set()), a[0])
            self.assertIn(sum(e["statBonuses"].values()), (1, 2), a[0])
            for n, ids in self.puentes.items():
                self.assertIn(f"item_{a[0]}", ids, n)

    def test_cada_uno_echa_sus_particulas(self):
        for a in self.ab.abalorios():
            b = self.sin_pieza[a[0]]["brillo"]
            self.assertTrue(b["abalorio"])
            self.assertEqual(b["particulas"], a[3][0][0], a[0])
            self.assertEqual(b.get("otrasParticulas"), a[3][1][0] if len(a[3]) > 1 else None, a[0])
            self.assertIn(b["nivel"], (3, 4))  # Tier 2: +3 o +4
        fuego_y_hielo = self.sin_pieza["anillo_de_fuego_y_escarcha"]["brillo"]
        self.assertEqual((fuego_y_hielo["particulas"], fuego_y_hielo["otrasParticulas"]), ("llamas", "escarcha"))

    def test_cada_clase_encuentra_el_suyo(self):
        import particulas
        familias = {f for a in self.ab.abalorios() for f, _c in a[3]}
        for clase, (fams, _por_que) in particulas.CLASES.items():
            self.assertIn(fams[0], familias, clase)


class ComposicionTests(unittest.TestCase):
    """Reglas de montaje de personajes (sin atlas: directamente con los modulos)."""

    @classmethod
    def setUpClass(cls):
        import actores_tallas
        cls.tallas = actores_tallas.construir_tallas()

    def test_alturas_de_las_cuatro_tallas(self):
        alturas = {k: t.altura() for k, t in self.tallas.items()}
        self.assertEqual(alturas, {"medio": 40, "bajo": 36, "pequeno": 33, "grande": 45})

    def test_la_cabeza_no_cambia_entre_tallas(self):
        # el pelo y los cascos se reutilizan: la cabeza solo se desplaza
        base = self.tallas["medio"]
        for clave, t in self.tallas.items():
            for f in (2, 5, 8):
                a = base.frames[f - 1][base.top[f]:base.top[f] + 12]
                b = t.frames[f - 1][t.top[f]:t.top[f] + 12]
                self.assertTrue(np.array_equal(a, b), f"{clave} frame {f}")

    def test_las_nagas_no_llevan_piernas_ni_pies(self):
        import actores_personajes as pers
        spec = {"raza": "nagas_venenosos",
                "equipo": {"torso": "cuero", "piernas": "pantalon_gris", "pies": "botas"}}
        ranuras = [r for r, _ in pers.capas_personaje(spec, self.tallas)]
        self.assertIn("torso", ranuras)
        self.assertNotIn("piernas", ranuras)
        self.assertNotIn("pies", ranuras)

    def test_el_yelmo_cerrado_tapa_el_pelo(self):
        import actores_personajes as pers
        spec = {"raza": "race_human_marches", "pelo": ("largo", "pelo_rubio"),
                "equipo": {"cabeza": "yelmo"}}
        self.assertNotIn("pelo", [r for r, _ in pers.capas_personaje(spec, self.tallas)])
        spec["equipo"]["cabeza"] = "sombrero_paja"
        self.assertIn("pelo", [r for r, _ in pers.capas_personaje(spec, self.tallas)])

    def test_las_anclas_de_los_abalorios(self):
        """PART-02b: de donde echan los abalorios (el anillo de la mano del arma, el de la otra, el collar
        y el cinturon), en los 20 frames de cada talla, y lo mismo en personajes.json."""
        import json

        import actores_anclas as anclas
        man = json.loads((TEX / "personajes.json").read_text(encoding="utf-8"))
        for clave, t in self.tallas.items():
            a = anclas.anclas_de_talla(t)
            self.assertEqual(sorted(a), sorted(anclas.PUNTOS))
            for f in range(1, t.n_frames + 1):
                lab = t.labs[f]
                mano, otra = a["mano"][f - 1], a["otraMano"][f - 1]
                pecho, cintura = a["pecho"][f - 1], a["cintura"][f - 1]
                # el puno, en una mano; el pecho, en el torso; la cintura, en la pelvis
                self.assertIn(lab[mano[1], mano[0]], ("m", "M"), f"{clave} frame {f}: mano {mano}")
                self.assertIn(lab[otra[1], otra[0]], ("m", "M"), f"{clave} frame {f}: otraMano {otra}")
                self.assertEqual(lab[pecho[1], pecho[0]], "t", f"{clave} frame {f}: pecho {pecho}")
                self.assertEqual(lab[cintura[1], cintura[0]], "p", f"{clave} frame {f}: cintura {cintura}")
                self.assertLess(pecho[1], cintura[1])
                # de frente la mano del arma queda a la izquierda de la imagen y de espaldas a la derecha
                d = t.direccion(f)
                if d == "abajo":
                    self.assertLess(mano[0], otra[0], f"{clave} frame {f}")
                elif d == "arriba":
                    self.assertGreater(mano[0], otra[0], f"{clave} frame {f}")
            # personajes.json lleva las mismas
            guardadas = man["tallas"][clave]["anclas"]
            for p in anclas.PUNTOS:
                self.assertEqual(guardadas[p], " ".join(f"{x},{y}" for x, y in a[p]), f"{clave}.{p}")

    def test_las_armas_van_en_la_mano_en_todas_las_tallas(self):
        import actores_armas as armas
        from actores_materiales import MATERIALES
        for clave, t in self.tallas.items():
            for ident in ("espada_acero", "hacha_hierro", "escudo_madera"):
                ranura, frames = armas.capa_arma(t, ident, MATERIALES)
                for f in range(1, t.n_frames + 1):  # tambien en reposo
                    d = t.direccion(f)
                    mano = armas.MANO[ranura][d]
                    ys, xs = np.nonzero(t.labs[f] == mano)
                    if not len(ys):
                        ys, xs = np.nonzero(t.labs[f] == ("M" if mano == "m" else "m"))
                    ay, ax = np.nonzero(frames[f - 1][..., 3])
                    self.assertTrue(len(ay), f"{clave} {ident} frame {f}: sin dibujo")
                    # el arma pasa por el puno: algun pixel suyo esta pegado a la mano
                    dist = min(abs(int(y) - int(yy)) + abs(int(x) - int(xx))
                               for y, x in zip(ay, ax) for yy, xx in zip(ys, xs))
                    self.assertLessEqual(dist, 1, f"{clave} {ident} frame {f}")

    def test_los_guanteletes_van_en_las_dos_manos(self):
        """SP-02 (Oriol, 07/10: «Guanteletes grandes», uno en cada mano): una pieza de la ranura arma
        que se pinta en los dos punos, con su propio orden de dibujo (encima del cuerpo tambien de
        espaldas). De frente y de espaldas tapa las dos manos; de perfil, la de delante, y la de atras
        solo donde se ve (si no se ve, no hay guantelete atras)."""
        import json

        import actores_armas as armas
        import actores_formas
        from actores_materiales import MATERIALES
        actores_formas.registrar()
        self.assertIsNone(armas.orden_de("f_hacha"))  # la espada no: la Forja la pone delante en unos frames
        parejas = sorted(k for k, v in armas.ARMAS.items() if v[0] in armas.PAREJA)
        self.assertEqual(parejas, ["f_brazos_mecanicos", "f_canon_de_mano", "f_garras", "f_garras_raro",
                                   "f_guanteletes", "f_guanteletes_raro", "f_guantes", "f_guantes_raro",
                                   "f_sable_y_pistola", "f_sable_y_pistola_raro", "f_tambor", "f_tambor_raro",
                                   "f_violin"])
        man = json.loads((TEX / "personajes.json").read_text(encoding="utf-8"))
        forja = set(armas.AJUSTES.get("capas", {})) | set(armas.AJUSTES.get("manos", {}))  # Oriol (23/10): la Forja
        con_orden = sorted(p["id"] for p in man["equipo"]["arma"]["piezas"] if "orden" in p
                           and (p["id"] not in armas.ARMAS or armas.ARMAS[p["id"]][0] not in forja))
        self.assertEqual(con_orden, parejas)
        for ident in parejas:
            orden = armas.orden_de(ident)
            self.assertEqual(set(orden), {"abajo", "arriba", "derecha", "izquierda"})
            self.assertTrue(all(z > man["orden"]["cuerpo"][d] for d, z in orden.items()), ident)
        # CP-01: el sable y la pistola del Forajido y la baqueta y el tambor del Tamborilero no tapan
        # las manos, las cogen (test_las_armas_de_las_clases_propias); INSTRUMENTOS-01: el violin y su arco tampoco
        guanteletes = [p for p in parejas if not p.startswith(("f_sable_y_pistola", "f_tambor", "f_violin"))]
        for ident in guanteletes:
            forma = armas.FORMAS[armas.ARMAS[ident][0]]
            delante = sum(c != "." for fila in forma.get("derecha_2", forma["derecha"]) for c in fila)
            for clave, t in self.tallas.items():
                ranura, frames = armas.capa_arma(t, ident, MATERIALES)
                self.assertEqual(ranura, "arma")
                for f in range(1, t.n_frames + 1):
                    lab, d = t.labs[f], t.direccion(f)
                    pintado = frames[f - 1][..., 3] > 0
                    for mano in ("m", "M"):
                        ys, xs = np.nonzero(lab == mano)
                        if len(ys):  # la mano que se ve, tapada por su guantelete
                            self.assertGreater(pintado[ys, xs].mean(), 0.7, f"{ident} {clave} {f} {mano}")
                    if d in ("derecha", "izquierda") and not (lab == "m").any():
                        # sin la mano de atras a la vista, solo el guantelete de delante
                        self.assertLessEqual(int(pintado.sum()), delante, f"{ident} {clave} {f}")

    def test_las_armas_de_las_clases_propias(self):
        """CP-01 (Oriol, 07/10: «una armadura de tier dos para cada uno»): el arma de cada set de las
        clases propias tiene su forma, con su escalon raro (las de los sets son raras); en la mano, las
        de pose como las de siempre y las que cuelgan a su tamano (un farol no se inclina); todas
        pasan por el puno, y las de las dos manos, por los dos."""
        import actores_armas as armas
        import actores_estetica as est
        import actores_formas
        import actores_poses as poses
        from actores_materiales import MATERIALES
        actores_formas.registrar()
        esperadas = {
            "Estoque del Comediante": "f_estoque", "Laúd del Juglar": "f_laud",
            "Tambor del Tamborilero": "f_tambor", "Bastón del Feriante": "f_baston_paseo",
            "Bastón de Feriante de Viaje": "f_baston_paseo",
            "Hoz de la Espiga Dorada": "f_hoz", "Martillo del Juramento": "f_martillo",
            "Llave del Ingeniero": "f_llave", "Bolsa del Mercader": "f_bolsa",
            "Matraz del Alquimista": "f_matraz", "Naipes del Cartomante": "f_naipes",
            "Tótem del Chamán de los Elementos": "f_totem", "Hacha del Chamán de los Espíritus": "f_hacha_mano",
            "Vara del Tempestario": "f_vara_rayo", "Farol del Necromago": "f_farol_vara",
            "Farol de Ánimas de Viaje": "f_farol_vara", "Sable del Corsario Vampírico": "f_sable",
            "Sable y Pistola del Forajido": "f_sable_y_pistola", "Sable y Pistola de Viaje": "f_sable_y_pistola",
        }
        for nombre, pieza in esperadas.items():
            self.assertEqual(est.forma_de_arma(nombre), pieza, nombre)
            self.assertIn(f"{pieza}_raro", actores_formas.BASES, pieza)
        self.assertEqual(poses.GRUPO_DE["vara_rayo"], "corto")
        self.assertEqual(poses.GRUPO_DE["baston_paseo"], "corto")
        self.assertEqual(poses.GRUPO_DE["totem"], "baston")
        for familia in ("laud", "bolsa", "matraz", "naipes", "farol_vara"):
            self.assertNotIn(familia, poses.GRUPO_DE, familia)
        for ident in ("f_laud_raro", "f_naipes_raro", "f_vara_rayo_raro", "f_baston_paseo_raro", "f_farol_vara_raro",
                      "f_totem_raro",
                      "f_tambor_raro", "f_sable_y_pistola_raro"):
            pareja = armas.ARMAS[ident][0] in armas.PAREJA
            for clave, t in self.tallas.items():
                ranura, frames = armas.capa_arma(t, ident, MATERIALES)
                for f in range(1, t.n_frames + 1):
                    d = t.direccion(f)
                    ay, ax = np.nonzero(frames[f - 1][..., 3])
                    self.assertTrue(len(ay), f"{ident} {clave} frame {f}: sin dibujo")
                    if pareja:
                        manos = [m for m in ("m", "M") if (t.labs[f] == m).any()]
                    else:
                        mano = armas.MANO[ranura][d]
                        manos = [mano if (t.labs[f] == mano).any() else ("M" if mano == "m" else "m")]
                    for mano in manos:
                        ys, xs = np.nonzero(t.labs[f] == mano)
                        dist = min(abs(int(y) - int(yy)) + abs(int(x) - int(xx))
                                   for y, x in zip(ay, ax) for yy, xx in zip(ys, xs))
                        self.assertLessEqual(dist, 1, f"{ident} {clave} frame {f} mano {mano}")

    def test_los_sets_de_la_tercera_iteracion(self):
        """SETS-03 (Oriol, 08/10: los sets «del apostata, senor de las armas, furia de los creadores, bardo de las 4
        estaciones de tier 2, siguiendo los mismos criterios, y el mago de los chasquidos y el monje»): cada pieza
        tiene su dibujo (lo propio, en actores_trajes_iteracion3; lo del catalogo, tenido por el tema del set), el
        arma su forma con su escalon raro si la forma lo tiene, y los guantes del Chasqueador van en las dos manos
        (test_los_guanteletes_van_en_las_dos_manos)."""
        import actores_cascos as cascos
        import actores_equipo as equipo
        import actores_estetica as est
        import actores_formas
        import actores_trajes  # noqa: F401
        import sets_iteracion3
        actores_formas.registrar()
        armas = {"Sable del Apóstata": "f_sable", "Maza de la Furia": "f_maza",
                 "Sable de las Cuatro Estaciones": "f_sable", "Guantes del Chasqueador": "f_guantes",
                 "Guanteletes del Despertar": "f_guanteletes",
                 # SETS-04: los tres sets del Señor de las Armas
                 "Orbe del Duque": "f_orbe", "Orbe del Soberano sin Rostro": "f_orbe",
                 "Orbe del Rey de la Forja": "f_orbe"}
        catalogos = {"cabeza": cascos.CABEZA, "torso": equipo.TORSO, "piernas": equipo.PIERNAS, "pies": equipo.PIES}
        for st in sets_iteracion3.SETS:
            self.assertEqual(sorted(p["slot"] for p in st["set"]), ["Feet", "Head", "Legs", "MainHand", "Torso"])
            for p in st["set"]:
                if p["slot"] == "MainHand":
                    self.assertEqual(est.forma_de_arma(p["name"]), armas[p["name"]], p["name"])
                    continue
                ranura, pieza = est.PIEZA_DE_ROPA[est.normalizar(p["name"])]
                self.assertIn(pieza, catalogos[ranura], p["name"])
        self.assertIn("f_guantes_raro", actores_formas.BASES)
        # el tema del set casa con todas sus piezas (y les da las particulas de su clase)
        for st in sets_iteracion3.SETS:
            for p in st["set"]:
                clave, de_donde = est._tema(est.normalizar(p["name"]), p)
                self.assertEqual(de_donde, "nombre", p["name"])

    def test_los_tres_sets_del_senor_de_las_armas(self):
        """SETS-04 (Oriol, 08/10: «vull modificar estèticament la classe del Senyor de les armes»; de los tres
        esbozos, «estan perfectes els 3»): el Señor de las Armas tiene tres sets (el Duque, el Soberano sin Rostro
        y el Rey de la Forja), iguales en slots, peso y bonos; el de SETS-03 ya no esta en el catalogo ni en los
        atlas; cada objeto se ve con su pieza (y el orbe, con el de su set), las piezas estan en el atlas, y solo
        el yelmo del Soberano tapa el pelo."""
        import json

        import actores_cascos as cascos
        import actores_equipo as equipo
        import actores_trajes  # noqa: F401
        import sets_iteracion3
        from sp_reglas import etiqueta_set, slug
        sets = [st for st in sets_iteracion3.SETS if st["id"] == "senor_de_las_armas"]
        self.assertEqual([etiqueta_set(st["set_de"]) for st in sets],
                         ["Set del Duque", "Set del Soberano sin Rostro", "Set del Rey de la Forja"])

        def firma(st):
            return sorted((p["slot"], p["weightCategory"], tuple(sorted(p["statBonuses"].items()))) for p in st["set"])
        self.assertEqual(firma(sets[0]), firma(sets[1]))
        self.assertEqual(firma(sets[0]), firma(sets[2]))
        raiz = TEX.parent
        catalogo = {e["id"]: e for e in
                    json.loads((raiz / "catalogs/equipment.json").read_text(encoding="utf-8"))["entries"]}
        for nombre in sets_iteracion3.RETIRADOS:
            self.assertNotIn(slug(nombre), catalogo, nombre)
        self.assertNotIn("it3_manto_senor_armas", equipo.TORSO)
        self.assertNotIn("it3_corona_senor_armas", cascos.CABEZA)
        visto = json.loads((TEX / "personajes/equipo_catalogo.json").read_text(encoding="utf-8"))["objetos"]
        man = json.loads((TEX / "personajes.json").read_text(encoding="utf-8"))
        en_atlas = {r: {p["id"] for p in man["equipo"][r]["piezas"]} for r in ("cabeza", "torso", "piernas", "pies")}
        piezas = {"Duque": ("it3_diadema_duque", "it3_casaca_duque", "it3_calzas_duque", "it3_botas_duque"),
                  "Soberano sin Rostro": ("it3_yelmo_soberano", "it3_placas_soberano", "it3_grebas_soberano",
                                          "it3_escarpes_soberano"),
                  "Rey de la Forja": ("it3_corona_forja", "it3_armadura_forja", "it3_grebas_forja", "it3_botas_forja")}
        ranuras = {"Head": "cabeza", "Torso": "torso", "Legs": "piernas", "Feet": "pies"}
        orbes = set()
        for st in sets:
            suyas = dict(zip(("Head", "Torso", "Legs", "Feet"), piezas[st["set_de"]]))
            for p in st["set"]:
                self.assertIn(p["id"], catalogo, p["name"])
                if p["slot"] == "MainHand":
                    self.assertTrue(visto[p["id"]]["pieza"].startswith("f_orbe"), p["name"])
                    orbes.add(tuple(map(tuple, visto[p["id"]]["tinte"])))
                    continue
                self.assertEqual(visto[p["id"]]["pieza"], suyas[p["slot"]], p["name"])
                self.assertIn(suyas[p["slot"]], en_atlas[ranuras[p["slot"]]], p["name"])
                self.assertEqual(visto[p["id"]]["brillo"]["particulas"], "oro", p["name"])  # las de su clase
        self.assertEqual(len(orbes), 3)  # cada orbe con el color de su set
        self.assertTrue(cascos.CABEZA["it3_yelmo_soberano"]["oculta_pelo"])
        self.assertFalse(cascos.CABEZA["it3_diadema_duque"]["oculta_pelo"])
        self.assertFalse(cascos.CABEZA["it3_corona_forja"]["oculta_pelo"])

    def test_los_objetos_de_rol(self):
        """INSTRUMENTOS-01 (Oriol, 08/10: «una variedad de instrumentos musicales? Escobas y otros objetos que no sean
        armas pero podamos usar en un juego de rol»): cada uno tiene su forma en la mano (tools/
        actores_formas_objetos), esta en el catalogo de equipo y en el de la ciudad con su tienda, que nunca es
        la de uno de los tres comerciantes del tutorial (sus listas se quedan como las presenta)."""
        import json

        import actores_armas as armas
        import actores_estetica as est
        import actores_formas
        import objetos_rol
        actores_formas.registrar()
        formas = {"Flauta de madera": "f_flauta", "Flauta de pan": "f_flauta_pan", "Pandereta": "f_pandereta",
                  "Lira": "f_lira", "Trompeta": "f_trompeta", "Gaita": "f_gaita", "Vihuela": "f_vihuela",
                  "Arpa de mano": "f_arpa", "Violín y arco": "f_violin", "Antorcha": "f_tea",
                  "Cuerda de cáñamo": "f_cuerda", "Pala": "f_pala", "Pico de minero": "f_pico",
                  "Caña de pescar": "f_cana", "Catalejo": "f_catalejo", "Escoba": "f_escoba",
                  "Almohada": "f_almohada", "Sartén": "f_sarten", "Jarra de cerveza": "f_jarra_cerveza",
                  "Bolas de malabares": "f_malabares", "Shurikens": "f_shurikens",
                  "Estandarte de batalla": "f_estandarte"}
        self.assertEqual(sorted(o["name"] for o in objetos_rol.TODOS), sorted(formas))
        for o in objetos_rol.TODOS:
            self.assertEqual(est.forma_de_arma(o["name"]), formas[o["name"]], o["name"])
            self.assertIn(formas[o["name"]], armas.ARMAS, o["name"])
        self.assertIn("violin", armas.PAREJA)
        raiz = TEX.parent
        equipo = {e["id"]: e for e in json.loads((raiz / "catalogs/equipment.json").read_text(encoding="utf-8"))["entries"]}
        ciudad = json.loads((raiz / "objects/ciudad_objetos.json").read_text(encoding="utf-8"))["objects"]
        objetos = {o["id"]: o for o in ciudad}
        visto = json.loads((TEX / "personajes/equipo_catalogo.json").read_text(encoding="utf-8"))["objetos"]
        for o in objetos_rol.TODOS:
            self.assertIn(o["id"], equipo, o["name"])
            self.assertIn(o["id"], objetos, o["name"])
            self.assertIn(o["id"], objetos[o["_tienda"]]["shop"]["items"], o["name"])
            self.assertEqual(visto[o["id"]]["pieza"], formas[o["name"]], o["name"])
        tutorial = ("tendero_mercado", "joyero", "sastre")
        for o in objetos_rol.TODOS + objetos_rol.MOCHILA:
            self.assertNotIn(o.get("_tienda") or o.get("tienda"), tutorial, o["name"])
        self.assertEqual(objetos["tendero_mercado"]["shop"]["items"], ["pocion", "eter", "pan"])
        for m in objetos_rol.MOCHILA:
            self.assertIn(m["id"], objetos)
            self.assertNotIn(m["id"], equipo)
            self.assertIn(m["id"], objetos[m["tienda"]]["shop"]["items"])

    def test_ningun_arma_se_sale_de_la_celda(self):
        """ESTETICA-01: pintar_arma corta lo que se sale de la celda; las hojas largas se
        acortan por talla (forma_en_talla) y la punta se ve siempre."""
        import actores_armas as armas
        import actores_formas
        import actores_oficios  # noqa: F401 (las piezas de los PNJ)
        import actores_poses as poses
        actores_formas.registrar()
        for clave, t in self.tallas.items():
            for ident, valor in armas.ARMAS.items():
                if any(valor[0] in armas.AJUSTES.get(k, {}) for k in ("dibujos", "desplazamientos", "giros")):
                    continue  # Oriol (23/10): lo dibujado o movido en la Forja se recorta al pintar (capa_arma)
                fuera = armas.desborde_en_talla(t, valor[0], armas.ranura_de(ident))
                self.assertEqual(fuera, (0, 0, 0), f"{ident} en talla {clave}: se sale (arriba, abajo, lados)")

    def test_acortar_quita_tramos_de_la_hoja_y_deja_la_punta(self):
        import actores_armas as armas
        hoja = ["..da..", "..@7..", "dabbad"] + [".4123."] * 8 + ["..43..", "..4..."]
        corta = armas._acortar_abajo(hoja, 3, 1)
        self.assertEqual(len(corta), len(hoja) - 3)
        self.assertEqual(corta[:3], hoja[:3])      # pomo, puno y guarda
        self.assertEqual(corta[-2:], hoja[-2:])    # la punta
        # de perfil (corrida una columna cada dos filas) sigue recta al quitar un paso
        perfil = ["@7....", "dbd...", ".412..", ".412..", "..412.", "..412.", "...412", "...412", "...4.."]
        perfil = [f + "...." for f in perfil]
        corta = armas._acortar_abajo(perfil, 2, 0)
        self.assertEqual(len(corta), len(perfil) - 2)
        self.assertEqual([f.strip(".") for f in corta[2:]], ["412"] * 4 + ["4"])
        desplazamientos = [len(f) - len(f.lstrip(".")) for f in corta[2:6]]
        self.assertEqual(desplazamientos, [1, 1, 2, 2])

    def test_veinte_frames_doce_de_andar_y_ocho_de_reposo(self):
        """ESTETICA-01: el reposo (13-20) sale del quieto de cada direccion; el segundo de
        cada par respira (lo de encima de las piernas baja un pixel)."""
        import actores_partes as partes
        for clave, t in self.tallas.items():
            self.assertEqual(t.n_frames, 20, clave)
            for f, q in partes.REPOSO.items():
                self.assertEqual(t.direccion(f), t.direccion(q))
                if partes.paso_de(f) == "reposo1":
                    self.assertTrue(np.array_equal(t.frames[f - 1], t.frames[q - 1]), f"{clave} {f}")
                else:
                    self.assertEqual(t.top[f], t.top[q] + 1, f"{clave} {f}")
        self.assertEqual([partes.paso_de(f) for f in (1, 2, 3, 13, 14)],
                         ["A", "quieto", "B", "reposo1", "reposo2"])

    def _arma(self, familia, dibujo, vista):
        """(filas, fila del puno, columna del puno) del arma de esa familia en la talla media."""
        import actores_armas as armas
        import actores_formas
        actores_formas.registrar()
        filas = armas.formas_en_talla(self.tallas["medio"], familia, "arma")[dibujo][vista]
        ay = next(i for i, f in enumerate(filas) if any(c in f for c in armas.ANCLAS))
        ax = next(filas[ay].index(c) for c in armas.ANCLAS if c in filas[ay])
        return filas, ay, ax

    @staticmethod
    def _caja(filas, ay, ax):
        """(arriba, abajo, izquierda, derecha): hasta donde llega el arma desde el puno."""
        pint = [(i, j) for i, f in enumerate(filas) for j, c in enumerate(f) if c != "."]
        return (ay - min(i for i, _ in pint), max(i for i, _ in pint) - ay,
                ax - min(j for _, j in pint), max(j for _, j in pint) - ax)

    def test_cada_arma_se_coge_como_en_nostale(self):
        """ESTETICA-01 (Oriol, 07/10): como se coge cada tipo, quieto y andando. De frente la
        mano derecha cae a la izquierda de la imagen (fuera = izquierda); de perfil el
        personaje mira a la derecha (delante = derecha)."""
        # la espada quieta, baja y en diagonal hacia fuera; andando de perfil, detras
        arr, aba, izq, der = self._caja(*self._arma("espada", "reposo1", "abajo"))
        self.assertGreater(aba, arr)
        self.assertGreater(izq, der)
        arr, aba, izq, der = self._caja(*self._arma("espada", "quieto", "derecha"))
        self.assertGreater(izq, 2 * der)  # la hoja atras
        self.assertGreater(izq, aba)      # casi horizontal
        # la daga con agarre invertido: de frente la hoja sube; de perfil, hacia atras
        arr, aba, izq, der = self._caja(*self._arma("daga", "reposo1", "abajo"))
        self.assertGreater(arr, aba)
        arr, aba, izq, der = self._caja(*self._arma("daga", "reposo1", "derecha"))
        self.assertGreater(izq, der)
        # el espadon quieto, con la punta arriba; andando, detras con la punta hacia abajo
        arr, aba, izq, der = self._caja(*self._arma("mandoble", "reposo1", "abajo"))
        self.assertGreater(arr, 2 * aba)
        arr, aba, izq, der = self._caja(*self._arma("mandoble", "quieto", "derecha"))
        self.assertGreater(izq, der)
        self.assertGreater(aba, 0)
        # las de asta, nunca rectas: la lanza tendida (punta fuera de frente, delante de perfil)
        for familia in ("lanza", "pica", "tridente", "u_astil_del_rayo_escrito"):
            for dibujo in ("reposo1", "A", "quieto", "B"):
                filas, ay, ax = self._arma(familia, dibujo, "abajo")
                arr, aba, izq, der = self._caja(filas, ay, ax)
                self.assertGreater(izq + der, 2 * (arr + aba), f"{familia} {dibujo}")
                self.assertGreater(izq, der, f"{familia} {dibujo}")
            arr, aba, izq, der = self._caja(*self._arma(familia, "reposo1", "derecha"))
            self.assertGreater(der, izq, familia)
        # el baston, inclinado: la cabeza arriba y hacia fuera
        filas, ay, ax = self._arma("baston", "reposo1", "abajo")
        arriba = filas[0]
        self.assertLess(len(arriba) - len(arriba.lstrip(".")) + 2, ax)
        # el arco con la cuerda hacia atras: de perfil, la madera (el puno) delante
        filas, ay, ax = self._arma("arco", "reposo1", "derecha")
        arr, aba, izq, der = self._caja(filas, ay, ax)
        self.assertGreater(izq, der)
        # la guadana cruzada: la hoja abajo y fuera, el astil sube hacia dentro
        arr, aba, izq, der = self._caja(*self._arma("guadana", "reposo1", "abajo"))
        self.assertGreater(aba, 0)
        self.assertGreater(izq, 0)
        self.assertGreater(der, 0)
        # las hachas de una mano (07/10, con volumen y hoja de barba): la cabeza arriba, junto al
        # hombro y hacia fuera
        for familia in ("hacha", "hacha_mano", "u_hacha_del_cuerno_roto"):
            arr, aba, izq, der = self._caja(*self._arma(familia, "reposo1", "abajo"))
            self.assertGreater(arr, aba, familia)
            self.assertGreater(izq, der, familia)
        # gran hacha y martillo (07/10): quietos, de frente cruzados hacia el hombro contrario
        # (arriba y hacia dentro, a la derecha de la imagen); de perfil al hombro, la cabeza
        # detras; andando de perfil, caidos hacia atras y abajo, como el espadon
        for familia in ("gran_hacha", "martillo"):
            arr, aba, izq, der = self._caja(*self._arma(familia, "reposo1", "abajo"))
            self.assertGreater(arr, aba, familia)
            self.assertGreater(der, izq, familia)
            arr, aba, izq, der = self._caja(*self._arma(familia, "reposo1", "derecha"))
            self.assertGreater(arr, aba, familia)
            self.assertGreater(izq, der, familia)
            arr, aba, izq, der = self._caja(*self._arma(familia, "quieto", "derecha"))
            self.assertGreater(aba, arr, familia)
            self.assertGreater(izq, der, familia)
        # el latigo (07/10, opcion B): la tralla cuelga del puno hasta el suelo
        arr, aba, izq, der = self._caja(*self._arma("latigo", "reposo1", "abajo"))
        self.assertGreater(aba, 3 * arr)
        # SP-02 (el Arquero): la escopeta, el rifle y el lanzagranadas horizontales a la cintura,
        # como la ballesta y el lanzacohetes de NosTale (de frente la boca hacia fuera, de perfil
        # hacia delante); los revolveres abajo junto a la pierna; el canon al hombro (de frente
        # cruzado hacia el hombro contrario; de perfil la boca detras, tambien andando: no se
        # arrastra), y el bumeran en alto junto al costado, hacia fuera
        # (y el lanzallamas de la Tormenta de fuego, SP-02 el Mago, como la escopeta)
        for familia in ("escopeta", "rifle", "lanzagranadas", "escopeta_raro", "lanzallamas", "lanzallamas_raro"):
            arr, aba, izq, der = self._caja(*self._arma(familia, "reposo1", "abajo"))
            self.assertGreater(izq + der, 2 * (arr + aba), familia)
            self.assertGreater(izq, der, familia)
            arr, aba, izq, der = self._caja(*self._arma(familia, "reposo1", "derecha"))
            self.assertGreater(der, izq, familia)
        # (y la pistola magica del Mago de las tinieblas y de Merlin, como el revolver)
        for familia in ("revolver", "revolver_raro", "pistola", "pistola_raro"):
            for vista in ("abajo", "derecha"):
                arr, aba, izq, der = self._caja(*self._arma(familia, "reposo1", vista))
                self.assertGreater(aba, 2 * arr, f"{familia} {vista}")
        for familia in ("canon", "canon_raro"):
            arr, aba, izq, der = self._caja(*self._arma(familia, "reposo1", "abajo"))
            self.assertGreater(arr, aba, familia)
            self.assertGreater(der, izq, familia)
            for dibujo in ("reposo1", "quieto"):
                arr, aba, izq, der = self._caja(*self._arma(familia, dibujo, "derecha"))
                self.assertGreater(arr, aba, f"{familia} {dibujo}")
                self.assertGreater(izq, der, f"{familia} {dibujo}")
        arr, aba, izq, der = self._caja(*self._arma("bumeran", "reposo1", "abajo"))
        self.assertGreater(arr, aba)
        self.assertGreater(izq, der)

    def test_el_arco_no_lleva_cuerda(self):
        """Oriol (07/10): «el arco si no le dibujas la cuerda, mejor». Del material 2 (la
        cuerda) solo queda el puno."""
        for familia in ("arco", "arco_epico", "u_arco_de_la_caceria_interminable"):
            for vista in ("abajo", "derecha"):
                filas, ay, ax = self._arma(familia, "reposo1", vista)
                lejos = [(i, j) for i, f in enumerate(filas) for j, c in enumerate(f)
                         if c in "6789" and (abs(i - ay) > 1 or abs(j - ax) > 1)]
                self.assertEqual(lejos, [], f"{familia} {vista}")

    def test_ningun_arma_tapa_la_cara(self):
        """ESTETICA-01 (Oriol, 07/10): «en las animaciones hay que evitar siempre tapar la
        cara del jugador». Donde el arma se pinta encima del cuerpo, ningun pixel suyo cae
        en la cara (la cabeza sin la coronilla; de perfil, su mitad de delante), en ninguna
        talla ni frame. Lo que la cabeza tapa (TAPA_LA_CABEZA: los bastones de frente, solo el
        trozo justo) y lo que el arma tiene por detras del cuerpo (actores_volumen) no cuenta:
        se mira en la capa pintada."""
        import actores_armas as armas
        import actores_formas
        import actores_oficios  # noqa: F401 (las piezas de los PNJ)
        import actores_poses as poses
        actores_formas.registrar()
        for clave, t in self.tallas.items():
            for ranura in ("arma", "mano_izq"):
                caras = armas._caras_por_dibujo(t, ranura)
                for ident, valor in armas.ARMAS.items():
                    if armas.ranura_de(ident) != ranura:
                        continue
                    if poses.tapada_por_la_cabeza(valor[0], "abajo"):
                        # de frente la cabeza lo tapa donde se cruzan: se mira la capa ya pintada
                        self.assertEqual(armas.pixeles_en_la_cara(t, ident), 0,
                                         f"{ident} en talla {clave}: tapa la cara")
                        continue
                    formas = armas.formas_en_talla(t, valor[0], ranura)
                    for (dibujo, vista), lista in caras.items():
                        self.assertEqual(armas.tapa_cara(formas[dibujo][vista], lista), 0,
                                         f"{ident} en talla {clave}: tapa la cara ({dibujo}, {vista})")

    def test_el_arma_no_cambia_de_largo_al_andar(self):
        """Los tres dibujos de andar salen de la misma maestra acortada: la misma cantidad
        de pixeles de hoja en el paso A, el quieto y el paso B."""
        for familia in ("espada", "mandoble", "lanza", "baston", "u_astil_del_rayo_escrito"):
            for vista in ("abajo", "derecha"):
                cuantos = {sum(c != "." for f in self._arma(familia, d, vista)[0] for c in f)
                           for d in ("A", "quieto", "B")}
                self.assertLessEqual(max(cuantos) - min(cuantos), max(cuantos) // 10,
                                     f"{familia} {vista}: {cuantos}")

    def test_la_capa_va_detras_del_cuerpo_de_frente_y_encima_de_espaldas(self):
        import actores_capas as capas
        import actores_equipo as equipo
        from actores_materiales import MATERIALES
        t = self.tallas["medio"]
        torso = equipo.capa_pieza(t, "torso", "tunica_verde", MATERIALES)
        capa = equipo.capa_pieza(t, "capa", "capa_roja", MATERIALES)
        frames = capas.componer([("torso", torso), ("capa", capa)])
        de_frente = torso[7][..., 3] > 0
        self.assertTrue(np.array_equal(frames[7][de_frente], torso[7][de_frente]))
        de_espaldas = capa[1][..., 3] > 0
        self.assertTrue(np.array_equal(frames[1][de_espaldas], capa[1][de_espaldas]))

    def test_las_43_razas_tienen_equipo_y_sus_rasgos_existen(self):
        import actores_personajes as pers
        import actores_rasgos as rasgos
        import actores_razas as razas
        ids = [r["id"] for r in razas.RAZAS]
        self.assertEqual(len(ids), 72)  # 45 + las 27 del 2000 b.f. (Oriol, 23/10)
        self.assertEqual(len(set(ids)), 72)
        self.assertEqual(set(pers.EQUIPO_RAZAS), set(ids))
        for r in razas.RAZAS:
            for _ranura, nombre, _m1, _m2 in r.get("rasgos", []):
                self.assertTrue(hasattr(rasgos, nombre), f"{r['id']}: {nombre}")

    def test_otra_piel_solo_en_razas_sin_rasgos(self):
        import actores_personajes as pers
        pers.capas_personaje({"raza": "race_human_marches", "piel": "piel_oscura",
                              "talla": "grande"}, self.tallas)
        with self.assertRaises(ValueError):
            pers.capas_personaje({"raza": "minotauros_de_la_llanura", "talla": "medio"}, self.tallas)

    def test_reglas_del_catalogo_de_objetos(self):
        import actores_catalogo as cat
        casos = {("Cota de malla", "Torso"): ("torso", "cota_malla"),
                 ("Faldon de malla", "Legs"): ("piernas", "grebas_malla"),
                 ("Botas claras", "Feet"): ("pies", "botas_claras"),
                 ("Espada de soldado", "MainHand"): ("arma", "espada_hierro"),
                 ("Escudo de roble", "MainHand"): ("mano_izq", "escudo_madera"),
                 ("Capa roja", "Torso"): ("capa", "capa_roja"),
                 ("Anillo de hierro", "Head"): None}
        for (nombre, slot), esperado in casos.items():
            self.assertEqual(cat.pieza_de(nombre, slot), esperado, nombre)


@unittest.skipUnless((TEX / "alas_moral.json").is_file(), "sin el atlas de alas de MORAL-02")
class AlasMoralTests(unittest.TestCase):
    """MORAL-02: el atlas de las alas de la moral casa con lo que lee el motor (Game/AuraMoral.h), sale tal cual
    del generador y crece de nivel en nivel por pisos."""

    @classmethod
    def setUpClass(cls):
        import gen_alas_moral
        cls.gen = gen_alas_moral
        cls.contrato = json.loads((TEX / "alas_moral.json").read_text(encoding="utf-8"))
        cls.img = np.array(Image.open(TEX / "alas_moral.png").convert("RGBA"))
        cabecera = (HERE.parent / "include/Game/AuraMoral.h").read_text(encoding="utf-8")
        import re
        cls.cpp = {m.group(1): int(m.group(2))
                   for m in re.finditer(r"constexpr int (k\w+) = (\d+);", cabecera)}
        cpp_hojas = (HERE.parent / "src/Game/AuraMoral.cpp").read_text(encoding="utf-8")
        cls.cpp_hojas = [int(x) for x in re.search(r"kHojas\[kNivelesAla\] = \{([^}]*)\}", cpp_hojas)
                         .group(1).split(",")]

    def test_el_contrato_casa_con_el_motor(self):
        c, k = self.contrato, self.cpp
        self.assertEqual(c["celda"], k["kCeldaAla"])
        self.assertEqual(c["pivote"], [k["kPivoteAlaX"], k["kPivoteAlaY"]])
        self.assertEqual(c["niveles"], k["kNivelesAla"])
        self.assertEqual(c["frames"], k["kFramesAla"])
        self.assertEqual(c["lados"], ["luz", "sombra"])
        self.assertEqual([c["hojasPorNivel"][str(n)] for n in range(1, c["niveles"] + 1)], self.cpp_hojas)

    def test_el_atlas_mide_lo_que_dice(self):
        c = self.contrato
        self.assertEqual(self.img.shape, (2 * c["niveles"] * c["celda"], c["frames"] * c["celda"], 4))

    def test_sale_tal_cual_del_generador(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.gen.generar(tmp)
            for nombre in ("alas_moral.png", "alas_moral.json"):
                self.assertEqual(_huella(Path(tmp) / nombre), _huella(TEX / nombre), nombre)

    def test_cada_celda_lleva_su_ala_sin_cortar(self):
        c = self.contrato["celda"]
        px, py = self.contrato["pivote"]
        for fila in range(2 * self.contrato["niveles"]):
            for col in range(self.contrato["frames"]):
                a = self.img[fila * c:(fila + 1) * c, col * c:(col + 1) * c, 3]
                self.assertGreater(int((a > 0).sum()), 20, (fila, col))
                # nada pegado al borde (el ala no se sale de la celda) y el hombro, dentro del ala
                borde = np.concatenate([a[0], a[-1], a[:, 0], a[:, -1]])
                self.assertEqual(int((borde > 0).sum()), 0, (fila, col))
                self.assertGreater(int(a[py - 2:py + 3, px:px + 4].max()), 0, (fila, col))

    def test_crece_de_nivel_en_nivel_por_pisos(self):
        g = self.gen
        pisos = [g.pisos(n) for n in range(1, g.NIVELES + 1)]
        self.assertEqual(pisos, [1, 1, 2, 2, 3, 3, 3, 3])
        largos = [g.NIVEL[n][0] for n in range(1, g.NIVELES + 1)]
        self.assertEqual(largos, sorted(set(largos)))  # cada nivel, más largas
        c = self.contrato["celda"]
        for lado in range(2):  # y ocupan más
            area = [int((self.img[(lado * 8 + n) * c:(lado * 8 + n + 1) * c, 0:c, 3] > 0).sum()) for n in range(8)]
            self.assertEqual(area, sorted(area), lado)


if __name__ == "__main__":
    unittest.main(verbosity=2)
