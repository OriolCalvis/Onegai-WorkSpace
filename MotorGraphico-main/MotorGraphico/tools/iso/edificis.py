#!/usr/bin/env python3
"""ISO — Edificis paramètrics de Boundington (estil Habbo + Stardew): una casa
o un local es descriu amb un estil (ESTILS: popular, noble, cabana, caserna)
i uns quants paràmetres, i surt en les 4 orientacions amb la mateixa llum,
les mateixes línies i els mateixos materials.

    from edificis import casa, local
    e = casa(2, 2, estil="popular", llavor=1)     # Escena; e.dibuixa(o) -> (imatge, àncora)
    e = local(5, 4, "ferreria", llavor=4)

La façana principal (la porta) és la cara +Y; davant hi ha el pati (fons
`pati` unitats) amb els decorats. El carener va al llarg del costat més llarg.
"""
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from oficis import OFICIS, xemeneia_fum  # noqa: E402
from peces_iso import Escena, color_de, mata  # noqa: E402
import patis  # noqa: E402

ESTILS = {
    # Casc Antic i Districte Comercial: pedra a baix, estuc i entramat a dalt, teula.
    "popular": dict(mur="pedra", tex_mur="carreus", alta="estuc", entramat=True, volat=2, teula="teula",
                    porta="arc", finestra="porticons", jardineres=True, roba=True, cornisa=False, balcons=False,
                    fanal="guardians", pati_decor="casa", alt_pb=50, alt_pa=28, plantes=2, cantonades=False, lucanes=True),
    # Barrios Altos: pedra polida, pissarra, balcons amb barana de llautó, dos fanals.
    "noble": dict(mur="pedra_clara", tex_mur="carreus", alta="pedra_clara", entramat=False, volat=0, teula="pissarra",
                  porta="doble", finestra="altes", jardineres=True, roba=False, cornisa=True, balcons=True,
                  fanal="doble", pati_decor="jardi", alt_pb=54, alt_pa=34, plantes=2, cantonades=True, lucanes=True),
    # Pico Dragón: taulons de fusta crua, teulada d'encenall, xarxes i llenya.
    "cabana": dict(mur="fusta", tex_mur="taulons", alta="fusta", entramat=False, volat=0, teula="encenall",
                   porta="taulons", finestra="simple", jardineres=False, roba=True, cornisa=False, balcons=False,
                   fanal="cap", pati_decor="pescador", alt_pb=44, alt_pa=24, plantes=1, cantonades=False, lucanes=False),
    # Barrio Militar: barracons de pedra en filera, pissarra, armes al pati.
    "caserna": dict(mur="pedra", tex_mur="carreus", alta="pedra", entramat=False, volat=0, teula="pissarra",
                    porta="taulons", finestra="simple", jardineres=False, roba=False, cornisa=True, balcons=False,
                    fanal="guardians", pati_decor="armes", alt_pb=48, alt_pa=26, plantes=1, cantonades=True, lucanes=False),
}
TEX = {"estuc": "estuc", "estuc_ocre": "estuc", "estuc_rosa": "estuc", "estuc_blau": "estuc",
       "pedra": "carreus", "pedra_clara": "carreus", "socol": "carreus", "fusta": "taulons", "noguera": "taulons"}


