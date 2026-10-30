#!/usr/bin/env python3
"""AV19-08 (Fable, 19/10) — La cripta de los Perdidos en cinco mapas: trama, cinemáticas y gente.

Oriol: «quan entrem al sotano de la iglesia i ens trobem a Edul i Verina, la cinemàtica ha de
saltar quan explorem i entrem a la sala d'estudi dels novicis. Ampliar la història i missió:
combats, engany o tirades per infiltrar-se, o que t'hagin convidat; la cinemàtica d'Edul i
Verina si entres allà fora de combat; i després Edul sempre s'ha d'oferir a acompanyar-te.»

Solo JSON (aventura, cinemáticas, catálogos, niveles) y tools/actores_pnj.py. Idempotente.
Uso: python3 tools/parche_secta.py            (desde la raíz del motor)
"""
import json, sys, copy
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ADV = ROOT / "assets/adventures/boundington_primer_dia.json"
CIN = ROOT / "assets/cinematicas/cinematicas.json"
NPC = ROOT / "assets/objects/boundington_npcs.json"
LV = ROOT / "assets/levels"

def lee(p): return json.loads(p.read_text(encoding="utf-8"))
def guarda(p, d, indent=1):
    p.write_text(json.dumps(d, ensure_ascii=False, indent=indent) + "\n", encoding="utf-8")

# ----------------------------------------------------------------- aventura
A = lee(ADV)
def beats(): return A["beats"]
def get(bid): return next(b for b in beats() if b["id"] == bid)
def put(b, after=None):
    lst = beats()
    lst[:] = [x for x in lst if x["id"] != b["id"]]
    i = next(i for i, x in enumerate(lst) if x["id"] == after) + 1 if after else len(lst)
    lst.insert(i, b)
    return b["id"]
def sf(a): return {"type": "setFlag", "arg": a}
def cf(a): return {"type": "clearFlag", "arg": a}
def log(t): return {"type": "log", "arg": t}
def pista(): return {"type": "addVariable", "arg": "pistas", "value": 1}
def nivel(v, t): return {"type": "grantLevel", "value": v, "arg": t}
def check(skill, rival, base):
    return {"type": "skillCheck", "skillCheck": {"skillId": skill, "rivalDice": rival, "flagBotch": f"{base}_pifia",
            "flagPartial": f"{base}_parcial", "flagSuccess": f"{base}_exito", "flagCritical": f"{base}_critico"}}
def pelea(devotos, carceleros, victoria):
    en = []
    if devotos: en.append({"id": "perdido_devoto", "cantidad": devotos})
    if carceleros: en.append({"id": "cultista_carcelero", "cantidad": carceleros})
    return {"type": "startBattle", "startBattle": {"monsterId": en[0]["id"], "enemigos": en, "flagVictory": victoria,
            "flagDefeat": "bnd1_guarida_perdida", "flagHuida": "bnd1_guarida_huida"}}
def opt(texto, lines, effects, requires=None, speaker=None):
    assert len(texto) <= 60, texto
    o = {"texto": texto, "lines": list(lines), "effects": list(effects)}
    if requires: o["requires"] = requires
    if speaker is not None: o["speaker"] = speaker
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
def enter(l): return {"type": "enter", "target": f"assets/levels/{l}.json"}

DEP, SAN, CAM = "dependencias_secta", "santuario_subterraneo", "camara_profunda"
HOSTIL = "bnd1_cripta_hostil"
A_LAS_BRAVAS = [sf(HOSTIL), log("Has peleado en la cripta de los Perdidos. Ya saben que estás aquí.")]

# --- 1. Las dependencias: llegar invitado, o colarse ---------------------------------------
DEP_DESC = ("El pasadizo del osario se abre a las dependencias de los Perdidos, excavadas en la roca: literas, "
            "un archivo con un atril, un almacén que huele a aceite y, al fondo, la sala donde los novicios "
            "copian las Escrituras.")
