"""Maquetas del modelo visual del juego (1280x720).

python3 pantallas.py            -> todas
python3 pantallas.py M06 M07a   -> algunas

Contenido: nombres, textos, precios y reglas salen del repo (catálogos,
objetos, aventuras y código). Los valores de ejemplo están listados en
MODELO_VISUAL.md §8 (p. ej. la hora de guardado o los PV tras combatir).
"""
import sys

from lib import (ETER, ayuda, boton, bloque_titulo, contador, deslizador, e, escribir, fila, ic, insignia,
                 moral, pagina, pestanas, px, recurso, render, selector, tecla)

NOMBRE = "Nerea"          # de la lista de nombres sugeridos (no «Luisarda»: ver §7)
RAZA = "Aarakocra de las Montañas"
CLASE = "Acechador de Tejados"
TRASFONDO = "Mes de la Agonía"
PANTALLAS = {}


def pantalla(fn):
    PANTALLAS[fn.__name__.split("_")[0]] = fn
    return fn


def oro(n, s=2):
    return f'<span class="fila-flex" style="gap:6px">{ic("oro", 1)}{px(str(n), s)}</span>'


# ------------------------------------------------------------------ HUD
def hud(compacto=False, objetivo=True, oro_n=150, pv=28):
    """HUD de exploración. compacto=True: durante un diálogo solo queda
    el bloque del personaje con PV y PM (no se ocultan riesgos activos)."""
    if compacto:
        return f'''
<div class="capa marco" style="left:24px;top:24px;width:300px;padding:12px 16px 14px">
  <div class="fila-flex" style="margin-bottom:10px"><span style="font-size:20px;font-weight:600;line-height:1">{NOMBRE}</span>{insignia("T1")}</div>
  <div class="col" style="gap:10px">{recurso("pv", "PV", pv, 28, 264, "pv")}{recurso("pm", "PM", 8, 8, 264, "pm")}</div>
</div>'''
    op = ""
    bloque = f'''
<div class="capa col" style="left:24px;top:24px;width:300px;gap:12px">
<div class="marco" style="padding:14px 16px 16px">
  <div class="fila-flex" style="margin-bottom:12px">
    <span style="font-size:22px;font-weight:600;line-height:1">{NOMBRE}</span>{insignia("T1")}
    <span style="margin-left:auto">{oro(oro_n)}</span>
  </div>
  <div class="col" style="gap:12px">
    {recurso("pv", "PV", pv, 28, 264, "pv")}
    {recurso("pm", "PM", 8, 8, 264, "pm")}
    {moral(0, "NEUTRAL", 264)}
  </div>
</div>'''
    obj = ""
    if objetivo:
        obj = f'''
<div class="marco objetivo">
  <div class="mision">{ic("mision_activa", 1)}{px("OBJETIVO", 2)}<span class="t3" style="margin-left:auto">{px("0/3", 2)}</span></div>
  <div class="sec t2" style="line-height:1.3">El primer día en Boundington</div>
  <div class="paso">{ic("objetivo", 1, estilo="margin-top:3px")}<span>Presentarte a los tres comerciantes de la plaza</span></div>
  <div class="meta t3" style="padding-left:24px">Mercado, joyería y sastrería</div>
</div>'''
    obj += "</div>"
    mapa = f'''
<div class="capa marco minimapa" style="right:24px;top:24px;width:248px;{op}">
  <div class="zona"><span class="sec b6" style="color:var(--texto)">Centro de Boundington</span></div>
  <img src="img/minimapa_ciudad.png" width="224" height="136" alt="Minimapa">
  <div class="leyenda">
    <span><b style="color:var(--texto)">{px("▲", 2)}</b>Tú</span>
    <span><b style="color:var(--texto-2)">{px("●", 2)}</b>Personaje</span>
    <span><b style="color:var(--info)">{px("■", 2)}</b>Entrada</span>
    <span><b style="color:var(--acento)">{px("◆", 2)}</b>Objetivo</span>
  </div>
</div>'''
    teclas = f'''
<div class="capa marco" style="right:24px;bottom:24px;padding:8px 14px;display:flex;gap:18px;{op}">
  {ayuda("TAB", "Menú")}{ayuda("ESC", "Pausa")}
</div>'''
    return bloque + obj + mapa + teclas


def marca_suelo(cx, cy, color="var(--acento)", w=64, h=32):
    """Rombo de la baldosa del personaje con el que se interactúa."""
    return (f'<svg class="capa" style="left:{cx - w // 2}px;top:{cy - h // 2}px" width="{w}" height="{h}" '
            f'viewBox="0 0 {w} {h}" shape-rendering="crispEdges">'
            f'<polygon points="{w // 2},1 {w - 1},{h // 2} {w // 2},{h - 1} 1,{h // 2}" fill="none" '
            f'stroke="#0B0D12" stroke-width="5"/>'
            f'<polygon points="{w // 2},1 {w - 1},{h // 2} {w // 2},{h - 1} 1,{h // 2}" fill="none" '
            f'stroke="{color}" stroke-width="2"/></svg>')


