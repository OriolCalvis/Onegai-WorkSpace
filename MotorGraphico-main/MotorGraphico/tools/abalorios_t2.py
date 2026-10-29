#!/usr/bin/env python3
"""PART-02c — Los abalorios del Tier 2: los anillos, los collares y los cinturones que dan las particulas.

Oriol (07/10/2026): «podemos crear un anillo que de esas particulas aumentando la equipacion del tier 2
donde van collares, cinturon y anillos como equipos añadidos», y «si un anillo aporta un hechizo de fuego
y uno de hielo, salgan sprites azules y rojos de la mano del pj». Reglas en
planificacion/CANON_PARTICULAS.md §6:

  - un anillo por elemento (los 13 de tools/elementos.py), con las particulas de su elemento;
  - cuatro anillos de dos efectos (el de fuego y escarcha de Oriol, la tormenta, el eclipse y la
    cienaga), que echan las dos familias a medias desde la mano;
  - un collar o un cinturon por cada familia de tema que es la primera de alguna clase
    (tools/particulas.py: las notas de los bardos, los dragones del Puño del dragon, los petalos de las
    Artes Misticas, las monedas del Mercader...), para que cada clase encuentre el suyo. Collar lo que
    sale de la voz, el alma o la fe (del pecho); cinturon lo del oficio y el cuerpo (de la cintura).

Todos de Tier 2, raros (lo que el Tier 2 deja llevar), +1 al atributo principal de las clases que los
buscaran (los de dos efectos, +1 y +1) y el nivel que pide su regla (tools/niveles_equipo.py: +1, 20;
+2, 26). El efecto de juego de los elementos (las resistencias) llegara con el combate que las use: hoy
el elemento va en sus etiquetas (grantedTags).

Escribe equipment.json y el objeto item_<id> de cada uno en assets/objects/libreria_armas.json y
libreria_completa.json (el puente del inventario y la tienda), como tools/importar_clases_sp.py: upsert
por id, no borra nada. Las particulas las lee actores_estetica.brillo_sin_pieza (BRILLO), y el motor las
echa desde su sitio (el anillo, de la mano; el collar, del pecho; el cinturon, de la cintura).

    python3 tools/abalorios_t2.py          # escribe
    python3 tools/abalorios_t2.py --ver    # solo comprueba y cuenta
"""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import elementos  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
CAT = ROOT / "assets/catalogs"
OBJ = ROOT / "assets/objects"

# Un anillo por elemento: (id, nombre, elemento, atributo, texto). El atributo, el principal de las clases que
# tienen las particulas de ese elemento primero (las del fuego, a medias DES e INT: INT, la magia).
ANILLOS = [
    ("anillo_de_brasa_viva", "Anillo de Brasa Viva", "Fuego", "INT", "Una brasa que no se apaga, engastada en hierro."),
    ("anillo_de_la_marea", "Anillo de la Marea", "Agua", "DES", "Sube y baja con el pulso de quien lo lleva."),
    ("anillo_de_escarcha", "Anillo de Escarcha", "Hielo", "INT", "Frío al tacto, incluso en pleno verano."),
    ("anillo_del_trueno", "Anillo del Trueno", "Rayo", "DES", "Chasquea un instante antes de cada tormenta."),
    ("anillo_de_piedra_viva", "Anillo de Piedra Viva", "Tierra", "CON", "Tallado de una sola piedra de río, sin una grieta."),
    ("anillo_del_vendaval", "Anillo del Vendaval", "Viento", "DES", "Silba cuando quien lo lleva echa a correr."),
    ("anillo_del_alba", "Anillo del Alba", "Luz", "CAR", "Guarda la primera luz de la mañana y la suelta despacio."),
    ("anillo_de_la_penumbra", "Anillo de la Penumbra", "Tinieblas", "DES", "La sombra de la mano que lo lleva siempre llega tarde."),
    ("anillo_de_raiz", "Anillo de Raíz", "Naturaleza", "INT", "Una raíz trenzada que todavía echa brotes."),
    ("anillo_de_la_vibora", "Anillo de la Víbora", "Veneno", "DES", "Dos colmillos de plata que casi se tocan."),
    ("anillo_de_sangre", "Anillo de Sangre", "Sangre", "CAR", "Late cuando hay una herida cerca."),
    ("anillo_arcano", "Anillo Arcano", "Arcano", "INT", "Sus runas giran solas alrededor de la piedra."),
    ("anillo_del_vacio", "Anillo del Vacío", "Vacio", "INT", "Pesa menos que nada, y a veces menos todavía."),
]

# Dos efectos: (id, nombre, (elemento, elemento), (atributo, atributo), texto): sus dos familias a medias.
DOBLES = [
    ("anillo_de_fuego_y_escarcha", "Anillo de Fuego y Escarcha", ("Fuego", "Hielo"), ("INT", "CON"),
     "Media piedra arde y la otra media escarcha, y ninguna de las dos cede."),
    ("anillo_de_la_tormenta", "Anillo de la Tormenta", ("Rayo", "Agua"), ("DES", "CAR"),
     "Huele a lluvia antes de que llueva."),
    ("anillo_del_eclipse", "Anillo del Eclipse", ("Luz", "Tinieblas"), ("CAR", "DES"),
     "Mitad sol y mitad noche, y ninguna gana."),
    ("anillo_de_la_cienaga", "Anillo de la Ciénaga", ("Veneno", "Naturaleza"), ("DES", "INT"),
     "Musgo vivo alrededor de una gota que nunca se seca."),
]

