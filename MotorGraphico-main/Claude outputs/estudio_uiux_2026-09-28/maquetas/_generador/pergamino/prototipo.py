"""Prototipo interactivo: cuaderno de pergamino con pasar hoja.

Genera un HTML autocontenido (CSS, fuentes e imágenes incrustadas):
  →  la hoja de arriba cae de arriba abajo (bisagra en el borde inferior)
  ←  la hoja anterior vuelve a subir y tapa la actual
  W/S o ↑/↓ elegir en la lista · TAB cerrar/abrir el cuaderno · clic en pestañas

python3 prototipo.py [--gif]
"""
import base64
import json
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(AQUI))
sys.path.insert(0, AQUI)

import muestra as M  # noqa: E402
import texturas as T  # noqa: E402
from lib import (ETER, FUENTE, SALIDA, ayuda, boton, bloque_titulo, e, fila, ic, insignia, px)  # noqa: E402

PX, PY, PW, PH = M.PX, M.PY, M.PW, M.PH
VX, VY, VW, VH = M.VX, M.VY, M.VW, M.VH
DESTINO = os.path.join(SALIDA, "pergamino")


# ------------------------------------------------------------ contenido
def cabeza(titulo, folio):
    return (f'<div class="fila-flex" style="position:absolute;left:40px;right:40px;top:26px;justify-content:center;gap:16px;color:var(--tinta-2)">'
            f'{M.filete(180)}{px(titulo, 2)}{M.filete(180)}</div>'
            f'<div class="folio" style="right:34px;bottom:14px">{px(folio, 2)}</div>')


SEP = '<div style="width:2px;background:repeating-linear-gradient(180deg,#6B5038 0 4px,transparent 4px 8px);opacity:.6;margin:8px 0"></div>'


def lista_detalle(filas, detalles, cabecera_lista="", ancho=360):
    return {"filas": filas, "detalles": detalles, "cab": cabecera_lista, "ancho": ancho}


def detalle_objeto(nombre, icono, efecto, icono_ef, tienes, recurso_txt, motivo, estilo_icono=""):
    return f'''<div class="detalle" style="padding:0 8px 0 32px;gap:14px;height:100%">
      <div class="fila-flex" style="gap:16px">{ic(icono, estilo=estilo_icono)}<h2>{nombre}</h2>{insignia("CONSUMIBLE")}</div>
      <div class="efecto">{ic(icono_ef, 1)}{efecto}</div>
      <dl class="datos"><dt>Tienes</dt><dd>{tienes}</dd><dt>{recurso_txt[0]}</dt><dd>{recurso_txt[1]}</dd></dl>
      <div class="acciones">{boton("USAR", "deshab")}
        <span class="motivo-boton">{px("!", 2, estilo="color:var(--aviso)")}{motivo}</span></div></div>'''


