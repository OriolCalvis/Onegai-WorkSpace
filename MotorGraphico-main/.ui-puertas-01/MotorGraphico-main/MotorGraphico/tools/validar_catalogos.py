#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""D-10 — Los catalogos reales no pueden perder campos en silencio.

POR QUE EXISTE
--------------
Asi es exactamente como el proyecto llego hasta aqui: el asset de
habilidades traia `defenseStat` en 465 de sus 492 cartas y el cargador
leia `saveAttribute`. Nadie lo noto. No fallaba ninguna prueba, no fallaba
ninguna carga: el campo simplemente llegaba vacio y las habilidades hacian
lo que no decia su carta.

Hoy nada impide que vuelva a pasar. Un `git mv` de una clave en el JSON,
un cargador que se reescribe, un generador de assets que cambia un nombre:
todos dejan el motor compilando, los tests en verde y el contenido roto.

QUE MIDE, Y POR QUE ASI
-----------------------
Para cada familia de catalogo cuenta, campo declarado a campo declarado,
cuantas entradas REALES lo llevan. Dos numeros por campo:

  presentes  — la clave esta en la entrada (con cualquiera de sus alias).
               Es exactamente el criterio del cargador
               (`catalog_field_present` en RpgCatalogs.h): si la clave no
               esta, el cargador se queda con el valor por defecto.
  con_valor  — ademas, el valor no esta vacio. Un `"saveAttribute": ""`
               en las 492 cartas pasa el primer contador y no sirve de
               nada; el desapareamiento tambien puede llegar por ahi.

Esos numeros se guardan en `tools/linea_base_catalogos.json` y en cada
commit se vuelven a medir. Si BAJAN, el job falla. Si suben, no falla:
subir es migrar un asset, que es justo lo que D-02..D-09 estan haciendo;
el job dice que conviene volver a registrar la linea base.

LA LISTA DE CAMPOS NO SE ESCRIBE AQUI
-------------------------------------
Se lee de `include/RPG/Catalogs/RpgCatalogs.h`, de las tablas
`expected_fields()` que D-01 dejo puestas. Escribirla a mano en Python
seria crear una SEGUNDA fuente de verdad, y una segunda fuente de verdad
se desincroniza: acabariamos vigilando campos que el cargador ya no lee y
sin vigilar los que lee. Al leer el header, la pregunta que responde este
guardia es la unica que importa: «lo que el cargador dice que lee, ¿sigue
estando en los assets?».

DEC-02 — manda el dato de la carta y el C++ se adapta. Este guardia no
opina sobre que nombre es el bueno: solo impide que la pareja
JSON <-> cargador se rompa sin que nadie se entere.

USO
---
  python3 tools/validar_catalogos.py                    # comprobar (lo de CI)
  python3 tools/validar_catalogos.py --escribir         # registrar linea base
  python3 tools/validar_catalogos.py --informe          # tabla de cobertura
  python3 tools/validar_catalogos.py --autoprueba       # probar el propio guardia
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(RAIZ, "include", "RPG", "Catalogs", "RpgCatalogs.h")
CATALOGOS = os.path.join(RAIZ, "assets", "catalogs")
LINEA_BASE = os.path.join(RAIZ, "tools", "linea_base_catalogos.json")

# Que fichero alimenta a cada cargador. Las habilidades son dos ficheros
# porque skills.json y spells.json son el mismo tipo de carta repartido en
# dos mazos (ver include/RPG/Definitions/SkillDefinition.h).
FAMILIAS = {
    "SkillDefinition":      ["skills.json", "spells.json"],
    "ClassDefinition":      ["classes.json"],
    "RaceDefinition":       ["races.json"],
    "BackgroundDefinition": ["backgrounds.json"],
    "PassiveDefinition":    ["passives.json"],
    "FeatDefinition":       ["feats.json"],
    "TraitDefinition":      ["traits.json"],
    "DeityDefinition":      ["deities.json"],
    "ConditionDefinition":  ["conditions.json"],
    "EquipmentDefinition":  ["equipment.json"],
    "ConsumableDefinition": ["consumables.json"],
    "MonsterDefinition":    ["monsters.json"],
    "SummonDefinition":     ["summons.json"],
    "MountDefinition":      ["mounts.json"],
    "TrapDefinition":       ["traps.json"],
    "LootTableDefinition":  ["loot_tables.json"],
    "NpcDefinition":        ["npcs.json"],
    "AdventureDefinition":  ["adventures.json"],
    "EventDefinition":      ["events.json"],
    "LocationDefinition":   ["locations.json"],
}

