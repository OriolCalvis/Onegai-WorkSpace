#!/usr/bin/env python3
"""Pruebas de validar_puertas.py (AV14-20) con niveles de mentira.

    python3 tools/test_validar_puertas.py

Cada regla tiene su caso que la hace fallar. Los mapas, catalogos y niveles se escriben
en un directorio temporal: no dependen de los niveles de verdad. Al final hay una prueba
de humo sobre el repo, que solo corre si existe assets/levels.
"""
import json
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import validar_puertas as vp  # noqa: E402

PISO, MURO = 1, 2   # firstgid 1; el tile id 1 es el muro (GID 2) y lleva collision


def tmx(ancho, alto, capas, firstgid=1, colision_ids=(1,)):
    """TMX minimo: un tileset embebido con colision en colision_ids, y una capa CSV por cada lista."""
    partes = [f'<map version="1.10" width="{ancho}" height="{alto}" tilewidth="16" tileheight="16">',
              f'<tileset firstgid="{firstgid}" name="t" tilewidth="16" tileheight="16" tilecount="4" columns="2">']
    for i in colision_ids:
        partes.append(f'<tile id="{i}"><properties><property name="collision" type="bool" value="true"/>'
                      '</properties></tile>')
    partes.append('</tileset>')
    for n, capa in enumerate(capas):
        filas = ",\n".join(",".join(str(g) for g in fila) for fila in capa)
        partes.append(f'<layer id="{n + 1}" name="capa{n}" width="{ancho}" height="{alto}">'
                      f'<data encoding="csv">\n{filas}\n</data></layer>')
    partes.append('</map>')
    return "\n".join(partes)


def suelo(ancho, alto, muros=(), gid=PISO):
    capa = [[gid] * ancho for _ in range(alto)]
    for x, y in muros:
        capa[y][x] = MURO
    return capa


def celda(x, y):
    return {"x": x, "y": y}


CATALOGO = {"entries": [
    {"id": "muro", "category": "prop", "blocksMovement": True},
    {"id": "moneda", "category": "pickup"},
    {"id": "cartel", "category": "prop"},
    {"id": "slime", "category": "enemy", "blocksMovement": True},
]}


class Repo:
    """Un repo de mentira con dos niveles: origen.json (puerta a destino) y destino.json."""

    def __init__(self, tmp: Path):
        self.raiz = tmp
        for d in ("assets/levels", "assets/maps", "assets/objects", "assets/proyectos"):
            (tmp / d).mkdir(parents=True, exist_ok=True)
        self.escribe_json("assets/objects/catalogo.json", CATALOGO)
        self.mapa("assets/maps/origen.tmx", tmx(4, 4, [suelo(4, 4)]))
        self.mapa("assets/maps/destino.tmx", tmx(4, 4, [suelo(4, 4)]))

    def escribe_json(self, rel, datos):
        (self.raiz / rel).write_text(json.dumps(datos, ensure_ascii=False), encoding="utf-8")

    def mapa(self, rel, texto):
        (self.raiz / rel).write_text(texto, encoding="utf-8")

    def nivel(self, nombre, mapa, objetos=(), enemigos=(), inicio=(0, 0)):
        self.escribe_json(f"assets/levels/{nombre}", {
            "name": nombre, "map": mapa, "playerStart": celda(*inicio),
            "objects": list(objetos), "enemies": list(enemigos)})

    def puerta(self, objetivo="assets/levels/destino.json", llegada=None, oid="puerta_a"):
        o = {"objectId": oid, "position": celda(0, 0), "targetLevel": objetivo}
        if llegada is not None:
            o["targetPosition"] = celda(*llegada)
        return o

    def vuelta(self, oid="puerta_vuelta"):
        return {"objectId": oid, "position": celda(0, 0), "targetLevel": "assets/levels/origen.json"}


