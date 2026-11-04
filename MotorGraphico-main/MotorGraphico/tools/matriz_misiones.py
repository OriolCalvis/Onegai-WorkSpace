#!/usr/bin/env python3
"""matriz_misiones.py — E-MISIONES-01: inventario de las MISIONES de un dia de Boundington.

La matriz de decisiones (matriz_decisiones.py) solo mira lo que pesa en los dias siguientes. Esta mira TODO el dia:
cada mision = un grupo de beats con un comienzo (el gancho: el primer beat que la abre) y un cierre (las flags que la
dan por hecha). Las misiones se definen en la tabla MISIONES de abajo (patrones sobre el id del beat, en orden: el
primero que encaja manda); una prueba exige que NINGUN beat quede fuera ni caiga en dos misiones. Lo demas se calcula
de los JSON, sin mirar el juego:

  * PNJ y nivel donde empieza, como se abre (trigger + requires), como se cierra (los beats que encienden el cierre y
    las salidas de sus tiradas y combates: exito, parcial, pifia, victoria, derrota, huida);
  * recompensa: oro (neto y por beat), objetos, nivel, actos (moral), heridas;
  * diario: los objetivos de la aventura y las misiones de assets/catalogs/quests.json cuya `doneFlag` enciende la mision;
  * cobertura del explorador (explorar_boundington): beats vistos / total y lo que no ha visto nadie, leido del log
    (--log; por defecto out/fabrica/UNI-02-11/explorar_20_post.log);
  * ¿SE PUEDE CERRAR?: analisis estatico de alcanzabilidad. Una flag es alcanzable si alguien puede encenderla: un beat
    cuyo `requires` positivo (allFlags, y al menos una de anyFlags) ya es alcanzable, o el exterior (el heroe creado, las
    cinematicas, los eventos del mundo, el codigo, otros dias). Un beat de `talk` ademas necesita que el PNJ exista en
    un nivel y que `apariciones` de cinematicas.json no lo esconda con lo que el propio beat exige. Es optimista con las
    `notFlags` y las variables (una cota superior): lo que sale ⚠ es inalcanzable seguro, con fichero:linea.

Uso (desde MotorGraphico/):
    python3 tools/matriz_misiones.py [--dia primer_dia] [--md salida.md] [--json salida.json] [--log explorador.log]

Reutilizable para los otros dias: añadir su tabla a TABLAS. ensure_ascii=False en todo lo que escribe. Solo stdlib.
"""
import argparse
import json
import os
import re
import sys
from collections import defaultdict, OrderedDict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import matriz_decisiones as md  # noqa: E402  (el lector de JSON con lineas y los efectos normalizados)

RAIZ = md.RAIZ
LOG_POR_DEFECTO = os.path.normpath(os.path.join(RAIZ, "..", "..", "out", "fabrica", "UNI-02-11", "explorar_20_post.log"))

