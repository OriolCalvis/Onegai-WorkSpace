#!/usr/bin/env python3
"""Encargo C (C4 / AV17-06..08), Fable 16/10: las cinemáticas que se pueden hacer con las
órdenes que el motor ya tiene (PROLOGO-02 + AV17-10: anda, patrulla, coloca, mira, espera,
beat, flag, nivel, funde, trepa, baja, arde, apaga, tumba, levanta, emocion).

  - xila_desprecio   (ciudad_oeste)  Xila te da la espalda al tratarle con desprecio (día 1).
  - fiesta_salon     (mansión)       los anfitriones enmascarados salen a recibirte al entrar
                                      con bnd2_fiesta_dentro; después vuelven al salón.
  - barricada_llamas (afueras)       al oír los gritos (bnd3_barricada_vista) arde el paso
                                      central de la tapia (19..21, 13) y las casillas de al lado.
  - barricada_rota   (afueras)       al resolverla, el fuego se apaga casilla a casilla.
  - epilogo_barca    (cualquiera)    fundido a blanco, el prado del río, la barca de Sho-Noco con
                                      los dos a bordo y vuelta; b3_barca espera a que acabe.

Toca: assets/cinematicas/cinematicas.json, assets/levels/mansion_barrio_alto.json (anfitriona y
mercader, que solo se ven con bnd2_fiesta_dentro) y boundington_ocaso.json (b3_barca pide
cine_epilogo_barca_vista). Idempotente. Ningún cambio de C++.
"""
import json
import os
import re
import sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def carga(rel):
    p = os.path.join(RAIZ, rel)
    texto = open(p, encoding="utf-8").read()
    m = re.match(r'\{\n( +)\S', texto)
    return p, json.loads(texto), (len(m.group(1)) if m else 1)


def guarda(p, d, ind):
    open(p, "w", encoding="utf-8").write(json.dumps(d, indent=ind, ensure_ascii=False) + "\n")


BARRICADA = [[20, 12], [19, 13], [21, 13], [20, 14], [20, 13]]  # el centro, el último en apagarse

CINEMATICAS = [
    {
        "id": "xila_desprecio",
        "_nota": "C4 / AV17-08 — Día 1: al tratar a Xila con desprecio (bnd1_xila_maltratado) se vuelve de espaldas y rumia. Luego sigue b1_xila_resentido como siempre.",
        "nivel": "assets/levels/ciudad_oeste.json",
        "requiere": ["bnd1_xila_maltratado"],
        "pasos": [
            {"mira": "xila", "hacia": "arriba"},
            {"emocion": "xila", "icono": "enfurecido", "segundos": 1.4, "aguarda": True},
            {"espera": 0.3},
        ],
    },
    {
        "id": "fiesta_salon",
        "_nota": "C4 / AV17-06 — Día 2: al entrar en la mansión con bnd2_fiesta_dentro, los dueños (máscaras de plumas) salen a recibirte y vuelven al salón. Lo que se dice sigue en b2_dentro / b2_invitados.",
        "nivel": "assets/levels/mansion_barrio_alto.json",
        "requiere": ["bnd2_fiesta_dentro"],
        "sin": ["bnd2_fiesta_resuelta"],
        "pasos": [
            {"anda": "mercader", "a": [3, 15], "velocidad": 1.4, "sinEsperar": True},
            {"anda": "anfitriona", "por": [[4, 14], [3, 14]], "velocidad": 1.4},
            {"mira": "anfitriona", "hacia": "izquierda"},
            {"mira": "mercader", "hacia": "izquierda"},
            {"mira": "jugador", "hacia": "derecha"},
            {"emocion": "anfitriona", "icono": "encantado", "segundos": 1.4, "aguarda": True},
            {"emocion": "mercader", "icono": "oro", "segundos": 1.0, "aguarda": True},
            {"espera": 0.3},
            {"anda": "mercader", "a": [9, 15], "velocidad": 1.4, "sinEsperar": True},
            {"anda": "anfitriona", "a": [9, 14], "velocidad": 1.4},
            {"mira": "anfitriona", "hacia": "arriba"},
            {"mira": "mercader", "hacia": "arriba"},
        ],
    },
    {
        "id": "barricada_llamas",
        "_nota": "C4 / AV17-07 — Ocaso: al oír los gritos (b3_barricada_vista), el paso central de la tapia de las afueras arde con fuego blanco. Se queda ardiendo hasta barricada_rota.",
        "nivel": "assets/levels/afueras_boundington.json",
        "requiere": ["bnd3_barricada_vista"],
        "pasos": [{"arde": c} if i == 0 else {"espera": 0.15} for c in BARRICADA for i in range(2)][:-1] + [
            {"espera": 0.6},
        ],
    },
    {
        "id": "barricada_rota",
        "_nota": "C4 / AV17-07 — Ocaso: resuelta la barricada (bnd3_barricada_resuelta), el fuego se apaga casilla a casilla, el centro el último, y el paso queda abierto.",
        "nivel": "assets/levels/afueras_boundington.json",
        "requiere": ["bnd3_barricada_resuelta"],
        "pasos": [{"apaga": c} if i == 0 else {"espera": 0.3} for c in BARRICADA for i in range(2)][:-1] + [
            {"espera": 0.5},
        ],
    },
    {
        "id": "epilogo_barca",
        "_nota": "C4 / AV17-07 — El epílogo (b3_epilogo / b3_epilogo_huido encienden bnd3_epi_empezado): fundido a blanco, el prado del río, la barca de Sho-Noco con él en la popa y tú en la proa, como en la subida del prólogo. b3_barca espera a cine_epilogo_barca_vista.",
        "requiere": ["bnd3_epi_empezado"],
        "pasos": [
            {"espera": 0.6},
            {"funde": 1, "segundos": 1.6, "color": "blanco"},
            {"nivel": "assets/levels/prado_del_rio.json", "en": [13, 8]},
            {"coloca": "barca_de_sho_noco", "en": [14, 8]},
            {"coloca": "sho_noco", "en": [14, 8], "desfase": [0, -0.35]},
            {"coloca": "jugador", "en": [13, 8], "desfase": [1.0, 0.3]},
            {"mira": "jugador", "hacia": "abajo"},
            {"mira": "sho_noco", "hacia": "abajo"},
            {"espera": 0.4},
            {"funde": 0, "segundos": 2.2, "color": "blanco"},
            {"espera": 0.8},
        ],
    },
]
APARICIONES_NUEVAS = [
    {"objeto": "anfitriona", "_nota": "C4 — Los dueños de la mansión solo están la noche de la fiesta.",
     "visibleSi": {"todas": ["bnd2_fiesta_dentro"]}, "ocultoSi": {"todas": ["bnd2_fiesta_resuelta"]}},
    {"objeto": "mercader", "visibleSi": {"todas": ["bnd2_fiesta_dentro"]}, "ocultoSi": {"todas": ["bnd2_fiesta_resuelta"]}},
]


