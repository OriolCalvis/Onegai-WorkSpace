#!/usr/bin/env python3
"""ISO (INT-01) — Els mobles dels interiors amb el kit, a la manera dels
«furni» d'Habbo: una cel·la cadascun (els llargs, en trams), la mateixa llum
i el mateix contorn que les cases, i la cara de davant a +Y (o0) com tots els
decorats. Els que tenen sentit girats surten en 4 orientacions.

Les peces són a mobles_comuns.py (taules, llits, armaris, llums, escales),
mobles_oficis.py (taverna, posada, mercat, ferreria, armeria, banc, joieria,
sastreria, casa de te, banys), mobles_monuments.py (castell, església, òpera,
universitat, biblioteca, coliseu, quarter) i mobles_nacions.py (INT-02: el toc de
cada nació de l'est); aquest mòdul les ajunta:

    from mobles import MOBLES, ORIENTABLES, TRAMS, NOMS, SENSE_NOSA
    e = MOBLES["silla"]()           # Escena; e.dibuixa(o) -> (imatge, àncora)

Els ids, en castellà com la resta del catàleg; quan un id ja existia (mesa,
silla, taburete, barril, cofre...) el dibuix del kit el substitueix arreu.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mobles_comuns import R as _COMUNS  # noqa: E402
from mobles_monuments import R as _MONUMENTS  # noqa: E402
from mobles_nacions import R as _NACIONS  # noqa: E402  (INT-02: el toc de cada nació de l'est)
from mobles_oficis import R as _OFICIS  # noqa: E402

MOBLES, TRAMS, NOMS = {}, {}, {}
ORIENTABLES, SENSE_NOSA = set(), set()
for _r in (_COMUNS, _OFICIS, _MONUMENTS, _NACIONS):
    repetits = (set(_r.mobles) | set(_r.trams)) & (set(MOBLES) | set(TRAMS))
    assert not repetits, f"mobles repetits entre mòduls: {sorted(repetits)}"
    MOBLES.update(_r.mobles)
    TRAMS.update(_r.trams)
    NOMS.update(_r.noms)
    ORIENTABLES |= _r.orientables
    SENSE_NOSA |= _r.sense_nosa
