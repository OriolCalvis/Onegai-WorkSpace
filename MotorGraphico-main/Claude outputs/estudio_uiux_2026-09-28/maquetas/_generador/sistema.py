"""Láminas del sistema: S1 tokens y tipografía, S2 componentes y estados."""
import sys

from lib import (ETER, ayuda, boton, bloque_titulo, contador, deslizador, e, escribir, fila, ic, insignia,
                 moral, pagina, pestanas, px, recurso, render, selector, tecla)


def lum(h):
    h = h.lstrip("#")
    r, g, b = [int(h[i:i + 2], 16) / 255 for i in (0, 2, 4)]
    f = lambda c: c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
    return 0.2126 * f(r) + 0.7152 * f(g) + 0.0722 * f(b)


def cr(a, b):
    la, lb = lum(a), lum(b)
    return (max(la, lb) + 0.05) / (min(la, lb) + 0.05)


PANEL = "#222735"
GRUPOS = [
    ("SUPERFICIES", [("fondo", "#151820"), ("panel", "#222735"), ("panel-2", "#2B3142"), ("panel-3", "#353C51"), ("pista", "#12141B")]),
    ("MARCO", [("marco-ext", "#0B0D12"), ("marco-int", "#5A6386"), ("divisor", "#3A4158")]),
    ("TEXTO", [("texto", "#F3EBDD"), ("texto-2", "#C1C9D6"), ("texto-3", "#8F99AB")]),
    ("SEMÁNTICOS", [("acento", "#EBC77D"), ("éxito", "#91D7B0"), ("aviso", "#F2A65A"), ("error", "#FFB2AC"), ("info", "#9CC3FF")]),
    ("RECURSOS", [("pv", "#E8705F"), ("pm", "#6A9BF4"), ("moral", "#B79CF0")]),
]


def muestra(nombre, hexa, con_ratio=True):
    ratio = cr(hexa, PANEL)
    txt = (f"{ratio:.1f}:1".replace(".", ",") + " sobre panel") if con_ratio else "&nbsp;"
    return f'''
<div class="col" style="gap:3px;width:136px">
  <div style="height:36px;background:{hexa};box-shadow:0 0 0 2px var(--marco-ext), inset 0 0 0 1px rgba(255,255,255,.06)"></div>
  <span class="meta b6" style="color:var(--texto);margin-top:3px">--{e(nombre.replace("é", "e").replace("á", "a"))}</span>
  <span class="meta t2" style="line-height:1.25">{hexa}</span>
  <span class="meta t3" style="line-height:1.25">{txt}</span>
</div>'''


