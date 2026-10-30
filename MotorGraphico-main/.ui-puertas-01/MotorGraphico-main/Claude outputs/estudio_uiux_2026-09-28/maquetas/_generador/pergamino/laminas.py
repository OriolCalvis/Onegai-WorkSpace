"""Láminas de la variante pergamino: comparativas oscuro → pergamino y S3 (sistema)."""
import html
import os
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(AQUI))
sys.path.insert(0, AQUI)

import muestra as M  # noqa: E402
import texturas as T  # noqa: E402
from lib import (FUENTE, SALIDA, ayuda, boton, escribir, fila, ic, insignia, moral, px, recurso, render,  # noqa: E402
                 tecla)

DESTINO = os.path.join(SALIDA, "pergamino")
PARES = [
    ("M03a_hud_exploracion", "P03a_hud_pergamino", "Exploración · HUD ligero",
     ["Retales de pergamino con canto rasgado: el mundo sigue visible y el HUD se lee con tinta.",
      "El minimapa se dibuja a tinta sobre el propio retal, con la misma leyenda de formas.",
      "El foco en el mundo (rombo de la baldosa) pasa a carmín, el color de foco del tema."]),
    ("M03b_dialogo", "P03b_dialogo_pergamino", "Diálogo",
     ["Hoja de pergamino con rótulo de cuero para el hablante.",
      "Mismo texto, tamaño y pie que el modelo oscuro: solo cambia el material.",
      "Durante el diálogo el HUD se queda en un retal con PV y PM."]),
    ("M04a_tab_inventario", "P04a_cuaderno_inventario", "Menú con pestañas · cuaderno",
     ["Tapas y lomo de cuero; cada pestaña es una hoja del cuaderno con su número (1/6).",
      "Al pasar de pestaña, la hoja cae de arriba abajo; al volver, sube (ver prototipo y GIF).",
      "El foco es una cinta de marcapáginas carmín, sin segundo cursor."]),
    ("M05a_pausa_guardado", "P05a_papiro_pausa", "Menú de una página · papiro",
     ["Papiro con rodillos de madera y remates de latón: sin pestañas, sin animación.",
      "Título pixel con filete de tinta; separador ornamental antes de las salidas.",
      "«Partida guardada» con sello de lacre, junto a la acción."]),
]


def comparativa(a, b, titulo, notas):
    li = "".join(f"<li>{html.escape(n)}</li>" for n in notas)
    return f"""<!doctype html><html lang="es"><head><meta charset="utf-8">
<link rel="stylesheet" href="file://{FUENTE}/sistema/tokens.css">
<link rel="stylesheet" href="file://{FUENTE}/sistema/componentes.css">
<style>body{{width:1328px;background:#151820}} .l{{padding:20px 24px 22px;display:flex;flex-direction:column;gap:14px}}
.g{{display:grid;grid-template-columns:640px 640px;gap:0 16px}} figure{{display:flex;flex-direction:column;gap:8px}}
figure img{{width:640px;height:360px;display:block;box-shadow:0 0 0 2px #0B0D12}}
figcaption{{display:flex;align-items:center;gap:10px;font-size:15px;color:var(--texto-3)}}
ul{{margin:2px 0 0 20px;font-size:17px;line-height:1.5;color:var(--texto-2)}}</style></head><body><div class="l">
<div class="fila-flex" style="gap:16px">{px(b.split("_")[0], 3)}<span style="font-size:24px;font-weight:600">{html.escape(titulo)}</span></div>
<div class="g"><figure><figcaption><span class="insignia">{px("OSCURO", 2)}</span>{a}.png</figcaption><img src="file://{SALIDA}/{a}.png"></figure>
<figure><figcaption><span class="insignia i-exito">{px("PERGAMINO", 2)}</span>{b}.png</figcaption><img src="file://{DESTINO}/{b}.png"></figure></div>
<ul>{li}</ul></div></body></html>"""


def muestra_color(nombre, hexa, ratio, fondo="#E2CD9C"):
    return (f'<div class="col" style="gap:3px;width:118px"><div style="height:34px;background:{hexa};box-shadow:0 0 0 2px #4A3421"></div>'
            f'<span class="meta b6">{nombre}</span><span class="meta t2" style="line-height:1.2">{hexa}</span>'
            f'<span class="meta t3" style="line-height:1.2">{ratio}</span></div>')


