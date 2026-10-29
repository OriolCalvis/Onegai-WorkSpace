#!/usr/bin/env python3
"""AV18-04. Saldo de tres días con niveles y botín generado por el C++ del juego."""
from __future__ import annotations
import argparse
from collections import defaultdict
from fractions import Fraction
import json
from pathlib import Path
import subprocess
from validar_botin import carga, entero, exige, valida
from recompensas_economia import audita as audita_escenas, recorre, reparto, lee


def precio_base(d, id):
    eq = d['equipo'].get(id.removeprefix('item_')) or d['equipo'].get(id)
    return eq['price'] if eq else d['objetos'].get(id, {}).get('price', 0)


def precio_compra(base, ajuste=0):
    exige(entero(ajuste, -25, 50), 'Ajuste fuera de -25..50')
    return (base * (100 + ajuste) + 50) // 100


def referencia(e):
    """Referencia continua del documento: nunca se usa como oro de los niveles."""
    r = {}
    for nombre, p in e['perfiles'].items():
        saldo = Fraction(e['saldo_inicial']); dias = []
        exige(len(p['recoge_pct']) == 5 and all(entero(n, 0, 100) for n in p['recoge_pct']), 'Cobertura inválida')
        exige(entero(p['vende_objetos_pct'], 0, 100), 'Venta inválida')
        for dia in range(1, 4):
            ingreso = Fraction(0)
            for b in e['presupuestos']:
                if b['dia'] != dia: continue
                for v, recoge, oro in zip(b['valor'], p['recoge_pct'], e['oro_por_canal_pct']):
                    captado = Fraction(v * recoge, 100)
                    ingreso += captado * (Fraction(oro, 100) + Fraction(100-oro, 100) * Fraction(p['vende_objetos_pct'], 100) * Fraction(e['venta_objetos_pct'], 100))
            saldo += ingreso + p['oficios_exito'][dia-1] * e['precio_oficio_exito'] - p['gastos'][dia-1]
            dias.append(float(round(saldo, 2)))
        r[nombre] = dias
    return r


def ofertas(d):
    """Solo tiendas cuyo PNJ aparece en un nivel del proyecto, no un stock imaginado."""
    stock = {}
    for nivel in d['niveles'].values():
        for s in nivel.get('objects', []):
            tendero = d['objetos'].get(s.get('objectId'), {})
            for oferta in tendero.get('shop', {}).get('items', []):
                id = oferta if isinstance(oferta, str) else oferta['objectId']
                explicito = 0 if isinstance(oferta, str) else oferta.get('price', 0)
                precio = explicito if explicito > 0 else precio_base(d, id)
                if precio > 0: stock[id] = min(stock.get(id, precio), precio)
    return stock


def elige(instancias, p, canales):
    # Selección entera por presupuesto; no se fracciona un cofre ni un objeto.
    grupos = defaultdict(list)
    for i in instancias: grupos[i['dia'], i['canal']].append(i)
    elegidas = []
    for (_, canal), grupo in sorted(grupos.items()):
        limite = sum(i['contenedor']['presupuesto'] for i in grupo) * p['recoge_pct'][canales.index(canal)] // 100
        captado = 0
        for i in sorted(grupo, key=lambda i:(i['orden'],i['nivel'],i['eventId'])):
            if captado + i['contenedor']['presupuesto'] <= limite:
                elegidas.append(i); captado += i['contenedor']['presupuesto']
    return sorted(elegidas, key=lambda i:(i['dia'],i['orden'],i['nivel'],i['eventId']))


def generador_nativo(motor, ejecutable, semilla, solicitudes):
    if not solicitudes: return []
    exige(ejecutable.is_file(), 'Compila simular_contenedores o indica --generador')
    r = subprocess.run([str(ejecutable.resolve())], cwd=motor, input=json.dumps(dict(semilla=semilla,aperturas=solicitudes),ensure_ascii=False), text=True, capture_output=True, timeout=60)
    exige(r.returncode == 0, 'Generador nativo: ' + r.stderr.strip())
    datos = json.loads(r.stdout)
    exige(isinstance(datos, list) and len(datos) == len(solicitudes), 'Respuesta incompleta del generador')
    return datos


