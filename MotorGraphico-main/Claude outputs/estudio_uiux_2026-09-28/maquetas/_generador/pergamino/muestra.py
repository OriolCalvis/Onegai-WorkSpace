"""Muestra del tema pergamino: HUD ligero, diálogo, cuaderno (menú con
pestañas) y papiro (menú de una sola página).

python3 muestra.py           -> P03a P03b P04a P05a
"""
import os
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(AQUI))
sys.path.insert(0, AQUI)

import numpy as np  # noqa: E402
from PIL import Image  # noqa: E402

import texturas as T  # noqa: E402
from lib import (ETER, FUENTE, SALIDA, ayuda, boton, bloque_titulo, e, escribir, fila, ic, insignia, moral,  # noqa: E402
                 px, recurso, render, tecla)

NOMBRE = "Nerea"
RAZA = "Aarakocra de las Montañas"
CLASE = "Acechador de Tejados"
IMG = os.path.join(FUENTE, "img", "pergamino")


# ---------------------------------------------------------------- texturas
def tex(nombre, fn, *a, **k):
    ruta = os.path.join(IMG, nombre + ".png")
    if not os.path.exists(ruta):
        T.guardar(fn(*a, **k), ruta)
    return f"img/pergamino/{nombre}.png"


def hoja(x, y, w, h, contenido="", semilla=1, rasgado=2, filete=None, clase="hoja", estilo="", manchas=3):
    url = tex(f"hoja_{w}x{h}_s{semilla}_r{rasgado}_f{filete}_m{manchas}", T.hoja, w, h, semilla, rasgado,
              filete=filete, manchas=manchas)
    return (f'<div class="capa {clase}" style="left:{x}px;top:{y}px;width:{w}px;height:{h}px;'
            f'background-image:url({url});{estilo}">{contenido}</div>')


def papiro(x, y, w, h, contenido="", estilo=""):
    url = tex(f"papiro_{w}x{h}", T.papiro, w, h)
    return (f'<div class="capa papiro" style="left:{x}px;top:{y}px;width:{w}px;height:{h}px;'
            f'background-image:url({url});{estilo}">{contenido}</div>')


def cuero(x, y, w, h, contenido="", costura=True, remaches=True, estilo="", clase="", esquinas=False):
    url = tex(f"cuero_{w}x{h}_c{int(costura)}_r{int(remaches)}_e{int(esquinas)}", T.cuero, w, h, costura=costura,
              remaches=remaches, esquinas=esquinas)
    return (f'<div class="capa cuero {clase}" style="left:{x}px;top:{y}px;width:{w}px;height:{h}px;'
            f'background-image:url({url});{estilo}">{contenido}</div>')


def pestana(x, y, w, h, contenido, activa=False, semilla=7):
    url = tex(f"pestana_{w}x{h}_{'a' if activa else 'i'}_s{semilla}", T.pestana, w, h, activa, semilla)
    cls = "hoja sin-sombra" if activa else "hoja"
    col = "var(--tinta)" if activa else "var(--tinta-2)"
    return (f'<div class="capa {cls}" style="left:{x}px;top:{y}px;width:{w}px;height:{h}px;'
            f'background-image:url({url});display:flex;align-items:center;justify-content:center;gap:8px;color:{col}">'
            f'{contenido}</div>')


def filete(w, estilo=""):
    url = tex(f"adorno_{w}", T.adorno, w)
    return f'<div class="filete" style="width:{w}px;background-image:url({url});{estilo}"></div>'


def sprite(nombre, fn, estilo=""):
    url = tex(nombre, fn)
    im = Image.open(os.path.join(IMG, nombre + ".png"))
    return f'<img src="{url}" width="{im.width}" height="{im.height}" style="image-rendering:pixelated;{estilo}" alt="">'


def marca_suelo(cx, cy, color="var(--carmin)", w=64, h=32):
    return (f'<svg class="capa" style="left:{cx - w // 2}px;top:{cy - h // 2}px" width="{w}" height="{h}" '
            f'viewBox="0 0 {w} {h}" shape-rendering="crispEdges">'
            f'<polygon points="{w // 2},1 {w - 1},{h // 2} {w // 2},{h - 1} 1,{h // 2}" fill="none" stroke="#2A1A0E" stroke-width="5"/>'
            f'<polygon points="{w // 2},1 {w - 1},{h // 2} {w // 2},{h - 1} 1,{h // 2}" fill="none" stroke="#C0392B" stroke-width="2"/></svg>')


