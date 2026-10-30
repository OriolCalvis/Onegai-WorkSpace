#!/usr/bin/env python3
"""Pruebas de tools/capa_ojos_iris.py (CAR-2), sobre los atlas de assets/textures. Sin ventana:
python3 tools/test_capa_ojos_iris.py   (~1 min)"""
import hashlib
import json
import os
import sys
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

AQUI = Path(__file__).resolve().parent
sys.path.insert(0, str(AQUI))
import capa_ojos_iris as CI  # noqa: E402
import laminas_desde_capas as LC  # noqa: E402

TEX = Path(os.environ.get("CAR2_TEX") or AQUI.parent / "assets" / "textures")  # CAR2_TEX: atlas de prueba (mutaciones)
MAN = json.loads((TEX / "personajes.json").read_text(encoding="utf-8"))
RAZAS = {r["id"]: r for r in MAN["razas"]["lista"]}
COLORES = [f["id"] for f in MAN["ojos"]["filas"]]
HUMANOIDES = [r for r in MAN["razas"]["lista"] if r["talla"] in CI.TALLAS]
CON_OJOS = [r for r in HUMANOIDES if "ojos" not in r.get("sin", [])]
SIN_OJOS = [r for r in HUMANOIDES if "ojos" in r.get("sin", [])]


# CAR-2c: iris azul claro con un pixel marron (drow_explorador): el detector da «marron» pero la etiqueta es la de siempre
AMBIGUAS = {"drow_explorador"}


def atlas(ruta, filas):
    return CI.filas_de(np.asarray(Image.open(TEX / ruta).convert("RGBA")), filas)


def hash_fila(f):
    return hashlib.sha1(np.ascontiguousarray(f).tobytes()).hexdigest()


def mascara_de(r):
    """La mascara de iris que sale del cuerpo de la raza (el mismo metodo que el generador)."""
    familia = atlas(MAN["cuerpos"]["atlas"][r["talla"]], len(MAN["cuerpos"]["filas"]))[0]
    cuerpo = atlas(MAN["razas"]["atlas"]["cuerpo"], MAN["razas"]["filas"])[r["fila"] - 1]
    return CI.detectar(cuerpo, CI.coronillas(familia))


def defectos_de_iris(cuerpo, cor, pintado):
    """CAR-2c: los pixeles pintados (KF, L, L) que no pueden ser iris: (frame, y, x, motivo). Criterio del validador:
    el iris son los colores que NO salen en las vistas de espalda del mismo cuerpo (dientes, pelo y cuernos se ven
    por detras; los ojos no); ademas, nada fuera de la banda de ojos (fila de ojos -1 .. +3 de cada frame)."""
    detras = [f for f in range(CI.KF) if f + 1 not in CI.FRAMES_OJOS]
    vistos = set()
    for f in detras:
        op = cuerpo[f][cuerpo[f][..., 3] > 0][:, :3]
        vistos.update(map(tuple, np.unique(op, axis=0).tolist()))
    out = []
    for f in range(CI.KF):
        ys, xs = np.nonzero(pintado[f])
        if not len(ys):
            continue
        if f + 1 not in CI.FRAMES_OJOS:
            out += [(f + 1, int(y), int(x), "ojos de espaldas") for y, x in zip(ys, xs)]
            continue
        fila = int(cor[f]) + CI.OJOS_BAJO_CORONILLA
        for y, x in zip(ys.tolist(), xs.tolist()):
            if not fila + CI.BANDA[0] <= y < fila + CI.BANDA[1]:
                out.append((f + 1, y, x, "fuera de la banda de ojos"))
            elif tuple(cuerpo[f][y, x][:3].tolist()) in vistos:
                out.append((f + 1, y, x, "color que sale de espaldas"))
    return out


