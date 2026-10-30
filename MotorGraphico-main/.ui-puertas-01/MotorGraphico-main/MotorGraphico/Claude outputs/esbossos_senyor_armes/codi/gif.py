"""The three proposals walking around (front, right, back, left), side by side, with the glow."""
import sys
sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parent))
from PIL import Image, ImageDraw
from ver import frames_de
import senor  # noqa: F401
from tres import PROPOSTES
from lamina import cela, font, CAJA, FONDO
import lamina

ESC = 3
lamina.ESC = ESC
CICLO = [7, 8, 9, 8] * 2 + [4, 5, 6, 5] * 2 + [1, 2, 3, 2] * 2 + [10, 11, 12, 11] * 2
NOMS = ["1 · El Duc", "2 · El Sobirà", "3 · El Rei de la Forja"]


def main(salida=str(__import__("pathlib").Path(__file__).resolve().parent.parent / "esbossos_senyor_armes.gif")):
    frs = [frames_de(spec, tintes) for _n, spec, tintes in PROPOSTES]
    cw, ch = (CAJA[2] - CAJA[0]) * ESC, (CAJA[3] - CAJA[1]) * ESC
    sep, peu = 10, 30
    W = 3 * cw + 4 * sep
    H = ch + peu + sep
    imgs = []
    for n, fi in enumerate(CICLO):
        img = Image.new("RGB", (W, H), FONDO)
        d = ImageDraw.Draw(img)
        for k, fr in enumerate(frs):
            x = sep + k * (cw + sep)
            img.paste(cela(fr, fi, k, t=n * 0.5), (x, sep // 2))
            nom = NOMS[k]
            tw = d.textlength(nom, font=font(15))
            d.text((x + cw // 2 - tw // 2, ch + sep // 2 + 6), nom, font=font(15), fill=(255, 226, 150))
        imgs.append(img.quantize(colors=255, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE))
    imgs[0].save(salida, save_all=True, append_images=imgs[1:], duration=150, loop=0, disposal=1)
    print(salida, imgs[0].size, len(imgs))


if __name__ == "__main__":
    main()
