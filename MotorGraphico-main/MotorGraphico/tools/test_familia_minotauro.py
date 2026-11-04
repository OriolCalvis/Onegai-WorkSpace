"""Regresiones de la familia centauro: datos completos, patas conservadas y ropa limitada al torso."""
import unittest
import numpy as np
import pixellab_cuerpo as cuerpo
import pixellab_minotauro as minotauro
import actores_razas as razas


class FamiliaMinotauroTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.talla = cuerpo.cargar_familia('minotauro')

    def test_cuarenta_frames_propios_y_cuatro_direcciones(self):
        self.assertEqual(self.talla.n_frames, 40)
        self.assertEqual(len(set(self.talla._dir.values())), 4)
        self.assertTrue(all((self.talla.labs[f] == 'q').any() for f in range(1, 41)))

    def test_las_capas_reconstruyen_el_dibujo_original_sin_fragmentos_ajenos(self):
        import json
        datos = json.loads((cuerpo.CARPETA / 'minotauro.json').read_text())
        for rid in datos['razas']:
            capas = dict(minotauro.capas(razas.POR_ID[rid], self.talla))
            for f, codificado in enumerate(datos['razas'][rid]):
                reconstruido = capas['cuerpo'][f].copy()
                for nombre in ('cola', 'cuernos'):
                    mask = capas[nombre][f][..., 3] > 0
                    reconstruido[mask] = capas[nombre][f][mask]
                esperado = cuerpo.bajar(cuerpo._dec(codificado), self.talla.dy_pies)
                self.assertTrue(np.array_equal(reconstruido, esperado), (rid, f))

    def test_banyes_no_quedan_cortadas_por_el_borde(self):
        import pixellab_extras
        for ident in pixellab_extras.CUERNOS:
            for f, px in enumerate(pixellab_extras.capa_extra(self.talla, ident)):
                self.assertFalse(px[0, :, 3].any() or px[-1, :, 3].any() or
                                 px[:, 0, 3].any() or px[:, -1, 3].any(), (ident, f))

    def test_anclas_sobre_cuerpo_en_los_cuarenta_frames(self):
        for f in range(1, 41):
            for x, y in minotauro.anclas(self.talla, f).values():
                self.assertTrue(self.talla.alfa[f][y, x])

    def test_los_pantalones_y_las_botas_no_pintan_sobre_las_cuatro_patas(self):
        llenos = [np.full((64, 64, 4), 255, np.uint8) for _ in range(40)]
        for ranura in ('piernas', 'pies'):
            self.assertTrue(all(not x.any() for x in minotauro.vestir(self.talla, llenos, ranura)))


if __name__ == '__main__':
    unittest.main()
