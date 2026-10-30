#!/usr/bin/env python3
"""ISO — Materials, textures i peces (caixa, teulada a dues aigües, cilindre,
rectangles sobre una paret, porta d'arc, plantes, roba estesa) per al motor
isomètric de tools/iso/motor_iso.py, i el dibuix d'una escena en les 4
orientacions.

Una escena és una llista d'elements en coordenades canòniques (l'empremta va
de (0, 0) a (W, D) unitats; 32 unitats = 1 cel·la). La porta principal va a la
cara +Y (la cara esquerra a l'orientació 0). Orientacions: 0 i 3 mostren la
façana de la porta; 1 i 2, l'esquena. Les orientacions 1 i 3 intercanvien
l'ample i el fons de l'empremta.
"""
import math

import numpy as np

from motor_iso import Llenc, gira, gira_normal

# Rampes de 5 tons (brillantor, llum, base, ombra, fosc): paleta càlida
# (Stardew) amb els materials d'Aegroum.
MATERIALS = {
    "pedra":     [(250, 238, 206), (232, 214, 176), (208, 186, 146), (171, 148, 112), (122, 103, 78)],
    "socol":     [(184, 170, 152), (150, 137, 122), (121, 109, 97), (92, 82, 74), (64, 56, 52)],
    "estuc":     [(255, 248, 226), (246, 234, 202), (228, 210, 172), (196, 174, 134), (150, 128, 96)],
    "estuc_ocre": [(255, 236, 190), (246, 214, 150), (226, 186, 116), (192, 150, 88), (146, 110, 64)],
    "estuc_blau": [(236, 244, 255), (212, 226, 246), (182, 200, 228), (146, 164, 196), (106, 120, 152)],
    "pedra_clara": [(255, 250, 236), (244, 236, 214), (226, 214, 186), (194, 178, 148), (146, 132, 108)],
    "pissarra":  [(150, 166, 196), (118, 134, 166), (92, 106, 138), (68, 80, 110), (48, 56, 80)],
    "encenall":  [(204, 170, 128), (174, 140, 100), (144, 112, 78), (112, 86, 60), (80, 62, 46)],
    "llauna":    [(214, 220, 226), (180, 188, 196), (148, 156, 166), (114, 122, 134), (82, 88, 100)],
    "tela":      [(255, 246, 222), (240, 226, 192), (218, 200, 160), (184, 164, 126), (140, 122, 92)],
    "tela_verm": [(255, 170, 150), (238, 116, 100), (204, 78, 70), (160, 52, 52), (116, 36, 40)],
    "boix":      [(150, 214, 110), (104, 178, 76), (72, 142, 58), (50, 108, 46), (34, 76, 36)],
    "estuc_rosa": [(255, 232, 222), (246, 206, 192), (226, 176, 160), (190, 140, 126), (144, 102, 92)],
    "noguera":   [(196, 128, 78), (163, 101, 58), (134, 79, 44), (101, 58, 33), (70, 40, 25)],
    "fusta":     [(232, 184, 120), (206, 154, 94), (176, 124, 72), (139, 95, 55), (99, 66, 40)],
    "teula":     [(255, 170, 120), (240, 128, 84), (214, 98, 62), (174, 72, 46), (128, 50, 34)],
    "verdet":    [(170, 236, 206), (120, 206, 170), (84, 170, 138), (58, 130, 106), (38, 92, 76)],
    "vidre":     [(236, 250, 255), (178, 228, 252), (120, 190, 236), (78, 140, 198), (52, 96, 150)],
    "cortina":   [(255, 150, 136), (236, 98, 92), (200, 64, 70), (156, 44, 58), (110, 30, 44)],
    "porticons": [(150, 212, 170), (104, 178, 132), (72, 144, 104), (50, 110, 82), (34, 78, 60)],
    "fulla":     [(196, 240, 120), (150, 214, 84), (104, 178, 62), (70, 138, 48), (44, 98, 38)],
    "llauto":    [(255, 240, 170), (246, 206, 104), (214, 164, 64), (166, 120, 44), (114, 80, 32)],
    "ferro":     [(150, 152, 168), (110, 112, 128), (80, 82, 98), (58, 58, 72), (40, 40, 52)],
    "cristall":  [(255, 255, 255), (220, 248, 255), (150, 222, 255), (90, 176, 236), (60, 120, 190)],
    "roba1":     [(255, 255, 250), (244, 240, 232), (220, 214, 204), (190, 182, 170), (150, 142, 132)],
    "roba2":     [(170, 210, 255), (120, 170, 236), (90, 136, 210), (66, 104, 170), (46, 74, 126)],
    "roba3":     [(255, 226, 140), (250, 200, 90), (226, 166, 60), (186, 128, 44), (136, 92, 32)],
    "flor1":     [(255, 200, 220), (255, 150, 186), (232, 104, 150), (190, 70, 116), (140, 50, 86)],
    "flor2":     [(255, 252, 200), (255, 236, 120), (240, 204, 70), (200, 160, 50), (150, 116, 36)],
    "terra":     [(196, 150, 110), (166, 120, 84), (136, 94, 64), (106, 72, 50), (76, 52, 38)],
    "foc":       [(255, 248, 200), (255, 214, 100), (250, 156, 56), (214, 92, 34), (150, 50, 24)],
    "aigua":     [(200, 240, 255), (130, 206, 236), (84, 168, 210), (56, 126, 176), (38, 88, 130)],
    "fum":       [(255, 255, 255), (240, 240, 244), (220, 222, 230), (196, 198, 210), (160, 162, 176)],
}