# ------------------------------------------------------- M01 · Título
@pantalla
def M01_titulo():
    cuerpo = f'''
<div class="velo" style="background:linear-gradient(90deg, rgba(10,11,16,.92) 0%, rgba(10,11,16,.78) 42%, rgba(10,11,16,.05) 75%)"></div>
<div class="capa" style="left:96px;top:92px;color:var(--texto)">
  <div class="sombra-px3">{px("LOS PERDIDOS", 6)}</div>
  <div class="sombra-px3" style="margin-top:14px">{px("DE BOUNDINGTON", 6)}</div>
  <div class="it t2" style="font-size:20px;margin-top:18px">Boundington, año 1981 b.f.</div>
</div>
<div class="capa marco" style="left:96px;top:330px;width:400px;padding:12px 8px">
  <div class="lista">
    {fila("Continuar", None, "", "deshab", motivo="No hay partidas guardadas")}
    {fila("Nueva partida", None, "", "foco")}
    {fila("Opciones")}
    {fila("Salir del juego")}
  </div>
</div>
<div class="capa" style="left:96px;bottom:28px;display:flex;gap:24px">
  {ayuda(["W", "S"], "Elegir")}{ayuda("ENTER", "Aceptar")}
</div>
<style>.pantalla .fila{{font-size:var(--t-grande);min-height:52px}}</style>
'''
    return pagina("Título", cuerpo, fondo="fondo_plaza_razas")


# ------------------------------------------------- Asistente de creación
ETAPAS = ["ORIGEN", "ATRIBUTOS", "HISTORIA", "IDENTIDAD", "REVISIÓN"]


def asistente(actual, paso, pregunta, ayuda_txt, cuerpo, pie, total=14):
    etapas = []
    for i, n in enumerate(ETAPAS):
        if i < actual:
            etapas.append(f'<span class="etapa hecha">{px("✓", 2)}{px(n, 2)}</span>')
        elif i == actual:
            etapas.append(f'<span class="etapa actual">{px(str(i + 1) if i < 4 else "·", 2)}{px(n, 2)}</span>')
        else:
            etapas.append(f'<span class="etapa">{px(str(i + 1) if i < 4 else "·", 2)}{px(n, 2)}</span>')
    etapas_html = '<span class="etapa-sep"></span>'.join(etapas)
    return f'''
<div class="capa marco ventana" style="left:24px;top:24px;width:1232px;height:672px">
  <div class="cabecera">
    {px("NUEVA PARTIDA", 3)}
    <div class="derecha"><div class="etapas">{etapas_html}</div></div>
  </div>
  <div style="display:flex;align-items:baseline;gap:18px;padding:18px 28px 6px 28px">
    <span style="font-size:28px;font-weight:600;line-height:1.2">{pregunta}</span>
    <span class="sec t2">{ayuda_txt}</span>
    <span class="meta t3" style="margin-left:auto;white-space:nowrap">Paso {paso} de {total}</span>
  </div>
  <div class="cuerpo">{cuerpo}</div>
  <div class="pie">{pie}</div>
</div>'''


@pantalla
def M02a_creacion_raza():
    razas = ["Aarakocra de las Montañas", "Aarakocra del Viento", "Alquimistas de Aegroum", "Cara Loca",
             "Cazador Nocturno", "Diplomáticos de Bastrea", "Dracónidos de Fuego", "Dracónidos de Hielo",
             "Drow de la Casa Noble"]
    filas = "".join(fila(r, None, "", "foco" if i == 0 else "") for i, r in enumerate(razas))
    lista = f'''
<div style="display:flex;flex-direction:column;gap:10px;padding:10px 0 12px 20px;min-height:0">
  <div class="fila-flex" style="height:40px;padding:0 12px;background:var(--pista);box-shadow:inset 0 0 0 2px var(--divisor)">
    <span class="t3 sec it">Buscar raza…</span>
    <span style="margin-left:auto" class="t3">{px("1/43", 2)}</span>
  </div>
  <div style="position:relative;flex:1">
    <div class="lista">{filas}</div>
    <div class="scroll" style="top:0;height:410px;right:-14px"><i style="top:0;height:86px"></i></div>
  </div>
</div>'''
    detalle = f'''
<div class="detalle" style="padding:10px 32px 16px 36px">
  <h2>Aarakocra de las Montañas</h2>
  <div class="etiquetas">{insignia("CAR +2", icono="car")}{insignia("DES +2", icono="des")}</div>
  <div class="desc">Están profundamente conectados con las corrientes de aire y las montañas.</div>
  {bloque_titulo("EN JUEGO")}
  <dl class="datos">
    <dt>Afinidades</dt><dd>Percepción, Atletismo</dd>
    <dt>Rasgo</dt><dd>Competencia en Supervivencia (Sabiduría) en terrenos montañosos.</dd>
  </dl>
  {bloque_titulo("CULTURA")}
  <dl class="datos">
    <dt>Madurez</dt><dd>Hacia los 15 años; viven hasta los 100</dd>
    <dt>Tamaño</dt><dd>Mediano, entre 1,50 m y 1,90 m</dd>
    <dt>Creencias</dt><dd>Sofía y Eros. Respetan los elementos del aire y el conocimiento ancestral.</dd>
  </dl>
</div>'''
    cuerpo = f'<div style="display:grid;grid-template-columns:420px 1fr;min-height:0">{lista}<div style="border-left:2px solid var(--divisor);display:flex">{detalle}</div></div>'
    pie = (ayuda(["W", "S"], "Elegir") + ayuda("ENTER", "Continuar") + ayuda("ESC", "Volver al título")
           + f'<span class="derecha">{boton("CONTINUAR", "primario", icono=None)}</span>')
    return pagina("Nueva partida · Raza", asistente(0, 1, "Elige tu raza",
                  "Da bonificaciones de atributo y un rasgo.", cuerpo, pie))


