#!/usr/bin/env python3
"""CAR-2 (Oriol, 29/10, «les cares surten bugejades»): la capa de ojos de verdad.

Antes, `actores_capas.capa_ojos` copiaba los pixeles del cuerpo base de la talla (piel, cejas, pico...) y las 10
filas de color salian iguales. Ahora cada color pinta SOLO el iris y la pupila del cuerpo, y de cada cuerpo por
separado: las cabezas de las razas de PixelLab estan dibujadas aparte (cada una con los ojos en su sitio, a 1-5
pixeles de la base de la familia), asi que una mascara por talla no encaja en casi ninguna.

Como se halla la mascara de ojos de un cuerpo (`detectar`), frame a frame, solo en las diagonales de frente
(7-18 andar y 29-36 respirar; de espaldas no hay ojos):
  1. Banda: de 1 fila por encima a 3 por debajo de la fila de los ojos (la boca y la barbilla quedan fuera). Esa fila es la de la familia
     (`cuerpos_<talla>`, fila 1): sus ojos quedan 8 filas bajo la coronilla de cada frame.
  2. Paleta de piel del cuerpo: los colores (L1 < 14) de todo lo que hay por debajo de la cabeza en los 40 frames
     (piel, contorno, sombras). Los ojos usan colores que no estan en el resto del cuerpo.
  3. Candidatos: pixeles interiores (sin vecino transparente) de la banda que no son de esa paleta, sin el blanco
     del ojo (muy claro y gris: queda como esta) ni los brillos de la piel (un color de la paleta aclarado hacia el
     blanco: `brillo_de_piel`). Se agrupan en piezas de 8 vecinos y se quedan las de 6 px o
     menos (el pelo, una marca o la frente no son iris).
Una raza admite `ojos` si en los 20 frames de frente hay iris «limpio» (`limpio`: de 1 a 12 pixeles, en uno o dos
grupos compactos de 8x4 como mucho); si no, el cuerpo no tiene unos ojos que se puedan repintar sin manchar (un
morro de reptil con dientes, una piel de hielo) y la raza lleva `sin: ["ojos"]`.

Cada color pinta esos pixeles con su rampa (OJOS: i1 oscuro, i2 base, i3 brillo; el tono sale de la luz del
pixel). La capa se guarda por raza (`personajes/ojos/ojos_<raza>.png`, 10 filas como el resto) y por talla
(`personajes/cuerpos/ojos_<talla>.png`: la del cuerpo liso de la familia, que es el que se monta al cambiar de
piel o de talla; `grande` no tiene: TALLAS_SIN_OJOS). Una capa que no pasa `admite` nunca se escribe. El motor elige la de la raza salvo con cuerpo liso (MontajePersonajes::capas).

Uso (desde MotorGraphico/):
  python3 tools/capa_ojos_iris.py                 # escribe atlas y personajes.json (ojos + sin de las razas)
  python3 tools/capa_ojos_iris.py --comprobar     # solo informa
  python3 tools/capa_ojos_iris.py --tex DIR       # otra carpeta de texturas
No necesita gen_personajes.py: solo Pillow, numpy y scipy.
"""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage

L = 64
KF = 40
TALLAS = ("medio", "bajo", "pequeno", "grande")
# Frames de frente (1-based): andar SE 7-12, andar SO 13-18, respirar SE 29-32, respirar SO 33-36
FRAMES_OJOS = tuple(range(7, 19)) + tuple(range(29, 37))
OJOS_BAJO_CORONILLA = 8     # los ojos del cuerpo de familia, bajo la fila mas alta del frame
BANDA = (-1, 4)             # filas de la banda, respecto a la fila de los ojos (arriba incluido, abajo no)
FILA_BAJO_CABEZA = 18       # la paleta de piel se saca de lo que queda bajo la coronilla + esto
TOL_PALETA = 14             # distancia L1 a un color de la paleta para ser «piel»
MAX_PIEZA = 6               # un iris es de 6 px o menos por pieza
MAX_PIXELES = 12            # y una raza con mas de esto por frame no tiene unos ojos limpios
MAX_OJOS = 2                # grupos de pixeles (cada uno con su holgura de 1) por frame: un ojo o dos
MAX_ANCHO, MAX_ALTO = 8, 4  # y lo que mide cada grupo: mas es una fila de dientes, escamas o un tatuaje
ESTRUCT_4 = np.array([[0, 1, 0], [1, 1, 1], [0, 1, 0]])

