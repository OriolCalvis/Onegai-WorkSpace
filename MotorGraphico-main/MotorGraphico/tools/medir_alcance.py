#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Medir el alcance REAL: qué código llega al jugador y al autor.

POR QUÉ EXISTE. En este repositorio «existe código» y «alguien puede
usarlo» se confunden a menudo: un paquete se da por hecho con su prueba
verde, pero ningún ejecutable llama a lo que la prueba prueba. La
auditoría del 03/09 (planificacion/AUDITORIA_2026-09-03_ORFES_I_COBERTURA.md)
lo midió por INCLUSIÓN, y ya avisaba de que no basta: IsometricRenderer
exponía setFog y Application no lo llamaba. Esto mide por USO.

QUÉ MIDE, para cada ejecutable que usa una persona (juego = jugador,
motor_editor = autor; level_editor = editor antiguo):

1. CABECERAS USADAS. Cierre desde las fuentes del ejecutable: una
   cabecera cuenta si (a) está en el cierre de #include de un fichero ya
   alcanzado y (b) ese fichero NOMBRA algo que la cabecera declara (una
   clase, un enum, un alias o una función libre). Entonces su cabecera y
   sus .cpp (los que definen `Clase::` o comparten nombre) pasan a estar
   alcanzados, pero solo si forman parte de lo que el ejecutable enlaza
   (CMakeLists). Incluir no es usar.
2. MÉTODOS QUE NADIE DE FUERA LLAMA. Para cada clase alcanzada, los
   métodos públicos cuyo nombre no aparece en ningún fichero alcanzado
   fuera de la propia clase. Dos avisos: es OPTIMISTA hacia fuera (un
   nombre corriente, «update», puede aparecer por otra clase y contar
   como llamado) y NO mira dentro (la propia clase puede usarlo: así
   GameSession::collectRent). Un método de la lista que tampoco se usa
   dentro no se ejecuta nunca desde ese ejecutable.
3. SOLO PRUEBAS. Cabeceras que no alcanza ningún ejecutable y sí alguna
   fuente de tests/ o examples/ distinta de las de los ejecutables.

Es un analizador heurístico de texto, hecho a la medida de este árbol
(misma familia que comprobar_declaraciones_rpg.py). No compila nada.

Uso:
  python3 tools/medir_alcance.py [raiz_del_motor] [--json salida.json]