@pantalla
def M02b_creacion_atributos():
    stats = [("con", "Constitución", "CON", 4, 0, True, True, ""),
             ("des", "Destreza", "DES", 4, 2, True, True, "Principal de tu clase"),
             ("int", "Inteligencia", "INT", 3, 0, True, True, "Secundario de tu clase"),
             ("car", "Carisma", "CAR", 1, 2, False, True, "")]
    filas = []
    for i, (icn, nom, ab, base, raza, menos, mas, nota) in enumerate(stats):
        foco = i == 2
        total = base + raza
        extra = f'<span class="t3 sec">+{raza} raza</span>' if raza else '<span class="t3 sec">—</span>'
        nota_html = f'<span class="meta {"t2" if foco else "t3"}">{nota}</span>' if nota else ""
        filas.append(f'''
<div class="fila {'foco' if foco else ''}" style="grid-template-columns:16px 32px 1fr auto 90px 70px;min-height:68px;padding-right:20px">
  <span class="marca">{px("▶", 2)}</span>{ic(icn)}
  <span class="principal"><span class="nombre">{nom} <span class="t3" style="font-weight:400">· {ab}</span></span>{nota_html}</span>
  {contador(base, menos, mas)}
  <span style="text-align:center">{extra}</span>
  <span style="text-align:right">{px(str(total), 3)}</span>
</div>''')
    lista = f'''
<div style="padding:12px 0 12px 20px;display:flex;flex-direction:column;gap:8px">
  <div class="fila-flex t3" style="padding:0 20px 0 68px;justify-content:flex-end;gap:0">
    <span style="width:140px;text-align:center">{px("BASE", 2)}</span><span style="width:90px;text-align:center">{px("RAZA", 2)}</span><span style="width:70px;text-align:right">{px("TOTAL", 2)}</span>
  </div>
  <div class="lista" style="gap:6px">{''.join(filas)}</div>
</div>'''
    lateral = f'''
<div class="detalle" style="padding:18px 32px;gap:14px">
  {bloque_titulo("PUNTOS POR REPARTIR")}
  <div class="fila-flex" style="gap:14px;align-items:flex-end">
    <span class="av">{px("4", 6)}</span><span class="t2" style="font-size:20px;padding-bottom:2px">de 12</span>
  </div>
  <div class="sec t2" style="line-height:1.45">Cada atributo va de 1 a 6. Reparte los 12 puntos para continuar.</div>
  {bloque_titulo("TU CLASE")}
  <div class="sec" style="line-height:1.45">{CLASE}<br><span class="t2">Atributo principal: Destreza · secundario: Inteligencia</span></div>
  {bloque_titulo("TU RAZA")}
  <div class="sec" style="line-height:1.45">{RAZA}<br><span class="t2">Carisma +2 · Destreza +2</span></div>
</div>'''
    cuerpo = f'<div style="display:grid;grid-template-columns:1fr 400px;min-height:0">{lista}<div style="border-left:2px solid var(--divisor);display:flex">{lateral}</div></div>'
    pie = (ayuda(["W", "S"], "Atributo") + ayuda(["←", "→"], "Quitar o poner") + ayuda("ESC", "Paso anterior")
           + f'<span class="derecha"><span class="motivo-boton">{px("!", 2, estilo="color:var(--aviso)")}Te quedan 4 puntos por repartir</span>'
           + boton("CONTINUAR", "deshab") + '</span>')
    return pagina("Nueva partida · Atributos", asistente(1, 4, "Reparte tus atributos",
                  "Se suman a lo que da tu raza.", cuerpo, pie))


