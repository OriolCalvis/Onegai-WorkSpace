#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""analizar_duplicados.py — ¿hay otro sitio que ya hace esto? ¿qué está muerto?

POR QUÉ EXISTE. El árbol lo escriben varias familias de agentes en paralelo y
sin un registro central de intenciones: el resultado es que la misma capacidad
aparece dos veces (dos validadores de catálogos, dos índices de proyecto, dos
menús de inventario), o aparece una vez y muere sin que nadie la llame. Las dos
cosas cuestan lo mismo: trabajo pagado dos veces o trabajo pagado y tirado.
Hoy eso se descubre por arqueología de sesión. Esto lo convierte en un informe
reproducible que dice QUÉ se repite, QUÉ diferencia a los que se parecen, y
QUÉ PASA si se toca.

QUÉ MIDE (cada hallazgo sale con evidencia, veredicto y afectación):
  1. DUPLICADOS EXACTOS. Mismo sha256, y los "* 2*" que cuela macOS (Parte 3).
  2. CLONES DE CÓDIGO. Shingles de tokens normalizados (sin comentarios ni
     literales) + Jaccard. Distingue clon duro, clon parcial y ENVOLTORIO
     (los shingles de A están dentro de B y A es mucho menor: A delega o copia).
  3. SOLAPAMIENTO FUNCIONAL DE SCRIPTS. Dos tools/*.py pueden no parecerse en
     el texto y hacer lo mismo. Se comparan por FIRMA: qué rutas leen, qué
     claves de JSON comprueban, qué otros scripts lanzan y qué funciones
     definen. Si la firma coincide y el texto no, el informe lo dice y exige
     justificar por qué conviven.
  4. API DUPLICADA Y CABECERAS ENVOLTORIO. Un tipo declarado en dos cabeceras
     distintas; cabeceras que solo reexportan otra.
  5. CATÁLOGOS GEMELOS. Dos JSON de assets/ con el mismo conjunto de ids.
  6. MUERTO Y DESACTUALIZADO. Cabeceras que no alcanza ningún ejecutable,
     registros de seguimiento "closed" cuyo producto no llega a nadie, y docs
     cuyo código citado cambió después de la última vez que se tocó el doc.
  7. DISÍMILARES. Pares con nombre parecido que NO son clones: el informe los
     lista aparte y dice en qué se diferencian, para que nadie los "arregle"
     fusionándolos. Es la mitad del trabajo: la otra mitad es no tocar lo que
     ya está bien separado.

QUÉ NO MIDE, PORQUE YA LO MIDEN OTROS (y duplicarlo sería el primer pecado):
  - Qué código llega al jugador y al autor: `medir_alcance.py`. Este script
    CONSUME su `planificacion/alcance_real.json` si existe.
  - Declaraciones sin cuerpo: `comprobar_declaraciones_rpg.py` (C-22).
  - Contenido roto, referencias cruzadas y puertas: `validar_todo.py`.
  - Paquetes con seguimiento escrito y código sin versionar:
    `detectar_wip_huerfano.py`.
  Si uno de esos cambia de formato, este script degrada el aviso en vez de
  inventarse un resultado.

LÍMITES. Es un analizador heurístico de texto, de la misma familia que los
anteriores: no compila, no resuelve plantillas ni macros, y la similitud de
tokens no entiende intención. Por eso ningún hallazgo sale sin su evidencia y
sin el veredicto de una regla escrita (VEREDICTOS, abajo): el informe propone,
la decisión es de Oriol o del dueño de la fila.

Uso:
  python3 tools/analizar_duplicados.py [raiz] [--json SALIDA] [--md SALIDA]
                                       [--fichero RUTA] [--comprobar]
                                       [--umbral-clon 0.70] [--solo TIPO]
"""
from __future__ import annotations

import hashlib
import json
import re
import subprocess
import sys
from collections import defaultdict
from difflib import SequenceMatcher
from pathlib import Path

# ---------------------------------------------------------------------
# Alcance del análisis
# ---------------------------------------------------------------------

EXT_CODIGO = {".py", ".cpp", ".cc", ".c", ".h", ".hpp", ".sh", ".cmake"}
EXT_CANONICO = {".py", ".cpp", ".cc", ".c", ".h", ".hpp"}
EXT_TEXTO = EXT_CODIGO | {".json", ".md", ".puml", ".txt", ".yml", ".yaml"}

# Directorios que no son fuente de verdad: salidas, vendores y papeleras.
EXCLUIR_PARTES = (
    ".git", "build", "builds", "out", "third_party", "__pycache__",
    "_to_delete", ".papelera_sandbox", "node_modules", "Testing",
    "entregas",   # salidas de tools/exportar.py, como builds/ lo es de build_proyecto.py

)
EXCLUIR_PREFIJOS = ("planificacion/render/", "planificacion/analisis/")
# salidas de este mismo script: si se analizaran, cada rerun cambiaría el informe
EXCLUIR_FICHEROS = ("planificacion/duplicados.json",
                    "planificacion/INFORME_DUPLICADOS_")

UMBRAL_DUPLICADO = 0.90   # clon duro: mismo código, dos sitios
UMBRAL_CLON = 0.70        # clon parcial: trozos copiados, hay que extraer
UMBRAL_ESTRUCTURAL = 0.85 # misma forma con otros nombres: plantilla repetida
UMBRAL_ENVOLTORIO = 0.75  # A cabe casi entero dentro de B, y B le dobla en tamaño
MIN_SHINGLES_ENVOLTORIO = 8  # por debajo de esto, cualquier par de ficheros pequeños "cabe"
UMBRAL_FIRMA = 0.55       # dos scripts hacen lo mismo aunque no se parezcan
UMBRAL_NOMBRE = 0.72      # nombre bastante parecido para confundir a alguien

# Reglas de veredicto, escritas aquí para que el informe las pueda citar.
VEREDICTOS = {
    "DUPLICADO": "Mismo código en dos sitios. Se borra uno y el otro queda como único dueño; si los dos tienen consumidores, se extrae el común y ambos delegan.",
    "CLON_ESTRUCTURAL": "Misma forma con otros nombres. O es copia-y-renombra del mismo asunto, o es una plantilla repetida con asuntos distintos: se distingue leyendo los nombres. Si son 3 o más con la misma forma, la salida es una plantilla o un generador, no borrar ficheros.",
    "CLON_PARCIAL": "Trozos copiados. Se extrae la parte común a un solo sitio; lo que difiere se queda donde está y se documenta por qué difiere.",
    "ENVOLTORIO": "A es un subconjunto de B. Legítimo si A delega de verdad (una línea que llama a B); es deuda si A reimplementa. El informe dice cuál de los dos casos es.",
    "ORACULO_HISTORICO_DE_PRUEBA": "Una prueba conserva una implementación histórica como oráculo de comportamiento. No es lógica de producto duplicada: el registro enumera el par exacto, el motivo, el oráculo y la prueba que lo vigila.",
    "SOLAPAMIENTO_FUNCIONAL": "Dos herramientas con la misma firma (leen lo mismo y comprueban las mismas claves) aunque el texto difiera. O una llama a la otra, o sobra una.",
    "NOMBRE_AMBIGUO": "El mismo nombre corto en dos módulos distintos (Render::Entity y Editor::Entity). No es un conflicto de compilación, es una trampa de lectura: en un árbol que escriben varios agentes a la vez, el nombre dice de quién es la responsabilidad. Se renombra uno con el prefijo de su módulo.",
    "LOGICA_EN_EL_TEST": "El registro del paquete promete unos nombres que solo están definidos dentro de su fichero de prueba: la prueba se prueba a sí misma y ningún ejecutable puede llamarlos. Se mueve la lógica a src/ y se reabre el registro.",
    "API_DUPLICADA": "El mismo tipo declarado en dos cabeceras: dos definiciones que pueden divergir en silencio. Una debe ganar y la otra reexportar o desaparecer.",
    "CABECERA_ENVOLTORIO": "Cabecera que solo reexporta otra. Se mantiene si existe por compatibilidad de rutas y se dice en un comentario; si no, los consumidores incluyen la real.",
    "CATALOGO_GEMELO": "Dos catálogos con los mismos ids: dos fuentes de verdad para el mismo contenido. Gana la que declara el manifiesto del proyecto.",
    "MUERTO": "Ningún ejecutable lo alcanza. O se cablea (y entonces hace falta la tarea), o se retira. Dejarlo cuesta: cada sesión lo vuelve a leer y a dudar.",
    "REGISTRO_CERRADO_SIN_ALCANCE": "El seguimiento dice cerrado y el producto no llega a nadie. Se reabre el registro: la regla de cierre la decide Oriol (ver decisiones/D0006).",
    "DOC_DESACTUALIZADO": "El doc cita código que cambió después. O se actualiza, o se fecha y se declara histórico en la cabecera (como la Parte 0 de la bitácora).",
    "DISIMILAR": "Se parecen de nombre, no de función. NO se fusionan: el informe dice qué hace cada uno para que la próxima sesión no vuelva a dudarlo.",
}

SEVERIDAD = {
    "DUPLICADO": "alta", "CLON_PARCIAL": "alta", "API_DUPLICADA": "alta",
    "LOGICA_EN_EL_TEST": "alta", "CLON_ESTRUCTURAL": "baja",
    "NOMBRE_AMBIGUO": "media",
    "REGISTRO_CERRADO_SIN_ALCANCE": "alta", "MUERTO": "media",
    "SOLAPAMIENTO_FUNCIONAL": "media", "CATALOGO_GEMELO": "media",
    "ENVOLTORIO": "media", "CABECERA_ENVOLTORIO": "baja",
    "DOC_DESACTUALIZADO": "baja", "DISIMILAR": "info",
    "ORACULO_HISTORICO_DE_PRUEBA": "info",
}


class Fichero:
    __slots__ = ("ruta", "rel", "rol", "ext", "bytes", "sha", "texto",
                 "lineas", "tokens", "shingles", "shingles_par", "firma", "stem")

    def __init__(self, ruta: Path, raiz: Path):
        self.ruta = ruta
        self.rel = str(ruta.relative_to(raiz))
        self.ext = ruta.suffix.lower()
        self.stem = ruta.stem
        self.rol = rol_de(self.rel)
        crudo = ruta.read_bytes()
        self.bytes = len(crudo)
        self.sha = hashlib.sha256(crudo).hexdigest()
        try:
            self.texto = crudo.decode("utf-8")
        except UnicodeDecodeError:
            self.texto = crudo.decode("utf-8", "replace")
        self.lineas = self.texto.count("\n") + 1
        self.tokens = []
        self.shingles = frozenset()
        self.shingles_par = frozenset()
        self.firma = {}


def rol_de(rel: str) -> str:
    """Para quién es este fichero: decide la severidad de un hallazgo."""
    if rel.startswith(("tests/", "examples/")) or "/tests/" in rel:
        return "prueba"
    if rel.startswith(("include/", "src/", "editor/include/", "editor/src/")):
        return "produccion"
    if rel.startswith("tools/") or rel.startswith("planificacion/pla/"):
        return "herramienta"
    if rel.startswith("assets/"):
        return "contenido"
    if rel.startswith("cmake/") or rel.endswith("CMakeLists.txt"):
        return "build"
    if rel.endswith((".md", ".puml")):
        return "plan"
    return "otro"


def dentro_del_analisis(rel: str) -> bool:
    partes = set(rel.split("/"))
    if partes & set(EXCLUIR_PARTES):
        return False
    if rel.startswith(EXCLUIR_PREFIJOS) or rel.startswith(EXCLUIR_FICHEROS):
        return False
    return True


def inventario(raiz: Path):
    """Todo fichero de texto que no sea una salida de build."""
    out = []
    for ruta in sorted(raiz.rglob("*")):
        if not ruta.is_file():
            continue
        rel = str(ruta.relative_to(raiz))
        if not dentro_del_analisis(rel):
            continue
        if ruta.suffix.lower() not in EXT_TEXTO and ruta.name != "CMakeLists.txt":
            continue
        if ruta.suffix.lower() == ".txt" and ruta.name != "CMakeLists.txt":
            continue
        try:
            out.append(Fichero(ruta, raiz))
        except OSError:
            continue
    return out


# ---------------------------------------------------------------------
# Normalización: comparar código, no formato ni comentarios
# ---------------------------------------------------------------------

_RE_COM_BLOQUE = re.compile(r"/\*.*?\*/", re.S)
_RE_COM_LINEA = re.compile(r"//[^\n]*")
_RE_LITERAL_C = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
_RE_TOKEN = re.compile(r"[A-Za-z_]\w*|\d+\.?\d*|[^\sA-Za-z_0-9]")

