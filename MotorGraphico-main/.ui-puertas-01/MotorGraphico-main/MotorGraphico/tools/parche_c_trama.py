#!/usr/bin/env python3
"""Encargo C (Fable, 16/10): C1 Xila, C2 terceros caminos, C3 bloqueantes y «confunde».
Solo JSON de aventura. Idempotente: cada beat nuevo se sustituye si ya existe.
Uso: python3 parche_c1_c2.py <carpeta assets/adventures>
"""
import json, sys, copy
D = sys.argv[1].rstrip('/')
A = {}
for f in ('boundington_primer_dia', 'boundington_segundo_dia', 'boundington_ocaso'):
    A[f] = json.load(open(f'{D}/{f}.json', encoding='utf-8'))

def beats(f): return A[f]['beats']
def get(f, bid): return next(b for b in beats(f) if b['id'] == bid)
def put(f, b, after=None, before=None):
    lst = beats(f)
    lst[:] = [x for x in lst if x['id'] != b['id']]
    if after:  i = next(i for i, x in enumerate(lst) if x['id'] == after) + 1
    elif before: i = next(i for i, x in enumerate(lst) if x['id'] == before)
    else: i = len(lst)
    lst.insert(i, b)
def sf(a): return {"type": "setFlag", "arg": a}
def cf(a): return {"type": "clearFlag", "arg": a}
def log(t): return {"type": "log", "arg": t}
def gold(v): return {"type": "grantGold", "value": v}
def check(skill, rival, base, exitos=None):
    s = {"skillId": skill, "rivalDice": rival, "flagBotch": f"{base}_pifia", "flagPartial": f"{base}_parcial",
         "flagSuccess": f"{base}_exito", "flagCritical": f"{base}_critico"}
    if exitos: s["exitos"] = exitos
    return {"type": "skillCheck", "skillCheck": s}
def opt(texto, lines, effects, requires=None):
    assert len(texto) <= 60, texto
    o = {"texto": texto, "lines": lines, "effects": effects}
    if requires: o["requires"] = requires
    return o
def beat(bid, trigger, requires, lines, effects=(), opciones=None, speaker=""):
    b = {"id": bid, "trigger": trigger, "requires": requires, "speaker": speaker, "lines": list(lines),
         "effects": list(effects)}
    if opciones:
        assert len(opciones) <= 4, bid
        b["opciones"] = opciones
    return b
AUTO = {"type": "auto"}
def talk(t): return {"type": "talk", "target": t}
def enter(l): return {"type": "enter", "target": l}

D1, D2, D3 = 'boundington_primer_dia', 'boundington_segundo_dia', 'boundington_ocaso'

# ============================== C1 — Xila ==================================
# X-1: la ruta del trago deja un aliado debil (pendiente de recuperarse el dia 2).
b = get(D1, 'b1_xila_habla')
b['effects'] = [e for e in b['effects'] if not (e['type'] == 'setFlag' and e['arg'] == 'bnd1_aliado_xila_debil')]
b['effects'].insert(3, sf('bnd1_aliado_xila_debil'))
b['effects'][-1] = log('Pista: los Perdidos hacen sus rituales en la iglesia medio derruida del Casco Antiguo. '
                       'Xila te ha contado la historia de Killville hasta quedarse dormido: mañana, si lo despiertas a tiempo, aún puede estar de tu lado.')
# C3 #5: el desprecio se arregla a cualquier hora, y la opcion lo dice.
b = get(D1, 'b1_xila_resentido')
b['requires'].pop('variables', None)
b['opciones'][0]['texto'] = 'Invitarle a un trago (1) y hacer las paces'

