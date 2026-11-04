import copy
from pathlib import Path
import unittest
from generar_acciones_monstruos import magnitud
from validar_monstruos_acciones import carga,valida

class AccionesTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):cls.base=carga(Path(__file__).resolve().parents[1])
    def setUp(self):self.d=copy.deepcopy(self.base)
    def falla(self,patron):
        with self.assertRaisesRegex(ValueError,patron):valida(self.d)
    def test_repertorios_y_cifras_reales(self):
        r=valida(self.d);self.assertEqual((900,434,15,5,44),(r['ataques'],r['control_sin_dano'],r['actores_actuales'],r['plantillas_previstas'],r['cartas']))
    def test_escalones_y_control_cero(self):
        self.assertEqual(2,magnitud(1,'criatura'));self.assertEqual(8,magnitud(3,'elite'));self.assertEqual(14,magnitud(5,'jefe'));self.assertEqual(0,magnitud(5,'jefe',True))
    def test_excepciones_no_intercambian_golpe_y_control(self):
        m={x['id']:x for x in self.d['monstruos']}
        self.assertEqual(0,m['itx_matriarca_de_la_sal']['attacks'][0]['magnitude'])
        self.assertEqual(12,m['itx_matriarca_de_la_sal']['attacks'][1]['magnitude'])
        self.assertEqual(0,m['el_portero_de_los_mil_marcos']['attacks'][0]['magnitude'])
        self.assertEqual(0,m['la_matriarca_del_pozo_azul']['attacks'][0]['magnitude'])
    def test_cifra_ausente(self):del self.d['monstruos'][0]['attacks'][0]['magnitude'];self.falla('sin cifra')
    def test_booleano_no_es_cifra(self):self.d['monstruos'][0]['attacks'][0]['magnitude']=True;self.falla('sin cifra')
    def test_formula_alterada(self):self.d['monstruos'][0]['attacks'][0]['magnitude']+=1;self.falla('Fórmula')
    def test_falta_actor_actual(self):self.d['repertorios'].pop(0);self.falla('Actor de Boundington')
    def test_carta_inexistente(self):id=self.d['repertorios'][0]['skillIds'][0];del self.d['cartas'][id];self.falla('inexistente')
    def test_estado_futuro_no_cuela(self):
        # Id que NO existe en conditions.json (antes se usaba 'quemado', que PO-37 anadio al catalogo y la prueba se desfaso).
        futuro='estado_futuro_inexistente_de_prueba'
        self.assertNotIn(futuro,self.d['condiciones'])
        id=self.d['repertorios'][0]['skillIds'][1];self.d['cartas'][id]['effects'][0]['mechanicHook']='condicion:'+futuro+':1';self.falla('Estado desconocido')
    def test_objeto_desconectado(self):id=self.d['repertorios'][0]['id'];self.d['objetos'][id]['combat']['skills']=[];self.falla('sin conectar')

if __name__=='__main__':unittest.main()