# Razas a las que el metodo no les ve bien los ojos aunque los pixeles pasen `limpio` (revisado a ojo en las
# diagonales de frente de las 61 razas): su cara lleva dibujos de otro color que no son iris.
TALLAS_SIN_OJOS = {"grande": "en respirar SE (29-32) la mascara coge un pixel de piel de la mejilla y en SO (33-36) solo 3-4 px: "
                             "ninguna mascara automatica pasa `admite`"}
SIN_A_MANO = {"race_dwarf_deepforge": "pintura de guerra roja en la frente y la mejilla",
              "trasgos_de_gliaddokx": "la mascara coge los colmillos amarillos (f5e881/f5dd8a), no los ojos",
              "hombres_lagarto_del_templo_profundo": "la mascara coge un diente inferior (87877a)",
              "drow_de_los_vientos": "la mascara coge un mechon azul junto al cuerno (a0bfdc) y un pixel de la cara (b36243)",
              # CAR-2c: sus colores de «iris» (amarillo claro) tambien salen en las vistas de espalda (test_capa_ojos_iris)
              "draconidos_de_fuego": "el amarillo claro de la mascara sale tambien de espaldas",
              "hombres_lagarto_de_la_selva": "el amarillo claro de la mascara sale tambien de espaldas",
              "hombres_lagarto_del_pantano": "el amarillo claro de la mascara sale tambien de espaldas",
              "draconidos_de_obsidiana": "el amarillo claro de la mascara sale tambien de espaldas",
              # CAR-2d: su unico iris de una diagonal era un parpado naranja (b36243): sin parpado, esa diagonal queda sin iris
              "alquimistas_de_aegroum": "en la diagonal SO el unico pixel era un parpado naranja (b36243)",
              "diplomaticos_de_bastrea": "en la diagonal SO el unico pixel era un parpado naranja (b36243)",
              "medianos_urbanos": "en la diagonal SE el unico pixel era un parpado naranja (b36243)",
              # CAR-2e: lista blanca conservadora (V-CAR-2d): la capa pintaba algo que no es iris
              "yokai_de_las_sombras": "en SO solo pintaba un pixel de la galta (d9ae99); los ojos negros no se pintan",
              "race_ascaria_fey_blood": "en SO pintaba la linea de parpado/ceja (y=28: 321130, 200b20, ba96c8)",
              "tieflings_de_la_sombra": "en SE pintaba el mechon (a0bfdc) en vez del iris",
              "sagas_de_ashye": "en SE su unico pixel era el mechon (a0bfdc)",
              "guardianes_del_elemento": "pintaba el blanco del ojo (feebd8)",
              # CAR-2e: no pasan el test independiente (color de la mejilla / casi blanco / tono distinto entre diagonales)
              "drow_de_la_casa_noble": "pintaba un color de la mejilla (fabdbd)",
              "drow_de_la_locura": "pintaba un color de la mejilla (f9eca9)",
              "elfos_de_ithon": "pintaba un color de la mejilla (966f87)",
              "race_elf_canopy": "pintaba un color de la mejilla (7d8b4c)",
              "tieflings_del_vacio": "pintaba el blanco del ojo (casi blanco)",
              "navegantes_de_ranmont": "tono de SE (mechon azul a0bfdc) distinto del de SO (f4e2bd)",
              # CAR-2e, revision a ojo del full ampliado (car2e_final.png)
              "archivistas_de_numandum": "en SO pintaba un pixel rojo en la ceja/raiz del pelo (26,28), fuera del ojo",
              "enanos_historiadores": "su capa no cambia ningun pixel visible en ninguna diagonal (pixel oculto bajo el pelo/barba)",
              "traidores_oscuros": "su capa no cambia ningun pixel visible en ninguna diagonal (pixel oculto bajo el pelo)",
              "elfos_del_aire": "su capa no cambia ningun pixel visible en ninguna diagonal (pixel oculto bajo el pelo)"}

# El azul de siempre (el de los cuerpos de PixelLab) como rampa propia: OJOS["azul"] esta vacio («los del cuerpo»).
AZUL = {"i1": (30, 60, 120), "i2": (70, 120, 200), "i3": (160, 191, 220)}


def rampa(color):
    """La rampa i1/i2/i3 de un color de ojos (OJOS), con el azul de PixelLab para «azul»."""
    from actores_materiales import OJOS
    return AZUL if color == "azul" else OJOS[color]


def _leer_atlas(ruta):
    return np.asarray(Image.open(ruta).convert("RGBA"))


def filas_de(atlas, n_filas):
    """(n_filas, KF, L, L, 4) de un atlas de 40 columnas."""
    return atlas.reshape(n_filas, L, KF, L, 4).transpose(0, 2, 1, 3, 4)