def finestra(e, eix, c, s, u0, u1, v0, v1, estil="porticons", jardinera=True, llavor=1, mat_p="porticons", marc="noguera"):
    """Finestra sobre la paret: 'porticons' (amb porticons), 'altes' (marc de llautó), 'simple', 'forat'."""
    if estil == "forat":
        e.rect(eix, c, s, u0, u1, v0, v1, "ferro", to=4)
        e.rect(eix, c, s, u0, u1, v1 - 3, v1, mat_p, biaix=0.8)
        return
    e.rect(eix, c, s, u0 - 1, u1 + 1, v0 - 1, v1 + 1, "llauto" if estil == "altes" else marc, to=3 if estil == "altes" else 4)
    e.rect(eix, c, s, u0, u1, v0, v1, "vidre", biaix=0.8)
    um, vm = (u0 + u1) / 2, (v0 + v1) / 2
    e.rect(eix, c, s, um - 1, um + 1, v0, v1, marc, biaix=1.0)
    e.rect(eix, c, s, u0, u1, vm - 1, vm + 1, marc, biaix=1.0)
    e.rect(eix, c, s, u0 + 1, u0 + 3, v0 + 2, v1 - 2, "vidre", to=0, biaix=1.1)
    if estil == "porticons":
        w = (u1 - u0) / 2 + 1
        e.rect(eix, c, s, u0 - 1 - w, u0 - 1, v0 - 1, v1 + 1, mat_p, "taulons_v")
        e.rect(eix, c, s, u1 + 1, u1 + 1 + w, v0 - 1, v1 + 1, mat_p, "taulons_v")
    if estil == "altes":
        e.rect(eix, c, s, u0 - 2, u1 + 2, v1 + 1, v1 + 4, "pedra_clara", to=1, biaix=1.2)   # llinda
    if jardinera:
        ample = 4
        if eix == "x":
            x0, x1 = (c, c + ample) if s > 0 else (c - ample, c)
            e.caixa(x0, u0 - 1, v0 - 6, x1, u1 + 1, v0 - 1, "fusta", "taulons")
            for k, u in enumerate(range(int(u0) + 2, int(u1), 5)):
                e.sprite(((x0 + x1) / 2, u, v0 - 1), mata(3, 3, llavor=llavor + k))
        else:
            y0, y1 = (c, c + ample) if s > 0 else (c - ample, c)
            e.caixa(u0 - 1, y0, v0 - 6, u1 + 1, y1, v0 - 1, "fusta", "taulons")
            for k, u in enumerate(range(int(u0) + 2, int(u1), 5)):
                e.sprite((u, (y0 + y1) / 2, v0 - 1), mata(3, 3, llavor=llavor + k))


def balco(e, eix, c, s, u0, u1, z):
    """Balcó de pedra amb barana de llautó davant d'una finestra de la planta alta."""
    if eix == "y":
        y0, y1 = (c, c + 6) if s > 0 else (c - 6, c)
        e.caixa(u0 - 3, y0, z - 3, u1 + 3, y1, z, "pedra_clara")
        yb = y1 - 1 if s > 0 else y0
        e.caixa(u0 - 3, yb, z + 9, u1 + 3, yb + 1, z + 10, "llauto")
        for u in range(int(u0) - 3, int(u1) + 4, 3):
            e.caixa(u, yb, z, u + 1, yb + 1, z + 9, "llauto")
    else:
        x0, x1 = (c, c + 6) if s > 0 else (c - 6, c)
        e.caixa(x0, u0 - 3, z - 3, x1, u1 + 3, z, "pedra_clara")
        xb = x1 - 1 if s > 0 else x0
        e.caixa(xb, u0 - 3, z + 9, xb + 1, u1 + 3, z + 10, "llauto")
        for u in range(int(u0) - 3, int(u1) + 4, 3):
            e.caixa(xb, u, z, xb + 1, u + 1, z + 9, "llauto")


def entramat(e, eix, c, s, u0, u1, v0, v1):
    e.rect(eix, c, s, u0, u1, v0, v0 + 3, "noguera")
    e.rect(eix, c, s, u0, u1, v1 - 3, v1, "noguera")
    trams = max(1, round((u1 - u0) / 30))
    pals = [u0 + (u1 - u0 - 3) * k / trams for k in range(trams + 1)]
    for u in pals:
        e.rect(eix, c, s, u, u + 3, v0, v1, "noguera")
    for k in range(trams):
        a, b = pals[k] + 3, pals[k + 1]
        if k % 2:
            a, b = b, a
        d = 3 if b > a else -3
        e.poligon_cara(eix, c, s, [(a, v0 + 3), (a + d, v0 + 3), (b, v1 - 3), (b - d, v1 - 3)], "noguera")