def pagina(titulo, cuerpo, fondo=None, velo=None):
    mundo = f'<div class="mundo" style="background-image:url(img/fondos/{fondo}.png)"></div>' if fondo else ""
    v = f'<div class="velo" style="background:{velo}"></div>' if velo else ""
    return f"""<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<title>{e(titulo)} · Tema pergamino · MotorGraphico</title>
<link rel="stylesheet" href="sistema/tokens.css">
<link rel="stylesheet" href="sistema/componentes.css">
<link rel="stylesheet" href="sistema/tema_pergamino.css">
</head>
<body>
<div class="pantalla pergamino">
{mundo}{v}
{cuerpo}
</div>
</body>
</html>
"""


# ----------------------------------------------------------- minimapa en tinta
def minimapa_tinta(nivel, jugador, objetivos=(), ancho=56, alto=34, destino="minimapa_tinta"):
    from nav import Level, catalog  # nav.py vive junto a lib.py
    L = Level(nivel)
    cat = catalog()
    x0 = max(0, min(L.w - ancho, jugador[0] - ancho // 2))
    y0 = max(0, min(L.h - alto, jugador[1] - alto // 2))
    S = 2  # píxeles lógicos por baldosa
    W, H = ancho * S, alto * S
    img = np.zeros((H, W, 4), dtype=np.uint8)
    bloq = np.zeros((alto, ancho), dtype=bool)
    for ty in range(alto):
        for tx in range(ancho):
            bloq[ty, tx] = (x0 + tx, y0 + ty) in L.blocked
    B = np.kron(bloq, np.ones((S, S), dtype=bool))
    ys, xs = np.mgrid[0:H, 0:W]
    trama = (xs + ys) % 3 == 0
    img[B & trama] = (107, 80, 56, 255)
    img[B & ~trama] = (198, 165, 108, 110)
    borde = B & ~(np.roll(B, 1, 0) & np.roll(B, -1, 0) & np.roll(B, 1, 1) & np.roll(B, -1, 1))
    img[borde] = (74, 52, 33, 255)

    def c(t):
        return ((t[0] - x0) * S + S // 2, (t[1] - y0) * S + S // 2)

    def dentro(t):
        return x0 <= t[0] < x0 + ancho and y0 <= t[1] < y0 + alto

    def pinta(cx, cy, patron, color):
        for (dy, fila_) in enumerate(patron):
            for (dx, ch) in enumerate(fila_):
                if ch != ".":
                    yy, xx = cy - len(patron) // 2 + dy, cx - len(fila_) // 2 + dx
                    if 0 <= yy < H and 0 <= xx < W:
                        img[yy, xx] = (*((42, 26, 14) if ch == "o" else color), 255)

    puertas = {(o["position"]["x"], o["position"]["y"]) for o in L.data["objects"] if "targetLevel" in o}
    for t in puertas:
        if dentro(t) and t not in objetivos:
            pinta(*c(t), ["ooooo", "o###o", "o###o", "o###o", "ooooo"], (41, 80, 122))
    for t, i in L.objects.items():
        if cat.get(i, {}).get("category") == "npc" and dentro(t):
            pinta(*c(t), [".o.", "o#o", ".o."], (85, 62, 43))
    for t in objetivos:
        if dentro(t):
            pinta(*c(t), ["...o...", "..o#o..", ".o###o.", "o#####o", ".o###o.", "..o#o..", "...o..."], (154, 42, 28))
    pinta(*c(jugador), ["...o...", "..o#o..", "..o#o..", ".o###o.", ".o###o.", "o#####o", "ooooooo"], (43, 29, 18))
    out = Image.fromarray(img, "RGBA").resize((W * 2, H * 2), Image.NEAREST)
    ruta = os.path.join(IMG, destino + ".png")
    os.makedirs(IMG, exist_ok=True)
    out.save(ruta)
    return f"img/pergamino/{destino}.png", out.size


# --------------------------------------------------------------- HUD ligero
ANCIANA = (702, 372)


def oro(n, s=2):
    return f'<span class="fila-flex" style="gap:6px">{ic("oro", 1)}{px(str(n), s)}</span>'


def hud_personaje(compacto=False):
    if compacto:
        cont = f'''<div style="padding:18px 22px">
  <div class="fila-flex" style="margin-bottom:10px"><span style="font-size:20px;font-weight:600;line-height:1">{NOMBRE}</span>{insignia("T1")}</div>
  <div class="col" style="gap:10px">{recurso("pv", "PV", 28, 28, 242, "pv")}{recurso("pm", "PM", 8, 8, 242, "pm")}</div></div>'''
        return hoja(24, 24, 290, 166, cont, semilla=11, rasgado=3, manchas=2, clase="retal")
    cont = f'''<div style="padding:18px 22px">
  <div class="fila-flex" style="margin-bottom:12px"><span style="font-size:22px;font-weight:600;line-height:1">{NOMBRE}</span>{insignia("T1")}<span style="margin-left:auto">{oro(150)}</span></div>
  <div class="col" style="gap:12px">{recurso("pv", "PV", 28, 28, 242, "pv")}{recurso("pm", "PM", 8, 8, 242, "pm")}{moral(0, "NEUTRAL", 242)}</div></div>'''
    return hoja(24, 24, 290, 222, cont, semilla=11, rasgado=3, manchas=2, clase="retal")


def hud_objetivo(y=258):
    cont = f'''<div class="objetivo" style="padding:22px 22px 16px">
  <div class="mision">{ic("mision_activa", 1)}{px("OBJETIVO", 2)}<span class="t3" style="margin-left:auto">{px("0/3", 2)}</span></div>
  <div class="meta t2" style="line-height:1.3;white-space:nowrap">El primer día en Boundington</div>
  <div class="paso">{ic("objetivo", 1, estilo="margin-top:3px")}<span>Presentarte a los tres comerciantes de la plaza</span></div>
  <div class="meta t3" style="padding-left:24px">Mercado, joyería y sastrería</div></div>
  <div style="position:absolute;left:139px;top:5px">{sprite("chincheta", T.chincheta)}</div>'''
    return hoja(24, y, 290, 190, cont, semilla=12, rasgado=3, manchas=2, clase="retal")


def hud_minimapa():
    url, (mw, mh) = minimapa_tinta("assets/levels/ciudad_centro.json", (26, 34), objetivos=[(54, 27), (8, 27), (42, 27)])
    cont = f'''<div class="col" style="padding:16px 16px 12px;gap:8px">
  <span class="sec b6">Centro de Boundington</span>
  <img src="{url}" width="{mw}" height="{mh}" alt="Minimapa" style="image-rendering:pixelated;display:block">
  <div class="leyenda" style="color:var(--tinta-2)">
    <span><b style="color:var(--tinta)">{px("▲", 2)}</b>Tú</span>
    <span><b style="color:var(--tinta-2)">{px("●", 2)}</b>Personaje</span>
    <span><b style="color:var(--info)">{px("■", 2)}</b>Entrada</span>
    <span><b style="color:var(--carmin)">{px("◆", 2)}</b>Objetivo</span>
  </div></div>'''
    return hoja(1000, 24, 256, 266, cont, semilla=13, rasgado=3, manchas=2, clase="retal")


def hud_teclas():
    cont = f'<div class="fila-flex" style="gap:18px;padding:12px 18px;justify-content:center">{ayuda("TAB", "Menú")}{ayuda("ESC", "Pausa")}</div>'
    return hoja(1010, 648, 246, 52, cont, semilla=14, rasgado=2, manchas=0, clase="retal")


def P03a_hud():
    cx, cy = ANCIANA
    aviso = hoja(410, 24, 460, 94, f'''<div class="fila-flex" style="gap:12px;padding:14px 20px">{ic("mision_activa")}
      <span class="col sec" style="line-height:1.3"><span class="b6">Nuevo objetivo</span><span class="t2">Presentarte a los tres comerciantes de la plaza</span></span></div>''',
                 semilla=15, rasgado=2, manchas=1, clase="retal")
    prompt = hoja(cx - 124, cy - 106, 248, 46, f'''<div class="fila-flex" style="gap:10px;padding:0 16px;height:100%;justify-content:center">{tecla("E")}<span class="sec">Hablar con <b>Anciana</b></span></div>
      <div style="position:absolute;left:118px;bottom:-12px;width:0;height:0;border:6px solid transparent;border-top-color:#4A3421"></div>''',
                  semilla=16, rasgado=1, manchas=0, clase="retal")
    cuerpo = (hud_personaje() + hud_objetivo() + hud_minimapa() + hud_teclas()
              + marca_suelo(cx, cy) + prompt + aviso)
    return pagina("HUD de exploración", cuerpo, fondo="fondo_ciudad_anciana")


def P03b_dialogo():
    cx, cy = ANCIANA
    caja = hoja(180, 530, 920, 162, f'''<div class="col" style="padding:34px 40px 18px;gap:12px;height:100%">
      <div style="font-size:22px;line-height:1.55;flex:1">Antes esta plaza era un huerto.</div>
      <div class="fila-flex"><span class="t3">{px("1/2", 2)}</span><span style="margin-left:auto">{ayuda("ENTER", "Continuar")}</span></div></div>''',
                semilla=21, rasgado=2, manchas=2)
    rotulo = cuero(212, 512, 150, 36, f'<div class="rotulo" style="inset:0">{px("ANCIANA", 2)}</div>', costura=False, remaches=False)
    cuerpo = hud_personaje(compacto=True) + marca_suelo(cx, cy) + caja + rotulo
    return pagina("Diálogo", cuerpo, fondo="fondo_ciudad_anciana")


# ------------------------------------------------ Cuaderno (menú con pestañas)
SECCIONES = [("INVENTARIO", "equipo"), ("EQUIPO", "ranura_torso"), ("FICHA", "personaje"),
             ("ACCIONES", "dote"), ("MISIONES", "mision_activa"), ("CÓDICE", "lore")]
VX, VY, VW, VH = 80, 60, 1120, 600          # ventana
PX, PY, PW, PH = 112, 156, 1056, 446        # hoja superior (la bisagra queda en PY + PH)


def cuaderno_marco(activa, con_pestanas=True):
    """Tapas de cuero, cabecera, pestañas, pila de hojas, lomo y pie."""
    cab = f'''<div class="fila-flex" style="position:absolute;left:32px;right:32px;top:12px;height:34px;gap:14px">
      <span style="font-size:24px;font-weight:600;line-height:1">{NOMBRE}</span>{insignia("T1")}
      <span style="font-size:17px;color:var(--claro-cuero-2)">{RAZA} · {CLASE}</span>
      <span style="margin-left:auto" class="fila-flex">{ic("pv", 1)}{px("28/28", 2)}</span>
      <span class="fila-flex">{ic("pm", 1)}{px("8/8", 2)}</span>{oro(150)}</div>'''
    pie = f'''<div class="fila-flex" style="position:absolute;left:32px;right:32px;bottom:12px;height:30px;gap:24px">
      {ayuda(["←", "→"], "Pasar hoja")}{ayuda(["W", "S"], "Objeto")}{ayuda("ENTER", "Usar")}
      <span style="margin-left:auto">{ayuda("TAB", "Cerrar")}</span></div>'''
    partes = [cuero(VX, VY, VW, VH, cab + pie, remaches=False, esquinas=True)]
    # pestañas de índice
    x = VX + 40
    for i, (et, icono) in enumerate(SECCIONES if con_pestanas else []):
        w = 44 + len(et) * 12 + 16
        w += w % 2
        activa_ = i == activa
        y = VY + 58 if activa_ else VY + 62
        h = PY - y + (2 if activa_ else -2)
        partes.append(pestana(x, y, w, h + (h % 2), f'{ic(icono, 1)}{px(et, 2)}', activa_, semilla=30 + i))
        x += w + 6
    # pila: dos hojas por debajo asoman a la derecha y abajo
    partes.append(hoja(PX + 14, PY + 6, PW, PH - 6, "", semilla=41, rasgado=1, manchas=4, clase="hoja"))
    partes.append(hoja(PX + 7, PY + 3, PW, PH - 3, "", semilla=42, rasgado=1, manchas=4, clase="hoja"))
    return "".join(partes)


def lomo():
    remaches = "".join(f'<i style="left:{x}px"></i>' for x in (24, PW // 2 - 4, PW - 32))
    return f'<div class="lomo" style="left:{PX - 8}px;top:{PY + PH - 4}px;width:{PW + 16}px">{remaches}</div>'


def pagina_inventario():
    lista = f'''<div style="padding:4px 0 0 0;display:flex;flex-direction:column;gap:10px">
      <div class="fila-flex t3" style="padding:0 12px 0 36px">{px("OBJETO", 2)}<span style="margin-left:auto">{px("CANT.", 2)}</span></div>
      <div class="lista">{fila("Poción", "consumible", px("×2", 2), "foco")}{fila("Éter", "consumible", px("×1", 2), "", estilo_icono=ETER)}</div></div>'''
    detalle = f'''<div class="detalle" style="padding:0 8px 0 32px;gap:14px">
      <div class="fila-flex" style="gap:16px">{ic("consumible")}<h2>Poción</h2>{insignia("CONSUMIBLE")}</div>
      <div class="efecto">{ic("pv", 1)}Recupera 15 PV.</div>
      <dl class="datos"><dt>Tienes</dt><dd>2</dd><dt>Tus PV</dt><dd>28 de 28</dd></dl>
      <div class="acciones">{boton("USAR", "deshab")}
        <span class="motivo-boton">{px("!", 2, estilo="color:var(--aviso)")}Tus PV ya están al máximo. No se gasta nada.</span></div></div>'''
    sep = '<div style="width:2px;background:repeating-linear-gradient(180deg,#6B5038 0 4px,transparent 4px 8px);opacity:.6;margin:8px 0"></div>'
    return f'''<div style="position:absolute;inset:26px 40px 30px 40px;display:flex;flex-direction:column;gap:10px">
      <div class="fila-flex" style="justify-content:center;gap:16px;color:var(--tinta-2)">{filete(180)}{px("INVENTARIO", 2)}{filete(180)}</div>
      <div style="display:grid;grid-template-columns:360px 2px 1fr;gap:0 18px;flex:1;min-height:0">{lista}{sep}{detalle}</div>
    </div>
    <div class="folio" style="right:34px;bottom:14px">{px("1/6", 2)}</div>'''


def P04a_cuaderno():
    top = hoja(PX, PY, PW, PH, pagina_inventario(), semilla=40, rasgado=1, manchas=2, filete=6)
    cuerpo = cuaderno_marco(0) + top + lomo()
    return pagina("Menú del jugador · cuaderno", cuerpo, fondo="fondo_ciudad_anciana", velo="rgba(24,14,6,.62)")


# ---------------------------------------------- Papiro (menú de una página)
def P05a_papiro():
    W, H = 480, 640
    x, y = (1280 - W) // 2, 40
    filas = "".join(fila(t, None, "", "foco" if t == "Guardar partida" else "") for t in
                    ("Reanudar", "Personaje", "Guardar partida", "Cargar partida", "Opciones"))
    salidas = "".join(fila(t) for t in ("Volver al título", "Salir del juego"))
    cont = f'''<div class="col" style="position:absolute;inset:42px 44px 40px 44px;gap:8px">
      <div class="col" style="align-items:center;gap:10px;margin-bottom:6px">{px("EN PAUSA", 3)}{filete(300)}</div>
      <div class="lista">{filas}</div>
      <div style="display:flex;justify-content:center;margin:4px 0">{filete(260)}</div>
      <div class="lista">{salidas}</div>
      <div class="linea-estado exito" style="margin-top:auto;padding:0 12px">{sprite("sello", T.sello)}
        <span class="col" style="line-height:1.3"><span>Partida guardada</span><span class="d">Centro de Boundington · 17:42</span></span></div>
      <div class="fila-flex" style="gap:22px;justify-content:center;padding-top:8px">{ayuda("ENTER", "Aceptar")}{ayuda("ESC", "Reanudar")}</div>
    </div>
    <style>.papiro .fila{{font-size:22px}}</style>'''
    cuerpo = papiro(x, y, W, H, cont)
    return pagina("Pausa · papiro", cuerpo, fondo="fondo_ciudad_anciana", velo="rgba(24,14,6,.62)")


PANTALLAS = {"P03a_hud_pergamino": P03a_hud, "P03b_dialogo_pergamino": P03b_dialogo,
             "P04a_cuaderno_inventario": P04a_cuaderno, "P05a_papiro_pausa": P05a_papiro}

if __name__ == "__main__":
    pedidas = sys.argv[1:] or list(PANTALLAS)
    nombres = []
    for n in PANTALLAS:
        if any(n.startswith(p) for p in pedidas):
            escribir(n, PANTALLAS[n]())
            nombres.append(n)
    render(nombres)
    destino = os.path.join(SALIDA, "pergamino")
    os.makedirs(destino, exist_ok=True)
    for n in nombres:  # las maquetas del tema van a salida/pergamino/
        os.replace(os.path.join(SALIDA, n + ".png"), os.path.join(destino, n + ".png"))