# Los temas de las clases (la primera familia de alguna clase): (id, nombre, familia, color o None para el de
# la familia, atributo, texto). Collar: la voz, el alma, la fe; cinturon: el oficio y el cuerpo. El
# atributo, el principal de las clases que tienen esa familia primero.
COLLARES = [
    ("colgante_del_diapason", "Colgante del Diapasón", "notas", "ffe6a0", "CAR", "Un diapasón de plata que suena solo cuando alguien canta cerca."),
    ("relicario_de_las_animas", "Relicario de las Ánimas", "animas", None, "CAR", "Dentro, un mechón de pelo y una voz que no es de nadie."),
    ("collar_del_firmamento", "Collar del Firmamento", "estrellas", "dce6ff", "INT", "Siete estrellas de vidrio que no se apagan ni de día."),
    ("collar_de_colmillos_del_espiritu", "Collar de Colmillos del Espíritu", "espiritus", None, "DES", "Cada colmillo, un espíritu que camina al lado."),
    ("collar_de_plumas_de_cuervo", "Collar de Plumas de Cuervo", "plumas", None, "INT", "Las plumas crujen como las páginas de un libro viejo."),
    ("colgante_del_tambor", "Colgante del Tambor", "ondas", "c8e6ff", "CAR", "Un tambor del tamaño de un pulgar que se oye a una legua."),
    ("gargantilla_del_murcielago", "Gargantilla del Murciélago", "murcielagos", None, "DES", "Se aprieta un poco cuando se pone el sol."),
    ("gorguera_del_bufon", "Gorguera del Bufón", "confeti", None, "CAR", "Suelta confeti cuando nadie la mira."),
    ("colgante_de_esporas", "Colgante de Esporas", "esporas", None, "INT", "Un frasquito de musgo que respira."),
    ("rosario_del_despertar", "Rosario del Despertar", "ki", None, "DES", "Ciento ocho cuentas y un solo aliento."),
    ("colgante_del_dragon", "Colgante del Dragón", "dragones", None, "DES", "Un dragón de jade enroscado que se despereza al pelear."),
    ("collar_del_loto", "Collar del Loto", "petalos", None, "DES", "Los pétalos se abren con la calma de quien lo lleva."),
    ("colgante_del_sol_blanco", "Colgante del Sol Blanco", "llamas_puras", None, "CON", "Quema sin humo, como el fuego que no ha comido nada."),
    ("colgante_de_la_espiga", "Colgante de la Espiga", "grano", None, "CAR", "Una espiga dorada que nunca se seca."),
    ("collar_de_los_elementos", "Collar de los Elementos", "elementos", None, "CAR", "Cuatro piedras, cuatro elementos, ninguna pelea."),
    ("collar_de_dientes_de_la_furia", "Collar de Dientes de la Furia", "furia", None, "CON", "Dientes de las bestias que no se rindieron."),
]
CINTURONES = [
    ("cinturon_de_la_fragua", "Cinturón de la Fragua", "chispas", "ffc864", "CON", "Cuero quemado de mil chispas y ninguna le ha hecho nada."),
    ("cinturon_de_saquitos_de_humo", "Cinturón de Saquitos de Humo", "humo", None, "CON", "Doce saquitos y en cada uno, una niebla."),
    ("bandolera_de_polvora", "Bandolera de Pólvora", "polvora", None, "DES", "Huele a pólvora aunque esté vacía."),
    ("cinturon_de_herramientas", "Cinturón de Herramientas", "engranajes", None, "INT", "Siempre falta la llave que se necesita, y siempre aparece."),
    ("faja_del_desierto", "Faja del Desierto", "polvo", None, "DES", "Lleva arena dentro, y no hay manera de sacarla."),
    ("faja_de_brasas", "Faja de Brasas", "brasas", "ff8c3c", "CON", "Tibia como un hogar recién apagado."),
    ("cinturon_del_mercader", "Cinturón del Mercader", "monedas", None, "CAR", "La bolsa tintinea más de lo que pesa."),
    ("cinturon_de_redomas", "Cinturón de Redomas", "alquimia", None, "INT", "Seis redomas, seis colores, y la séptima nadie sabe."),
    ("fajin_del_tahur", "Fajín del Tahúr", "naipes", None, "INT", "Siempre asoma un as por algún pliegue."),
    ("cinturon_del_campeon", "Cinturón del Campeón", "oro", None, "INT", "Hebilla de oro con los nombres de todos los que cayeron."),
]

STATS = ("CAR", "CON", "DES", "INT")


def _familia_de(elemento):
    _palabra, color, familia = elementos.ELEMENTOS[elemento]
    return familia, color


