"""SETS-03 — Los guantes del Mago Chasqueador (el arma de su set de tier 2, tools/sets_iteracion3.py).

El Mago Chasqueador lanza su magia chasqueando los dedos: su arma son los guantes con que chasquea, uno en cada
mano (actores_armas.PAREJA, como los guanteletes del Artista Marcial), a su tamano (sin pose): el guante
blanco de mago con su puno vuelto (todo material 1) y, en el escalon raro (el del set), las runas de las
yemas (la gema, material 4), que chispean. Tapan la mano entera, de frente y de perfil.

    1-5 material 1 (luz, base, sombra, contorno, contorno duro)   a-e material 3   f-j material 4 (la gema)
    & el ancla (el puno; se pinta como el 2)

Va al final del atlas de armas (ninguna fila de antes se mueve): la importa actores_formas despues de
actores_formas_propias, antes de registrar().
"""
import actores_formas as F

# Tapan la mano en todas las tallas y frames (test_los_guanteletes_van_en_las_dos_manos: mas del 70 % de la
# mano que se ve): de frente 7 x 6 y de perfil 8 x 6, el ancla '&' en el centro del puno. Arriba el puno
# vuelto del guante (el borde claro y el doblez en sombra); abajo los dedos, separados por dos lineas.
GUANTES = [
    "4111114",   # el borde del puno vuelto
    "4333334",   # el doblez
    "4122224",
    "412&224",
    "4232324",   # los dedos
    ".44444.",
]
GUANTES_LADO = [
    "41111114",
    "43333334",
    "41122224",
    "412&2214",  # el pulgar, delante
    ".4232324",
    "..44444.",
]
GUANTES_RARO = [  # las runas violeta de las yemas (la gema), que chispean
    "4111114",
    "4333334",
    "4122224",
    "412&224",
    "4g3g3g4",
    ".44444.",
]
GUANTES_RARO_LADO = [
    "41111114",
    "43333334",
    "41122224",
    "412&2214",
    ".4g3g3g4",
    "..44444.",
]

FORMAS = {"guantes": {"abajo": GUANTES, "derecha": GUANTES_LADO}}
BASES = {"f_guantes": ("guantes", "guante", F.MANGO, "Guantes", F.GUARNICION)}
ESCALERA = {"guantes": {"raro": {"abajo": GUANTES_RARO, "derecha": GUANTES_RARO_LADO}}}
PAREJA = {"guantes", "guantes_raro"}


def _sumar():
    F.FORMAS.update(FORMAS)
    F.BASES.update(BASES)
    F.ESCALERAS.update(ESCALERA)
    for forma, pasos in ESCALERA.items():
        for paso, dibujo in pasos.items():
            F.FORMAS[f"{forma}_{paso}"] = dibujo
            base = F.BASES[f"f_{forma}"]
            F.BASES[f"f_{forma}_{paso}"] = (f"{forma}_{paso}", base[1], base[2], f"{base[3]} ({paso})", base[4],
                                            F.GEMA)
    F.PAREJA.update(PAREJA)


_sumar()