last = put(beat("b1_dep_llegada_dentro", enter(DEP),
    {"allFlags": ["bnd1_guarida_dentro"], "notFlags": ["bnd1_dep_vista", HOSTIL]},
    [DEP_DESC,
     "Un novicio con un candil te sale al paso: «¿Nuevo? La ceremonia aún no ha empezado. Espera en la sala "
     "de estudio, con los demás. Y no toques el archivo.»"],
    [sf("bnd1_dep_vista"), sf("bnd1_dep_novicio_aviso")]), after="b1_guarida_puerta")
last = put(beat("b1_dep_llegada_colarse", enter(DEP),
    {"notFlags": ["bnd1_guarida_dentro", "bnd1_dep_vista", HOSTIL]},
    [DEP_DESC, "Nadie te ha visto bajar. Al fondo, voces y el rasgar de plumas sobre papel."],
    [],
    [opt("Pasar sin que te vean [Sigilo]", ["(Tirada de Sigilo.)"], [sf("bnd1_dep_vista"), check("sigilo", 2, "bnd1_colarse")]),
     opt("Hacerte pasar por novicio [Engaño]", ["(Tirada de Engaño.)"], [sf("bnd1_dep_vista"), check("engano", 2, "bnd1_novicio")]),
     opt("Entrar a las bravas", ["El primero que te ve es un guardia con una maza. No pregunta."],
         [sf("bnd1_dep_vista")] + A_LAS_BRAVAS + [pelea(2, 1, "bnd1_cripta_ganada")]),
     opt("Volver arriba", ["Subes sin hacer ruido. El osario guarda el secreto."], [])]), after=last)
last = put(beat("b1_colarse_exito", AUTO,
    {"anyFlags": ["bnd1_colarse_exito", "bnd1_colarse_critico"], "notFlags": ["bnd1_colarse_cerrado"]},
    ["Pegado a la pared, de sombra en sombra. Un novicio pasa a un palmo con un candil y no levanta la vista."],
    [sf("bnd1_colarse_cerrado"), sf("bnd1_cripta_colado"), nivel(0.2, "colarte en la cripta de los Perdidos")]), after=last)
last = put(beat("b1_colarse_parcial", AUTO,
    {"allFlags": ["bnd1_colarse_parcial"], "notFlags": ["bnd1_colarse_cerrado"]},
    ["A medio pasillo, un novicio con un fardo se para en seco.", "Novicio: «¿Y tú? No te he visto en el banco.»"],
    [sf("bnd1_colarse_cerrado")],
    [opt("Soy nuevo. Me manda el herrero. [Engaño]", ["(Tirada de Engaño.)"], [check("engano", 2, "bnd1_novicio")]),
     opt("Taparle la boca", ["Grita antes de que llegues. Y el guardia de la sala de estudio llega antes que tú."],
         A_LAS_BRAVAS + [pelea(1, 1, "bnd1_cripta_ganada")])]), after=last)
last = put(beat("b1_colarse_pifia", AUTO,
    {"allFlags": ["bnd1_colarse_pifia"], "notFlags": ["bnd1_colarse_cerrado"]},
    ["Tropiezas con un cubo de ceniza. Tres cabezas se giran a la vez."],
    [sf("bnd1_colarse_cerrado")] + A_LAS_BRAVAS + [pelea(2, 1, "bnd1_cripta_ganada")]), after=last)
last = put(beat("b1_novicio_exito", AUTO,
    {"anyFlags": ["bnd1_novicio_exito", "bnd1_novicio_critico"], "notFlags": ["bnd1_novicio_cerrado"]},
    ["«Otro del herrero. Pues al banco, que ya sois cuatro esta semana.»",
     "Te dan un candil y una túnica que huele a humo. Nadie vuelve a mirarte."],
    [sf("bnd1_novicio_cerrado"), sf("bnd1_cripta_colado"), sf("bnd1_cripta_tunica"),
     nivel(0.2, "hacerte pasar por novicio de los Perdidos")]), after=last)