def abalorios():
    """[(id, nombre, slot, [(familia, color)], bonos, texto, etiquetas)] de los 43."""
    out = []
    for ident, nombre, elemento, stat, texto in ANILLOS:
        out.append((ident, nombre, "Ring", [_familia_de(elemento)], {stat: 1}, texto, [elementos.nombre(elemento)]))
    for ident, nombre, (e1, e2), (s1, s2), texto in DOBLES:
        bonos = {s1: 1}
        bonos[s2] = bonos.get(s2, 0) + 1
        out.append((ident, nombre, "Ring", [_familia_de(e1), _familia_de(e2)], bonos, texto,
                    [elementos.nombre(e1), elementos.nombre(e2)]))
    for slot, lista in (("Neck", COLLARES), ("Belt", CINTURONES)):
        for ident, nombre, familia, color, stat, texto in lista:
            out.append((ident, nombre, slot, [(familia, color)], {stat: 1}, texto, []))
    return out


def entrada(a):
    """La entrada de equipment.json de un abalorio."""
    import niveles_equipo
    ident, nombre, slot, _familias, bonos, texto, etiquetas = a
    e = {
        "compatibleSymbols": [], "defenseBonuses": {}, "description": texto, "flavorText": texto,
        "grantedTags": list(etiquetas), "id": ident, "linkedSkill": None, "name": nombre,
        "penalties": {s: 0 for s in STATS}, "price": 550, "rarity": "rare", "requiredLevel": 20,
        "requiredStats": {s: 0 for s in STATS}, "restrictions": [], "slot": slot,
        "statBonuses": {s: bonos.get(s, 0) for s in STATS}, "tier": 2, "type": "equipment",
        "weightCategory": "light",
    }
    e["requiredLevel"] = niveles_equipo.nivel_por_regla(e, set())
    return e


def _color_de(familia, color):
    """El color de la luz de un efecto: el dado o el primero de los propios de su familia."""
    if color:
        return color
    import particulas
    propios = particulas.FAMILIAS[familia].get("colores") or []
    return propios[0] if propios else "ffffff"


# id -> {"particulas", "color"[, "otrasParticulas", "otroColor"]}: lo que lee actores_estetica.brillo_sin_pieza
BRILLO = {}
for _a in abalorios():
    (_f1, _c1), *_otra = _a[3]
    BRILLO[_a[0]] = {"particulas": _f1, "color": _color_de(_f1, _c1)}
    if _otra:
        _f2, _c2 = _otra[0]
        BRILLO[_a[0]].update(otrasParticulas=_f2, otroColor=_color_de(_f2, _c2))


def comprobar():
    """Que cada abalorio tenga una familia del motor y que cada familia de tema que es la primera de una
    clase tenga el suyo (y los 13 elementos, su anillo)."""
    import particulas
    errores = []
    ids = [a[0] for a in abalorios()]
    if len(ids) != len(set(ids)):
        errores.append("ids repetidos")
    for a in abalorios():
        for familia, _color in a[3]:
            if familia not in particulas.FAMILIAS:
                errores.append(f"{a[0]}: la familia {familia} no existe")
    primeras = {v[0][0] if isinstance(v[0], list) else v[0] for v in particulas.CLASES.values()}
    con_abalorio = {f for a in abalorios() for f, _c in a[3]}
    for familia in sorted(primeras - con_abalorio):
        errores.append(f"la familia {familia} es la primera de alguna clase y no tiene abalorio")
    for e in elementos.ELEMENTOS:
        if not any(a[2] == "Ring" and len(a[3]) == 1 and a[3][0][0] == elementos.ELEMENTOS[e][2] for a in abalorios()):
            errores.append(f"el elemento {e} no tiene anillo")
    return errores


def main():
    import importar_clases_sp as imp
    from sp_reglas import SLOT_PUENTE
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--ver", action="store_true", help="solo comprueba y cuenta")
    args = ap.parse_args()
    errores = comprobar()
    if errores:
        raise SystemExit("\n".join(errores))
    rutas = {"equipment": CAT / "equipment.json", "armas": OBJ / "libreria_armas.json",
             "completa": OBJ / "libreria_completa.json"}
    docs = {n: imp.cargar(r) for n, r in rutas.items()}
    cuenta = {"nueva": 0, "actualizada": 0}
    for a in abalorios():
        e = entrada(a)
        cuenta[imp.upsert(docs["equipment"]["entries"], e)] += 1
        assert e["slot"] in SLOT_PUENTE, e["slot"]
        for n in ("armas", "completa"):
            imp.upsert(docs[n]["objects"], imp.puente(e))
    por_slot = {}
    for a in abalorios():
        por_slot[a[2]] = por_slot.get(a[2], 0) + 1
    print(f"{len(abalorios())} abalorios de Tier 2 ({por_slot}): {cuenta['nueva']} nuevos, "
          f"{cuenta['actualizada']} actualizados")
    if args.ver:
        return
    for n, r in rutas.items():
        imp.guardar(r, docs[n])


if __name__ == "__main__":
    main()