class ReglasPuertas(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.r = Repo(Path(self._tmp.name))

    def tearDown(self):
        self._tmp.cleanup()

    def comprueba(self):
        return vp.comprueba(self.r.raiz)

    def base(self, puerta, destino_objetos=(), destino_enemigos=(), inicio=(0, 0), con_vuelta=True):
        """origen con la puerta; destino con sus objetos, enemigos y (opcional) la puerta de vuelta."""
        self.r.nivel("origen.json", "assets/maps/origen.tmx", [puerta])
        extra = list(destino_objetos) + ([self.r.vuelta()] if con_vuelta else [])
        self.r.nivel("destino.json", "assets/maps/destino.tmx", extra, destino_enemigos, inicio)

    def una(self, problemas, fragmento):
        textos = [p[3] for p in problemas]
        self.assertTrue(any(fragmento in t for t in textos),
                        f"no aparece '{fragmento}' en {textos}")

    def test_puerta_correcta_no_da_problemas(self):
        self.base(self.r.puerta(llegada=(2, 2)))
        niveles, puertas, problemas = self.comprueba()
        self.assertEqual(problemas, [])
        self.assertEqual((niveles, puertas), (2, 2))  # la puerta y la de vuelta

    def test_destino_inexistente(self):
        self.base(self.r.puerta(objetivo="assets/levels/no_existe.json", llegada=(1, 1)))
        _, _, problemas = self.comprueba()
        self.una(problemas, "no existe")

    def test_llegada_en_pared(self):
        self.r.mapa("assets/maps/destino.tmx", tmx(4, 4, [suelo(4, 4, muros=[(2, 2)])]))
        self.base(self.r.puerta(llegada=(2, 2)))
        _, _, problemas = self.comprueba()
        self.una(problemas, "colision")

    def test_colision_en_una_capa_basta(self):
        # la capa de suelo esta libre; la de arriba pone muro en la llegada
        self.r.mapa("assets/maps/destino.tmx",
                    tmx(4, 4, [suelo(4, 4), suelo(4, 4, muros=[(2, 2)], gid=0)]))
        self.base(self.r.puerta(llegada=(2, 2)))
        _, _, problemas = self.comprueba()
        self.una(problemas, "colision")

    def test_firstgid_distinto_se_respeta(self):
        # tileset con firstgid 5: el tile id 0 es el GID 5, y ese es el muro
        capa = suelo(4, 4, gid=PISO)
        capa[2][2] = 5
        self.r.mapa("assets/maps/destino.tmx", tmx(4, 4, [capa], firstgid=5, colision_ids=(0,)))
        self.base(self.r.puerta(llegada=(2, 2)))
        _, _, problemas = self.comprueba()
        self.una(problemas, "colision")

    def test_objeto_bloqueante_en_la_llegada(self):
        muro = {"objectId": "muro", "position": celda(2, 2)}
        self.base(self.r.puerta(llegada=(2, 2)), destino_objetos=[muro])
        _, _, problemas = self.comprueba()
        self.una(problemas, "bloquea (muro)")

    def test_pickup_no_bloquea_la_llegada(self):
        moneda = {"objectId": "moneda", "position": celda(2, 2)}
        self.base(self.r.puerta(llegada=(2, 2)), destino_objetos=[moneda])
        _, _, problemas = self.comprueba()
        self.assertEqual(problemas, [])

    def test_objeto_sin_ficha_no_bloquea(self):
        fantasma = {"objectId": "no_esta_en_ningun_catalogo", "position": celda(2, 2)}
        self.base(self.r.puerta(llegada=(2, 2)), destino_objetos=[fantasma])
        _, _, problemas = self.comprueba()
        self.assertEqual(problemas, [])

    def test_enemigo_en_la_llegada(self):
        enemigo = {"type": "slime", "position": celda(2, 2)}
        self.base(self.r.puerta(llegada=(2, 2)), destino_enemigos=[enemigo])
        _, _, problemas = self.comprueba()
        self.una(problemas, "enemigo")

    def test_enemigo_de_categoria_enemy_en_objects(self):
        slime = {"objectId": "slime", "position": celda(2, 2)}
        self.base(self.r.puerta(llegada=(2, 2)), destino_objetos=[slime])
        _, _, problemas = self.comprueba()
        self.una(problemas, "enemigo (slime)")

    def test_sin_puerta_de_vuelta(self):
        self.base(self.r.puerta(llegada=(2, 2)), con_vuelta=False)
        _, _, problemas = self.comprueba()
        self.una(problemas, "no tiene puerta de vuelta")

    def _vuelta_por_tierra(self, con_camino):
        """origen -> destino (sin puerta directa de vuelta); destino -> tierra -> origen si hay camino."""
        p = self.r.puerta(llegada=(2, 2))
        p["vueltaPorTierra"] = "assets/levels/tierra.json"
        self.r.nivel("origen.json", "assets/maps/origen.tmx", [p])
        destino = [self.r.puerta(objetivo="assets/levels/tierra.json", llegada=(1, 1), oid="a_tierra")] if con_camino else []
        self.r.nivel("destino.json", "assets/maps/destino.tmx", destino)
        self.r.nivel("tierra.json", "assets/maps/destino.tmx",
                     [self.r.vuelta(oid="tierra_a_origen"), self.r.puerta(llegada=(1, 1), oid="tierra_a_destino")])

    def test_vuelta_por_tierra_con_camino(self):
        self._vuelta_por_tierra(con_camino=True)
        _, _, problemas = self.comprueba()
        self.assertEqual([p for p in problemas if p[1] == "puerta_a"], [])

    def test_vuelta_por_tierra_sin_camino(self):
        self._vuelta_por_tierra(con_camino=False)
        _, _, problemas = self.comprueba()
        self.una(problemas, "no hay camino de vuelta por puertas")

    def test_sin_targetPosition_usa_el_playerStart_del_destino(self):
        self.r.mapa("assets/maps/destino.tmx", tmx(4, 4, [suelo(4, 4, muros=[(2, 2)])]))
        self.base(self.r.puerta(llegada=None), inicio=(2, 2))   # playerStart en el muro
        _, _, problemas = self.comprueba()
        self.una(problemas, "colision")

    def test_sin_targetPosition_playerStart_libre(self):
        self.r.mapa("assets/maps/destino.tmx", tmx(4, 4, [suelo(4, 4, muros=[(2, 2)])]))
        self.base(self.r.puerta(llegada=None), inicio=(1, 1))   # playerStart libre
        _, _, problemas = self.comprueba()
        self.assertEqual(problemas, [])

    def test_llegada_fuera_del_mapa(self):
        self.base(self.r.puerta(llegada=(9, 9)))
        _, _, problemas = self.comprueba()
        self.una(problemas, "fuera del mapa")

    def test_catalogo_es_el_del_proyecto_que_lista_el_nivel(self):
        # el proyecto P1 lista destino.json y su catalogo define "cartel" sin bloquear;
        # el catalogo de P2 (que no lista el nivel) lo haria bloquear: no debe contar.
        self.r.escribe_json("assets/objects/p1.json", {"entries": [
            {"id": "cartel", "category": "prop", "blocksMovement": False}]})
        self.r.escribe_json("assets/objects/p2.json", {"entries": [
            {"id": "cartel", "category": "prop", "blocksMovement": True}]})
        self.r.escribe_json("assets/proyectos/index.json", {"proyectos": ["p1", "p2"]})
        self.r.escribe_json("assets/proyectos/p1.json", {
            "settings": {"catalogsDirectory": "objects"}, "catalogs": ["p1.json"],
            "levels": ["destino.json", "origen.json"]})
        self.r.escribe_json("assets/proyectos/p2.json", {
            "settings": {"catalogsDirectory": "objects"}, "catalogs": ["p2.json"],
            "levels": ["otro.json"]})
        cartel = {"objectId": "cartel", "position": celda(2, 2)}
        self.base(self.r.puerta(llegada=(2, 2)), destino_objetos=[cartel])
        _, _, problemas = self.comprueba()
        self.assertEqual(problemas, [])

    def test_mapa_con_tileset_externo_se_marca(self):
        self.r.mapa("assets/maps/destino.tmx",
                    '<map width="4" height="4"><tileset firstgid="1" source="x.tsx"/></map>')
        self.base(self.r.puerta(llegada=(1, 1)))
        _, _, problemas = self.comprueba()
        self.una(problemas, "tileset externo")

    def test_un_problema_no_tumba_el_resto(self):
        self.r.escribe_json("assets/levels/roto.json", {"map": "assets/maps/no_hay.tmx", "objects": [
            self.r.puerta(objetivo="assets/levels/destino.json", llegada=(1, 1), oid="x")]})
        self.base(self.r.puerta(llegada=(9, 9)))
        _, puertas, problemas = self.comprueba()
        self.assertGreaterEqual(puertas, 2)
        self.una(problemas, "fuera del mapa")


class ResumDeProblemas(unittest.TestCase):
    def test_un_problema_de_cada_tipo(self):
        probs = [
            ("n", "a", "targetLevel", "no se pudo comprobar: [Errno 35] Resource deadlock avoided"),
            ("n", "b", "targetLevel", "x.json no tiene puerta de vuelta a y.json"),
            ("n", "c", "targetLevel", "el nivel 'z.json' no existe"),
            ("n", "d", "targetPosition", "llegada (1,1): celda con colision; no es pisable"),
            ("n", "e", "targetPosition", "cosa rara"),
        ]
        self.assertEqual(vp.resum(probs), {
            "icloud": 1, "sin_vuelta": 1, "destino_inexistente": 1, "casilla_no_pisable": 1, "otro": 1})

    def test_lista_vacia(self):
        self.assertEqual(sum(vp.resum([]).values()), 0)


class PruebaDeHumo(unittest.TestCase):
    def test_repo_real(self):
        if not (vp.MOTOR / "assets" / "levels").is_dir():
            self.skipTest("no hay assets/levels junto a tools/")
        niveles, puertas, problemas = vp.comprueba(vp.MOTOR)
        self.assertGreater(niveles, 0)
        self.assertGreater(puertas, 0)
        for p in problemas:
            self.assertEqual(len(p), 4)


if __name__ == "__main__":
    unittest.main()