# ---------------------------------------------------------------------------------------------------------------
# La tabla de misiones del dia 1. (id, nombre, tipo, patrones, beat_que_abre, flags_de_cierre)
#   tipo: "mision" = tiene un cierre propio (una flag que nadie mas enciende por casualidad);
#         "charla" = un PNJ o una escena de ambiente: se cierra al despedirse, y su flag de «visto» es lo unico que queda.
# ---------------------------------------------------------------------------------------------------------------
M = lambda i, n, t, p, a, c: dict(id=i, nombre=n, tipo=t, patrones=p, abre=a, cierre=c)
MISIONES_PRIMER_DIA = [
    M("llegada_cama", "La primera noche: cama y rumores en la Taberna Humilde", "mision",
      [r"b0_llegada$", r"b0_se_oye", r"b0_rosita", r"b0_subiendo", r"b0_subir", r"b0_calle", r"b0_puerta_cerrada$",
       r"b1_skilla_hora$", r"b1_posadero_hora$"], "b0_llegada", ["bnd0_dormido"]),
    M("ventana", "Los vecinos de la ventana", "charla", [r"b0_(bordadora|carretero|marido|ventana)"], "b0_bordadora", ["bnd0_ventana_visto"]),
    M("jornaleros", "El rincon de los jornaleros (Tomas, Berto y el tercero)", "mision",
      [r"b0_(tomas|berto|tercero|rincon)", r"b1_rincon_", r"beat_parroquiano$"], "b0_tomas", ["bnd0_rincon_hecho", "bnd_rumor_parroquiano"]),
    M("encapuchado", "El hombre de la capucha", "charla", [r"b0_capucha"], "b0_capucha", ["bnd0_capucha_vista"]),
    M("venides_noche", "El caballero que escribe (Venides, de noche)", "charla", [r"b0_venides_(?!x)", r"b0_venides$"], "b0_venides", ["bnd0_venides_visto"]),
    M("desayuno", "El desayuno con Venides y su encargo", "mision",
      [r"b0_puerta_cerrada_venides", r"b1_desayuno", r"b1_venides_llama", r"b1_venides_dia$"], "b1_desayuno", ["bnd1_desayuno_hecho"]),
    M("presentaciones", "Presentarte a los tenderos y la emboscada de la plaza", "mision",
      [r"beat_llegada$", r"b1_reloj$", r"tut_(encargo|tendero|joyero|sastre|emboscada|victoria|derrota|huida)"],
      "beat_llegada", ["bnd_tut_cerrado"]),
    M("plaza", "Recorrer la plaza y escuchar al vecindario", "mision",
      [r"tut_plaza", r"b1_vecina_plaza", r"b1_vieja"], "tut_plaza", ["bnd1_plaza_explorada"]),
    M("carterista", "El carterista de la plaza (la pulga)", "mision", [r"b1_carterista", r"b1_pulga"], "b1_carterista", ["bnd1_carterista_resuelto"]),
    M("aigren", "Aigren, el herrero (por su hijo o por los Perdidos)", "mision", [r"b1_aigren"], "b1_aigren_charla", ["bnd1_aigren_resuelto"]),
    M("luisarda", "Luisarda, la sastra: cenar con Rosita o sonsacarla", "mision",
      [r"b1_luisarda", r"b1_rosita_luisarda", r"b1_sala", r"b1_sastre_cobro"], "b1_luisarda_tienda", ["bnd1_luisarda_con_rosita", "bnd1_luisarda_vista", "bnd1_luisarda_dejada"]),
    M("sectarios", "Los sectarios de la plaza (la bolsa y la invitacion)", "mision", [r"b1_secta", r"b1_sectarios"], "b1_secta", ["bnd1_sectarios_resueltos"]),
    M("xila", "Xila, el antiguo mercader de Pico Dragon", "mision", [r"b1_xila"], "b1_xila_desconocido", ["bnd1_xila_resuelto"]),
    M("familia", "La familia de la puerta azul (el gato, los Verdaderos y la cena)", "mision",
      [r"b1_mira", r"b1_verdaderos", r"b1_familia", r"b1_gato", r"b1_fuego", r"rec_fuego"], "b1_mira", ["bnd1_familia_resuelta"]),
    M("iglesia", "Las pruebas de la Iglesia de la Luz y el sacerdote", "mision",
      [r"beat_iglesia$", r"b1_sacerdote", r"beat_castillo_cerrado$"], "beat_iglesia", ["bnd_pruebas_suficientes"]),
    M("ronda_noche", "La ronda de noche: el cuartel y el vigilante", "mision", [r"b1_cuartel", r"b1_vigilante"], "b1_cuartel_noche", ["bnd1_vigilante_visto", "bnd1_cuartel_visto"]),
    M("entrada_guarida", "Entrar en la guarida: el herrero, las armas y la puerta", "mision",
      [r"b1_guarida_puerta", r"b1_herrero", r"b1_puerta_ganada", r"b1_dep_", r"b1_colarse", r"b1_novicio", r"b1_guarida_por_aigren"],
      "b1_guarida_puerta", ["bnd1_guarida_dentro", "bnd1_cripta_colado"]),
    M("descubierto", "Descubierto en la cripta: calmar, correr, rendirse o pelear", "mision",
      [r"b1_cripta_descubierto", r"b1_cripta_ganada", r"b1_cripta$", r"b1_calma", r"b1_carrera", r"b1_capturado", r"b1_escape"],
      "b1_cripta_descubierto", ["bnd1_capturado_resuelto", "bnd1_cripta_huida", "bnd1_calma_cerrado"]),
    M("ceremonia", "La ceremonia de los Perdidos y el idolo", "mision",
      [r"b1_ceremonia", r"b1_santuario", r"b1_guarida_ceremonia", r"b1_idolo"], "b1_guarida_ceremonia", ["bnd1_ceremonia_acabada", "bnd1_ceremonia_visto"]),
    M("archivo", "El archivo y las dependencias: copistas, literas, tunica y sabotaje", "mision",
      [r"b1_archivo", r"b1_archivero", r"b1_copista", r"b1_estudio", r"b1_literas", r"b1_almacen", r"b1_tunica", r"b1_sabotaje"],
      "b1_archivo_aviso", ["bnd1_estudio_visto", "bnd1_archivo_hecho", "bnd1_sabotaje_hecho"]),
    M("camara_sereno", "La camara honda: Sandes, el prisionero y el pozo", "mision",
      [r"b1_camara", r"b1_sandes", r"b1_sereno", r"b1_cadenas", r"b1_pozo", r"b1_charla_sandes"], "b1_camara_sandes", ["bnd1_sereno_libre", "bnd1_sereno_llevado", "bnd1_salir_pozo"]),
    M("verina_edul", "Verina, Edul y la guarida: sacar, sonsacar, unirte o acabar", "mision",
      [r"b1_verina", r"b1_edul", r"b1_banco", r"b1_frascos", r"b1_llave", r"b1_charla_(verina|edul)", r"b1_guarida_(ganada|exterminada|perdida)"],
      "b1_edul", ["bnd1_guarida_resuelta"]),
    M("informe_venides", "El informe a Venides: las pruebas y el aviso antes del alba", "mision",
      [r"b1_borracho", r"b1_venides_(tarde|pagina|informe)"], "b1_borracho", ["bnd_aviso_dado", "bnd1_venides_informe_hecho"]),
    M("pelea_taberna", "La pelea de la taberna y el camorrista", "mision", [r"b1_pelea", r"b1_camorrista"], "b1_pelea", ["bnd1_camorrista_visto", "bnd1_pelea_lanzada"]),
    M("cierre_dia", "Subir al cuarto y cerrar el dia (la espera de los hombres de Venides)", "mision", [r"b1_sube", r"cierre_", r"b1_rosita_noche"], "b1_sube_al_cuarto", ["bnd_dia_cerrado"]),
]