last = put(beat("b1_novicio_parcial", AUTO,
    {"allFlags": ["bnd1_novicio_parcial"], "notFlags": ["bnd1_novicio_cerrado"]},
    ["Te mira de arriba abajo. «Al banco. Pero Sandes querrá verte luego.»", "No te cree del todo. Tampoco le importa."],
    [sf("bnd1_novicio_cerrado"), sf("bnd1_cripta_colado"), sf("bnd1_cripta_sospecha")]), after=last)
last = put(beat("b1_novicio_pifia", AUTO,
    {"allFlags": ["bnd1_novicio_pifia"], "notFlags": ["bnd1_novicio_cerrado"]},
    ["«¿El herrero? El herrero no manda a nadie que no conozcamos.» Y grita."],
    [sf("bnd1_novicio_cerrado")] + A_LAS_BRAVAS + [pelea(2, 1, "bnd1_cripta_ganada")]), after=last)
last = put(beat("b1_cripta_ganada", AUTO,
    {"allFlags": ["bnd1_cripta_ganada"], "notFlags": ["bnd1_cripta_ganada_vista"]},
    ["Los guardias caen. El pasillo se queda en silencio, pero no vacío: al fondo siguen las voces, y alguien ha dejado de cantar.",
     "Los que quedan abajo no van a subir a buscarte. Van a esperar."],
    [sf("bnd1_cripta_ganada_vista"), sf("bnd1_guarida_charlada"), nivel(0.3, "abrirte paso en la cripta")]), after=last)

# --- 2. La sala de estudio: Edul y Verina (la cinemática cripta_estudio enciende bnd1_cripta_estudio) --
last = put(beat("b1_estudio_vuelta", AUTO,
    {"allFlags": ["bnd1_cripta_estudio", "bnd1_edul_hablado"], "notFlags": ["bnd1_estudio_visto"]},
    ["La sala de estudio. Edul ya está dentro, con Verina y su libro.", "Edul: «Aquí no. Aquí leen.»"],
    [sf("bnd1_estudio_visto")]), after=last)
last = put(beat("b1_estudio", AUTO,
    {"allFlags": ["bnd1_cripta_estudio"], "notFlags": ["bnd1_estudio_visto", "bnd1_edul_hablado"]},
    ["La sala de estudio: tres mesas, tinta, pliegos clavados en la pared con la misma frase copiada cien veces. "
     "En el banco, una chica con un libro abierto en las rodillas levanta la vista y la baja enseguida.",
     "Por la puerta entra un hombre con un frasco en la mano y las manos de alguien que trabaja el hierro. "
     "Se para en seco al verte.",
     "Edul: «Tú no deberías estar aquí.» Y a la chica, bajito: «Verina, sigue leyendo.»"],
    [sf("bnd1_estudio_visto"), sf("bnd1_verina_vista"), sf("bnd1_con_edul")]), after=last)
# b1_edul (ya existía) sigue a bnd1_con_edul: «Escucha. No sé qué eres...». Dos retoques:
b = get("b1_edul")
for o in b["opciones"]:
    o["lines"] = [l.replace("Sandes, desde el estrado:", "Sandes, desde la escalera:") for l in o["lines"]]
    if o["texto"] == "Salir con él" and not any(e.get("arg") == "bnd1_edul_escolta_si" for e in o["effects"]):
        o["effects"].insert(0, sf("bnd1_edul_escolta_si"))
# Edul siempre se ofrece a acompañarte (Oriol): justo después, y cada vez que le hablas.
last = put(beat("b1_edul_oferta", AUTO,
    {"allFlags": ["bnd1_edul_hablado", "bnd1_guarida_resuelta"],
     "notFlags": ["bnd1_edul_escolta_si", "bnd1_edul_oferta_vista", "bnd1_guarida_pelea", "bnd1_edul_escoltado"]},
    ["Cuando quieras subir, dímelo. Te acompaño hasta la calle; aquí abajo nadie sube solo la primera vez."],
    [sf("bnd1_edul_oferta_vista")], speaker="Edul"), after="b1_edul")