def _guardar_atlas(filas, ruta):
    """filas: (n, KF, L, L, 4) -> PNG."""
    n = filas.shape[0]
    img = filas.transpose(0, 2, 1, 3, 4).reshape(n * L, KF * L, 4)
    Path(ruta).parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(np.ascontiguousarray(img), "RGBA").save(ruta, optimize=True)


def coronillas(familia):
    """Fila mas alta de cada frame del cuerpo de la familia (KF,)."""
    return np.array([int(np.nonzero((familia[f, ..., 3] > 0).any(axis=1))[0].min()) for f in range(KF)])


def paleta_piel(cuerpo, coronilla):
    """Los colores de lo que hay bajo la cabeza en todos los frames: piel, sombras y contorno del cuerpo."""
    cols = set()
    for f in range(KF):
        sub = cuerpo[f, coronilla[f] + FILA_BAJO_CABEZA:]
        cols.update(map(tuple, np.unique(sub[sub[..., 3] > 0][:, :3], axis=0).tolist()))
    return np.array(sorted(cols), int) if cols else np.zeros((0, 3), int)


def brillo_de_piel(px, pal, tol=10):
    """Los pixeles (N,3) que son un brillo de la piel: un color de la paleta aclarado hacia el blanco (la mejilla
    clara de un enano rojo no es un iris)."""
    blanco = np.array([255, 255, 255])
    out = np.zeros(len(px), bool)
    for q in pal:
        d = blanco - q
        dd = int((d * d).sum())
        if dd == 0:
            continue
        t = np.clip(((px - q) @ d) / dd, 0.0, 0.8)
        resto = np.abs(px - (q + t[:, None] * d)).sum(-1)
        out |= (t > 0.05) & (resto <= tol)
    return out


def detectar(cuerpo, coronilla, tol=TOL_PALETA):
    """Mascara de iris/pupila (KF, L, L) de un cuerpo (KF, L, L, 4). Ver la cabecera para el metodo."""
    pal = paleta_piel(cuerpo, coronilla)
    mask = np.zeros((KF, L, L), bool)
    for fr in FRAMES_OJOS:
        f = fr - 1
        fila_ojos = int(coronilla[f]) + OJOS_BAJO_CORONILLA
        alfa = cuerpo[f, ..., 3] > 0
        interior = ndimage.binary_erosion(alfa, structure=ESTRUCT_4)
        banda = np.zeros_like(alfa)
        banda[max(0, fila_ojos + BANDA[0]):fila_ojos + BANDA[1]] = True
        ys, xs = np.nonzero(interior & banda)
        if not len(ys):
            continue
        px = cuerpo[f][ys, xs][:, :3].astype(int)
        if len(pal):
            lejos = np.abs(px[:, None, :] - pal[None]).sum(-1).min(-1) >= tol
        else:
            lejos = np.ones(len(ys), bool)
        luz = px @ np.array([0.299, 0.587, 0.114])
        blanco = (luz >= 215) & ((px.max(1) - px.min(1)) <= 30)
        piel = np.array([es_piel_naranja(p) for p in px.tolist()])  # CAR-2d: parpado/piel naranja, no iris
        ok = lejos & ~blanco & ~piel
        if len(pal) and ok.any():
            ok[ok] = ~brillo_de_piel(px[ok], pal)
        cand = np.zeros_like(alfa)
        cand[ys[ok], xs[ok]] = True
        piezas, n = ndimage.label(cand, structure=np.ones((3, 3)))
        for k in range(1, n + 1):
            pieza = piezas == k
            if pieza.sum() <= MAX_PIEZA:
                mask[f] |= pieza
    return mask


def admite_familia(mask):
    """True si hay iris limpio en los 20 frames de frente (el cuerpo liso de una talla no admite frames vacios)."""
    return all(limpio(mask[fr - 1]) for fr in FRAMES_OJOS)


def mascara_familia(familia, cor):
    """Mascara del cuerpo liso de una talla. Sus cejas quedan fuera de la paleta de piel: se prueba con holgura 14
    y con 40 y se queda la primera que pasa `admite` (si ninguna, la de 14)."""
    return next((x for x in (detectar(familia, cor, t) for t in (TOL_PALETA, 40)) if admite_familia(x)),
                detectar(familia, cor, TOL_PALETA))


def _hsv(px):
    import colorsys
    h, s_, v = colorsys.rgb_to_hsv(*(x / 255 for x in px))
    return h * 360, s_, v