"""
import json
import re
import sys
from collections import defaultdict, deque
from pathlib import Path

EJECUTABLES = {
    "juego": "jugador",
    "motor_editor": "autor",
    "level_editor": "autor (editor antiguo)",
}

PALABRAS_CPP = set("""
alignas alignof and auto bool break case catch char class const constexpr
continue decltype default delete do double else enum explicit export extern
false float for friend goto if inline int long mutable namespace new noexcept
not nullptr operator or private protected public return short signed sizeof
static struct switch template this throw true try typedef typename union
unsigned using virtual void volatile while override final std size_t string
vector include define ifdef ifndef endif pragma once
""".split())

_ALCANZADOS_JUEGO = set()
RE_ID = re.compile(r"[A-Za-z_]\w*")
RE_INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)


def limpiar(texto):
    """Quita comentarios y el contenido de los literales (conserva la
    forma de las líneas). Un nombre en un comentario no es un uso."""
    out, i, n = [], 0, len(texto)
    while i < n:
        c = texto[i]
        if texto.startswith("//", i):
            j = texto.find("\n", i)
            i = n if j < 0 else j
        elif texto.startswith("/*", i):
            j = texto.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("\n" * texto.count("\n", i, j))
            i = j
        elif c == '"' or c == "'":
            # R"(...)" crudo
            if c == '"' and i > 0 and texto[i - 1] == "R":
                m = re.match(r'"([^(]*)\(', texto[i:])
                if m:
                    fin = texto.find(")" + m.group(1) + '"', i)
                    fin = n if fin < 0 else fin + len(m.group(1)) + 2
                    out.append('""' + "\n" * texto.count("\n", i, fin))
                    i = fin
                    continue
            j = i + 1
            while j < n and texto[j] != c and texto[j] != "\n":
                j += 2 if texto[j] == "\\" else 1
            out.append(c + c)
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


class Arbol:
    def __init__(self, raiz):
        self.raiz = Path(raiz).resolve()
        self.texto = {}      # ruta relativa -> texto limpio
        self.crudo = {}
        self.includes = {}   # rel -> [rel]
        self.tokens = {}
        self.cargar()

    def rel(self, p):
        return str(Path(p).resolve().relative_to(self.raiz))

    def cargar(self):
        for carpeta in ("include", "src", "editor", "examples", "tests"):
            base = self.raiz / carpeta
            if not base.exists():
                continue
            for p in base.rglob("*"):
                if p.suffix not in (".h", ".hpp", ".cpp") or not p.is_file():
                    continue
                if " 2" in p.name:
                    continue
                r = self.rel(p)
                crudo = p.read_text(encoding="utf-8", errors="replace")
                self.crudo[r] = crudo
                self.texto[r] = limpiar(crudo)
        for r, crudo in self.crudo.items():
            self.includes[r] = [x for x in (self.resolver(r, inc) for inc in RE_INCLUDE.findall(crudo)) if x]
            self.tokens[r] = set(RE_ID.findall(self.texto[r])) - PALABRAS_CPP

    def resolver(self, desde, inc):
        candidatos = [Path(desde).parent / inc, Path("include") / inc, Path("editor/include") / inc,
                      Path("tests") / inc, Path("examples") / inc, Path(inc)]
        for c in candidatos:
            k = str(Path(c))
            k = str((self.raiz / k).resolve().relative_to(self.raiz)) if (self.raiz / k).exists() else None
            if k and k in self.texto:
                return k
        return None

    def cierre(self, r, _memo={}):
        if r in _memo:
            return _memo[r]
        vistos, pila = set(), [r]
        while pila:
            x = pila.pop()
            for y in self.includes.get(x, []):
                if y not in vistos:
                    vistos.add(y)
                    pila.append(y)
        _memo[r] = vistos
        return vistos


def es_cabecera(r):
    return r.endswith((".h", ".hpp"))


def declarados(texto):
    """Nombres de nivel de espacio de nombres que declara una cabecera."""
    nombres = set()
    for m in re.finditer(r"^\s*(?:template\s*<[^>]*>\s*)?(?:class|struct|union)\s+(?:\w+\s+)*?(\w+)\s*(?:final\s*)?(?::[^;{]*)?\{", texto, re.M):
        nombres.add(m.group(1))
    for m in re.finditer(r"^\s*enum\s+(?:class\s+|struct\s+)?(\w+)", texto, re.M):
        nombres.add(m.group(1))
    for m in re.finditer(r"^\s*using\s+(\w+)\s*=", texto, re.M):
        nombres.add(m.group(1))
    for m in re.finditer(r"^\s*typedef\s+[^;]*\b(\w+)\s*;", texto, re.M):
        nombres.add(m.group(1))
    # Funciones libres: a columna 0, fuera de cuerpos de clase.
    profundidad_clase = 0
    for linea in texto.splitlines():
        if re.match(r"^(?:class|struct)\s+\w+[^;]*$", linea):
            profundidad_clase = 1
        if linea.startswith("};"):
            profundidad_clase = 0
            continue
        if profundidad_clase:
            continue
        m = re.match(r"^(?:inline\s+|static\s+|constexpr\s+|template\s*<[^>]*>\s*)*"
                     r"(?:const\s+)?[\w:<>,]+[\s\*&]+(\w+)\s*\(", linea)
        if m and m.group(1) not in PALABRAS_CPP and m.group(1) != "operator":
            nombres.add(m.group(1))
    # Macros de función (MOTOR_CHECK...) y constantes a nivel de fichero
    for m in re.finditer(r"^\s*#\s*define\s+(\w+)", texto, re.M):
        nombres.add(m.group(1))
    for m in re.finditer(r"^\s*(?:inline\s+)?(?:static\s+)?constexpr\s+[\w:<>]+\s+(\w+)\s*=", texto, re.M):
        nombres.add(m.group(1))
    # Template specializations: TemplateClass<T>::metodo(
    for m in re.finditer(r"\b(\w+)\s*<[^>]+>\s*::\s*(\w+)\s*\(", texto):
        nombres.add(m.group(2))
    return {n for n in nombres if n not in PALABRAS_CPP and len(n) > 1}


def es_paraguas(texto):
    """True si la cabecera solo incluye otras cabeceras y no declara símbolos propios."""
    lineas = [l.strip() for l in texto.splitlines() if l.strip()]
    if not lineas:
        return False
    # Debe tener al menos un include
    tiene_include = any(l.startswith("#include") for l in lineas)
    if not tiene_include:
        return False
    # No debe declarar clases, enums, using, typedef, funciones libres, etc.
    declarados_local = declarados(texto)
    return len(declarados_local) == 0


def clases_y_metodos(texto):
    """{Clase: [metodos publicos]} aproximado, con llaves emparejadas."""
    res = {}
    for m in re.finditer(r"^\s*(class|struct)\s+(\w+)\s*(?:final\s*)?(?::[^;{]*)?\{", texto, re.M):
        tipo, nombre = m.group(1), m.group(2)
        i, prof = m.end(), 1
        while i < len(texto) and prof:
            if texto[i] == "{":
                prof += 1
            elif texto[i] == "}":
                prof -= 1
            i += 1
        cuerpo = texto[m.end():i - 1]
        publico = tipo == "struct"
        metodos = []
        nivel = 0
        for linea in cuerpo.splitlines():
            s = linea.strip()
            if nivel == 0:
                if s.startswith("public:"):
                    publico = True
                elif s.startswith(("private:", "protected:")):
                    publico = False
                elif publico:
                    mm = re.match(r"^(?:virtual\s+|static\s+|inline\s+|explicit\s+|constexpr\s+|\[\[nodiscard\]\]\s*)*"
                                  r"(?:const\s+)?[\w:<>,\s\*&]*?\b(\w+)\s*\(", s)
                    if mm and mm.group(1) not in PALABRAS_CPP and mm.group(1) != nombre \
                            and not s.startswith(("return", "if", "for", "while")) and "operator" not in s \
                            and mm.group(1) != "~" + nombre:
                        metodos.append(mm.group(1))
            nivel += linea.count("{") - linea.count("}")
            if nivel < 0:
                nivel = 0
        res[nombre] = sorted(set(metodos))
    return res


def objetivos_cmake(raiz):
    """ejecutable/libreria -> (fuentes, enlaces), desde CMakeLists y cmake/*.cmake."""
    textos = [(raiz / "CMakeLists.txt").read_text(encoding="utf-8")]
    for p in sorted((raiz / "cmake").glob("*.cmake")):
        textos.append(p.read_text(encoding="utf-8"))
    fuentes, enlaces = defaultdict(list), defaultdict(set)

    def bloques(texto, orden):
        for m in re.finditer(orden + r"\((\w+)\b", texto):
            prof, trozo = 1, []
            for linea in texto[m.end():].splitlines():
                linea = linea.split("#")[0]
                fin = False
                for ch in linea:
                    if ch == "(":
                        prof += 1
                    elif ch == ")":
                        prof -= 1
                        if prof == 0:
                            fin = True
                            break
                trozo.append(linea.split(")")[0] if fin else linea)
                if fin:
                    break
            yield m.group(1), " ".join(trozo).split()

    for t in textos:
        for orden in ("add_executable", "add_library"):
            for nombre, toks in bloques(t, orden):
                fuentes[nombre] += [x for x in toks if x.endswith((".cpp", ".c")) and not x.startswith("$")]
        for nombre, toks in bloques(t, "target_link_libraries"):
            enlaces[nombre] |= {x for x in toks if x not in ("PUBLIC", "PRIVATE", "INTERFACE")}
    return fuentes, enlaces


def enlazado(objetivo, fuentes, enlaces):
    todo, pila, vistos = set(), [objetivo], set()
    while pila:
        o = pila.pop()
        if o in vistos:
            continue
        vistos.add(o)
        todo |= set(fuentes.get(o, []))
        pila += list(enlaces.get(o, []))
    return todo


def implementaciones(arbol, cabecera, nombres):
    base = Path(cabecera).stem
    impl = set()
    for r, t in arbol.texto.items():
        if es_cabecera(r) or not r.startswith(("src/", "editor/")):
            continue
        if Path(r).stem == base and cabecera in arbol.cierre(r):
            impl.add(r)
            continue
        # Define (no llama): «Tipo Clase::metodo(» empezando en columna 0.
        # Una llamada «JsonValue::parse(...)» va sangrada dentro de una
        # funcion y no cuenta.
        # Tambien detecta especializaciones de plantilla: template<> Tipo Clase<T>::metodo(
        patrones = []
        for n in nombres:
            if n[0].isupper():
                patrones.append(r"^[A-Za-z_][^\n;=]*\b" + re.escape(n) + r"::~?\w+\s*\(")
                # Template specialization: template<> Ret Clase<T>::metodo(
                patrones.append(r"^template\s*<[^>]*>\s*(?:inline\s+)?[\w:<>,]+[\s\*&]*\b" + re.escape(n) + r"::~?\w+\s*\(")
        if cabecera in arbol.cierre(r) and any(
                re.search(p, t, re.M) for p in patrones):
            impl.add(r)
    return impl


def alcanzar(arbol, entradas, enlace, decl, impl):
    """Devuelve (ficheros alcanzados, {cabecera: por_quien})."""
    alcanzados, porque = set(), {}
    cola = deque()
    for e in entradas:
        if e in arbol.texto:
            alcanzados.add(e)
            cola.append(e)
    
    # Mapa inverso de includes: cabecera -> {cabeceras que la incluyen}
    padres_include = defaultdict(set)
    for h in arbol.texto:
        if es_cabecera(h):
            for inc in arbol.includes.get(h, []):
                if es_cabecera(inc):
                    padres_include[inc].add(h)
    
    # Identificar cabeceras paraguas (solo includes, sin símbolos propios)
    paraguas = {h for h in arbol.texto if es_cabecera(h) and es_paraguas(arbol.texto[h])}
    
    while cola:
        f = cola.popleft()
        toks = arbol.tokens[f]
        for h in arbol.cierre(f):
            if h in porque or not es_cabecera(h):
                continue
            usados = decl.get(h, set()) & toks
            if not usados:
                continue
            porque[h] = (f, sorted(usados)[:4])
            nuevos = {h}
            for c in impl.get(h, ()):
                if c in enlace:
                    nuevos.add(c)
            for n in nuevos:
                if n not in alcanzados:
                    alcanzados.add(n)
                    cola.append(n)
            
            # Si la cabecera alcanzada es un paraguas o tiene paraguas que la incluyen,
            # marcar también los paraguas como alcanzados
            for p in padres_include.get(h, set()):
                if p in paraguas and p not in porque:
                    porque[p] = (f, ["paraguas"])
                    if p not in alcanzados:
                        alcanzados.add(p)
                        cola.append(p)
    return alcanzados, porque


def medir(raiz):
    raiz = Path(raiz).resolve()
    arbol = Arbol(raiz)
    fuentes, enlaces = objetivos_cmake(raiz)
    cabeceras = [r for r in arbol.texto if es_cabecera(r) and r.startswith(("include/", "editor/include/"))]
    decl = {h: declarados(arbol.texto[h]) for h in cabeceras}
    impl = {h: implementaciones(arbol, h, decl[h]) for h in cabeceras}

    informe = {"ejecutables": {}, "cabeceras": {}}
    alcanzados_por = {}
    for exe, quien in EJECUTABLES.items():
        if exe not in fuentes:
            continue
        enl = enlazado(exe, fuentes, enlaces)
        # Se entra SOLO por el fichero que tiene main(). Las demas fuentes
        # propias del ejecutable estan enlazadas, pero enlazar no es usar:
        # cuentan cuando alguien alcanzado usa lo que declaran.
        entradas = [x for x in fuentes[exe]
                    if x in arbol.texto and re.search(r"\bint\s+main\s*\(", arbol.texto[x])]
        alc, porque = alcanzar(arbol, entradas, enl, decl, impl)
        alcanzados_por[exe] = (alc, porque)
        if exe == "juego":
            _ALCANZADOS_JUEGO.clear()
            _ALCANZADOS_JUEGO.update(alc)
        informe["ejecutables"][exe] = {
            "para": quien, "fuentes_propias": entradas,
            "fuentes_enlazadas": len(enl), "ficheros_alcanzados": len(alc),
            "cabeceras_usadas": len([h for h in alc if es_cabecera(h)]),
            "alcanzados": sorted(alc),
        }

    # pruebas / demos: cualquier fuente de tests/ o examples/ que no sea de un ejecutable
    propias = {x for exe in EJECUTABLES for x in fuentes.get(exe, [])}
    otras = [r for r in arbol.texto if r.startswith(("tests/", "examples/")) and not es_cabecera(r) and r not in propias]
    usada_en_prueba = defaultdict(set)
    for r in otras:
        toks = arbol.tokens[r]
        for h in arbol.cierre(r):
            if h in decl and decl[h] & toks:
                usada_en_prueba[h].add(r)

    for h in sorted(cabeceras):
        fila = {"lineas": arbol.crudo[h].count("\n"), "implementacion": sorted(impl[h])}
        for exe, (alc, porque) in alcanzados_por.items():
            if h in porque:
                f, nombres = porque[h]
                fila[exe] = {"desde": f, "por": nombres}
        fila["pruebas"] = len(usada_en_prueba.get(h, ()))
        # metodos publicos nunca nombrados fuera de la propia clase
        sin_llamar = {}
        for exe, (alc, porque) in alcanzados_por.items():
            if h not in porque:
                continue
            propios = {h} | set(impl[h])
            toks_fuera = set()
            for f in alc:
                if f not in propios:
                    toks_fuera |= arbol.tokens[f]
            for clase, metodos in clases_y_metodos(arbol.texto[h]).items():
                if clase not in toks_fuera:
                    continue
                muertos = [m for m in metodos if m not in toks_fuera]
                if muertos:
                    sin_llamar.setdefault(exe, {})[clase] = muertos
        if sin_llamar:
            fila["metodos_sin_llamar"] = sin_llamar
        informe["cabeceras"][h] = fila
    return informe


# ---------------------------------------------------------------------
# Contenido: qué niveles, catálogos y aventuras llegan al jugador
# ---------------------------------------------------------------------

def _json(ruta):
    try:
        return json.loads(Path(ruta).read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return None


def _ids_catalogo(ruta):
    d = _json(ruta)
    if not isinstance(d, dict):
        return None
    return {o.get("id") for o in d.get("objects", []) if isinstance(o, dict)}


def _recorrer_niveles(raiz, inicios, catalogos):
    """BFS por puertas (targetLevel) desde los niveles de inicio.
    Devuelve (alcanzados, puertas_rotas, objetos_sin_catalogo)."""
    ids = set()
    for c in catalogos:
        ids |= _ids_catalogo(raiz / c) or set()
    vistos, rotas, huerfanos = [], [], {}
    cola = deque(inicios)
    while cola:
        nivel = cola.popleft()
        if nivel in vistos:
            continue
        d = _json(raiz / nivel)
        if not isinstance(d, dict) or "map" not in d:
            rotas.append(nivel)
            continue
        vistos.append(nivel)
        if not (raiz / d["map"]).exists():
            rotas.append(nivel + " -> " + d["map"])
        for o in d.get("objects", []):
            if not isinstance(o, dict):
                continue
            if o.get("objectId") and ids and o["objectId"] not in ids:
                huerfanos.setdefault(nivel, set()).add(o["objectId"])
            destino = o.get("targetLevel")
            if destino:
                if (raiz / destino).exists():
                    cola.append(destino)
                else:
                    rotas.append(nivel + " -> " + destino)
    return vistos, sorted(set(rotas)), {k: sorted(v) for k, v in huerfanos.items()}


def medir_contenido(raiz, arbol_textos_juego):
    raiz = Path(raiz).resolve()
    literales = set()
    for texto in arbol_textos_juego:
        # Sin comentarios: un ejemplo de uso en un comentario («hará:
        # classes.loadFromFile("assets/catalogs/classes.json")») no carga nada.
        sin = re.sub(r"/\*.*?\*/", "", texto, flags=re.S)
        sin = "\n".join(l for l in sin.splitlines() if not l.lstrip().startswith("//"))
        sin = re.sub(r"\s//[^\n\"]*$", "", sin, flags=re.M)
        literales |= set(re.findall(r'"(assets/[^"]+)"', sin))
    base_cat = ["assets/objects/boundington_npcs.json"]  # el que juego.cpp pasa siempre
    informe = {"rutas": {}, "niveles_totales": 0}
    todos = sorted(str(p.relative_to(raiz)) for p in (raiz / "assets/levels").glob("*.json") if " 2" not in p.name)
    informe["niveles_totales"] = len(todos)
    alcanzables = set()

    # Campaña: prólogo en El Vacío y, por narrativa, la ciudad.
    campana = [x for x in ("assets/levels/interior_vacio.json", "assets/levels/ciudad_centro.json")
               if x in literales or x.endswith("interior_vacio.json")]
    niv, rotas, huer = _recorrer_niveles(raiz, campana, base_cat)
    alcanzables |= set(niv)
    informe["rutas"]["campaña (Título → Campaña)"] = {
        "inicio": campana, "niveles": len(niv), "puertas_rotas": rotas,
        "objetos_sin_catalogo": sum(len(v) for v in huer.values()),
        "catalogos": base_cat,
    }

    indice = _json(raiz / "assets/proyectos/index.json") or {}
    for pid in indice.get("proyectos", []):
        m = _json(raiz / "assets/proyectos" / (pid + ".json"))
        fila = {}
        if not isinstance(m, dict):
            informe["rutas"]["proyecto " + pid] = {"error": "manifiesto ilegible"}
            continue
        ajustes = m.get("settings", {})
        dir_niv = ajustes.get("levelsDirectory", "levels")
        dir_cat = ajustes.get("catalogsDirectory", "objects")
        dir_av = ajustes.get("adventuresDirectory", "adventures")
        entrada = m.get("entry", "")
        rel = entrada[7:] if entrada.startswith("assets/") else entrada
        inicio, tipo = None, "sin entrada"
        if entrada:
            if "/" in entrada:
                ruta = "assets/" + rel
                if (raiz / ruta).exists():
                    tipo = "aventura" if rel.startswith(dir_av + "/") else "nivel"
                    inicio = ruta
                else:
                    tipo = "entrada que no existe"
            else:
                for d, k in ((dir_av, "aventura"), (dir_niv, "nivel")):
                    if (raiz / "assets" / d / rel).exists():
                        inicio, tipo = "assets/%s/%s" % (d, rel), k
                        break
                else:
                    tipo = "entrada que no existe"
        niveles_decl = ["assets/%s/%s" % (dir_niv, n) for n in m.get("levels", [])]
        if tipo == "aventura":
            # Application::openProject arranca en levels[0] con el guion.
            inicio_nivel = niveles_decl[:1]
        elif tipo == "nivel":
            inicio_nivel = [inicio]
        else:
            inicio_nivel = []
        visible = pid != "boundington" and bool(entrada) and bool(m.get("levels")) and tipo in ("aventura", "nivel")
        cats = ["assets/%s/%s" % (dir_cat, c) for c in m.get("catalogs", [])]
        niv, rotas, huer = _recorrer_niveles(raiz, inicio_nivel, base_cat + cats)
        if visible:
            alcanzables |= set(niv)
        fila.update({
            "nombre": m.get("name", pid), "entrada": entrada, "tipo": tipo,
            "en_el_titulo": visible if pid != "boundington" else "como campaña",
            "niveles_declarados": len(niveles_decl),
            "niveles_alcanzables": len(niv),
            "declarados_inalcanzables": sorted(set(niveles_decl) - set(niv)),
            "puertas_rotas": rotas,
            "objetos_sin_catalogo": sum(len(v) for v in huer.values()),
            "catalogos_que_faltan": [c for c in cats if not (raiz / c).exists()],
        })
        informe["rutas"]["proyecto " + pid] = fila

    informe["niveles_alcanzables_desde_el_titulo"] = len(alcanzables)
    informe["niveles_que_ninguna_ruta_alcanza"] = sorted(set(todos) - alcanzables)

    def cargados(carpeta):
        res = {}
        for p in sorted((raiz / carpeta).glob("*.json")):
            if " 2" in p.name:
                continue
            r = str(p.relative_to(raiz))
            res[r] = r in literales
        return res
    informe["catalogos_rpg_que_lee_el_juego"] = cargados("assets/catalogs")
    aventuras = cargados("assets/adventures")
    for pid in indice.get("proyectos", []):
        m = _json(raiz / "assets/proyectos" / (pid + ".json")) or {}
        e = m.get("entry", "")
        if e.startswith("assets/adventures/") and e in aventuras:
            aventuras[e] = "entrada de " + pid
    if "assets/adventures/boundington_precuela_taberna.json" in aventuras:
        aventuras["assets/adventures/boundington_precuela_taberna.json"] = "solo con juego --precuela"
    informe["aventuras"] = aventuras
    informe["mapas_nativos_con_eventos"] = sorted(
        str(p.relative_to(raiz)) for p in (raiz / "assets").rglob("*.json")
        if p.parent.name == "misiones" or p.name.endswith(".map.json"))
    return informe