last = put(beat("b1_edul_escolta", talk("edul_cripta"),
    {"allFlags": ["bnd1_edul_hablado"], "notFlags": ["bnd1_edul_escolta_si", "bnd1_guarida_pelea", "bnd1_edul_escoltado"]},
    ["¿Subes? Te acompaño hasta la calle."],
    [],
    [opt("Sí, acompáñame", ["«Pues vamos. Y no mires atrás, que aquí abajo eso se nota.»"],
         [sf("bnd1_edul_escolta_si"), sf("bnd1_guarida_resuelta"), log("Edul te saca de la cripta.")]),
     opt("Todavía no", ["«Como quieras. Estaré aquí.»"], [])], speaker="Edul"), after=last)
last = put(beat("b1_verina_cripta", talk("verina_cripta"),
    {"allFlags": ["bnd1_estudio_visto"], "notFlags": ["bnd1_verina_cripta_hablada"]},
    ["Dicen que la luz se gana leyendo. Yo leo. Todavía no la veo.",
     "(Mira a Edul antes de seguir.) No le digas que te he dicho eso."],
    [sf("bnd1_verina_cripta_hablada"), sf("bnd1_verina_vista")], speaker="Verina"), after=last)

# --- 3. El santuario: la ceremonia (ya existía) ahora está en su mapa -----------------------
b = get("b1_guarida_ceremonia")
b["trigger"] = enter(SAN)
b["requires"] = {"anyFlags": ["bnd1_guarida_dentro", "bnd1_cripta_colado"], "notFlags": ["bnd1_guarida_vista", HOSTIL]}
b["lines"][0] = ("El conducto baja en espiral, con antorchas nuevas cada diez pasos, hasta una cueva tallada a mano. "
                 "El canto te llega antes que la luz.")
last = put(beat("b1_santuario_hostil", enter(SAN),
    {"allFlags": [HOSTIL], "notFlags": ["bnd1_guarida_vista"]},
    ["El santuario: el ídolo de cuernos retorcidos, las brasas a sus pies, los bancos en semicírculo. Vacíos.",
     "El canto se cortó cuando empezó la pelea. Los que quedan han preferido la oscuridad a huir, y te esperan en ella."],
    [sf("bnd1_guarida_vista"), sf("bnd_pruebas_suficientes"), log("PRUEBA: el santuario de la cripta y su ídolo."),
     nivel(0.3, "entrar en la guarida de los Perdidos")]), after="b1_guarida_ceremonia")
last = put(beat("b1_santuario_intruso", enter(SAN),
    {"notFlags": ["bnd1_guarida_vista", "bnd1_guarida_dentro", "bnd1_cripta_colado", HOSTIL]},
    ["Bajas al santuario sin que nadie te haya dado permiso: el ídolo, las brasas, veinte Perdidos en los bancos.",
     "Y veinte cabezas que se giran."],
    [sf("bnd1_dep_vista")],
    [opt("Sentarte en un banco como si nada [Engaño]", ["(Tirada de Engaño.)"], [check("engano", 2, "bnd1_novicio")]),
     opt("Pelear", ["Los de los bancos no se mueven. Los de la puerta, sí."], A_LAS_BRAVAS + [pelea(2, 1, "bnd1_cripta_ganada")])]),
    after=last)

# --- 4. La cámara profunda: Sandes y el sereno encadenado ----------------------------------
CAM_DESC = ("La cámara más honda. Un pozo, runas en el suelo y, encadenado a la pared, un hombre con la casaca azul "
            "de los serenos de la ciudad.")