K_C = 8   # shingles de C/C++: 8 tokens ~ una sentencia corta
K_PY = 6


def tokens_de(f: Fichero):
    """Tokens normalizados. En Python se usa `tokenize` (sabe qué es un
    comentario y qué un literal); si el fichero no parsea —y aquí pasa, con
    WIP en vuelo— se cae a la vía regex sin abortar el análisis."""
    if f.ext == ".py":
        try:
            import io
            import tokenize
            out = []
            gen = tokenize.generate_tokens(io.StringIO(f.texto).readline)
            for tok in gen:
                if tok.type in (tokenize.COMMENT, tokenize.NL, tokenize.NEWLINE,
                                tokenize.INDENT, tokenize.DEDENT, tokenize.ENCODING):
                    continue
                if tok.type == tokenize.STRING:
                    out.append("STR")
                elif tok.type == tokenize.ENDMARKER:
                    continue
                else:
                    out.append(tok.string)
            if out:
                return out
        except Exception:
            pass
        t = re.sub(r'(?s)("""|\'\'\').*?\1', " STR ", f.texto)
        t = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', "STR", t)
        t = re.sub(r"#[^\n]*", " ", t)
        return _RE_TOKEN.findall(t)
    if f.ext in (".cpp", ".cc", ".c", ".h", ".hpp"):
        t = _RE_COM_BLOQUE.sub(" ", f.texto)
        t = _RE_COM_LINEA.sub(" ", t)
        t = _RE_LITERAL_C.sub("STR", t)
        return _RE_TOKEN.findall(t)
    if f.ext in (".sh", ".cmake") or f.ruta.name == "CMakeLists.txt":
        t = re.sub(r"(?m)^\s*#[^\n]*", " ", f.texto)
        t = _RE_LITERAL_C.sub("STR", t)
        return _RE_TOKEN.findall(t)
    return []


# Nombres que NO se parameterizan: si `std`, `print` o `int` se convirtieran en
# IDn, dos ficheros cualesquiera con la misma forma darían iguales.
RESERVADAS_C = set("""
alignas alignof and and_eq auto bitand bitor bool break case catch char char8_t
char16_t char32_t class compl const consteval constexpr constinit const_cast
continue co_await co_return co_yield decltype default delete do double
dynamic_cast else enum explicit export extern false float for friend goto if
inline int long mutable namespace new noexcept not not_eq nullptr operator or
or_eq private protected public register reinterpret_cast requires return short
signed sizeof static static_assert static_cast struct switch template this
thread_local throw true try typedef typeid typename union unsigned using
virtual void volatile wchar_t while xor xor_eq override final
std string string_view vector map unordered_map set array size_t int8_t int16_t
int32_t int64_t uint8_t uint16_t uint32_t uint64_t pair shared_ptr unique_ptr
printf memcpy memset main
""".split())

RESERVADAS_PY = set("""
and as assert async await break class continue def del elif else except False
finally for from global if import in is lambda None nonlocal not or pass raise
return True try while with yield
print len range open sorted set list dict tuple int str float bool bytes enumerate
zip min max sum any all isinstance issubclass super type repr abs round reversed
Path json re sys os subprocess argparse hashlib tempfile shutil time keyword self cls
__name__ __file__ __init__ __slots__ main
""".split())

_RE_IDENT = re.compile(r"^[A-Za-z_]\w*$")
_RE_NUM = re.compile(r"^\d+\.?\d*$")


def canonizar(tokens, ext):
    """Segunda vista del fichero: los identificadores de usuario pasan a IDn
    por orden de aparición y los números a NUM. Es lo que permite ver el clon
    de verdad —el que se copió y se renombró—, que en la vista literal no se
    parece en nada (Jaccard 0,08 en el caso sembrado de la prueba)."""
    reservadas = RESERVADAS_PY if ext == ".py" else RESERVADAS_C
    mapa = {}
    out = []
    for tok in tokens:
        if tok == "STR":
            out.append("STR")
        elif _RE_IDENT.match(tok):
            if tok in reservadas:
                out.append(tok)
            else:
                if tok not in mapa:
                    mapa[tok] = "ID%d" % len(mapa)
                out.append(mapa[tok])
        elif _RE_NUM.match(tok):
            out.append("NUM")
        else:
            out.append(tok)
    return out


def shingles_de(tokens, k):
    if len(tokens) < k:
        return frozenset()
    return frozenset(
        hash("\x1f".join(tokens[i:i + k])) for i in range(len(tokens) - k + 1)
    )


def preparar(ficheros):
    """Una sola pasada: tokens, shingles literales y shingles canonizados."""
    for f in ficheros:
        if f.ext not in EXT_CODIGO:
            continue
        f.tokens = tokens_de(f)
        k = K_PY if f.ext == ".py" else K_C
        f.shingles = shingles_de(f.tokens, k)
        # En .cmake/.sh los nombres SON el contenido (target, fuentes,
        # etiquetas): parameterizarlos convierte los 55 ficheros de test en
        # clones entre sí. La vista canonizada es solo para C++ y Python.
        if f.ext in EXT_CANONICO:
            f.shingles_par = shingles_de(canonizar(f.tokens, f.ext), k)


def jaccard(a, b):
    if not a or not b:
        return 0.0
    inter = len(a & b)
    return inter / (len(a) + len(b) - inter)


def parecidos_por_nombre(a: str, b: str) -> float:
    """Similitud de nombres de fichero, que es como la gente los confunde."""
    def norm(s):
        s = re.sub(r"(?i)(tests?|_2|2)$", "", s.lower())
        return re.sub(r"[^a-z0-9]", "", s)
    return SequenceMatcher(None, norm(a), norm(b)).ratio()


# ---------------------------------------------------------------------
# Firma funcional de un script: qué hace, no cómo está escrito
# ---------------------------------------------------------------------

_RE_RUTA_LIT = re.compile(
    r"""["']((?:[A-Za-z0-9_.{}*-]+/)*[A-Za-z0-9_.{}*-]+\.(?:json|py|sh|cpp|h|cmake|puml|md|tmx|yml))["']""")
_RE_DIR_LIT = re.compile(
    r"""["']((?:assets|include|src|tools|tests|examples|planificacion|editor|cmake|schemas|data)/[A-Za-z0-9_./{}*-]*)["']""")