# Ficheros de assets/catalogs/ que a proposito NO vigila nadie, con el
# motivo. La lista es explicita para que un asset nuevo no se cuele sin
# guardia: si aparece un .json que no esta ni aqui ni en FAMILIAS, el
# guardia falla y obliga a decidir.
SIN_CARGADOR = {
    "MANIFIESTO.json": "es el manifiesto de la conversion, no un catalogo",
    "pregen_characters.json": "no tiene cargador tipado todavia (personajes pregenerados)",
    "locations_trae_test.json": "fixture de prueba, no contenido publicado",
}


# ---------------------------------------------------------------------
# Leer los campos que declara cada cargador
# ---------------------------------------------------------------------

RE_BLOQUE = re.compile(
    r"CatalogLoader<(\w+)>::expected_fields\(\)\s*\{(.*?)\n\}", re.DOTALL)
RE_CAMPO = re.compile(r"\{\{([^{}]*)\}\s*,\s*(true|false)\s*\}")
RE_NOMBRE = re.compile(r'"([^"]+)"')


def campos_declarados(ruta_header=HEADER):
    """family -> [(alias..., obligatorio)], tal como lo dice el header."""
    with open(ruta_header, encoding="utf-8") as fh:
        texto = fh.read()
    familias = {}
    for bloque in RE_BLOQUE.finditer(texto):
        familia = bloque.group(1)
        campos = []
        for campo in RE_CAMPO.finditer(bloque.group(2)):
            alias = RE_NOMBRE.findall(campo.group(1))
            if alias:
                campos.append((alias, campo.group(2) == "true"))
        if campos:
            familias[familia] = campos
    return familias


# ---------------------------------------------------------------------
# Medir los assets
# ---------------------------------------------------------------------

def _tiene_valor(v):
    """Un campo con valor de verdad. `0` y `false` cuentan: son valores
    legitimos (channelRounds 0, oneShot false). Lo que no cuenta es lo que
    equivale a «no hay dato»."""
    if v is None:
        return False
    if isinstance(v, str):
        return v.strip() != ""
    if isinstance(v, (list, dict)):
        return len(v) > 0
    return True


def medir(directorio=CATALOGOS, ruta_header=HEADER):
    """Cobertura real, familia a familia y campo a campo."""
    declarados = campos_declarados(ruta_header)
    medida = {}
    problemas = []

    usados = set()
    for familia, ficheros in sorted(FAMILIAS.items()):
        if familia not in declarados:
            problemas.append(
                "%s esta en FAMILIAS pero ya no declara expected_fields() en "
                "RpgCatalogs.h: o se ha renombrado el cargador o se ha "
                "borrado la tabla de campos" % familia)
            continue
        entradas = []
        for nombre in ficheros:
            usados.add(nombre)
            ruta = os.path.join(directorio, nombre)
            if not os.path.isfile(ruta):
                problemas.append("falta el asset %s (lo pide %s)" % (nombre, familia))
                continue
            with open(ruta, encoding="utf-8") as fh:
                datos = json.load(fh)
            if not isinstance(datos, dict) or not isinstance(datos.get("entries"), list):
                problemas.append("%s no tiene la clave 'entries' con un array" % nombre)
                continue
            entradas.extend(e for e in datos["entries"] if isinstance(e, dict))

        campos = {}
        for alias, obligatorio in declarados[familia]:
            presentes = 0
            con_valor = 0
            for entrada in entradas:
                clave = next((a for a in alias if a in entrada), None)
                if clave is None:
                    continue
                presentes += 1
                if _tiene_valor(entrada[clave]):
                    con_valor += 1
            campos[alias[0]] = {
                "alias": alias,
                "obligatorio": obligatorio,
                "presentes": presentes,
                "con_valor": con_valor,
            }
            # Un campo OBLIGATORIO que no esta en todas las entradas no es
            # una regresion futura: es contenido que hoy no carga. Se dice
            # ya, sin esperar a que alguien lo empeore.
            if obligatorio and presentes < len(entradas):
                problemas.append(
                    "%s: el campo obligatorio '%s' falta en %d de %d entradas"
                    % (familia, alias[0], len(entradas) - presentes, len(entradas)))
        medida[familia] = {"entradas": len(entradas), "ficheros": ficheros, "campos": campos}

    # Ningun asset sin vigilancia y sin decirlo. Un catalogo nuevo que no
    # este en FAMILIAS ni en SIN_CARGADOR se colaria sin guardia, que es
    # justo el agujero que este fichero existe para tapar.
    for nombre in sorted(os.listdir(directorio)):
        if not nombre.endswith(".json"):
            continue
        if nombre in usados or nombre in SIN_CARGADOR:
            continue
        problemas.append(
            "assets/catalogs/%s no lo carga ninguna familia y no esta en "
            "SIN_CARGADOR: añadelo a una de las dos listas de "
            "tools/validar_catalogos.py" % nombre)

    return medida, problemas