TABLAS = {"primer_dia": MISIONES_PRIMER_DIA}
# Marcas de las flags del dia (para decir que el cierre de otra tabla/ dia no se mezcle): se usan en la prueba.


def re_encaja(patrones, bid):
    return any(re.match(p, bid) for p in patrones)


def asigna(beats, tabla):
    """{mision_id: [beat]} y la lista de beats sin mision / en dos misiones (la prueba exige que sean vacias)."""
    grupos = OrderedDict((m["id"], []) for m in tabla)
    sin, dobles = [], []
    for b in beats:
        bid = str(b["id"])
        hit = [m["id"] for m in tabla if re_encaja(m["patrones"], bid)]
        if not hit:
            sin.append(bid)
        else:
            grupos[hit[0]].append(b)
            if len(hit) > 1:
                dobles.append((bid, hit))
    return grupos, sin, dobles


# ---------------------------------------------------------------------------------------------------------------
# Lo que ofrece el resto del juego: flags de fuera de las aventuras, PNJ de los niveles, apariciones, diario
# ---------------------------------------------------------------------------------------------------------------
def cadenas_json(x, salida):
    if isinstance(x, str):
        salida.add(str(x))
    elif isinstance(x, dict):
        for k, v in x.items():
            cadenas_json(v, salida)
    elif isinstance(x, list):
        for v in x:
            cadenas_json(v, salida)


def flags_exteriores(dia):
    """Nombres que algo ajeno al dia puede encender: otros dias, cinematicas, eventos del mundo, codigo."""
    fuera = set()
    for d in md.ORDEN:
        if d == dia:
            continue
        for b in md.carga(os.path.join(RAIZ, "assets/adventures/boundington_%s.json" % d))["beats"]:
            for es in [b.get("effects", [])] + [o.get("effects", []) for o in b.get("opciones", [])]:
                ef = md.efectos_de(es)
                fuera.update(md.flags_que_pone(ef))
    # el prologo tambien
    ruta = os.path.join(RAIZ, "assets/adventures/boundington_prologo.json")
    if os.path.exists(ruta):
        for b in md.carga(ruta)["beats"]:
            for es in [b.get("effects", [])] + [o.get("effects", []) for o in b.get("opciones", [])]:
                fuera.update(md.flags_que_pone(md.efectos_de(es)))
    for f in ("assets/cinematicas/cinematicas.json", "assets/eventos/boundington.json"):
        cadenas_json(md.carga(os.path.join(RAIZ, f)), fuera)
    codigo = set()
    for raiz, _, fs in os.walk(os.path.join(RAIZ, "src")):
        for f in fs:
            if f.endswith((".cpp", ".h")):
                texto = open(os.path.join(raiz, f), encoding="utf-8", errors="replace").read()
                codigo.update(re.findall(r'"([a-z][a-z0-9_]{3,})"', texto))
    return fuera, codigo


def objetos_de_niveles():
    """{objectId: [nivel]} de assets/levels/*.json."""
    out = defaultdict(list)
    d = os.path.join(RAIZ, "assets/levels")
    for f in sorted(os.listdir(d)):
        if not f.endswith(".json"):
            continue
        try:
            nivel = md.carga(os.path.join(d, f))
        except Exception:
            continue
        for o in nivel.get("objects", []) if isinstance(nivel, dict) else []:
            if isinstance(o, dict) and "objectId" in o:
                out[str(o["objectId"])].append("assets/levels/" + f)
            if isinstance(o, dict) and o.get("beatCerrada"):  # una puerta cerrada habla con el beat que dice `beatCerrada`
                out[str(o["beatCerrada"])].append("assets/levels/" + f)
    return out


def apariciones():
    c = md.carga(os.path.join(RAIZ, "assets/cinematicas/cinematicas.json"))
    return c.get("apariciones", []), "assets/cinematicas/cinematicas.json"


def tri(cond, ciertas, falsas):
    """Evalua una condicion {todas, alguna, ninguna} en logica de tres valores: True / False / None (no se sabe)."""
    if not isinstance(cond, dict) or not cond:
        return None
    res = []
    for f in cond.get("todas", []) or []:
        res.append(True if f in ciertas else False if f in falsas else None)
    todas = res
    r = True
    if any(x is False for x in todas):
        r = False
    elif any(x is None for x in todas):
        r = None
    alg = cond.get("alguna") or []
    if alg:
        v = [True if f in ciertas else False if f in falsas else None for f in alg]
        if any(x is True for x in v):
            ra = True
        elif all(x is False for x in v):
            ra = False
        else:
            ra = None
        r = False if (r is False or ra is False) else (None if (r is None or ra is None) else True)
    for f in cond.get("ninguna", []) or []:
        if f in ciertas:
            r = False
    return r


