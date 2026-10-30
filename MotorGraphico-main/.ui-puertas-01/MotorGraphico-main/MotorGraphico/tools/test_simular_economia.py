import argparse
import copy
import json
from pathlib import Path
import subprocess
import sys
import unittest
from simular_economia import referencia, simula, precio_compra, generador_nativo, elige
from validar_botin import carga

parser=argparse.ArgumentParser(add_help=False);parser.add_argument('--generador',type=Path);args,resto=parser.parse_known_args();sys.argv=[sys.argv[0]]+resto
MOTOR=Path(__file__).resolve().parents[1]

class SimuladorTests(unittest.TestCase):
    def modelo(self, orden=10, compra=20, venta=100, oro=0, items=None):
        p=dict(recoge_pct=[100]*5,vende_objetos_pct=venta,gastos=[0]*3,oficios_exito=[0]*3,margenes=[[0,1000]]*3,compras=[[dict(id='pan',cantidad=1,orden=compra)],[],[]])
        e=dict(saldo_inicial=0,venta_objetos_pct=25,canales=['cofres_visibles','combates','misiones','entorno','tesoros_ocultos'],oro_por_canal_pct=[25,50,75,0,0],presupuestos=[],precio_oficio_exito=8,oficio_id='servir_bar',perfiles={'prueba':p})
        d=dict(economia=e,equipo={},objetos={'pan':dict(price=8),'barato':dict(price=2),'npc':dict(shop=dict(items=['pan']))},niveles={'n.json':dict(objects=[dict(objectId='npc')])})
        i=dict(dia=1,canal='cofres_visibles',orden=orden,nivel='n.json',eventId='cofre',objectId='botin_ciudad_cofre',contenedor=dict(presupuesto=40))
        def genera(xs):return [dict(grupo=x['grupo'],oro=oro,objetos=items or []) for x in xs]
        return d,[i],genera
    def test_no_gasta_oro_de_un_cofre_futuro(self):
        d,i,g=self.modelo(orden=30,compra=20,oro=8);r=simula(d,i,g,{})['prueba']['dias'][0];self.assertEqual(8,r['saldo']);self.assertEqual([],r['comprado']);self.assertEqual('saldo insuficiente en ese momento',r['omitido'][0]['motivo'])
    def test_puede_comprar_despues_del_premio(self):
        d,i,g=self.modelo(oro=8);r=simula(d,i,g,{})['prueba']['dias'][0];self.assertEqual(0,r['saldo']);self.assertEqual(1,r['comprado'][0]['cantidad'])
    def test_venta_por_unidad_con_minimo_uno(self):
        d,i,g=self.modelo(items=['barato','barato']);r=simula(d,i,g,{})['prueba']['dias'][0];self.assertEqual(2,r['ventas']);self.assertEqual(2,r['saldo'])
    def test_no_vende_fracciones_de_objeto(self):
        d,i,g=self.modelo(venta=45,items=['pan']);r=simula(d,i,g,{})['prueba'];self.assertEqual(['pan'],r['conservado']);self.assertEqual(0,r['dias'][0]['ventas'])
    def test_sin_tendero_no_inventa_compra(self):
        d,i,g=self.modelo(oro=100);d['niveles']={};r=simula(d,i,g,{})['prueba']['dias'][0];self.assertEqual(100,r['saldo']);self.assertIn('ningún tendero',r['omitido'][0]['motivo'])
    def test_stock_explicito_manda_y_redondea(self):
        d,i,g=self.modelo(oro=100);d['objetos']['npc']['shop']['items']=[dict(objectId='pan',price=10)];r=simula(d,i,g,{},50)['prueba']['dias'][0];self.assertEqual(15,r['gasto']);self.assertEqual(85,r['saldo']);self.assertEqual(8,precio_compra(10,-25))
    def test_historia_no_cobra_escondites(self):
        d,i,g=self.modelo(oro=100);i[0]['canal']='tesoros_ocultos';d['economia']['perfiles']['prueba']['recoge_pct'][-1]=0;r=simula(d,i,g,{})['prueba']['dias'][0];self.assertEqual(0,r['ingresos'])
    def test_compra_parcial_sin_deuda(self):
        d,i,g=self.modelo(oro=20);d['economia']['perfiles']['prueba']['compras'][0][0]['cantidad']=3;r=simula(d,i,g,{})['prueba']['dias'][0];self.assertEqual(2,r['comprado'][0]['cantidad']);self.assertEqual(4,r['saldo'])
    def test_los_tres_objetivos_documentados_se_recalculan(self):
        e=carga(MOTOR)['economia'];r=referencia(e)
        for nombre,p in e['perfiles'].items():self.assertEqual(p['saldo_esperado'],r[nombre])
    def test_cestas_reales_cuadran_con_el_documento(self):
        d=carga(MOTOR);oficios=json.loads((MOTOR/'assets/catalogs/oficios.json').read_text())['oficios']
        for p in d['economia']['perfiles'].values():
            costes=[]
            for dia in p['compras']:
                importe=0
                for c in dia:
                    if c['id']=='reserva_historia':importe+=c['cantidad']
                    elif c['id'].startswith('oficio:'):importe+=oficios[c['id'][7:]]['coste']['oro']*c['cantidad']
                    else:importe+=d['objetos'][c['id']]['price']*c['cantidad']
                costes.append(importe)
            self.assertEqual(p['gastos'],costes)
    def test_porcentajes_invalidos_se_rechazan(self):
        d,i,g=self.modelo();d['economia']['perfiles']['prueba']['recoge_pct'][0]=101
        with self.assertRaisesRegex(ValueError,'Cobertura'):simula(d,i,g,{})
    def test_generador_real_reproducible_y_partidas_separadas(self):
        if not args.generador:self.skipTest('Indica --generador o usa CTest')
        xs=[dict(grupo=g,nivel='assets/levels/prueba.json',instancia='cofre',objectId='botin_cueva_cofre') for g in ['a','a','b']]
        a=generador_nativo(MOTOR,args.generador,17,xs);b=generador_nativo(MOTOR,args.generador,17,xs)
        self.assertEqual(a,b);self.assertEqual(a[0]['objetos'],a[1]['objetos']);self.assertEqual(a[0]['oro'],a[2]['oro'])
    def test_generador_real_rechaza_objeto_desconocido(self):
        if not args.generador:self.skipTest('Indica --generador o usa CTest')
        with self.assertRaisesRegex(ValueError,'Generador nativo'):generador_nativo(MOTOR,args.generador,17,[dict(grupo='a',nivel='n',instancia='i',objectId='inexistente')])

if __name__=='__main__':unittest.main()