def S1_tokens():
    paleta = "".join(
        f'<div class="col" style="gap:8px">{px(g, 2, estilo="color:var(--texto-3)")}<div class="fila-flex" style="gap:12px;align-items:flex-start">{"".join(muestra(n, h, g != "SUPERFICIES") for n, h in cs)}</div></div>'
        for g, cs in GRUPOS)
    espacios = "".join(f'<div class="col" style="gap:4px;align-items:center"><div style="width:{v}px;height:{v}px;background:var(--marco-int)"></div><span class="meta t2">{v}</span></div>'
                       for v in (4, 8, 16, 24, 32, 48))
    cuerpo = f'''
<div class="capa" style="left:40px;top:32px;right:40px;display:flex;align-items:baseline;gap:20px">
  {px("MODELO VISUAL · TOKENS", 3)}<span class="t2 sec it">Fantasía sobria con arte pixel, texto cómodo y paneles ordenados.</span>
</div>
<div class="capa col" style="left:40px;top:92px;width:760px;gap:20px">
  {paleta}
  <div class="col" style="gap:8px">
    {px("CONTRASTE", 2, estilo="color:var(--texto-3)")}
    <div class="sec t2" style="line-height:1.5">Texto normal ≥ 4,5:1 y barras o marcos ≥ 3:1. <b style="color:var(--texto)">--texto-3</b> da 5,2:1 sobre panel y 4,5:1 sobre panel-2, pero 3,8:1 sobre la fila con foco: ahí pasa a --texto-2. <b style="color:var(--texto)">--aviso</b> pasa a naranja para no confundirse con el dorado del foco.</div>
  </div>
  <div class="col" style="gap:8px">
    {px("ESPACIADO", 2, estilo="color:var(--texto-3)")}
    <div class="fila-flex" style="gap:22px;align-items:flex-end">{espacios}<span class="sec t2" style="margin-left:18px;line-height:1.45">Margen seguro 24 · cabecera 56<br>navegación 44 · pie 56 · fila 44</span></div>
  </div>
</div>
<div class="capa col" style="left:840px;top:92px;width:400px;gap:20px">
<div class="marco" style="padding:18px 20px;display:flex;flex-direction:column;gap:12px">
  {px("DOS VOCES", 2, estilo="color:var(--texto-3)")}
  <div class="col" style="gap:6px">
    <span class="meta t3">Pixel 5×7 del motor · x3 · títulos</span>{px("INVENTARIO · CÓDICE", 3)}
    <span class="meta t3" style="margin-top:6px">x2 · pestañas, teclas, insignias, cifras</span>{px("PV 28/28 · AÑO 1981 · CATALÀ", 2)}
  </div>
  <div class="col" style="gap:4px;border-top:2px solid var(--divisor);padding-top:10px">
    <span class="meta t3">Lora · lectura, nombres y descripciones</span>
    <span style="font-size:28px;font-weight:600;line-height:1.2">Aarakocra de las Montañas</span>
    <span style="font-size:20px">Recupera 15 PV. «La bolsa.» ¿Seguro?</span>
    <span style="font-size:18px" class="t2">Col·lecció, façana, añoranza, pingüino</span>
    <span style="font-size:16px" class="t3 it">No hay partidas guardadas</span>
  </div>
</div>
<div class="marco" style="padding:16px 20px;display:flex;flex-direction:column;gap:10px">
  {px("AJUSTE DEL PIXEL", 2, estilo="color:var(--texto-3)")}
  <div class="fila-flex" style="gap:14px"><span class="meta t3" style="width:74px">Hoy</span><span style="color:var(--acento)">{px("INVENTARIO · REVISIÓN", 2, hoy=True)}</span></div>
  <div class="fila-flex" style="gap:14px"><span class="meta t3" style="width:74px">Propuesta</span><span style="color:var(--texto)">{px("INVENTARIO · REVISIÓN", 2)}</span></div>
  <div class="meta t2" style="line-height:1.45">Avance de 6 columnas (5 + 1 de aire) y tildes en el interlineado: la letra no se aplasta y «Ó» ya no se lee «O».</div>
</div>
<div class="marco" style="padding:16px 20px;display:flex;flex-direction:column;gap:10px">
  {px("MARCO PIXEL", 2, estilo="color:var(--texto-3)")}
  <div class="meta t2" style="line-height:1.45">Exterior 2 px --marco-ext (se ve sobre suelos claros) + interior 2 px --marco-int (se ve sobre el fondo oscuro, 3,0:1) + esquinas recortadas 2 px. Panel siempre opaco.</div>
</div>
</div>'''
    return pagina("Sistema · tokens", cuerpo, alto=1130)


