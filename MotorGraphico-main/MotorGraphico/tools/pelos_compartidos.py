#!/usr/bin/env python3
"""H-PEL-2 (Oriol, 29/10: «los pelos de serie de cada raza estan guapisimos y los otros a escoger son una basura»):
los 12 mejores cabellos propios de raza (G3-pelo.md, grupo A1) como peinados nuevos para todas las razas de cabeza
compatible (tallas medio, bajo y pequeno: la cabeza mide 14 px en las tres) y en todos los colores.

Para cada cabello: se toma la pieza `propio/<raza de origen>` del atlas de pelo de su talla; se limpia lo que no es
pelo (orejas, cuernos, piel que se cuela: reglas por color en LIMPIEZA, revisadas a mano sobre la hoja de
contactos); se cuantiza por luminancia a los 5 tonos de `pelo_castano` (el motor lo tine a cualquier color); y se
mueve fotograma a fotograma de la cabeza de su raza a la cabeza generica de cada talla destino (piel_clara de
`cuerpos`, a la que se ajustan los peinados genericos). Cada talla destino recibe la fila `<estilo>/pelo_castano`
en su atlas de pelo y `personajes.json` la apunta con `tallas` y `nombre`. Los otros 5 atlas solo crecen con filas
transparentes (validar_sprites exige que todos midan lo mismo; no la mira vacia si `tallas` no la incluye).

Uso (desde MotorGraphico/; solo Pillow, numpy y scipy; NO importa gen_personajes.py):
  python3 tools/pelos_compartidos.py                  # escribe atlas y personajes.json (idempotente)
  python3 tools/pelos_compartidos.py --comprobar      # no escribe; sale 1 si algo no coincide
  python3 tools/pelos_compartidos.py --contacto OUT   # hoja: los 12 sobre 6 razas x 3 colores (frente y perfil)
  python3 tools/pelos_compartidos.py --limpieza OUT   # hoja: cada origen antes/despues de limpiar y cuantizar
`--tex DIR` cambia la carpeta de texturas (copias de prueba).
"""
import argparse, json, sys
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage

Image.MAX_IMAGE_PIXELS = None
L = 64
KF = 40
BASE = "pelo_castano"
TALLAS = ("medio", "bajo", "pequeno")
TEX_DEFECTO = Path(__file__).resolve().parent.parent / "assets" / "textures"
MIN_COMPONENTE = 3  # px: lo que queda suelto tras limpiar se va (como PL-ROPA-01)


def _hsv(px):
    r, g, b = (px[..., i].astype(np.float32) / 255 for i in range(3))
    mx, mn = np.maximum(np.maximum(r, g), b), np.minimum(np.minimum(r, g), b)
    d = mx - mn
    s = np.where(mx > 0, d / np.maximum(mx, 1e-6), 0)
    h = np.zeros_like(mx)
    dd = np.maximum(d, 1e-6)
    h = np.where(mx == r, ((g - b) / dd) % 6, np.where(mx == g, (b - r) / dd + 2, (r - g) / dd + 4)) * 60
    return np.where(d > 0, h, 0), s, mx


# --- los 12 cabellos: (estilo nuevo, raza de origen, nombre visible, regla que marca lo que NO es pelo) ---------
# Las reglas son por color (h en grados, s y v en 0-1): la pieza de PixelLab lleva la oreja, el cuerno o la piel de
# la frente con sus propios colores. Cada una esta revisada sobre `--limpieza`; se documenta al lado.
def _ninguna(h, s, v):
    return np.zeros(h.shape, bool)


