"""The sketch sheet: the three proposals, standing (front, side, back) and walking, with the glow."""
import sys
sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parent))
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from ver import frames_de
import senor  # noqa: F401
from tres import PROPOSTES
from fx import con_efectos, tonos_que_brillan

FONDO = (34, 36, 42)
ESC = 4
CAJA = (6, 2, 58, 64)   # x0, y0, x1, y1 of the 64x64 cell
BRILLAN = [
    (tonos_que_brillan(["mana_duc"]) | tonos_que_brillan(["gema_duc"], ("H",)), (255, 205, 90)),
    (tonos_que_brillan(["visera_sobira", "espectre_sobira"]), (255, 196, 80)),
    (tonos_que_brillan(["or_fos"], ("H", "s", "d")), (255, 170, 60)),
]
CHISPAS = [
    [(14, 40, 9, (255, 220, 120)), (46, 30, 7, (255, 90, 80)), (20, 22, 6, (255, 230, 150)), (42, 48, 8, (255, 210, 110))],
    [(12, 44, 8, (255, 90, 80)), (50, 36, 7, (255, 210, 110)), (18, 28, 6, (255, 70, 70)), (44, 50, 9, (255, 220, 120))],
    [(16, 42, 10, (255, 150, 60)), (46, 34, 8, (255, 200, 90)), (22, 26, 7, (255, 120, 50)), (40, 50, 9, (255, 180, 70))],
]
TITOLS = [
    ("1 · El Duc", "Casaca negra de coll alt amb cantells d'or, espatlleres d'or, capa vermella des de la cintura, "
                   "línies de mana vermell als braços i quatre fulles que suren."),
    ("2 · El Sobirà sense rostre", "Elm negre amb corona de fulles i l'escletxa que brilla de vermell, placa negra i "
                                   "or, tabard i capa vermells, fulles espectrals al voltant."),
    ("3 · El Rei de la Forja", "Ferro fosc amb esquerdes d'or fos, corona de fulles trencades amb les puntes "
                               "roents, espadó a l'esquena acabat de forjar."),
]


def font(n, bold=True):
    nom = "DejaVuSans-Bold.ttf" if bold else "DejaVuSans.ttf"
    return ImageFont.truetype(f"/usr/share/fonts/truetype/dejavu/{nom}", n)


def cela(fr, i, k, t=0.0):
    px = fr[i - 1][CAJA[1]:CAJA[3], CAJA[0]:CAJA[2]]
    brillan, aura = BRILLAN[k]
    ch = [(x - CAJA[0], y - CAJA[1], v, c) for (x, y, v, c) in CHISPAS[k]]
    return con_efectos(px, ESC, brillan, aura=aura, chispas=ch, fondo=FONDO, t=t)


def main(salida=str(__import__("pathlib").Path(__file__).resolve().parent.parent / "esbossos_senyor_armes.png")):
    cols = [17, 15, 13, 7, 9]
    cw, ch = (CAJA[2] - CAJA[0]) * ESC, (CAJA[3] - CAJA[1]) * ESC
    marge, cap, cap0 = 24, 74, 30
    W = marge * 2 + len(cols) * cw
    H = marge + cap0 + 3 * (cap + ch + 18)
    img = Image.new("RGB", (W, H), FONDO)
    d = ImageDraw.Draw(img)
    for j, etiqueta in enumerate(["quiet, de cara", "de perfil", "d'esquena", "caminant", "caminant"]):
        x = marge + j * cw + cw // 2 - int(d.textlength(etiqueta, font=font(14, False))) // 2
        d.text((x, marge), etiqueta, font=font(14, False), fill=(150, 152, 165))
    for k, (nombre, spec, tintes) in enumerate(PROPOSTES):
        fr = frames_de(spec, tintes)
        y = marge + cap0 + k * (cap + ch + 18)
        titol, sub = TITOLS[k]
        d.text((marge, y), titol, font=font(26), fill=(255, 226, 150))
        # wrap the subtitle
        paraules, linia, linies = sub.split(), "", []
        for p in paraules:
            prova = (linia + " " + p).strip()
            if d.textlength(prova, font=font(15, False)) > W - 2 * marge:
                linies.append(linia)
                linia = p
            else:
                linia = prova
        linies.append(linia)
        for j, l in enumerate(linies[:2]):
            d.text((marge, y + 34 + j * 19), l, font=font(15, False), fill=(214, 214, 222))
        for j, fi in enumerate(cols):
            img.paste(cela(fr, fi, k, t=0.35 * j), (marge + j * cw, y + cap))
    img.save(salida)
    print(salida, img.size)


if __name__ == "__main__":
    main()
