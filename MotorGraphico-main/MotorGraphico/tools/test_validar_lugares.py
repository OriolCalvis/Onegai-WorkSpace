"""Pruebas de contratos y de las regresiones de identidad de los locales."""

import json
import tempfile
import unittest
from pathlib import Path

from validar_lugares import examina


class LugaresTests(unittest.TestCase):
    def setUp(self):
        self.temporal = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporal.cleanup)
        self.motor = Path(self.temporal.name)
        self.sitios = [{"nivel": "assets/levels/interior_banco.json", "nombre": "La Caja Firme",
                        "funcion": "banco y cambio"}]
        self.puertas = [{"objectId": "puerta_banco", "targetLevel": self.sitios[0]["nivel"]}]

    def escribe(self, ruta, valor):
        destino = self.motor / ruta
        destino.parent.mkdir(parents=True, exist_ok=True)
        destino.write_text(json.dumps(valor, ensure_ascii=False), encoding="utf-8")

    def prepara(self):
        self.escribe("assets/catalogs/lugares.json", {"lugares": self.sitios})
        self.escribe("assets/proyectos/boundington.json", {"levels": ["ciudad_nueva.json"]})
        self.escribe("assets/levels/ciudad_nueva.json", {"objects": self.puertas})
        for sitio in self.sitios:
            if sitio["nivel"].startswith("assets/levels/"):
                self.escribe(sitio["nivel"], {"objects": []})

    def errores(self):
        self.prepara()
        return examina(self.motor)[0]

    def test_un_local_una_puerta(self):
        self.assertEqual([], self.errores())
        self.assertEqual("La Caja Firme", examina(self.motor)[1][0]["nombre"])

    def test_dos_puertas_al_mismo_local_son_validas(self):
        self.puertas.append({"objectId": "puerta_banco_sur", "targetLevel": self.sitios[0]["nivel"]})
        self.assertEqual([], self.errores())
        self.assertEqual(2, len(examina(self.motor)[1]))

    def test_copias_con_nombres_distintos_son_validas(self):
        self.sitios.append({"nivel": "assets/levels/interior_banco_sur.json", "nombre": "La Otra Caja",
                            "funcion": "banco y cambio"})
        self.puertas.append({"objectId": "puerta_banco_sur", "targetLevel": self.sitios[1]["nivel"]})
        self.assertEqual([], self.errores())

    def test_nombre_repetido_ignora_mayusculas_y_espacios(self):
        self.sitios.append({"nivel": "assets/levels/interior_otro.json", "nombre": "  LA CAJA   FIRME ",
                            "funcion": "otro banco"})
        self.assertTrue(any("nombre repetido" in e for e in self.errores()))

    def test_normaliza_unicode(self):
        self.sitios[0]["nombre"] = "El Café"
        self.sitios.append({"nivel": "assets/levels/interior_otro.json", "nombre": "El Cafe\u0301",
                            "funcion": "otro café"})
        self.assertTrue(any("nombre repetido" in e for e in self.errores()))

    def test_puerta_sin_ficha(self):
        self.puertas[0]["targetLevel"] = "assets/levels/interior_desconocido.json"
        self.assertTrue(any("puerta sin sitio" in e for e in self.errores()))

    def test_nivel_de_ficha_inexistente(self):
        self.prepara()
        (self.motor / self.sitios[0]["nivel"]).unlink()
        self.assertTrue(any("el nivel no existe" in e for e in examina(self.motor)[0]))

    def test_no_limita_las_ciudades_a_los_tres_nombres_de_hoy(self):
        self.prepara()
        self.escribe("assets/proyectos/boundington.json", {"levels": ["ciudad_nueva.json", "ciudad_futura.json"]})
        self.escribe("assets/levels/ciudad_futura.json", {"objects": self.puertas})
        self.assertEqual([], examina(self.motor)[0])
        self.assertEqual(2, len(examina(self.motor)[1]))

    def test_no_confunde_pasajes_entre_barrios_con_locales(self):
        self.puertas.append({"objectId": "paso", "targetLevel": "assets/levels/ciudad_vecina.json"})
        self.assertEqual([], self.errores())

    def test_rechaza_fichas_sin_nombre(self):
        self.sitios[0]["nombre"] = " "
        self.assertTrue(any("textos no vacíos" in e for e in self.errores()))

    def test_rechaza_rutas_fuera_del_proyecto(self):
        self.sitios[0]["nivel"] = "../interior_banco.json"
        self.assertTrue(any("ruta fuera" in e for e in self.errores()))

    def test_no_da_verde_sin_ciudades(self):
        self.prepara()
        self.escribe("assets/proyectos/boundington.json", {"levels": []})
        self.assertTrue(any("ninguna ciudad" in e for e in examina(self.motor)[0]))

    def test_json_roto_es_error_legible(self):
        self.prepara()
        (self.motor / "assets/catalogs/lugares.json").write_text("{", encoding="utf-8")
        self.assertTrue(any("json" in e for e in examina(self.motor)[0]))

    def test_nivel_repetido_por_otra_ruta(self):
        self.sitios.append({**self.sitios[0], "nivel": "assets/levels/./interior_banco.json", "nombre": "Otro"})
        self.assertTrue(any("nivel repetido" in e for e in self.errores()))

    def test_tres_accesos_no_necesitan_lista_de_permisos(self):
        self.puertas.extend([
            {"objectId": "puerta_banco_sur", "targetLevel": self.sitios[0]["nivel"]},
            {"objectId": "puerta_banco_este", "targetLevel": self.sitios[0]["nivel"]},
        ])
        self.assertEqual([], self.errores())
        self.assertEqual(3, len(examina(self.motor)[1]))

    def test_accesos_con_rutas_equivalentes_comparten_ficha(self):
        self.puertas.append({"objectId": "puerta_sur", "targetLevel": "assets/levels/./interior_banco.json"})
        self.assertEqual([], self.errores())
        self.assertEqual(1, len({p["destino"] for p in examina(self.motor)[1]}))

    def test_compartir_local_no_oculta_una_puerta_sin_ficha(self):
        self.puertas.extend([
            {"objectId": "puerta_banco_sur", "targetLevel": self.sitios[0]["nivel"]},
            {"objectId": "puerta_sin_ficha", "targetLevel": "assets/levels/interior_ausente.json"},
        ])
        self.assertTrue(any("puerta sin sitio" in e and "puerta_sin_ficha" in e for e in self.errores()))


if __name__ == "__main__":
    unittest.main()
