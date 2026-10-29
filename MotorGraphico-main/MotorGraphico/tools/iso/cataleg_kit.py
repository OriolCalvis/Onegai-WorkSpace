#!/usr/bin/env python3
"""ISO — El catàleg del kit (només dades, sense dibuix): quines famílies de
cases i quins oficis hi ha, a quines mides (canòniques, cel·les ample x fons),
amb quantes variants, i com es diuen les rajoles. El fan servir
tools/iso/exportar_kit.py (per dibuixar les peces) i tools/gen_ciudad.py (per
saber quines empremtes pot pintar i on va el llindar de cada local).

Orientació o (0..3): o0 porta a +y, o1 a -x, o2 a -y, o3 a +x. A o1 i o3
l'empremta del mapa (w, h) correspon a la peça canònica (h, w).
"""

# Famílies d'habitatge: mides canòniques i variants (paràmetres per a edificis.casa
# o families.barraca/tenda). per_mida: quantes variants es dibuixen a cada mida.
FAMILIES = {
    "edifici": dict(estil="popular", mides=[(2, 2), (3, 2), (2, 3), (3, 3)], per_mida=3, variants=[
        dict(), dict(plantes=1, porticons="roba2"), dict(alta="estuc_ocre", porticons="cortina"),
        dict(alta="fusta", teula="pissarra", porticons="cortina"), dict(alta="estuc_rosa", teula="verdet", porticons="roba2"),
        dict(alta="estuc_blau", porticons="tela_verm")]),
    "noble": dict(estil="noble", mides=[(4, 4), (5, 4), (4, 5), (5, 3), (3, 5), (4, 3), (3, 4)], per_mida=2, variants=[
        dict(), dict(teula="verdet"), dict(alta="estuc_blau")]),
    "cabana": dict(estil="cabana", mides=[(2, 2), (3, 2), (2, 3), (3, 3)], per_mida=2, variants=[
        dict(), dict(mur="encenall", alta="encenall"), dict(teula="teula")]),
    "caserna": dict(estil="caserna", mides=[(2, 4), (4, 2), (3, 3)], per_mida=1, variants=[dict()]),
    "barraca": dict(estil="barraca", mides=[(2, 2), (3, 2), (2, 3), (3, 3)], per_mida=3, variants=[dict()]),
    "tenda": dict(estil="tenda", mides=[(2, 2)], per_mida=3, variants=[dict(model="paret"), dict(model="pavello"), dict(model="tendal")]),
}

# Oficis: mides del mapa on surten (el catàleg hi afegeix les transposades per a o1 i o3).
OFICIS = {
    "ferreria": [(5, 4)], "taverna": [(5, 3)], "botiga": [(5, 3)], "mercat": [(5, 4)], "joieria": [(4, 4)],
    "banc": [(5, 4), (4, 3)], "sastreria": [(4, 4), (4, 3)], "posada": [(5, 4)], "banys": [(5, 3), (7, 4)],
    "casa_te": [(6, 4)], "armeria": [(7, 5)],
}

# La porta de cada local diu quin ofici és.
PORTA_OFICI = {
    "puerta_herreria": "ferreria", "puerta_taberna": "taverna", "puerta_mercado_sur": "botiga", "puerta_mercado": "mercat",
    "puerta_joyeria": "joieria", "puerta_banco": "banc", "puerta_banco_sur": "banc", "puerta_sastreria": "sastreria",
    "puerta_ropa_sur": "sastreria", "puerta_posada": "posada", "puerta_posada_sur": "posada", "puerta_banos": "banys",
    "puerta_banos_sur": "banys", "puerta_casa_te": "casa_te", "puerta_armeria": "armeria",
}

PATI = 12            # unitats de pati davant la façana

# El terra de la rajola de cada família (el que es veu on la peça no tapa l'empremta:
# al voltant de les tendes i de les barraques): el terra on surt al mapa.
SOL_FAMILIA = {"edifici": "adoquin_limpio", "noble": "pradera", "cabana": "sendero_tierra",
               "caserna": "tierra_piedras", "barraca": "callejon_embarrado", "tenda": "sendero_tierra"}
SOL_OFICI = {"casa_te": "marmol_real"}           # la resta, adoquin_limpio


def rajola_familia(fam, o):
    return f"{fam}_o{o}"


def rajola_ofici(ofici, o):
    return f"local_{ofici}_o{o}"


def mides_ofici(ofici):
    """Mides canòniques d'un ofici: les del mapa i les transposades."""
    out = []
    for (w, h) in OFICIS[ofici]:
        for m in ((w, h), (h, w)):
            if m not in out:
                out.append(m)
    return out


def porta_u(ample_cel):
    """X canònica (unitats) del centre de la porta d'un local: la cel·la central."""
    return (ample_cel // 2 + 0.5) * 32


def gira_punt(X, Y, o, W, D):
    """Igual que motor_iso.gira (punt canònic -> vista o) però sense dependències."""
    if o == 0:
        return X, Y
    if o == 1:
        return D - Y, X
    if o == 2:
        return W - X, D - Y
    return Y, W - X


def cella_llindar(w_mapa, h_mapa, o):
    """Cel·la (dx, dy) dins l'empremta del mapa on cau el llindar de la porta d'un
    local a l'orientació o (la cel·la de la façana davant la porta)."""
    wc, hc = (w_mapa, h_mapa) if o % 2 == 0 else (h_mapa, w_mapa)
    X, Y = porta_u(wc), hc * 32 - 16                         # centre de la cel·la de la façana
    x, y = gira_punt(X, Y, o, wc * 32, hc * 32)
    return int(x // 32), int(y // 32)


def costat_porta(o):
    """Vector cap a fora de la façana de la porta, a l'orientació o."""
    return {0: (0, 1), 1: (-1, 0), 2: (0, -1), 3: (1, 0)}[o]