_RE_GLOB = re.compile(r"""(?:rglob|glob)\(\s*["']([^"']+)["']""")
# `RAIZ / "assets" / "catalogs"`: cada segmento suelto de una cadena de Path.
# Sin esto, dos scripts que leen la misma carpeta escrita de dos maneras
# (`"assets/catalogs"` y `/ "assets" / "catalogs"`) no se parecen en nada.
_RE_SEGMENTO = re.compile(r"""[/(]\s*["']([A-Za-z0-9_.\- ]+)["']""")
_RE_CLAVE = re.compile(r"""(?:\[\s*|\.get\(\s*)["']([A-Za-z_][A-Za-z0-9_]*)["']""")
# un literal entrecomillado con forma de nombre de campo: es lo que de verdad
# dice QUÉ comprueba un validador, esté o no dentro de un `["..."]`
_RE_LIT_IDENT = re.compile(r"""["']([A-Za-z_][A-Za-z0-9_]{2,23})["']""")
STOP_LITERALES = set("""
json utf-8 yaml rb wb append extend items keys values get set default
true false none null yes no ok fail error warning info debug __main__
""".split())
_RE_DEF = re.compile(r"(?m)^\s*def\s+([A-Za-z_]\w*)")
_RE_CLASE = re.compile(r"(?m)^\s*class\s+([A-Za-z_]\w*)")
_RE_SUBPROC = re.compile(r"subprocess|check_call|check_output|os\.system|Popen")

PESOS_FIRMA = (("lee", 0.35), ("claves", 0.30), ("def", 0.20), ("lanza", 0.15))


_RE_COMPONENTE = re.compile(r"^[a-z0-9_.\-*]{2,40}$")


def _componentes(rutas):
    """`assets/catalogs` y `BASE / "assets" / "catalogs"` son la misma entrada
    escrita de dos maneras: se comparan por componentes, no por la cadena.
    El componente tiene que parecerse a un trozo de ruta: sin el filtro, la
    tupla `("CON", "DES")` entraba como si `CON` fuera un directorio."""
    out = set()
    for r in rutas:
        for parte in str(r).strip("/").split("/"):
            if parte in (".", ".."):
                continue
            if _RE_COMPONENTE.match(parte):
                out.add(parte)
    return out


def firma_de(f: Fichero):
    """Conjunto de intenciones observables de un script."""
    texto = f.texto
    lee = _componentes(set(_RE_RUTA_LIT.findall(texto))
                       | set(_RE_DIR_LIT.findall(texto))
                       | set(_RE_GLOB.findall(texto))
                       | set(_RE_SEGMENTO.findall(texto)))
    # las rutas propias (el nombre del script) no cuentan como entrada
    lee.discard(f.ruta.name)
    # lo que ya se ha reconocido como ruta no puede contar también como campo:
    # si no, `catalogs` aparece en las dos listas y la firma se infla sola
    claves = (set(_RE_CLAVE.findall(texto)) | {
        x for x in _RE_LIT_IDENT.findall(texto)
        if x.lower() not in STOP_LITERALES and "/" not in x and "." not in x}) - lee
    defs = set(_RE_DEF.findall(texto)) | set(_RE_CLASE.findall(texto))
    lanza = {r.rsplit("/", 1)[-1] for r in
             set(_RE_RUTA_LIT.findall(texto)) if r.endswith((".py", ".sh"))}
    lanza.discard(f.ruta.name)
    if _RE_SUBPROC.search(texto):
        lanza.add("<subprocess>")
    f.firma = {"lee": lee, "claves": claves, "def": defs, "lanza": lanza,
               "lineas": f.lineas, "usa_subprocess": bool(_RE_SUBPROC.search(texto))}
    return f.firma


def similitud_firma(a: dict, b: dict) -> float:
    total = 0.0
    for clave, peso in PESOS_FIRMA:
        A, B = a.get(clave, set()), b.get(clave, set())
        if not A and not B:
            continue
        total += peso * (len(A & B) / len(A | B))
    return round(total, 4)


def diferencia_firma(a: dict, b: dict, na: str, nb: str) -> dict:
    """La justificación: qué comprueba cada uno que el otro no."""
    out = {}
    for clave, _ in PESOS_FIRMA:
        A, B = a.get(clave, set()), b.get(clave, set())
        out[clave] = {"solo_en": na, "solo_en_b": nb,
                      na: sorted(A - B)[:12], nb: sorted(B - A)[:12]}
    return out


# ---------------------------------------------------------------------
# Hallazgos
# ---------------------------------------------------------------------

def hallazgo(tipo, archivos, evidencia, umbral=None, extra=None):
    h = {
        "id": "%s-%s" % (tipo, hashlib.sha1("|".join(sorted(archivos)).encode()).hexdigest()[:8]),
        "tipo": tipo,
        "severidad": SEVERIDAD.get(tipo, "media"),
        "archivos": sorted(archivos),
        "evidencia": evidencia,
        "regla": VEREDICTOS.get(tipo, ""),
    }
    if umbral is not None:
        h["similitud"] = umbral
    if extra:
        h.update(extra)
    return h


def fase_exactos(ficheros):
    por_sha = defaultdict(list)
    for f in ficheros:
        por_sha[f.sha].append(f)
    out = []
    for sha, grupo in sorted(por_sha.items()):
        if len(grupo) < 2:
            continue
        # un fichero vacío duplicado no es un hallazgo, es ruido
        if grupo[0].bytes == 0:
            continue
        rutas = [g.rel for g in grupo]
        ev = {"sha256": sha[:16], "bytes": grupo[0].bytes,
              "roles": sorted({g.rol for g in grupo})}
        out.append(hallazgo("DUPLICADO", rutas, ev, 1.0))
    # los " 2" de macOS: mismo nombre con un 2 colgado (Parte 3 de la bitácora)
    nombres = {f.rel for f in ficheros}
    for f in ficheros:
        m = re.match(r"^(.*) 2(\.[A-Za-z0-9]+)?$", f.ruta.name)
        if not m:
            continue
        base = f.rel.replace(f.ruta.name, m.group(1) + (m.group(2) or ""))
        ev = {"motivo": "nombre con ' 2' (copia de macOS/Finder)",
              "original_presente": base in nombres, "original": base}
        out.append(hallazgo("DUPLICADO", [f.rel], ev, None,
                            {"subtipo": "copia_macos"}))
    return out


def candidatos_a_clon(ficheros, max_df=24):
    """Índice invertido de shingles. Solo generan candidatos los shingles poco
    frecuentes: los que salen en 24 ficheros son plantilla (el esqueleto de un
    test, el main de un demo) y pondrían todo el árbol en la cola de pares."""
    idx = defaultdict(list)
    for f in ficheros:
        for s in set(f.shingles) | set(f.shingles_par):
            idx[s].append(f)
    pares = set()
    for s, grupo in idx.items():
        if len(grupo) < 2 or len(grupo) > max_df:
            continue
        for i in range(len(grupo)):
            for j in range(i + 1, len(grupo)):
                a, b = grupo[i], grupo[j]
                if a.ext == ".py" or b.ext == ".py":
                    if a.ext != b.ext:
                        continue   # no comparamos Python con C++
                pares.add((a.rel, b.rel) if a.rel < b.rel else (b.rel, a.rel))
    return pares


def fase_clones(ficheros, umbral_clon, excepciones_duplicados=None):
    por_rel = {f.rel: f for f in ficheros}
    out = []
    for ra, rb in sorted(candidatos_a_clon(ficheros)):
        a, b = por_rel[ra], por_rel[rb]
        sim_lit = jaccard(a.shingles, b.shingles)
        sim_par = jaccard(a.shingles_par, b.shingles_par)
        sim = max(sim_lit, sim_par)
        def _cont(x, y):
            return len(x & y) / max(1, min(len(x), len(y)))
        conten_lit = _cont(a.shingles, b.shingles)
        conten_par = _cont(a.shingles_par, b.shingles_par)
        conten = max(conten_lit, conten_par)
        vista = ("literal" if max(sim_lit, conten_lit) >= max(sim_par, conten_par)
                 else "tras renombrado")
        menor, mayor = (a, b) if len(a.tokens) <= len(b.tokens) else (b, a)
        # ENVOLTORIO se juzga por CONTENCIÓN, no por Jaccard: un fichero que
        # cabe entero dentro de otro mayor siempre da un Jaccard bajo, y si se
        # filtrara antes por similitud esta rama no se ejecutaría nunca.
        envoltorio = (conten >= UMBRAL_ENVOLTORIO and menor.lineas * 2 <= mayor.lineas
                      and len(menor.shingles) >= MIN_SHINGLES_ENVOLTORIO)
        if sim < umbral_clon and not envoltorio:
            continue
        # DUPLICADO solo con coincidencia LITERAL: si lo que coincide es la
        # forma (parameterizada) y no el texto, no es una copia que se borra,
        # es una plantilla repetida — y llamarla "duplicado" manda a borrar
        # ficheros que hacen cosas distintas (TextureManager vs ShaderManager).
        if envoltorio:
            tipo = "ENVOLTORIO"
        elif sim_lit >= UMBRAL_DUPLICADO:
            tipo = "DUPLICADO"
        elif sim_lit >= umbral_clon:
            tipo = "CLON_PARCIAL"
        elif sim_par >= UMBRAL_ESTRUCTURAL:
            tipo = "CLON_ESTRUCTURAL"
        elif sim_par >= umbral_clon:
            tipo = "CLON_PARCIAL"
        else:
            tipo = "ENVOLTORIO" if envoltorio else None
        if tipo is None:
            continue
        ev = {"jaccard": round(sim, 3), "contencion": round(conten, 3),
              "jaccard_literal": round(sim_lit, 3),
              "jaccard_tras_renombrado": round(sim_par, 3),
              "coincide_por": vista,
              "shingles_comunes": len(a.shingles & b.shingles),
              "lineas": {a.rel: a.lineas, b.rel: b.lineas},
              "roles": sorted({a.rol, b.rol})}
        extra = {}
        if envoltorio:
            delega = mayor.rel in menor.texto or mayor.ruta.stem in menor.texto
            tipo = "ENVOLTORIO"
            ev["menor"] = menor.rel
            ev["mayor"] = mayor.rel
            ev["delega_de_verdad"] = bool(delega)
            extra["justificacion"] = (
                "%s cabe al %.0f%% dentro de %s y tiene la mitad de líneas. "
                "%s" % (menor.rel, conten * 100, mayor.rel,
                        "Menciona a %s: parece delegación legítima." % mayor.ruta.name
                        if delega else
                        "No menciona a %s: parece reimplementación, no envoltorio."
                        % mayor.ruta.name))
        excepcion = (excepciones_duplicados or {}).get(
            ("ENVOLTORIO", tuple(sorted((ra, rb))))) if tipo == "ENVOLTORIO" else None
        if excepcion:
            tipo = "ORACULO_HISTORICO_DE_PRUEBA"
            excepcion["aplicada"] = True
            ev["oraculo_historico"] = excepcion["oraculo"]
            ev["registro_de_intencion"] = excepcion["registro"]
            ev["prueba_que_lo_vigila"] = excepcion["prueba"]
            ev.pop("delega_de_verdad", None)
            extra["justificacion"] = excepcion["motivo"]
            extra["regla_de_cambio"] = excepcion["regla_de_cambio"]
        out.append(hallazgo(tipo, [ra, rb], ev, round(sim, 3), extra))
    return out


