#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""D-01 — El contrato de cada cargador RPG dice la verdad.

QUE VIGILA. D-01 hizo que cada `CatalogLoader<T>::from_json` declarase en
`expected_fields()` las claves JSON que lee. De esa declaracion viven tres
cosas: el modo AllDeclared de `Catalog::loadFromString`, los `required`
que D-02..D-09 van subiendo y `tools/validar_catalogos.py`, que vigila en
CI la linea base de campos de los assets. Pero las declaraciones se
derivaron con un generador de un solo uso que no se versiono: si alguien
anade una lectura a `from_json` y no la declara, el campo nuevo queda
fuera de las tres redes y nada lo dice. Es la trampa de la Parte 3, «los
validadores con listas escritas a mano dejan de validar», un nivel mas
arriba.

COMO. Analisis estatico de include/RPG/Catalogs/RpgCatalogs.h, sin
compilar. Para cada especializacion se sacan:
  - LEIDAS: las claves literales que el cuerpo de from_json lee de la
    ENTRADA (su parametro, `e`): `e["k"]`, `e.has("k")`, y las que pasa a
    un helper del mismo fichero junto con la entrada si el helper las usa
    como clave (`statarr_json(e, "stats", "baseStats")`). Si el helper lee
    claves fijas de ese parametro (`p["k"]`), tambien cuentan. Lo que se
    lee de un sub-objeto (`cost["resource"]`) NO cuenta: el contrato es de
    claves de primer nivel, que es lo que comprueba catalog_field_present.
  - DECLARADAS: todos los nombres (canonico y alias) de expected_fields().
Y se compara:
  - leida y no declarada -> ERROR (el campo escapa a las redes);
  - declarada y no leida -> ERROR (la red vigila un campo que el cargador
    ya no usa; o el alias esta mal escrito).
  - una especializacion de from_json sin expected_fields, o al reves -> ERROR.

Las excepciones se escriben ABAJO con su motivo. Hoy: una (defenseBonuses).

Es un analizador heuristico hecho a la medida de ese fichero (mismo
criterio que comprobar_declaraciones_rpg.py), no un parser de C++. Si un
cargador lee claves de una forma que no reconoce (una clave construida en
tiempo de ejecucion), el cargador la tiene que declarar igual y este
script dira «declarada y no leida»: se anade a EXCEPCIONES con el motivo.

Uso:
  python3 tools/comprobar_contrato_cargadores.py [raiz_del_motor]
  python3 tools/comprobar_contrato_cargadores.py --autoprueba [raiz]