def centres(u0, u1, pas=24, evita=None, marge=18):
    n = max(1, int((u1 - u0 - 8) // pas))
    cs = [u0 + (u1 - u0) * (k + 0.5) / n for k in range(n)]
    return [c for c in cs if evita is None or all(abs(c - x) > marge for x in (evita if isinstance(evita, (list, tuple)) else [evita]))]


def cares(x0, y0, x1, y1):
    """Les quatre façanes: (eix, coordenada, signe, u0, u1)."""
    return (("y", y1, 1, x0, x1), ("x", x1, 1, y0, y1), ("y", y0, -1, x0, x1), ("x", x0, -1, y0, y1))


def porta(e, est, pc, by1, z_pb, alt=44):
    if est == "arc":
        e.rect("y", by1, 1, pc - 10, pc + 10, z_pb, z_pb + alt, "noguera", to=4, arc=True)
        e.rect("y", by1, 1, pc - 8, pc + 8, z_pb, z_pb + alt - 2, "cortina", "taulons_v", arc=True, biaix=0.8)
        e.rect("y", by1, 1, pc - 8, pc + 8, z_pb + 3, z_pb + 5, "llauto", biaix=1.0)
    elif est == "doble":
        e.rect("y", by1, 1, pc - 14, pc + 14, z_pb, z_pb + alt + 4, "pedra_clara", to=1, arc=True)
        e.rect("y", by1, 1, pc - 11, pc + 11, z_pb, z_pb + alt, "noguera", "taulons_v", arc=True, biaix=0.8)
        e.rect("y", by1, 1, pc - 0.5, pc + 0.5, z_pb, z_pb + alt - 6, "noguera", to=4, biaix=1.0)
        for du in (-3, 2):
            e.rect("y", by1, 1, pc + du, pc + du + 1, z_pb + 18, z_pb + 21, "llauto", to=0, biaix=1.1)
    elif est == "taulons":
        e.rect("y", by1, 1, pc - 9, pc + 9, z_pb, z_pb + alt - 2, "noguera", to=4)
        e.rect("y", by1, 1, pc - 7, pc + 7, z_pb, z_pb + alt - 4, "noguera", "taulons_v", biaix=0.8)
        e.rect("y", by1, 1, pc + 4, pc + 5, z_pb + 18, z_pb + 21, "ferro", biaix=1.0)
    if est != "forja":
        e.caixa(pc - 12, by1, 0, pc + 12, by1 + 5, z_pb, "socol", dalt="socol")


def fanal(e, x, by1, z_pb):
    e.caixa(x, by1, z_pb + 26, x + 2, by1 + 3, z_pb + 28, "ferro")
    e.caixa(x, by1 + 2, z_pb + 18, x + 5, by1 + 6, z_pb + 26, "cristall", dalt="llauto")
    e.caixa(x - 1, by1 + 1, z_pb + 26, x + 6, by1 + 7, z_pb + 28, "llauto")


def lucanes(e, p, eix, r, z_t, mig, mur, evita=(), volad=4):
    """Lucanes (finestres de teulada amb teuladeta) als dos vessants d'una teulada
    llarga (4 cel·les o més al llarg del carener), lluny de la xemeneia i de les cúpules
    (evita: parells (u al llarg del carener, vessant +1/-1, o 0 per a tots dos))."""
    rx0, ry0, rx1, ry1 = r
    llarg = (rx1 - rx0) if eix == "x" else (ry1 - ry0)
    if llarg < 110:
        return
    n = round(llarg / 72)
    a0 = rx0 if eix == "x" else ry0
    mig_t = (ry0 + ry1) / 2 if eix == "x" else (rx0 + rx1) / 2       # el carener, en la coordenada transversal
    H = ((ry1 - ry0) if eix == "x" else (rx1 - rx0)) / 2
    run, ad, tm = H + volad, 9, TEX.get(mur, "estuc")
    df = H - 6                                                     # la façana de la lucana, 6 per dins de la paret
    zb = z_t + mig * (1 - df / run)
    zt = zb + 18
    if zt + ad > z_t + mig - 4:
        return
    db, db2 = run * (1 - (zt - z_t) / mig), run * (1 - (zt + ad - z_t) / mig)
    est = "altes" if p["finestra"] == "altes" else "simple"
    for k in range(n):
        uc = a0 + llarg * (k + 0.5) / n
        for s in (1, -1):
            if any(abs(uc - x) < 24 and sv in (0, s) for x, sv in evita):
                continue
            wf, wb, wb2 = mig_t + s * df, mig_t + s * db, mig_t + s * db2
            if eix == "x":
                e.caixa(uc - 9, min(wf, wb), zb - 2, uc + 9, max(wf, wb), zt, mur, tm)
                e.teulada(uc - 9, min(wf, wb2), uc + 9, max(wf, wb2), zt, ad, mat=p["teula"], voladis=2, gruix=2,
                          mat_timpa=mur, tex_timpa=tm, eix="y")
                finestra(e, "y", wf, s, uc - 4, uc + 4, zb + 2, zb + 13, est, jardinera=False)
            else:
                e.caixa(min(wf, wb), uc - 9, zb - 2, max(wf, wb), uc + 9, zt, mur, tm)
                e.teulada(min(wf, wb2), uc - 9, max(wf, wb2), uc + 9, zt, ad, mat=p["teula"], voladis=2, gruix=2,
                          mat_timpa=mur, tex_timpa=tm, eix="x")
                finestra(e, "x", wf, s, uc - 4, uc + 4, zb + 2, zb + 13, est, jardinera=False)


def casa(ample, fons, estil="popular", pati=12, porta_u=None, llavor=0, ofici=None, porticons="porticons", **kw):
    """Una casa de l'estil donat (ESTILS) o un local (ofici: oficis.py). kw sobreescriu l'estil."""
    p = dict(ESTILS[estil])
    p.update(kw)
    rnd = random.Random(llavor)
    e = Escena(ample, fons)
    W, D = ample * 32, fons * 32
    bx0, by0, bx1, by1 = 4, 4, W - 4, D - pati
    z_pb = 5
    z_pa = z_pb + p["alt_pb"]
    plantes = p["plantes"]
    z_t = z_pa + p["alt_pa"] * (plantes - 1)
    v = p["volat"]
    e.cara([(2, by1, 0), (W - 2, by1, 0), (W - 2, D, 0), (2, D, 0)], (0, 0, 1), color_de("socol", "llambordes", "z", desp=-1), e.nova_peca())
    e.caixa(2, 2, 0, W - 2, by1 + 2, z_pb, "socol", "carreus", tex_dalt="llambordes")
    e.caixa(bx0, by0, z_pb, bx1, by1, z_pa, p["mur"], p["tex_mur"])
    if p["cantonades"]:                                          # cantoneres de pedra clara
        for (eix, c, s, u0, u1) in cares(bx0, by0, bx1, by1):
            for u in (u0, u1 - 4):
                for z in range(z_pb, z_pa - 4, 8):
                    e.rect(eix, c, s, u, u + 4, z, z + 6, "pedra_clara", to=1, biaix=0.5)
    for k in range(1, plantes):
        z0, z1 = z_pa + p["alt_pa"] * (k - 1), z_pa + p["alt_pa"] * k
        e.caixa(bx0 - v, by0 - v, z0, bx1 + v, by1 + v, z1, p["alta"], TEX.get(p["alta"], "estuc"))
        if p["entramat"]:
            for (eix, c, s, u0, u1) in cares(bx0 - v, by0 - v, bx1 + v, by1 + v):
                entramat(e, eix, c, s, u0, u1, z0, z1)
    rx0, ry0, rx1, ry1 = (bx0 - v, by0 - v, bx1 + v, by1 + v) if plantes > 1 else (bx0, by0, bx1, by1)
    if p["cornisa"]:
        e.caixa(rx0 - 2, ry0 - 2, z_pa - 3, rx1 + 2, ry1 + 2, z_pa, "pedra_clara", dalt="pedra_clara")
        if plantes > 1:
            e.caixa(rx0 - 2, ry0 - 2, z_t - 3, rx1 + 2, ry1 + 2, z_t, "pedra_clara", dalt="pedra_clara")
    eix = "x" if (rx1 - rx0) >= (ry1 - ry0) else "y"
    mig = (ry1 - ry0) / 2 + 4 if eix == "x" else (rx1 - rx0) / 2 + 4
    timpa = p["alta"] if plantes > 1 else p["mur"]
    e.teulada(rx0, ry0, rx1, ry1, z_t, mig, mat=p["teula"], tex="teules", eix=eix, mat_timpa=timpa,
              tex_timpa=TEX.get(timpa, "estuc"))
    cx = rnd.choice([rx0 + 10, rx1 - 20])                       # xemeneia amb caputxa de vent
    e.caixa(cx, ry0 + 6, z_t + 8, cx + 10, ry0 + 16, z_t + mig + 14, "pedra", "carreus")
    e.caixa(cx - 2, ry0 + 4, z_t + mig + 14, cx + 12, ry0 + 18, z_t + mig + 19, "verdet")
    e.caixa(cx + 4, ry0 + 10, z_t + mig + 19, cx + 6, ry0 + 12, z_t + mig + 28, "llauto")
    if rnd.random() < 0.7 or ofici:
        xemeneia_fum(e, cx + 5, ry0 + 11, z_t + mig + 20, llavor)
    if p["lucanes"]:
        evita = [(cx + 5, -1) if eix == "x" else (ry0 + 11, -1 if cx < (rx0 + rx1) / 2 else 1)]   # (u, vessant; 0 = tots dos)
        if p.get("cupula"):
            evita.append(((bx0 + bx1) / 2 if eix == "x" else (by0 + by1) / 2, 0))
        lucanes(e, p, eix, (rx0, ry0, rx1, ry1), z_t, mig, timpa, evita)
    if eix == "x":                                              # cavalló
        e.caixa(rx0 - 4, (ry0 + ry1) / 2 - 1.5, z_t + mig - 1, rx1 + 4, (ry0 + ry1) / 2 + 1.5, z_t + mig + 2, p["teula"], dalt=p["teula"])
    else:
        e.caixa((rx0 + rx1) / 2 - 1.5, ry0 - 4, z_t + mig - 1, (rx0 + rx1) / 2 + 1.5, ry1 + 4, z_t + mig + 2, p["teula"], dalt=p["teula"])
    pc = porta_u if porta_u is not None else (bx0 + bx1) / 2 + rnd.choice([-10, 0, 10])
    if ofici and porta_u is None:
        pc = bx1 - 26 if ofici != "ferreria" else (bx0 + bx1) / 2
    porta(e, "forja" if ofici == "ferreria" else p["porta"], pc, by1, z_pb)
    if p["fanal"] == "guardians":
        fanal(e, pc + 13 if ofici != "ferreria" else pc + 18, by1, z_pb)
    elif p["fanal"] == "doble":
        fanal(e, pc + 16, by1, z_pb)
        fanal(e, pc - 21, by1, z_pb)
    vb0, vb1 = z_pb + 14, z_pb + (36 if p["finestra"] == "altes" else 32)
    k = 0
    for (eix_c, c, s, u0, u1) in cares(bx0, by0, bx1, by1):
        if ofici and eix_c == "y" and s > 0:
            continue
        evita = [pc, pc - 21, pc + 16] if (eix_c == "y" and s > 0) else None
        for u in centres(u0, u1, evita=evita):
            k += 1
            est = p["finestra"] if p["finestra"] != "porticons" or rnd.random() < 0.6 else "simple"
            finestra(e, eix_c, c, s, u - 6, u + 6, vb0, vb1, est, jardinera=p["jardineres"] and s > 0,
                     llavor=llavor + k, mat_p=porticons)
    for planta in range(1, plantes):
        z0 = z_pa + p["alt_pa"] * (planta - 1)
        for (eix_c, c, s, u0, u1) in cares(bx0 - v, by0 - v, bx1 + v, by1 + v):
            for u in centres(u0 + v, u1 - v):
                k += 1
                est = p["finestra"] if p["finestra"] != "porticons" or rnd.random() < 0.5 else "simple"
                finestra(e, eix_c, c, s, u - 6, u + 6, z0 + 7, z0 + p["alt_pa"] - 7, est,
                         jardinera=p["jardineres"] and s > 0 and not p["balcons"] and rnd.random() < 0.5,
                         llavor=llavor + k, mat_p=porticons)
                if p["balcons"] and s > 0:
                    balco(e, eix_c, c, s, u - 6, u + 6, z0 + 6)
    e.rect("y", by0, -1, (bx0 + bx1) / 2 - 7, (bx0 + bx1) / 2 + 7, z_pb, z_pb + 34, "noguera", "taulons_v")
    if p["roba"]:                                               # roba estesa a la cara +X
        zr = (z_pa if plantes > 1 else z_pb + p["alt_pb"]) - 4
        e.rect("x", bx1 + 1, 1, by0 + 2, by1 - 2, zr, zr + 1, "ferro", biaix=1.4)
        for j, u in enumerate(range(int(by0) + 6, int(by1) - 6, 8)):
            e.rect("x", bx1 + 1, 1, u, u + 6, zr - 8 + (j % 2) * 2, zr, rnd.choice(["roba1", "roba2", "roba3"]), biaix=1.5)
    ctx = dict(pc=pc, by1=by1, by0=by0, z_pb=z_pb, z_pa=z_pa, z_t=z_t, mig=mig, eix=eix, vb0=vb0, vb1=vb1, bx0=bx0,
               bx1=bx1, yp=(by1 + D) / 2 + 1, W=W, D=D, aparador=(bx0 + 8, pc - 16), llavor=llavor, rnd=rnd,
               rx0=rx0, ry0=ry0, rx1=rx1, ry1=ry1)
    if ofici:
        OFICIS[ofici](e, ctx)
    else:
        patis.PATIS[p["pati_decor"]](e, ctx)
    return e


def local(ample, fons, ofici="taverna", llavor=0, **kw):
    """Un local: l'estil de l'ofici (oficis.ESTIL_OFICI) + el que digui kw."""
    from oficis import ESTIL_OFICI
    p = dict(ESTIL_OFICI.get(ofici, {}))
    estil = p.pop("estil", "popular")
    p.update(kw)
    return casa(ample, fons, estil=estil, ofici=ofici, llavor=llavor, **p)