# X-2: la resaca del dia 2 (segunda oportunidad: Persuasion o un desayuno pagado).
put(D2, beat('b2_xila_resaca', talk('xila'),
    {"allFlags": ["bnd2_encargo", "bnd1_xila_borracho"], "notFlags": ["bnd1_aliado_xila", "bnd2_xila_resaca_visto"]},
    ["Xila, hecho un ovillo contra el tonel de anoche, con la capa buena por encima y la boca abierta. Huele a lo que huele.",
     "Xila, sin abrir los ojos: «Si eres el tabernero, no tengo. Si eres otro, tampoco.»"],
    [sf('bnd2_xila_resaca_visto')],
    [opt("Despertarle y hablarle claro [Persuasión]",
         ["Le sacudes por el hombro hasta que abre un ojo. Le hablas de Venides, de la capa y de lo que se juega hoy la ciudad.",
          "(Tirada de Persuasión.)"],
         [check('persuasion', 2, 'bnd2_resaca')]),
     opt("Pagarle un desayuno caliente (6)",
         ["Pan, caldo y un cuenco de agua. Lo pones delante de él y esperas.",
          "Tarda. Come. Al tercer trago de caldo te mira como si acabara de verte."],
         [gold(-6), sf('bnd2_resaca_exito')]),
     opt("Dejarle dormir",
         ["Hoy no es su día. Puede que tampoco el tuyo."], [])]),
    after='b2_tienda_tras_xila')
put(D2, beat('b2_xila_resaca_bien', AUTO,
    {"anyFlags": ["bnd2_resaca_exito", "bnd2_resaca_critico"], "notFlags": ["bnd1_aliado_xila", "bnd2_xila_resaca_cerrada"]},
    ["Se levanta apoyándose en el tonel. Se pasa la mano por la cara y la mira, como si no fuera suya.",
     "Xila: «Me duele todo. Me lo tengo merecido. Dame un rato y estoy contigo: hay una tienda en Pico Dragón que te va a hacer falta.»",
     "Dobla la capa de Venides con cuidado y se la pone bajo el brazo."],
    [sf('bnd1_aliado_xila'), sf('bnd1_xila_honor'), cf('bnd1_xila_borracho'), sf('bnd2_xila_resaca_cerrada'),
     {"type": "acto", "arg": "xila_sin_alcohol"},
     log('Xila está de tu lado. Sobrio, o casi.')]),
    after='b2_xila_resaca')
put(D2, beat('b2_xila_resaca_mal', AUTO,
    {"anyFlags": ["bnd2_resaca_pifia", "bnd2_resaca_parcial"], "notFlags": ["bnd1_aliado_xila", "bnd2_xila_resaca_cerrada", "bnd2_xila_resaca_mal_visto"]},
    ["Xila: «Vete. Vete con tu caballero y tus prisas.»",
     "Se da la vuelta hacia la pared. El tabernero tuerto, desde la barra: «Con el estómago vacío no hay quien le hable. Con un caldo dentro, a veces sí.»"],
    [sf('bnd2_xila_resaca_mal_visto')],
    [opt("Pagarle el desayuno de todas formas (6)",
         ["Pan, caldo y un cuenco de agua. Lo dejas a su lado y te sientas a esperar sin decir nada.",
          "Tarda. Come. Al tercer trago de caldo te mira como si acabara de verte."],
         [gold(-6), sf('bnd2_resaca_exito')]),
     opt("Dejarle",
         ["Que duerma. La tienda de Pico Dragón tendrás que encontrarla tú solo."],
         [sf('bnd2_xila_resaca_cerrada'), log('Xila no vendrá. La tienda abandonada de Pico Dragón sigue ahí: pregunta por el barrio.')])]),
    after='b2_xila_resaca_bien')
# La tienda sin Xila tambien vale si eres su aliado pero no has hablado con el en el escalon.
b = get(D2, 'b2_tienda_abandonada')
b['requires'] = {"allFlags": ["bnd2_encargo"], "notFlags": ["bnd2_tienda_vista", "bnd2_xila_pista", "bnd2_xila_escalon_visto"]}