LIMPIEZA = {
    # pelo blanco-gris (s<9 %); lo marron (h 13-21, s>55) es piel de la frente/oreja.
    "archivistas_de_numandum": lambda h, s, v: (s > 0.3) & (h < 40),
    # pelo rojo saturado (s>=55 %, v<0.9 salvo reflejos); la piel de la frente es mas clara (v>=0.9) o grisacea.
    "enanos_forjadores": lambda h, s, v: (v > 0.90) | (s < 0.30),
    # pelo gris neutro; la oreja es verde oliva (h 50-130, s>=10 %).
    "sagas_de_ashye": lambda h, s, v: (h > 50) & (h < 130) & (v > 0.09),
    # pelo gris oscuro azulado (s<=13 %); la piel de la cara y sus contornos son rojizos/pardos saturados.
    "diplomaticos_de_bastrea": lambda h, s, v: (((h < 30) | (h > 330)) & (s > 0.30) & (v > 0.03)) | (v > 0.95),
    # pelo negro/gris neutro; la oreja es verde (h 50-160, s>=14 %, no casi negro) y hay dos px amarillos.
    "trasgos_de_gliaddokx": lambda h, s, v: (((h > 50) & (h < 160) & (s > 0.14) & (v > 0.05)) |
                                             ((h > 40) & (h < 60) & (v > 0.9))),
    "nobles_de_ascaria": _ninguna,  # rubio limpio; los adornos van en `rasgos`, no en el pelo
    # pelo castano oscuro (v<=0.50); la piel de la oreja es mas clara (v>=0.58) y los azules son el ojo.
    "medianos_urbanos": lambda h, s, v: (v > 0.55) | ((h > 150) & (h < 250)),
    # pelo castano (v<=0.67); un par de px de piel clara de la frente.
    "alquimistas_de_aegroum": lambda h, s, v: v > 0.755,
    # rubio oscuro: los abalorios blancos (v>=0.99) y la piel rojiza oscura (h<=17, s 50-60) del cuello.
    "navegantes_de_ranmont": lambda h, s, v: (v > 0.97) | ((h < 18) & (s > 0.48) & (v > 0.40) & (v < 0.70)),
    # pelo azul negruzco (v<=0.30); lo azul claro (v>=0.40) es piel del ojo tapado y su brillo.
    "revenants": lambda h, s, v: v > 0.38,
    # pelo gris azulado oscuro; la piel (h<50, v>=0.33) y su contorno pardo saturado, mas las camisas blancas.
    "mercenarios_de_ascaria": lambda h, s, v: (((h < 50) & (v > 0.33)) | ((h < 50) & (s > 0.6) & (v > 0.09)) | (v > 0.95)),
    # pelo blanco azulado (s<20 %); la piel y la oreja son rosas/rojizas (s>=13 %) y hay un px de ojo azul.
    "gnomos_de_la_isla": lambda h, s, v: ((((h < 20) | (h > 320)) & (s > 0.13)) | ((h > 190) & (h < 215) & (s > 0.5))),
}

PELOS = [  # (estilo, origen, nombre visible [esborrany: textos/H-PEL-2.md])
    ("mono_mechones", "archivistas_de_numandum", "Moño con mechones"),
    ("pelo_voluminoso", "enanos_forjadores", "Pelo voluminoso"),
    ("melena_salvaje", "sagas_de_ashye", "Melena salvaje"),
    ("peinado_atras", "diplomaticos_de_bastrea", "Peinado hacia atrás"),
    ("pinchos_revueltos", "trasgos_de_gliaddokx", "Pinchos revueltos"),
    ("ondas_nobles", "nobles_de_ascaria", "Ondas largas"),
    ("rizos_cortos", "medianos_urbanos", "Rizos cortos"),
    ("despeinado", "alquimistas_de_aegroum", "Despeinado"),
    ("corto_y_cola_baja", "navegantes_de_ranmont", "Corto con cola baja"),
    ("mechon_mojado", "revenants", "Mechón mojado"),
    ("melena_suelta", "mercenarios_de_ascaria", "Melena suelta"),
    ("rizos_con_mono", "gnomos_de_la_isla", "Rizos con moño"),
]
ESTILOS = [p[0] for p in PELOS]