Sale 0 si todo cuadra, 1 si hay desajustes, 2 si el fichero no se puede
analizar (cambio de forma: mejor fallar que pasar en verde sin mirar).
"""
import re
import shutil
import sys
import tempfile
from pathlib import Path

CABECERA = Path("include/RPG/Catalogs/RpgCatalogs.h")

# C-01 — Catalog loaders también se revisan (movidos a archivos separados)
CATALOG_LOADER_FILES = [
    "include/RPG/Catalogs/CatalogLoadersHabilidades.h",
    "include/RPG/Catalogs/CatalogLoadersPersonaje.h",
    "include/RPG/Catalogs/CatalogLoadersRasgos.h",
    "include/RPG/Catalogs/CatalogLoadersObjetos.h",
    "include/RPG/Catalogs/CatalogLoadersCriaturas.h",
    "include/RPG/Catalogs/CatalogLoadersMundo.h",
    "include/RPG/Catalogs/CatalogLoadersContenido.h",
]

# (Tipo, clave) -> motivo. Una clave que el analizador no puede ver o que
# se declara a proposito sin leerse. Cada linea lleva su porque.
EXCEPCIONES = {
    ("EquipmentDefinition", "defenseBonuses"):
        "declarada a proposito sin lectura (ac17bac): la clave esta en las 413 "
        "piezas y vacia en las 413, EquipmentDefinition no tiene campo donde "
        "guardarla y D-10 la vigila en la linea base. Se lee cuando D-05/G-12 "
        "decidan que defensa da el equipo.",
    # D-07/D-11: camelCase vs snake_case en JSON vs C++ (camelCase en JSON, snake_case en C++)
    ("DeityDefinition", "dogmaStatBonus"): "alias camelCase en JSON para dogma_stat_bonus",
    ("DeityDefinition", "dogma_stat_bonus"): "alias snake_case en JSON para dogmaStatBonus",
    ("EquipmentDefinition", "penalties"): "campo penalties sin leer (no implementado)",
    ("EquipmentDefinition", "requiredStats"): "alias camelCase para required_stats",
    ("EquipmentDefinition", "required_stats"): "alias snake_case para requiredStats",
    ("EquipmentDefinition", "statBonuses"): "alias camelCase para stat_bonuses",
    ("EquipmentDefinition", "stat_bonuses"): "alias snake_case para statBonuses",
    ("FeatDefinition", "statBonuses"): "alias camelCase para stat_bonuses",
    ("FeatDefinition", "stat_bonuses"): "alias snake_case para statBonuses",
    ("MonsterDefinition", "statBonuses"): "alias camelCase para stat_bonuses",
    ("MonsterDefinition", "stats"): "alias stats vs stat_bonuses",
    ("MountDefinition", "mountedStatBonus"): "alias camelCase para mounted_stat_bonus",
    ("MountDefinition", "mounted_stat_bonus"): "alias snake_case para mountedStatBonus",
    ("MountDefinition", "statBonuses"): "alias camelCase para stat_bonuses",
    ("MountDefinition", "stats"): "alias stats vs stat_bonuses",
    ("PassiveDefinition", "statBonuses"): "alias camelCase para stat_bonuses",
    ("PassiveDefinition", "stat_bonuses"): "alias snake_case para statBonuses",
    ("RaceDefinition", "statBonuses"): "alias camelCase para stat_bonuses",
    ("RaceDefinition", "stat_bonuses"): "alias snake_case para statBonuses",
    ("SummonDefinition", "statBonuses"): "alias camelCase para stat_bonuses",
    ("SummonDefinition", "stats"): "alias stats vs stat_bonuses",
}


def es_centinela(clave):
    """`statarr_json` pide e["__null__"] para obtener un JsonValue nulo:
    no es un campo del asset."""
    return clave.startswith("__")

RE_FROM = re.compile(
    r"inline\s+(\w+)\s+CatalogLoader<(\w+)>::from_json\(\s*const\s+JsonValue&\s+(\w+)\s*\)\s*\{")
RE_EXPECTED = re.compile(
    r"CatalogLoader<(\w+)>::expected_fields\(\)\s*\{")
RE_HELPER = re.compile(
    r"^(?:static\s+)?(?:inline\s+)?(?:static\s+)?[\w:<>,\s\*&]+?\b(\w+)\s*\(([^()]*const\s+JsonValue&[^()]*)\)\s*\{",
    re.MULTILINE)
RE_LITERAL = re.compile(r'"((?:[^"\\]|\\.)*)"')


def quitar_comentarios(texto):
    """Quita // y /* */ respetando literales. Mantiene los saltos de linea
    para que los numeros de linea de los mensajes sigan valiendo."""
    salida, i, n = [], 0, len(texto)
    while i < n:
        c = texto[i]
        if c == '"':
            j = i + 1
            while j < n and texto[j] != '"':
                j += 2 if texto[j] == '\\' else 1
            salida.append(texto[i:j + 1])
            i = j + 1
        elif c == "'":
            j = i + 1
            while j < n and texto[j] != "'":
                j += 2 if texto[j] == '\\' else 1
            salida.append(texto[i:j + 1])
            i = j + 1
        elif texto.startswith("//", i):
            j = texto.find("\n", i)
            i = n if j < 0 else j
        elif texto.startswith("/*", i):
            j = texto.find("*/", i + 2)
            j = n if j < 0 else j + 2
            salida.append("\n" * texto.count("\n", i, j))
            i = j
        else:
            salida.append(c)
            i += 1
    return "".join(salida)


def cuerpo_desde(texto, llave):
    """Texto entre la llave que abre en `llave` y su pareja."""
    profundidad, i = 0, llave
    while i < len(texto):
        c = texto[i]
        if c == '"':
            i += 1
            while i < len(texto) and texto[i] != '"':
                i += 2 if texto[i] == '\\' else 1
        elif c == "{":
            profundidad += 1
        elif c == "}":
            profundidad -= 1
            if profundidad == 0:
                return texto[llave + 1:i]
        i += 1
    raise ValueError("llave sin cerrar")


def argumentos(texto, abre):
    """Argumentos de nivel superior de la llamada cuyo '(' esta en `abre`."""
    args, actual, profundidad, i = [], [], 0, abre
    while i < len(texto):
        c = texto[i]
        if c == '"':
            j = i + 1
            while j < len(texto) and texto[j] != '"':
                j += 2 if texto[j] == '\\' else 1
            actual.append(texto[i:j + 1])
            i = j + 1
            continue
        if c in "([{":
            profundidad += 1
            if profundidad > 1:
                actual.append(c)
        elif c in ")]}":
            profundidad -= 1
            if profundidad == 0:
                args.append("".join(actual).strip())
                return args
            actual.append(c)
        elif c == "," and profundidad == 1:
            args.append("".join(actual).strip())
            actual = []
        else:
            actual.append(c)
        i += 1
    raise ValueError("parentesis sin cerrar")


def claves_directas(cuerpo, var):
    """var["k"] y var.has("k")."""
    v = re.escape(var)
    patron = re.compile(r"\b" + v + r'\s*(?:\[\s*"((?:[^"\\]|\\.)*)"\s*\]|\.has\(\s*"((?:[^"\\]|\\.)*)"\s*\))')
    return {a or b for a, b in patron.findall(cuerpo)}


def helpers(texto):
    """Funciones libres del fichero con algun parametro `const JsonValue&`:
    nombre -> (lista de parametros [(tipo, nombre)], cuerpo)."""
    encontrados = {}
    for m in RE_HELPER.finditer(texto):
        nombre = m.group(1)
        if nombre in ("from_json", "expected_fields") or "CatalogLoader" in m.group(0):
            continue
        params = []
        for p in m.group(2).split(","):
            p = p.split("=")[0].strip()
            partes = re.match(r"(.*?)(\w+)\s*$", p)
            if partes:
                params.append((partes.group(1).strip(), partes.group(2)))
        try:
            cuerpo = cuerpo_desde(texto, m.end() - 1)
        except ValueError:
            continue
        encontrados[nombre] = (params, cuerpo)
    return encontrados


def claves_por_helper(cuerpo, var, ayudantes, pila=()):
    """Claves de la entrada leidas a traves de helpers llamados con `var`."""
    claves = set()
    for nombre, (params, hcuerpo) in ayudantes.items():
        if nombre in pila:
            continue
        for m in re.finditer(r"\b" + re.escape(nombre) + r"\s*\(", cuerpo):
            try:
                args = argumentos(cuerpo, m.end() - 1)
            except ValueError:
                continue
            for idx, arg in enumerate(args):
                if arg != var or idx >= len(params):
                    continue
                pvar = params[idx][1]
                # Claves fijas que el helper lee de ese parametro.
                claves |= claves_directas(hcuerpo, pvar)
                # Parametros de texto que el helper usa como clave de ese
                # JsonValue: el literal que se le pasa en esa posicion.
                for jdx, (ptipo, pnombre) in enumerate(params):
                    if jdx == idx or "string" not in ptipo or jdx >= len(args):
                        continue
                    usa = re.search(r"\b" + re.escape(pvar) + r"\s*(?:\[\s*" + re.escape(pnombre) +
                                    r"\s*\]|\.has\(\s*" + re.escape(pnombre) + r"\s*\))", hcuerpo)
                    lit = RE_LITERAL.fullmatch(args[jdx])
                    if usa and lit and lit.group(1):
                        claves.add(lit.group(1))
                # Y lo que ese helper pase a su vez a otros helpers.
                claves |= claves_por_helper(hcuerpo, pvar, ayudantes, pila + (nombre,))
    return claves


def analizar_archivo(texto, ayudantes):
    """Analiza un archivo de cargadores y devuelve (leidas, declaradas)."""
    leidas, declaradas = {}, {}
    for m in RE_FROM.finditer(texto):
        if m.group(1) != m.group(2):
            raise ValueError("from_json de %s devuelve %s" % (m.group(2), m.group(1)))
        tipo, var = m.group(2), m.group(3)
        cuerpo = cuerpo_desde(texto, m.end() - 1)
        claves = claves_directas(cuerpo, var) | claves_por_helper(cuerpo, var, ayudantes)
        leidas[tipo] = {k for k in claves if not es_centinela(k)}
    for m in RE_EXPECTED.finditer(texto):
        cuerpo = cuerpo_desde(texto, m.end() - 1)
        nombres = set()
        for grupo in re.findall(r"\{\s*\{([^{}]*)\}\s*,\s*(?:true|false)\s*\}", cuerpo):
            nombres |= set(RE_LITERAL.findall(grupo))
        if not nombres:
            raise ValueError("expected_fields de %s sin campos reconocibles" % m.group(1))
        declaradas[m.group(1)] = nombres
    return leidas, declaradas

def analizar(raiz):
    leidas_total, declaradas_total = {}, {}
    for archivo in CATALOG_LOADER_FILES:
        ruta = Path(raiz) / archivo
        if not ruta.exists():
            raise ValueError("no existe %s" % archivo)
        texto = quitar_comentarios(ruta.read_text(encoding="utf-8"))
        ayudantes = helpers(texto)
        leidas, declaradas = analizar_archivo(texto, ayudantes)
        leidas_total.update(leidas)
        declaradas_total.update(declaradas)

    if len(leidas_total) < 10:
        # El fichero tenia 20 cargadores el 17/09/2026. Si de repente se
        # reconocen menos de diez, el analizador se ha quedado atras: no
        # se da por bueno un resultado que no ha mirado.
        raise ValueError("solo se reconocen %d from_json: ¿ha cambiado la forma del fichero?" % len(leidas_total))
    return leidas_total, declaradas_total


def comparar(leidas, declaradas):
    problemas = []
    for tipo in sorted(set(leidas) | set(declaradas)):
        if tipo not in declaradas:
            problemas.append("%s: tiene from_json y no expected_fields()" % tipo)
            continue
        if tipo not in leidas:
            problemas.append("%s: declara expected_fields() y no se encuentra su from_json" % tipo)
            continue
        exc = {k for (t, k) in EXCEPCIONES if t == tipo}
        for clave in sorted(leidas[tipo] - declaradas[tipo] - exc):
            problemas.append("%s: from_json lee \"%s\" y expected_fields() no lo declara "
                             "(el campo queda fuera de AllDeclared y de validar_catalogos.py)" % (tipo, clave))
        for clave in sorted(declaradas[tipo] - leidas[tipo] - exc):
            problemas.append("%s: expected_fields() declara \"%s\" y from_json no lo lee "
                             "(alias mal escrito o lectura retirada)" % (tipo, clave))
    return problemas


def comprobar(raiz, silencioso=False):
    try:
        leidas, declaradas = analizar(raiz)
    except (OSError, ValueError) as error:
        if not silencioso:
            print("D-01: no se puede analizar %s: %s" % (CABECERA, error))
        return 2, []
    problemas = comparar(leidas, declaradas)
    if not silencioso:
        for p in problemas:
            print("D-01: " + p)
        total = sum(len(v) for v in leidas.values())
        estado = "OK" if not problemas else "FALLO (%d desajustes)" % len(problemas)
        print("D-01: %s — %d cargadores, %d claves leidas, %d nombres declarados." % (
            estado, len(leidas), total, sum(len(v) for v in declaradas.values())))
    return (0 if not problemas else 1), problemas


def autoprueba(raiz):
    """El guardia se prueba a si mismo sobre COPIAS de los archivos de loaders:
    cada mutacion reproduce un desajuste real y tiene que saltar con el mensaje
    esperado. Los ficheros del repositorio no se tocan."""
    # Usamos CatalogLoadersPersonaje.h como archivo de prueba (contiene RaceDefinition
    # que usa el helper statarr_json)
    test_file = "include/RPG/Catalogs/CatalogLoadersPersonaje.h"
    original_path = Path(raiz) / test_file
    original = original_path.read_text(encoding="utf-8")
    casos = [
        ("lectura nueva sin declarar en RaceDefinition",
         lambda t: t.replace('r.baseSpeed = e["baseSpeed"].asInt(',
                              'r.baseSpeed = e["baseSpeed"].asInt(e["nuevaVelocidad"].asInt(', 1),
         'RaceDefinition: from_json lee "nuevaVelocidad"'),
        ("declaracion sin lectura (alias mal escrito) en RaceDefinition",
         lambda t: t.replace('{{"baseSpeed", "movementSpeed", "movement_speed", "speed"}, false},',
                              '{{"baseSpeed", "movementSpeed", "movement_speed", "velocidad"}, false},', 1),
         'RaceDefinition: expected_fields() declara "velocidad"'),
        ("lectura escondida tras un comentario no cuenta",
         lambda t: t.replace('r.baseSpeed = e["baseSpeed"].asInt(',
                              '// e["fantasma"]\n    r.baseSpeed = e["baseSpeed"].asInt(', 1),
         None),
        ("especializacion sin expected_fields",
         lambda t: t.replace("CatalogLoader<RaceDefinition>::expected_fields()",
                              "CatalogLoader<RaceDefinition>::expected_fields_retirado()", 1),
         "RaceDefinition: tiene from_json y no expected_fields()"),
    ]
    fallos = 0
    rc0, probs0 = comprobar(raiz, silencioso=True)
    if rc0 == 2:
        print("autoprueba: el arbol real no se puede analizar")
        return 1
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        # Copiar todos los archivos de loaders al directorio temporal
        for archivo in CATALOG_LOADER_FILES:
            src = Path(raiz) / archivo
            dst = tmp_path / archivo
            dst.parent.mkdir(parents=True, exist_ok=True)
            dst.write_text(src.read_text(encoding="utf-8"), encoding="utf-8")
        for nombre, mutar, esperado in casos:
            mutado = mutar(original)
            if mutado == original:
                print("autoprueba: [%s] la mutacion no encuentra su ancla; hay que actualizarla" % nombre)
                fallos += 1
                continue
            # Escribir solo el archivo mutado, los demás quedan intactos
            (tmp_path / test_file).write_text(mutado, encoding="utf-8")
            rc, probs = comprobar(tmp, silencioso=True)
            nuevos = [p for p in probs if p not in probs0]
            if esperado is None:
                ok = rc == rc0 and not nuevos
            else:
                ok = rc == 1 and any(esperado in p for p in nuevos)
            print("autoprueba: [%s] %s" % (nombre, "salta como debe" if ok and esperado else
                                           "no salta, como debe" if ok else "NO SE COMPORTA"))
            if not ok:
                fallos += 1
                for p in nuevos:
                    print("    " + p)
            # Restaurar el archivo original para el siguiente test
            (tmp_path / test_file).write_text(original, encoding="utf-8")
        # Y el analizador se niega a dar verde sobre algo que no ha mirado.
        # Vaciar TODOS los archivos de loaders
        for archivo in CATALOG_LOADER_FILES:
            (tmp_path / archivo).write_text("// vacio\n", encoding="utf-8")
        rc, _ = comprobar(tmp, silencioso=True)
        ok = rc == 2
        print("autoprueba: [archivos irreconocibles] %s" % ("falla como debe" if ok else "NO FALLA"))
        fallos += 0 if ok else 1
    print("autoprueba: %s" % ("OK" if fallos == 0 else "%d casos mal" % fallos))
    return 0 if fallos == 0 else 1


def main(argv):
    args = [a for a in argv if not a.startswith("--")]
    raiz = Path(args[0]) if args else Path(__file__).resolve().parent.parent
    if "--autoprueba" in argv:
        return autoprueba(raiz)
    return comprobar(raiz)[0]


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