# X-3: el fallo de la Investigacion en la tienda (parcial: un traje gastado; pifia: segunda tirada o comprarlo).
put(D2, beat('b2_disfraz_parcial', AUTO,
    {"allFlags": ["bnd2_tienda_vista"], "anyFlags": ["bnd2_disfraz_parcial"], "notFlags": ["bnd2_disfraz_resuelto"]},
    ["Entre los baúles vacíos, un solo traje entero: de actor, con la costura del hombro abierta y olor a polvo de teatro.",
     "No es elegante. Es lo que hay. En el forro de la capa, un juego de bolas de malabares."],
    [sf('bnd2_disfraz'), sf('bnd2_disfraz_actor'), sf('bnd2_disfraz_resuelto'),
     {"type": "darObjeto", "arg": "bolas_de_malabares"},
     log('Un disfraz de actor, gastado. Servirá si nadie mira de cerca.')]),
    after='b2_disfraz_hallado')
put(D2, beat('b2_disfraz_pifia', AUTO,
    {"allFlags": ["bnd2_tienda_vista"], "anyFlags": ["bnd2_disfraz_botch"], "notFlags": ["bnd2_disfraz_resuelto", "bnd2_disfraz_pifia_vista"]},
    ["Pisas una tabla podrida y el suelo se abre bajo tu pie. Un clavo, una rasgadura y una nube de polvo que no te deja ver nada.",
     "Cuando se asienta, los baúles que no has mirado siguen ahí. Y una rata te observa desde el espejo roto."],
    [sf('bnd2_disfraz_pifia_vista'), {"type": "herida", "arg": "la tabla podrida", "value": 4}],
    [opt("Volver a mirar, con calma [Investigación]",
         ["Esta vez vas baúl por baúl, sin fiarte del suelo.", "(Tirada de Investigación.)"],
         [check('investigacion', 3, 'bnd2_disfraz2')]),
     opt("Irte: Luisarda vende capas",
         ["Sales sacudiéndote el polvo. La Boutique de Madame, en el centro, tiene ropa de gala. Cara."],
         [sf('bnd2_disfraz_resuelto'),
          log('Sin disfraz. Luisarda, en La Boutique de Madame, vende trajes de gala.')])]),
    after='b2_disfraz_parcial')
put(D2, beat('b2_disfraz_segunda_bien', AUTO,
    {"anyFlags": ["bnd2_disfraz2_exito", "bnd2_disfraz2_critico"], "notFlags": ["bnd2_disfraz_resuelto"]},
    ["En el último baúl, debajo de un telón doblado: un traje de noble excéntrico de Ascaria, de colores vivos, entero.",
     "En el bolsillo interior, una entrada de ópera sin usar."],
    [sf('bnd2_disfraz'), sf('bnd2_disfraz_noble'), sf('bnd2_disfraz_resuelto'), {"type": "darObjeto", "arg": "entrada_opera"}]),
    after='b2_disfraz_pifia')
put(D2, beat('b2_disfraz_segunda_mal', AUTO,
    {"anyFlags": ["bnd2_disfraz2_pifia", "bnd2_disfraz2_parcial"], "notFlags": ["bnd2_disfraz_resuelto"]},
    ["Telones, polvo y ratones. Lo que hubo aquí se lo llevó alguien antes que tú.",
     "Si quieres entrar en esa fiesta, la ropa habrá que comprarla. O cogerla prestada."],
    [sf('bnd2_disfraz_resuelto'),
     log('Sin disfraz. Luisarda, en La Boutique de Madame, vende trajes de gala.')]),
    after='b2_disfraz_segunda_bien')