@pantalla
def M02c_creacion_revision():
    def grupo(titulo, contenido, ancho):
        return f'''
<div style="display:flex;flex-direction:column;gap:12px;padding:16px 24px;width:{ancho}px">
  <div class="fila-flex">{px(titulo, 2, estilo="color:var(--texto-2)")}<span style="margin-left:auto">{boton("EDITAR")}</span></div>
  {contenido}
</div>'''
    ident = f'''
<div style="font-size:30px;font-weight:600;line-height:1.1">{NOMBRE}</div>
<dl class="datos apilados">
  <dt>Raza</dt><dd>{RAZA}</dd><dt>Clase</dt><dd>{CLASE}</dd><dt>Trasfondo</dt><dd>{TRASFONDO}</dd>
  <dt>Deidad</dt><dd>Sin deidad</dd>
</dl>'''
    def stat(icn, ab, total, det):
        return f'<div class="fila-flex" style="height:34px">{ic(icn, 1)}{px(ab, 2)}<span style="margin-left:10px">{px(str(total), 3)}</span><span class="t3 sec" style="margin-left:10px">{det}</span></div>'
    atrib = f'''
<div class="col" style="gap:2px">
  {stat("con", "CON", 4, "base 4")}{stat("des", "DES", 6, "base 4 + 2 raza")}
  {stat("int", "INT", 4, "base 4")}{stat("car", "CAR", 6, "base 4 + 2 raza")}
</div>
<div class="fila-flex sec" style="gap:18px">{ic("pv", 1)}{px("PV 28", 2)}{ic("pm", 1)}{px("PM 8", 2)}</div>
<dl class="datos" style="row-gap:6px">
  <dt>Rasgo</dt><dd>Terreno Elevado</dd>
  <dt>Equipo</dt><dd>Arco Cazador de Viaje, Armadura de Cuero, Botas de Cazador</dd>
</dl>'''
    hist = [("Vínculo", "El banco del cementerio"), ("Miedo", "La pena sin testigos"),
            ("Defecto", "Envidia silenciosa"), ("Meta", "Aprender la canción de mi casa"),
            ("Ideal", "Crecer no es competir"), ("Personalidad", "Acompaña en silencio"),
            ("Virtud", "Comprende el sufrimiento ajeno")]
    historia = '<dl class="datos" style="row-gap:9px">' + "".join(f"<dt>{a}</dt><dd>{b}</dd>" for a, b in hist) + "</dl>"
    cuerpo = f'''
<div style="display:flex;min-height:0;padding:4px 4px 0 4px">
  {grupo("IDENTIDAD", ident, 360)}
  <div style="width:2px;background:var(--divisor);margin:16px 0"></div>
  {grupo("ATRIBUTOS", atrib, 404)}
  <div style="width:2px;background:var(--divisor);margin:16px 0"></div>
  {grupo("HISTORIA PERSONAL", historia, 440)}
</div>'''
    pie = (ayuda(["W", "S"], "Elegir") + ayuda("ENTER", "Aceptar") + ayuda("ESC", "Paso anterior")
           + f'<span class="derecha">{boton("EMPEZAR AVENTURA", "primario", foco=True)}</span>')
    return pagina("Nueva partida · Revisión", asistente(4, 14, "Revisa tu personaje",
                  "Editar un grupo no borra los demás.", cuerpo, pie))


# ----------------------------------------------------- M03 · Exploración
ANCIANA = (702, 372)   # centro de su baldosa en fondo_ciudad_anciana


@pantalla
def M03a_hud_exploracion():
    cx, cy = ANCIANA
    cuerpo = hud() + marca_suelo(cx, cy) + f'''
<div class="prompt" style="left:{cx - 118}px;top:{cy - 104}px;width:236px;justify-content:center">
  {tecla("E")}<span>Hablar con <b>Anciana</b></span>
</div>
<div class="capa aviso" style="left:440px;top:24px;width:400px">
  {ic("mision_activa")}<span class="col"><span class="t">Nuevo objetivo</span><span class="d">Presentarte a los tres comerciantes de la plaza</span></span>
</div>'''
    return pagina("HUD de exploración", cuerpo, fondo="fondo_ciudad_anciana")


@pantalla
def M03b_dialogo():
    cx, cy = ANCIANA
    cuerpo = hud(compacto=True) + marca_suelo(cx, cy) + f'''
<div class="dialogo" style="left:180px;bottom:28px;width:920px">
  <div class="marco caja">
    <div class="texto">Antes esta plaza era un huerto.</div>
    <div class="pie-dialogo">
      <span class="t3">{px("1/2", 2)}</span>
      <span class="derecha">{ayuda("ENTER", "Continuar")}</span>
    </div>
  </div>
  <div class="hablante">{px("ANCIANA", 2)}</div>
</div>'''
    return pagina("Diálogo", cuerpo, fondo="fondo_ciudad_anciana")


# ------------------------------------------------- M04 · Menú del jugador
SECCIONES = [("INVENTARIO", "equipo"), ("EQUIPO", "ranura_torso"), ("FICHA", "personaje"),
             ("ACCIONES", "dote"), ("MISIONES", "mision_activa"), ("CÓDICE", "lore")]


def ventana_jugador(activa, cuerpo, pie):
    return f'''
<div class="capa marco ventana" style="left:80px;top:60px;width:1120px;height:600px">
  <div class="cabecera">
    <span class="nombre">{NOMBRE}</span>{insignia("T1")}
    <span class="contexto">{RAZA} · {CLASE}</span>
    <div class="derecha">
      <span class="fila-flex">{ic("pv", 1)}{px("28/28", 2)}</span>
      <span class="fila-flex">{ic("pm", 1)}{px("8/8", 2)}</span>
      {oro(150)}
    </div>
  </div>
  {pestanas(SECCIONES, activa)}
  <div class="cuerpo">{cuerpo}</div>
  <div class="pie">{pie}</div>
</div>'''