def paginas():
    P = []
    # 1 · Inventario
    P.append({"titulo": "INVENTARIO", "tipo": "lista", **lista_detalle(
        [fila("Poción", "consumible", px("×2", 2)), fila("Éter", "consumible", px("×1", 2), estilo_icono=ETER)],
        [detalle_objeto("Poción", "consumible", "Recupera 15 PV.", "pv", 2, ("Tus PV", "28 de 28"),
                        "Tus PV ya están al máximo. No se gasta nada."),
         detalle_objeto("Éter", "consumible", "Restaura 5 PM.", "pm", 1, ("Tus PM", "8 de 8"),
                        "Tus PM ya están al máximo. No se gasta nada.", estilo_icono=ETER)],
        f'<div class="fila-flex t3" style="padding:0 12px 0 36px">{px("OBJETO", 2)}<span style="margin-left:auto">{px("CANT.", 2)}</span></div>')})
    # 2 · Equipo
    ranuras = [("Cabeza", "ranura_cabeza"), ("Torso", "ranura_torso"), ("Piernas", "ranura_piernas"), ("Pies", "ranura_pies"),
               ("Mano principal", "ranura_mano_principal"), ("Mano izquierda", "ranura_mano_izquierda"), ("Accesorio", "ranura_accesorio")]
    filas = [fila(n, i, '<span class="t3 sec it">Vacío</span>', x_icono=1) for n, i in ranuras]
    dets = [f'''<div class="detalle" style="padding:0 8px 0 32px;gap:14px;height:100%">
      <div class="fila-flex" style="gap:16px">{ic(i)}<h2>{n}</h2></div>
      <div class="desc">No llevas nada en esta ranura.</div>
      <div class="acciones">{boton("EQUIPAR", "deshab")}<span class="motivo-boton">{px("!", 2, estilo="color:var(--aviso)")}No tienes objetos compatibles.</span></div></div>''' for n, i in ranuras]
    P.append({"titulo": "EQUIPO", "tipo": "lista", **lista_detalle(filas, dets,
             f'<div class="fila-flex t3" style="padding:0 12px 0 36px">{px("RANURA", 2)}</div>')})
    # 3 · Ficha (sin lista)
    def stat(icn, nombre, ab, total, det):
        return (f'<div class="fila-flex" style="height:36px">{ic(icn, 1)}<span style="width:120px">{nombre}</span>'
                f'<span class="t3" style="width:40px">{px(ab, 2)}</span>{px(str(total), 3)}'
                f'<span class="t3 sec" style="margin-left:12px;white-space:nowrap">{det}</span></div>')
    hist = [("Vínculo", "El banco del cementerio"), ("Miedo", "La pena sin testigos"), ("Defecto", "Envidia silenciosa"),
            ("Meta", "Aprender la canción de mi casa"), ("Ideal", "Crecer no es competir"),
            ("Personalidad", "Acompaña en silencio"), ("Virtud", "Comprende el sufrimiento ajeno")]
    ficha = f'''<div style="display:grid;grid-template-columns:280px 2px 340px 2px 1fr;gap:0 20px;height:100%">
      <div class="col" style="gap:10px">{bloque_titulo("IDENTIDAD")}
        <dl class="datos apilados"><dt>Raza</dt><dd>{M.RAZA}</dd><dt>Clase</dt><dd>{M.CLASE}</dd><dt>Trasfondo</dt><dd>Mes de la Agonía</dd><dt>Deidad</dt><dd>Sin deidad</dd></dl>
        {bloque_titulo("ESTADO")}<dl class="datos"><dt>Moral</dt><dd>Neutral (0)</dd><dt>Condiciones</dt><dd>Ninguna</dd></dl></div>
      {SEP}
      <div class="col" style="gap:8px">{bloque_titulo("ATRIBUTOS")}
        {stat("con", "Constitución", "CON", 4, "base 4")}{stat("des", "Destreza", "DES", 6, "4 + 2 raza")}
        {stat("int", "Inteligencia", "INT", 4, "base 4")}{stat("car", "Carisma", "CAR", 6, "4 + 2 raza")}
        {bloque_titulo("RASGO")}<div class="fila-flex b6">{ic("pasiva", 1)}Terreno Elevado</div>
        <div class="sec t2" style="line-height:1.4">Tus ataques ganan +1 y tus pruebas para trepar o saltar tienen ventaja.</div></div>
      {SEP}
      <div class="col" style="gap:10px">{bloque_titulo("HISTORIA PERSONAL")}
        <dl class="datos" style="row-gap:6px;font-size:17px">{"".join(f"<dt>{a}</dt><dd>{b}</dd>" for a, b in hist)}</dl></div></div>'''
    P.append({"titulo": "FICHA", "tipo": "fija", "html": ficha})
    # 4 · Acciones (estado vacío honesto)
    vacio = f'''<div class="vacio" style="height:100%;color:var(--tinta-2)">{ic("dote")}
      <span class="t" style="color:var(--tinta)">No tienes acciones disponibles fuera de combate.</span>
      <span class="d">Cuando puedas usar una habilidad explorando, aparecerá aquí con su efecto y su coste.</span></div>'''
    P.append({"titulo": "ACCIONES", "tipo": "fija", "html": vacio})
    # 5 · Misiones
    det_m = f'''<div class="detalle" style="padding:0 8px 0 32px;gap:12px;height:100%">
      <div class="fila-flex" style="gap:14px">{ic("mision_activa")}<h2>El primer día en Boundington</h2>{insignia("SEGUIDA", "info")}</div>
      {bloque_titulo("OBJETIVO ACTUAL")}
      <div class="efecto">{ic("objetivo", 1)}Presentarte a los tres comerciantes de la plaza <span class="t3" style="margin-left:6px">{px("0/3", 2)}</span></div>
      <div class="col sec t2" style="gap:4px;padding-left:24px"><span>· Tendero del mercado</span><span>· Joyero</span><span>· Sastre</span></div>
      <div class="meta t3 it">Los siguientes objetivos aparecerán cuando los descubras.</div></div>'''
    P.append({"titulo": "MISIONES", "tipo": "lista", **lista_detalle(
        [fila("El primer día en Boundington", "mision_activa", px("0/3", 2), x_icono=1)], [det_m],
        f'<div class="fila-flex t3" style="padding:0 12px 0 36px">{px("ACTIVAS", 2)}</div>')})
    # 6 · Códice
    det_anc = f'''<div class="detalle" style="padding:0 8px 0 32px;gap:12px;height:100%">
      <div class="fila-flex" style="gap:14px">{ic("personaje")}<h2>Anciana</h2>{insignia("PERSONAJE")}</div>
      <div class="desc">Hablaste con ella en el centro de Boundington.</div>
      <div class="desc it">«Antes esta plaza era un huerto.»</div></div>'''
    det_bnd = f'''<div class="detalle" style="padding:0 8px 0 32px;gap:12px;height:100%">
      <div class="fila-flex" style="gap:14px">{ic("lugar")}<h2>Boundington</h2>{insignia("ESTÁS AQUÍ", "exito")}</div>
      <div class="desc">Ciudad de Aegroum.</div>
      <dl class="datos"><dt>Población</dt><dd>~25.000</dd><dt>Gobierna</dt><dd>Regidora Gorlak</dd><dt>Rasgo</dt><dd>Muralla a medio caer</dd></dl></div>'''
    P.append({"titulo": "CÓDICE", "tipo": "lista", **lista_detalle(
        [fila("Anciana", "personaje", '<span class="t3 meta">Personaje</span>', x_icono=1),
         fila("Boundington", "lugar", '<span class="t3 meta">Lugar</span>', x_icono=1)], [det_anc, det_bnd],
        f'<div class="fila-flex t3" style="padding:0 12px 0 36px">{px("DESCUBIERTO", 2)}</div>')})
    verbos = [("Objeto", "Usar"), ("Ranura", "Equipar"), (None, None), (None, None), ("Misión", None), ("Entrada", None)]
    for i, p in enumerate(P):
        p["cabeza"] = cabeza(p["titulo"], f"{i + 1}/{len(P)}")
        p["elegir"], p["verbo"] = verbos[i]
    P[4]["ancho"] = 420
    return P


