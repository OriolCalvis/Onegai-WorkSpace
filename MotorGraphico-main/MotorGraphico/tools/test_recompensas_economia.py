import argparse
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

from recompensas_economia import audita, recorre, reparto, lee
from simular_economia import simula

parser = argparse.ArgumentParser(add_help=False)
parser.add_argument('--recorrido', type=Path)
args, resto = parser.parse_known_args()
sys.argv = [sys.argv[0]] + resto
MOTOR = Path(__file__).resolve().parents[1]


class RecorridoTests(unittest.TestCase):
    def fixture(self, carpeta):
        m = Path(carpeta)
        for d in ['adventures', 'proyectos', 'catalogs', 'levels']:
            (m/'assets'/d).mkdir(parents=True)
        self.guarda(m/'assets/proyectos/boundington.json', dict(adventures=['a.json'], levels=['n.json']))
        self.guarda(m/'assets/catalogs/quests.json', dict(entries=[dict(id='q', rewardGold=0, rewardItemIds=[])]))
        self.guarda(m/'assets/levels/n.json', dict(objects=[]))
        premio = lambda n: [dict(type='grantGold', value=n), dict(type='setFlag', arg='pagado')]
        beat = dict(id='oferta', trigger=dict(type='talk', target='npc'), once=True, lines=['Elige.'], opciones=[
            dict(texto='Aceptar', effects=premio(30)), dict(texto='Robar', effects=premio(40))])
        self.guarda(m/'assets/adventures/a.json', dict(id='a', beats=[beat]))
        ruta = dict(perfil='p', grado='exito', victoria=True, flags_finales=['pagado'],
                    preferencias=['Aceptar'], acciones=[dict(dia=1, orden=10, aventura='a.json', accion='hablar', destino='npc') for _ in range(2)])
        return m, dict(rutas=[ruta])

    def guarda(self, p, d):
        p.write_text(json.dumps(d, ensure_ascii=False))

    def nativo(self, m, d):
        if not args.recorrido:
            self.skipTest('Usa CTest o --recorrido')
        return recorre(m, args.recorrido, d)

    def test_dos_ramas_no_suman_y_no_se_recobra_una_escena(self):
        with tempfile.TemporaryDirectory() as t:
            m, d = self.fixture(t)
            a = self.nativo(m, d)['p']['eventos']
            self.assertEqual([30], [e['oro'] for e in a])
            d['rutas'][0]['preferencias'] = ['Robar']
            self.assertEqual([40], [e['oro'] for e in self.nativo(m, d)['p']['eventos']])

    def test_decision_sin_politica_no_inventa_rama(self):
        with tempfile.TemporaryDirectory() as t:
            m, d = self.fixture(t); d['rutas'][0]['preferencias'] = []
            with self.assertRaisesRegex(ValueError, 'Falta preferencia'):
                self.nativo(m, d)

    def test_requisito_de_opcion_se_evalua_en_el_motor(self):
        with tempfile.TemporaryDirectory() as t:
            m, d = self.fixture(t); p = m/'assets/adventures/a.json'; a = lee(p)
            a['beats'][0]['opciones'][0]['requires'] = dict(allFlags=['permiso']); self.guarda(p, a)
            with self.assertRaisesRegex(ValueError, 'Falta preferencia'):
                self.nativo(m, d)

    def test_ruta_incompleta_falla(self):
        with tempfile.TemporaryDirectory() as t:
            m, d = self.fixture(t); d['rutas'][0]['flags_finales'].append('imposible')
            with self.assertRaisesRegex(ValueError, 'Ruta incompleta'):
                self.nativo(m, d)

    def test_aventura_fuera_del_manifiesto_falla(self):
        with tempfile.TemporaryDirectory() as t:
            m, d = self.fixture(t); d['rutas'][0]['acciones'][0]['aventura'] = '../a.json'
            with self.assertRaisesRegex(ValueError, 'fuera del proyecto'):
                self.nativo(m, d)

    def test_el_tiempo_no_retrocede(self):
        with tempfile.TemporaryDirectory() as t:
            m, d = self.fixture(t); d['rutas'][0]['acciones'] = copy.deepcopy(d['rutas'][0]['acciones'])
            d['rutas'][0]['acciones'][1]['orden'] = 5
            with self.assertRaisesRegex(ValueError, 'orden inválido'):
                self.nativo(m, d)

    def test_auditoria_incluye_opciones_y_excluye_otras_aventuras(self):
        with tempfile.TemporaryDirectory() as t:
            m, _ = self.fixture(t)
            self.guarda(m/'assets/adventures/otra.json', dict(beats=[dict(id='otro', effects=[dict(type='grantGold', value=999)])]))
            a = audita(m)
            self.assertEqual([0, 1], [e['opcion'] for e in a['efectos']])
            self.assertEqual(999, a['fuera_del_proyecto'][0]['efecto']['value'])
            self.assertEqual(0, a['misiones'][0]['oro'])

    def test_cambios_de_fuente_se_detectan_por_hash(self):
        with tempfile.TemporaryDirectory() as t:
            m, _ = self.fixture(t); a = audita(m)['sha256']; p = m/'assets/adventures/a.json'
            p.write_text(p.read_text()+'\n')
            self.assertNotEqual(a, audita(m)['sha256'])

    def test_avisar_loot_que_el_modelo_nativo_no_lee(self):
        with tempfile.TemporaryDirectory() as t:
            m, _ = self.fixture(t)
            self.guarda(m/'assets/levels/n.json', dict(objects=[dict(objectId='saco', eventId='mismo', loot=['pan'])]*2))
            _, r = reparto(m, {})
            self.assertFalse(r['compatible']); self.assertEqual(2, len(r['loot_fijo']))
            self.assertEqual(2, r['identidades_repetidas'][0]['veces'])

    def modelo(self):
        p = dict(recoge_pct=[100]*5, vende_objetos_pct=100, oficios_exito=[0]*3,
                 margenes=[[0, 1000]]*3, gastos=[25,0,0], compras=[[dict(id='reserva_historia', cantidad=25, orden=95)], [], []])
        e = dict(saldo_inicial=30, precio_oficio_exito=8, venta_objetos_pct=25, presupuestos=[], canales=['cofres_visibles', 'combates', 'misiones', 'entorno', 'tesoros_ocultos'], perfiles={'p': p})
        return dict(economia=e, niveles={}, objetos={'reliquia': dict(price=400)}, equipo={})

    def test_la_cama_no_se_cobra_dos_veces(self):
        escenas = {'p': dict(eventos=[dict(dia=1, orden=10, aventura='a', beat='cama', opcion='Pagar', oro=-25, objetos=[])])}
        r = simula(self.modelo(), [], lambda _: [], {}, escenas=escenas)['p']['dias'][0]
        self.assertEqual(5, r['saldo']); self.assertEqual(0, r['reservado']); self.assertEqual(25, r['gasto'])

    def test_cobro_mayor_que_bolsa_se_acota_como_gamesession(self):
        escenas = {'p': dict(eventos=[dict(dia=1, orden=10, aventura='a', beat='robo', opcion='', oro=-50, objetos=[])])}
        r = simula(self.modelo(), [], lambda _: [], {}, escenas=escenas)['p']['dias'][0]
        self.assertEqual(0, r['saldo']); self.assertEqual(30, r['gasto'])
        self.assertEqual(-50, r['movimientos'][0]['solicitado'])

    def test_premio_conservado_y_copia_del_cofre_vendible(self):
        d = self.modelo(); d['economia']['perfiles']['p']['compras'] = [[], [], []]
        i = dict(dia=1, canal='cofres_visibles', orden=20, nivel='n.json', eventId='cofre',
                 objectId='cofre', contenedor=dict(presupuesto=400))
        escenas = {'p': dict(eventos=[dict(dia=1, orden=10, aventura='a', beat='premio', opcion='', oro=0, objetos=['reliquia'])])}
        r = simula(d, [i], lambda _: [dict(grupo='p', oro=0, objetos=['reliquia'])], {}, escenas=escenas)['p']
        self.assertEqual(['reliquia'], r['conservado'])
        self.assertEqual(100, r['dias'][0]['ventas']); self.assertEqual(130, r['dias'][0]['saldo'])

    def test_las_tres_rutas_publicadas_llegan_al_final_sin_sumar_fiesta_doble(self):
        r = self.nativo(MOTOR, lee(MOTOR/'tools/rutas_economia.json'))
        self.assertEqual({'historia', 'explora_todo', 'optimiza'}, set(r))
        for perfil, datos in r.items():
            self.assertIn('bnd3_epilogo', datos['flags'])
            self.assertEqual([-25], [e['oro'] for e in datos['eventos'] if e['oro'] < 0])
            fiestas = [e['oro'] for e in datos['eventos'] if e['beat'] in ('b2_robo_bien', 'b2_sonsaca_exito')]
            self.assertLessEqual(len(fiestas), 1)
        self.assertIn(40, [e['oro'] for e in r['optimiza']['eventos']])


if __name__ == '__main__':
    unittest.main()