def oculto_por_beat(npc, nivel_flags_ciertas, falsas, aps):
    """Una regla de `apariciones` que esconde `npc` con lo que el propio beat exige (ciertas) -> (True, motivo) | (False, '')."""
    for a in aps:
        if str(a.get("objeto")) != npc:
            continue
        sin = a.get("sin")
        if sin and sin in nivel_flags_ciertas:
            return True, "sin:%s (cinematicas.json:%d)" % (sin, md.linea(a["objeto"]))
        oc = tri(a.get("ocultoSi"), nivel_flags_ciertas, falsas)
        if oc is True:
            return True, "ocultoSi cumplido (cinematicas.json:%d)" % md.linea(a["objeto"])
    return False, ""


# ---------------------------------------------------------------------------------------------------------------
# Alcanzabilidad estatica
# ---------------------------------------------------------------------------------------------------------------
def requisitos(req):
    """(allFlags, anyFlags, notFlags, variables) de un requires, con las claves de aventura y de cinematica."""
    if not isinstance(req, dict):
        return [], [], [], {}
    return ([str(f) for f in req.get("allFlags", []) or []], [str(f) for f in req.get("anyFlags", []) or []],
            [str(f) for f in req.get("notFlags", []) or []], req.get("variables") or {})


def flags_de_beat(b):
    """Todo lo que enciende un beat (sus efectos y los de sus opciones, con las salidas de tiradas y combates)."""
    s = set()
    for es in [b.get("effects", [])] + [o.get("effects", []) for o in b.get("opciones", [])]:
        ef = md.efectos_de(es)
        s.update(md.flags_que_pone(ef))
    return s


def variables_puestas(beats):
    v = set(["dia", "franja", "cripta_sospecha", "cripta_reloj"])
    for b in beats:
        for es in [b.get("effects", [])] + [o.get("effects", []) for o in b.get("opciones", [])]:
            v.update(x[0] for x in md.efectos_de(es)["vars"])
    return v


def alcanzables(beats, externas, codigo, vars_puestas, npc_nivel, aps, nivel_existe):
    """Punto fijo: {beat_id: motivo_de_inalcanzable | ''} y el conjunto de flags alcanzables. Optimista con notFlags y variables."""
    puestas_por = defaultdict(list)
    for b in beats:
        for f in flags_de_beat(b):
            puestas_por[f].append(str(b["id"]))
    todos_puestos = set(puestas_por)
    semilla = set(externas) | set(codigo)
    alc = set()
    motivo = {}
    vivos = {str(b["id"]): b for b in beats}
    def ok_requisitos(b, alc):
        allf, anyf, notf, vs = requisitos(b.get("requires"))
        falta = [f for f in allf if not (f.startswith("heroe_") or f in alc or (f in semilla and f not in todos_puestos))]
        if falta:
            return False, "pide %s, que nadie enciende alcanzablemente" % ", ".join(falta[:3])
        if anyf and not any(f.startswith("heroe_") or f in alc or (f in semilla and f not in todos_puestos) for f in anyf):
            return False, "ninguna de %s es alcanzable" % ", ".join(anyf[:4])
        if set(allf) & set(notf):
            return False, "pide y prohibe a la vez %s" % ", ".join(sorted(set(allf) & set(notf)))
        for n in vs:
            if n not in vars_puestas and not str(n).startswith("heroe_") and not (n in semilla):
                return False, "la variable %s no la pone nadie" % n
        return True, ""
    def ok_disparo(b):
        t = b.get("trigger", {})
        if t.get("type") == "talk":
            npc = str(t.get("target") or t.get("npc") or "")
            if npc and npc not in npc_nivel:
                return False, "el PNJ «%s» no esta en ningun nivel" % npc
            allf, _, notf, _ = requisitos(b.get("requires"))
            h, why = oculto_por_beat(npc, set(allf), set(notf), aps)
            if h:
                return False, "el PNJ «%s» esta oculto con lo que el beat exige: %s" % (npc, why)
        if t.get("type") == "enter":
            tgt = str(t.get("target") or "")
            if tgt and not nivel_existe(tgt):
                return False, "el nivel %s no existe" % tgt
        return True, ""
    cambio = True
    while cambio:
        cambio = False
        for bid, b in vivos.items():
            if bid in motivo:
                continue
            ok, why = ok_requisitos(b, alc)
            if ok:
                ok, why = ok_disparo(b)
            if ok:
                motivo[bid] = ""
                for f in flags_de_beat(b):
                    if f not in alc:
                        alc.add(f)
                cambio = True
    for bid in vivos:
        if bid not in motivo:
            ok, why = ok_requisitos(vivos[bid], alc)
            if ok:
                ok, why = ok_disparo(vivos[bid])
            motivo[bid] = why or "sin motivo claro (cadena de requisitos)"
    return motivo, alc, todos_puestos, semilla