def fase_scripts(ficheros, umbral_firma):
    """Dos herramientas que hacen lo mismo aunque no se parezcan."""
    scripts = [f for f in ficheros if f.ext == ".py" and f.rol == "herramienta"]
    for f in scripts:
        firma_de(f)
    out = []
    casi = []
    for i in range(len(scripts)):
        for j in range(i + 1, len(scripts)):
            a, b = scripts[i], scripts[j]
            sim = similitud_firma(a.firma, b.firma)
            clon = max(jaccard(a.shingles, b.shingles),
                       jaccard(a.shingles_par, b.shingles_par))
            if sim < umbral_firma:
                if sim >= umbral_firma * 0.6 and clon < UMBRAL_CLON:
                    casi.append((sim, a.rel, b.rel))
                continue
            if clon >= UMBRAL_CLON:
                continue   # ya lo cuenta fase_clones; no duplicar el aviso
            lanza_a_b = b.ruta.name in a.texto
            lanza_b_a = a.ruta.name in b.texto
            ev = {"firma": round(sim, 3), "clon_de_texto": round(clon, 3),
                  "lee_comun": sorted(a.firma["lee"] & b.firma["lee"])[:10],
                  "claves_comunes": sorted(a.firma["claves"] & b.firma["claves"])[:12],
                  "uno_llama_al_otro": lanza_a_b or lanza_b_a}
            extra = {"diferencias": diferencia_firma(a.firma, b.firma, a.rel, b.rel)}
            if lanza_a_b or lanza_b_a:
                extra["justificacion"] = (
                    "%s lanza a %s: es orquestación, no duplicación. Verificar "
                    "que la parte delegada no se reimplemente encima."
                    % (a.rel if lanza_a_b else b.rel, b.rel if lanza_a_b else a.rel))
            out.append(hallazgo("SOLAPAMIENTO_FUNCIONAL", [a.rel, b.rel], ev,
                                sim, extra))
    casi.sort(reverse=True)
    # El resultado NEGATIVO también es un resultado: si el par más parecido del
    # árbol está muy por debajo del umbral, «hay scripts duplicados» es falso y
    # conviene dejarlo escrito con los números, no solo callárselo.
    _CASI[:] = [{"firma": s, "par": [a, b]} for s, a, b in casi[:10]]
    return out


_CASI = []


# Sin anclar a principio de línea: en este árbol hay cabeceras de una sola línea
# (`namespace alpha { class Widget { ... }; }`) y el anclaje las dejaba fuera.
_RE_TIPO_DEF = re.compile(
    r"\b(class|struct|union|enum[ \t]+class|enum)[ \t]+([A-Za-z_]\w*)"
    r"[ \t]*(?:final[ \t]*)?(?::[^{;()]{0,80})?[ \t]*([{;])")
_RE_INCLUDE = re.compile(r'(?m)^[ \t]*#[ \t]*include[ \t]*[<"]([^">]+)[">]')
_RE_USING = re.compile(r"(?m)^[ \t]*using[ \t]+([A-Za-z_]\w*)[ \t]*=")


_RE_EVENTO = re.compile(
    r"\b(namespace|class|struct|union|enum)\b[ \t]*([A-Za-z_]\w*)?|[{};]")
_ARNESES = ("TestHarness", "Stub", "Stubs", "Fake", "Mock", "Scripted",
            "Helper", "Support", "Fixture")


def ambitos_de(f: Fichero):
    """Definiciones de tipo CON su ámbito: (nombre, linea, clase, fqn, anidado).

    Sin esto, `struct EditOp` privado de dos clases distintas salía como «el
    mismo tipo declarado dos veces», y no lo es: son dos tipos en dos ámbitos
    que no se ven. Lo que sí es un conflicto es el mismo nombre plenamente
    cualificado definido dos veces, y lo que es una trampa de lectura es el
    mismo nombre corto en dos módulos (Render::Entity y Editor::Entity)."""
    defs = [(m.start(), m.group(2), m.group(1).replace("\t", " "))
            for m in _RE_TIPO_DEF.finditer(f.texto) if m.group(3) == "{"]
    if not defs:
        return []
    defs.sort()
    pila, pendiente, out, di = [], None, [], 0

    def registrar(pos, nombre, clase):
        nss = [n for k, n in pila if k == "namespace" and n]
        anidado = any(k in ("class", "struct", "union", "enum") for k, _ in pila)
        out.append((nombre, f.texto.count("\n", 0, pos) + 1, clase,
                    "::".join(nss), anidado))

    for m in _RE_EVENTO.finditer(f.texto):
        while di < len(defs) and defs[di][0] < m.start():
            registrar(*defs[di]); di += 1
        tok = m.group(0)
        if tok == "{":
            pila.append(pendiente or ("block", None))
            pendiente = None
        elif tok == "}":
            if pila:
                pila.pop()
            pendiente = None
        elif tok == ";":
            pendiente = None      # `class X;` es una forward declaration
        else:
            pendiente = (m.group(1), m.group(2))
    while di < len(defs):
        registrar(*defs[di]); di += 1
    return out


def tipos_declarados(f: Fichero):
    """Compatibilidad con la vista simple: (nombre, linea, clase)."""
    return [(n, l, c) for n, l, c, _fqn, _an in ambitos_de(f)]


def fase_api(ficheros):
    cabeceras = [f for f in ficheros if f.ext in (".h", ".hpp")]
    por_fqn = defaultdict(list)
    por_nombre = defaultdict(list)
    for f in cabeceras:
        for nombre, linea, clase, fqn, anidado in ambitos_de(f):
            if anidado:
                continue     # tipo interno de una clase: no hay conflicto
            clave = (fqn + "::" if fqn else "") + nombre
            por_fqn[clave].append((f.rel, linea, clase))
            por_nombre[nombre].append((f.rel, linea, fqn))
    out = []
    for clave, sitios in sorted(por_fqn.items()):
        distintos = sorted({s[0] for s in sitios})
        if len(distintos) < 2:
            continue
        out.append(hallazgo("API_DUPLICADA", distintos, {
            "tipo": clave,
            "declarado_en": ["%s:%d (%s)" % s for s in sorted(sitios)],
        }))
    for nombre, sitios in sorted(por_nombre.items()):
        fqns = sorted({s[2] for s in sitios})
        distintos = sorted({s[0] for s in sitios})
        if len(distintos) < 2 or len(fqns) < 2:
            continue
        # solo si los dos son de producción: un tipo de test con el nombre de
        # uno real es ruido habitual y no confunde a nadie en el juego
        if any(rol_de(d) != "produccion" for d in distintos):
            continue
        out.append(hallazgo("NOMBRE_AMBIGUO", distintos, {
            "tipo": nombre,
            "ambitos": fqns,
            "declarado_en": ["%s:%d (%s::)" % s for s in sorted(sitios)],
        }))
    # cabeceras que solo reexportan otra
    for f in cabeceras:
        if f.lineas > 25 or ambitos_de(f):
            continue
        inc = _RE_INCLUDE.findall(f.texto)
        if not inc:
            continue
        cuerpo = re.sub(r"(?m)^[ \t]*#[ \t]*(include|pragma|ifndef|define|endif)[^\n]*\n?", "", f.texto)
        cuerpo = _RE_COM_LINEA.sub("", cuerpo)
        if len(cuerpo.strip()) > 40 and not _RE_USING.search(f.texto):
            continue
        out.append(hallazgo("CABECERA_ENVOLTORIO", [f.rel], {
            "lineas": f.lineas, "reexporta": inc,
            "contenido_extra": cuerpo.strip()[:80]}))
    return out


# ---------------------------------------------------------------------
# La lógica que vive dentro del test
# ---------------------------------------------------------------------

_RE_DEF_FUNC = re.compile(
    r"(?m)^[A-Za-z_][\w:<>,\*&\s]{0,60}?\b(\w+)[ \t]*\([^;{)]*\)[^;{]*\{")


