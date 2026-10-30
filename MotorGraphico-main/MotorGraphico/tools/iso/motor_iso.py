#!/usr/bin/env python3
"""ISO — Motor de dibuix isomètric 2:1 a l'estil Habbo per construir les peces
del joc des de geometria (cases, locals, decorats) en les 4 orientacions, com
els mobles d'Habbo.

Unitats: X i Y en unitats de cel·la (1 cel·la = 32 unitats), Z en píxels.
Projecció (la del motor, IsoMath): sx = X - Y, sy = (X + Y) / 2 - Z, de manera
que una cel·la és un rombe de 64x32 i +X va avall-dreta, +Y avall-esquerra.
Amb vèrtexs a coordenades parelles les arestes horitzontals surten com a
línies 2:1 perfectes (dos píxels per esglaó) i les verticals, verticals.

Cada peça és una llista de cares planes (polígons 3D amb normal i una funció
de color). El llenç fa z-buffer (profunditat X + Y + Z: més gran = més a prop
de la càmera), la llum ve de dalt a l'esquerra (cara de dalt clara, cara
esquerra mitjana, cara dreta fosca) i al final es dibuixen els contorns
(silueta i vores entre peces) d'un color fosc i càlid.
"""
import numpy as np
from PIL import Image

CEL = 32                                  # unitats per cel·la
CONTORN = (43, 29, 24, 255)               # marró molt fosc (Habbo/Stardew: mai negre pur)


def gira(P, o, W, D):
    """Punt canònic (empremta W x D unitats) -> punt de la vista o (0..3, passos de 90°)."""
    X, Y, Z = P
    if o == 0:
        return (X, Y, Z)
    if o == 1:
        return (D - Y, X, Z)
    if o == 2:
        return (W - X, D - Y, Z)
    return (Y, W - X, Z)


def desgira(X, Y, o, W, D):
    """La inversa de gira per a arrays (per a les textures, que van amb la peça)."""
    if o == 0:
        return X, Y
    if o == 1:
        return Y, D - X
    if o == 2:
        return W - X, D - Y
    return W - Y, X


def gira_normal(n, o):
    nx, ny, nz = n
    for _ in range(o):
        nx, ny = -ny, nx
    return (nx, ny, nz)


def to_llum(n):
    """To de la rampa (0 = brillantor ... 4 = ombra fosca) segons la normal de la vista:
    llum de dalt a l'esquerra, com Habbo: dalt clar, esquerra mitjà, dreta fosc."""
    v = np.array(n, float)
    v = v / (np.linalg.norm(v) or 1)
    lum = 0.62 * v[2] + 0.34 * v[1] - 0.18 * v[0]
    if lum >= 0.55:
        return 1
    if lum >= 0.24:
        return 2
    if lum >= -0.25:
        return 3
    return 4