# ------------------------------------------------------------ incrustar
def datauri(ruta):
    ext = os.path.splitext(ruta)[1].lower()
    mime = {".png": "image/png", ".jpg": "image/jpeg", ".ttf": "font/ttf"}[ext]
    return f"data:{mime};base64," + base64.b64encode(open(ruta, "rb").read()).decode()


def incrustar_css(ruta_css):
    css = open(ruta_css, encoding="utf-8").read()
    base = os.path.dirname(ruta_css)

    def rep(m):
        ref = m.group(1).strip("'\"")
        if ref.startswith("data:") or ref.startswith("http"):
            return m.group(0)
        return 'url("' + datauri(os.path.normpath(os.path.join(base, ref))) + '")'
    return re.sub(r"url\(([^)]+)\)", rep, css)


def incrustar_html(html):
    def rep_url(m):
        ref = m.group(1)
        return f"url({datauri(os.path.join(FUENTE, ref))})"

    def rep_src(m):
        return f'src="{datauri(os.path.join(FUENTE, m.group(1)))}"'
    html = re.sub(r"url\((img/[^)]+)\)", rep_url, html)
    html = re.sub(r'src="(img/[^"]+)"', rep_src, html)
    return html


def fondo_jpg():
    from PIL import Image
    ruta = os.path.join(FUENTE, "img", "pergamino", "_mundo.jpg")
    Image.open(os.path.join(FUENTE, "img", "fondos", "fondo_ciudad_anciana.png")).convert("RGB").save(ruta, quality=86)
    return ruta