def es_piel_naranja(px):
    """CAR-2d: True si el pixel (r,g,b) es de piel o parpado (matiz naranja, saturacion media, claro): el mismo
    criterio con el que `color_del_iris` lo descarta. Un pixel asi nunca entra en la mascara de iris."""
    h, s_, v = _hsv(px)
    return 5 <= h <= 35 and s_ < 0.7 and 0.55 < v <= 0.84  # CAR-2f: 0.82 dejaba pasar d29b74 (v=0.8235) en ojos_medio


def color_del_iris(cuerpo, mask):
    """(color, ambiguo): el color de la rampa OJOS mas cercano al iris que dibuja el cuerpo (los pixeles de la
    mascara en los frames de frente). Se descartan los oscuros (pupila, contorno) y los de piel (matiz naranja,
    claros); si quedan pixeles de color (saturacion >= 0.3) cuentan solo esos, y si no, los palidos (el azul claro
    de PixelLab). Cada pixel puntua por su distancia RGB a la rampa (i1/i2/i3). Es ambiguo si el segundo color
    queda a menos de un 25 % mas lejos (amarillo/dorado/ambar) o si el matiz dominante no coincide."""
    from actores_materiales import OJOS
    cols = list(OJOS)
    fuerte, palido = [], []
    for fr in FRAMES_OJOS:
        for px in cuerpo[fr - 1][mask[fr - 1]][:, :3].astype(int).tolist():
            h, s_, v = _hsv(px)
            if v < 0.25 or es_piel_naranja(px):
                continue
            if s_ >= 0.3:
                fuerte.append(px)
            elif s_ >= 0.12:
                palido.append(px)
    usados = fuerte or palido
    if not usados:
        return None, True
    P = np.array(usados, float)
    rampas = {c: np.array([v for k, v in rampa(c).items() if k in ("i1", "i2", "i3")], float) for c in cols}
    sc = {c: float(np.sqrt(((P[:, None, :] - rp[None]) ** 2).sum(-1)).min(1).mean()) for c, rp in rampas.items()}
    o = sorted(sc, key=sc.get)
    # matiz dominante: el de la rampa i2 mas cercano en angulo (marron: naranja oscuro); negro/blanco no tienen matiz
    def matiz(px):
        h, s_, v = _hsv(px)
        if v < 0.55 and 10 <= h <= 45:
            return "marron"
        cro = [c for c in cols if c not in ("negro", "blanco", "marron")]
        return min(cro, key=lambda c: min(abs(h - _hsv(rampa(c)["i2"])[0]) % 360, 360 - abs(h - _hsv(rampa(c)["i2"])[0]) % 360))
    votos = {}
    for px in usados:
        votos[matiz(px)] = votos.get(matiz(px), 0) + 1
    dominante = max(votos, key=votos.get)
    return o[0], sc[o[1]] < 1.25 * sc[o[0]] or dominante != o[0]


def limpio(m):
    """True si los pixeles de un frame son uno o dos ojos y no manchas: pocos, en MAX_OJOS grupos compactos."""
    n = int(m.sum())
    if n < 1 or n > MAX_PIXELES:
        return False
    grupos, k = ndimage.label(ndimage.binary_dilation(m, structure=np.ones((3, 3))))
    if k > MAX_OJOS:
        return False
    for g in range(1, k + 1):
        ys, xs = np.nonzero((grupos == g) & m)
        if xs.max() - xs.min() + 1 > MAX_ANCHO or ys.max() - ys.min() + 1 > MAX_ALTO:
            return False
    return True


GRUPO_SE = tuple(range(7, 13)) + tuple(range(29, 33))   # diagonal SE: andar 7-12, respirar 29-32
GRUPO_SO = tuple(range(13, 19)) + tuple(range(33, 37))  # diagonal SO: andar 13-18, respirar 33-36


def admite(mask):
    """True si hay iris limpio en TODOS los frames de frente de las dos diagonales (SE 7-12 y 29-32; SO 13-18 y
    33-36). CAR-2e: se vuelve a exigir en cada frame (CAR-2d dejaba frames vacios y dejaba pasar razas sin iris
    en una diagonal); si no, la raza no lleva capa."""
    return all(limpio(mask[fr - 1]) for g in (GRUPO_SE, GRUPO_SO) for fr in g)


def pintar(cuerpo, mask, color):
    """(KF, L, L, 4): solo los pixeles de la mascara, con la rampa del color (tono por la luz del pixel)."""
    r = rampa(color)
    out = np.zeros_like(cuerpo)
    luz = cuerpo[..., :3].astype(float) @ np.array([0.299, 0.587, 0.114])
    for clave, sel in (("i1", luz < 85), ("i2", (luz >= 85) & (luz < 175)), ("i3", luz >= 175)):
        m = mask & sel
        out[m, :3] = r[clave]
    out[mask, 3] = 255
    return out