def simbolos_definidos(ficheros, roles):
    """Nombre → ficheros donde está DEFINIDO (no solo mencionado)."""
    out = defaultdict(set)
    for f in ficheros:
        if f.rol not in roles or f.ext not in (".cpp", ".cc", ".c", ".h", ".hpp"):
            continue
        for m in _RE_DEF_FUNC.finditer(f.texto):
            out[m.group(1)].add(f.rel)
        for nombre, _l, _c, _fqn, _an in ambitos_de(f):
            out[nombre].add(f.rel)
    return out


def _parece_api(nombre: str) -> bool:
    """Un nombre de símbolo, no una palabra del texto: camelCase, PascalCase o
    snake_case. Los TODO_MAYUS son constantes de enum, no definiciones."""
    if nombre.isupper():
        return False
    if "_" in nombre:
        return True
    return any(c.isupper() for c in nombre[1:])


def fase_logica_en_test(ficheros, raiz: Path, estados):
    """El patrón que midió ALCANCE_REAL §6: G-13 y G-14 figuran cerrados y su
    lógica está escrita en el propio fichero de prueba, así que la prueba se
    prueba a sí misma y el jugador no ve nada. Se detecta cruzando lo que el
    registro del paquete PROMETE (los nombres que cita) con dónde está
    definido cada uno de esos nombres."""
    prod = simbolos_definidos(ficheros, {"produccion"})
    prueba = simbolos_definidos(ficheros, {"prueba"})
    arnes = {n for n, fs in prueba.items()
             if all(Path(x).stem.startswith(_ARNESES) for x in fs)}
    solo_en_prueba = {n: sorted(fs) for n, fs in prueba.items()
                      if n not in prod and n not in arnes and len(n) > 4
                      and len(fs) == 1}
    carpeta = raiz / "planificacion/seguimiento"
    if not carpeta.is_dir():
        return []
    out = []
    for ruta in sorted(carpeta.glob("*.json")):
        try:
            d = json.loads(ruta.read_text(encoding="utf-8"))
        except Exception:
            continue
        if not isinstance(d, dict):
            continue
        pid = str(d.get("paquete") or ruta.stem)
        if str(d.get("estado", "")).lower() not in ("closed", "cerrado", "done", "hecho", "fet"):
            continue
        # Solo los CRITERIOS de cierre: es lo que el paquete promete. La
        # sección `tests` describe casos con prosa ("carga el catalogo") y
        # mete palabras corrientes que algún test usa como nombre de función.
        texto = json.dumps(d.get("criterios_cierre"), ensure_ascii=False)
        citados = {n for n in re.findall(r"\b[A-Za-z_]\w{4,}\b", texto)
                   if _parece_api(n)}
        culpables = sorted(n for n in citados if n in solo_en_prueba)
        if not culpables:
            continue
        ficheros_implicados = sorted({x for n in culpables for x in solo_en_prueba[n]})
        out.append(hallazgo("LOGICA_EN_EL_TEST", ficheros_implicados, {
            "paquete": pid, "estado": d.get("estado"),
            "nombres_que_solo_define_el_test": culpables,
            "registro": "planificacion/seguimiento/%s" % ruta.name},
            None, {"justificacion":
                   "%s está cerrado y su registro promete %s, pero esos nombres "
                   "solo están definidos en %s: ningún ejecutable puede llamarlos."
                   % (pid, ", ".join("`%s`" % c for c in culpables[:5]),
                      ", ".join("`%s`" % x for x in ficheros_implicados[:3]))}))
    return out


def ids_de_catalogo(ruta: Path):
    try:
        d = json.loads(ruta.read_text(encoding="utf-8"))
    except Exception:
        return set()
    if not isinstance(d, dict):
        return set()
    ids = set()
    for clave, val in d.items():
        if isinstance(val, list) and val and all(isinstance(x, dict) for x in val):
            for x in val:
                if isinstance(x.get("id"), str):
                    ids.add("%s:%s" % (clave, x["id"]))
    return ids


def fase_catalogos(raiz: Path, ficheros):
    """Dos catálogos con los mismos ids = dos fuentes de verdad."""
    assets = [f for f in ficheros if f.rol == "contenido" and f.ext == ".json"]
    conjuntos = {}
    for f in assets:
        ids = ids_de_catalogo(f.ruta)
        if len(ids) >= 5:
            conjuntos[f.rel] = ids
    idx = defaultdict(list)
    for rel, ids in conjuntos.items():
        for i in ids:
            idx[i].append(rel)
    pares = set()
    for i, grupo in idx.items():
        if len(grupo) < 2 or len(grupo) > 12:
            continue
        for a in range(len(grupo)):
            for b in range(a + 1, len(grupo)):
                x, y = grupo[a], grupo[b]
                pares.add((x, y) if x < y else (y, x))
    out = []
    for ra, rb in sorted(pares):
        A, B = conjuntos[ra], conjuntos[rb]
        sim = len(A & B) / len(A | B)
        if sim < 0.80:
            continue
        out.append(hallazgo("CATALOGO_GEMELO", [ra, rb], {
            "ids_comunes": len(A & B), "ids_solo_a": len(A - B),
            "ids_solo_b": len(B - A), "ejemplos_comunes": sorted(A & B)[:6],
        }, round(sim, 3)))
    return out


# ---------------------------------------------------------------------
# Afectación: qué pasa si se toca esto
# ---------------------------------------------------------------------

_RE_MENCION = re.compile(
    r"[A-Za-z0-9_./+\- ]{2,120}\.(?:h|hpp|cpp|cc|c|py|json|cmake|sh|puml|md)")


def indice_menciones(ficheros):
    """Quién nombra a quién. Sirve para el radio de explosión: no es lo mismo
    retirar una cabecera que no incluye nadie que retirar una con 14 consumidores."""
    sufijos = defaultdict(set)
    for f in ficheros:
        partes = f.rel.split("/")
        for i in range(len(partes)):
            sufijos["/".join(partes[i:])].add(f.rel)
    mencionado_por = defaultdict(set)
    for f in ficheros:
        if f.ext not in EXT_TEXTO:
            continue
        vistos = set()
        for m in _RE_MENCION.findall(f.texto):
            clave = m.strip().strip('"\'')
            for k in (clave, clave.split("/")[-1]):
                for rel in sufijos.get(k, ()):
                    if rel != f.rel:
                        vistos.add(rel)
        for rel in vistos:
            mencionado_por[rel].add(f.rel)
    return {k: sorted(v) for k, v in mencionado_por.items()}


def paquetes_que_citan(raiz: Path):
    """seguimiento/*.json → qué ficheros dice cada paquete que tocó."""
    out = defaultdict(set)
    estado = {}
    carpeta = raiz / "planificacion/seguimiento"
    if not carpeta.is_dir():
        return out, estado
    for ruta in sorted(carpeta.glob("*.json")):
        try:
            d = json.loads(ruta.read_text(encoding="utf-8"))
        except Exception:
            continue
        if not isinstance(d, dict):
            continue
        pid = str(d.get("paquete") or ruta.stem)
        estado[pid] = str(d.get("estado", ""))
        textos = []
        # `archivos_compartidos_sin_tocar` se excluye a propósito: son los que
        # el paquete dice que NO ha tocado, y atribuírselos inventa un dueño.
        for clave in ("archivos_creados", "archivos_editados", "files_changed",
                      "archivos_modificados_1linea_include", "archivos", "ficheros"):
            v = d.get(clave)
            if isinstance(v, list):
                textos += [x for x in v if isinstance(x, str)]
        for t in textos:
            for m in re.findall(r"[A-Za-z0-9_./+\-]{3,120}\.(?:h|cpp|py|cmake|json)", t):
                out[pid].add(m)
    return out, estado


def cargar_excepciones_duplicados(raiz: Path):
    """Excepciones estrechas para oráculos históricos de prueba.

    Un registro no puede silenciar una familia de hallazgos: solo admite un
    par exacto producción/prueba, con su dueño, motivo y regla de cambio. Si
    el seguimiento está incompleto, mal formado o duplica un par, se ignora y
    el envoltorio continúa visible como deuda potencial.
    """
    out = {}
    claves_repetidas = set()
    carpeta = raiz / "planificacion/seguimiento"
    if not carpeta.is_dir():
        return out
    for ruta in sorted(carpeta.glob("*.json")):
        try:
            datos = json.loads(ruta.read_text(encoding="utf-8"))
        except Exception:
            continue
        if not isinstance(datos, dict):
            continue
        entradas = datos.get("excepciones_duplicados")
        if not isinstance(entradas, list):
            continue
        for entrada in entradas:
            if not isinstance(entrada, dict):
                continue
            archivos = entrada.get("archivos")
            if (entrada.get("tipo") != "ENVOLTORIO"
                    or entrada.get("clasificacion") != "ORACULO_HISTORICO_DE_PRUEBA"
                    or not isinstance(archivos, list) or len(archivos) != 2):
                continue
            if (not all(isinstance(a, str) and a and not a.startswith("/")
                        and ".." not in Path(a).parts for a in archivos)
                    or len(set(archivos)) != 2):
                continue
            pruebas = [a for a in archivos if a.startswith("tests/")]
            produccion = [a for a in archivos if not a.startswith("tests/")]
            if len(pruebas) != 1 or len(produccion) != 1 or rol_de(produccion[0]) != "produccion":
                continue
            requeridos = ("motivo", "oraculo", "prueba", "regla_de_cambio")
            if not all(isinstance(entrada.get(campo), str) and entrada[campo].strip()
                       for campo in requeridos):
                continue
            clave = ("ENVOLTORIO", tuple(sorted(archivos)))
            if clave in claves_repetidas:
                continue
            if clave in out:
                del out[clave]
                claves_repetidas.add(clave)
                continue
            out[clave] = {
                "motivo": entrada["motivo"].strip(),
                "oraculo": entrada["oraculo"].strip(),
                "prueba": entrada["prueba"].strip(),
                "regla_de_cambio": entrada["regla_de_cambio"].strip(),
                "registro": str(ruta.relative_to(raiz)),
            }
    return out