@pantalla
def M04a_tab_inventario():
    lista = f'''
<div style="padding:14px 0 14px 16px;display:flex;flex-direction:column;gap:10px">
  <div class="fila-flex t3" style="padding:0 12px 0 36px">{px("OBJETO", 2)}<span style="margin-left:auto">{px("CANT.", 2)}</span></div>
  <div class="lista">
    {fila("Poción", "consumible", px("×2", 2), "foco")}
    {fila("Éter", "consumible", px("×1", 2), "", estilo_icono=ETER)}
  </div>
</div>'''
    detalle = f'''
<div class="detalle" style="padding:20px 32px">
  <div class="fila-flex" style="gap:16px">{ic("consumible")}<h2>Poción</h2>{insignia("CONSUMIBLE")}</div>
  <div class="efecto">{ic("pv", 1)}Recupera 15 PV.</div>
  <dl class="datos"><dt>Tienes</dt><dd>2</dd><dt>Tus PV</dt><dd>28 de 28</dd></dl>
  <div class="acciones">{boton("USAR", "deshab")}
    <span class="motivo-boton">{px("!", 2, estilo="color:var(--aviso)")}Tus PV ya están al máximo. No se gasta nada.</span>
  </div>
</div>'''
    cuerpo = f'<div style="display:grid;grid-template-columns:400px 1fr;min-height:0">{lista}<div style="border-left:2px solid var(--divisor);display:flex">{detalle}</div></div>'
    pie = (ayuda(["←", "→"], "Sección") + ayuda(["W", "S"], "Objeto") + ayuda("ENTER", "Usar")
           + f'<span class="derecha">{ayuda("TAB", "Cerrar")}</span>')
    return pagina("Menú del jugador · Inventario", ventana_jugador(0, cuerpo, pie),
                  fondo="fondo_ciudad_anciana", velo=True)


@pantalla
def M04b_tab_ficha():
    def stat(icn, nombre, ab, total, det):
        return (f'<div class="fila-flex" style="height:40px">{ic(icn)}<span style="width:130px">{nombre}</span>'
                f'<span class="t3" style="width:44px">{px(ab, 2)}</span>{px(str(total), 3)}'
                f'<span class="t3 sec" style="margin-left:14px;white-space:nowrap">{det}</span></div>')
    col1 = f'''
<div class="col" style="gap:14px;padding:18px 24px;width:330px">
  {bloque_titulo("IDENTIDAD")}
  <dl class="datos apilados">
    <dt>Raza</dt><dd>{RAZA}</dd><dt>Clase</dt><dd>{CLASE}</dd>
    <dt>Trasfondo</dt><dd>{TRASFONDO}</dd><dt>Deidad</dt><dd>Sin deidad</dd>
  </dl>
  {bloque_titulo("ESTADO")}
  <dl class="datos" style="row-gap:7px">
    <dt>Moral</dt><dd>Neutral (0)</dd><dt>Condiciones</dt><dd>Ninguna</dd>
  </dl>
</div>'''
    col2 = f'''
<div class="col" style="gap:12px;padding:18px 24px;width:430px">
  {bloque_titulo("ATRIBUTOS")}
  <div class="col">
    {stat("con", "Constitución", "CON", 4, "base 4")}{stat("des", "Destreza", "DES", 6, "4 + 2 raza")}
    {stat("int", "Inteligencia", "INT", 4, "base 4")}{stat("car", "Carisma", "CAR", 6, "4 + 2 raza")}
  </div>
  {bloque_titulo("RASGOS")}
  <div class="col" style="gap:4px">
    <div class="fila-flex b6">{ic("pasiva", 1)}Terreno Elevado</div>
    <div class="meta t3 it" style="line-height:1.35">Mientras estés en posición elevada respecto a tu objetivo</div>
    <div class="sec t2" style="line-height:1.4">Tus ataques ganan +1 y tus pruebas para trepar o saltar tienen ventaja. Caer con estilo es parte del oficio: reduces todo daño por caída en tu DES.</div>
  </div>
</div>'''
    hist = [("Vínculo", "El banco del cementerio"), ("Miedo", "La pena sin testigos"),
            ("Defecto", "Envidia silenciosa"), ("Meta", "Aprender la canción de mi casa"),
            ("Ideal", "Crecer no es competir"), ("Personalidad", "Acompaña en silencio"),
            ("Virtud", "Comprende el sufrimiento ajeno")]
    col3 = f'''
<div class="col" style="gap:14px;padding:18px 24px;flex:1">
  {bloque_titulo("HISTORIA PERSONAL")}
  <dl class="datos apilados">{"".join(f"<dt>{a}</dt><dd>{b}</dd>" for a, b in hist)}</dl>
</div>'''
    sep = '<div style="width:2px;background:var(--divisor);margin:18px 0"></div>'
    cuerpo = f'<div style="display:flex;min-height:0">{col1}{sep}{col2}{sep}{col3}</div>'
    pie = ayuda(["←", "→"], "Sección") + f'<span class="derecha">{ayuda("TAB", "Cerrar")}</span>'
    return pagina("Menú del jugador · Ficha", ventana_jugador(2, cuerpo, pie),
                  fondo="fondo_ciudad_anciana", velo=True)