# ---------------------------------------------------------------------------------------------------------------
# Explorador: lo que el log dice que no ha visto nadie
# ---------------------------------------------------------------------------------------------------------------
def lee_log(ruta, prefijo="dia1/"):
    """(partidas, beats_sin_ver, opciones_sin_elegir, vistos_total_por_fase) del log del explorador."""
    r = {"partidas": 0, "no_vistos": set(), "no_elegidas": defaultdict(list), "heroe": set(), "ruta": ruta}
    if not os.path.exists(ruta):
        return r
    seccion = ""
    for linea in open(ruta, encoding="utf-8", errors="replace"):
        l = linea.rstrip("\n")
        mm = re.match(r"== explorar_boundington: (\d+) partidas", l)
        if mm:
            r["partidas"] = int(mm.group(1))
        if l.startswith("-- "):
            seccion = l
            continue
        if not l.startswith("   " + prefijo):
            continue
        if seccion.startswith("-- Beats que no ha visto"):
            r["no_vistos"].add(l.strip().split()[0][len(prefijo):])
        elif seccion.startswith("-- Opciones que no ha elegido"):
            mo = re.match(r"\s+" + re.escape(prefijo) + r"(\S+)#(\d+)\s+(.*)", l)
            if mo:
                r["no_elegidas"][mo.group(1)].append((int(mo.group(2)), mo.group(3).strip()))
        elif seccion.startswith("-- Del h"):
            r["heroe"].add(l.strip().split()[0][len(prefijo):])
    return r


# ---------------------------------------------------------------------------------------------------------------
# Diario
# ---------------------------------------------------------------------------------------------------------------
def diario(ruta_aventura):
    """[(id, texto, doneFlag, visibleFlag, fuente)] de los objetivos de la aventura y de la mision bnd_primer_dia de quests.json."""
    out = []
    adv = md.carga(ruta_aventura)
    for ob in adv.get("objectives", []):
        out.append((str(ob.get("id")), str(ob.get("text", ob.get("description", ""))), str(ob.get("doneFlag", "")), str(ob.get("visibleFlag", "")),
                    "%s:%d" % (md.rel(ruta_aventura), md.linea(ob.get("id")))))
    ruta = os.path.join(RAIZ, "assets/catalogs/quests.json")
    if os.path.exists(ruta):
        q = md.carga(ruta)
        for e in q.get("entries", []):
            if str(e.get("adventureId", "")) != "adv_" + os.path.basename(ruta_aventura)[:-5]:
                continue
            for ob in e.get("objectives", []):
                out.append((str(ob.get("id")), str(ob.get("description", "")), str(ob.get("doneFlag", "")), str(ob.get("visibleFlag", "")),
                            "assets/catalogs/quests.json:%d" % md.linea(ob.get("id"))))
    return out


# ---------------------------------------------------------------------------------------------------------------
# El inventario
# ---------------------------------------------------------------------------------------------------------------
def salidas_de(b):
    """Tiradas y combates de un beat, con las flags de cada salida."""
    out = []
    for n, es in [(None, b.get("effects", []))] + [(i, o.get("effects", [])) for i, o in enumerate(b.get("opciones", []))]:
        ef = md.efectos_de(es)
        for t in ef["tiradas"]:
            out.append({"beat": str(b["id"]), "opcion": n, "tipo": "tirada", "que": t["skill"], "flags": t["flags"]})
        for c in ef["combates"]:
            out.append({"beat": str(b["id"]), "opcion": n, "tipo": "combate", "que": "+".join(c["enemigos"]), "flags": c["flags"]})
    return out


def recompensa(grupo, catalogo_actos):
    r = {"oro_pos": 0, "oro_neg": 0, "oro_por_beat": {}, "niveles": 0.0, "objetos": [], "heridas": [], "actos": [], "descansos": 0}
    vistos_actos = set()
    for b in grupo:
        bid = str(b["id"])
        for es in [b.get("effects", [])] + [o.get("effects", []) for o in b.get("opciones", [])]:
            ef = md.efectos_de(es)
            if ef["oro"]:
                r["oro_por_beat"][bid] = r["oro_por_beat"].get(bid, 0) + ef["oro"]
                if ef["oro"] > 0:
                    r["oro_pos"] += ef["oro"]
                else:
                    r["oro_neg"] += ef["oro"]
            r["niveles"] += ef["nivel"]
            r["objetos"] += ["%s (%s)" % (o, bid) for o in ef["objetos"]]
            r["heridas"] += ["%s (%s)" % (h, bid) for h in ef["heridas"]]
            for a in ef["actos"]:
                if a not in vistos_actos:
                    vistos_actos.add(a)
                    x = catalogo_actos.get(a, {})
                    r["actos"].append({"acto": a, "gravedad": x.get("gravedad", "?"), "signo": x.get("signo", 0), "beat": bid})
            r["descansos"] += sum(1 for e in es if e.get("type") == "descanso")
    r["niveles"] = round(r["niveles"], 2)
    return r