def afectacion_de(rel, mencionados, paquetes, alcance):
    consumidores = [c for c in mencionados.get(rel, [])
                    if rol_de(c) == "produccion"]
    pruebas = [c for c in mencionados.get(rel, []) if rol_de(c) == "prueba"]
    planes = [c for c in mencionados.get(rel, []) if rol_de(c) in ("plan", "herramienta")]
    pids = sorted(p for p, fs in paquetes.items()
                  if any(rel.endswith(x) or x.endswith(rel) for x in fs))
    llega = []
    if alcance:
        for exe in ("juego", "motor_editor", "level_editor"):
            d = alcance.get("cabeceras", {}).get(rel)
            if d and exe in d:
                llega.append(exe)
    return {
        "consumidores_de_produccion": consumidores[:20],
        "n_consumidores": len(consumidores),
        "pruebas_que_lo_usan": pruebas[:20],
        "n_pruebas": len(pruebas),
        "citado_en_docs_o_tools": planes[:10],
        "paquetes_del_gantt": pids,
        "llega_a": llega,
        "radio_de_explosion": len(consumidores) + len(pruebas),
    }


# ---------------------------------------------------------------------
# Muerto, cerrado sin alcance y desactualizado
# ---------------------------------------------------------------------

def _mtime(ruta):
    try:
        return int(Path(ruta).stat().st_mtime)
    except OSError:
        return 0


def cargar_alcance(raiz: Path):
    ruta = raiz / "planificacion/alcance_real.json"
    if not ruta.exists():
        return None
    try:
        return json.loads(ruta.read_text(encoding="utf-8"))
    except Exception:
        return None


def fase_muertos(alcance, paquetes, estados):
    """Consume medir_alcance.py en vez de volver a calcular el alcance:
    dos herramientas midiendo lo mismo es justo lo que este script persigue."""
    out = []
    if not alcance:
        out.append(hallazgo("MUERTO", [], {
            "motivo": "falta planificacion/alcance_real.json; ejecuta "
                      "tools/medir_alcance.py --json para que esta fase exista"}))
        return out
    cab = alcance.get("cabeceras", {})
    for h in sorted(cab):
        d = cab[h]
        llega = [e for e in ("juego", "motor_editor", "level_editor") if e in d]
        if llega:
            continue
        pids = sorted(p for p, fs in paquetes.items()
                      if any(h.endswith(x) or x.endswith(h) for x in fs))
        cerrados = [p for p in pids if estados.get(p, "").lower()
                    in ("closed", "cerrado", "done", "hecho", "fet")]
        ev = {"lineas": d.get("lineas"), "implementacion": d.get("implementacion", []),
              "n_pruebas_que_lo_usan": d.get("pruebas", 0),
              "paquetes": pids, "paquetes_que_lo_dan_por_cerrado": cerrados}
        extra = {}
        if cerrados:
            extra["justificacion"] = (
                "%s no lo alcanza ningún ejecutable, pero %s lo registra como "
                "cerrado. Es el patrón que midió ALCANCE_REAL_2026-09-17.md §6."
                % (h, ", ".join(cerrados)))
        out.append(hallazgo("MUERTO", [h] + list(d.get("implementacion", [])),
                            ev, None, extra))
    # paquetes cerrados cuyo producto no llega a nadie
    for pid, fila in sorted(alcance.get("paquetes", {}).items()):
        if estados.get(pid, "").lower() not in ("closed", "cerrado", "done", "hecho", "fet"):
            continue
        nadie = fila.get("nadie") or []
        if not nadie:
            continue
        out.append(hallazgo("REGISTRO_CERRADO_SIN_ALCANCE", nadie, {
            "paquete": pid, "estado": estados.get(pid),
            "ficheros_que_no_llegan": nadie,
            "ficheros_totales_del_paquete": fila.get("ficheros")},
            None, {"justificacion":
                   "%s está cerrado y %d de sus %d ficheros de producto no los "
                   "usa ningún ejecutable." % (pid, len(nadie), fila.get("ficheros", 0))}))
    return out


def fechas_git(raiz: Path):
    """Una sola pasada de git log para todo el árbol. Si git no está o el
    fichero no se ha versionado, se cae a mtime sin abortar."""
    def mtime(r):
        try:
            return int(r.stat().st_mtime)
        except OSError:
            return 0
    fechas = {}
    try:
        repo = subprocess.run(["git", "rev-parse", "--show-toplevel"], cwd=str(raiz),
                              capture_output=True, text=True).stdout.strip()
    except Exception:
        repo = ""
    if repo:
        try:
            pref = str(raiz.resolve().relative_to(Path(repo).resolve()))
        except ValueError:
            pref = ""
        try:
            out = subprocess.run(
                ["git", "--no-optional-locks", "log", "--pretty=format:@%ct",
                 "--name-only", "-n", "8000"],
                cwd=str(raiz), capture_output=True, text=True, timeout=180).stdout
            ts = 0
            for line in out.split("\n"):
                if line.startswith("@"):
                    ts = int(line[1:])
                elif line.strip():
                    rel = line.strip()
                    if pref and rel.startswith(pref + "/"):
                        rel = rel[len(pref) + 1:]
                    fechas.setdefault(rel, ts)
        except Exception:
            pass
    return fechas, mtime


_RE_CITA_CODIGO = re.compile(
    r"[A-Za-z0-9_./+\-]{3,120}\.(?:h|hpp|cpp|cc|py|cmake)")
_HISTORICO = re.compile(r"(?i)congelad|hist[oó]ric|snapshot|no se actualiza")


def fase_docs(ficheros, fechas, mtime, raiz):
    """Un doc que cita código que cambió después miente. O se actualiza o se
    fecha y se declara histórico, como la Parte 0 de la bitácora."""
    existentes = {f.rel for f in ficheros}
    por_rel = {f.rel: f for f in ficheros}
    out = []
    for f in ficheros:
        if f.ext not in (".md", ".puml"):
            continue
        cabecera = "\n".join(f.texto.split("\n")[:25])
        if _HISTORICO.search(cabecera):
            continue
        citados = set()
        for m in _RE_CITA_CODIGO.findall(f.texto):
            m = m.strip()
            for c in existentes:
                if c == m or c.endswith("/" + m) or c.endswith(m):
                    citados.add(c)
                    break
        citados = sorted(citados)[:400]
        if len(citados) < 3:
            continue
        dts = fechas.get(f.rel) or mtime(f.ruta)
        cambiados = []
        for c in citados:
            cts = fechas.get(c) or mtime(por_rel[c].ruta)
            if cts > dts:
                cambiados.append(c)
        if len(cambiados) < 3:
            continue
        out.append(hallazgo("DOC_DESACTUALIZADO", [f.rel], {
            "codigo_citado": len(citados),
            "cambiado_despues": len(cambiados),
            "ejemplos": cambiados[:10]},
            None, {"justificacion":
                   "%s cita %d ficheros de código y %d cambiaron después de su "
                   "última revisión." % (f.rel, len(citados), len(cambiados))}))
    return out


_SUFIJOS_TEST = ("Tests", "Test", "tests", "test")


def _es_su_propio_test(a, b) -> bool:
    """`src/Editor/EditorState.cpp` y `tests/Editor/EditorStateTests.cpp` dan
    similitud de nombre 1.00 y no le dicen nada a nadie: es el par obvio. Lo
    interesante es `examples/Check.h` contra `include/Core/Check.h`."""
    if a.rol == b.rol:
        return False
    prod, test = (a, b) if a.rol == "prueba" else (b, a)
    base = prod.stem
    return test.stem in tuple(base + s for s in _SUFIJOS_TEST)


def fase_disimilares(ficheros, reportados, umbral_clon):
    """Se parecen de nombre, no de función. La gracia está en decir EN QUÉ
    se diferencian, para que nadie los fusione por error."""
    por_rel = {f.rel: f for f in ficheros}
    pares = []
    solo = [f for f in ficheros if f.ext in EXT_CODIGO or f.rol == "herramienta"]
    for i in range(len(solo)):
        for j in range(i + 1, len(solo)):
            a, b = solo[i], solo[j]
            if a.ext != b.ext:
                continue
            key = tuple(sorted((a.rel, b.rel)))
            if key in reportados:
                continue
            if _es_su_propio_test(a, b):
                continue      # X.cpp contra XTests.cpp no confunde a nadie
            sim = parecidos_por_nombre(a.stem, b.stem)
            if sim < UMBRAL_NOMBRE:
                continue
            if max(jaccard(a.shingles, b.shingles),
                   jaccard(a.shingles_par, b.shingles_par)) >= umbral_clon:
                continue      # esos ya son clones: otra sección
            pares.append((sim, a.rel, b.rel))
    por_rel2 = {f.rel: f for f in ficheros}
    def _mismo_rol(par):
        return 0 if por_rel2[par[1]].rol == por_rel2[par[2]].rol else 1
    pares.sort(key=lambda x: (_mismo_rol(x), -x[0], x[1], x[2]))
    out = []
    for sim, ra, rb in pares[:60]:
        a, b = por_rel[ra], por_rel[rb]
        if a.ext == ".py":
            firma_de(a), firma_de(b)
            dif = {"lee": {"a": sorted(a.firma["lee"])[:8], "b": sorted(b.firma["lee"])[:8]},
                   "comprueba": {"a": sorted(a.firma["claves"])[:8], "b": sorted(b.firma["claves"])[:8]},
                   "lanza": {"a": sorted(a.firma["lanza"])[:6], "b": sorted(b.firma["lanza"])[:6]}}
        else:
            dif = {"tipos_que_define": {"a": [t[0] for t in tipos_declarados(a)][:10],
                                        "b": [t[0] for t in tipos_declarados(b)][:10]},
                   "incluye": {"a": _RE_INCLUDE.findall(a.texto)[:8],
                               "b": _RE_INCLUDE.findall(b.texto)[:8]}}
        out.append(hallazgo("DISIMILAR", [ra, rb], {
            "similitud_de_nombre": round(sim, 3),
            "similitud_de_codigo": round(max(jaccard(a.shingles, b.shingles),
                                             jaccard(a.shingles_par, b.shingles_par)), 3),
            "lineas": {ra: a.lineas, rb: b.lineas},
            "roles": {ra: a.rol, rb: b.rol},
            "en_que_se_diferencian": dif}, round(sim, 3)))
    return out