last = put(beat("b1_camara_sandes", enter(CAM),
    {"notFlags": [HOSTIL, "bnd1_camara_vista"]},
    [CAM_DESC, "Sandes está delante de él, sin tocarlo, hablándole bajito.",
     "Sandes, sin volverse: «Los novicios no bajan aquí. Pero tú no eres novicio, ¿verdad?»"],
    [sf("bnd1_camara_vista"), sf("bnd1_sabe_sandes"), pista(),
     log("Sandes guarda a un sereno encadenado en lo más hondo de la cripta.")],
    [opt("Me he perdido.", ["«Nadie se pierde hacia abajo. Sube. Y no cuentes lo que has visto, porque no has visto nada.»"],
         [sf("bnd1_sandes_te_conoce")], speaker="Sandes"),
     opt("¿Quién es ese hombre?",
         ["«Un sereno con buena vista y mala suerte. Mañana lo soltamos. ¿Verdad que sí?» El sereno no contesta.", "«Sube.»"],
         [sf("bnd1_sabe_sereno"), sf("bnd1_sandes_te_conoce")], speaker="Sandes"),
     opt("Soltarlo ahora mismo", ["Sandes suspira. «Qué pena. Me caías bien.»", "Dos carceleros salen de la sombra."],
         [sf("bnd1_sandes_enfadado"), sf("bnd1_guarida_pelea")] + A_LAS_BRAVAS + [pelea(0, 2, "bnd1_camara_ganada")])]),
    after=last)
last = put(beat("b1_camara_hostil", enter(CAM),
    {"allFlags": [HOSTIL], "notFlags": ["bnd1_camara_vista"]},
    [CAM_DESC, "Sandes no está. El sereno te mira como si fueras el primero que baja en días."],
    [sf("bnd1_camara_vista"), log("En lo hondo de la cripta hay un sereno encadenado.")]), after=last)
last = put(beat("b1_camara_ganada", AUTO,
    {"allFlags": ["bnd1_camara_ganada"], "notFlags": ["bnd1_camara_ganada_vista"]},
    ["Los carceleros caen. Sandes ya no está: ni por la escalera ni por ningún sitio que veas.",
     "Sandes, desde la oscuridad: «Ya lo había previsto. Lo tengo todo previsto.»"],
    [sf("bnd1_camara_ganada_vista"), sf("bnd1_guarida_charlada"), nivel(0.3, "plantar cara a Sandes en la cripta")]), after=last)
SOLTAR = opt("Soltarle las cadenas", ["Las cadenas ceden al tercer golpe. Se frota las muñecas y no da las gracias: sube corriendo."],
             [sf("bnd1_sereno_libre"), nivel(0.3, "liberar al sereno de la cripta"),
              log("Has liberado al sereno. Si llega arriba, la torre sabrá lo de las barricadas.")],
             requires={"allFlags": [HOSTIL]})
last = put(beat("b1_sereno", talk("prisionero_cripta"),
    {"notFlags": ["bnd1_sereno_hablado"]},
    ["Vi los carros de noche. Barriles, lona, y a ese hablando de «cuando arda». Al puente y a Las Afueras, eso dijo.",
     "Si subes, díselo a alguien de la torre. Y si no subes, que al menos lo sepas."],
    [sf("bnd1_sereno_hablado"), sf("bnd1_sabe_barricadas"), sf("bnd1_pista_guarida"), pista(),
     log("Pista: el sereno encadenado oyó dónde irán las barricadas: el puente y Las Afueras.")],
    [SOLTAR, opt("Dejarlo", ["«Ya. Como todos.»"], [])], speaker="Sereno encadenado"), after=last)
last = put(beat("b1_sereno_otra_vez", talk("prisionero_cripta"),
    {"allFlags": ["bnd1_sereno_hablado"], "notFlags": ["bnd1_sereno_libre"]},
    ["¿Vas a soltarme o no?"],
    [], [copy.deepcopy(SOLTAR), opt("Todavía no", ["«Ya. Como todos.»"], [])], speaker="Sereno encadenado"), after=last)

guarda(ADV, A)

# ------------------------------------------------------------- cinemáticas
C = lee(CIN)
MIOS = ("cripta_estudio", "edul_escolta", "edul_escolta_fuera")
C["cinematicas"] = [c for c in C["cinematicas"] if c["id"] not in MIOS]
SALIDA = [{"funde": 1, "segundos": 0.8}, {"nivel": "assets/levels/ciudad_centro.json", "en": [56, 53]},
          {"espera": 0.2}, {"funde": 0, "segundos": 1.4}]