# ---------------------------------------------------------- M05 · Pausa
@pantalla
def M05a_pausa_guardado():
    cuerpo = f'''
<div class="capa marco ventana" style="left:380px;top:92px;width:520px;height:536px;grid-template-rows:var(--cabecera) 1fr var(--pie)">
  <div class="cabecera">{px("EN PAUSA", 3)}</div>
  <div style="padding:12px 8px;display:flex;flex-direction:column">
    <div class="lista">
      {fila("Reanudar")}
      {fila("Personaje")}
      {fila("Guardar partida", None, "", "foco")}
      {fila("Cargar partida")}
      {fila("Opciones")}
    </div>
    <div class="separador"></div>
    <div class="lista">{fila("Volver al título")}{fila("Salir del juego")}</div>
    <div class="linea-estado exito" style="margin:auto 12px 4px 12px;padding-top:10px">
      {px("✓", 2)}<span class="col" style="line-height:1.3"><span>Partida guardada</span><span class="d">Centro de Boundington · 17:42</span></span>
    </div>
  </div>
  <div class="pie">{ayuda(["W", "S"], "Elegir")}{ayuda("ENTER", "Aceptar")}<span class="derecha">{ayuda("ESC", "Reanudar")}</span></div>
</div>
<style>.pantalla .fila{{font-size:22px}}</style>'''
    return pagina("Pausa", cuerpo, fondo="fondo_ciudad_anciana", velo=True)


@pantalla
def M05b_opciones():
    def opcion(nombre, control, valor="", foco=False):
        return (f'<div class="fila {"foco" if foco else ""}" style="grid-template-columns:16px 1fr auto 64px;padding-right:20px">'
                f'<span class="marca">{px("▶", 2)}</span><span class="nombre">{nombre}</span>{control}'
                f'<span style="text-align:right">{px(valor, 2) if valor else ""}</span></div>')
    def grupo(t):
        return f'<div class="t3" style="padding:14px 12px 6px 36px">{px(t, 2)}</div>'
    cuerpo = f'''
<div class="capa marco ventana" style="left:240px;top:60px;width:800px;height:600px;grid-template-rows:var(--cabecera) 1fr auto var(--pie)">
  <div class="cabecera">{px("OPCIONES", 3)}<span class="contexto">Desde la pausa</span></div>
  <div style="padding:4px 12px 8px 8px">
    {grupo("SONIDO")}
    <div class="lista">
      {opcion("Música", deslizador(8), "80 %", True)}
      {opcion("Efectos", deslizador(8), "80 %")}
      {opcion("Ambiente", deslizador(8), "80 %")}
    </div>
    {grupo("IDIOMA Y TEXTO")}
    <div class="lista">
      {opcion("Idioma", selector("Español"))}
      {opcion("Tamaño del texto", selector("100 %"))}
    </div>
    {grupo("CONTROLES")}
    <div class="lista">{opcion("Ver teclas", '<span class="t2 sec">Teclado</span>', "→")}</div>
  </div>
  <div style="margin:0 2px;padding:12px 28px;background:var(--panel-2);border-top:2px solid var(--divisor)" class="sec t2">
    Música: volumen de 0 a 10; cada pulsación cambia un 10 %.
  </div>
  <div class="pie">{ayuda(["W", "S"], "Elegir")}{ayuda(["←", "→"], "Ajustar")}<span class="derecha">{ayuda("ESC", "Volver a la pausa")}</span></div>
</div>'''
    return pagina("Opciones", cuerpo, fondo="fondo_ciudad_anciana", velo=True)


# --------------------------------------------------------- M06 · Tienda
@pantalla
def M06_tienda():
    lista = f'''
<div style="padding:14px 0 14px 16px">
  <div class="lista">
    {fila("Poción", "consumible", px("30", 2) + ic("oro", 1), "foco", motivo="Tienes 3")}
    {fila("Éter", "consumible", px("45", 2) + ic("oro", 1), "", motivo="Tienes 1", estilo_icono=ETER)}
  </div>
</div>'''
    detalle = f'''
<div class="detalle" style="padding:18px 28px 18px 28px;gap:14px">
  <div class="fila-flex" style="gap:14px">{ic("consumible")}<h2>Poción</h2>{insignia("CONSUMIBLE")}</div>
  <div class="efecto">{ic("pv", 1)}Recupera 15 PV.</div>
  <dl class="datos"><dt>Precio</dt><dd>30 de oro</dd><dt>Tienes</dt><dd>3</dd><dt>Te quedarán</dt><dd>90 de oro</dd></dl>
  <div class="acciones">{boton("COMPRAR 1 · 30 ORO", "primario", icono="comprar")}</div>
</div>'''
    cuerpo = f'''
<div style="display:grid;grid-template-rows:1fr auto;min-height:0">
  <div style="display:grid;grid-template-columns:330px 1fr;min-height:0">{lista}<div style="border-left:2px solid var(--divisor);display:flex">{detalle}</div></div>
  <div class="linea-estado exito" style="padding:12px 28px;border-top:2px solid var(--divisor)">{px("✓", 2)}<span>Poción comprada</span><span class="d">−30 de oro · ahora tienes 3</span></div>
</div>'''
    ventana = f'''
<div class="capa marco ventana" style="left:170px;top:88px;width:940px;height:544px">
  <div class="cabecera">
    {px("TIENDA", 3)}<span class="nombre">Luisarda</span>
    <div class="derecha"><span class="sec t2">Tu oro</span>{oro(120, 3)}</div>
  </div>
  {pestanas([("COMPRAR", "comprar"), ("VENDER", "vender")], 0)}
  <div class="cuerpo">{cuerpo}</div>
  <div class="pie">{ayuda(["←", "→"], "Comprar / Vender")}{ayuda(["W", "S"], "Elegir")}{ayuda("ENTER", "Comprar")}
    <span class="derecha">{ayuda("ESC", "Cerrar tienda")}</span></div>
</div>'''
    return pagina("Tienda", ventana, fondo="fondo_sastreria_luisarda", velo=True)


