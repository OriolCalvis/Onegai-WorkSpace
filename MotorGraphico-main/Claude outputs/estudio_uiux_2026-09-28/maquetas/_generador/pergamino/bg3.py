"""Pantallas en cuadrícula con distribución tipo BG3 y el tema pergamino.

Encargo de Oriol (05/10/2026): menús en cuadrícula que se manejen con ratón y
teclado, como en Baldur's Gate 3; una barra de cartas abajo en el combate en
vez de la lista; el título en el pergamino; y un cofre en El Vacío con el kit
básico de la clase (va a la mochila).

    python3 bg3.py             -> todas
    python3 bg3.py C02a C03    -> algunas

Los datos salen del repo (datos/catalogo.json, extraído de assets/catalogs) y
los dibujos, de los atlas del juego (img/sprites). Los fondos son capturas
del juego sin HUD (img/fondos/fondo_*_plaza|ciudad|cofre.png).
"""
import json
import os
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(AQUI))
sys.path.insert(0, AQUI)

import texturas as T  # noqa: E402
from lib import (FUENTE, SALIDA, ayuda, bloque_titulo, boton, e, escribir, ic, insignia, px,  # noqa: E402
                 recurso, render, tecla)
from muestra import cuero, filete, hoja, marca_suelo, papiro, pestana, sprite  # noqa: E402

DATOS = json.load(open(os.path.join(AQUI, "..", "..", "datos", "catalogo.json"), encoding="utf-8"))
NOMBRE = "Nerea"
RAZA_ID = "alquimistas_de_aegroum"
CLASE_ID = "bardo_de_las_cuatro_estaciones"
TRASFONDO_ID = "mes_de_la_cosecha"
STATS = ("CON", "DES", "INT", "CAR")
ICONO_STAT = {"CON": "con", "DES": "des", "INT": "int", "CAR": "car"}
NOMBRE_STAT = {"CON": "Constitución", "DES": "Destreza", "INT": "Inteligencia", "CAR": "Carisma"}
# Kit básico del cofre: PROPUESTA para el Bardo (sin requisitos; +1 a CAR y a DES).
KIT_BARDO = [("Sable ligero de viaje", "ranura_mano_principal", "Arma", ""),
             ("Armadura de cuero", "ranura_torso", "Torso", "+1 DES"),
             ("Amuleto de foco", "ranura_accesorio", "Cabeza", "+1 CAR")]


TILDES = {"Cancion": "Canción", "Otono": "Otoño", "Bendicion": "Bendición", "Invocacion": "Invocación",
          "Elevacion": "Elevación", "Egida": "Égida", "Ultimo": "Último", "dia": "día", "Otono:": "Otoño:",
          "fisicos": "físicos", "Anades": "Añades"}


def con_tildes(texto):
    return " ".join(TILDES.get(p, p) for p in texto.split(" "))


def raza(id_):
    return next(r for r in DATOS["razas"] if r["id"] == id_)


def clase(id_):
    return next(c for c in DATOS["clases"] if c["id"] == id_)


def trasfondo(id_):
    return next(t for t in DATOS["trasfondos"] if t["id"] == id_)


# ------------------------------------------------------------------ página
def pagina(titulo, cuerpo, fondo=None, velo=None):
    mundo = f'<div class="mundo" style="background-image:url(img/fondos/{fondo}.png)"></div>' if fondo else ""
    v = f'<div class="velo" style="background:{velo}"></div>' if velo else ""
    return f"""<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<title>{e(titulo)} · Pantallas en cuadrícula · MotorGraphico</title>
<link rel="stylesheet" href="sistema/tokens.css">
<link rel="stylesheet" href="sistema/componentes.css">
<link rel="stylesheet" href="sistema/tema_pergamino.css">
<link rel="stylesheet" href="sistema/bg3.css">
</head>
<body>
<div class="pantalla pergamino">
{mundo}{v}
{cuerpo}
</div>
</body>
</html>
"""


def img(src, w, h, cls="", estilo="", alt=""):
    c = f' class="{cls}"' if cls else ""
    st = f' style="{estilo}"' if estilo else ""
    return f'<img{c} src="{src}" width="{w}" height="{h}" alt="{e(alt)}"{st}>'


def capa(x, y, contenido, w=None, h=None, estilo=""):
    medidas = (f"width:{w}px;" if w else "") + (f"height:{h}px;" if h else "")
    return f'<div class="capa" style="left:{x}px;top:{y}px;{medidas}{estilo}">{contenido}</div>'


# Cursor de flecha en pixel (2 = relleno claro, 1 = contorno)
_CURSOR = ["1..........", "11.........", "121........", "1221.......", "12221......", "122221.....",
           "1222221....", "12222221...", "122222221..", "1222222221.", "1222211111.", "1221221....",
           "1211.121...", "11...121...", "1.....121..", "......11..."]


def cursor(x, y):
    rects = []
    for fy, fila_ in enumerate(_CURSOR):
        for fx, ch in enumerate(fila_):
            if ch != ".":
                color = "#1A1008" if ch == "1" else "#FBF3DE"
                rects.append(f'<rect x="{fx}" y="{fy}" width="1" height="1" fill="{color}"/>')
    return (f'<svg class="cursor" style="left:{x}px;top:{y}px" viewBox="0 0 11 16" width="22" height="32" '
            f'shape-rendering="crispEdges" aria-hidden="true">{"".join(rects)}</svg>')


def sello_mini():
    return f'<span class="sello-mini">{px("✓", 2)}</span>'


def peana(cx, cy, w=150, h=56):
    return (f'<svg class="capa" style="left:{cx - w // 2}px;top:{cy - h // 2}px" width="{w}" height="{h}" viewBox="0 0 {w} {h}" '
            f'shape-rendering="crispEdges" aria-hidden="true">'
            f'<polygon points="{w // 2},2 {w - 2},{h // 2} {w // 2},{h - 2} 2,{h // 2}" fill="rgba(107,80,56,.16)" '
            f'stroke="#6B5038" stroke-width="2" stroke-dasharray="6 4"/></svg>')