# X-4: el disfraz sin Xila y sin tienda: comprarlo o robarlo en la Boutique.
put(D2, beat('b2_luisarda_disfraz', talk('luisarda'),
    {"allFlags": ["bnd2_encargo"], "notFlags": ["bnd2_disfraz", "bnd2_luisarda_disfraz_visto"], "variables": {"franja": {"max": 1}}},
    ["La Boutique de Madame. Luisarda, con alfileres en la boca, te mira de arriba abajo.",
     "Luisarda: «Esta noche hay fiesta en el Barrio Alto y tú tienes cara de querer entrar. Tengo un traje de gala. Cuarenta. No regateo con el que me salvó.»"],
    [sf('bnd2_luisarda_disfraz_visto')],
    [opt("Comprar el traje de gala (40)",
         ["Un traje sobrio y bien cortado, de los que no se miran dos veces. Luisarda te lo ajusta en un momento.",
          "Luisarda: «Y las manos quietas en esa fiesta, que ese forro lo cosí yo.»"],
         [gold(-40), sf('bnd2_disfraz'), sf('bnd2_disfraz_musico'), sf('bnd2_disfraz_resuelto'),
          log('Un traje de gala de la Boutique de Madame.')]),
     opt("Llevártelo del probador [Sigilo]",
         ["Luisarda se vuelve hacia la trastienda. El probador está a dos pasos.", "(Tirada de Sigilo.)"],
         [check('sigilo', 3, 'bnd2_boutique')]),
     opt("Hoy no",
         ["Luisarda: «Pues la fiesta es hoy. Tú sabrás.»"], [])]),
    after='b2_disfraz_segunda_mal')
put(D2, beat('b2_boutique_robo_bien', AUTO,
    {"anyFlags": ["bnd2_boutique_exito", "bnd2_boutique_critico"], "notFlags": ["bnd2_disfraz"]},
    ["Sales con el traje bajo la capa y Luisarda sigue con los alfileres en la boca. No te ha visto. Todavía."],
    [sf('bnd2_disfraz'), sf('bnd2_disfraz_musico'), sf('bnd2_disfraz_resuelto'), {"type": "acto", "arg": "robo_en_la_fiesta"},
     log('Un traje de gala robado de la Boutique de Madame.')]),
    after='b2_luisarda_disfraz')
put(D2, beat('b2_boutique_robo_mal', AUTO,
    {"anyFlags": ["bnd2_boutique_pifia", "bnd2_boutique_parcial"], "notFlags": ["bnd2_disfraz", "bnd2_boutique_pillado"]},
    ["Luisarda, sin volverse: «Ese probador cruje. Lo sé porque lo puse yo.»",
     "Te quita el traje de las manos y te lo vuelve a ofrecer, esta vez con cara de piedra.",
     "Luisarda: «Cuarenta. Y hoy sí regateo: cuarenta por el traje y por el susto.»"],
    [sf('bnd2_boutique_pillado')],
    [opt("Pagar (40)", ["Pagas. Luisarda guarda las monedas sin contarlas y te ajusta el traje con más alfileres de los necesarios."],
         [gold(-40), sf('bnd2_disfraz'), sf('bnd2_disfraz_musico'), sf('bnd2_disfraz_resuelto')]),
     opt("Irte", ["Luisarda: «La puerta está donde la dejaste.»"], [])]),
    after='b2_boutique_robo_bien')

# ======================= C2 — terceros caminos ============================
# Gitanos: la Persuasion fallida se arregla con las manos (parcial) o con grano y manos (pifia).
put(D2, beat('b2_gitanos_recelo', AUTO,
    {"allFlags": ["bnd2_gitanos_hablado"], "anyFlags": ["bnd2_gitanos_parcial"], "notFlags": ["bnd2_zarela_confia", "bnd2_gitanos_recelo_visto"]},
    ["Guardián: «Palabras. Aquí sobran palabras.»",
     "Señala con la barbilla los cubos vacíos junto a la empalizada, y el río, a cien pasos."],
    [sf('bnd2_gitanos_recelo_visto')],
    [opt("Cargar agua del río hasta llenar los cubos",
         ["Veinte viajes. Los niños te siguen el tercero; al décimo te ayudan. Al último, el guardián te quita el cubo de las manos.",
          "Guardián: «Siéntate. Zarela quiere verte.»"],
         [{"type": "pasarFranja", "value": 1}, sf('bnd2_zarela_confia'), sf('bnd2_aliado_gitanos'), sf('bnd2_tiene_aliados'),
          sf('bnd2_gitanos_resuelto'), sf('bnd2_zarela_abierta'),
          log('ALIADO: los gitanos de Surysal saldrán por el oeste. Te lo has ganado con los brazos, no con la boca.'),
          {"type": "grantLevel", "value": 0.25, "arg": "la alianza con los gitanos de Surysal"}]),
     opt("Irte", ["Guardián: «Eso pensaba.»"], [])]),
    after='b2_gitanos_alianza')