# -------------------------------------------------------- M07 · Negocio
def ficha_negocio(insig, cuerpo, pie, alto=468):
    return f'''
<div class="capa marco ventana" style="left:320px;top:{(720 - alto) // 2}px;width:640px;height:{alto}px;grid-template-rows:var(--cabecera) 1fr var(--pie)">
  <div class="cabecera">{px("NEGOCIO", 3)}<span class="nombre">Sastrería</span><div class="derecha">{insig}</div></div>
  <div style="padding:20px 28px;display:flex;flex-direction:column;gap:14px">{cuerpo}</div>
  <div class="pie">{pie}</div>
</div>'''


@pantalla
def M07a_negocio_en_venta():
    cuerpo = f'''
<dl class="datos" style="font-size:20px;row-gap:10px">
  <dt>Precio</dt><dd class="fila-flex">{ic("oro", 1)}1000 de oro</dd>
  <dt>Renta</dt><dd>24 de oro por ciclo, con el alquiler al 100 %</dd>
  <dt>Ciclo</dt><dd>Cada 40 pasos tuyos se cobra una vez</dd>
  <dt>Tu oro</dt><dd class="fila-flex">{ic("oro", 1)}150</dd>
</dl>
<div class="aviso aviso-a" style="margin-top:6px"><span class="av">{px("!", 3)}</span><span class="col"><span class="t">Te faltan 850 de oro</span><span class="d" style="color:var(--texto-2)">Puedes consultar el negocio y volver cuando lo tengas.</span></span></div>
<div class="fila-flex" style="gap:16px;margin-top:auto">{boton("COMPRAR · 1000 ORO", "deshab")}{boton("VOLVER", foco=True)}</div>'''
    pie = ayuda(["←", "→"], "Elegir") + ayuda("ENTER", "Aceptar") + f'<span class="derecha">{ayuda("ESC", "Volver")}</span>'
    return pagina("Negocio en venta", ficha_negocio(insignia("EN VENTA", "info"), cuerpo, pie),
                  fondo="fondo_cartel_sastreria", velo=True)


@pantalla
def M07b_tu_negocio():
    cuerpo = f'''
<div class="fila foco" style="grid-template-columns:16px 1fr auto;min-height:60px;padding-right:16px">
  <span class="marca">{px("▶", 2)}</span><span class="nombre" style="font-size:22px">Alquiler</span>{contador("110 %", s=2)}
</div>
<dl class="datos" style="font-size:20px;row-gap:9px;padding-left:36px">
  <dt>Ocupación</dt><dd>96 %</dd>
  <dt>Ingresos</dt><dd>25 de oro por ciclo (40 pasos)</dd>
  <dt>Moral</dt><dd>Neutral · este cambio: −2</dd>
</dl>
<div class="sec t2" style="padding-left:36px;line-height:1.45">Con +10 %: ocupación 92 % y 26 de oro por ciclo.<br>Desde el 125 %, cada cobro resta 1 de moral.</div>
<div class="linea-estado" style="color:var(--info);margin-top:auto">{px("✓", 2)}<span>Alquiler subido al 110 %</span><span class="d">ocupación 96 %</span></div>'''
    pie = ayuda(["←", "→"], "Bajar / Subir") + f'<span class="derecha">{ayuda("ESC", "Volver")}</span>'
    return pagina("Tu negocio", ficha_negocio(insignia("TU NEGOCIO", "exito"), cuerpo, pie, alto=480),
                  fondo="fondo_cartel_sastreria", velo=True)


# -------------------------------------------------------- M08 · Combate
ENEMIGO = (703, 372)   # centro de la baldosa del Perdido saqueador en fondo_emboscada_adyacente