C["cinematicas"] += [
    {"id": "cripta_estudio",
     "_nota": "AV19-08 — Oriol: la escena de Edul y Verina salta al entrar en la sala de estudio de los novicios "
              "(dependencias_secta, losa clara 8..14 × 15..21), fuera de combate. Verina lee en el banco; Edul entra "
              "por el pasillo con su frasco. Lo que se dice: b1_estudio → b1_edul.",
     "nivel": f"assets/levels/{DEP}.json", "zona": [8, 15, 14, 21], "sin": ["bnd1_guarida_pelea", "bnd1_cripta_estudio"],
     "pasos": [{"coloca": "edul_cripta", "en": [7, 17]}, {"mira": "verina_cripta", "hacia": "arriba"},
               {"espera": 0.3}, {"anda": "edul_cripta", "a": [10, 19], "velocidad": 1.6},
               {"mira": "edul_cripta", "hacia": "derecha"}, {"mira": "jugador", "hacia": "izquierda"},
               {"emocion": "verina_cripta", "icono": "asustado", "segundos": 1.0},
               {"espera": 0.4}, {"beat": "bnd1_cripta_estudio"}],
     "alTerminar": ["bnd1_cripta_estudio"]},
    {"id": "edul_escolta",
     "_nota": "AV19-08 — «Edul siempre se ofrece a acompañarte a salvo»: desde las dependencias, Edul va contigo "
              "hasta la galería del osario y salís a la calle, junto a la puerta de la iglesia.",
     "nivel": f"assets/levels/{DEP}.json", "requiere": ["bnd1_edul_escolta_si"], "sin": ["bnd1_edul_escoltado"],
     "pasos": [{"anda": "edul_cripta", "a": [4, 28], "velocidad": 2.0, "sinEsperar": True},
               {"anda": "jugador", "a": [5, 28], "velocidad": 2.5}, {"mira": "edul_cripta", "hacia": "abajo"},
               {"espera": 0.3}] + SALIDA,
     "alTerminar": ["bnd1_edul_escoltado"]},
    {"id": "edul_escolta_fuera",
     "_nota": "AV19-08 — La misma salida con Edul desde cualquier otro mapa de la cripta (santuario, osario).",
     "requiere": ["bnd1_edul_escolta_si"], "sin": ["bnd1_edul_escoltado"],
     "pasos": SALIDA, "alTerminar": ["bnd1_edul_escoltado"]},
]
# Dejar la cinemática genérica detrás de la de las dependencias.
pos = {c["id"]: i for i, c in enumerate(C["cinematicas"])}
assert pos["edul_escolta"] < pos["edul_escolta_fuera"]
AP = {
    "perdido_devoto": {"_nota": "AV19-08: en la cripta solo cuando has peleado; el día 3, siempre.",
                       "visibleSi": {"alguna": [HOSTIL, "bnd2_dia_cerrado"]}},
    "cultista_carcelero": {"visibleSi": {"alguna": [HOSTIL, "bnd2_dia_cerrado"]}},
    "edul_cripta": {"_nota": "AV19-08: el primer día, en la sala de estudio de la cripta.",
                    "ocultoSi": {"alguna": ["bnd1_guarida_pelea", "bnd1_edul_escoltado", "bnd_dia_cerrado"]}},
    "verina_cripta": {"ocultoSi": {"alguna": ["bnd1_guarida_pelea", "bnd1_verina_salvada_guarida", "bnd_dia_cerrado"]}},
    "sandes_cripta": {"ocultoSi": {"alguna": [HOSTIL, "bnd_dia_cerrado"]}},
    "prisionero_cripta": {"ocultoSi": {"alguna": ["bnd1_sereno_libre", "bnd_dia_cerrado"]}},
}
C["apariciones"] = [a for a in C["apariciones"] if a["objeto"] not in AP]
for k, v in AP.items():
    C["apariciones"].append({"objeto": k, **v})