# ---------------------------------------------------------------------
# Orquestación
# ---------------------------------------------------------------------

ORDEN = ["DUPLICADO", "CLON_PARCIAL", "CLON_ESTRUCTURAL", "ENVOLTORIO",
         "ORACULO_HISTORICO_DE_PRUEBA",
         "API_DUPLICADA", "NOMBRE_AMBIGUO", "LOGICA_EN_EL_TEST",
         "SOLAPAMIENTO_FUNCIONAL", "CATALOGO_GEMELO",
         "REGISTRO_CERRADO_SIN_ALCANCE", "MUERTO", "CABECERA_ENVOLTORIO",
         "DOC_DESACTUALIZADO", "DISIMILAR"]


def analizar(raiz: Path, umbral_clon=UMBRAL_CLON, umbral_firma=UMBRAL_FIRMA,
             con_git=True):
    ficheros = inventario(raiz)
    preparar(ficheros)
    alcance = cargar_alcance(raiz)
    paquetes, estados = paquetes_que_citan(raiz)
    excepciones_duplicados = cargar_excepciones_duplicados(raiz)

    hallazgos = []
    hallazgos += fase_exactos(ficheros)
    hallazgos += fase_clones(ficheros, umbral_clon, excepciones_duplicados)
    excepciones_sin_aplicar = [
        {
            "archivos": list(clave[1]),
            "oraculo": excepcion["oraculo"],
            "prueba": excepcion["prueba"],
            "registro": excepcion["registro"],
        }
        for clave, excepcion in sorted(excepciones_duplicados.items())
        if not excepcion.get("aplicada")
    ]
    hallazgos += fase_scripts(ficheros, umbral_firma)
    hallazgos += fase_api(ficheros)
    hallazgos += fase_logica_en_test(ficheros, raiz, estados)
    hallazgos += fase_catalogos(raiz, ficheros)
    hallazgos += fase_muertos(alcance, paquetes, estados)
    fechas, mtime = fechas_git(raiz) if con_git else ({}, _mtime)
    hallazgos += fase_docs(ficheros, fechas, mtime, raiz)

    # los disímilares excluyen los pares ya juzgados: un par no puede ser a la
    # vez clon y "distintos de función"
    reportados = set()
    for h in hallazgos:
        if len(h["archivos"]) == 2:
            reportados.add(tuple(h["archivos"]))
    hallazgos += fase_disimilares(ficheros, reportados, umbral_clon)

    # afectación: una sola consulta por fichero implicado
    mencionados = indice_menciones(ficheros)
    implicados = {a for h in hallazgos for a in h["archivos"]}
    afec = {a: afectacion_de(a, mencionados, paquetes, alcance)
            for a in sorted(implicados)}
    for h in hallazgos:
        h["afectacion"] = {a: afec[a] for a in h["archivos"] if a in afec}

    orden = {t: i for i, t in enumerate(ORDEN)}
    sev = {"alta": 0, "media": 1, "baja": 2, "info": 3}
    hallazgos.sort(key=lambda h: (orden.get(h["tipo"], 99),
                                  sev.get(h["severidad"], 9),
                                  h["archivos"]))
    por_tipo = defaultdict(int)
    for h in hallazgos:
        por_tipo[h["tipo"]] += 1
    return {
        "raiz": str(raiz),
        "ficheros_analizados": len(ficheros),
        "por_rol": _contador(f.rol for f in ficheros),
        "umbrales": {"clon": umbral_clon, "firma": umbral_firma,
                     "nombre": UMBRAL_NOMBRE, "duplicado": UMBRAL_DUPLICADO},
        "alcance_disponible": bool(alcance),
        "resumen": dict(por_tipo),
        "excepciones_duplicados_sin_aplicar": excepciones_sin_aplicar,
        "casi_solapamientos_de_scripts": list(_CASI),
        "hallazgos": hallazgos,
        "reglas": VEREDICTOS,
    }


def _contador(it):
    c = defaultdict(int)
    for x in it:
        c[x] += 1
    return dict(sorted(c.items()))


def enfocar(informe, ruta):
    """«¿Hay otro script que hace lo mismo que ESTE?» — informe de un fichero."""
    rel = ruta.replace("\\", "/")
    propios = [h for h in informe["hallazgos"] if rel in h["archivos"]]
    accionables = ("DUPLICADO", "CLON_PARCIAL", "ENVOLTORIO", "API_DUPLICADA",
                   "SOLAPAMIENTO_FUNCIONAL", "CATALOGO_GEMELO")
    obsoletos = ("MUERTO", "REGISTRO_CERRADO_SIN_ALCANCE", "DOC_DESACTUALIZADO",
                 "CABECERA_ENVOLTORIO")
    tipos = {h["tipo"] for h in propios}
    return {
        "fichero": rel,
        "en_el_informe": len(propios),
        "hallazgos": propios,
        "veredicto": ("DUPLICADO_O_SOLAPADO" if tipos & set(accionables)
                      else "MUERTO_O_DESACTUALIZADO" if tipos & set(obsoletos)
                      else "ORACULO_HISTORICO_INTENCIONAL"
                      if "ORACULO_HISTORICO_DE_PRUEBA" in tipos
                      else "SIN_PAREJA_CONOCIDA"),
    }


# ---------------------------------------------------------------------
# Informe legible
# ---------------------------------------------------------------------

TITULOS = {
    "DUPLICADO": "1. Duplicados exactos (mismo contenido en dos sitios)",
    "CLON_PARCIAL": "2. Clones parciales (trozos copiados)",
    "CLON_ESTRUCTURAL": "2b. Misma forma con otros nombres (plantilla repetida)",
    "NOMBRE_AMBIGUO": "4b. El mismo nombre de tipo en dos módulos distintos",
    "LOGICA_EN_EL_TEST": "6b. Paquetes cerrados cuya lógica vive dentro del test",
    "ENVOLTORIO": "3. Envoltorios (uno cabe dentro del otro)",
    "ORACULO_HISTORICO_DE_PRUEBA": "3b. Oráculos históricos de prueba (intencionales)",
    "API_DUPLICADA": "4. Tipos declarados en dos cabeceras",
    "SOLAPAMIENTO_FUNCIONAL": "5. Scripts que hacen lo mismo sin parecerse",
    "CATALOGO_GEMELO": "6. Catálogos gemelos (mismos ids)",
    "REGISTRO_CERRADO_SIN_ALCANCE": "7. Registros cerrados que no llegan a nadie",
    "MUERTO": "8. Código muerto (ningún ejecutable lo alcanza)",
    "CABECERA_ENVOLTORIO": "9. Cabeceras que solo reexportan",
    "DOC_DESACTUALIZADO": "10. Documentación desactualizada",
    "DISIMILAR": "11. Disímilares: se parecen de nombre, NO se fusionan",
}