def leer_man(tex):
    ruta = tex / "personajes.json"
    texto = ruta.read_text(encoding="utf-8")
    return json.loads(texto), texto


def formato(man):
    return json.dumps(man, ensure_ascii=False, indent=1) + "\n"


def frames(atlas, fila):
    """(40, 64, 64, 4) de una fila (1-based) del atlas."""
    y0 = (fila - 1) * L
    return np.stack([atlas[y0:y0 + L, k * L:(k + 1) * L] for k in range(KF)]).copy()


def cargar(tex, ruta):
    return np.asarray(Image.open(tex / ruta).convert("RGBA"))


def cabeza(fr):
    """(cx, ytop) de un frame de cuerpo: centro de las 7 filas de arriba (el craneo) y la fila mas alta."""
    ys, xs = np.nonzero(fr[..., 3] > 0)
    if len(ys) == 0:
        return None
    y0 = int(ys.min())
    m = ys < y0 + 7
    return (xs[m].min() + xs[m].max()) / 2.0, y0


def desplazar(fr, dx, dy):
    out = np.zeros_like(fr)
    ys0, ys1 = max(0, dy), min(L, L + dy)
    xs0, xs1 = max(0, dx), min(L, L + dx)
    if ys1 > ys0 and xs1 > xs0:
        out[ys0:ys1, xs0:xs1] = fr[ys0 - dy:ys1 - dy, xs0 - dx:xs1 - dx]
    return out


# (px bajo la coronilla desde los que no es pelo, frames 0-based a los que se aplica o None = todos): una tira del
# cuello en los gnomos; en las sagas, el mechon que cubre la oreja y que de frente queda colgando junto al hombro
# (de perfil y de espalda es pelo largo de verdad y se queda).
FRENTE_F = set(range(12, 18)) | set(range(32, 36))
# J-1: en los perfiles de las sagas (fotogramas 6-11 y 28-31) el pelo es un arco ralo con un mechon colgando suelto
# a la altura del hombro; se corta igual que de frente (a 10 px de la coronilla).
PERFIL_F = set(range(6, 12)) | set(range(28, 32))
ZONA = {"gnomos_de_la_isla": (14, None), "sagas_de_ashye": (10, FRENTE_F | PERFIL_F)}
# J-1: componentes sueltas junto a la cabeza que no son pelo: por debajo de este tamano se van (solo estos origenes;
# los demas cabellos no cambian). Las sagas y los enanos las llevan de frente (4-7 px a un lado de la cabeza).
SUELTO = {"sagas_de_ashye": 10, "enanos_forjadores": 10}

def limpiar(fr, regla, ytop=None, zona=None, k=0, minimo=MIN_COMPONENTE):
    """Quita lo que marca `regla`, lo que cae `zona` px o mas bajo la coronilla del cuerpo de origen (`ytop`) y
    las componentes (8-conexas) de menos de MIN_COMPONENTE px."""
    out = fr.copy()
    h, s, v = _hsv(fr)
    out[regla(h, s, v) & (fr[..., 3] > 0)] = 0
    if zona is not None and ytop is not None and (zona[1] is None or k in zona[1]):
        out[int(ytop + zona[0]):] = 0
    m = out[..., 3] > 0
    lab, n = ndimage.label(m, structure=np.ones((3, 3)))
    if n:
        tam = ndimage.sum(m, lab, index=np.arange(1, n + 1))
        for i in np.nonzero(tam < minimo)[0]:
            out[lab == i + 1] = 0
    return out


def tonos(man):
    """Los 5 tonos de pelo_castano, de claro a oscuro."""
    return np.array([[int(h.lstrip("#")[i:i + 2], 16) for i in (0, 2, 4)] for h in man["paletas"][BASE].split()], np.uint8)