guarda(CIN, C, indent=2)

# ---------------------------------------------------------------- catálogo
N = lee(NPC)
NUEVOS = [
    {"id": "edul_cripta", "name": "Edul", "category": "npc", "spriteId": 6, "blocksMovement": True, "interactable": True,
     "dialogue": ["Ahora no. Luego."]},
    {"id": "verina_cripta", "name": "Verina", "category": "npc", "spriteId": 6, "blocksMovement": True, "interactable": True,
     "dialogue": ["Sigo leyendo."]},
    {"id": "sandes_cripta", "name": "Sandes", "category": "npc", "spriteId": 6, "blocksMovement": True, "interactable": True,
     "dialogue": ["Sube."]},
    {"id": "prisionero_cripta", "name": "Sereno encadenado", "category": "npc", "spriteId": 6, "blocksMovement": True,
     "interactable": True, "dialogue": ["..."]},
]
ids = {o["id"] for o in NUEVOS}
N["objects"] = [o for o in N["objects"] if o["id"] not in ids] + NUEVOS
guarda(NPC, N, indent=2)

# ----------------------------------------------------------------- niveles
def coloca(nombre, cambios, nuevos):
    p = LV / f"{nombre}.json"
    L = lee(p)
    for o in L["objects"]:
        if (o["objectId"], o["position"]["x"], o["position"]["y"]) in cambios:
            o["objectId"] = cambios[(o["objectId"], o["position"]["x"], o["position"]["y"])]
    for oid, x, y in nuevos:
        L["objects"] = [o for o in L["objects"] if o["objectId"] != oid]
        L["objects"].append({"objectId": oid, "position": {"x": x, "y": y}})
    guarda(p, L, indent=2)
coloca(CAM, {("edul", 14, 8): "sandes_cripta", ("verina_joven", 21, 8): "prisionero_cripta"}, [])
coloca(DEP, {}, [("edul_cripta", 10, 19), ("verina_cripta", 12, 20)])

# ----------------------------------------------------------- actores_pnj.py
PNJ = ROOT / "tools/actores_pnj.py"
src = PNJ.read_text(encoding="utf-8")
MARCA = "# AV19-08 (19/10/2026) — la cripta en cinco mapas"
BLOQUE = f'''
{MARCA}: Edul y Verina en la sala de estudio (con la capucha
# de la cripta), Sandes en la cámara honda y el sereno encadenado (su casaca azul, sin lo demás).
PNJ.update({{
    "edul_cripta": P("Edul", AEG, "m", **_EDUL,
                     equipo={{"cabeza": "capucha_purpura", "torso": "camisa_chaleco_granate", "piernas": "pantalon_pardo",
                             "pies": "botas", "mano_izq": "pocion"}}, _unico=True),
    "verina_cripta": P("Verina", AEG, "f", **_VERINA,
                       equipo={{"cabeza": "capucha_purpura", "torso": "vestido_azul", "pies": "zapatos", "mano_izq": "grimorio"}},
                       _unico=True),
    "sandes_cripta": P("Sandes", AEG, "m", **_SANDES,
                       equipo={{"torso": "casaca_granate", "piernas": "calzas_rojas", "pies": "botas_altas",
                               "capa": "capa_corta_roja"}}, _unico=True),
    "prisionero_cripta": P("Sereno encadenado", AEG, "m", ojos="marron", pelo=("greñas", "pelo_castano"),
                           barba=("BARBA_CORTA", "pelo_castano"),
                           equipo={{"torso": "casaca_azul", "piernas": "pantalon_gris"}}, _unico=True),
}})
'''
if MARCA not in src:
    PNJ.write_text(src.rstrip("\n") + "\n" + BLOQUE, encoding="utf-8")

print("parche aplicado:", len(beats()), "beats;", len(C["cinematicas"]), "cinemáticas;", len(N["objects"]), "PNJ")