def a_markdown(informe, max_por_tipo=25):
    L = []
    L.append("# Informe de duplicados, código muerto y disímilares")
    L.append("")
    L.append("Generado por `tools/analizar_duplicados.py` sobre `%s`. "
             "%d ficheros analizados. Alcance real disponible: %s."
             % (informe["raiz"], informe["ficheros_analizados"],
                "sí" if informe["alcance_disponible"] else "no"))
    L.append("")
    L.append("Umbrales: clon %.2f · duplicado %.2f · firma %.2f · nombre %.2f. "
             "Ningún hallazgo es una sentencia: cada uno lleva su evidencia y "
             "la regla que lo clasifica (`VEREDICTOS` en el script)."
             % (informe["umbrales"]["clon"], informe["umbrales"]["duplicado"],
                informe["umbrales"]["firma"], informe["umbrales"]["nombre"]))
    L.append("")
    sin_aplicar = informe.get("excepciones_duplicados_sin_aplicar") or []
    if sin_aplicar:
        L.append("> **Advertencia:** hay excepciones de oráculos históricos que ya no "
                 "corresponden a un `ENVOLTORIO` detectado. Revísalas antes de que "
                 "silencien una intención obsoleta.")
        for excepcion in sin_aplicar:
            L.append("> - `%s` → %s" % (excepcion["registro"],
                                          " · ".join(excepcion["archivos"])))
        L.append("")
    L.append("| Tipo | Hallazgos | Severidad |")
    L.append("| --- | ---: | --- |")
    for t in ORDEN:
        if informe["resumen"].get(t):
            L.append("| `%s` | %d | %s |" % (t, informe["resumen"][t], SEVERIDAD.get(t, "")))
    casi = informe.get("casi_solapamientos_de_scripts") or []
    if casi:
        L.append("## Los 10 pares de scripts más parecidos (por debajo del umbral)")
        L.append("")
        L.append("Ninguno llega a `SOLAPAMIENTO_FUNCIONAL` (%.2f). Se listan para "
                 "que el resultado negativo también tenga números: si el más "
                 "parecido del árbol es %.3f, «hay herramientas duplicadas» es "
                 "falso hoy — y deja de serlo en cuanto uno de estos suba."
                 % (informe["umbrales"]["firma"], casi[0]["firma"]))
        L.append("")
        L.append("| Firma | Script A | Script B |")
        L.append("| ---: | --- | --- |")
        for c in casi:
            L.append("| %.3f | `%s` | `%s` |" % (c["firma"], c["par"][0], c["par"][1]))
        L.append("")
    por_tipo = defaultdict(list)
    for h in informe["hallazgos"]:
        por_tipo[h["tipo"]].append(h)
    for t in ORDEN:
        grupo = por_tipo.get(t) or []
        if not grupo:
            continue
        L.append("## %s" % TITULOS[t])
        L.append("")
        L.append("*Regla:* %s" % VEREDICTOS[t])
        L.append("")
        for h in grupo[:max_por_tipo]:
            L.append("### %s — %s" % (h["id"], " · ".join(h["archivos"][:4])))
            L.append("")
            if len(h["archivos"]) > 4:
                L.append("*(y %d ficheros más)*" % (len(h["archivos"]) - 4))
            for k, v in sorted(h["evidencia"].items()):
                L.append("- **%s:** %s" % (k, _fmt(v)))
            if h.get("similitud") is not None:
                L.append("- **similitud:** %s" % h["similitud"])
            if h.get("justificacion"):
                L.append("- **por qué:** %s" % h["justificacion"])
            if h.get("regla_de_cambio"):
                L.append("- **regla de cambio:** %s" % h["regla_de_cambio"])
            for f, a in sorted(h.get("afectacion", {}).items()):
                L.append("- **afectación de `%s`:** %d consumidores de producción, "
                         "%d pruebas, llega a %s, paquetes %s, radio %d"
                         % (f, a["n_consumidores"], a["n_pruebas"],
                            ", ".join(a["llega_a"]) or "nadie",
                            ", ".join(a["paquetes_del_gantt"]) or "—",
                            a["radio_de_explosion"]))
                if a["consumidores_de_produccion"]:
                    L.append("  - consumidores: %s" % ", ".join(
                        "`%s`" % c for c in a["consumidores_de_produccion"][:8]))
            if h.get("diferencias"):
                L.append("- **diferencias que lo justifican:**")
                for clave, d in h["diferencias"].items():
                    na, nb = d["solo_en"], d["solo_en_b"]
                    if d.get(na) or d.get(nb):
                        L.append("  - *%s* → solo en `%s`: %s | solo en `%s`: %s"
                                 % (clave, na, _fmt(d.get(na, [])),
                                    nb, _fmt(d.get(nb, []))))
            if h.get("en_que_se_diferencian"):
                L.append("- **en qué se diferencian:**")
                for clave, d in h["en_que_se_diferencian"].items():
                    L.append("  - *%s* → a: %s | b: %s" % (clave, _fmt(d["a"]), _fmt(d["b"])))
            L.append("")
        if len(grupo) > max_por_tipo:
            L.append("*(%d hallazgos más de este tipo en el JSON)*" % (len(grupo) - max_por_tipo))
            L.append("")
    return "\n".join(L) + "\n"


def _fmt(v):
    if isinstance(v, (list, tuple)):
        if not v:
            return "—"
        return ", ".join("`%s`" % x for x in list(v)[:10]) + (
            " …(+%d)" % (len(v) - 10) if len(v) > 10 else "")
    if isinstance(v, dict):
        return json.dumps(v, ensure_ascii=False, sort_keys=True)
    return str(v)


def resumen_stdout(informe):
    print("analizar_duplicados: %d ficheros, %d hallazgos" %
          (informe["ficheros_analizados"], len(informe["hallazgos"])))
    for t in ORDEN:
        n = informe["resumen"].get(t)
        if n:
            print("  %-32s %3d  (%s)" % (t, n, SEVERIDAD.get(t, "")))
    altos = [h for h in informe["hallazgos"] if h["severidad"] == "alta"]
    if altos:
        print("  severidad alta: %d — empieza por aquí" % len(altos))
        for h in altos[:8]:
            print("    %s %s" % (h["tipo"], " · ".join(h["archivos"][:3])))
    sin_aplicar = informe.get("excepciones_duplicados_sin_aplicar") or []
    if sin_aplicar:
        print("  advertencia: %d excepción(es) de oráculo sin aplicar" % len(sin_aplicar))
        for excepcion in sin_aplicar[:5]:
            print("    %s %s" % (excepcion["registro"],
                                  " · ".join(excepcion["archivos"])))
    return 0


# ---------------------------------------------------------------------
# Comprobación: que el analizador vea lo que tiene delante
# ---------------------------------------------------------------------

def comprobar(raiz_real: Path):
    """Siembra un árbol pequeño con casos plantados y exige que el analizador
    los vea —y que NO vea los que no son. Es la red contra el fallo clásico
    de esta casa: un validador con la lista escrita a mano que deja de validar
    (Parte 3 de la bitácora)."""
    import importlib.util
    ruta_test = Path(__file__).resolve().parent / "test_analizar_duplicados.py"
    if not ruta_test.exists():
        print("analizar_duplicados: falta %s para --comprobar" % ruta_test.name)
        return 2
    spec = importlib.util.spec_from_file_location("test_analizar_duplicados", ruta_test)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    ok, fallos = mod.ejecutar_casos(analizar)
    if not ok:
        print("analizar_duplicados --comprobar: FALLO")
        for f in fallos:
            print("  - " + f)
        return 2
    print("analizar_duplicados --comprobar: OK — %d casos plantados vistos, "
          "ningún falso positivo" % ok)
    # anclas sobre el árbol real: si dejan de verse, el informe no vale
    if (raiz_real / "planificacion/alcance_real.json").exists():
        inf = analizar(raiz_real, con_git=False)
        if inf.get("excepciones_duplicados_sin_aplicar"):
            print("analizar_duplicados --comprobar: hay excepciones de oráculo sin "
                  "aplicar en el árbol real; revisar C-19/seguimiento antes de cerrar")
            return 2
        muertos = [h for h in inf["hallazgos"] if h["tipo"] == "MUERTO"]
        if not muertos:
            print("analizar_duplicados --comprobar: el árbol real no da ningún "
                  "MUERTO y alcance_real.json sí los tiene → el analizador se ha roto")
            return 2
        print("  ancla árbol real: %d MUERTO, %d DISIMILAR, %d SOLAPAMIENTO_FUNCIONAL"
              % (inf["resumen"].get("MUERTO", 0), inf["resumen"].get("DISIMILAR", 0),
                 inf["resumen"].get("SOLAPAMIENTO_FUNCIONAL", 0)))
    return 0


CON_VALOR = ("--json", "--md", "--solo", "--fichero", "--umbral-clon", "--umbral-firma")


def opciones(argv):
    """Parser mínimo. Antes se filtraba con `startswith("--")` y el VALOR de
    cada opción se colaba como ruta raíz: `--json salida.json` analizaba
    `salida.json`. Con opciones que toman valor, hay que saltárselo."""
    opts, libres = {}, []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a in CON_VALOR:
            if i + 1 >= len(argv):
                raise SystemExit("%s necesita un valor" % a)
            opts[a] = argv[i + 1]
            i += 2
        elif a.startswith("--"):
            opts[a] = True
            i += 1
        else:
            libres.append(a)
            i += 1
    return opts, libres


def main(argv):
    opts, libres = opciones(argv)
    raiz = Path(libres[0]).resolve() if libres else Path(__file__).resolve().parent.parent
    if "--comprobar" in opts:
        return comprobar(raiz)
    umbral_clon = float(opts.get("--umbral-clon", UMBRAL_CLON))
    umbral_firma = float(opts.get("--umbral-firma", UMBRAL_FIRMA))
    sin_git = "--sin-git" in opts
    informe = analizar(raiz, umbral_clon, umbral_firma, con_git=not sin_git)
    if "--solo" in opts:
        solo = opts["--solo"].split(",")
        informe["hallazgos"] = [h for h in informe["hallazgos"] if h["tipo"] in solo]
        informe["resumen"] = dict(_contador(h["tipo"] for h in informe["hallazgos"]))
    salida = informe
    if "--fichero" in opts:
        salida = enfocar(informe, opts["--fichero"])
    if "--json" in opts:
        destino = Path(opts["--json"])
        destino.parent.mkdir(parents=True, exist_ok=True)
        destino.write_text(json.dumps(salida, ensure_ascii=False, indent=1, sort_keys=True),
                           encoding="utf-8")
        print("escrito %s" % destino)
    if "--md" in opts:
        destino = Path(opts["--md"])
        destino.parent.mkdir(parents=True, exist_ok=True)
        if "hallazgos" in salida and "raiz" in salida:
            destino.write_text(a_markdown(salida), encoding="utf-8")
        else:
            destino.write_text("```json\n" + json.dumps(salida, ensure_ascii=False, indent=1)
                               + "\n```\n", encoding="utf-8")
        print("escrito %s" % destino)
    if "--fichero" in opts:
        print("fichero %s → %s (%d hallazgos)"
              % (salida["fichero"], salida["veredicto"], salida["en_el_informe"]))
        for h in salida["hallazgos"]:
            print("  %s %s → %s" % (h["tipo"], " · ".join(h["archivos"][:3]),
                                    h.get("justificacion", h["regla"])[:110]))
        return 0
    return resumen_stdout(informe)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