class PruebaOjos(unittest.TestCase):
    def test_hay_razas_con_y_sin_ojos(self):
        self.assertGreater(len(CON_OJOS), 20)  # CAR-2e: 26
        self.assertGreater(len(SIN_OJOS), 5)
        for r in CON_OJOS:
            self.assertIn(r["id"], MAN["ojos"]["atlas"], r["id"])
        for r in SIN_OJOS:
            self.assertNotIn(r["id"], MAN["ojos"]["atlas"], r["id"])
        # CAR-1: las no humanoides siguen sin ojos
        for r in MAN["razas"]["lista"]:
            if r["talla"] in ("minotauro", "aarakocra", "naga", "espectro"):
                self.assertIn("ojos", r["sin"], r["id"])

    def test_las_diez_filas_de_color_son_distintas(self):
        rutas = [MAN["ojos"]["atlas"][t] for t in CI.TALLAS if t in MAN["ojos"]["atlas"]] + [MAN["ojos"]["atlas"][r["id"]] for r in CON_OJOS]
        for ruta in rutas:
            filas = atlas(ruta, len(COLORES))
            self.assertEqual(len({hash_fila(f) for f in filas}), len(COLORES), ruta)

    def test_la_capa_solo_pinta_la_mascara_de_iris(self):
        for r in CON_OJOS:
            m = mascara_de(r)
            self.assertTrue(CI.admite(m), r["id"])
            filas = atlas(MAN["ojos"]["atlas"][r["id"]], len(COLORES))
            for i, f in enumerate(filas):
                pintado = f[..., 3] > 0
                self.assertTrue((pintado == m).all(), f"{r['id']} fila {i + 1}: pinta fuera de la mascara o le falta")
                self.assertTrue((f[..., 3][pintado] == 255).all())
            # solo los frames de frente llevan ojos, y TODOS los llevan (CAR-2e); de espaldas, nada
            for fr in range(1, CI.KF + 1):
                if fr in CI.FRAMES_OJOS:
                    self.assertTrue(m[fr - 1].any(), f"{r['id']} frame {fr}: sin iris")
                else:
                    self.assertFalse(m[fr - 1].any(), f"{r['id']} frame {fr}")

    def test_el_compuesto_solo_cambia_pixeles_dentro_de_la_mascara(self):
        M = LC.Montaje(TEX)
        muestra = ["race_human_marches", "elfos_de_las_estrellas", "elfos_del_bosque", "enanos_forjadores",
                   "gnomos_de_la_isla", "oni_de_himetsumota"]
        for rid in muestra:
            r = RAZAS[rid]
            self.assertIn(rid, MAN["ojos"]["atlas"])  # tiene capa propia
            m = mascara_de(r)
            base = M.montar({"raza": rid})
            for color in [c for c in ("rojo", "verde", "violeta") if c != r["ojos"]]:
                con = M.montar({"raza": rid, "ojos": color})
                cambio = np.any(base != con, axis=-1)  # (64, 64*40)
                for fr in range(1, CI.KF + 1):
                    sub = cambio[:, (fr - 1) * 64: fr * 64]
                    self.assertFalse((sub & ~m[fr - 1]).any(), f"{rid} {color} frame {fr}: cambia fuera del iris")
                self.assertTrue(cambio.any(), f"{rid} {color}: no cambia nada")
            # y los colores se ven distintos entre si
            a, b = M.montar({"raza": rid, "ojos": "rojo"}), M.montar({"raza": rid, "ojos": "violeta"})
            self.assertTrue(np.any(a != b), rid)

    def test_cuerpo_liso_usa_la_capa_de_la_talla(self):
        M = LC.Montaje(TEX)
        for talla in [t for t in CI.TALLAS if t in MAN["ojos"]["atlas"]]:
            familia = atlas(MAN["cuerpos"]["atlas"][talla], len(MAN["cuerpos"]["filas"]))[0]
            capa = atlas(MAN["ojos"]["atlas"][talla], len(COLORES))[COLORES.index("rojo")]
            self.assertTrue(((capa[..., 3] > 0) <= (familia[..., 3] > 0)).all(), talla)
            # con cuerpo liso: otra talla, o (en la suya) otra piel
            comp = {"raza": "race_human_marches", "talla": talla} if talla != "medio" else \
                {"raza": "race_human_marches", "piel": "piel_morena"}
            base = M.montar(comp)
            con = M.montar({**comp, "ojos": "rojo"})
            cambio = np.any(base != con, axis=-1)
            self.assertTrue(cambio.any(), talla)
            for fr in range(1, CI.KF + 1):
                self.assertFalse((cambio[:, (fr - 1) * 64: fr * 64] & ~(capa[fr - 1][..., 3] > 0)).any(), f"{talla} {fr}")

    def test_una_raza_sin_ojos_no_pinta_ninguno(self):
        M = LC.Montaje(TEX)
        for rid in ("revenants", "elementales_de_agua", "nagas_arcanos", "minotauros_de_la_llanura"):
            base = M.montar({"raza": rid})
            con = M.montar({"raza": rid, "ojos": "rojo"})
            self.assertTrue((base == con).all(), rid)

    def test_cada_capa_escrita_pinta_solo_iris_y_nunca_piel(self):
        """CAR-2b: de toda capa escrita (raza y talla), cada pixel pintado es de la mascara de iris de ese frame,
        cae sobre un pixel del cuerpo que no es de la paleta de piel ni uno de los 3 colores mas comunes del
        frame, y la mascara pasa `admite` (una capa que no lo pasa no se escribe)."""
        casos = []
        for talla in CI.TALLAS:
            familia = atlas(MAN["cuerpos"]["atlas"][talla], len(MAN["cuerpos"]["filas"]))[0]
            cor = CI.coronillas(familia)
            if talla in MAN["ojos"]["atlas"]:
                casos.append((talla, MAN["ojos"]["atlas"][talla], familia, cor, CI.mascara_familia(familia, cor)))
            else:  # sin capa de talla: declarada, sin archivo
                self.assertIn(talla, CI.TALLAS_SIN_OJOS)
                self.assertFalse((TEX / "personajes" / "cuerpos" / f"ojos_{talla}.png").exists(), talla)
                self.assertFalse(CI.admite_familia(CI.mascara_familia(familia, cor)), talla)
        for r in CON_OJOS:
            familia = atlas(MAN["cuerpos"]["atlas"][r["talla"]], len(MAN["cuerpos"]["filas"]))[0]
            cuerpo = atlas(MAN["razas"]["atlas"]["cuerpo"], MAN["razas"]["filas"])[r["fila"] - 1]
            cor = CI.coronillas(familia)
            casos.append((r["id"], MAN["ojos"]["atlas"][r["id"]], cuerpo, cor, CI.detectar(cuerpo, cor)))
        for nombre, ruta, cuerpo, cor, m in casos:
            self.assertTrue(CI.admite(m), f"{nombre}: la mascara no pasa admite y aun asi hay capa")
            pal = CI.paleta_piel(cuerpo, cor)
            for i, f in enumerate(atlas(ruta, len(COLORES))):
                pintado = f[..., 3] > 0
                for fr in range(CI.KF):
                    fuera = pintado[fr] & ~m[fr]
                    self.assertFalse(fuera.any(), f"{nombre} fila {i + 1} frame {fr + 1}: pinta fuera del iris")
            for fr in range(CI.KF):
                ys, xs = np.nonzero(m[fr])
                if not len(ys):
                    continue
                px = cuerpo[fr][ys, xs][:, :3].astype(int)
                if len(pal):
                    d = np.abs(px[:, None, :] - pal[None]).sum(-1).min(-1)
                    self.assertTrue((d >= CI.TOL_PALETA).all(), f"{nombre} frame {fr + 1}: pinta sobre piel")
                opacos = cuerpo[fr][cuerpo[fr][..., 3] > 0][:, :3]
                cols, n = np.unique(opacos, axis=0, return_counts=True)
                comunes = {tuple(c) for c in cols[np.argsort(-n)[:3]].tolist()}
                for p_ in px.tolist():
                    self.assertNotIn(tuple(p_), comunes, f"{nombre} frame {fr + 1}: pinta sobre un color de piel")

    def test_la_etiqueta_de_color_propio_es_el_color_del_iris(self):
        """CAR-2b: el color propio de cada raza (el que no pinta capa) es el de la rampa OJOS mas cercano al iris
        dibujado; si el detector lo ve ambiguo (amarillo/dorado/ambar...) se deja como esta."""
        sin_ambiguas = 0
        for r in CON_OJOS:
            if r["id"] in AMBIGUAS:
                continue
            familia = atlas(MAN["cuerpos"]["atlas"][r["talla"]], len(MAN["cuerpos"]["filas"]))[0]
            cuerpo = atlas(MAN["razas"]["atlas"]["cuerpo"], MAN["razas"]["filas"])[r["fila"] - 1]
            color, ambiguo = CI.color_del_iris(cuerpo, CI.detectar(cuerpo, CI.coronillas(familia)))
            if not ambiguo:
                sin_ambiguas += 1
                self.assertEqual(r["ojos"], color, r["id"])
        self.assertGreater(sin_ambiguas, 10)  # CAR-2e: 14

    def test_ninguna_capa_de_raza_pinta_colores_que_salen_de_espaldas(self):
        """CAR-2c: dientes, colmillos o mechones (que se ven tambien por detras) no son iris. Cada pixel pintado de
        una capa de raza cae en la banda de ojos y su color no sale en las vistas de espalda del mismo cuerpo."""
        for r in CON_OJOS:
            familia = atlas(MAN["cuerpos"]["atlas"][r["talla"]], len(MAN["cuerpos"]["filas"]))[0]
            cuerpo = atlas(MAN["razas"]["atlas"]["cuerpo"], MAN["razas"]["filas"])[r["fila"] - 1]
            cor = CI.coronillas(familia)
            for i, f in enumerate(atlas(MAN["ojos"]["atlas"][r["id"]], len(COLORES))):
                mal = defectos_de_iris(cuerpo, cor, f[..., 3] > 0)
                self.assertEqual(mal, [], f"{r['id']} fila {i + 1}: {mal[:4]}")

    def test_el_criterio_de_espaldas_ve_las_capas_de_dientes(self):
        """CAR-2c: las razas movidas a SIN_A_MANO tenian una mascara automatica que pasaba `admite` pero no
        pintaba el iris: con ella el criterio de espaldas tiene que fallar (y ya no tienen capa)."""
        for rid in ("trasgos_de_gliaddokx", "hombres_lagarto_del_templo_profundo", "drow_de_los_vientos",
                    "draconidos_de_fuego", "hombres_lagarto_de_la_selva", "hombres_lagarto_del_pantano",
                    "draconidos_de_obsidiana"):
            r = RAZAS[rid]
            self.assertIn(rid, CI.SIN_A_MANO)
            self.assertIn("ojos", r["sin"], rid)
            self.assertNotIn(rid, MAN["ojos"]["atlas"], rid)
            self.assertFalse((TEX / "personajes" / "ojos" / f"ojos_{rid}.png").exists(), rid)
            familia = atlas(MAN["cuerpos"]["atlas"][r["talla"]], len(MAN["cuerpos"]["filas"]))[0]
            cuerpo = atlas(MAN["razas"]["atlas"]["cuerpo"], MAN["razas"]["filas"])[r["fila"] - 1]
            cor = CI.coronillas(familia)
            m = CI.detectar(cuerpo, cor)
            if rid != "draconidos_de_obsidiana":  # (CAR-2d: a esa le faltan los frames SE sin el parpado)
                self.assertTrue(CI.admite(m), rid)  # el metodo sigue dandola por buena...
            self.assertTrue(defectos_de_iris(cuerpo, cor, m), rid)  # ...y el criterio de espaldas no

    def test_ningun_pixel_pintado_de_una_capa_de_raza_es_piel_o_parpado(self):
        """CAR-2d: el propio `color_del_iris` descarta la piel/parpado naranja (b36243); la capa no debe pintarlo.
        Cada pixel pintado de cada capa de raza cuenta como iris para `color_del_iris` (no como piel)."""
        self.assertTrue(CI.es_piel_naranja((0xb3, 0x62, 0x43)))
        for r in CON_OJOS:
            cuerpo = atlas(MAN["razas"]["atlas"]["cuerpo"], MAN["razas"]["filas"])[r["fila"] - 1]
            for i, f in enumerate(atlas(MAN["ojos"]["atlas"][r["id"]], len(COLORES))):
                for fr in range(CI.KF):
                    ys, xs = np.nonzero(f[fr][..., 3] > 0)
                    for y, x in zip(ys.tolist(), xs.tolist()):
                        px = tuple(int(c) for c in cuerpo[fr][y, x][:3])
                        self.assertFalse(CI.es_piel_naranja(px),
                                         f"{r['id']} fila {i + 1} frame {fr + 1} ({x},{y}): {px} es piel/parpado")
                if i == 0:  # los pixeles pintados son los mismos en las 10 filas
                    solo = np.zeros((CI.KF, 64, 64), bool)
                    solo[...] = f[..., 3] > 0
                    col, _ = CI.color_del_iris(cuerpo, solo)
                    self.assertIsNotNone(col, f"{r['id']}: color_del_iris no ve ningun iris en lo pintado")

    def test_independiente_cada_capa_de_raza_pinta_solo_iris(self):
        """CAR-2e: test que NO usa `admite` ni `detectar`. Para cada capa de raza del disco: en CADA frame de cara
        (SE 7-12, 29-32; SO 13-18, 33-36) hay >= 1 pixel pintado, y ningun pixel pintado tiene un color que salga en
        la mejilla (filas de la cara bajo la banda de ojos, del mismo frame), ni es casi blanco (v > 0,92 y
        s < 0,15: el blanco del ojo), ni un color de las vistas de espaldas."""
        import colorsys
        cara = tuple(range(7, 19)) + tuple(range(29, 37))
        for r in CON_OJOS:
            familia = atlas(MAN["cuerpos"]["atlas"][r["talla"]], len(MAN["cuerpos"]["filas"]))[0]
            cuerpo = atlas(MAN["razas"]["atlas"]["cuerpo"], MAN["razas"]["filas"])[r["fila"] - 1]
            capa = atlas(MAN["ojos"]["atlas"][r["id"]], len(COLORES))
            pintado = capa[0][..., 3] > 0
            for k in range(1, len(COLORES)):  # las 10 filas pintan los mismos pixeles
                self.assertTrue(((capa[k][..., 3] > 0) == pintado).all(), r["id"])
            # el mismo iris se ve en las dos diagonales: el tono medio de lo pintado (sin pupila oscura) no puede
            # diferir mas de 90 grados entre SE y SO (un mechon azul en SE y un iris amarillo en SO no son un ojo)
            if r["id"] not in AMBIGUAS:
                tonos = []
                for g in (tuple(range(7, 13)) + tuple(range(29, 33)), tuple(range(13, 19)) + tuple(range(33, 37))):
                    vx = vy = 0.0
                    for fr in g:
                        for y, x in zip(*np.nonzero(pintado[fr - 1])):
                            h, s_, v = colorsys.rgb_to_hsv(*(q / 255 for q in cuerpo[fr - 1][y, x][:3]))
                            if v >= 0.25 and s_ >= 0.12:
                                vx += np.cos(h * 2 * np.pi) * s_
                                vy += np.sin(h * 2 * np.pi) * s_
                    tonos.append(None if vx == vy == 0 else float(np.degrees(np.arctan2(vy, vx)) % 360))
                if None not in tonos:
                    d = abs(tonos[0] - tonos[1]) % 360
                    with self.subTest(raza=r["id"], motivo="tono"):
                        self.assertLessEqual(min(d, 360 - d), 90, f"{r['id']}: tono SE {tonos[0]:.0f} y SO {tonos[1]:.0f}: tono distinto entre diagonales")
            espalda = set()
            for f in range(40):
                if f + 1 not in cara:
                    op = cuerpo[f][cuerpo[f][..., 3] > 0][:, :3]
                    espalda.update(map(tuple, np.unique(op, axis=0).tolist()))
            for fr in cara:
                f = fr - 1
                with self.subTest(raza=r["id"], frame=fr):
                    self.assertTrue(pintado[f].any(), f"{r['id']} frame {fr}: sin ningun pixel de iris")
                coronilla = int(np.nonzero((familia[f][..., 3] > 0).any(axis=1))[0].min())
                ojos = coronilla + 8
                mejilla = set()
                sub = cuerpo[f][ojos + 4: ojos + 7]
                mejilla.update(map(tuple, np.unique(sub[sub[..., 3] > 0][:, :3], axis=0).tolist()))
                for y, x in zip(*np.nonzero(pintado[f])):
                    c = tuple(int(v) for v in cuerpo[f][y, x][:3])
                    h, s_, v = colorsys.rgb_to_hsv(*(q / 255 for q in c))
                    donde = f"{r['id']} frame {fr} ({x},{y}) {c}"
                    with self.subTest(donde=donde):
                        self.assertNotIn(c, mejilla, donde + ": color de la mejilla")
                        self.assertFalse(v > 0.92 and s_ < 0.15, donde + ": casi blanco")
                        self.assertNotIn(c, espalda, donde + ": color de espaldas")

    def test_drow_explorador_conserva_su_etiqueta(self):
        # el iris es azul claro con un pixel marron: ambiguo, se queda la etiqueta de siempre
        self.assertEqual(RAZAS["drow_explorador"]["ojos"], "rojo")

    def test_no_quedan_capas_de_ojos_huerfanas(self):
        usados = {MAN["ojos"]["atlas"][r["id"]] for r in CON_OJOS}
        en_disco = {f"personajes/ojos/{p.name}" for p in (TEX / "personajes" / "ojos").glob("*.png")}
        self.assertEqual(en_disco, usados)

    def test_admite_exige_iris_en_todos_los_frames_de_cada_diagonal(self):
        """CAR-2e: una mascara buena a la que se le quita el iris de un solo frame de frente (SE o SO) ya no pasa."""
        r = RAZAS["race_human_marches"]
        m = mascara_de(r)
        self.assertTrue(CI.admite(m))
        for fr in CI.FRAMES_OJOS:
            m2 = m.copy()
            m2[fr - 1] = False
            self.assertFalse(CI.admite(m2), f"frame {fr}")
        m3 = m.copy()
        for fr in CI.GRUPO_SE:
            m3[fr - 1] = False
        self.assertFalse(CI.admite(m3))

    def test_el_metodo_no_pinta_sin_iris(self):
        # un cuerpo sin color distinto del de la piel no tiene ojos que repintar
        cuerpo = np.zeros((CI.KF, 64, 64, 4), np.uint8)
        cuerpo[:, 20:50, 20:40] = [200, 150, 120, 255]
        m = CI.detectar(cuerpo, np.full(CI.KF, 20))
        self.assertFalse(m.any())
        self.assertFalse(CI.admite(m))


if __name__ == "__main__":
    unittest.main(verbosity=1)