def desliza(x, y, h, ini, tam):
    return f'<div class="desliza" style="left:{x}px;top:{y}px;height:{h}px"><i style="top:{ini}px;height:{tam}px"></i></div>'


def stat_pills(valores, origen, extra=None, sube=()):
    """valores: {'CON': 3...}; origen: texto bajo la cifra; extra: {'DES': '+1 kit'}"""
    pills = []
    for s in STATS:
        o = origen.get(s, "base 0")
        ex = (extra or {}).get(s)
        cls = "stat sube" if s in sube else "stat"
        linea_o = f'<span class="o mas">{e(ex)}</span>' if ex else f'<span class="o">{e(o)}</span>'
        pills.append(f'<div class="{cls}"><span class="fila-flex" style="gap:5px;color:var(--tinta-2)">{ic(ICONO_STAT[s], 1)}{px(s, 2)}</span>'
                     f'<span class="v">{px(str(valores.get(s, 0)), 3)}</span>{linea_o}</div>')
    return f'<div class="stats">{"".join(pills)}</div>'


# ------------------------------------------------- marco de la creación
PASOS = [("RAZA", "personaje", "Raza"), ("ASPECTO", "ranura_cabeza", "Aspecto"), ("CLASE", "dote", "Clase"),
         ("ORIGEN", "lugar", "Origen"), ("HISTORIA", "lore", "Historia"), ("DEIDAD", "deidad", "Deidad"),
         ("NOMBRE", "mejora", "Nombre")]
VX, VY, VW, VH = 16, 10, 1248, 700
PX, PY, PW, PH = 36, 98, 1208, 542


def marco_creacion(activa, boton_txt="CONTINUAR"):
    cab = f'''<div class="fila-flex" style="position:absolute;left:30px;right:30px;top:12px;height:34px;gap:16px">
      <span class="claro">{px("NUEVA PARTIDA", 3)}</span>
      <span class="claro-2" style="font-size:16px;margin-left:6px">Paso {activa + 1} de 7 · {PASOS[activa][2]}</span>
      <span style="margin-left:auto;font-size:16px" class="claro-2">Los Perdidos de Boundington · Tier 1</span></div>'''
    pie = f'''<div class="fila-flex" style="position:absolute;left:30px;right:22px;bottom:8px;height:44px;gap:20px">
      {ayuda(["Q", "E"], "Paso")}{ayuda(["W", "A", "S", "D"], "Moverte")}{ayuda("ENTER", "Elegir")}{ayuda("ESC", "Atrás")}
      <span class="claro-2" style="font-size:15px">Ratón: encima para ver, clic para elegir</span>
      <span style="margin-left:auto">{boton(boton_txt, "primario") if boton_txt else ""}</span></div>'''
    partes = [cuero(VX, VY, VW, VH, cab + pie, remaches=False, esquinas=True)]
    partes.append(capa(VX + 26, VY + 60, tecla("Q")))
    x = VX + 70
    for i, (et, icono, _n) in enumerate(PASOS):
        hecho = i < activa
        w = 44 + len(et) * 12 + 16 + (26 if hecho else 0)
        w += w % 2
        act = i == activa
        y = VY + 52 if act else VY + 56
        h = PY - y + (2 if act else -2)
        cont = f'{ic(icono, 1)}{px(et, 2)}' + (f'<span class="paso-ok">{px("✓", 2)}</span>' if hecho else "")
        partes.append(pestana(x, y, w, h + (h % 2), cont, act, semilla=60 + i))
        x += w + 6
    partes.append(capa(x + 4, VY + 60, tecla("E")))
    partes.append(hoja(PX + 14, PY + 6, PW, PH - 6, "", semilla=71, rasgado=1, manchas=4))
    partes.append(hoja(PX + 7, PY + 3, PW, PH - 3, "", semilla=72, rasgado=1, manchas=4))
    return "".join(partes)