def combates_modelados(d, escenas):
    """AV18-04b. El canal «combates» lo paga el motor (LootResolver) al ganar, no las escenas, y el
    recorrido no libra combates: se modela. Cada victoria del recorrido vale el presupuesto del
    canal de su día repartido entre los combates POSIBLES de ese día (startBattle de las
    aventuras); el 50 % en oro (oro_por_canal_pct) y el resto en objetos de botin_objetos.json
    que caben, vendidos luego según el perfil. Hipótesis hasta que LootResolver use las tablas."""
    import glob as _glob
    canales=d['economia']['canales']; i_com=canales.index('combates')
    oro_pct=d['economia']['oro_por_canal_pct'][i_com]
    dias={'boundington_primer_dia.json':1,'boundington_segundo_dia.json':2,'boundington_ocaso.json':3}
    posibles=defaultdict(set)
    for archivo,dia in dias.items():
        try: a=json.loads((d['motor']/'assets/adventures'/archivo).read_text(encoding='utf-8'))
        except OSError: continue
        for b in a['beats']:
            for e in b.get('effects',[])+[e for o in b.get('opciones',[]) for e in o.get('effects',[])]:
                if e['type']=='startBattle' and e['startBattle'].get('flagVictory'): posibles[dia].add(e['startBattle']['flagVictory'])
    presupuesto={}
    for p in d['economia']['presupuestos']: presupuesto[p['dia']]=presupuesto.get(p['dia'],0)+p['valor'][i_com]
    baratos=sorted((b['id'] for b in d['botin'] if not b.get('unico') and 0<b['price']<=60), key=lambda i:precio_base(d,i))
    salida={}
    for perfil,r in (escenas or {}).items():
        flags=set(r.get('flags',[])); eventos=[]
        for dia in (1,2,3):
            ganados=[f for f in sorted(posibles[dia]) if f in flags]
            if not ganados or not posibles[dia]: continue
            valor=presupuesto[dia]//len(posibles[dia])
            for n,f in enumerate(ganados):
                oro=valor*oro_pct//100; resto=valor-oro; objetos=[]
                for id in baratos[::-1]:
                    if precio_base(d,id)<=resto: objetos.append(id); resto-=precio_base(d,id)
                    if resto<=0: break
                eventos.append(dict(dia=dia,orden=50+n,aventura='combate',beat=f,opcion='victoria (modelada)',oro=oro,objetos=objetos))
        salida[perfil]=dict(eventos=eventos,posibles={k:len(v) for k,v in posibles.items()},valor_por_combate={k:(presupuesto[k]//len(v) if v else 0) for k,v in posibles.items()})
    return salida


def simula(d, instancias, generar, oficios, ajuste=0, escenas=None):
    e=d['economia']; referencia(e); stock=ofertas(d); salida={}
    solicitudes=[]; elegidas={}
    for perfil,p in e['perfiles'].items():
        exige(len(p['compras']) == len(p['margenes']) == 3, 'Cestas o márgenes incompletos')
        elegidas[perfil]=elige(instancias,p,e['canales'])
        for i in elegidas[perfil]:
            solicitudes.append(dict(grupo=perfil,nivel='assets/levels/'+i['nivel'],instancia=i['eventId'],objectId=i['objectId']))
    resultados=iter(generar(solicitudes))
    for perfil,p in e['perfiles'].items():
        saldo=e['saldo_inicial'];inventario=[];premios_guardados=[];adquirido=vendido=0;dias=[];contenidos=[]
        for i in elegidas[perfil]:
            c=next(resultados);exige(c['grupo']==perfil,'Orden del generador distinto de solicitud');contenidos.append((i,c))
        for dia in range(1,4):
            movimientos=[]; ingresos=ventas=gasto=reservado=0;comprado=[];omitido=[];eventos=[]
            for i,c in contenidos:
                if i['dia']==dia:eventos.append((i['orden'],0,dict(tipo='cofre',instancia=i,contenido=c)))
            for j,t in enumerate((escenas or {}).get(perfil, {}).get('eventos', [])+(escenas or {}).get(perfil, {}).get('combates', [])):
                if t['dia']==dia:eventos.append((t['orden'],j,dict(tipo='escena',**t)))
            costes_escenas=0
            trabajos=p['oficios_exito'][dia-1]
            exige(entero(trabajos,0,3),'Demasiados turnos de oficio')
            if trabajos:eventos.append((5,0,dict(tipo='oficio',veces=trabajos)))
            for j,compra in enumerate(p['compras'][dia-1]):
                exige(entero(compra['cantidad'],1,100) and entero(compra['orden']), 'Compra inválida')
                eventos.append((compra['orden'],j+1,dict(tipo='compra',**compra)))
            for orden,_,evento in sorted(eventos,key=lambda x:(x[0],x[1])):
                if evento['tipo']=='cofre':
                    c=evento['contenido']; saldo+=c['oro'];ingresos+=c['oro']
                    for id in c['objetos']:
                        exige(id in d['objetos'], 'Objeto generado desconocido');inventario.append(id);adquirido+=precio_base(d,id)
                    objetivo=adquirido*p['vende_objetos_pct']//100
                    for id in sorted(inventario[:],key=lambda id:(precio_base(d,id),id)):
                        precio=precio_base(d,id)
                        if precio <= 0 or vendido+precio > objetivo:continue
                        cobro=max(1,precio*e['venta_objetos_pct']//100);saldo+=cobro;ventas+=cobro;vendido+=precio;inventario.remove(id)
                    movimientos.append(dict(orden=orden,fuente=evento['instancia']['eventId'],oro=c['oro'],objetos=c['objetos'],saldo=saldo))
                elif evento['tipo']=='escena':
                    oro=evento['oro'];exige(type(oro) is int,'Oro narrativo inválido')
                    cobro=min(saldo,-oro) if oro<0 else 0
                    saldo+=oro if oro>=0 else -cobro
                    ingresos+=max(0,oro);gasto+=cobro;costes_escenas+=cobro
                    for id in evento['objetos']:
                        exige(id in d['objetos'],'Premio narrativo desconocido: '+id)
                        premios_guardados.append(id)
                    movimientos.append(dict(orden=orden,fuente=evento['aventura']+':'+evento['beat'],opcion=evento['opcion'],oro=oro if oro>=0 else -cobro,solicitado=oro,objetos=evento['objetos'],saldo=saldo))
                elif evento['tipo']=='oficio':
                    oficio=oficios[e['oficio_id']];coste=oficio.get('coste',{}).get('oro',0)*evento['veces']
                    exige(oficio['duracion']*evento['veces']<=3,'Oficio sin tiempo disponible')
                    paga=oficio['paga']['exito']['oro']*evento['veces']
                    if saldo>=coste: saldo+=paga-coste;ingresos+=paga-coste;movimientos.append(dict(orden=orden,fuente=e['oficio_id'],oro=paga-coste,saldo=saldo,hipotesis='éxitos modelados; ejecución AV18-07 pendiente'))
                else:
                    id=evento['id'];cantidad=evento['cantidad']
                    if id=='reserva_historia':
                        cantidad=max(0,cantidad-costes_escenas)  # La cama recorrida no se cobra dos veces.
                        if saldo>=cantidad:saldo-=cantidad;reservado+=cantidad
                        else:omitido.append(dict(id=id,cantidad=cantidad,motivo='sin saldo para reservar'))
                        continue
                    if id.startswith('oficio:'):
                        base=oficios[id[7:]].get('coste',{}).get('oro',0);precio=base
                    else:
                        if id not in stock:omitido.append(dict(id=id,cantidad=cantidad,motivo='ningún tendero colocado lo ofrece'));continue
                        precio=precio_compra(stock[id],ajuste)
                    adquiridas=0
                    while adquiridas<cantidad and saldo>=precio:
                        saldo-=precio;gasto+=precio;adquiridas+=1
                    if adquiridas:comprado.append(dict(id=id,cantidad=adquiridas,precio=precio,orden=orden));movimientos.append(dict(orden=orden,fuente='compra:'+id,oro=-precio*adquiridas,saldo=saldo))
                    if adquiridas<cantidad:omitido.append(dict(id=id,cantidad=cantidad-adquiridas,motivo='saldo insuficiente en ese momento'))
            minimo,maximo=p['margenes'][dia-1]
            dias.append(dict(dia=dia,saldo=saldo,reservado=reservado,costes_escenas=costes_escenas,ingresos=ingresos,ventas=ventas,gasto=gasto,comprado=comprado,omitido=omitido,dentro_margen=minimo<=saldo<=maximo,margen=[minimo,maximo],movimientos=movimientos))
        salida[perfil]=dict(dias=dias,conservado=inventario+premios_guardados,premios_narrativos_conservados=premios_guardados,valor_conservado=sum(precio_base(d,id) for id in inventario+premios_guardados))
    return salida


def audita(motor):
    return dict(escenas=audita_escenas(motor),
                bucle_runtime='PO-49 en el motor: rentas una vez por día de historia (cobraRentasDelDia) y actuar una vez por sitio y día (Actuacion); los oficios entran con AV18-09',
                hipotesis='Resultados de tiradas y combates impuestos; moral neutral, nivel 1 y sin tiempo/desplazamiento; premios narrativos conservados. Compras a orden 90 antes del ocaso; falta disponibilidad y desplazamiento real.')


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--motor',type=Path,default=Path(__file__).resolve().parents[1]);p.add_argument('--generador',type=Path);p.add_argument('--semilla',type=int,default=17);p.add_argument('--ajuste',type=int,default=0,choices=[-25,0,50]);p.add_argument('--json',action='store_true');p.add_argument('--recorrido',type=Path);p.add_argument('--rutas',type=Path,default=Path(__file__).with_name('rutas_economia.json'));p.add_argument('--reparto',type=Path,help='Motor de Qwen: solo lectura, superposición de niveles en memoria');a=p.parse_args()
    try:
        motor=a.motor.resolve();exige(entero(a.semilla,1,1000000000),'Semilla inválida');d=carga(motor);niveles,contrato=reparto((a.reparto or motor).resolve(),d['objetos']);d['niveles']=niveles;r=valida(d)
        instancias=[dict(i,objectId=next(s['objectId'] for s in d['niveles'][i['nivel']]['objects'] if s.get('eventId')==i['eventId'])) for i in r['instancias']]
        oficios=json.loads((motor/'assets/catalogs/oficios.json').read_text())['oficios'];generador=a.generador or motor/'build-release/simular_contenedores'
        rutas=recorre(motor,a.recorrido or motor/'build-release/recorrer_recompensas',lee(a.rutas))
        d['motor']=motor
        for perfil,c in combates_modelados(d,rutas).items(): rutas[perfil]['combates']=c['eventos']; rutas[perfil]['combates_modelo']=dict(posibles=c['posibles'],valor_por_combate=c['valor_por_combate'])
        exige(set(rutas)==set(d['economia']['perfiles']),'Los escenarios deben cubrir los perfiles')
        perfiles=simula(d,instancias,lambda xs:generador_nativo(motor,generador,a.semilla,xs),oficios,a.ajuste,rutas)
        cumple=bool(instancias) and contrato['compatible'] and all(dia['dentro_margen'] for perfil in perfiles.values() for dia in perfil['dias'])
        # AV18-04b (16/10): el reparto es nativo (repartir_botin.py), las rutas salen de las aventuras
        # (sintetizar_rutas.py), el canal de combates se modela por victoria, la venta es el 25 % y PO-49
        # (rentas y actuar una vez por día) ya está en el motor. Los márgenes de ECONOMIA.md §8 son los
        # contrastados con esta simulación; los objetivos del 15/10 quedan en margenes_objetivo_15_10.
        informe=dict(semilla=a.semilla,ajuste=a.ajuste,instancias=len(instancias),referencia_teorica=referencia(d['economia']),perfiles=perfiles,auditoria=audita(motor),recorridos=rutas,reparto=contrato,calibrado=cumple,estado='calibrado: reparto nativo, rutas sintetizadas, combates modelados, 25 % y PO-49 en el motor' if cumple else 'pendiente de reparto compatible y calibración; no calibrado')
        if a.json:print(json.dumps(informe,ensure_ascii=False,indent=2))
        else:
            print(f"Simulación de niveles: {len(instancias)} contenedores; semilla {a.semilla}; ajuste {a.ajuste}%")
            for nombre,r in perfiles.items():
                print(nombre+': '+', '.join(f"día {x['dia']} saldo {x['saldo']} ({'dentro' if x['dentro_margen'] else 'FUERA'} del margen), {sum(c['cantidad'] for c in x['comprado'])} compras" for x in r['dias']))
            print(informe['estado'] + ('' if cumple else ' — no se certifican los tres perfiles.'))
            faltan=[k for k in ('afueras','cueva') if not any(i['region']==k for i in instancias)]
            if faltan: print('AVISO: sin instancias en', ', '.join(faltan), '(sus niveles aún no están en esta rama; repartir_botin.py los cubre al entrar).')
        return 0 if cumple else 2
    except (OSError,ValueError,KeyError,TypeError,StopIteration,subprocess.SubprocessError) as e:
        print('ERROR '+str(e));return 1

if __name__=='__main__':raise SystemExit(main())