def _hash(u, v, s=0):
    h = (np.floor(u).astype(np.int64) * 73856093) ^ (np.floor(v).astype(np.int64) * 19349663) ^ (s * 83492791)
    return (h & 0xFFFF) / 65535.0


def textura(nom, u, v):
    """Desplaçament de to (+1 més fosc, -1 més clar) segons la textura."""
    u, v = np.asarray(u, float), np.asarray(v, float)
    t = np.zeros(u.shape, int)
    if nom == "carreus":                                 # carreus de pedra: fileres de 8, peces de 16
        fila = np.floor(v / 8)
        t += (np.mod(v, 8) < 1).astype(int)
        t += (np.mod(u + 8 * np.mod(fila, 2), 16) < 1).astype(int)
        t -= ((np.mod(v, 8) >= 7) & (t == 0)).astype(int)
    elif nom == "estuc":
        t += (_hash(u, v, 3) > 0.965).astype(int)
    elif nom == "taulons":                               # taulons horitzontals de 5
        t += (np.mod(v, 5) < 1).astype(int)
    elif nom == "taulons_v":
        t += (np.mod(u, 4) < 1).astype(int)
    elif nom == "teules":                                # fileres de 6 (per Z), teules de 6
        fila = np.floor(v / 6)
        f = np.mod(v, 6)
        t += (f < 1).astype(int)
        t += ((np.mod(u + 3 * np.mod(fila, 2), 6) < 1) & (f < 4)).astype(int)
        t -= ((f >= 5) & (t == 0)).astype(int)
        t += ((_hash(np.floor((u + 3 * np.mod(fila, 2)) / 6), fila, 7) > 0.86) & (f >= 1)).astype(int)
    elif nom == "llambordes":
        t += ((np.mod(u, 8) < 1) | (np.mod(v, 8) < 1)).astype(int)
    elif nom == "cercles":                               # bandes d'un barril
        t += (np.mod(v, 7) < 1).astype(int) * 2
    elif nom == "ratlles":                               # lona a ratlles
        t += (np.mod(np.floor(u / 6), 2) == 1).astype(int)
    elif nom == "ondulat":                               # llauna ondulada
        t += (np.mod(u, 3) < 1).astype(int)
    elif nom == "xarxa":                                 # xarxa de pesca
        t += ((np.mod(u + v, 4) < 1) | (np.mod(u - v, 4) < 1)).astype(int) * 2
    elif nom == "llenya":                                # testes de troncs apilats
        t += ((np.mod(u, 5) < 1) | (np.mod(v, 5) < 1)).astype(int)
    elif nom == "cercles_x":                             # INT-01: cèrcols d'una bota ajaguda (al llarg de u)
        t += (np.mod(u, 9) < 1.2).astype(int) * 2
    elif nom == "damasc":                                # INT-01: paper pintat (rombes de 16 amb una flor al mig)
        a, b = np.mod(u + v, 16), np.mod(u - v, 16)
        t += ((a < 1) | (b < 1)).astype(int)
        t -= ((np.abs(a - 8) < 1.6) & (np.abs(b - 8) < 1.6)).astype(int)
    elif nom == "rajoles":                               # INT-01: rajoles de 8, junta clara
        t -= ((np.mod(u, 8) < 1) | (np.mod(v, 8) < 1)).astype(int)
    elif nom == "maons":                                 # INT-01: maons de 8 x 4 a trencajunts
        fila = np.floor(v / 4)
        t += ((np.mod(v, 4) < 1) | (np.mod(u + 4 * np.mod(fila, 2), 8) < 1)).astype(int)
    elif nom == "plafons":                               # INT-01: arrimador de plafons de 16 amb motllura
        uu, vv = np.mod(u, 16), np.mod(v, 30)
        t += ((uu < 1) | (uu >= 15) | (vv < 2) | (vv >= 28)).astype(int)
        t -= (((uu >= 2) & (uu < 3)) | ((vv >= 3) & (vv < 4) & (uu >= 2) & (uu < 14))).astype(int)
    elif nom == "plecs":                                 # INT-01: plecs de cortina (bandes de 4)
        t += (np.mod(u, 4) >= 3).astype(int)
        t -= (np.mod(u, 4) < 1).astype(int)
    return t