def cuantizar(piezas, paleta):
    """Cuantiza por luminancia a los 5 tonos: normaliza con los percentiles 3-97 de TODOS los frames del estilo
    (el pelo blanco, el negro y el rubio reparten bien las sombras) y corta en 5 tramos iguales. Alfa 0/255."""
    m = piezas[..., 3] >= 128
    lum = (0.299 * piezas[..., 0] + 0.587 * piezas[..., 1] + 0.114 * piezas[..., 2]).astype(np.float32)
    lo, hi = np.percentile(lum[m], [3, 97])
    t = np.clip((lum - lo) / max(hi - lo, 1.0), 0, 0.9999)
    idx = (4 - np.floor(t * 5)).astype(int)  # 0 = el tono mas claro
    out = np.zeros_like(piezas)
    out[m, :3] = paleta[idx[m]]
    out[m, 3] = 255
    return out


def origen_limpio(tex, man, ests):
    """{estilo: (40,64,64,4) ya limpio y cuantizado, aun sin mover} y {estilo: anclas (cx, ytop) de su raza}.
    Un atlas cada vez."""
    paleta = tonos(man)
    razas = {r["id"]: r for r in man["razas"]["lista"]}
    filas = {f["id"]: f for f in man["pelo"]["filas"]}
    piezas, anclas = {}, {}
    por_talla = {}
    for estilo, origen, _ in PELOS:
        if estilo in ests:
            por_talla.setdefault(razas[origen]["talla"], []).append((estilo, origen))
    for talla, lista in por_talla.items():
        atl = cargar(tex, man["pelo"]["atlas"][talla])
        for estilo, origen in lista:
            piezas[estilo] = (frames(atl, filas["propio/" + origen]["fila"]), origen)
        del atl
    cuerpo = cargar(tex, man["razas"]["atlas"]["cuerpo"])
    for estilo, origen, _ in PELOS:
        if estilo in ests:
            fr = frames(cuerpo, razas[origen]["fila"])
            anclas[estilo] = [cabeza(f) for f in fr]
    del cuerpo
    for estilo in list(piezas):
        crudo, origen = piezas[estilo]
        piezas[estilo] = (crudo, limpiar_todo(crudo, LIMPIEZA[origen], anclas[estilo], ZONA.get(origen), SUELTO.get(origen, MIN_COMPONENTE)))
    for estilo in list(piezas):
        crudo, limpio = piezas[estilo]
        piezas[estilo] = (crudo, cuantizar(limpio, paleta))
    return piezas, anclas


def limpiar_todo(crudo, regla, anclas, zona, minimo=MIN_COMPONENTE):
    return np.stack([limpiar(f, regla, a[1] if a else None, zona, k, minimo) for k, (f, a) in enumerate(zip(crudo, anclas))])


def anclas_destino(tex, man, talla):
    atl = cargar(tex, man["cuerpos"]["atlas"][talla])
    fila = next(f for f in man["cuerpos"]["filas"] if f["id"] == "piel_clara")["fila"]
    return [cabeza(f) for f in frames(atl, fila)]


def mover(pieza, a_origen, a_destino):
    out = np.zeros_like(pieza)
    for k in range(KF):
        if a_origen[k] is None or a_destino[k] is None:
            out[k] = pieza[k]
            continue
        dx = int(round(a_destino[k][0] - a_origen[k][0]))
        dy = int(a_destino[k][1] - a_origen[k][1])
        out[k] = desplazar(pieza[k], dx, dy)
    return out


def filas_nuevas(man):
    """{estilo: fila 1-based} en `personajes.json`: las que ya estan o las que van a continuacion."""
    ya = {f["estilo"]: f["fila"] for f in man["pelo"]["filas"] if f.get("tallas")}
    sig = max(f["fila"] for f in man["pelo"]["filas"]) + 1
    out = {}
    for estilo in ESTILOS:
        if estilo in ya:
            out[estilo] = ya[estilo]
        else:
            out[estilo] = sig
            sig += 1
    return out