def capa_color(cuerpo, mask, colores):
    """(len(colores), KF, L, L, 4): una fila por color."""
    return np.stack([pintar(cuerpo, mask, c) for c in colores])


def generar(tex, man=None, escribir=True, informe=None):
    """Rehace los atlas de ojos desde las capas que hay en `tex` (carpeta de texturas). Devuelve
    ({raza: mascara}, {talla: mascara}, razas sin ojos). Con escribir=False no toca nada.
    man: el manifiesto en memoria (lo llama gen_personajes.py: se le pone el atlas de ojos y el «sin» de las
    razas, y el que llama lo guarda); sin el, se lee y se reescribe tex/personajes.json."""
    tex = Path(tex)
    en_disco = man is None
    if en_disco:
        man = json.loads((tex / "personajes.json").read_text(encoding="utf-8"))
    colores = [f["id"] for f in man["ojos"]["filas"]]
    razas = man["razas"]["lista"]
    cuerpo_r = _leer_atlas(tex / man["razas"]["atlas"]["cuerpo"])
    cuerpo_r = filas_de(cuerpo_r, cuerpo_r.shape[0] // L)
    masc_talla, masc_raza, sin = {}, {}, []
    atlas = dict(man["ojos"]["atlas"])
    for talla in TALLAS:
        familia = filas_de(_leer_atlas(tex / man["cuerpos"]["atlas"][talla]), len(man["cuerpos"]["filas"]))[0]
        cor = coronillas(familia)
        m = mascara_familia(familia, cor)
        ruta_t = tex / "personajes" / "cuerpos" / f"ojos_{talla}.png"
        if not admite_familia(m):
            # Nunca se escribe una capa que no pasa `admite`: o la talla esta declarada sin capa, o es un error
            if talla not in TALLAS_SIN_OJOS:
                raise ValueError(f"el cuerpo liso de {talla} no pasa `admite` y no esta en TALLAS_SIN_OJOS")
            atlas.pop(talla, None)
            if escribir and ruta_t.exists():
                ruta_t.unlink()
            if informe is not None:
                informe.append(f"{talla} (cuerpo liso): sin ojos, {TALLAS_SIN_OJOS[talla]}")
        else:
            masc_talla[talla] = m
            if escribir:
                _guardar_atlas(capa_color(familia, m, colores), ruta_t)
            atlas[talla] = f"personajes/cuerpos/ojos_{talla}.png"
        for r in razas:
            if r["talla"] != talla:
                continue
            cuerpo = cuerpo_r[r["fila"] - 1]
            mr = detectar(cuerpo, cor)
            if r["id"] in SIN_A_MANO or not admite(mr):
                sin.append(r["id"])
                atlas.pop(r["id"], None)
                huerfano = tex / "personajes" / "ojos" / f"ojos_{r['id']}.png"
                if escribir and huerfano.exists():
                    huerfano.unlink()  # una raza sin ojos no deja capa suelta
                if informe is not None:
                    informe.append(f"{r['id']} ({talla}): sin ojos (px por frame de frente: "
                                   f"{[int(mr[fr - 1].sum()) for fr in FRAMES_OJOS]})")
                continue
            masc_raza[r["id"]] = mr
            ruta = f"personajes/ojos/ojos_{r['id']}.png"
            atlas[r["id"]] = ruta
            if escribir:
                _guardar_atlas(capa_color(cuerpo, mr, colores), tex / ruta)
    if escribir:
        man["ojos"]["atlas"] = atlas
        for r in razas:
            if r["talla"] in TALLAS:
                s = set(r.get("sin", [])) - {"ojos"}
                if r["id"] in sin:
                    s.add("ojos")
                r["sin"] = sorted(s)
        if en_disco:
            (tex / "personajes.json").write_text(json.dumps(man, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    return masc_raza, masc_talla, sin


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--tex", default=str(Path(__file__).resolve().parent.parent / "assets" / "textures"))
    ap.add_argument("--comprobar", action="store_true", help="solo informa, no escribe")
    a = ap.parse_args(argv)
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    informe = []
    masc_raza, masc_talla, sin = generar(a.tex, escribir=not a.comprobar, informe=informe)
    print("\n".join(informe))
    print(f"razas con ojos: {len(masc_raza)}; sin ojos (nuevas): {len(sin)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