# ------------------------------------------------------------ documento
def construir():
    P = paginas()
    marco = M.cuaderno_marco(0, con_pestanas=False)
    # pie contextual: los textos de W/S y ENTER cambian con la hoja
    marco = marco.replace('<span>Objeto</span></span>', '<span id="elegir">Objeto</span></span>', 1)
    marco = marco.replace('<span>Usar</span></span>', '<span id="verbo">Usar</span></span>', 1)
    lomo = M.lomo()
    hoja_url = M.tex(f"hoja_{PW}x{PH}_s40_r1_fNone_m2_filete", T.hoja, PW, PH, 40, 1, filete=6, manchas=2)
    dorso_url = M.tex(f"dorso_{PW}x{PH}", T.hoja, PW, PH, 77, 1, manchas=5, oscuro=0.22)
    # texturas de pestaña activa/inactiva para cada sección
    pest = []
    x = VX + 40
    for i, (et, icono) in enumerate(M.SECCIONES):
        w = 44 + len(et) * 12 + 16
        w += w % 2
        ya, yi = VY + 58, VY + 62
        ha, hi = PY - ya + 2, PY - yi - 2
        ha += ha % 2
        hi += hi % 2
        ua = M.tex(f"pestana_{w}x{ha}_a_s{30 + i}", T.pestana, w, ha, True, 30 + i)
        ui = M.tex(f"pestana_{w}x{hi}_i_s{30 + i}", T.pestana, w, hi, False, 30 + i)
        pest.append(f'<div class="pest" data-i="{i}" style="left:{x}px;width:{w}px;--ya:{ya}px;--yi:{yi}px;--ha:{ha}px;--hi:{hi}px;'
                    f'--ta:url({ua});--ti:url({ui})">{ic(icono, 1)}{px(et, 2)}</div>')
        x += w + 6
    hud = M.hud_personaje() + M.hud_objetivo() + M.hud_minimapa() + M.hud_teclas()
    datos = json.dumps(P, ensure_ascii=False)
    css = "\n".join(incrustar_css(os.path.join(FUENTE, "sistema", f)) for f in
                    ("tokens.css", "componentes.css", "tema_pergamino.css"))
    mundo = datauri(fondo_jpg())
    doc = f"""<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Pasar hoja · prototipo</title>
<style>
{css}
html, body {{ margin: 0; background: #120c07; overflow: hidden; }}
#marco {{ position: absolute; left: 0; top: 0; transform-origin: 0 0; }}
.pest {{ position: absolute; top: var(--yi); height: var(--hi); display: flex; align-items: center; justify-content: center; gap: 8px;
        background-image: var(--ti); background-size: 100% 100%; image-rendering: pixelated; color: var(--tinta-2); cursor: pointer;
        filter: drop-shadow(0 3px 0 rgba(26,16,8,.45)); }}
.pest.activa {{ top: var(--ya); height: var(--ha); background-image: var(--ta); color: var(--tinta); filter: none; z-index: 3; }}
#escena {{ position: absolute; left: {PX}px; top: {PY}px; width: {PW}px; height: {PH}px; perspective: 2800px; perspective-origin: 50% 20%;
          clip-path: inset(-600px -600px -{(VY + VH - 4) - (PY + PH)}px -600px); z-index: 2; }}
.hoja-pag {{ position: absolute; inset: 0; background-size: 100% 100%; image-rendering: pixelated; color: var(--tinta); }}
#hoja {{ position: absolute; inset: 0; transform-origin: 50% 100%; transform-style: preserve-3d; will-change: transform; }}
#hoja .cara {{ position: absolute; inset: 0; backface-visibility: hidden; -webkit-backface-visibility: hidden; background-size: 100% 100%; image-rendering: pixelated; }}
#hoja .dorso {{ transform: rotateX(180deg); }}
#sombra {{ position: absolute; inset: 0; pointer-events: none; opacity: 0;
          background: linear-gradient(to top, rgba(38,22,8,.62), rgba(38,22,8,.18) 45%, rgba(38,22,8,0) 80%); }}
.cont {{ position: absolute; inset: 62px 40px 34px 40px; }}
.cont .ld {{ display: grid; gap: 0 18px; height: 100%; }}
.cont .lista .fila {{ cursor: pointer; }}
#lomo-capa {{ position: absolute; inset: 0; z-index: 4; pointer-events: none; }}
#cuaderno {{ transition: opacity .16s; }}
#cuaderno.cerrado {{ opacity: 0; pointer-events: none; }}
#hud {{ transition: opacity .16s; }}
#hud.oculto {{ opacity: 0; }}
#controles {{ position: fixed; left: 0; right: 0; bottom: 0; display: flex; flex-wrap: wrap; gap: 8px 18px; align-items: center; justify-content: center;
             padding: 8px 12px; background: #1d140c; color: #E9D6A8; font: 14px/1.4 Lora, Georgia, serif; border-top: 2px solid #4A3421; }}
#controles label {{ display: inline-flex; gap: 6px; align-items: center; cursor: pointer; }}
#controles .t {{ color: #C9B083; }}
@media (prefers-reduced-motion: reduce) {{ #cuaderno, #hud {{ transition: none; }} }}
</style>
</head>
<body>
<div id="marco">
<div class="pantalla pergamino" id="pantalla">
  <div class="mundo" style="background-image:url({mundo})"></div>
  <div id="hud" class="oculto">{hud}</div>
  <div id="cuaderno">
    <div class="velo" style="background:rgba(24,14,6,.62)"></div>
    {marco}
    {''.join(pest)}
    <div id="escena">
      <div id="debajo" class="hoja-pag" style="background-image:url({hoja_url})"></div>
      <div id="sombra"></div>
      <div id="hoja"><div class="cara frente" style="background-image:url({hoja_url})"></div><div class="cara dorso" style="background-image:url({dorso_url})"></div></div>
    </div>
    <div id="lomo-capa">{lomo}</div>
  </div>
</div>
</div>
<div id="controles">
  <span class="t">←/→ pasar hoja · W/S elegir · TAB cerrar o abrir · clic en las pestañas</span>
  <label><input type="checkbox" id="lento"> Cámara lenta</label>
  <label><input type="checkbox" id="reducido"> Animaciones reducidas</label>
  <label><input type="checkbox" id="sonido"> Sonido de papel</label>
</div>
<script>
const PAGINAS = {datos};
const DUR = 360;                       // ms de una vuelta de hoja
let actual = 0, foco = PAGINAS.map(() => 0), anim = null, abierto = true;
const $ = s => document.querySelector(s);
const hoja = $('#hoja'), frente = hoja.querySelector('.frente'), dorso = hoja.querySelector('.dorso');
const debajo = $('#debajo'), sombra = $('#sombra');

function contenido(i) {{
  const p = PAGINAS[i];
  let cuerpo;
  if (p.tipo === 'lista') {{
    const filas = p.filas.map((f, k) => f.replace('class="fila ', 'data-k="' + k + '" class="fila ' + (k === foco[i] ? 'foco ' : ''))).join('');
    cuerpo = '<div class="ld" style="grid-template-columns:' + p.ancho + 'px 2px 1fr">'
      + '<div class="col" style="gap:10px">' + p.cab + '<div class="lista">' + filas + '</div></div>'
      + '<div style="width:2px;background:repeating-linear-gradient(180deg,#6B5038 0 4px,transparent 4px 8px);opacity:.6;margin:8px 0"></div>'
      + '<div class="det">' + p.detalles[foco[i]] + '</div></div>';
  }} else {{
    cuerpo = p.html;
  }}
  return p.cabeza + '<div class="cont">' + cuerpo + '</div>';
}}
function pestanas(i) {{
  document.querySelectorAll('.pest').forEach(t => t.classList.toggle('activa', +t.dataset.i === i));
}}
function pie(i) {{
  const p = PAGINAS[i], el = document.getElementById('elegir'), vb = document.getElementById('verbo');
  el.parentElement.style.display = p.elegir ? '' : 'none'; if (p.elegir) el.textContent = p.elegir;
  vb.parentElement.style.display = p.verbo ? '' : 'none'; if (p.verbo) vb.textContent = p.verbo;
}}
function reposo(i) {{
  actual = i; pestanas(i); pie(i);
  frente.innerHTML = contenido(i); debajo.innerHTML = '';
  hoja.style.transform = 'rotateX(0deg)'; frente.style.filter = ''; dorso.style.filter = '';
  sombra.style.opacity = 0; hoja.style.opacity = 1; debajo.style.opacity = 1;
}}
// Estado visual de una vuelta como función pura del ángulo (0 = hoja plana, 180 = caída)
function pinta(ang) {{
  const r = ang * Math.PI / 180, s = Math.sin(r);
  hoja.style.transform = 'rotateX(' + (-ang) + 'deg)';
  frente.style.filter = 'brightness(' + (1 - 0.30 * s).toFixed(3) + ')';
  dorso.style.filter = 'brightness(' + (0.80 - 0.22 * s).toFixed(3) + ')';
  sombra.style.opacity = (0.9 * s * (ang <= 90 ? 1 : (180 - ang) / 90)).toFixed(3);
}}
const facilIn = t => 0.25 * t + 0.75 * t * t;                     // cae: arranca y acelera (gravedad)
const facilOut = t => 1 + 1.9 * Math.pow(t - 1, 3) + 0.9 * Math.pow(t - 1, 2);  // sube y se asienta (rebote de ~5°)
function prepara(desde, hacia) {{
  pestanas(hacia); pie(hacia);
  if (hacia > desde) {{          // la hoja actual cae; debajo espera la siguiente
    frente.innerHTML = contenido(desde); debajo.innerHTML = contenido(hacia);
  }} else {{                     // la anterior vuelve a subir y tapa la actual
    frente.innerHTML = contenido(hacia); debajo.innerHTML = contenido(desde);
  }}
}}
function fija(desde, hacia, t) {{   // para capturas: estado de la vuelta en el instante t (0..1)
  prepara(desde, hacia);
  const ang = hacia > desde ? 180 * facilIn(t) : 180 * (1 - facilOut(t));
  pinta(ang);
}}
function papel() {{
  if (!$('#sonido').checked) return;
  const ctx = papel.ctx || (papel.ctx = new (window.AudioContext || window.webkitAudioContext)());
  const n = ctx.sampleRate * 0.22, buf = ctx.createBuffer(1, n, ctx.sampleRate), d = buf.getChannelData(0);
  for (let i = 0; i < n; i++) d[i] = (Math.random() * 2 - 1) * Math.pow(1 - i / n, 2);
  const src = ctx.createBufferSource(); src.buffer = buf;
  const f = ctx.createBiquadFilter(); f.type = 'bandpass'; f.frequency.value = 2400; f.Q.value = 0.7;
  const g = ctx.createGain(); g.gain.value = 0.25;
  src.connect(f).connect(g).connect(ctx.destination); src.start();
}}
const destino = () => anim ? anim.hacia : actual;   // página a la que se va (o en la que se está)
function ir(hacia) {{
  if (hacia < 0 || hacia >= PAGINAS.length || hacia === destino()) return;
  if (anim) anim.fin();                         // una pulsación nueva completa la vuelta en curso
  const desde = actual;
  if (hacia === desde) return;
  papel();
  if ($('#reducido').checked) {{                // fundido corto en lugar de vuelta
    prepara(desde, hacia); pinta(0);
    const t0 = performance.now();
    const paso = now => {{
      const t = Math.min(1, (now - t0) / 160);
      if (hacia > desde) hoja.style.opacity = 1 - t; else {{ hoja.style.opacity = t; }}
      if (t < 1) anim.raf = requestAnimationFrame(paso); else anim.fin();
    }};
    anim = {{ hacia, fin: () => {{ cancelAnimationFrame(anim.raf); anim = null; reposo(hacia); }} }};
    anim.raf = requestAnimationFrame(paso);
    return;
  }}
  prepara(desde, hacia);
  const dur = DUR * ($('#lento').checked ? 5 : 1), t0 = performance.now();
  const paso = now => {{
    const t = Math.min(1, (now - t0) / dur);
    pinta(hacia > desde ? 180 * facilIn(t) : 180 * (1 - facilOut(t)));
    if (t < 1) anim.raf = requestAnimationFrame(paso); else anim.fin();
  }};
  anim = {{ hacia, fin: () => {{ cancelAnimationFrame(anim.raf); anim = null; reposo(hacia); }} }};
  anim.raf = requestAnimationFrame(paso);
}}
function mueveFoco(d) {{
  const p = PAGINAS[actual];
  if (p.tipo !== 'lista' || anim) return;
  foco[actual] = (foco[actual] + d + p.filas.length) % p.filas.length;
  frente.innerHTML = contenido(actual);
}}
document.addEventListener('keydown', ev => {{
  const k = ev.key;
  if (k === 'Tab') {{ ev.preventDefault(); abierto = !abierto; $('#cuaderno').classList.toggle('cerrado', !abierto); $('#hud').classList.toggle('oculto', abierto); return; }}
  if (!abierto) return;
  if (k === 'ArrowRight' || k === 'd' || k === 'D') {{ ev.preventDefault(); ir(destino() + 1); }}
  else if (k === 'ArrowLeft' || k === 'a' || k === 'A') {{ ev.preventDefault(); ir(destino() - 1); }}
  else if (k === 'ArrowDown' || k === 's' || k === 'S') {{ ev.preventDefault(); mueveFoco(1); }}
  else if (k === 'ArrowUp' || k === 'w' || k === 'W') {{ ev.preventDefault(); mueveFoco(-1); }}
}});
document.querySelectorAll('.pest').forEach(t => t.addEventListener('click', () => ir(+t.dataset.i)));
frente.addEventListener('click', ev => {{
  const f = ev.target.closest('.fila[data-k]');
  if (!f || anim) return;
  foco[actual] = +f.dataset.k; frente.innerHTML = contenido(actual);
}});
function ajusta() {{
  const barra = document.getElementById('controles').offsetHeight;
  const s = Math.min(window.innerWidth / 1280, (window.innerHeight - barra) / 720);
  const m = document.getElementById('marco');
  m.style.transform = 'scale(' + s + ')';
  m.style.left = Math.max(0, (window.innerWidth - 1280 * s) / 2) + 'px';
  m.style.top = Math.max(0, (window.innerHeight - barra - 720 * s) / 2) + 'px';
}}
window.addEventListener('resize', ajusta);
ajusta(); reposo(0);
window.__fija = fija; window.__reposo = reposo;
</script>
</body>
</html>
"""
    return incrustar_html(doc)


