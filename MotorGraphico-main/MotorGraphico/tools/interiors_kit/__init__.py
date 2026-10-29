"""INT-01 — Els plans dels interiors de Boundington (un mòdul per edifici o
grup). EDIFICIS: edifici -> [(nivell, fer_sala)], la planta baixa primer; el
generador és tools/gen_interiores_kit.py. INT-02: i els 33 de l'est (este.py,
en_<assentament>_<local>, una planta cadascun)."""
from interiors_kit.banco import PLANTES_BANCO, PLANTES_JOYERIA
from interiors_kit.banos import PLANTES as BANOS
from interiors_kit.castillo import PLANTES as CASTILLO
from interiors_kit.coliseo import PLANTES_COLISEO, PLANTES_CUARTEL
from interiors_kit.este import plantes as plantes_est
from interiors_kit.herreria import PLANTES_ARMERIA, PLANTES_HERRERIA
from interiors_kit.iglesia import PLANTES as IGLESIA
from interiors_kit.mercado import PLANTES as MERCADO
from interiors_kit.opera import PLANTES as OPERA
from interiors_kit.posada import PLANTES as POSADA
from interiors_kit.sastreria import PLANTES_CASA_TE, PLANTES_SASTRERIA
from interiors_kit.taberna import PLANTES as TABERNA
from interiors_kit.universidad import PLANTES_BIBLIOTECA, PLANTES_UNIVERSIDAD

EDIFICIS = {
    "taberna": TABERNA, "posada": POSADA, "mercado": MERCADO, "herreria": PLANTES_HERRERIA,
    "armeria": PLANTES_ARMERIA, "banco": PLANTES_BANCO, "joyeria": PLANTES_JOYERIA, "sastreria": PLANTES_SASTRERIA,
    "casa_te": PLANTES_CASA_TE, "banos": BANOS, "castillo": CASTILLO, "iglesia": IGLESIA, "opera": OPERA,
    "universidad": PLANTES_UNIVERSIDAD, "biblioteca": PLANTES_BIBLIOTECA, "coliseo": PLANTES_COLISEO,
    "cuartel": PLANTES_CUARTEL,
}
EDIFICIS.update(plantes_est())    # INT-02: interior_en_<assentament>_<local>