def inventario(dia="primer_dia", ruta_aventura=None, ruta_log=None):
    tabla = TABLAS[dia]
    ruta = ruta_aventura or os.path.join(RAIZ, "assets/adventures/boundington_%s.json" % dia)
    adv = md.carga(ruta)
    beats = adv["beats"]
    grupos, sin, dobles = asigna(beats, tabla)
    catalogo_actos = md.actos()
    fuera, codigo = flags_exteriores(dia)
    npc_nivel = objetos_de_niveles()
    aps, ruta_aps = apariciones()
    vars_p = variables_puestas(beats)
    nivel_existe = lambda t: os.path.exists(os.path.join(RAIZ, t))
    motivo, alc, puestas, semilla = alcanzables(beats, fuera, codigo, vars_p, npc_nivel, aps, nivel_existe)
    log = lee_log(ruta_log or LOG_POR_DEFECTO, "dia1/")
    obj = diario(ruta)
    por_id = {str(b["id"]): b for b in beats}
    misiones = []
    for m in tabla:
        g = grupos[m["id"]]
        ids = [str(b["id"]) for b in g]
        flags_grupo = set()
        for b in g:
            flags_grupo |= flags_de_beat(b)
        abre = por_id.get(m["abre"])
        if abre is None or m["abre"] not in ids:
            abre = g[0]
        t = abre.get("trigger", {})
        allf, anyf, notf, vs = requisitos(abre.get("requires"))
        req_txt = "; ".join(x for x in [("todas: " + ", ".join(allf)) if allf else "", ("alguna: " + ", ".join(anyf)) if anyf else "",
                                        ("no: " + ", ".join(notf)) if notf else "", ("vars: " + json.dumps(vs, ensure_ascii=False)) if vs else ""] if x)
        pnjs = OrderedDict()
        for b in g:
            tt = b.get("trigger", {})
            if tt.get("type") == "talk":
                pnjs[str(tt.get("target") or tt.get("npc"))] = 1
        niveles = OrderedDict()
        for b in g:
            tt = b.get("trigger", {})
            if tt.get("type") == "enter":
                niveles[str(tt.get("target"))] = 1
            lug = (b.get("requires") or {}).get("lugar")
            if lug:
                niveles[str(lug)] = 1
        for n in list(pnjs):
            for niv in npc_nivel.get(n, []):
                niveles[niv] = 1
        cierres = [b for b in g if flags_de_beat(b) & set(m["cierre"])]
        # los beats que no son del cierre pero acaban en una salida: tiradas y combates del grupo
        sal = []
        for b in g:
            sal += salidas_de(b)
        # cobertura
        total = len(ids)
        no_vistos = [i for i in ids if i in log["no_vistos"]]
        solo_heroe = [i for i in no_vistos if i in log["heroe"]]
        no_elegidas = []
        for i in ids:
            for k, texto in log["no_elegidas"].get(i, []):
                no_elegidas.append("%s#%d %s" % (i, k, texto))
        # diario: objetivos cuya doneFlag la enciende la mision
        entradas = []
        for (oid, texto, done, vis, fuente) in obj:
            if done and done in flags_grupo:
                entradas.append({"id": oid, "texto": texto, "doneFlag": done, "visibleFlag": vis, "fuente": fuente})
        # alcanzabilidad de los cierres
        cerrable = []
        for b in cierres:
            bid = str(b["id"])
            cerrable.append({"beat": bid, "linea": md.linea(b["id"]), "ok": motivo.get(bid, "") == "", "motivo": motivo.get(bid, "")})
        alcanzables_cierre = [c for c in cerrable if c["ok"]]
        # beats del grupo inalcanzables (aviso, aunque no sean de cierre)
        muertos = [{"beat": i, "linea": md.linea(por_id[i]["id"]), "motivo": motivo[i]} for i in ids if motivo.get(i)]
        misiones.append({
            "id": m["id"], "nombre": m["nombre"], "tipo": m["tipo"], "beats": ids, "n": total,
            "abre": {"beat": str(abre["id"]), "linea": md.linea(abre["id"]), "trigger": str(t.get("type")), "objetivo": str(t.get("target") or t.get("npc") or ""),
                     "requires": req_txt},
            "pnj": list(pnjs), "niveles": list(niveles),
            "cierre_flags": m["cierre"], "cierre_beats": [{"beat": str(b["id"]), "linea": md.linea(b["id"]),
                                                          "pone": sorted(flags_de_beat(b) & set(m["cierre"]))} for b in cierres],
            "salidas": sal,
            "recompensa": recompensa(g, catalogo_actos),
            "diario": entradas,
            "cobertura": {"vistos": total - len(no_vistos), "total": total, "no_vistos": no_vistos, "solo_heroe": solo_heroe, "no_elegidas": no_elegidas,
                          "partidas": log["partidas"]},
            "cerrable": {"ok": bool(alcanzables_cierre) and bool(cierres), "cierres": cerrable, "sin_cierre_en_el_grupo": not cierres},
            "inalcanzables": muertos,
        })
    return {"dia": dia, "ruta": md.rel(ruta), "n_beats": len(beats), "sin_mision": sin, "dobles": dobles, "misiones": misiones,
            "log": md.rel(log["ruta"]) if os.path.exists(log["ruta"]) else None}