def S3():
    colores = [("tinta", "#2B1D12", "10,5:1"), ("tinta-2", "#553E2B", "6,4:1"), ("tinta-3", "#6B5038", "4,8:1"),
               ("carmín", "#9A2A1C", "4,9:1"), ("éxito", "#2E6337", "4,6:1"), ("aviso", "#7A4409", "5,1:1"),
               ("error", "#8E1B12", "5,8:1"), ("info", "#29507A", "5,3:1")]
    papeles = [("papel", "#E2CD9C", "base del texto"), ("sombra", "#D6BC85", "canto y manchas"),
               ("cuero", "#4E321D", "9,3:1 texto claro"), ("contorno", "#4A3421", "5,8:1 sobre arena")]
    fila_c = "".join(muestra_color(*c) for c in colores)
    fila_p = "".join(muestra_color(*c) for c in papeles)
    materiales = (
        M.hoja(40, 640, 300, 190, '<div style="padding:22px 26px" class="col"><span class="b6">Hoja</span><span class="sec t2">Diálogos, fichas y tienda. Canto rasgado y envejecido.</span></div>', semilla=51, rasgado=2, filete=6)
        + M.papiro(370, 604, 250, 262, '<div style="position:absolute;inset:34px 30px" class="col"><span class="b6">Papiro</span><span class="sec t2">Menús de una sola página. Sin animación.</span></div>')
        + M.cuero(650, 640, 300, 190, '<div style="padding:20px 24px" class="col"><span class="b6">Cuero</span><span class="sec" style="color:var(--claro-cuero-2)">Tapas y lomo del cuaderno: menús con pestañas.</span></div>', esquinas=True, remaches=False)
        + M.hoja(980, 640, 260, 130, '<div style="padding:18px 22px" class="col"><span class="b6">Retal</span><span class="sec t2">HUD ligero de exploración.</span></div>', semilla=52, rasgado=3, clase="retal")
    )
    comps = M.hoja(640, 92, 600, 450, f'''<div class="col" style="padding:26px 30px;gap:12px">
      {px("COMPONENTES", 2, estilo="color:var(--tinta-3)")}
      <div class="lista" style="width:360px">{fila("Normal", "consumible", px("30", 2))}{fila("Con foco: cinta carmín", "consumible", px("30", 2), "foco")}{fila("Deshabilitada", None, "", "deshab", motivo="Con su motivo")}</div>
      <div class="fila-flex" style="gap:16px;flex-wrap:wrap">{boton("PRINCIPAL", "primario")}{boton("SECUNDARIO")}{boton("USAR", "deshab")}</div>
      <div class="fila-flex" style="gap:14px;flex-wrap:wrap">{ayuda(["←", "→"], "Pasar hoja")}{ayuda("ENTER", "Aceptar")}{insignia("T1")}{insignia("TU NEGOCIO", "exito")}{insignia("SIN FONDOS", "aviso")}</div>
      <div class="fila-flex" style="gap:22px">{recurso("pv", "PV", 19, 28, 240, "pv")}{moral(-30, "CANALLA", 240)}</div></div>''',
                   semilla=53, rasgado=2, filete=6)
    cuerpo = f'''
<div class="capa" style="left:40px;top:30px;display:flex;align-items:baseline;gap:18px;color:var(--claro-cuero)">
  {px("TEMA PERGAMINO", 3)}<span class="sec it" style="color:#D9C49A">Tinta sobre pergamino, cuero para los cuadernos y carmín para el foco.</span></div>
<div class="capa col" style="left:40px;top:92px;width:560px;gap:14px">
  {M.hoja(0, 0, 560, 500, f"""<div class="col" style="padding:24px 28px;gap:14px">
    {px("TINTAS · CONTRASTE SOBRE PAPEL BASE", 2, estilo="color:var(--tinta-3)")}
    <div class="fila-flex" style="gap:10px;flex-wrap:wrap;align-items:flex-start">{fila_c}</div>
    {px("MATERIALES", 2, estilo="color:var(--tinta-3)")}
    <div class="fila-flex" style="gap:10px;flex-wrap:wrap;align-items:flex-start">{fila_p}</div></div>""", semilla=54, rasgado=2, filete=6, estilo="position:relative")}
</div>
{comps}{materiales}'''
    return M.pagina("Tema pergamino · sistema", cuerpo).replace('<div class="pantalla pergamino">',
                                                               '<div class="pantalla pergamino" style="height:890px;background:#2A1A0E">')


if __name__ == "__main__":
    escribir("S3_tema_pergamino", S3())
    render(["S3_tema_pergamino"], altos={"S3_tema_pergamino": 890})
    os.makedirs(DESTINO, exist_ok=True)
    os.replace(os.path.join(SALIDA, "S3_tema_pergamino.png"), os.path.join(DESTINO, "S3_tema_pergamino.png"))
    from playwright.sync_api import sync_playwright
    os.makedirs(DESTINO, exist_ok=True)
    import tempfile
    tmp = tempfile.mkdtemp()
    with sync_playwright() as p:
        nav = p.chromium.launch()
        pag = nav.new_page(viewport={"width": 1328, "height": 600})
        for a, b, t, n in PARES:
            r = os.path.join(tmp, b + ".html")
            open(r, "w", encoding="utf-8").write(comparativa(a, b, t, n))
            pag.goto("file://" + r)
            pag.evaluate("document.fonts.ready")
            pag.wait_for_timeout(100)
            pag.screenshot(path=os.path.join(DESTINO, f"oscuro_vs_{b}.png"), full_page=True)
            print("ok", b)
        nav.close()