# ---------------------------------------------------------------------
# Por paquete del Gantt: ¿llega a alguien lo que el paquete escribió?
# ---------------------------------------------------------------------

def por_paquete(raiz, informe):
    import importlib.util
    ruta = Path(raiz) / "planificacion/pla/dades.py"
    if not ruta.exists():
        return {}
    spec = importlib.util.spec_from_file_location("dades_alcance", ruta)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    juego = set(informe["ejecutables"].get("juego", {}).get("alcanzados", []))
    editor = set(informe["ejecutables"].get("motor_editor", {}).get("alcanzados", []))
    antiguo = set(informe["ejecutables"].get("level_editor", {}).get("alcanzados", []))
    res = {}
    for paq in mod.P:
        pid, ficheros = paq[0], paq[6]
        codigo = [f for f in ficheros if f.startswith(("include/", "src/", "editor/")) and
                  f.endswith((".h", ".cpp")) and (Path(raiz) / f).exists()]
        if not codigo:
            continue
        fila = {"ficheros": len(codigo),
                "juego": sorted(f for f in codigo if f in juego),
                "motor_editor": sorted(f for f in codigo if f in editor),
                "level_editor": sorted(f for f in codigo if f in antiguo),
                "nadie": sorted(f for f in codigo if f not in juego | editor | antiguo)}
        res[pid] = fila
    return res


