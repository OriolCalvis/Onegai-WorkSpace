"""C8-03a — tools/validar_eventos.py: el contrato de los eventos del mundo (FORMATO_EVENTOS.md)."""
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

REAL = Path(__file__).resolve().parents[2]

AVENTURA = {"beats": [
    {"id": "b_avisar", "trigger": {"type": "talk", "target": "venides_base"},
     "effects": [{"type": "setFlag", "arg": "bnd3_avisado"}]},
    {"id": "b_xila_avisa", "trigger": {"type": "auto"}, "requires": {"allFlags": ["bnd1_xila_honor"]},
     "effects": [{"type": "setFlag", "arg": "bnd3_xila_avisa"}]},
    {"id": "b_honor", "trigger": {"type": "auto"}, "effects": [{"type": "setFlag", "arg": "bnd1_xila_honor"}]},
    {"id": "b_arenga", "trigger": {"type": "auto"}, "requires": {"lugar": "assets/levels/base_militar.json"},
     "lines": ["¡Aventurero!"]},
]}
CINES = {"cinematicas": [{"id": "base_campanas", "nivel": "assets/levels/base_militar.json", "pasos": []}]}

OCASO = {
    "id": "comienza_ocaso",
    "ausencia": "avanza",
    "condiciones": {"notFlags": ["bnd3_campanas"]},
    "disparadores": [
        {"id": "aviso_venides", "senal": "bnd3_avisado"},
        {"id": "aviso_xila", "senal": "bnd3_xila_avisa", "condiciones": {"allFlags": ["bnd1_xila_honor"]}},
        {"id": "puesta_sol", "cruzaMomento": "ocaso_puesta"},
    ],
    "efectos": [{"type": "setFlag", "arg": "bnd3_campanas"}],
    "escenas": [{"beat": "b_arenga"}, {"cinematica": "base_campanas"}],
}


def catalogo(*eventos, **extra):
    d = {"momentos": {"ocaso_puesta": {"dia": 4, "franja": 2}},
         "banderasDeObservacion": ["bnd3_plaza_vista", "bnd3_arenga"],
         "eventos": [copy.deepcopy(e) for e in eventos]}
    d.update(extra)
    return d


class ValidarEventosTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory(prefix="eventos ")
        self.addCleanup(tmp.cleanup)
        self.raiz = Path(tmp.name)
        for rel, datos in (("assets/adventures/a.json", AVENTURA), ("assets/cinematicas/cinematicas.json", CINES)):
            (self.raiz / rel).parent.mkdir(parents=True, exist_ok=True)
            (self.raiz / rel).write_text(json.dumps(datos), encoding="utf-8")

    def corre(self, cat):
        if cat is not None:
            (self.raiz / "assets/eventos").mkdir(parents=True, exist_ok=True)
            (self.raiz / "assets/eventos/boundington.json").write_text(json.dumps(cat), encoding="utf-8")
        r = subprocess.run([sys.executable, str(REAL / "tools/validar_eventos.py"), "--raiz", str(self.raiz)],
                           capture_output=True, text=True)
        return r.returncode, r.stdout

    def falla(self, cat, *textos):
        codigo, salida = self.corre(cat)
        self.assertEqual(codigo, 1, salida)
        for t in textos:
            self.assertIn(t, salida)

    def con(self, **cambios):
        e = copy.deepcopy(OCASO)
        e.update(cambios)
        return e

    def test_sin_catalogo_no_hay_nada_que_comprobar(self):
        self.assertEqual(self.corre(None)[0], 0)

    def test_el_ocaso_con_tres_disparadores_es_valido(self):
        codigo, salida = self.corre(catalogo(OCASO))
        self.assertEqual(codigo, 0, salida)
        self.assertIn("1 eventos en 1 catálogos; 0 errores, 0 avisos", salida)

    def test_el_sitio_del_jugador_no_es_causa(self):
        self.falla(catalogo(self.con(condiciones={"lugar": "assets/levels/base_militar.json"})), "«lugar»")
        self.falla(catalogo(self.con(disparadores=[{"entrar": "assets/levels/base_militar.json"}])),
                   "no es un disparador")
        self.falla(catalogo(self.con(lugar="x")), "clave desconocida")

    def test_haber_visto_una_escena_no_es_causa(self):
        self.falla(catalogo(self.con(disparadores=[{"senal": "cine_base_campanas_vista"}])), "haber visto")
        self.falla(catalogo(self.con(condiciones={"allFlags": ["bnd3_plaza_vista"]})), "haber visto")
        self.falla(catalogo(self.con(condiciones={"notFlags": ["bnd3_arenga"]})), "de observación")

    def test_solo_efectos_de_estado_y_sin_tocar_el_tiempo(self):
        for malo in ({"type": "log", "arg": "La Matanza ha empezado."}, {"type": "startBattle", "arg": "x"},
                     {"type": "grantGold", "arg": 10}, {"type": "pasarFranja", "arg": "noche"}):
            self.falla(catalogo(self.con(efectos=[malo])), "no vale en un evento")
        self.falla(catalogo(self.con(efectos=[{"type": "setVariable", "arg": "franja", "value": 2}])),
                   "no cambia «franja»")

    def test_momentos_resueltos_y_ventanas_en_orden(self):
        self.falla(catalogo(self.con(disparadores=[{"cruzaMomento": "calendario.ocaso"}])), "sin resolver")
        self.falla(catalogo(self.con(disparadores=[{"cruzaMomento": {"dia": 4, "franja": 3}}])), "un momento es")
        self.falla(catalogo(self.con(ventana={"desde": {"dia": 4, "franja": 2}, "hasta": {"dia": 4, "franja": 1}})),
                   "no es anterior")

    def test_politica_de_ausencia(self):
        self.falla(catalogo(self.con(ausencia=None)), "di qué pasa si el jugador no está")
        self.falla(catalogo(self.con(ausencia="caduca")), "necesita saber hasta cuándo", "alCaducar")
        self.falla(catalogo(self.con(ausencia="espera")), "qué intervención real espera")
        ok = self.con(ausencia="caduca", ventana={"hasta": "ocaso_puesta"},
                      alCaducar=[{"type": "setFlag", "arg": "bnd3_sermon_acabado"}])
        self.assertEqual(self.corre(catalogo(ok))[0], 0)

    def test_senales_que_nadie_emite(self):
        self.falla(catalogo(self.con(disparadores=[{"senal": "bnd3_nadie_la_emite"}])), "nadie emite")

    def test_escenas_y_rastros_existen(self):
        self.falla(catalogo(self.con(escenas=[{"beat": "b_no_existe"}])), "ningún beat")
        self.falla(catalogo(self.con(rastros=[{"cinematica": "no_existe"}])), "ninguna cinemática")

    def test_eventos_previos_sin_ciclos(self):
        a = self.con(id="a", disparadores=[{"eventoPrevio": "b"}])
        b = self.con(id="b", disparadores=[{"eventoPrevio": "a"}])
        self.falla(catalogo(a, b), "ciclo de eventos previos")
        self.falla(catalogo(self.con(disparadores=[{"eventoPrevio": "no_hay"}])), "ningún evento «no_hay»")
        self.falla(catalogo(OCASO, OCASO), "repetido")

    def test_un_evento_puede_seguir_a_otro_por_su_fase(self):
        sermon = {"id": "sermon", "ausencia": "avanza", "disparadores": [{"cruzaMomento": {"dia": 4, "franja": 0}}],
                  "fases": [{"id": "acabado", "disparadores": [{"cruzaMomento": {"dia": 4, "franja": 1}}],
                             "efectos": [{"type": "setFlag", "arg": "bnd3_sermon_acabado"}]}]}
        tras = self.con(id="tras_sermon", disparadores=[{"eventoPrevio": "sermon.acabado"}])
        codigo, salida = self.corre(catalogo(sermon, tras))
        self.assertEqual(codigo, 0, salida)
        self.falla(catalogo(sermon, self.con(id="t2", disparadores=[{"eventoPrevio": "sermon.no_fase"}])),
                   "no tiene la fase")

    def test_las_banderas_evt_son_solo_del_evaluador(self):
        def con_aventura(beat=None, cine=None):
            aventura = copy.deepcopy(AVENTURA)
            if beat:
                aventura["beats"].append(beat)
            cines = copy.deepcopy(CINES)
            if cine:
                cines["cinematicas"][0].update(cine)
            (self.raiz / "assets/adventures/a.json").write_text(json.dumps(aventura), encoding="utf-8")
            (self.raiz / "assets/cinematicas/cinematicas.json").write_text(json.dumps(cines), encoding="utf-8")
            return catalogo(OCASO)

        for efecto in ({"type": "setFlag", "arg": "evt_comienza_ocaso"},
                       {"type": "clearFlag", "arg": "evt_comienza_ocaso__caducado"},
                       {"type": "setVariable", "arg": "evt_comienza_ocaso__causa", "value": 1},
                       {"type": "addVariable", "arg": "evt_x__causa", "value": 1}):
            self.falla(con_aventura(beat={"id": "b_malo", "trigger": {"type": "auto"}, "effects": [efecto]}),
                       "evt_", "b_malo")
        self.falla(con_aventura(beat={"id": "b_opc", "trigger": {"type": "auto"}, "opciones": [
            {"texto": "x", "effects": [{"type": "setFlag", "arg": "evt_comienza_ocaso"}]}]}), "b_opc")
        self.falla(con_aventura(cine={"alTerminar": ["evt_comienza_ocaso"]}), "base_campanas")
        self.falla(con_aventura(cine={"pasos": [{"flag": "evt_comienza_ocaso"}]}), "base_campanas")
        # leerlas en requires es válido
        ok = con_aventura(beat={"id": "b_rastro", "trigger": {"type": "auto"},
                                "requires": {"allFlags": ["evt_comienza_ocaso"]}, "lines": ["Suenan campanas."]})
        codigo, salida = self.corre(ok)
        self.assertEqual(codigo, 0, salida)

    def test_evt_se_vigila_aunque_no_haya_catalogo(self):
        aventura = copy.deepcopy(AVENTURA)
        aventura["beats"].append({"id": "b_malo", "trigger": {"type": "auto"},
                                  "effects": [{"type": "setFlag", "arg": "evt_x"}]})
        (self.raiz / "assets/adventures/a.json").write_text(json.dumps(aventura), encoding="utf-8")
        codigo, salida = self.corre(None)
        self.assertEqual(codigo, 1, salida)
        self.assertIn("evt_x", salida)

    def test_avisa_de_peajes(self):
        aventura = copy.deepcopy(AVENTURA)
        aventura["beats"].append({"id": "b_local", "requires": {"lugar": "assets/levels/interior_cuartel.json"},
                                  "trigger": {"type": "auto"}, "effects": [{"type": "setFlag", "arg": "bnd3_local"}]})
        (self.raiz / "assets/adventures/a.json").write_text(json.dumps(aventura), encoding="utf-8")
        codigo, salida = self.corre(catalogo(self.con(condiciones={"allFlags": ["bnd3_local"]})))
        self.assertEqual(codigo, 0, salida)
        self.assertIn("posible peaje", salida)


if __name__ == "__main__":
    unittest.main()