def lomo():
    remaches = "".join(f'<i style="left:{x}px"></i>' for x in (24, PW // 2 - 4, PW - 32))
    return f'<div class="lomo" style="left:{PX - 8}px;top:{PY + PH - 4}px;width:{PW + 16}px">{remaches}</div>'


def pagina_creacion(titulo, activa, contenido, boton_txt="CONTINUAR", extra=""):
    top = hoja(PX, PY, PW, PH, contenido, semilla=70, rasgado=1, manchas=2, filete=6)
    cuerpo = marco_creacion(activa, boton_txt) + top + lomo() + extra
    return pagina(titulo, cuerpo, fondo="fondo_titulo_ciudad", velo="rgba(24,14,6,.70)")


def detalle_raza(r):
    bonos = "".join(f'<span>{ic(ICONO_STAT[s], 1)}+{v} {s}</span>' for s, v in r["bonos"].items() if v)
    desc = r["desc"].split(" Edad:")[0]
    datos = r["desc"]
    edad = datos.split("Edad: ")[1].split(" Tamaño")[0] if "Edad: " in datos else ""
    tam = datos.split("Tamaño: ")[1].split(" Religión")[0] if "Tamaño: " in datos else ""
    talla = (tam.split(" ")[0] or "").upper()
    return f'''<div class="det col" style="gap:12px;height:100%">
      <h2>{e(r["nombre"])}</h2>
      <div class="fila-flex" style="gap:8px">{insignia("POCO COMÚN" if r["rareza"] == "uncommon" else "COMÚN")}{insignia(talla) if talla else ""}</div>
      <div class="bonos">{bonos}</div>
      {bloque_titulo("RASGO")}
      <div class="col" style="gap:4px"><span class="rasgo">{e(r["rasgo"] or "")}</span>
        <p style="-webkit-line-clamp:4;display:-webkit-box;-webkit-box-orient:vertical;overflow:hidden">{e(r["rasgoDesc"] or "")}</p></div>
      {bloque_titulo("CULTURA")}
      <p style="-webkit-line-clamp:4;display:-webkit-box;-webkit-box-orient:vertical;overflow:hidden">{e(desc)}</p>
      <dl class="res" style="font-size:15px"><dt>Afinidades</dt><dd>{e(" · ".join(r["afinidades"]))}</dd>
        <dt>Edad</dt><dd>{e(edad.split(" y ")[0])}</dd></dl>
    </div>'''


# ------------------------------------------------------- C02a · Raza
def C02a_raza():
    razas = DATOS["razas"]
    foco = next(i for i, r in enumerate(razas) if r["id"] == RAZA_ID)
    r = razas[foco]
    teselas = []
    for i, rz in enumerate(razas[:20]):
        cls = "tesela foco" if i == foco else "tesela"
        retrato = img("img/sprites/razas/" + rz["id"] + "_busto.png", 60, 60, "retrato")
        teselas.append(f'<button class="{cls}" type="button"><span class="cinta"></span>{retrato}'
                       f'<span class="nom">{e(rz["nombre"])}</span></button>')
    rejilla = f'<div class="rejilla" style="grid-template-columns:repeat(5,104px);gap:8px 9px">{"".join(teselas)}</div>'
    izquierda = f'''<div class="capa" style="left:28px;top:18px;width:566px;height:506px">
      <div class="fila-flex" style="gap:12px;height:36px;margin-bottom:10px">
        <span class="t3">{px("ELIGE TU PUEBLO", 2)}</span>
        <span class="campo" style="margin-left:auto;width:220px">{px("/", 2)}<span>Buscar raza…</span></span>
      </div>
      {rejilla}
      <div class="fila-flex t3" style="margin-top:10px;font-size:14px;gap:8px">1–20 de 43 · rueda del ratón o
        {tecla("S")} para ver más</div>
    </div>{desliza(600, 64, 452, 0, 210)}'''
    centro = f'''{peana(745, 276)}
      {img(f"img/sprites/razas/{r['id']}_frente.png", 256, 256, "figura", "left:617px;top:28px")}
      <div class="capa girar" style="left:625px;top:316px;width:240px"><span class="b">{px("◀", 2)}</span>
        <span>Girar · o arrastra</span><span class="b">{px("▶", 2)}</span></div>
      <div class="capa col" style="left:622px;top:362px;width:246px;gap:8px">
        <span class="t3">{px("AL EMPEZAR", 2)}</span>
        {stat_pills(r["bonos"], {s: (f"raza +{v}" if v else "base 0") for s, v in r["bonos"].items()},
                    sube=[s for s, v in r["bonos"].items() if v])}
        <span class="t3" style="font-size:13px;line-height:1.3">Base 0. La raza suma; lo demás lo pone el equipo.</span>
      </div>'''
    derecha = f'<div class="capa" style="left:892px;top:22px;width:288px;height:500px">{detalle_raza(r)}</div>'
    contenido = izquierda + centro + derecha
    extra = cursor(PX + 28 + 2 * 113 + 60, PY + 18 + 46 + 50)
    return pagina_creacion("Creación · raza", 0, contenido, extra=extra)


# ------------------------------------------------------- C02b · Clase
ROLES = [("Todas", None), ("Lanzador", "caster"), ("Ágil", "agile"), ("Defensa", "tank"),
         ("Equilibrio", "balanced"), ("Apoyo", "support"), ("Golpe", "striker")]
GLIFO_TIPO = {"accion_principal": ("principal", "●"), "libre": ("libre", "▶"), "reaccion": ("reaccion", "ϟ"),
              "canalizacion": ("canaliza", "◆"), "movimiento": ("movimiento", "▲")}


def tipo_glifo(tipo):
    cls, g = GLIFO_TIPO.get(tipo, ("principal", "●"))
    return f'<span class="tipo {cls}">{px(g, 2)}</span>'


def C02b_clase():
    clases = DATOS["clases"]
    foco = next(i for i, c in enumerate(clases) if c["id"] == CLASE_ID)
    c = clases[foco]
    r = raza(RAZA_ID)
    from collections import Counter
    cuenta = Counter(x["rol"] for x in clases)
    chips = "".join(f'<button type="button" class="chip{" activa" if rol is None else ""}">{e(n)}'
                    + (f'<span class="n">{len(clases)}</span>' if rol is None else "") + '</button>' for n, rol in ROLES)
    teselas = []
    for i, cl in enumerate(clases[:15]):
        cls = "tesela foco" if i == foco else "tesela"
        prim = ICONO_STAT.get(cl["principal"], "tier")
        teselas.append(f'<button class="{cls}" type="button"><span class="cinta"></span>'
                       f'<span class="esq der">{ic(prim, 1)}</span>'
                       + img("img/sprites/clases/" + cl["id"] + "_busto.png", 60, 60, "retrato") +
                       f'<span class="nom">{e(cl["nombre"])}</span></button>')
    rejilla = f'<div class="rejilla" style="grid-template-columns:repeat(5,104px);gap:8px 9px">{"".join(teselas)}</div>'
    izquierda = f'''<div class="capa" style="left:28px;top:18px;width:566px;height:506px">
      <div class="fila-flex" style="gap:12px;height:30px;margin-bottom:8px"><span class="t3">{px("ELIGE TU OFICIO", 2)}</span>
        <span class="t3" style="margin-left:auto;font-size:15px">El icono de la esquina es su atributo principal</span></div>
      <div class="chips" style="margin-bottom:12px">{chips}</div>
      {rejilla}
      <div class="fila-flex t3" style="margin-top:10px;font-size:14px;gap:8px">1–15 de 63 · filtra por papel o baja con la rueda</div>
    </div>{desliza(600, 96, 420, 0, 100)}'''
    kit_extra = {"DES": "+1 kit", "CAR": "+1 kit"}
    valores = dict(r["bonos"])
    valores["DES"] = valores.get("DES", 0) + 1
    valores["CAR"] = valores.get("CAR", 0) + 1
    kit = "".join(f'<span class="fila-flex" style="gap:6px">{ic(i_, 1)}{e(n)}<b style="margin-left:auto;color:var(--exito)">{e(b)}</b></span>' for n, i_, _r, b in KIT_BARDO)
    centro = f'''{peana(745, 252)}
      {img(f"img/sprites/clases/{c['id']}_frente.png", 256, 256, "figura", "left:617px;top:4px")}
      <div class="capa col" style="left:622px;top:290px;width:246px;gap:4px">
        <span class="t3">{px("KIT DEL COFRE", 2)}</span>
        <div class="col" style="gap:1px;font-size:15px;color:var(--tinta-2)">{kit}</div></div>
      <div class="capa col" style="left:622px;top:384px;width:246px;gap:8px">
        <span class="t3">{px("CON EL KIT PUESTO", 2)}</span>
        {stat_pills(valores, {s: (f"raza +{v}" if v else "base 0") for s, v in r["bonos"].items()}, kit_extra, sube=["DES", "CAR"])}
      </div>'''
    estandar = [x for x in c["cartas"] if x["tier"] <= 1]
    creador = [x for x in c["cartas"] if x["tier"] > 1]
    mano = "".join(f'<span>{tipo_glifo(x["tipo"])}{e(con_tildes(x["nombre"]))}</span>' for x in estandar)
    t2 = " · ".join(con_tildes(x["nombre"]) for x in creador)
    derecha = f'''<div class="capa det col" style="left:892px;top:18px;width:288px;height:504px;gap:8px">
      <h2>{e(c["nombre"])}</h2>
      <div class="fila-flex" style="gap:8px">{insignia("ÁGIL")}<span class="fila-flex" style="gap:4px;font-size:16px">{ic("car", 1)}CAR</span>
        <span class="fila-flex" style="gap:4px;font-size:16px;color:var(--tinta-2)">{ic("des", 1)}DES</span>
        <span class="fila-flex" style="gap:4px;font-size:16px;color:var(--tinta-2)">{ic("pv", 1)}10</span></div>
      <p style="font-size:15px;-webkit-line-clamp:2;display:-webkit-box;-webkit-box-orient:vertical;overflow:hidden">{e(c["desc"])}</p>
      {bloque_titulo("PASIVA")}
      <div class="col" style="gap:2px"><span class="rasgo">{e(c["pasiva"][0]["nombre"])}</span>
        <p style="font-size:15px;-webkit-line-clamp:2;display:-webkit-box;-webkit-box-orient:vertical;overflow:hidden">{e(con_tildes(c["pasiva"][0]["desc"]))}</p></div>
      {bloque_titulo(f"TU MANO · {len(estandar)} + {len(creador)}")}
      <div class="mano">{mano}</div>
      <span class="t3" style="font-size:14px;line-height:1.3">{px("T2", 2)} {e(t2)}</span>
    </div>'''
    contenido = izquierda + centro + derecha
    return pagina_creacion("Creación · clase", 2, contenido)


# ------------------------------------------------------- C02c · Historia
LISTAS = [("bonds", "VÍNCULO"), ("fears", "MIEDO"), ("flaws", "DEFECTO"), ("goals", "META"),
          ("ideals", "IDEAL"), ("personalities", "CARÁCTER"), ("virtues", "VIRTUD")]


def C02c_historia():
    t = trasfondo(TRASFONDO_ID)
    elegidas = {"bonds": 2, "fears": 1, "flaws": 4, "ideals": 0, "personalities": 0}
    foco = ("goals", 2)
    celdas = []
    for clave, rot in LISTAS:
        hecha = clave in elegidas
        ok = f'<span class="paso-ok">{px("✓", 2)}</span>' if hecha else ""
        celdas.append(f'<div class="rot">{px(rot, 2)}{ok}</div>')
        for j, op in enumerate(t["listas"][clave][:5]):
            cls = "tarjeta"
            sello = ""
            if elegidas.get(clave) == j:
                cls += " elegida"
                sello = f'<span class="sello">{sello_mini()}</span>'
            if foco == (clave, j):
                cls += " foco"
            celdas.append(f'<button type="button" class="{cls}"><span class="cinta"></span>{sello}'
                          f'<span class="n">{e(op["nombre"])}</span><span class="d">{e(op["desc"])}</span></button>')
    matriz = f'<div class="matriz" style="grid-auto-rows:56px;row-gap:6px">{"".join(celdas)}</div>'
    cab = f'''<div class="fila-flex" style="gap:12px;margin-bottom:12px;height:36px">
        {ic("lugar")}<span style="font-size:22px;font-weight:600">{e(t["nombre"])}</span>
        <span class="t2" style="font-size:17px">· {e(t["alineamiento"] or "")}</span>
        <span style="margin-left:auto" class="fila-flex t3">{px("5 DE 7", 2)}<span style="font-size:15px">· una tarjeta por fila</span></span></div>'''
    contenido = f'<div class="capa" style="left:28px;top:18px;width:1150px">{cab}{matriz}</div>'
    meta = t["listas"]["goals"][2]
    cx, cy = PX + 28 + 545 + 130, PY + 18 + 48 + 3 * 62 + 32
    emerge = hoja(cx + 18, cy + 18, 300, 108, f'<div class="emergente" style="inset:0"><b>{e(meta["nombre"])}</b><span>{e(meta["desc"])}</span>'
                  f'<span class="t3" style="font-size:13px">Clic o ENTER para elegirla</span></div>', semilla=73, rasgado=2, manchas=0, clase="retal")
    extra = emerge + cursor(cx, cy)
    return pagina_creacion("Creación · historia", 4, contenido, extra=extra)


# ------------------------------------------------------- C02d · Nombre
def C02d_nombre():
    r = raza(RAZA_ID)
    c = clase(CLASE_ID)
    t = trasfondo(TRASFONDO_ID)
    sugeridos = ["Nerea", "Aldric", "Brisa", "Telmo", "Ivet", "Oren"]
    chips = "".join(f'<button type="button" class="chip{" activa" if n == NOMBRE else ""}">{e(n)}</button>' for n in sugeridos)
    valores = dict(r["bonos"])
    hist = [(rot.capitalize(), t["listas"][k][i]["nombre"]) for (k, rot), i in
            zip(LISTAS, (2, 1, 4, 2, 0, 0, 3))]
    hist_html = "".join(f'<dt>{e(a.replace("Vínculo", "Vínculo"))}</dt><dd>{e(b)}</dd>' for a, b in hist)
    izquierda = f'''{peana(170, 300)}
      {img(f"img/sprites/clases/{c['id']}_frente.png", 256, 256, "figura", "left:42px;top:52px")}
      <div class="capa girar" style="left:50px;top:338px;width:240px"><span>Con el kit del cofre</span></div>
      <div class="capa col" style="left:44px;top:384px;width:252px;gap:8px">
        <span class="t3">{px("AL EMPEZAR", 2)}</span>
        {stat_pills(valores, {s: (f"raza +{v}" if v else "base 0") for s, v in r["bonos"].items()}, sube=[s for s, v in r["bonos"].items() if v])}
        <span class="t3" style="font-size:13px;line-height:1.3">El kit del cofre sumará +1 DES y +1 CAR.</span></div>'''
    centro = f'''<div class="capa col" style="left:332px;top:22px;width:420px;gap:12px">
      <span class="t3">{px("CÓMO TE LLAMAS", 2)}</span>
      <span class="campo grande">{e(NOMBRE)}<span class="caret"></span></span>
      <div class="chips">{chips}</div>
      <span class="t3" style="font-size:14px">Escribe el tuyo o elige uno; {tecla("A")} {tecla("D")} cambian de sugerencia.</span>
      {bloque_titulo("QUIÉN ERES")}
      <dl class="res"><dt>Raza</dt><dd>{e(r["nombre"])} <a class="editar">editar</a></dd>
        <dt>Clase</dt><dd>{e(c["nombre"])} <a class="editar">editar</a></dd>
        <dt>Origen</dt><dd>{e(t["nombre"])} <a class="editar">editar</a></dd>
        <dt>Deidad</dt><dd>Ninguna <a class="editar">editar</a></dd></dl>
      <div class="linea-estado" style="margin-top:6px;color:var(--tinta-2);align-items:flex-start">{ic("objeto_mision", 1, estilo="margin-top:3px")}
        <span style="font-size:15px;line-height:1.35">Empiezas sin equipo. En El Vacío te espera un cofre con el kit de tu clase: va a la mochila y te lo pones desde el cuaderno.</span></div>
    </div>'''
    derecha = f'''<div class="capa col" style="left:788px;top:22px;width:392px;gap:10px">
      {bloque_titulo("TU HISTORIA")}
      <dl class="res" style="font-size:15px">{hist_html}</dl>
      <a class="editar" style="align-self:flex-start">editar la historia</a>
      <div style="margin-top:30px" class="col">{boton("EMPEZAR LA AVENTURA", "primario", foco=True)}</div>
    </div>'''
    return pagina_creacion("Creación · nombre y resumen", 6, izquierda + centro + derecha, boton_txt=None)


# ---------------------------------------------------------------- C01 · Título
def C01_titulo():
    W, H = 440, 452
    x, y = (1280 - W) // 2, 232
    filas = [("Continuar", "deshab", "No hay ninguna partida guardada"), ("Nueva partida", "foco", None),
             ("Opciones", "", None), ("Salir", "", None)]
    html_filas = "".join(
        f'<div class="fila {cls}"><span class="marca"></span><span class="sin-icono"></span>'
        f'<span class="principal"><span class="nombre">{e(n)}</span>{f"<span class=motivo>{e(m)}</span>" if m else ""}</span>'
        f'<span class="valor"></span></div>' for n, cls, m in filas)
    cont = f'''<div class="col" style="position:absolute;inset:44px 46px 38px 46px;gap:8px">
      <div class="lista">{html_filas}</div>
      <div style="display:flex;justify-content:center;margin:10px 0 0">{filete(260)}</div>
      <div class="fila-flex" style="gap:18px;justify-content:center;margin-top:auto;flex-wrap:wrap">{ayuda(["W", "S"], "Elegir")}{ayuda("ENTER", "Aceptar")}{ayuda("ESC", "Salir")}</div>
      <span class="t3" style="font-size:14px;text-align:center">o haz clic con el ratón</span>
    </div>
    <style>.papiro .fila{{font-size:24px;min-height:50px}}</style>'''
    titulo = f'''<div class="capa col" style="left:0;top:58px;width:1280px;align-items:center;gap:14px">
      <span class="claro sombra-px3">{px("LOS PERDIDOS", 6)}</span>
      <span class="claro sombra-px3">{px("DE BOUNDINGTON", 4)}</span>
      <span class="claro-2" style="font-size:18px;font-style:italic;text-shadow:2px 2px 0 rgba(0,0,0,.5)">Una campaña en Aegroum · Tier 1</span></div>'''
    cuerpo = titulo + papiro(x, y, W, H, cont) + cursor(x + 250, y + 120) + \
        capa(24, 684, '<span class="claro-2" style="font-size:14px;text-shadow:1px 1px 0 #000">MotorGraphico</span>')
    return pagina("Título", cuerpo, fondo="fondo_titulo_ciudad", velo="radial-gradient(ellipse at center, rgba(24,14,6,.5), rgba(12,7,3,.85))")


# --------------------------------------------------------------- C03 · Combate
HERO = (672, 368)       # pies del héroe en el fondo de la plaza
ENEMIGO = (706, 380)

CARTAS_BASICAS = [("Atacar", "ranura_mano_principal", "accion_principal", "1"),
                  ("Poción ×2", "consumible", "accion_principal", "2"),
                  ("Huir", "cerrar", "accion_principal", "3")]
CARTAS_BARDO = [("Canción del Otoño", "perjuicio", "accion_principal", "4"),
                ("Ola de Elementos", "mejora", "libre", "5"),
                ("Bendición de las Estrellas", "inspiracion", "reaccion", "6"),
                ("Canción de la Primavera", "pv", "accion_principal", "7"),
                ("Invocación Elemental", "dote", "accion_principal", "8"),
                ("Canción del Invierno", "ralentizado", "accion_principal", "9"),
                ("Canción del Verano", "enfurecido", "accion_principal", "0")]


def carta_mini(nombre, icono, tipo, tecla_n, estado=""):
    return (f'<button type="button" class="carta {estado}"><span class="cab">{tipo_glifo(tipo)}'
            f'<span class="tecla-n">{px(tecla_n, 2)}</span></span>'
            f'<span class="ico">{ic(icono, 2)}</span><span class="nom">{e(nombre)}</span></button>')


def bandeja(estados, apagado=False, titulo_turno=None):
    basicas = "".join(carta_mini(*c) for c in CARTAS_BASICAS)
    mano = "".join(carta_mini(*c, estado=estados.get(c[0], "")) for c in CARTAS_BARDO)
    t2 = ('<button type="button" class="carta bloq"><span class="cab"><span></span><span class="tecla-n">' + px("T2", 2) +
          '</span></span><span class="ico">' + ic("tier", 2) + '</span><span class="nom">+3 de creador</span></button>')
    cls = "bandeja apagado-turno" if apagado else "bandeja"
    cont = f'''<div class="{cls}" style="position:absolute;left:18px;right:18px;bottom:14px">
      <div class="grupo" style="position:relative"><span class="rot">{px("BÁSICAS", 2)}</span>{basicas}</div>
      <span class="costura"></span>
      <div class="grupo" style="position:relative"><span class="rot">{px("TU MANO", 2)}</span>{mano}{t2}</div></div>'''
    return cuero(146, 552, 988, 160, cont, costura=True, remaches=False)


def econ(gastados=(), preparada=True):
    def pieza(g, et, clave):
        cls = ' class="gastado"' if clave in gastados else ""
        return f'<span{cls}>{px(g, 2)}{px(et, 2)}</span>'
    reac = '<span style="color:var(--carmin)">' + px("ϟ", 2) + px("REACCIÓN LISTA", 2) + '</span>' if preparada and "reaccion" not in gastados \
        else pieza("ϟ", "REACCIÓN", "reaccion")
    cont = f'<div class="econ" style="padding:10px 18px">{pieza("●", "ACCIÓN", "accion")}{pieza("▲", "MOVIMIENTO", "mov")}{pieza("◆", "CANALIZA", "canal")}{reac}</div>'
    return hoja(166, 500, 640, 44, cont, semilla=81, rasgado=2, manchas=0, clase="retal")


def retal_heroe(pv, maxpv, insignias=""):
    cont = f'''<div style="padding:16px 20px">
      <div class="fila-flex" style="margin-bottom:10px"><span style="font-size:21px;font-weight:600;line-height:1">{NOMBRE}</span>{insignia("T1")}
        <span style="margin-left:auto">{insignias}</span></div>
      {recurso("pv", "PV", pv, maxpv, 250, "pv")}</div>'''
    return hoja(24, 20, 290, 112, cont, semilla=11, rasgado=3, manchas=2, clase="retal")


def retal_turnos(ronda, activo="heroe"):
    def turno(src, nombre, frac, cls):
        return (f'<div class="turno {cls}">{img(src, 60, 60, alt=nombre)}<span class="vida"><i style="width:{frac * 100:.0f}%"></i></span></div>')
    cont = f'''<div class="fila-flex" style="padding:12px 18px;gap:16px;height:100%">
      <span class="col" style="gap:4px">{px("RONDA", 2)}<span>{px(str(ronda), 3)}</span></span>
      <div class="turnos">{turno("img/sprites/clases/bardo_de_las_cuatro_estaciones_busto.png", NOMBRE, 14 / 19, "activo" if activo == "heroe" else "")}
        {turno("img/sprites/enemigo_busto.png", "Perdido saqueador", 1.0, "enemigo activo" if activo == "enemigo" else "enemigo")}</div></div>'''
    return hoja(486, 12, 308, 104, cont, semilla=82, rasgado=2, manchas=1, clase="retal")


def placa_enemigo(pv, maxpv, estados):
    est = "".join(f'<span class="fila-flex" style="gap:4px;font-size:14px;color:var(--tinta-2)">{ic(i_, 1)}{e(n)}</span>' for i_, n in estados)
    cont = f'''<div class="col" style="padding:12px 16px;gap:6px">
      <span class="b6" style="font-size:17px;white-space:nowrap;line-height:1.2">Perdido saqueador</span>
      {recurso("pv", "PV", pv, maxpv, 226, "pv")}
      <div class="fila-flex" style="gap:10px">{est}</div></div>'''
    return hoja(ENEMIGO[0] - 20, ENEMIGO[1] - 196, 258, 120, cont, semilla=83, rasgado=2, manchas=1, clase="retal")


def registro(lineas, alto=268):
    filas = "".join(f'<span class="{"nuevo" if i >= len(lineas) - 2 else ""}">{l}</span>' for i, l in enumerate(lineas))
    cont = f'''<div class="col" style="padding:16px 18px;gap:8px">
      <div class="fila-flex">{px("REGISTRO", 2)}<span style="margin-left:auto">{ayuda("L", "Ocultar")}</span></div>
      <div class="registro">{filas}</div></div>'''
    return hoja(966, 140, 290, alto, cont, semilla=84, rasgado=3, manchas=2, clase="retal")


def carta_grande(nombre, icono, tipo_txt, tecla_n, texto, lineas, x=24, y=140, w=262, h=352, reaccion=False):
    cls = "cinta-tipo reaccion" if reaccion else "cinta-tipo"
    ls = "".join(f'<div class="linea">{ic(i_, 1, estilo="margin-top:2px")}<span>{t}</span></div>' for i_, t in lineas)
    cont = f'''<div class="carta-grande" style="inset:0">
      <div class="{cls}">{px(tipo_txt, 2)}<span>{px(tecla_n, 2)}</span></div>
      <div class="fila-flex" style="gap:12px">{ic(icono, 3) if False else ic(icono, 2)}<h3>{e(nombre)}</h3></div>
      <div class="texto">{e(texto)}</div>
      <div class="col" style="gap:6px;margin-top:2px">{ls}</div>
      <div class="fila-flex" style="margin-top:auto;gap:12px">{ayuda("ENTER", "Jugar")}<span class="t3" style="font-size:15px">o clic</span></div>
    </div>'''
    return hoja(x, y, w, h, cont, semilla=85, rasgado=2, manchas=1, clase="hoja")


def C03a_combate():
    estados = {"Canción del Otoño": "gastada", "Bendición de las Estrellas": "preparada", "Canción del Invierno": "foco"}
    lineas = ["<b>Ronda 1</b> · Nerea elige <b>Invierno</b>.",
              "Nerea juega Canción del Otoño.",
              "Perdido saqueador queda con Canción del Otoño (2 turnos).",
              "Perdido saqueador ataca a Nerea (parcial, 1 contra 1: 5 de daño).",
              "<b>Ronda 2</b> · tu turno.",
              "Nerea prepara Bendición de las Estrellas."]
    grande = carta_grande("Canción del Invierno", "ralentizado", "ACCIÓN PRINCIPAL", "9",
                          "Congelas a los enemigos cercanos. Quedan fuera de combate 2 turnos y sus tiradas de defensa tienen desventaja.",
                          [("objetivo", "Perdido saqueador y los que estén cerca"),
                           ("perjuicio", "Si gana su tirada, la resiste"),
                           ("cerrar", "Se gasta al jugarla. Cierra tu turno")])
    insig = f'<span class="insignia">{ic("inspiracion", 1)}{px("INVIERNO", 2)}</span>'
    cuerpo = (marca_suelo(*ENEMIGO) + retal_heroe(14, 19, insig) + retal_turnos(2) +
              placa_enemigo(22, 22, [("perjuicio", "Otoño · 1")]) + registro(lineas) + grande + econ(preparada=True) +
              bandeja(estados) +
              hoja(1144, 640, 120, 58, f'<div class="fila-flex" style="height:100%;justify-content:center;gap:8px">{tecla("F")}<span class="sec">Pasar</span></div>',
                   semilla=86, rasgado=2, manchas=0, clase="retal") +
              cursor(146 + 18 + 3 * 80 + 16 + 5 * 80 + 44, 612))
    return pagina("Combate · tu turno", cuerpo, fondo="fondo_combate_plaza")


def C03b_estacion():
    estaciones = [("PRIMAVERA", "pv", "Tus curaciones curan +2."),
                  ("VERANO", "enfurecido", "Tienes ventaja en ataque."),
                  ("OTOÑO", "perjuicio", "Tienes ventaja en defensa."),
                  ("INVIERNO", "ralentizado", "Ganas +2 a la CA y reflejas los ataques físicos durante 1 turno.")]
    tarjetas = "".join(f'''<button type="button" class="estacion{" foco" if n == "INVIERNO" else ""}" style="width:214px;height:236px">
        <span class="cinta"></span>{px(n, 3)}{ic(i_, 2)}<p>{e(t)}</p></button>''' for n, i_, t in estaciones)
    cont = f'''<div class="col" style="position:absolute;inset:30px 40px 24px;gap:14px;align-items:center">
      <div class="col" style="align-items:center;gap:8px">{px("LAS CUATRO ESTACIONES", 3)}{filete(300)}</div>
      <span class="t2" style="font-size:18px">Pasiva del bardo. Elige la estación de este combate: es una acción libre.</span>
      <div class="fila-flex" style="gap:16px;margin-top:4px">{tarjetas}</div>
      <div class="fila-flex" style="gap:20px;margin-top:auto">{ayuda(["←", "→"], "Estación")}{ayuda("ENTER", "Elegir")}{ayuda("ESC", "Ahora no")}
        <span style="margin-left:24px">{boton("AHORA NO", "secundario")}</span>{boton("ELEGIR INVIERNO", "primario", foco=True)}</div>
    </div>'''
    cuerpo = (retal_heroe(19, 19) + retal_turnos(1) + bandeja({}) +
              '<div class="velo" style="background:rgba(24,14,6,.6)"></div>' +
              hoja(150, 128, 980, 440, cont, semilla=87, rasgado=2, manchas=2, filete=6) +
              cursor(150 + 40 + 3 * 230 + 150, 128 + 330))
    return pagina("Combate · elegir estación", cuerpo, fondo="fondo_combate_plaza")


def C03c_reaccion():
    estados = {"Canción del Otoño": "gastada", "Bendición de las Estrellas": "salta", "Canción del Invierno": "gastada"}
    lineas = ["Nerea juega Canción del Invierno.",
              "Perdido saqueador queda con Canción del Invierno (2 turnos).",
              "Perdido saqueador resiste: sale de la canción.",
              "<b>Turno de Perdido saqueador</b>.",
              "Perdido saqueador ataca a Nerea.",
              "<b>Bendición de las Estrellas</b>: Nerea bloquea el ataque."]
    aviso = hoja(330, 176, 330, 82, f'''<div class="fila-flex" style="gap:12px;padding:12px 18px;height:100%">{ic("inspiracion")}
      <span class="col" style="line-height:1.3"><span class="b6" style="font-size:17px">¡Bendición de las Estrellas!</span>
      <span class="t2" style="font-size:15px">Bloqueas el ataque. Reacción gastada.</span></span></div>''',
                 semilla=88, rasgado=2, manchas=1, clase="retal")
    turno = hoja(440, 446, 400, 46, f'<div class="fila-flex" style="height:100%;justify-content:center;gap:10px">{px("TURNO DE PERDIDO SAQUEADOR", 2)}</div>',
                 semilla=89, rasgado=2, manchas=0, clase="retal")
    insig = f'<span class="insignia">{ic("inspiracion", 1)}{px("INVIERNO", 2)}</span>'
    cuerpo = (retal_heroe(14, 19, insig) + retal_turnos(2, activo="enemigo") + placa_enemigo(22, 22, [("perjuicio", "Otoño · 1")]) +
              registro(lineas) + aviso + econ(gastados=("accion", "reaccion"), preparada=False) + turno + bandeja(estados, apagado=True))
    return pagina("Combate · turno enemigo y reacción", cuerpo, fondo="fondo_combate_plaza")


# ------------------------------------------------------- C04 · El Vacío
COFRE = (737, 368)


def hud_vacio(objetivo_txt, cuenta):
    heroe = hoja(24, 20, 290, 112, f'''<div style="padding:16px 20px">
      <div class="fila-flex" style="margin-bottom:10px"><span style="font-size:21px;font-weight:600;line-height:1">{NOMBRE}</span>{insignia("T1")}</div>
      {recurso("pv", "PV", 19, 19, 250, "pv")}</div>''', semilla=11, rasgado=3, manchas=2, clase="retal")
    obj = hoja(24, 144, 290, 148, f'''<div class="objetivo" style="padding:20px 22px 14px">
      <div class="mision">{ic("mision_activa", 1)}{px("OBJETIVO", 2)}<span class="t3" style="margin-left:auto">{px(cuenta, 2)}</span></div>
      <div class="meta t2" style="line-height:1.3">El Vacío</div>
      <div class="paso">{ic("objetivo", 1, estilo="margin-top:3px")}<span>{e(objetivo_txt)}</span></div></div>
      <div style="position:absolute;left:139px;top:5px">{sprite("chincheta", T.chincheta)}</div>''',
                semilla=12, rasgado=3, manchas=2, clase="retal")
    teclas = hoja(1010, 648, 246, 52, f'<div class="fila-flex" style="gap:18px;padding:12px 18px;justify-content:center">{ayuda("TAB", "Menú")}{ayuda("ESC", "Pausa")}</div>',
                  semilla=14, rasgado=2, manchas=0, clase="retal")
    return heroe + obj + teclas


def C04a_cofre():
    cx, cy = COFRE
    prompt = hoja(cx - 116, cy - 104, 232, 46, f'''<div class="fila-flex" style="gap:10px;padding:0 16px;height:100%;justify-content:center">{tecla("E")}<span class="sec">Abrir <b>cofre</b></span></div>
      <div style="position:absolute;left:110px;bottom:-12px;width:0;height:0;border:6px solid transparent;border-top-color:#4A3421"></div>''',
                  semilla=16, rasgado=1, manchas=0, clase="retal")
    cuerpo = hud_vacio("Abre el cofre: son tus cosas.", "0/1") + marca_suelo(cx, cy) + prompt
    return pagina("El Vacío · el cofre", cuerpo, fondo="fondo_vacio_cofre")


def C04b_cofre_abierto():
    filas = "".join(f'''<div class="fila"><span class="marca"></span>{ic(i_)}<span class="principal"><span class="nombre">{e(n)}</span>
        <span class="motivo">{e(ranura)} · sin requisitos</span></span><span class="valor" style="color:var(--exito);font-weight:600">{e(b)}</span></div>'''
                    for n, i_, ranura, b in KIT_BARDO)
    cont = f'''<div class="col" style="position:absolute;inset:30px 36px 24px;gap:12px">
      <div class="col" style="align-items:center;gap:8px">{px("COFRE", 3)}{filete(240)}</div>
      <span class="t2" style="font-size:17px;text-align:center">El kit básico del Bardo de las Cuatro Estaciones</span>
      <div class="lista">{filas}</div>
      <div class="linea-estado exito" style="margin-top:4px">{px("✓", 2)}<span>Guardado en tu mochila</span></div>
      <div class="fila-flex" style="gap:16px;margin-top:auto;justify-content:center">{boton("PONÉRMELO", "primario", foco=True)}{boton("CERRAR", "secundario")}</div>
      <div class="fila-flex" style="gap:18px;justify-content:center">{ayuda("ENTER", "Abrir el cuaderno en Equipo")}{ayuda("ESC", "Cerrar")}</div>
    </div>
    <style>.hoja .lista .fila{{font-size:19px;min-height:52px}}</style>'''
    aviso = hoja(410, 20, 460, 94, f'''<div class="fila-flex" style="gap:12px;padding:14px 20px">{ic("mision_activa")}
      <span class="col sec" style="line-height:1.3"><span class="b6">Nuevo objetivo</span><span class="t2">Ponte tu equipo (TAB, hoja Equipo)</span></span></div>''',
                 semilla=15, rasgado=2, manchas=1, clase="retal")
    cuerpo = (hud_vacio("Ponte tu equipo (TAB, hoja Equipo).", "1/2") + aviso +
              '<div class="velo" style="background:rgba(24,14,6,.5)"></div>' +
              hoja(380, 128, 520, 478, cont, semilla=90, rasgado=2, manchas=2, filete=6))
    return pagina("El Vacío · cofre abierto", cuerpo, fondo="fondo_vacio_cofre")


PANTALLAS = {
    "C01_titulo": C01_titulo,
    "C02a_creacion_raza": C02a_raza,
    "C02b_creacion_clase": C02b_clase,
    "C02c_creacion_historia": C02c_historia,
    "C02d_creacion_nombre": C02d_nombre,
    "C03a_combate_tu_turno": C03a_combate,
    "C03b_combate_estacion": C03b_estacion,
    "C03c_combate_reaccion": C03c_reaccion,
    "C04a_vacio_cofre": C04a_cofre,
    "C04b_vacio_cofre_abierto": C04b_cofre_abierto,
}

if __name__ == "__main__":
    pedidas = sys.argv[1:] or list(PANTALLAS)
    nombres = []
    for n in PANTALLAS:
        if any(n.startswith(p) for p in pedidas):
            escribir(n, PANTALLAS[n]())
            nombres.append(n)
    render(nombres)
