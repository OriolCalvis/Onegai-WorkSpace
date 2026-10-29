#!/usr/bin/env python3
"""AV18-06: cifras de referencia y cartas nativas; no cambia la IA del combate."""
from pathlib import Path
import copy
import json

MOTOR=Path(__file__).resolve().parents[1]
EXTRA_RANGO={'criatura':0,'elite':2,'jefe':4}
CAMPANA=[
 ('perdido_saqueador','Cuchillada del saqueador','Corte sin limpiar','sangrado',False),
 ('perdido_fanatico','Golpe del fanático','Polvo de los Perdidos','cegado',False),
 ('perdido_devoto','Golpe del devoto','Grito del culto','asustado',False),
 ('cultista_carcelero','Golpe del carcelero','Zancadilla del carcelero','caido',False),
 ('gorran_bandido','Tajo de Gorran','Finta del bandido','guardia_rota',True),
 ('saqueador_local','Golpe de Surysal','Corte de retirada','marcado',False),
 ('bandido_de_gorran','Golpe de bandido','Zancadilla de la cueva','caido',False),
 ('sandes','Tajo de Sandes','Mirada de Sandes','asustado',True),
 ('guardia_comprado','Golpe de guardia','Abrir la guardia','guardia_rota',False),
 ('edul_furioso','Golpe de Edul','Grito furioso','marcado',True)]
PREVISTOS=[('lobo_famelico','Mordisco famélico','Derribo famélico','caido'),
 ('lobo_de_cripta','Mordisco de cripta','Herida de cripta','sangrado'),
 ('rata_de_reliquia','Mordisco de reliquia','Mordisco infectado','envenenado'),
 ('naga_de_contrato','Golpe de contrato','Cláusula quebrada','guardia_rota'),
 ('acolito_de_penumbra','Golpe de penumbra','Susurro amenazante','asustado')]


def es_control(id, indice):
    # Cuatro excepciones revisadas contra el texto de la carta: la posición
    # de la técnica no determina siempre si pega o solo controla.
    if id=='itx_matriarca_de_la_sal':return indice==0
    if indice==0 and id in {'el_portero_de_los_mil_marcos','la_matriarca_del_pozo_azul'}:return True
    return indice==1


def magnitud(tier,rango,control=False):
    if rango not in EXTRA_RANGO or not 1<=tier<=5:raise ValueError('Tier/rango fuera de la fórmula')
    return 0 if control else 2*tier+EXTRA_RANGO[rango]


def guardar(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')


def generar():
    p=MOTOR/'assets/catalogs/monsters.json';monstruos=json.loads(p.read_text(encoding='utf-8'))
    for m in monstruos['entries']:
        for i,a in enumerate(m['attacks']):
            # El control puro declara cero, también en las excepciones revisadas.
            a['magnitude']=magnitud(m['tier'],m['rank'],es_control(m['id'],i))
    monstruos['_formula_av1806']='Nueva referencia editorial: 2*tier + rango (criatura 0, elite 2, jefe 4); control puro 0 (con excepciones por id revisadas). magnitudeHint se carga, pero el combate no lo usa como daño plano. No es la fórmula de vida de DEUDA §7.'
    por_id={m['id']:m for m in monstruos['entries']}
    pskills=MOTOR/'assets/catalogs/skills.json';original=pskills.read_text(encoding='utf-8');skills=json.loads(original)
    base=next(x for x in skills['entries'] if x['id']=='bnd_unico_tiza_carta')
    nuevas=[];filas=[]
    def cartas(id,nombre,ataque,control,cond,jefe,tier,tipo):
        defs=[(id+'_golpe',ataque,'ataque_arma','Ataque de arma con los dados físicos y defensas enfrentadas del combatiente.'),
              (id+'_control',control,'condicion:'+cond+':1','Si gana la tirada enfrentada, aplica '+cond+' durante una ronda.')]
        if jefe:defs.append((id+'_impulso','Tomar la iniciativa: '+nombre,'propio:2:ventaja:ataque','Ganas ventaja al atacar durante dos rondas. Consume la acción principal.'))
        ids=[]
        for sufijo,n,hook,desc in defs:
            c=copy.deepcopy(base);c.update(id='bnd_enemigo_'+sufijo,name=n,description=desc,flavorText='',tags=['MonstruoBoundington'],mechanicTags=['MonstruoBoundington'],target='self' if hook.startswith('propio') else 'single_enemy',tier=tier,range='melee',duration='instant',castingStat='DES',effects=[dict(description=desc,mechanicHook=hook)],effect=dict(description=desc,scaling='none'))
            nuevas.append(c);ids.append(c['id'])
        filas.append(dict(id=id,name=nombre,tier=tier,rarity='common',tipo_ficha=tipo,skillIds=ids,reservado=tipo=='monstruo',condicion=cond))
        return ids
    op=MOTOR/'assets/objects/boundington_enemigos.json';od=json.loads(op.read_text(encoding='utf-8'));objetos={o['id']:o for o in od['objects']}
    for id,at,co,cond,jefe in CAMPANA:
        o=objetos[id];o['combat']['skills']=cartas(id,o['name'],at,co,cond,jefe,1,'objeto')
    for id,at,co,cond in PREVISTOS:
        m=por_id[id];m['skillIds']=cartas(id,m['name'],at,co,cond,False,m['tier'],'monstruo')
    guardar(MOTOR/'assets/catalogs/monsters.json',monstruos);guardar(op,od)
    # Mantener los bytes de las cartas anteriores (sangría de una columna).
    existentes={x['id'] for x in skills['entries']};agregadas=[x for x in nuevas if x['id'] not in existentes]
    if agregadas:
        pos=original.rfind(']');texto=',\n'.join('\n'.join('  '+line for line in json.dumps(x,ensure_ascii=False,indent=1).splitlines()) for x in agregadas)
        pskills.write_text(original[:pos].rstrip()+',\n'+texto+'\n '+original[pos:],encoding='utf-8')
    guardar(MOTOR/'assets/catalogs/monstruos_boundington_cartas.json',dict(entries=filas,estados_pendientes=['quemado','congelado','aturdido','silenciado'],ejecucion_enemiga='pendiente de puente nativo/legado de Claude; no se cambia combate ni Application'))

if __name__=='__main__':generar()