# Opciones que llevan un valor detras. El valor NO es un argumento suelto.
CON_VALOR = ("--json",)


def _posicionales(argv):
    """Los argumentos que no son opcion ni valor de una opcion.

    POR QUE EXISTE. `medir_alcance.py --json /tmp/x.json` tomaba
    /tmp/x.json como raiz del repositorio y moria buscando su
    CMakeLists.txt. Y ese es, palabra por palabra, el comando que
    CHECKLIST_CIERRE_TAREA.md manda ejecutar en la pregunta 0.1: la
    pregunta que abre el cierre de CADA tarea no se podia contestar.
    """
    sueltos, saltar = [], False
    for a in argv:
        if saltar:
            saltar = False
            continue
        if a in CON_VALOR:
            saltar = True
            continue
        if a.startswith("--"):
            continue
        sueltos.append(a)
    return sueltos


def main(argv):
    args = _posicionales(argv)
    raiz = Path(args[0]) if args else Path(__file__).resolve().parent.parent
    informe = medir(raiz)
    if (raiz / "assets").exists():
        # Los textos que lee el JUGADOR: los que alcanza juego.
        arbol = Arbol(raiz)
        textos = [arbol.crudo[f] for f, fila in ((h, None) for h in arbol.crudo)
                  if f in _ALCANZADOS_JUEGO]
        informe["contenido"] = medir_contenido(raiz, textos)
    informe["paquetes"] = por_paquete(raiz, informe)
    if "--json" in argv:
        destino = Path(argv[argv.index("--json") + 1])
        destino.write_text(json.dumps(informe, ensure_ascii=False, indent=1, sort_keys=True), encoding="utf-8")
    if "--comprobar" in argv:
        # Anclas: si el analizador deja de ver lo evidente, el informe no
        # vale. Mejor fallar que publicar un mapa equivocado.
        anclas = [("juego", "include/Engine/Application.h"), ("juego", "include/Game/GameSession.h"),
                  ("motor_editor", "include/Editor/EditorDocumentSession.h"),
                  ("level_editor", "include/Editor/EditorState.h")]
        malas = [a for a in anclas if a[0] not in informe["cabeceras"].get(a[1], {})]
        if malas:
            print("medir_alcance: el analizador ya no ve " + ", ".join("%s<-%s" % m for m in malas))
            return 2
    for exe, d in informe["ejecutables"].items():
        print(f"{exe} ({d['para']}): {d['cabeceras_usadas']} cabeceras usadas, "
              f"{d['ficheros_alcanzados']} ficheros alcanzados de {d['fuentes_enlazadas']} fuentes enlazadas")
    nadie = [h for h, f in informe["cabeceras"].items() if not any(e in f for e in EJECUTABLES)]
    print(f"cabeceras que no usa ningun ejecutable: {len(nadie)} de {len(informe['cabeceras'])}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