put(D2, beat('b2_gitanos_puerta_cerrada', AUTO,
    {"allFlags": ["bnd2_gitanos_hablado"], "anyFlags": ["bnd2_gitanos_botch"], "notFlags": ["bnd2_zarela_confia", "bnd2_gitanos_puerta_vista"]},
    ["Guardián: «Vete. Ya han venido hoy tres como tú, con la misma cara de buena gente.»",
     "La empalizada se cierra. Detrás, alguien afila algo despacio.",
     "Un niño, por una rendija: «Al abuelo le faltan tablas y grano. Siempre le faltan.»"],
    [sf('bnd2_gitanos_puerta_vista')],
    [opt("Volver con un saco de grano (10) y cargar tablas",
         ["Compras el grano en el mercado y vuelves. El guardián no abre; tú dejas el saco y te pones a cargar tablas del río sin que nadie te lo pida.",
          "A la tercera tabla, la empalizada se abre.", "Guardián: «Zarela dice que pases. Yo digo que no te fíes de que lo diga dos veces.»"],
         [gold(-10), {"type": "pasarFranja", "value": 1}, sf('bnd2_zarela_confia'), sf('bnd2_aliado_gitanos'), sf('bnd2_tiene_aliados'),
          sf('bnd2_gitanos_resuelto'), sf('bnd2_zarela_abierta'),
          log('ALIADO: los gitanos de Surysal saldrán por el oeste. Te costó grano, tablas y una franja del día.')]),
     opt("Irte", ["La empalizada se queda cerrada. Hay más gente en la ciudad."], [])]),
    after='b2_gitanos_recelo')

# Secta (dia 1): tras el fallo, seguirles de lejos.
put(D1, beat('b1_sectarios_seguir', talk('sectario_perdido'),
    {"allFlags": ["bnd1_sectarios_resueltos"], "notFlags": ["bnd1_invitado_guarida", "bnd_pista_sectarios", "bnd1_sectarios_seguir_visto"]},
    ["Los tres sectarios recogen sus cosas sin prisa y echan a andar hacia el Casco Antiguo, en fila, sin hablar."],
    [sf('bnd1_sectarios_seguir_visto')],
    [opt("Seguirles de lejos [Sigilo]",
         ["Dejas veinte pasos. Ellos no se vuelven: la gente que se cree con razón nunca mira atrás.", "(Tirada de Sigilo.)"],
         [check('sigilo', 2, 'bnd1_seguir')]),
     opt("Dejarles ir", ["Que se vayan. La ciudad es pequeña."], [])]),
    after='b1_sectarios_nada')
put(D1, beat('b1_sectarios_seguidos', AUTO,
    {"anyFlags": ["bnd1_seguir_exito", "bnd1_seguir_critico"], "notFlags": ["bnd_pista_sectarios"]},
    ["Entran por una puerta lateral de la iglesia medio caída del Casco Antiguo. Uno se queda fuera, de guardia, con las manos en las mangas.",
     "Desde la esquina cuentas a once más que entran antes de que anochezca."],
    [sf('bnd_pista_sectarios'), {"type": "addVariable", "arg": "pistas", "value": 1},
     log('Pista: los Perdidos se reúnen en la iglesia medio caída del Casco Antiguo. Hay un guardia en la puerta lateral.')]),
    after='b1_sectarios_seguir')
put(D1, beat('b1_sectarios_te_ven', AUTO,
    {"anyFlags": ["bnd1_seguir_pifia", "bnd1_seguir_parcial"], "notFlags": ["bnd_pista_sectarios", "bnd1_sectarios_te_vieron"]},
    ["A la segunda esquina, el mayor se para y se vuelve. Te mira sin odio, que es peor.",
     "Sectario mayor: «La luz también ve de espaldas, forastero.»",
     "Se van por una calle que no es la suya. Esta noche no vas a saber dónde duermen."],
    [sf('bnd1_sectarios_te_vieron'), log('Te han visto seguirles. Habrá que encontrar la iglesia por otro lado: pregunta en la taberna o a Xila.')]),
    after='b1_sectarios_seguidos')