def S2_componentes():
    def cab(t, extra=""):
        return px(t, 2, estilo="color:var(--texto-3)" + extra)
    izq = f'''
<div class="capa col" style="left:40px;top:84px;width:380px;gap:10px">
  {cab("FILA DE LISTA · 44 PX")}
  <div class="marco" style="padding:8px 6px"><div class="lista">
    {fila("Normal", "consumible", px("30", 2) + ic("oro", 1))}
    {fila("Con foco", "consumible", px("30", 2) + ic("oro", 1), "foco")}
    {fila("Elegida", "deidad", "", "", elegida=True)}
    {fila("Deshabilitada", None, "", "deshab", motivo="Explica siempre el motivo")}
    {fila("Foco sobre deshabilitada", None, "", "foco deshab", motivo="El motivo sube a --texto-2")}
  </div></div>
  {cab("BOTONES", ";margin-top:6px")}
  <div class="fila-flex" style="gap:18px;flex-wrap:wrap;padding:4px">
    {boton("PRINCIPAL", "primario")}{boton("CON FOCO", "primario", foco=True)}{boton("SECUNDARIO")}
  </div>
  <div class="fila-flex" style="gap:14px;padding:4px">{boton("USAR", "deshab")}<span class="motivo-boton">{px("!", 2, estilo="color:var(--aviso)")}Motivo al lado</span></div>
  {cab("VALORES", ";margin-top:6px")}
  <div class="fila-flex" style="gap:16px">{contador(4)}{contador(1, menos=False)}</div>
  <div class="fila-flex foco" style="gap:12px">{deslizador(8, ancho=220)}<span>{px("80 %", 2)}</span></div>
  <div>{selector("Español")}</div>
</div>'''
    medio = f'''
<div class="capa col" style="left:460px;top:84px;width:380px;gap:10px">
  {cab("TECLAS Y AYUDAS")}
  <div class="fila-flex" style="gap:16px;flex-wrap:wrap">{ayuda(["W", "S"], "Elegir")}{ayuda("ENTER", "Aceptar")}{ayuda("ESC", "Volver")}{ayuda(["←", "→"], "Sección")}</div>
  {cab("INSIGNIAS", ";margin-top:6px")}
  <div class="fila-flex" style="gap:10px;flex-wrap:wrap">{insignia("T1")}{insignia("TU NEGOCIO", "exito")}{insignia("OPCIONAL", "info")}{insignia("SIN FONDOS", "aviso")}{insignia("DAÑADA", "error")}</div>
  {cab("RECURSOS", ";margin-top:6px")}
  <div class="marco col" style="padding:14px 16px;gap:12px">
    {recurso("pv", "PV", 28, 28, 344, "pv")}{recurso("pm", "PM", 3, 8, 344, "pm")}
    <div class="recurso pv" style="width:344px"><div class="linea">{ic("pv", 1)}{px("PV", 2)}<span class="cifra av">{px("! 5/28", 2)}</span></div><div class="barra"><i style="width:17.9%"></i></div></div>
    {moral(-45, "CANALLA", 344)}
  </div>
  {cab("EN EL MUNDO", ";margin-top:6px")}
  <div style="position:relative;height:52px"><div class="prompt" style="left:0;top:0">{tecla("E")}<span>Hablar con <b>Anciana</b></span></div></div>
  <div class="marco objetivo">
    <div class="mision">{ic("mision_activa", 1)}{px("OBJETIVO", 2)}<span class="t3" style="margin-left:auto">{px("0/3", 2)}</span></div>
    <div class="paso">{ic("objetivo", 1, estilo="margin-top:3px")}<span>Presentarte a los tres comerciantes de la plaza</span></div>
  </div>
</div>'''
    der = f'''
<div class="capa col" style="left:880px;top:84px;width:360px;gap:10px">
  {cab("RESPUESTA")}
  <div class="aviso exito">{px("✓", 2, estilo="color:var(--exito)")}<span class="col"><span class="t">Partida guardada</span><span class="d">Centro de Boundington · 17:42</span></span></div>
  <div class="aviso">{ic("mision_activa")}<span class="col"><span class="t">Diario actualizado</span><span class="d">Nueva pista en Misiones</span></span></div>
  <div class="aviso aviso-a"><span class="av">{px("!", 3)}</span><span class="col"><span class="t">Te faltan 850 de oro</span><span class="d">Precio 1000 · tienes 150</span></span></div>
  <div class="aviso error"><span class="ko">{px("!", 3)}</span><span class="col"><span class="t">No se ha podido guardar</span><span class="d" style="color:var(--texto-2)">Pulsa ENTER para reintentar</span></span></div>
  <div class="aviso">{ic("consumible", estilo=ETER)}<span class="col"><span class="t">Éter ×1</span><span class="d">Recogido · ahora tienes 2</span></span></div>
  {cab("TARJETA DE PARTIDA", ";margin-top:6px")}
  <div class="tarjeta">{ic("lugar")}<span class="t">Centro de Boundington</span><span class="t3">{px("T1", 2)}</span><span class="m">Hoy, 17:42 · Guardado rápido (F5)</span></div>
  {cab("ESTADO VACÍO", ";margin-top:6px")}
  <div class="marco vacio" style="padding:18px">{ic("equipo")}<span class="t">No llevas objetos</span><span class="d">Compra en las tiendas de la plaza o recoge lo que encuentres.</span></div>
</div>'''
    abajo = f'''
<div class="capa col" style="left:40px;top:830px;width:1200px;gap:10px">
  {cab("NAVEGACIÓN")}
  <div class="marco" style="width:760px;padding:0">{pestanas([("INVENTARIO", "equipo"), ("EQUIPO", "ranura_torso"), ("FICHA", "personaje"), ("MISIONES", "mision_activa")], 0)}</div>
  {cab("DIÁLOGO", ";margin-top:14px")}
  <div style="position:relative;height:150px">
    <div class="dialogo" style="left:0;top:34px;width:760px">
      <div class="marco caja" style="padding:22px 24px 12px"><div class="texto" style="font-size:20px;min-height:0">Boundington huele a piedra mojada y a humo de cocina.</div>
        <div class="pie-dialogo"><span class="t3">{px("1/3", 2)}</span><span class="derecha">{ayuda("ENTER", "Continuar")}</span></div></div>
      <div class="hablante">{px("NARRACIÓN", 2)}</div>
    </div>
  </div>
</div>'''
    cabecera = f'''<div class="capa" style="left:40px;top:32px;right:40px;display:flex;align-items:baseline;gap:20px">
  {px("MODELO VISUAL · COMPONENTES", 3)}<span class="t2 sec it">Foco = barra dorada, marcador y fondo. Nunca solo color.</span></div>'''
    return pagina("Sistema · componentes", cabecera + izq + medio + der + abajo, alto=1110)


if __name__ == "__main__":
    escribir("S1_tokens", S1_tokens())
    escribir("S2_componentes", S2_componentes())
    render(["S1_tokens", "S2_componentes"], altos={"S1_tokens": 1130, "S2_componentes": 1110})