# ---------------------------------------------------------------------
# Comparar contra la linea base
# ---------------------------------------------------------------------

def comparar(medida, base):
    """Devuelve (regresiones, mejoras). Solo las regresiones fallan."""
    regresiones = []
    mejoras = []

    for familia, antes in sorted(base.items()):
        ahora = medida.get(familia)
        if ahora is None:
            regresiones.append(
                "la familia %s ha desaparecido: estaba en la linea base con "
                "%d entradas" % (familia, antes["entradas"]))
            continue
        if ahora["entradas"] < antes["entradas"]:
            regresiones.append(
                "%s: %d entradas, la linea base tenia %d (%d perdidas)"
                % (familia, ahora["entradas"], antes["entradas"],
                   antes["entradas"] - ahora["entradas"]))
        elif ahora["entradas"] > antes["entradas"]:
            mejoras.append("%s: %d entradas nuevas"
                           % (familia, ahora["entradas"] - antes["entradas"]))

        for campo, c_antes in sorted(antes["campos"].items()):
            c_ahora = ahora["campos"].get(campo)
            if c_ahora is None:
                regresiones.append(
                    "%s: el cargador ya no declara el campo '%s', que la linea "
                    "base media en %d entradas. Si se ha retirado a proposito "
                    "(DEC-02), vuelve a registrar la linea base."
                    % (familia, campo, c_antes["presentes"]))
                continue
            for contador, etiqueta in (("presentes", "lo llevan"),
                                       ("con_valor", "lo llevan con valor")):
                if c_ahora[contador] < c_antes[contador]:
                    regresiones.append(
                        "%s.%s: %d entradas %s, la linea base registraba %d. "
                        "Alias que busca el cargador: %s"
                        % (familia, campo, c_ahora[contador], etiqueta,
                           c_antes[contador], ", ".join(c_ahora["alias"])))
                elif c_ahora[contador] > c_antes[contador]:
                    mejoras.append("%s.%s: +%d %s"
                                   % (familia, campo,
                                      c_ahora[contador] - c_antes[contador], etiqueta))

    for familia in sorted(set(medida) - set(base)):
        mejoras.append("familia nueva sin linea base: %s" % familia)

    return regresiones, mejoras


# ---------------------------------------------------------------------
# Autoprueba: que el guardia sirva de verdad
# ---------------------------------------------------------------------

def autoprueba():
    """Reproduce el desapareamiento historico y comprueba que falla.

    Es la prueba de cierre que pide el plan: «el job de CI falla si se
    vuelve a introducir a mano el desapareamiento de saveAttribute /
    defenseStat». Se hace sobre una COPIA en un temporal; los assets del
    repositorio no se tocan.
    """
    fallos = []
    with tempfile.TemporaryDirectory() as tmp:
        copia = os.path.join(tmp, "catalogs")
        shutil.copytree(CATALOGOS, copia)

        base_sana, problemas = medir(copia)
        if problemas:
            print("  aviso: la medida de partida ya trae problemas:")
            for p in problemas:
                print("    - %s" % p)

        # 1) Renombrar la clave en el asset: es el desapareamiento exacto.
        ruta = os.path.join(copia, "skills.json")
        with open(ruta, encoding="utf-8") as fh:
            datos = json.load(fh)
        renombradas = 0
        for entrada in datos["entries"]:
            if "defenseStat" in entrada:
                entrada["defensa_stat"] = entrada.pop("defenseStat")
                renombradas += 1
        with open(ruta, "w", encoding="utf-8") as fh:
            json.dump(datos, fh, ensure_ascii=False)
        print("  renombrado defenseStat -> defensa_stat en %d cartas" % renombradas)
        if renombradas == 0:
            fallos.append("ninguna carta llevaba defenseStat: la autoprueba no prueba nada")

        rotas, _ = medir(copia)
        regresiones, _ = comparar(rotas, base_sana)
        if not any("defenseStat" in r for r in regresiones):
            fallos.append("renombrar defenseStat NO ha producido regresion")
        else:
            print("  OK: el guardia lo detecta -> %s" % regresiones[0])

        # 2) Vaciar el valor sin quitar la clave: el otro camino, el que un
        #    contador de presencia a secas no ve.
        shutil.rmtree(copia)
        shutil.copytree(CATALOGOS, copia)
        with open(ruta, encoding="utf-8") as fh:
            datos = json.load(fh)
        vaciadas = 0
        for entrada in datos["entries"]:
            if _tiene_valor(entrada.get("defenseStat")):
                entrada["defenseStat"] = ""
                vaciadas += 1
        with open(ruta, "w", encoding="utf-8") as fh:
            json.dump(datos, fh, ensure_ascii=False)
        print("  vaciado defenseStat (sin quitar la clave) en %d cartas" % vaciadas)

        vacias, _ = medir(copia)
        regresiones, _ = comparar(vacias, base_sana)
        if not any("defenseStat" in r and "con valor" in r for r in regresiones):
            fallos.append("vaciar defenseStat NO ha producido regresion de con_valor")
        else:
            print("  OK: el guardia tambien ve el vaciado")

        # 3) Y que una carta NUEVA no haga fallar nada: subir no es romper.
        shutil.rmtree(copia)
        shutil.copytree(CATALOGOS, copia)
        with open(ruta, encoding="utf-8") as fh:
            datos = json.load(fh)
        nueva = dict(datos["entries"][0])
        nueva["id"] = "carta_de_prueba_d10"
        datos["entries"].append(nueva)
        with open(ruta, "w", encoding="utf-8") as fh:
            json.dump(datos, fh, ensure_ascii=False)
        crecida, _ = medir(copia)
        regresiones, mejoras = comparar(crecida, base_sana)
        if regresiones:
            fallos.append("añadir una carta ha hecho fallar el guardia: %s" % regresiones[0])
        elif not mejoras:
            fallos.append("añadir una carta no se ha notado en ninguna medida")
        else:
            print("  OK: añadir contenido no falla (%d mejoras)" % len(mejoras))

    if fallos:
        print("\nAUTOPRUEBA ROJA:")
        for f in fallos:
            print("  - %s" % f)
        return 1
    print("\nAutoprueba verde: el guardia detecta el desapareamiento por las dos vias.")
    return 0