# Gorran: lo que pasa cuando el Sigilo no es critico (antes: nada, y empezaba el combate sin decirlo).
put(D2, beat('b2_gorran_sigilo_casi', AUTO,
    {"allFlags": ["bnd2_gorran_resuelto"], "anyFlags": ["bnd2_sigilo_exito", "bnd2_sigilo_parcial"], "notFlags": ["bnd2_cueva_cerrada", "bnd2_gorran_lanzado", "bnd2_gorran_sigilo_dicho"]},
    ["Llegas a la piedra. Cierras la mano sobre la empuñadura y la Furia está tibia, como si acabaran de soltarla.",
     "El perro levanta la cabeza. Un ladrido. Gorran se gira con la calma de quien ya ha visto esto antes.",
     "Gorran: «Esa no se coge así. Esa se gana.»"],
    [sf('bnd2_gorran_sigilo_dicho')]),
    before='b2_gorran_combate')
put(D2, beat('b2_gorran_sigilo_torpe', AUTO,
    {"allFlags": ["bnd2_gorran_resuelto"], "anyFlags": ["bnd2_sigilo_botch"], "notFlags": ["bnd2_cueva_cerrada", "bnd2_gorran_lanzado", "bnd2_gorran_sigilo_dicho"]},
    ["Un cacharro de latón rueda escaleras abajo con un estruendo que despierta hasta al perro.",
     "Gorran, sin volverse del todo: «Si vas a robarme, al menos no me rompas la vajilla.»",
     "Silba. De detrás de las lonas salen sus bandidos."],
    [sf('bnd2_gorran_sigilo_dicho')]),
    before='b2_gorran_combate')

# ====================== C3 — bloqueantes y «confunde» ======================
# #3 La fiesta: avisar al irse, y una vuelta pagando al portero.
for bid in ('b2_mansion', 'b2_dentro', 'b2_invitados'):
    b = get(D2, bid)
    for o in b['opciones']:
        if o['texto'].startswith('Irte') and any(e.get('arg') == 'bnd2_fiesta_resuelta' for e in o['effects']):
            o['texto'] = 'Irte (esta noche no podrás volver sin pagar)'
put(D2, beat('b2_fiesta_vuelta', talk('skilla'),
    {"allFlags": ["bnd2_encargo", "bnd2_fiesta_resuelta", "bnd2_mansion_vista"],
     "notFlags": ["bnd2_documentos", "bnd2_documentos_fiesta", "bnd2_fiesta_vuelta_usada"], "variables": {"franja": 2}},
    ["Skilla, secando una jarra: «¿Ya has vuelto del Barrio Alto? Esas fiestas duran hasta el alba. El portero de la mansión cobra por olvidar caras: diez, me han dicho.»"],
    [],
    [opt("Volver y pagar al portero (10)",
         ["Diez monedas y el portero mira al otro lado. La música sigue sonando arriba como si no te hubieras ido."],
         [gold(-10), sf('bnd2_fiesta_vuelta_usada'), cf('bnd2_fiesta_resuelta'), cf('bnd2_mansion_vista'), cf('bnd2_dentro_visto'),
          cf('bnd2_invitados_visto'), sf('bnd2_en_la_mansion'), log('Vuelves a la fiesta del Barrio Alto. Esta vez no hay más vueltas.')]),
     opt("Dejarlo estar", ["Skilla: «Tú sabrás. Yo no entraría ahí ni pagada.»"], [sf('bnd2_fiesta_vuelta_usada')])]),
    after='b2_invitados')