def capturar_gif(ruta_html):
    """Fotogramas deterministas de ida (1→2) y vuelta (2→1) y GIF/MP4 con ffmpeg."""
    import shutil
    import subprocess
    import tempfile
    from playwright.sync_api import sync_playwright
    tmp = tempfile.mkdtemp(prefix="pasar_hoja_")
    clip = {"x": VX - 16, "y": VY - 16, "width": VW + 32, "height": VH + 24}
    fotos = []
    with sync_playwright() as p:
        nav = p.chromium.launch()
        pag = nav.new_page(viewport={"width": 1280, "height": 820}, device_scale_factor=1)
        pag.goto("file://" + ruta_html)
        pag.evaluate("document.fonts.ready")
        pag.add_style_tag(content="#controles{display:none!important} #marco{transform:none!important;left:0!important;top:0!important}")
        pag.wait_for_timeout(200)
        n = 0

        def foto():
            nonlocal n
            r = os.path.join(tmp, f"f{n:03d}.png")
            pag.screenshot(path=r, clip=clip)
            fotos.append(r)
            n += 1
        pag.evaluate("__reposo(0)")
        for _ in range(12):
            foto()
        for k in range(14):
            pag.evaluate(f"__fija(0, 1, {k / 13})")
            foto()
        pag.evaluate("__reposo(1)")
        for _ in range(18):
            foto()
        for k in range(14):
            pag.evaluate(f"__fija(1, 0, {k / 13})")
            foto()
        pag.evaluate("__reposo(0)")
        for _ in range(12):
            foto()
        nav.close()
    os.makedirs(DESTINO, exist_ok=True)
    gif = os.path.join(DESTINO, "pasar_hoja.gif")
    mp4 = os.path.join(DESTINO, "pasar_hoja.mp4")
    ent = os.path.join(tmp, "f%03d.png")
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "30", "-i", ent,
                    "-vf", "scale=860:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=128[p];[b][p]paletteuse=dither=bayer:bayer_scale=4",
                    gif], check=True)
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "30", "-i", ent,
                    "-vf", "pad=ceil(iw/2)*2:ceil(ih/2)*2", "-c:v", "libx264", "-pix_fmt", "yuv420p", "-crf", "20", mp4], check=True)
    # tira de fotogramas para el documento
    from PIL import Image
    sel = [fotos[12 + k] for k in (0, 3, 6, 9, 12)] + [fotos[12 + 14 + 18 + k] for k in (0, 4, 8, 12)]
    ims = [Image.open(f).convert("RGB") for f in sel]
    w, h = 380, int(ims[0].height * 380 / ims[0].width)
    tira = Image.new("RGB", (w * 5 + 16, h * 2 + 8), (18, 12, 7))
    for i, im in enumerate(ims[:5]):
        tira.paste(im.resize((w, h), Image.LANCZOS), (i * (w + 4), 0))
    for i, im in enumerate(ims[5:]):
        tira.paste(im.resize((w, h), Image.LANCZOS), (i * (w + 4), h + 8))
    tira.save(os.path.join(DESTINO, "pasar_hoja_fotogramas.png"))
    shutil.rmtree(tmp)
    print("gif", os.path.getsize(gif) // 1024, "KB · mp4", os.path.getsize(mp4) // 1024, "KB")


if __name__ == "__main__":
    os.makedirs(DESTINO, exist_ok=True)
    ruta = os.path.join(DESTINO, "prototipo_pasar_hoja.html")
    open(ruta, "w", encoding="utf-8").write(construir())
    print("html", os.path.getsize(ruta) // 1024, "KB")
    if "--gif" in sys.argv:
        capturar_gif(ruta)