# ---------------------------------------------------------------------------------------------------------------
# Salida
# ---------------------------------------------------------------------------------------------------------------
def c(s):
    return str(s).replace("|", "\\|").replace("\n", " ")


def corta(lista, n=4):
    return ", ".join(lista[:n]) + (" (+%d)" % (len(lista) - n) if len(lista) > n else "")


def escribe_md(inv, ruta_md):
    ms = inv["misiones"]
    cerr = sum(1 for m in ms if m["cerrable"]["ok"])
    with open(ruta_md, "w", encoding="utf-8") as f:
        f.write("# MISIONES_DIA1 — inventario de misiones del día 1 (E-MISIONES-01)\n\n")
        f.write("Generado por `MotorGraphico/tools/matriz_misiones.py --dia %s` (no editar a mano: se regenera). Fuente: `%s` (%d beats, todos asignados a una misión: %d sin misión, %d en dos).\n\n"
                % (inv["dia"], inv["ruta"], inv["n_beats"], len(inv["sin_mision"]), len(inv["dobles"])))
        f.write("**%d misiones**, **%d** se pueden cerrar (alcanzabilidad estática: el beat que enciende el cierre tiene requisitos alcanzables, PNJ en un nivel y no escondido por `ocultoSi`). "
                "Cobertura del explorador (%s, %s partidas): ver columna «vistos».\n\n"
                % (len(ms), cerr, inv["log"], ms[0]["cobertura"]["partidas"] if ms else "?"))
        f.write("Una misión = grupo de beats con un comienzo y un cierre. `mision` tiene cierre propio; `charla` es ambiente de un PNJ (su cierre es despedirse; la flag es la de «visto»).\n\n")
        f.write("## Tabla\n\n| # | misión | tipo | empieza (PNJ / nivel) | se abre con | se cierra con | recompensa | diario | vistos | ¿cierra? |\n|---|---|---|---|---|---|---|---|---|---|\n")
        for n, m in enumerate(ms, 1):
            r = m["recompensa"]
            rec = []
            if r["oro_pos"] or r["oro_neg"]:
                rec.append("oro +%d/%d" % (r["oro_pos"], r["oro_neg"]))
            if r["niveles"]:
                rec.append("nivel +%s" % r["niveles"])
            if r["objetos"]:
                rec.append("objetos: " + corta([o.split(" (")[0] for o in r["objetos"]], 3))
            if r["actos"]:
                rec.append("moral: " + corta(["%s(%s%s)" % (a["acto"], a["gravedad"], "+" if a["signo"] > 0 else "-" if a["signo"] < 0 else "0") for a in r["actos"]], 4))
            if r["heridas"]:
                rec.append("heridas %d" % len(r["heridas"]))
            cov = m["cobertura"]
            dia = corta([e["id"] for e in m["diario"]], 3) if m["diario"] else "—"
            donde = (", ".join(m["pnj"][:2]) or "—") + " / " + (m["niveles"][0].replace("assets/levels/", "").replace(".json", "") if m["niveles"] else "—")
            ab = m["abre"]
            f.write("| %d | **%s** `%s` | %s | %s | `%s:%d` %s `%s` | %s | %s | %s | %d/%d | %s |\n" % (
                n, c(m["nombre"]), m["id"], m["tipo"], c(donde), ab["beat"], ab["linea"], ab["trigger"], c(ab["objetivo"].replace("assets/levels/", "")),
                c(", ".join("`%s`" % x for x in m["cierre_flags"])) + " (" + c(", ".join("%s:%d" % (b["beat"], b["linea"]) for b in m["cierre_beats"][:3])) + ")",
                c("; ".join(rec) or "—"), c(dia), cov["vistos"], cov["total"],
                "sí" if m["cerrable"]["ok"] else "**⚠ NO**"))
        f.write("\n## Detalle por misión\n\n")
        for n, m in enumerate(ms, 1):
            cov = m["cobertura"]
            f.write("### %d. %s (`%s`, %s)\n\n" % (n, m["nombre"], m["id"], m["tipo"]))
            ab = m["abre"]
            f.write("- **Se abre**: `%s` (línea %d), trigger `%s` %s. Requisitos: %s.\n" % (ab["beat"], ab["linea"], ab["trigger"], ab["objetivo"] or "(auto)", ab["requires"] or "ninguno"))
            f.write("- **PNJ / niveles**: %s / %s.\n" % (", ".join(m["pnj"]) or "—", ", ".join(x.replace("assets/levels/", "") for x in m["niveles"]) or "—"))
            f.write("- **Se cierra** (flags %s): %s.\n" % (", ".join("`%s`" % x for x in m["cierre_flags"]),
                    "; ".join("`%s:%d` pone %s" % (b["beat"], b["linea"], ", ".join(b["pone"])) for b in m["cierre_beats"][:6]) or "⚠ ningún beat del grupo enciende el cierre"))
            if m["salidas"]:
                f.write("- **Salidas** (tiradas y combates, %d):" % len(m["salidas"]))
                f.write(" " + "; ".join("%s `%s`%s → %s" % (s["tipo"], s["beat"], " [" + s["que"] + "]" if s["tipo"] == "tirada" else "", "/".join(s["flags"].values())) for s in m["salidas"][:6]))
                f.write(" (+%d más)\n" % (len(m["salidas"]) - 6) if len(m["salidas"]) > 6 else "\n")
            r = m["recompensa"]
            f.write("- **Recompensa**: oro neto %+d (cobros %d, pagos %d)%s%s%s%s.\n" % (
                r["oro_pos"] + r["oro_neg"], r["oro_pos"], r["oro_neg"], ", nivel +%s" % r["niveles"] if r["niveles"] else "",
                ", objetos " + corta(r["objetos"], 3) if r["objetos"] else "", ", heridas " + corta(r["heridas"], 3) if r["heridas"] else "",
                ", actos " + corta(["%s (%s%s)" % (a["acto"], a["gravedad"], "+" if a["signo"] > 0 else "-" if a["signo"] < 0 else "0") for a in r["actos"]], 6) if r["actos"] else ""))
            f.write("- **Diario**: %s.\n" % ("; ".join("`%s` (doneFlag `%s`, %s)" % (e["id"], e["doneFlag"], e["fuente"]) for e in m["diario"]) or "sin entrada: ningún objetivo ni misión de quests.json se completa con esta misión"))
            f.write("- **Explorador**: %d/%d beats vistos%s.%s\n" % (cov["vistos"], cov["total"], " (%d solo del héroe creado)" % len(cov["solo_heroe"]) if cov["solo_heroe"] else "",
                    " Sin ver: " + corta(cov["no_vistos"], 10) if cov["no_vistos"] else ""))
            if cov["no_elegidas"]:
                f.write("  - Opciones que no ha elegido nadie (%d): %s\n" % (len(cov["no_elegidas"]), corta(cov["no_elegidas"], 6)))
            f.write("- **¿Se puede cerrar?** %s\n" % ("sí: " + corta(["`%s:%d`" % (x["beat"], x["linea"]) for x in m["cerrable"]["cierres"] if x["ok"]], 4) if m["cerrable"]["ok"] else "**⚠ NO**"))
            for x in m["cerrable"]["cierres"]:
                if not x["ok"]:
                    f.write("  - ⚠ `%s:%d` inalcanzable: %s\n" % (x["beat"], x["linea"], x["motivo"]))
            for x in m["inalcanzables"]:
                if not any(x["beat"] == y["beat"] for y in m["cerrable"]["cierres"]):
                    f.write("  - ⚠ beat muerto `%s:%d`: %s\n" % (x["beat"], x["linea"], x["motivo"]))
            f.write("\n")
        f.write("## Hallazgos ⚠ del análisis estático\n\n")
        n = 0
        for m in ms:
            for x in m["cerrable"]["cierres"]:
                if not x["ok"]:
                    n += 1
                    f.write("- ⚠ **%s**: el cierre `%s` (`%s:%d`) no se puede alcanzar: %s.\n" % (m["id"], ", ".join(m["cierre_flags"]), x["beat"], x["linea"], x["motivo"]))
            if m["cerrable"]["sin_cierre_en_el_grupo"]:
                n += 1
                f.write("- ⚠ **%s**: ningún beat del grupo enciende su cierre (%s).\n" % (m["id"], ", ".join(m["cierre_flags"])))
            for x in m["inalcanzables"]:
                n += 1
                f.write("- ⚠ **%s**: beat muerto `%s:%d`: %s.\n" % (m["id"], x["beat"], x["linea"], x["motivo"]))
        if n == 0:
            f.write("(ninguno)\n")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--dia", default="primer_dia", choices=sorted(TABLAS))
    ap.add_argument("--md", default=None)
    ap.add_argument("--json", default=None)
    ap.add_argument("--log", default=None)
    ap.add_argument("--adventure", default=None, help="JSON de aventura alternativo (para la prueba)")
    a = ap.parse_args(argv)
    inv = inventario(a.dia, a.adventure, a.log)
    md_ruta = a.md or os.path.join(RAIZ, "MISIONES_%s.md" % a.dia)
    escribe_md(inv, md_ruta)
    if a.json:
        with open(a.json, "w", encoding="utf-8") as j:
            json.dump(inv, j, ensure_ascii=False, indent=1)
            j.write("\n")
    ms = inv["misiones"]
    print("%s: %d misiones, %d cerrables, %d beats sin mision, %d en dos -> %s" % (
        a.dia, len(ms), sum(1 for m in ms if m["cerrable"]["ok"]), len(inv["sin_mision"]), len(inv["dobles"]), md_ruta))
    return 0


if __name__ == "__main__":
    sys.exit(main())