class Llenc:
    """Llenç RGBA amb z-buffer i identificador de peça per píxel."""

    def __init__(self, ample, alt, ox, oy):
        self.w, self.h, self.ox, self.oy = ample, alt, ox, oy
        self.col = np.zeros((alt, ample, 4), np.uint8)
        self.prof = np.full((alt, ample), -1e9)
        self.peca = np.full((alt, ample), -1, np.int32)

    def proj(self, P):
        X, Y, Z = P
        return (X - Y + self.ox, (X + Y) / 2.0 - Z + self.oy)

    def poligon(self, verts, normal, color, peca, biaix=0.0, o=0, W=0, D=0):
        """Omple un polígon pla (vèrtexs ja girats a la vista) amb z-buffer.
        color(Xc, Yc, Z, to) -> (N, 4) uint8, amb coordenades canòniques."""
        n = np.array(normal, float)
        if n[0] + n[1] + n[2] <= 1e-9:
            return                                        # d'esquena a la càmera
        pts = np.array([self.proj(v) for v in verts])
        x0, y0 = np.floor(pts.min(0)).astype(int)
        x1, y1 = np.ceil(pts.max(0)).astype(int) + 1
        x0, y0, x1, y1 = max(x0, 0), max(y0, 0), min(x1, self.w), min(y1, self.h)
        if x0 >= x1 or y0 >= y1:
            return
        GX, GY = np.meshgrid(np.arange(x0, x1) + 0.5 + 1e-4, np.arange(y0, y1) + 0.5 + 2e-4)
        dins = np.zeros(GX.shape, bool)
        for i in range(len(pts)):
            xa, ya = pts[i]
            xb, yb = pts[(i + 1) % len(pts)]
            if ya == yb:
                continue
            creua = (ya > GY) != (yb > GY)
            xt = (xb - xa) * (GY - ya) / (yb - ya) + xa
            dins ^= creua & (GX < xt)
        if not dins.any():
            return
        # El punt 3D del pla que cau a cada píxel (solució tancada de la projecció).
        a, b = GX - self.ox, GY - self.oy
        d = float(n @ np.array(verts[0], float))
        s = (d - a * (n[0] - n[1]) / 2.0 + n[2] * b) * 2.0 / (n[0] + n[1] + n[2])
        X, Y, Z = (s + a) / 2.0, (s - a) / 2.0, s / 2.0 - b
        prof = X + Y + Z + biaix
        zona = self.prof[y0:y1, x0:x1]
        m = dins & (prof > zona)
        if not m.any():
            return
        Xc, Yc = desgira(X[m], Y[m], o, W, D)
        rgba = color(Xc, Yc, Z[m], to_llum(n))
        self.col[y0:y1, x0:x1][m] = rgba
        zona[m] = prof[m]
        self.peca[y0:y1, x0:x1][m] = peca

    def contorns(self, salt=3.0, interior=True, junta=None):
        """Silueta amb el color de contorn; vores entre peces (la de davant, si hi
        ha salt de profunditat) amb el to fosc de la mateixa peça.
        junta = ("e" | "d", columna): INT-01, peces que continuen a la cel·la del
        costat (parets): a l'esquerra (o a la dreta) de la columna, la vora que
        només toca buit per aquell costat no es contorneja."""
        op = self.col[:, :, 3] > 0
        out = self.col.copy()
        h, w = op.shape
        # Silueta: píxels opacs amb un veí transparent (contorn interior: l'empremta no creix).
        vora = np.zeros_like(op)
        vora[1:, :] |= ~op[:-1, :]
        vora[:-1, :] |= ~op[1:, :]
        vora[:, 1:] |= ~op[:, :-1]
        vora[:, :-1] |= ~op[:, 1:]
        vora[0, :] = vora[-1, :] = True
        vora[:, 0] = vora[:, -1] = True
        if junta is not None:
            costat, col = junta
            dalt = np.zeros_like(op)
            dalt[1:, :] = op[:-1, :]
            baix = np.zeros_like(op)
            baix[:-1, :] = op[1:, :]
            lat = np.zeros_like(op)
            xs = np.arange(w)[None, :]
            if costat == "e":
                lat[:, 1:] = ~op[:, :-1]
                lat[:, 0] = True
                zona = xs <= col
            else:
                lat[:, :-1] = ~op[:, 1:]
                lat[:, -1] = True
                zona = xs >= col
            vora &= ~(lat & dalt & baix & zona)
        sil = op & vora
        if interior:
            p, z = self.peca, self.prof
            for dy, dx in ((0, 1), (1, 0), (0, -1), (-1, 0)):
                p2 = np.full_like(p, -1)
                z2 = np.full_like(z, -1e9)
                ys = slice(max(dy, 0), h + min(dy, 0))
                yd = slice(max(-dy, 0), h + min(-dy, 0))
                xs = slice(max(dx, 0), w + min(dx, 0))
                xd = slice(max(-dx, 0), w + min(-dx, 0))
                p2[yd, xd] = p[ys, xs]
                z2[yd, xd] = z[ys, xs]
                vora_int = op & (p2 >= 0) & (p2 != p) & (z - z2 > salt)
                out[vora_int] = fosc(out[vora_int])
        out[sil] = CONTORN
        self.col = out

    def imatge(self):
        return Image.fromarray(self.col, "RGBA")


def fosc(rgba, f=0.62):
    r = rgba.astype(float)
    r[:, :3] = r[:, :3] * f + np.array([43, 29, 24]) * (1 - f) * 0.5
    return np.clip(r, 0, 255).astype(np.uint8)