def esperado_talla(tex, man, talla, piezas, anclas):
    """{estilo: (40,64,64,4)} de esa talla destino."""
    ad = anclas_destino(tex, man, talla)
    return {e: mover(piezas[e][1], anclas[e], ad) for e in ESTILOS}


def guardar_atlas(tex, man, talla, nuevas, total_filas):
    """Pone las filas `nuevas` {fila: frames} en el atlas de esa talla (lo alarga con filas transparentes si falta)."""
    ruta = tex / man["pelo"]["atlas"][talla]
    a = cargar(tex, man["pelo"]["atlas"][talla])
    if not nuevas and a.shape[0] >= total_filas * L:
        return False
    if a.shape[0] < total_filas * L:
        a = np.concatenate([a, np.zeros((total_filas * L - a.shape[0], a.shape[1], 4), np.uint8)])
    else:
        a = a.copy()
    for fila, fr in nuevas.items():
        for k in range(KF):
            a[(fila - 1) * L:fila * L, k * L:(k + 1) * L] = fr[k]
    Image.fromarray(a, "RGBA").save(ruta, optimize=True)
    return True


def actualizar_manifiesto(man, filas):
    nom = {e: n for e, _, n in PELOS}
    por_id = {f["id"]: f for f in man["pelo"]["filas"]}
    for e in ESTILOS:
        ident = f"{e}/{BASE}"
        reg = {"id": ident, "estilo": e, "color": BASE, "fila": filas[e], "tallas": list(TALLAS), "nombre": nom[e]}
        if ident in por_id:
            por_id[ident].clear()
            por_id[ident].update(reg)
        else:
            man["pelo"]["filas"].append(reg)


def procesar(tex, comprobar):
    man, texto = leer_man(tex)
    if texto != formato(man):
        print("AVISO: personajes.json no tiene el formato de json.dumps(indent=1); se reescribira entero", file=sys.stderr)
    filas = filas_nuevas(man)
    total = max(max(filas.values()), max(f["fila"] for f in man["pelo"]["filas"]))
    piezas, anclas = origen_limpio(tex, man, set(ESTILOS))
    fallos = []
    for talla in man["pelo"]["atlas"]:  # un atlas cada vez
        if talla in TALLAS:
            esp = esperado_talla(tex, man, talla, piezas, anclas)
            if comprobar:
                a = cargar(tex, man["pelo"]["atlas"][talla])
                if a.shape[0] < total * L:
                    fallos.append(f"{talla}: el atlas mide {a.shape[0] // L} filas y faltan {total}")
                else:
                    for e in ESTILOS:
                        if not np.array_equal(frames(a, filas[e]), esp[e]):
                            fallos.append(f"{talla}: la fila {filas[e]} ({e}) no es la esperada")
                del a
            else:
                guardar_atlas(tex, man, talla, {filas[e]: esp[e] for e in ESTILOS}, total)
                print("escrito", man["pelo"]["atlas"][talla], flush=True)
        else:
            if comprobar:
                with Image.open(tex / man["pelo"]["atlas"][talla]) as im:
                    if im.size[1] < total * L:
                        fallos.append(f"{talla}: el atlas mide {im.size[1] // L} filas y faltan {total}")
            else:
                if guardar_atlas(tex, man, talla, {}, total):
                    print("alargado", man["pelo"]["atlas"][talla], flush=True)
    previo = json.loads(json.dumps(man))
    actualizar_manifiesto(man, filas)
    if comprobar:
        if man != previo:
            fallos.append("personajes.json: faltan filas o campos tallas/nombre")
        for f in fallos:
            print("FALLA", f)
        print("pelos_compartidos:", "bien" if not fallos else f"{len(fallos)} fallos")
        return 1 if fallos else 0
    (tex / "personajes.json").write_text(formato(man), encoding="utf-8")
    print("personajes.json:", len(man["pelo"]["filas"]), "filas de pelo")
    return 0


