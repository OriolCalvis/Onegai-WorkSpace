import copy
from pathlib import Path
import unittest
from validar_botin import carga, valida

class BotinTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.base = carga(Path(__file__).resolve().parents[1])
    def setUp(self): self.d = copy.deepcopy(self.base)
    def falla(self, texto):
        with self.assertRaisesRegex(ValueError, texto): valida(self.d)
    def test_catalogos_reales(self):
        r = valida(self.d); self.assertEqual((50, 12, 30), (r['reutilizables'], r['unicos'], r['tablas']))
    def test_id_desconocido(self):
        self.d['tablas'][1]['entradas'][0]['id'] = 'nada'; self.falla('inexistente')
    def test_precio_inventado(self):
        self.d['tablas'][1]['entradas'][0]['valor'] += 1; self.falla('precio')
    def test_unica_repetida(self):
        t = self.d['tablas'][1]; u = next(e for e in t['entradas'] if e['unico'])
        self.d['tablas'][2]['entradas'].append(u); self.falla('repetida entre tablas')
    def test_efecto_sin_estado(self):
        u = next(e for e in self.d['botin'] if e['unico']); self.d['cartas'][u['skillId']]['effects'][0]['mechanicHook'] = 'condicion:inexistente_av18:1'; self.falla('Condición inexistente')  # quemado ya existe desde el 15/10
    def test_presupuesto_alterado(self):
        self.d['economia']['presupuestos'][0]['valor'][0] += 1; self.falla('Sumas diarias')
    def test_sin_tabla(self):
        self.d['tablas'].pop(); self.falla('Faltan tablas|ausente')
    def test_precio_de_equipo_distinto(self):
        u = next(e for e in self.d['botin'] if e.get('equipmentId'));self.d['equipo'][u['equipmentId']]['price'] += 1;self.falla('Equipo/precio')
    def coloca(self, dia=1, repetido=False):
        o = self.d['objetos']['botin_ciudad_cofre']; s = dict(objectId=o['id'], eventId='cofre_1', botin=dict(dia=dia,region='ciudad',canal='cofres_visibles',orden=1))
        self.d['niveles'] = {'ciudad_prueba.json':dict(objects=[s,copy.deepcopy(s)] if repetido else [s])};return s
    def test_reparto_valido(self):
        self.coloca();self.assertEqual(1,len(valida(self.d)['instancias']))
    def test_identidad_repetida(self): self.coloca(repetido=True);self.falla('eventId')
    def test_sin_disponibilidad(self): s=self.coloca();del s['botin'];self.falla('sin disponibilidad')
    def test_reparto_sobrepresupuesto(self):
        s=self.coloca(2);otro=copy.deepcopy(s);otro['eventId']='cofre_2';self.d['niveles']['ciudad_prueba.json']['objects'].append(otro);self.falla('Reparto supera')
    def test_booleano_no_es_presupuesto(self):
        self.d['objetos']['botin_ciudad_cofre']['contenedor']['presupuesto']=True;self.falla('fuera de rango')
    def test_unica_garantizada_necesita_la_marca_de_su_tabla(self):
        u=next(e['id'] for e in self.d['botin'] if e['unico'] and e['region']=='ciudad')
        self.d['objetos']['botin_interiores_cofre']['contenedor']['garantizados']=[u]
        self.falla('Única garantizada fuera')
    def test_no_mision_escondida(self):
        self.d['objetos']['botin_ciudad_cofre']['contenedor']['garantizados']=['llave_que_no_existe'];self.falla('Garantizado desconocido')

if __name__ == '__main__': unittest.main()