def main():
    # 1) cinemáticas
    p, cat, ind = carga("assets/cinematicas/cinematicas.json")
    ids = {c["id"]: i for i, c in enumerate(cat["cinematicas"])}
    for c in CINEMATICAS:
        if c["id"] in ids:
            cat["cinematicas"][ids[c["id"]]] = c
        else:
            cat["cinematicas"].append(c)
    # Sho-Noco y su barca vuelven en el epílogo: antes solo se veían en el prólogo.
    for a in cat["apariciones"]:
        if a["objeto"] == "sho_noco":
            a.pop("si", None); a.pop("sin", None)
            a["_nota"] = "Prólogo (de que la barca viene a que se va) y epílogo (bnd3_epi_empezado)."
            a["visibleSi"] = {"alguna": ["bnd_prologo_barca_viene", "bnd3_epi_empezado"]}
            a["ocultoSi"] = {"todas": ["bnd_pr_shonoco_se_va"], "ninguna": ["bnd3_epi_empezado"]}
        if a["objeto"] == "barca_de_sho_noco":
            a.pop("si", None); a.pop("sin", None)
            a["visibleSi"] = {"alguna": ["bnd_prologo_barca_viene", "bnd3_epi_empezado"]}
    hay = {a["objeto"] for a in cat["apariciones"]}
    for a in APARICIONES_NUEVAS:
        if a["objeto"] not in hay:
            cat["apariciones"].append(a)
    guarda(p, cat, ind)

    # 2) la mansión: los anfitriones
    p, lvl, ind = carga("assets/levels/mansion_barrio_alto.json")
    hay = {o["objectId"] for o in lvl["objects"]}
    for oid, x, y in (("anfitriona", 6, 14), ("mercader", 6, 15)):
        if oid not in hay:
            lvl["objects"].append({"objectId": oid, "position": {"x": x, "y": y}})
    guarda(p, lvl, ind)

    # 3) b3_barca espera a la cinemática
    p, adv, ind = carga("assets/adventures/boundington_ocaso.json")
    for b in adv["beats"]:
        if b["id"] == "b3_barca":
            req = b.setdefault("requires", {}).setdefault("allFlags", [])
            if "cine_epilogo_barca_vista" not in req:
                req.append("cine_epilogo_barca_vista")
    guarda(p, adv, ind)
    print("parche_c_cinematicas: 5 cinemáticas, 2 anfitriones, b3_barca encadenado")
    return 0


if __name__ == "__main__":
    sys.exit(main())