@pantalla
def M08a_combate():
    cx, cy = ENEMIGO
    placa = f'''
<div class="capa marco" style="left:{cx - 110}px;top:{cy - 138}px;width:220px;padding:8px 12px 10px">
  <div class="fila-flex sec b6" style="margin-bottom:6px">{px("▶", 2, estilo="color:var(--acento)")}Perdido saqueador</div>
  {recurso("pv", "PV", 22, 22, 192)}
</div>'''
    tira = f'''
<div class="capa marco" style="left:24px;bottom:24px;width:1232px;height:212px;display:grid;grid-template-columns:270px 290px 330px 1fr">
  <div class="col" style="padding:16px 20px;gap:12px">
    <span style="font-size:22px;font-weight:600;line-height:1">{NOMBRE}</span>
    {recurso("pv", "PV", 28, 28, 226, "pv")}{recurso("pm", "PM", 8, 8, 226, "pm")}
  </div>
  <div class="col" style="padding:14px 8px;gap:8px;border-left:2px solid var(--divisor)">
    <div class="t2" style="padding-left:36px">{px("ELIGE UNA ACCIÓN", 2)}</div>
    <div class="lista">
      {fila("Atacar", "ranura_mano_principal", "", "foco", x_icono=1)}
      {fila("Poción", "consumible", px("×2", 2), "", x_icono=1)}
      {fila("Huir", "cerrar", "", "", x_icono=1)}
    </div>
  </div>
  <div class="col" style="padding:16px 20px;gap:8px;border-left:2px solid var(--divisor)">
    <span class="b6" style="font-size:22px">Atacar</span>
    <span class="sec">Objetivo: <b>Perdido saqueador</b></span>
    <span class="sec t2" style="line-height:1.4">Ataque básico con tirada enfrentada. Puede salir fracaso, parcial, éxito o crítico.</span>
  </div>
  <div class="col" style="padding:16px 20px;gap:8px;border-left:2px solid var(--divisor)">
    <span class="t3">{px("ÚLTIMAS ACCIONES", 2)}</span>
    <span class="sec t2" style="line-height:1.4"><b>Saqueador:</b> <span class="it">«La bolsa. Y el anillo, si lo tienes.»</span></span>
    <span class="sec" style="line-height:1.4">Empieza el combate contra Perdido saqueador.</span>
  </div>
</div>'''
    pie_teclas = f'''
<div class="capa marco" style="right:24px;bottom:246px;display:flex;gap:18px;padding:8px 14px">
  {ayuda(["W", "S"], "Elegir")}{ayuda("ENTER", "Atacar")}
</div>'''
    cuerpo = marca_suelo(cx, cy) + placa + tira + pie_teclas
    return pagina("Combate", cuerpo, fondo="fondo_emboscada_adyacente")


@pantalla
def M08b_victoria():
    cuerpo = f'''
<div class="capa marco ventana" style="left:360px;top:170px;width:560px;grid-template-rows:auto var(--pie)">
  <div style="padding:30px 36px 20px;display:flex;flex-direction:column;gap:16px;align-items:flex-start">
    <span class="ok sombra-px3">{px("VICTORIA", 5)}</span>
    <span style="font-size:20px">Has vencido a Perdido saqueador.</span>
    {recurso("pv", "TUS PV", 19, 28, 488, "pv")}
    <div class="aviso exito" style="width:100%">{ic("objetivo_hecho")}<span class="col"><span class="t">Objetivo cumplido</span><span class="d">Sobrevivir al atraco en la plaza</span></span></div>
    <span class="sec t3">No hay botín.</span>
  </div>
  <div class="pie">{ayuda("ENTER", "Continuar")}<span class="derecha">{boton("CONTINUAR", "primario", foco=True)}</span></div>
</div>'''
    return pagina("Victoria", cuerpo, fondo="fondo_emboscada_vacia", velo="rgba(10,11,16,.55)")


# --------------------------------------------------------- M09 · Derrota
@pantalla
def M09_derrota():
    cuerpo = f'''
<div class="capa" style="left:0;right:0;top:92px;display:flex;flex-direction:column;align-items:center;gap:16px">
  <span class="sombra-px3" style="color:var(--texto)">{px("HAS CAÍDO", 6)}</span>
  <span class="it t2" style="font-size:20px">La partida ha terminado. Puedes volver a tu último guardado.</span>
</div>
<div class="capa marco" style="left:390px;top:262px;width:500px;padding:16px 12px 12px">
  <div class="tarjeta" style="margin:0 8px 12px">
    {ic("lugar")}<span class="t">Centro de Boundington</span><span class="t3 sec">{px("T1", 2)}</span>
    <span class="m">Hoy, 17:42 · Guardado rápido (F5) · {NOMBRE}</span>
  </div>
  <div class="lista">
    {fila("Cargar último guardado", None, "", "foco")}
    {fila("Elegir partida")}
    {fila("Volver al título")}
  </div>
</div>
<div class="capa" style="left:0;right:0;bottom:28px;display:flex;justify-content:center;gap:24px">
  {ayuda(["W", "S"], "Elegir")}{ayuda("ENTER", "Aceptar")}
</div>
<style>.pantalla .fila{{font-size:22px}}</style>'''
    return pagina("Derrota", cuerpo, fondo="fondo_emboscada_adyacente",
                  velo="rgba(12,10,14,.80)", filtro_fondo="grayscale(.85) brightness(.8)")


if __name__ == "__main__":
    pedidas = sys.argv[1:] or list(PANTALLAS)
    hechas = []
    for clave in pedidas:
        fn = PANTALLAS[clave]
        nombre = fn.__name__
        escribir(nombre, fn())
        hechas.append(nombre)
    render(hechas)
    from lib import USADOS_PROPUESTOS
    print("glifos propuestos usados:", "".join(sorted(USADOS_PROPUESTOS)))