# --- hojas de contactos ---------------------------------------------------------------------------------------
def hoja_limpieza(tex, salida):
    from PIL import ImageDraw
    man, _ = leer_man(tex)
    piezas, _ = origen_limpio(tex, man, set(ESTILOS))
    k = 6
    fs = (12, 6, 0, 18)  # frame - 1: frente, derecha, espalda, izquierda
    img = Image.new("RGBA", (3 * 4 * 36 * k + 40, 12 * 36 * k // 1), (70, 70, 80, 255))
    d = ImageDraw.Draw(img)
    # tres columnas por estilo: crudo | limpio y cuantizado (4 vistas cada una)
    img = Image.new("RGBA", (2 * 4 * 36 * k + 30, 12 * 40 * k), (70, 70, 80, 255))
    d = ImageDraw.Draw(img)
    for i, (e, o, _) in enumerate(PELOS):
        crudo, limpio = piezas[e]
        for lado, datos in enumerate((crudo, limpio)):
            for q, f in enumerate(fs):
                im = Image.fromarray(np.ascontiguousarray(datos[f])).crop((14, 10, 50, 50)).resize((36 * k, 40 * k), Image.NEAREST)
                img.alpha_composite(im, (lado * (4 * 36 * k + 30) + q * 36 * k, i * 40 * k))
        d.text((2, i * 40 * k + 2), o, fill=(255, 255, 0, 255))
    img.convert("RGB").save(salida)


RAZAS_HOJA = ["race_human_marches", "elfos_del_bosque", "drow_de_la_casa_noble", "enanos_forjadores",
              "medianos_del_bosque", "orcos_del_consejo"]
COLORES_HOJA = ["pelo_castano", "pelo_negro", "pelo_plata_lila"]


def hoja_contacto(tex, salida):
    from PIL import ImageDraw
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from laminas_desde_capas import Montaje
    M = Montaje(tex)
    k = 3
    w, h = 36 * k, 40 * k
    celdas = len(RAZAS_HOJA) * len(COLORES_HOJA) * 2
    img = Image.new("RGBA", (70 + celdas * w + len(RAZAS_HOJA) * 6, 12 * (h + 2) + 20), (58, 58, 68, 255))
    d = ImageDraw.Draw(img)
    for i, (e, _, n) in enumerate(PELOS):
        y = 20 + i * (h + 2)
        d.text((2, y + 2), e[:12], fill=(255, 230, 120, 255))
        x = 70
        for ri, raza in enumerate(RAZAS_HOJA):
            for c in COLORES_HOJA:
                try:
                    hoja = M.montar({"raza": raza, "pelo": [e, c]})
                except ValueError:
                    hoja = None  # la talla de la raza no lo tiene
                for f in (13, 7):  # frente y perfil (derecha)
                    if hoja is not None:
                        fr = Image.fromarray(np.ascontiguousarray(hoja[:, (f - 1) * L:f * L])).crop((14, 10, 50, 50))
                        img.alpha_composite(fr.resize((w, h), Image.NEAREST), (x, y))
                    else:
                        d.text((x + 6, y + h // 2), "--", fill=(160, 160, 160, 255))
                    x += w
            x += 6
    x = 70
    for raza in RAZAS_HOJA:
        d.text((x + 2, 4), raza[:30], fill=(180, 220, 255, 255))
        x += len(COLORES_HOJA) * 2 * w + 6
    img.convert("RGB").save(salida)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tex", default=str(TEX_DEFECTO))
    ap.add_argument("--comprobar", action="store_true")
    ap.add_argument("--contacto")
    ap.add_argument("--limpieza")
    a = ap.parse_args(argv)
    tex = Path(a.tex)
    if a.limpieza:
        hoja_limpieza(tex, a.limpieza)
        print("limpieza:", a.limpieza)
        return 0
    if a.contacto:
        hoja_contacto(tex, a.contacto)
        print("contacto:", a.contacto)
        return 0
    return procesar(tex, a.comprobar)


if __name__ == "__main__":
    sys.exit(main())