# #12 La cueva antes de hablar con Skilla.
put(D2, beat('b2_cueva_sin_encargo', enter('assets/levels/cueva_gorran.json'),
    {"allFlags": ["bnd2_encargo"], "notFlags": ["bnd2_skilla_encargo", "bnd2_cueva_entrada", "bnd2_cueva_sin_encargo_vista"]},
    ["Una cueva en la ladera de Pico Dragón, pasando el árbol torcido. Una hoguera, lonas tendidas y un perro atado que te enseña los dientes.",
     "Bandido de guardia: «¿Y tú qué buscas? Aquí no se viene a mirar. Vuelve cuando tengas algo que decir, o alguien que te mande.»",
     "Te cierra el paso con el cuerpo. Detrás, en la sombra de la cueva, brilla algo largo sobre una piedra."],
    [sf('bnd2_cueva_sin_encargo_vista'), log('Los bandidos de la cueva no te dejan pasar sin motivo. Rosita y Skilla, en la Taberna Humilde, saben algo de una espada.')]),
    before='b2_cueva_llegada')
b = get(D2, 'b2_cueva_llegada')
b['lines'][2] = b['lines'][2].replace(' Privadísima.', '')
# #8 Gorran: avisar al matarlo, y que el Ocaso lo note.
b = get(D2, 'b2_gorran_victoria')
for o in b['opciones']:
    if o['texto'] == 'Acabar con él':
        o['texto'] = 'Acabar con él (no volverá a ayudarte)'
b['lines'][1] = 'Gorran: «Vale, vale. Tú ganas. Llévate la espada. Y los papeles, que son del que me la encargó, no míos.»'
put(D3, beat('b3_gorran_sin_jefe', AUTO,
    {"allFlags": ["bnd3_afueras_vistas", "bnd2_gorran_muerto"], "notFlags": ["bnd3_gorran_sin_jefe_visto"]},
    ["En el camino de las afueras, tres de los bandidos de Gorran saquean un carro volcado. Sin jefe, sin orden y con mucha prisa.",
     "Uno te reconoce. Deja caer el saco."],
    [sf('bnd3_gorran_sin_jefe_visto'),
     {"type": "startBattle", "startBattle": {"monsterId": "gorran_bandido", "enemigos": [{"id": "gorran_bandido", "cantidad": 3}],
                                             "flagVictory": "bnd3_bandidos_vencidos", "flagDefeat": "bnd3_bandidos_ganan", "flagHuida": "bnd3_bandidos_huido"}}]),
    after='b3_gorran')
put(D3, beat('b3_bandidos_vencidos', AUTO,
    {"allFlags": ["bnd3_bandidos_vencidos"], "notFlags": ["bnd3_bandidos_cerrado"]},
    ["Los que quedan en pie huyen monte arriba. En el carro volcado, lo que no se llevaron: veinte monedas sueltas y un odre de agua."],
    [sf('bnd3_bandidos_cerrado'), gold(20), log('Recibes 20 de oro (el carro de los bandidos sin jefe).')]),
    after='b3_gorran_sin_jefe')
put(D3, beat('b3_bandidos_otros', AUTO,
    {"anyFlags": ["bnd3_bandidos_ganan", "bnd3_bandidos_huido"], "notFlags": ["bnd3_bandidos_cerrado"]},
    ["Te apartas del camino con lo puesto. Los bandidos no te siguen: tienen un carro que vaciar."],
    [sf('bnd3_bandidos_cerrado')]),
    after='b3_bandidos_vencidos')