# ---------------------------------------------------------------------

def informe(medida):
    print("%-22s %7s  %s" % ("familia", "entradas", "campos (con_valor/presentes)"))
    for familia, datos in sorted(medida.items()):
        n = datos["entradas"]
        vacios = [c for c, d in datos["campos"].items() if d["presentes"] == 0]
        print("%-22s %7d  %d campos, %d sin ni una entrada"
              % (familia, n, len(datos["campos"]), len(vacios)))
        for campo, d in sorted(datos["campos"].items()):
            marca = " (obligatorio)" if d["obligatorio"] else ""
            print("    %-34s %5d/%-5d%s" % (campo, d["con_valor"], d["presentes"], marca))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--escribir", action="store_true",
                    help="registra la medida de hoy como linea base")
    ap.add_argument("--informe", action="store_true", help="tabla de cobertura")
    ap.add_argument("--autoprueba", action="store_true",
                    help="comprueba que el guardia detecta el desapareamiento")
    args = ap.parse_args()

    if args.autoprueba:
        return autoprueba()

    medida, problemas = medir()

    if args.informe:
        informe(medida)
        print()

    if args.escribir:
        with open(LINEA_BASE, "w", encoding="utf-8") as fh:
            json.dump(medida, fh, ensure_ascii=False, indent=2, sort_keys=True)
            fh.write("\n")
        total = sum(d["entradas"] for d in medida.values())
        print("linea base registrada: %d familias, %d entradas -> %s"
              % (len(medida), total, os.path.relpath(LINEA_BASE, RAIZ)))
        for p in problemas:
            print("  aviso: %s" % p)
        return 0

    if not os.path.isfile(LINEA_BASE):
        print("no hay linea base registrada. Crea una con:")
        print("  python3 tools/validar_catalogos.py --escribir")
        return 1

    with open(LINEA_BASE, encoding="utf-8") as fh:
        base = json.load(fh)
    regresiones, mejoras = comparar(medida, base)

    for p in problemas:
        print("PROBLEMA: %s" % p)
    for r in regresiones:
        print("REGRESION: %s" % r)
    if mejoras:
        print("\n%d mejoras respecto de la linea base (no fallan). Las primeras:" % len(mejoras))
        for m in mejoras[:10]:
            print("  + %s" % m)
        print("  Cuando la migracion del asset este cerrada, vuelve a registrar:")
        print("  python3 tools/validar_catalogos.py --escribir")

    if regresiones or problemas:
        print("\nFALLA: %d regresiones, %d problemas." % (len(regresiones), len(problemas)))
        return 1
    total = sum(d["entradas"] for d in medida.values())
    print("OK: %d familias y %d entradas, sin perdidas respecto de la linea base."
          % (len(medida), total))
    return 0


if __name__ == "__main__":
    sys.exit(main())