def color_de(mat, tex, eix, to_fix=None, desp=0):
    """Funció de color per a una cara: eix diu quines coordenades són (u, v)."""
    rampa = np.array(MATERIALS[mat], np.uint8)

    def f(X, Y, Z, to):
        if eix == "x":
            u, v = Y, Z
        elif eix == "y":
            u, v = X, Z
        elif eix == "z":
            u, v = X, Y
        else:                                            # teulada: u al llarg del carener, v = alçada
            u, v = (X if eix == "tx" else Y), Z
        base = (to if to_fix is None else to_fix) + desp
        i = np.clip(base + textura(tex, u, v), 0, 4)
        out = np.empty((len(i), 4), np.uint8)
        out[:, :3] = rampa[i]
        out[:, 3] = 255
        return out
    return f


class Escena:
    """Elements d'una peça en coordenades canòniques; dibuixa(o) -> (imatge, àncora)."""

    def __init__(self, ample_cel, fons_cel):
        self.W, self.D = ample_cel * 32, fons_cel * 32
        self.cares = []                                  # (verts, normal, color, peca, biaix)
        self.sprites = []                                # (P, funcio(llenc, sx, sy, prof))
        self.sprites_post = []
        self._peca = 0
        self.careners = []                              # MASC-01: (x0, y0, x1, y1, z) on es posen els coloms
        self.evita_posador = []                         # (x, y, radi): cúpules i altres coses al carener

    def nova_peca(self):
        self._peca += 1
        return self._peca

    def cara(self, verts, normal, color, peca, biaix=0.0, doble=False, si_visible=None):
        """si_visible: normal d'una altra cara (el vessant); si aquesta queda d'esquena,
        aquesta tampoc es dibuixa (les vores del ràfec del vessant del darrere)."""
        self.cares.append((verts, normal, color, peca, biaix, doble, si_visible))

    # ---------- peces ----------
    def caixa(self, x0, y0, z0, x1, y1, z1, mat, tex="llis", dalt=None, tex_dalt=None, peca=None):
        p = peca or self.nova_peca()
        md = dalt or mat
        self.cara([(x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)], (0, 0, 1), color_de(md, tex_dalt or "llis", "z"), p)
        self.cara([(x1, y0, z0), (x1, y1, z0), (x1, y1, z1), (x1, y0, z1)], (1, 0, 0), color_de(mat, tex, "x"), p)
        self.cara([(x0, y1, z0), (x1, y1, z0), (x1, y1, z1), (x0, y1, z1)], (0, 1, 0), color_de(mat, tex, "y"), p)
        self.cara([(x0, y0, z0), (x0, y1, z0), (x0, y1, z1), (x0, y0, z1)], (-1, 0, 0), color_de(mat, tex, "x"), p)
        self.cara([(x0, y0, z0), (x1, y0, z0), (x1, y0, z1), (x0, y0, z1)], (0, -1, 0), color_de(mat, tex, "y"), p)
        return p

    def teulada(self, x0, y0, x1, y1, z0, alt, mat="teula", tex="teules", voladis=4, gruix=3,
                mat_timpa="estuc", tex_timpa="estuc", eix="x"):
        """Teulada a dues aigües amb el carener paral·lel a l'eix (x o y), ràfec i
        frontons (timpans) del material de la paret."""
        p = self.nova_peca()
        v = voladis
        if eix == "x":
            self.careners.append((x0 - v, (y0 + y1) / 2, x1 + v, (y0 + y1) / 2, z0 + alt))
        else:
            self.careners.append(((x0 + x1) / 2, y0 - v, (x0 + x1) / 2, y1 + v, z0 + alt))
        if eix == "x":
            ym = (y0 + y1) / 2
            xa, xb = x0 - v, x1 + v
            for ye, s in ((y1 + v, 1), (y0 - v, -1)):
                r = abs(ye - ym)
                self.cara([(xa, ye, z0), (xb, ye, z0), (xb, ym, z0 + alt), (xa, ym, z0 + alt)], (0, s * alt, r),
                          color_de(mat, tex, "tx"), p)
                self.cara([(xa, ye, z0 - gruix), (xb, ye, z0 - gruix), (xb, ye, z0), (xa, ye, z0)], (0, s, 0),
                          color_de(mat, "llis", "y", desp=1), p)
            for xx, s in ((x1, 1), (x0, -1)):                 # timpans
                self.cara([(xx, y0, z0), (xx, y1, z0), (xx, ym, z0 + alt)], (s, 0, 0), color_de(mat_timpa, tex_timpa, "x"), p, -0.5)
            for xx, s in ((xb, 1), (xa, -1)):                 # vores del ràfec als frontons
                for ye, sv in ((y1 + v, 1), (y0 - v, -1)):
                    self.cara([(xx, ye, z0 - gruix), (xx, ye, z0), (xx, ym, z0 + alt), (xx, ym, z0 + alt - gruix)],
                              (s, 0, 0), color_de(mat, "llis", "x", desp=1), p, si_visible=(0, sv * alt, abs(ye - ym)))
        else:
            xm = (x0 + x1) / 2
            ya, yb = y0 - v, y1 + v
            for xe, s in ((x1 + v, 1), (x0 - v, -1)):
                r = abs(xe - xm)
                self.cara([(xe, ya, z0), (xe, yb, z0), (xm, yb, z0 + alt), (xm, ya, z0 + alt)], (s * alt, 0, r),
                          color_de(mat, tex, "ty"), p)
                self.cara([(xe, ya, z0 - gruix), (xe, yb, z0 - gruix), (xe, yb, z0), (xe, ya, z0)], (s, 0, 0),
                          color_de(mat, "llis", "x", desp=1), p)
            for yy, s in ((y1, 1), (y0, -1)):
                self.cara([(x0, yy, z0), (x1, yy, z0), (xm, yy, z0 + alt)], (0, s, 0), color_de(mat_timpa, tex_timpa, "y"), p, -0.5)
            for yy, s in ((yb, 1), (ya, -1)):
                for xe, sv in ((x1 + v, 1), (x0 - v, -1)):
                    self.cara([(xe, yy, z0 - gruix), (xe, yy, z0), (xm, yy, z0 + alt), (xm, yy, z0 + alt - gruix)],
                              (0, s, 0), color_de(mat, "llis", "y", desp=1), p, si_visible=(sv * alt, 0, abs(xe - xm)))
        return p

    def un_vessant(self, x0, y0, x1, y1, z_baix, z_alt, mat="llauna", tex="ondulat", voladis=3, gruix=2,
                   mat_paret="fusta", tex_paret="taulons"):
        """Teulada d'un sol vessant (de la part del darrere, alta, cap a la façana +Y,
        baixa) i els dos trapezis de paret dels costats."""
        p = self.nova_peca()
        v = voladis
        xa, xb, ya, yb = x0 - v, x1 + v, y0 - v, y1 + v
        self.careners.append((xa, ya + 1, xb, ya + 1, z_alt))      # la vora alta, la del darrere
        r, run = z_alt - z_baix, yb - ya
        self.cara([(xa, yb, z_baix), (xb, yb, z_baix), (xb, ya, z_alt), (xa, ya, z_alt)], (0, r, run),
                  color_de(mat, tex, "tx"), p)
        self.cara([(xa, yb, z_baix - gruix), (xb, yb, z_baix - gruix), (xb, yb, z_baix), (xa, yb, z_baix)], (0, 1, 0),
                  color_de(mat, "llis", "y", desp=1), p)
        for xx, s in ((xb, 1), (xa, -1)):
            self.cara([(xx, yb, z_baix - gruix), (xx, yb, z_baix), (xx, ya, z_alt), (xx, ya, z_alt - gruix)], (s, 0, 0),
                      color_de(mat, "llis", "x", desp=1), p)
        zb = z_baix
        for xx, s in ((x1, 1), (x0, -1)):
            self.cara([(xx, y0, zb), (xx, y1, zb), (xx, y1, zb + r * (yb - y1) / run), (xx, y0, zb + r * (yb - y0) / run)],
                      (s, 0, 0), color_de(mat_paret, tex_paret, "x"), p, -0.5)
        self.cara([(x0, y0, zb), (x1, y0, zb), (x1, y0, zb + r * (yb - y0) / run), (x0, y0, zb + r * (yb - y0) / run)],
                  (0, -1, 0), color_de(mat_paret, tex_paret, "y"), p, -0.5)
        return p

    def cilindre(self, cx, cy, z0, z1, radi, mat, tex="llis", costats=12, dalt=None):
        p = self.nova_peca()
        punts = [(cx + radi * math.cos(2 * math.pi * k / costats), cy + radi * math.sin(2 * math.pi * k / costats))
                 for k in range(costats)]
        for k in range(costats):
            (xa, ya), (xb, yb) = punts[k], punts[(k + 1) % costats]
            n = ((xa + xb) / 2 - cx, (ya + yb) / 2 - cy, 0)            # radial cap enfora
            self.cara([(xa, ya, z0), (xb, yb, z0), (xb, yb, z1), (xa, ya, z1)], n, color_de(mat, tex, "y"), p)
        self.cara([(x, y, z1) for x, y in punts], (0, 0, 1), color_de(dalt or mat, "llis", "z"), p)
        return p

    def rect(self, eix, c, signe, u0, u1, v0, v1, mat, tex="llis", to=None, desp=0, biaix=0.6, peca=None, arc=False):
        """Rectangle (o porta d'arc) sobre el pla X = c (eix 'x') o Y = c (eix 'y')."""
        if arc:
            r = (u1 - u0) / 2
            uc = (u0 + u1) / 2
            vs = v1 - r
            uv = [(u0, v0), (u1, v0), (u1, vs)]
            for k in range(1, 8):
                a = math.pi * k / 8
                uv.append((uc + r * math.cos(a), vs + r * math.sin(a) * 0.9))
            uv.append((u0, vs))
        else:
            uv = [(u0, v0), (u1, v0), (u1, v1), (u0, v1)]
        if eix == "x":
            verts = [(c, u, v) for u, v in uv]
            n = (signe, 0, 0)
        else:
            verts = [(u, c, v) for u, v in uv]
            n = (0, signe, 0)
        self.cara(verts, n, color_de(mat, tex, eix, to, desp), peca or self.nova_peca(), biaix)

    def poligon_cara(self, eix, c, signe, uv, mat, tex="llis", to=None, desp=0, biaix=0.6, peca=None):
        verts = [(c, u, v) for u, v in uv] if eix == "x" else [(u, c, v) for u, v in uv]
        n = (signe, 0, 0) if eix == "x" else (0, signe, 0)
        self.cara(verts, n, color_de(mat, tex, eix, to, desp), peca or self.nova_peca(), biaix)

    def sprite(self, P, funcio, despres=False):
        """Dibuix en pantalla ancorat a un punt 3D; despres=True: després dels contorns (fum)."""
        (self.sprites_post if despres else self.sprites).append((P, funcio))

    # ---------- l'ombra al terra (INT-03) ----------
    def ombra(self, o, marge=6, alt_max=240, alfa=100, desp=(2, 1), creix=1):
        """L'ombra de la peça sobre el terra, al mateix llenç que dibuixa(o), per posar-la a
        sota del dibuix: la planta (la projecció vertical de les cares sobre Z = 0) i, per als
        dibuixos sense cares (capçades, mates, joncs), una el·lipse de l'ample del dibuix.
        Tot plegat creix un píxel, es desplaça cap a la dreta (la llum ve de l'esquerra, com
        a to_llum) i es retalla a l'empremta: no cau mai sobre les cel·les del darrere (que es
        dibuixen abans) ni passa del vèrtex de davant. Les peces planes (estores, runes,
        flors) no en fan. Abans, el motor posava una taca rodona a la punta de davant del
        rombe, separada de la base dels mobles: semblava que suressin."""
        from PIL import Image, ImageDraw, ImageFilter
        Wv, Dv = (self.W, self.D) if o % 2 == 0 else (self.D, self.W)
        ample = Wv + Dv + 2 * marge
        alt = alt_max + (Wv + Dv) // 2 + 2 * marge
        ox, oy = Dv + marge, alt_max + marge
        terra = lambda X, Y: (X - Y + ox, (X + Y) / 2.0 + oy)     # noqa: E731
        mascara = Image.new("L", (ample, alt), 0)
        d = ImageDraw.Draw(mascara)
        for verts, *_ in self.cares:
            vv = [gira(P, o, self.W, self.D) for P in verts]
            if max(Z for _, _, Z in vv) < 3:                     # a ran de terra: no fa ombra
                continue
            xy = [(X, Y) for X, Y, _ in vv]
            area = 0.5 * abs(sum(xa * yb - xb * ya for (xa, ya), (xb, yb) in zip(xy, xy[1:] + xy[:1])))
            if area >= 1.0:
                d.polygon([terra(X, Y) for X, Y in xy], fill=255)
        for P, funcio in self.sprites:
            X, Y, Z = gira(P, o, self.W, self.D)
            prova = Llenc(ample, alt, ox, oy)
            sx, sy = prova.proj((X, Y, Z))
            funcio(prova, int(round(sx)), int(round(sy)), X + Y + Z)
            caixa = Image.fromarray(prova.col, "RGBA").getbbox()
            cx, cy = terra(X, Y)
            if caixa is None or cy - caixa[1] < 8:               # buit o pla (flors al terra)
                continue
            r = (caixa[2] - caixa[0]) / 2.0 * getattr(self, "ombra_capcada", 0.8)   # l'arbre sec: branques, poca ombra
            r = min(r, 21.0)                                     # que hi càpiga dins del rombe (sense cantonades)
            d.ellipse([cx - r, cy - r / 2.0, cx + r, cy + r / 2.0], fill=255)
        if mascara.getbbox() is None:
            return Image.new("RGBA", (ample, alt), (0, 0, 0, 0))
        if creix:
            mascara = mascara.filter(ImageFilter.MaxFilter(2 * creix + 1))
        moguda = Image.new("L", (ample, alt), 0)
        moguda.paste(mascara, desp)
        retall = Image.new("L", (ample, alt), 0)
        ImageDraw.Draw(retall).polygon([terra(0, 0), terra(Wv, 0), terra(Wv, Dv), terra(0, Dv)], fill=255)
        m = (np.array(moguda) > 0) & (np.array(retall) > 0)
        m[(Wv + Dv) // 2 + alt_max + marge:, :] = False           # res sota el vèrtex de davant (l'àncora)
        m[:, :ox - Dv] = False                                     # ni fora del rombe pels costats
        m[:, ox + Wv:] = False
        out = np.zeros((alt, ample, 4), np.uint8)
        out[m] = (43, 29, 24, alfa)
        return Image.fromarray(out, "RGBA")

    # ---------- dibuix ----------
    def dibuixa(self, o, marge=6, alt_max=240):
        """junta (atribut, INT-01): les parets continuen per un costat (el de la
        cel·la anterior); per aquell costat la peça no porta contorn."""
        Wv, Dv = (self.W, self.D) if o % 2 == 0 else (self.D, self.W)
        ample = Wv + Dv + 2 * marge
        alt = alt_max + (Wv + Dv) // 2 + 2 * marge
        ll = Llenc(ample, alt, Dv + marge, alt_max + marge)
        for verts, normal, color, peca, biaix, doble, si_visible in self.cares:
            if si_visible is not None and sum(gira_normal(si_visible, o)) <= 1e-9:
                continue
            vv = [gira(P, o, self.W, self.D) for P in verts]
            nn = gira_normal(normal, o)
            if doble and nn[0] + nn[1] + nn[2] < 0:
                nn = (-nn[0], -nn[1], -nn[2])
            ll.poligon(vv, nn, color, peca, biaix, o, self.W, self.D)
        for P, funcio in self.sprites:
            X, Y, Z = gira(P, o, self.W, self.D)
            sx, sy = ll.proj((X, Y, Z))
            funcio(ll, int(round(sx)), int(round(sy)), X + Y + Z)
        # INT-01: veïns (els altres trams d'un moble llarg) dibuixats només perquè
        # el contorn no talli la juntura; després s'esborren.
        veins = getattr(self, "veins", ())
        for fer_vei, dx, dy in veins:
            v = fer_vei()
            for verts, normal, color, peca, biaix, doble, si_visible in v.cares:
                if si_visible is not None and sum(gira_normal(si_visible, o)) <= 1e-9:
                    continue
                vv = [gira((X + dx * 32, Y + dy * 32, Z), o, self.W, self.D) for X, Y, Z in verts]
                nn = gira_normal(normal, o)
                if doble and nn[0] + nn[1] + nn[2] < 0:
                    nn = (-nn[0], -nn[1], -nn[2])
                ll.poligon(vv, nn, color, peca + 100000, biaix, o, self.W, self.D)
        junta = None
        if getattr(self, "junta", False):
            ax = Wv + marge
            junta = ("e", ax - Dv + 1) if o == 0 else (("d", ax + Wv - 1) if o == 3 else None)
        ll.contorns(junta=junta)
        if veins:
            ll.col[ll.peca >= 100000] = 0
        for P, funcio in self.sprites_post:
            X, Y, Z = gira(P, o, self.W, self.D)
            sx, sy = ll.proj((X, Y, Z))
            funcio(ll, int(round(sx)), int(round(sy)), X + Y + Z)
        ancora = (Wv + marge, (Wv + Dv) // 2 + alt_max + marge)
        return ll.imatge(), ancora


def mata(radi=7, alt=None, flors=("flor1", "flor2"), llavor=1, peca_id=900):
    """Plantes com a sprite (les veus igual des de tots els costats): una mata
    arrodonida de fulla amb flors, contornejada."""
    alt = alt or radi

    def f(ll, sx, sy, prof):
        rng = np.random.default_rng(abs(int(llavor)) % (2 ** 32))
        fulla = np.array(MATERIALS["boix" if "boix" in flors else "fulla"], np.uint8)
        y0 = sy - 2 * alt
        for yy in range(y0 - 1, sy + 1):
            for xx in range(sx - radi - 1, sx + radi + 2):
                if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                    continue
                dx, dy = (xx - sx) / radi, (yy - (sy - alt)) / alt
                d = dx * dx + dy * dy + 0.18 * math.sin(xx * 1.7 + yy * 0.9)
                if d > 1.0:
                    continue
                prof_p = prof + 2 + (1 - d) * 2
                if prof_p <= ll.prof[yy, xx]:
                    continue
                to = 1 if (dx < -0.2 and dy < -0.1) else (3 if dx > 0.35 or dy > 0.45 else 2)
                if rng.random() < 0.16:
                    to = min(4, to + 1)
                c = fulla[to]
                if rng.random() < 0.09 and dy < 0.3 and "boix" not in flors:
                    c = np.array(MATERIALS[flors[rng.integers(len(flors))]][1], np.uint8)
                ll.col[yy, xx] = (*c, 255)
                ll.prof[yy, xx] = prof_p
                ll.peca[yy, xx] = peca_id + abs(int(llavor)) % 1000
    return f


def fum(llavor=1, bufs=3):
    """Fum de xemeneia (Stardew): bufs arrodonits que pugen en diagonal, sense contorn."""
    def f(ll, sx, sy, prof):
        rng = np.random.default_rng(abs(int(llavor)) % (2 ** 32))
        rampa = np.array(MATERIALS["fum"], np.uint8)
        for k in range(bufs):
            r = 3 + k
            cx, cy = sx + 2 + k * 4 + int(rng.integers(-1, 2)), sy - 4 - k * 7
            for yy in range(cy - r, cy + r + 1):
                for xx in range(cx - r, cx + r + 1):
                    if not (0 <= xx < ll.w and 0 <= yy < ll.h):
                        continue
                    d = ((xx - cx) ** 2 + (yy - cy) ** 2) / (r * r)
                    if d > 1:
                        continue
                    to = 1 if (xx - cx) + (yy - cy) < -r * 0.4 else (3 if d > 0.6 and (xx - cx) > 0 else 2)
                    ll.col[yy, xx] = (*rampa[min(4, to + k // 2)], 255)
    return f