# #9 Verina: segunda oportunidad ante Sandes.
put(D3, beat('b3_verina_segunda', AUTO,
    {"allFlags": ["bnd3_verina_abandonada", "bnd3_sandes_abierta"], "notFlags": ["bnd3_verina_libre", "bnd3_verina_muerta", "bnd3_verina_segunda_vista"]},
    ["Detrás de Sandes, entre sus hombres, la mujer de la barricada: Verina, con las manos atadas y la mirada fija en él.",
     "Sandes, sin volverse: «Ella se queda conmigo. Es la única que entendió lo que hacíamos.»",
     "Verina no dice nada. Te mira a ti."],
    [sf('bnd3_verina_segunda_vista')],
    [opt("Liberarla ahora, cueste lo que cueste",
         ["Te echas sobre los que la sujetan. Son más que en la barricada, y esta vez te esperaban."],
         [cf('bnd3_verina_cerrada'), cf('bnd3_verina_abandonada'),
          {"type": "startBattle", "startBattle": {"monsterId": "cultista_carcelero",
             "enemigos": [{"id": "cultista_carcelero", "cantidad": 2}, {"id": "perdido_devoto", "cantidad": 1}],
             "flagVictory": "bnd3_verina_libre", "flagDefeat": "bnd3_verina_muerta",
             "acompanantes": [{"id": "venides", "siFlag": "bnd3_con_venides"}, {"id": "xila", "siFlag": "bnd3_con_xila"}]}}]),
     opt("Dejarla donde está", ["Ya la dejaste una vez. La segunda pesa menos. O eso te dices."], [])]),
    after='b3_sandes_oferta')

# #10 La barca: los epilogos van antes de poder cerrar los ojos (orden del fichero) y la opcion lo exige.
oc = beats(D3)
barcas = [b for b in oc if b['id'].startswith('b3_barca')]
oc[:] = [b for b in oc if not b['id'].startswith('b3_barca')] + barcas
for b in barcas:
    for o in b.get('opciones', []):
        if o['texto'] == 'Cerrar los ojos':
            o['texto'] = 'Cerrar los ojos (fin de la aventura)'

# #7 Venides lee la confianza del dia 1.
for bid in ('b2_venides_noche', 'b2_venides_noche_rosita', 'b2_venides_noche_todo', 'b2_venides_noche_rosita2'):
    b = get(D2, bid)
    ops = b['opciones']
    todo = ops[0]
    todo.setdefault('requires', {}).setdefault('notFlags', []).append('bnd1_venides_desconfianza')
    con_pruebas = copy.deepcopy(todo)
    con_pruebas['texto'] = 'Contárselo todo y enseñarle las pruebas'
    con_pruebas['requires'] = {"allFlags": ["bnd1_venides_desconfianza"], "anyFlags": ["bnd2_documentos", "bnd2_documentos_fiesta"]}
    con_pruebas['lines'] = ["Venides: «Ayer no te fiabas de mí. Hoy traes papeles. Me gusta más esto.»"] + todo['lines'][1:]
    parcial = ops[1]
    parcial.setdefault('requires', {}).setdefault('anyFlags', []).append('bnd1_venides_desconfianza')
    b['opciones'] = [todo, con_pruebas, parcial, ops[2]]
    assert len(b['opciones']) == 4

# #14 (pule, barato): el epilogo lee como acabas.
b = get(D3, 'b3_epi_sandes')
b.setdefault('lines', [])
put(D3, beat('b3_epi_compania', AUTO,
    {"allFlags": ["bnd3_fuera", "bnd3_epi_empezado", "bnd3_compania"], "notFlags": ["bnd3_epilogo", "b3_epi_compania_dicho"]},
    ["No saliste solo. Eso, en una ciudad que ardía, ya es una historia."],
    [sf('b3_epi_compania_dicho')], speaker='Sho-Noco'), after='b3_epi_sandes')
put(D3, beat('b3_epi_solo', AUTO,
    {"allFlags": ["bnd3_fuera", "bnd3_epi_empezado", "bnd3_final_solos"], "notFlags": ["bnd3_epilogo", "b3_epi_solo_dicho", "bnd3_compania"]},
    ["Saliste solo. Hay quien dice que así se sale siempre. Yo no estoy tan seguro."],
    [sf('b3_epi_solo_dicho')], speaker='Sho-Noco'), after='b3_epi_compania')

for f, a in A.items():
    json.dump(a, open(f'{D}/{f}.json', 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    open(f'{D}/{f}.json', 'a', encoding='utf-8').write('\n')
print('parche aplicado:', {f: len(a['beats']) for f, a in A.items()})
