#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""La guardia del core: la deuda medida puede bajar, nunca subir.

    python3 tools/linea_base_core.py            # la guardia entera
    python3 tools/linea_base_core.py --rapido   # solo la parte instantanea
    python3 tools/linea_base_core.py --actualizar   # refresca la linea base

POR QUE EXISTE
--------------
El 20/09/2026 se descubrio que HEAD llevaba seis commits sin poder
configurar: `CMakeLists.txt` hacia `include(cmake/AventurasCatalogNetTests
.cmake)` y ese fichero nunca se commiteo. Nadie se entero porque la CI
fallaba en el paso de configuracion, antes de compilar, y porque «CI
pendiente» es una frase que aparece en veinte registros de paquete.

Es la cuarta vez que pasa lo mismo (checklist de cierre, pregunta 0.3).
Un documento que lo cuente no lo evita: lo evita una prueba que se ponga
roja el mismo dia.

Lo mismo con la otra deuda medida: `analizar_duplicados.py` sabe contar
los ficheros que no alcanza ningun ejecutable, las pruebas que se prueban
a si mismas y los paquetes cerrados sin consumidor. Saberlo no ha servido
de nada — el informe del 17/09 decia 127 hallazgos y el del 20/09 decia
128. Lo que sirve es fijar el numero de hoy y no dejar que suba.

QUE COMPRUEBA
-------------
1. **Que el repositorio configura.** Cada `include(cmake/X.cmake)` de
   `CMakeLists.txt` tiene su fichero, y ese fichero esta VERSIONADO. Un
   `.cmake` que existe solo en el arbol de trabajo de quien lo escribio
   configura en su maquina y en ninguna otra: es justo el fallo de arriba.
   Esta comprobacion no tiene linea base y no admite excepciones.
2. **Que la deuda no crece.** Se compara con `planificacion/
   linea_base_core.json`: si aparece un fichero muerto nuevo, una prueba
   con la logica dentro o un paquete cerrado sin alcance, falla y lo
   nombra. Si la lista BAJA, pasa y pide que se actualice la linea base.
3. **Que no se mezcle la forma canónica.** Un `.cpp` no puede definir
   una clase local `...Data`/`...State` y dirigir a la vez un
   `...Controller`: los datos se extraen y el controlador los recibe.
   La comprobación es deliberadamente conservadora; solo rechaza ese
   patrón inequívoco de nombres y por eso no necesita excepciones.

NO DUPLICA EL ANALISIS. La medida la hace `tools/analizar_duplicados.py`,
que ya existe y es la fuente. Aqui solo se compara y se decide.
"""
import argparse
import json
import os
import re
import subprocess
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ_POR_DEFECTO = os.path.dirname(AQUI)
BASE_REL = os.path.join("planificacion", "linea_base_core.json")

# El techo. Por encima de esto un fichero deja de ser trabajable en
# trozos: no cabe en la cabeza de nadie, lo cruzan varios paquetes a la
# vez y dos agentes que lo abran el mismo dia se pisan seguro.
#
# 21/09/2026: baja de 600 a 500 por decision de Oriol. El numero no es
# sagrado —a nadie le van a echar por 520 lineas— pero la lista solo
# puede bajar, y eso si lo es.
#
# EL ESTADO IMPORTA MAS QUE LAS LINEAS. Un fichero es largo porque su
# clase guarda demasiadas cosas: `Application.h` declara CIENTO CUATRO
# miembros, 2,6 veces mas que la siguiente. Por eso se mide tambien, con
# el mismo trinquete: los datos van en clases de datos que se pasan como
# parametro, no en el estado del controlador.
UMBRAL_MIEMBROS = 15
TECHO = 500
CARPETAS_DEL_CORE = ("src", "include", "editor/src", "editor/include")

# La frontera. Una libreria que mezcla ficheros que hablan con OpenGL y
# ficheros que no, obliga a todo el que quiera probar los segundos a
# arrastrar una ventana — o a esquivarla compilando las fuentes sueltas
# dentro de su propio objetivo, que es lo que hacen hoy 28 objetivos con
# 36 fuentes, 64 veces. De ahi sale la «logica dentro del test».
MARCAS_GL = ("glad/glad.h", "GLFW/glfw3.h", "<GL/")

INCLUDE = re.compile(r"^\s*include\(\s*(cmake/[A-Za-z0-9_./-]+\.cmake)\s*\)",
                     re.M)

# Fuentes que un fichero de CMake nombra. El inicio exigido (principio de
# linea, espacio, comilla o parentesis) deja fuera las rutas construidas
# con una variable -- "${GLAD_DIR}/src/glad.c" no es una ruta del
# repositorio y no se puede comprobar contra git.
#
# LIMITE CONOCIDO: una fuente montada dentro de un foreach() con una
# variable ("tests/Render/${_fuente}.cpp") tampoco se ve. Ahi el autor
# sigue solo.
FUENTE_CITADA = re.compile(
    r'(?:^|[\s"(])((?:tests|src|editor|examples)/[\w./+-]+\.(?:cpp|cc|c))', re.M)

# Las tres familias de deuda que esta guardia congela. La clave es la que
# usa el informe de analizar_duplicados; el texto es lo que se lee cuando
# la prueba se pone roja.
FAMILIAS = {
    "MUERTO": ("muertos",
               "no lo alcanza ningun ejecutable"),
    "LOGICA_EN_EL_TEST": ("logica_en_pruebas",
                          "su logica vive dentro de su propia prueba"),
    "REGISTRO_CERRADO_SIN_ALCANCE": ("cierres_sin_alcance",
                                     "paquete cerrado que no llega a nadie"),
}


MIEMBRO = re.compile(r"^\s{2,}[A-Za-z_][\w:<>,\*&\s]*\bm_[A-Za-z]\w*\s*(=|;|\{|\[)")


def mide_estado(raiz, umbral=UMBRAL_MIEMBROS):
    """Cabeceras cuya clase guarda demasiado estado propio.

    La regla de la casa desde el 21/09: para cada logica, una clase de
    DATOS (que los guarda), un CONTROLADOR (que los recibe y dirige el
    flujo) y varias clases que se reparten el trabajo. Un controlador con
    cuarenta miembros no esta dirigiendo: esta guardando, y entonces nadie
    puede probar una parte sin construir el todo.

    Una clase de DATOS con muchos campos es legitima y aqui no molesta: su
    numero se congela y ya esta. Lo que la guardia impide es que crezca.
    """
    fuera = {}
    for base in ("include", "editor/include"):
        carpeta = os.path.join(raiz, base)
        if not os.path.isdir(carpeta):
            continue
        for r, _, nombres in os.walk(carpeta):
            for n in nombres:
                if not n.endswith((".h", ".hpp")):
                    continue
                ruta = os.path.join(r, n)
                with open(ruta, encoding="utf-8", errors="replace") as f:
                    cuenta = sum(1 for l in f if MIEMBRO.match(l))
                if cuenta > umbral:
                    fuera[os.path.relpath(ruta, raiz).replace(os.sep, "/")] = cuenta
    return fuera


def mide_cuellos(raiz, techo=TECHO):
    """Ficheros del core por encima del techo, con su tamano de hoy."""
    fuera = {}
    for base in CARPETAS_DEL_CORE:
        for carpeta, _, nombres in os.walk(os.path.join(raiz, base)):
            for n in nombres:
                if not n.endswith((".cpp", ".h", ".hpp")):
                    continue
                ruta = os.path.join(carpeta, n)
                rel = os.path.relpath(ruta, raiz).replace(os.sep, "/")
                with open(ruta, encoding="utf-8", errors="replace") as f:
                    lineas = sum(1 for _ in f)
                if lineas > techo:
                    fuera[rel] = lineas
    return fuera


def _bloques(texto, orden):
    """(nombre, cuerpo) de cada add_library/add_executable del CMake.

    Con contador de parentesis y no con una expresion regular: dentro de
    un `add_executable` hay `$<TARGET_FILE:x>` y `${VAR}`, y una regla
    perezosa corta en el primer `)` y se deja medio bloque fuera. Se
    descubrio contando 21 fuentes sueltas donde habia 64.
    """
    salida, i = [], 0
    while True:
        i = texto.find(orden + "(", i)
        if i < 0:
            return salida
        j, nivel = i + len(orden) + 1, 1
        while j < len(texto) and nivel:
            nivel += (texto[j] == "(") - (texto[j] == ")")
            j += 1
        cuerpo = texto[i + len(orden) + 1:j - 1]
        nombre = cuerpo.split()[0] if cuerpo.split() else ""
        salida.append((nombre, cuerpo))
        i = j


def _habla_gl(raiz, rel):
    ruta = os.path.join(raiz, rel)
    if not os.path.isfile(ruta):
        return False
    with open(ruta, encoding="utf-8", errors="replace") as f:
        texto = f.read()
    return any(marca in texto for marca in MARCAS_GL)


def _cmakes(raiz):
    rutas = [os.path.join(raiz, "CMakeLists.txt")]
    carpeta = os.path.join(raiz, "cmake")
    if os.path.isdir(carpeta):
        rutas += [os.path.join(carpeta, n) for n in sorted(os.listdir(carpeta))
                  if n.endswith(".cmake")]
    return [r for r in rutas if os.path.isfile(r)]


def mide_librerias(raiz):
    """Librerias que cruzan la frontera GL, y fuentes compiladas sueltas.

    `mezcladas`: una libreria con fuentes GL y fuentes GL-free dentro.
    `fuentes_directas`: cuantas veces un objetivo compila un `src/*.cpp`
    en vez de enlazar la libreria que lo contiene. Es el sintoma, y por
    eso se cuenta: mientras no baje, la frontera no esta puesta.
    """
    mezcladas, directas = [], 0
    for ruta in _cmakes(raiz):
        with open(ruta, encoding="utf-8", errors="replace") as f:
            texto = f.read()
        for lib, cuerpo in _bloques(texto, "add_library"):
            fuentes = sorted(set(re.findall(r"(src/[\w/]+\.cpp)", cuerpo)))
            if not fuentes:
                continue
            con = [f for f in fuentes if _habla_gl(raiz, f)]
            sin = [f for f in fuentes if not _habla_gl(raiz, f)]
            if con and sin:
                mezcladas.append(f"{lib} ({len(con)} con GL, {len(sin)} sin)")
        for _obj, cuerpo in _bloques(texto, "add_executable"):
            directas += len(set(re.findall(r"(src/[\w/]+\.cpp)", cuerpo)))
    return {"mezcladas": sorted(set(mezcladas)), "fuentes_directas": directas}


_REHEN = re.compile(r"^(?P<lib>.+?) \((?P<con>\d+) con GL, (?P<sin>\d+) sin\)$")


def _rehenes(mezcladas):
    """{libreria: cuantas fuentes GL-free tiene atrapadas}.

    Acepta la foto tal como se escribe en la linea base (una lista de
    textos "lib (N con GL, M sin)") y tambien una linea base vieja con
    cualquier otro texto: lo que no case, cuenta como una libreria
    mezclada de la que no sabemos el numero de rehenes (0), que es la
    lectura prudente -- no inventa una mejora que nadie ha hecho.
    """
    salida = {}
    for entrada in (mezcladas or []):
        casa = _REHEN.match(str(entrada))
        if casa:
            salida[casa.group("lib")] = int(casa.group("sin"))
        else:
            salida[str(entrada)] = 0
    return salida


def frescura_del_alcance(raiz):
    """¿La medida de alcance que se esta usando es de hoy o de hace tres dias?

    `analizar_duplicados.py` no calcula el alcance: CONSUME
    `planificacion/alcance_real.json`. Si esa foto es vieja, el veredicto
    «no lo alcanza ningun ejecutable» habla de un repositorio que ya no
    existe — y esta guardia estaria congelando una deuda inventada. Se
    descubrio el 20/09: la foto era del 17 y seguia diciendo que un
    fichero borrado estaba muerto.
    """
    cache = os.path.join(raiz, "planificacion", "alcance_real.json")
    if not os.path.isfile(cache):
        return ["falta planificacion/alcance_real.json: los muertos no se "
                "pueden medir. python3 tools/medir_alcance.py "
                "--json planificacion/alcance_real.json"]
    cuando = os.path.getmtime(cache)
    mas_nuevo, nombre = 0.0, ""
    for base in CARPETAS_DEL_CORE:
        for carpeta, _, nombres in os.walk(os.path.join(raiz, base)):
            for n in nombres:
                if not n.endswith((".cpp", ".h", ".hpp")):
                    continue
                t = os.path.getmtime(os.path.join(carpeta, n))
                if t > mas_nuevo:
                    mas_nuevo, nombre = t, os.path.relpath(
                        os.path.join(carpeta, n), raiz)
    if mas_nuevo <= cuando:
        return []
    dias = (mas_nuevo - cuando) / 86400.0
    return [f"planificacion/alcance_real.json es {dias:.1f} dias mas viejo "
            f"que {nombre}: los muertos de abajo salen de esa foto y pueden "
            "estar desfasados. Regenerala: python3 tools/medir_alcance.py "
            "--json planificacion/alcance_real.json"]


def versionados(raiz):
    """Ficheros que git conoce. Vacio si aqui no hay git (no se inventa)."""
    try:
        salida = subprocess.run(
            ["git", "-C", raiz, "ls-files"],
            capture_output=True, text=True, timeout=60)
    except (OSError, subprocess.SubprocessError):
        return None
    if salida.returncode != 0:
        return None
    prefijo = ""
    cima = subprocess.run(["git", "-C", raiz, "rev-parse", "--show-prefix"],
                          capture_output=True, text=True)
    if cima.returncode == 0:
        prefijo = cima.stdout.strip()
    conocidos = set()
    for linea in salida.stdout.splitlines():
        if prefijo and linea.startswith(prefijo):
            linea = linea[len(prefijo):]
        conocidos.add(linea)
    return conocidos


DEFINICION = re.compile(r"^[A-Za-z_][\w:<>,&*\s]*?\b([A-Z]\w+)::(~?\w+)\s*\(")

# DEFINICION solo casa `Clase::metodo`. Una funcion LIBRE dentro de un
# namespace con nombre (`namespace RPG { ValidationReport ValidateAdventure(...)`)
# no lleva calificador en la definicion, asi que se escapaba: el 28/09/2026
# ValidateAdventure estaba definida en dos fuentes y la regla no lo vio. Estos
# tres patrones cubren ese caso.
NS_NOMBRADO = re.compile(r"^namespace\s+([A-Za-z_]\w*(?:::\w+)*)\s*\{")
NS_ANONIMO = re.compile(r"^namespace\s*\{")
DEFINICION_LIBRE = re.compile(r"^[A-Za-z_][\w:<>,&*\s]*?\b(~?[A-Za-z_]\w*)\s*\(")
# Lo que empieza una linea pero no es una definicion de funcion.
NO_DEFINICION = re.compile(
    r"^(static|inline|template|constexpr|extern|friend|virtual|explicit|using"
    r"|return|if|for|while|switch|else|do|case|default|goto|break|continue"
    r"|namespace|struct|class|enum|union|typedef|public|private|protected"
    r"|try|catch|throw|operator|delete|new|sizeof|decltype|noexcept"
    r"|\}|\)|#)\b")

# EST-50 — La forma canónica usa nombres explícitos. No intentamos adivinar
# semántica a partir de cualquier `struct` o de cualquier método: eso
# castigaría adaptadores y RAII locales legítimos. Sí es inequívoco que un
# fichero que DEFINE `DialogData` y además IMPLEMENTA `DialogController`
# conserva datos y flujo en el mismo lugar.
DEFINICION_DATO_LOCAL = re.compile(
    r"^\s*(?:class|struct)\s+[A-Za-z_]\w*(?:Data|State)\b[^;{]*\{", re.M)
DEFINICION_CONTROLADOR_LOCAL = re.compile(
    r"^\s*class\s+[A-Za-z_]\w*Controller\b[^;{]*\{", re.M)
METODO_CONTROLADOR = re.compile(
    r"^[^/\n]*\b[A-Za-z_]\w*Controller::~?[A-Za-z_]\w*\s*\(", re.M)


def _sin_comentarios_cpp(linea, en_bloque):
    """La linea sin comentarios ni literales, y si seguimos dentro de /* */.

    Hace falta porque las llaves de un comentario o de un string romperian el
    recuento de profundidad, y de ahi salian los falsos positivos del primer
    intento. No entiende raw strings (R"(...)"): en este repo no hay, y si
    algun dia aparece el efecto es un recuento mal, no un falso positivo
    silencioso.
    """
    salida = []
    i, n = 0, len(linea)
    while i < n:
        c = linea[i]
        if en_bloque:
            if c == "*" and i + 1 < n and linea[i + 1] == "/":
                en_bloque = False
                i += 2
            else:
                i += 1
            continue
        if c == "/" and i + 1 < n and linea[i + 1] == "/":
            break
        if c == "/" and i + 1 < n and linea[i + 1] == "*":
            en_bloque = True
            i += 2
            continue
        if c == '"' or c == "'":
            i += 1
            while i < n:
                if linea[i] == "\\":
                    i += 2
                    continue
                if linea[i] == c:
                    i += 1
                    break
                i += 1
            salida.append(" ")
            continue
        salida.append(c)
        i += 1
    return "".join(salida), en_bloque


def comprueba_simbolos(raiz):
    """Ningun `Clase::metodo` definido en dos fuentes a la vez.

    No lleva linea base y no admite excepciones: dos definiciones del mismo
    simbolo son un error de enlace, siempre.

    POR QUE EXISTE. El 23/09/2026 aparecio que los dos relevos de los dias
    21 y 22 habian sido revertidos: alguien guardo `ProjectSerializer.cpp`
    y `NarrativeEngine.cpp` desde una copia anterior al corte. Los ficheros
    nuevos seguian en disco, los originales volvieron a tener los mismos
    cuerpos dentro, y las lineas del CMakeLists que los enganchaban se
    perdieron. Resultado: cada funcion definida dos veces y nadie enterado,
    porque lo que no esta en el CMakeLists no se compila.

    El techo de lineas lo destapo dos dias tarde. Esto lo destapa al
    momento, y es la misma familia de fallo que ya conocia el repo
    (tools/comprobar_fragmentos_cmake.py, 13/09: cinco fragmentos
    huerfanos de tres agentes).
    """
    import collections
    donde = collections.defaultdict(set)
    for base in ("src", "editor/src"):
        carpeta = os.path.join(raiz, base)
        if not os.path.isdir(carpeta):
            continue
        for r, _, nombres in os.walk(carpeta):
            for n in nombres:
                if not n.endswith(".cpp"):
                    continue
                ruta = os.path.join(r, n)
                rel = os.path.relpath(ruta, raiz).replace(os.sep, "/")
                pila = []        # (nombre del namespace o None, profundidad)
                prof = 0
                en_bloque = False
                with open(ruta, encoding="utf-8", errors="replace") as f:
                    for linea in f:
                        limpia, en_bloque = _sin_comentarios_cpp(linea, en_bloque)

                        m = DEFINICION.match(limpia)
                        if m:
                            donde[m.group(1) + "::" + m.group(2)].add(rel)
                        elif (pila and None not in [x[0] for x in pila]
                              and prof == pila[-1][1] + 1
                              and not limpia.rstrip().endswith(";")
                              and not NO_DEFINICION.match(limpia)):
                            ml = DEFINICION_LIBRE.match(limpia)
                            if ml:
                                ns = "::".join(x[0] for x in pila)
                                donde[ns + "::" + ml.group(1)].add(rel)

                        apertura = NS_NOMBRADO.match(limpia)
                        if apertura:
                            pila.append((apertura.group(1), prof))
                        elif NS_ANONIMO.match(limpia):
                            pila.append((None, prof))

                        prof += limpia.count("{") - limpia.count("}")
                        while pila and prof <= pila[-1][1]:
                            pila.pop()
    problemas = []
    for simbolo, ficheros in sorted(donde.items()):
        if len(ficheros) > 1:
            problemas.append(
                f"{simbolo} esta definido en {len(ficheros)} fuentes "
                f"({', '.join(sorted(ficheros))}): eso no enlaza. Si acabas "
                "de partir un fichero, al original le sobra ese cuerpo; si "
                "acabas de guardar encima de un corte ajeno, mira el "
                "CMakeLists antes de seguir")
    return problemas


def comprueba_estructura_mixta(raiz):
    """Detecta datos locales y flujo de controlador en el mismo `.cpp`.

    No es un analizador C++ completo: vigila el convenio que hace revisable
    el corte (`XData`/`XState` y `XController`). Al limitarse a declaraciones
    reales y métodos de controlador, una mención en un comentario o una
    clase de datos aislada no da una alarma falsa.
    """
    problemas = []
    for base in ("src", "editor/src"):
        carpeta = os.path.join(raiz, base)
        if not os.path.isdir(carpeta):
            continue
        for r, _, nombres in os.walk(carpeta):
            for n in nombres:
                if not n.endswith(".cpp"):
                    continue
                ruta = os.path.join(r, n)
                with open(ruta, encoding="utf-8", errors="replace") as f:
                    texto = f.read()
                if not DEFINICION_DATO_LOCAL.search(texto):
                    continue
                if (DEFINICION_CONTROLADOR_LOCAL.search(texto)
                        or METODO_CONTROLADOR.search(texto)):
                    rel = os.path.relpath(ruta, raiz).replace(os.sep, "/")
                    problemas.append(
                        f"{rel}: define datos locales y dirige un Controller. "
                        "Extrae XData/XState a su propio fichero y deja al "
                        "controlador recibirlo")
    return sorted(problemas)


def _del_indice(raiz, rel):
    """El contenido que se va a COMMITEAR de un fichero, no el del disco."""
    try:
        nombre = subprocess.run(["git", "-C", raiz, "ls-files", "--full-name", rel],
                                capture_output=True, text=True, timeout=30)
        if nombre.returncode != 0 or not nombre.stdout.strip():
            return None
        salida = subprocess.run(["git", "-C", raiz, "show", ":" + nombre.stdout.strip()],
                                capture_output=True, text=True, timeout=30)
        return salida.stdout if salida.returncode == 0 else None
    except (OSError, subprocess.SubprocessError):
        return None


def comprueba_configuracion(raiz, desde_indice=False):
    """Los include(cmake/X) que dejarian el repo sin configurar.

    `desde_indice=True` mira lo que se COMMITEA, no lo que hay en disco.

    POR QUE. El 24/09/2026 esta guardia, puesta de gancho en pre-commit,
    bloqueo un commit que no tenia nada que ver: otra sesion acababa de
    anadir un include() en SU arbol de trabajo y su .cmake aun no estaba
    versionado. HEAD estaba a salvo y el commit en curso tampoco lo
    tocaba, pero el gancho miraba el disco y decia que no.

    Una regla que castiga a quien no la incumple deja de cumplirse. En el
    gancho se mira el indice; fuera del gancho se sigue mirando el disco,
    que es lo que quieres cuando preguntas «como esta esto ahora mismo».
    """
    """Los include() de CMakeLists.txt que dejarian el repo sin configurar."""
    problemas = []
    texto = _del_indice(raiz, "CMakeLists.txt") if desde_indice else None
    if texto is None:
        ruta = os.path.join(raiz, "CMakeLists.txt")
        if not os.path.isfile(ruta):
            return [f"no encuentro {ruta}"]
        with open(ruta, encoding="utf-8", errors="replace") as f:
            texto = f.read()
    conocidos = versionados(raiz)
    for rel in sorted(set(INCLUDE.findall(texto))):
        if desde_indice:
            # En el indice lo unico que importa es si el .cmake va a estar
            # versionado: si lo esta, cualquier clon lo tendra.
            if conocidos is not None and rel not in conocidos:
                problemas.append(
                    f"CMakeLists.txt hace include({rel}) y ese fichero NO ESTA "
                    "VERSIONADO: va en el mismo commit que su include()")
            continue
        if not os.path.isfile(os.path.join(raiz, rel)):
            problemas.append(
                f"CMakeLists.txt hace include({rel}) y ese fichero NO EXISTE: "
                "el repositorio no configura en ninguna maquina")
        elif conocidos is not None and rel not in conocidos:
            problemas.append(
                f"CMakeLists.txt hace include({rel}) y ese fichero NO ESTA "
                "VERSIONADO: configura en el arbol de quien lo escribio y en "
                "ningun clon. Va en el mismo commit que su include()")
    return problemas


def _sin_comentarios(texto):
    """CMake comenta con #. Un comentario que nombra un fichero no lo cita."""
    limpio = []
    for linea in texto.split("\n"):
        if linea.lstrip().startswith("#"):
            continue
        corte = linea.find(" #")
        limpio.append(linea if corte < 0 else linea[:corte])
    return "\n".join(limpio)


def _cmakes_del_indice(raiz):
    """CMakeLists.txt y los fragmentos que EL INDICE incluye de verdad.

    Por que solo los incluidos. La primera version devolvia todos los
    `cmake/*.cmake` versionados, y eso castiga un fragmento HUERFANO: uno que
    ninguna linea de CMakeLists incluye no participa en la configuracion y por
    tanto no puede romper ningun clon. En HEAD hay uno de verdad
    (`cmake/FichaVistaTests.cmake`, versionado el 17/09 por el commit
    automatico del arbol entero, sin sus cuatro fuentes y sin su `include()`),
    y con la version anterior bloqueaba todos los commits de todos los agentes
    de forma permanente justo cuando el gancho empezaba a mirar el indice.

    La ley no se relaja por esto. Lo que se incluye se comprueba igual que
    antes, y un fragmento que se incluya sin estar versionado lo canta
    comprueba_configuracion («va en el mismo commit que su include()»), que es
    la comprobacion que de verdad protege la configuracion de un clon.
    """
    conocidos = versionados(raiz) or set()
    texto = _del_indice(raiz, "CMakeLists.txt")
    if texto is None:
        # Sin CMakeLists en el indice no hay configuracion que juzgar: el
        # bucle de comprueba_fuentes no encontrara texto y no dira nada.
        return ["CMakeLists.txt"]
    incluidos = set(INCLUDE.findall(_sin_comentarios(texto)))
    return ["CMakeLists.txt"] + sorted(
        r for r in conocidos
        if r.startswith("cmake/") and r.endswith(".cmake") and r in incluidos)


def comprueba_fuentes(raiz, desde_indice=False):
    """Fuentes que CMake nombra y que git no conoce.

    POR QUE EXISTE ESTA REGLA. El 24/09/2026 aparecieron siete baterias de
    pruebas -- 126 casos, entre ellas 23 sobre la camara -- que CMake citaba
    desde C-24 y que no estaban versionadas: existian SOLO en el disco de
    quien las escribio. En un clon limpio, CMake no las encontraba. Nadie se
    entero porque esos objetivos tampoco enlazaban ninguna libreria, asi que
    nunca se construian: dos fallos tapandose el uno al otro.

    Es la misma ley que ya se aplicaba a los include(cmake/X) -- "configura
    en el arbol de quien lo escribio y en ningun clon" -- pero para las
    fuentes, que es donde vive el codigo.

    EN EL INDICE FALLA; EN EL DISCO SOLO AVISA. Mirando el disco, lo que se
    ve casi siempre es trabajo en vuelo de OTRA sesion, y una regla que
    castiga a quien no la incumple deja de cumplirse (misma razon por la que
    comprueba_configuracion tiene modo indice). El gancho de pre-commit mira
    el indice y ahi si es un fallo: lo que tu commiteas, tiene que configurar.
    """
    conocidos = versionados(raiz)
    if conocidos is None:
        return []
    if desde_indice:
        nombres = _cmakes_del_indice(raiz)
    else:
        nombres = ["CMakeLists.txt"] + [
            os.path.relpath(r, raiz) for r in _cmakes(raiz)[1:]]
    problemas = []
    for rel in nombres:
        texto = _del_indice(raiz, rel) if desde_indice else None
        if texto is None and not desde_indice:
            ruta = os.path.join(raiz, rel)
            if os.path.isfile(ruta):
                with open(ruta, encoding="utf-8", errors="replace") as f:
                    texto = f.read()
        if texto is None:
            continue
        for fuente in sorted(set(FUENTE_CITADA.findall(_sin_comentarios(texto)))):
            if fuente in conocidos:
                continue
            if not desde_indice and not os.path.isfile(os.path.join(raiz, fuente)):
                problemas.append(
                    f"{rel} compila {fuente} y ese fichero NO EXISTE: el "
                    "repositorio no configura en ninguna maquina")
            else:
                problemas.append(
                    f"{rel} compila {fuente} y ese fichero NO ESTA VERSIONADO: "
                    "configura en el arbol de quien lo escribio y en ningun "
                    "clon. Va en el mismo commit que su linea de CMake")
    return problemas


def mide(raiz, informe_json=None):
    """La foto de hoy. La saca analizar_duplicados; aqui solo se ordena."""
    if informe_json is None:
        destino = os.path.join(
            os.environ.get("TMPDIR", "/tmp"), "linea_base_core_informe.json")
        orden = [sys.executable, os.path.join(AQUI, "analizar_duplicados.py"),
                 "--json", destino]
        r = subprocess.run(orden, cwd=raiz, capture_output=True, text=True)
        if r.returncode not in (0, 1) or not os.path.isfile(destino):
            raise RuntimeError(
                "no he podido medir con analizar_duplicados.py: "
                + (r.stderr or r.stdout or "sin salida")[-400:])
        informe_json = destino
    with open(informe_json, encoding="utf-8") as f:
        informe = json.load(f)

    foto = {clave: [] for clave, _ in FAMILIAS.values()}
    for hallazgo in informe.get("hallazgos", []):
        par = FAMILIAS.get(hallazgo.get("tipo"))
        if not par:
            continue
        archivos = hallazgo.get("archivos")
        if not isinstance(archivos, list):
            archivos = [archivos]
        foto[par[0]].extend(a for a in archivos if a)
    salida = {clave: sorted(set(valores)) for clave, valores in foto.items()}
    salida["cuellos"] = mide_cuellos(raiz)
    salida["estado"] = mide_estado(raiz)
    salida.update(mide_librerias(raiz))
    return salida


def compara(actual, base):
    """(problemas, mejoras). Funcion pura: es la que prueba el test."""
    problemas, mejoras = [], []
    for tipo, (clave, explicacion) in FAMILIAS.items():
        antes = set(base.get(clave, []))
        ahora = set(actual.get(clave, []))
        for nuevo in sorted(ahora - antes):
            problemas.append(f"{nuevo}: {explicacion}. No estaba en la linea "
                             f"base del {base.get('fecha', '?')}")
        for ido in sorted(antes - ahora):
            mejoras.append(f"{ido}: ya no aparece como «{tipo}»")

    # El trinquete: un fichero que ya pasa el techo puede adelgazar, no
    # engordar; y ninguno nuevo puede cruzarlo. Es lo que convierte
    # «habria que partir Application.cpp» en algo que se cumple solo.
    antes = base.get("cuellos", {}) or {}
    ahora = actual.get("cuellos", {}) or {}
    for ruta, lineas in sorted(ahora.items()):
        if ruta not in antes:
            problemas.append(
                f"{ruta}: {lineas} lineas, ha pasado el techo de {TECHO}. "
                "Un fichero nuevo no nace grande: parte lo que estes "
                "anadiendo en su propia clase, con su prueba")
        elif lineas > antes[ruta]:
            problemas.append(
                f"{ruta}: ha crecido de {antes[ruta]} a {lineas} lineas y ya "
                f"estaba por encima del techo de {TECHO}. Lo que anadas aqui "
                "va en un fichero propio, o sacas antes otro tanto")
        elif lineas < antes[ruta]:
            mejoras.append(f"{ruta}: baja de {antes[ruta]} a {lineas} lineas")
    for ruta in sorted(set(antes) - set(ahora)):
        mejoras.append(f"{ruta}: ya esta por debajo del techo de {TECHO}")

    # El estado: mismo trinquete que el techo.
    antes = base.get("estado", {}) or {}
    ahora = actual.get("estado", {}) or {}
    for ruta, n in sorted(ahora.items()):
        if ruta not in antes:
            problemas.append(
                f"{ruta}: su clase declara {n} miembros (umbral {UMBRAL_MIEMBROS}). "
                "Lo que ibas a guardar va en una clase de datos que se pasa "
                "como parametro, no en el estado del controlador")
        elif n > antes[ruta]:
            problemas.append(
                f"{ruta}: ha pasado de {antes[ruta]} a {n} miembros. Cada "
                "miembro nuevo de una clase asi hay que justificarlo por "
                "escrito, o sacarlo a una clase de datos")
        elif n < antes[ruta]:
            mejoras.append(f"{ruta}: baja de {antes[ruta]} a {n} miembros")
    for ruta in sorted(set(antes) - set(ahora)):
        mejoras.append(f"{ruta}: ya no acumula estado por encima del umbral")

    # La frontera GL.
    #
    # Se compara por NOMBRE de libreria, no por el texto entero. Comparar
    # el texto hacia que anadir una fuente CON GL a una libreria que YA
    # estaba mezclada contase a la vez como deuda nueva ("motor_core (16
    # con GL, 12 sin)" no estaba en la base) y como mejora ("motor_core
    # (13 con GL, 12 sin)" ya no esta) -- las dos mentiras a la vez, y la
    # segunda es la peor: decia "ya no cruza la frontera GL" de una
    # libreria que la sigue cruzando. Una guardia que miente una vez deja
    # de leerse.
    #
    # Lo que duele es un REHEN: una fuente GL-free atrapada en una
    # libreria con GL, que obliga a abrir una ventana para probarla. Por
    # eso el numero que sube o baja es el de rehenes, y no el total.
    antes_m = _rehenes(base.get("mezcladas"))
    ahora_m = _rehenes(actual.get("mezcladas"))
    for lib in sorted(set(ahora_m) - set(antes_m)):
        problemas.append(
            f"{lib}: esta libreria mezcla fuentes con GL y sin GL. Quien "
            "quiera probar las segundas tendra que abrir una ventana o "
            "compilarlas sueltas. Separa la libreria")
    for lib in sorted(set(antes_m) - set(ahora_m)):
        mejoras.append(f"{lib}: ya no cruza la frontera GL")
    for lib in sorted(set(antes_m) & set(ahora_m)):
        if ahora_m[lib] > antes_m[lib]:
            problemas.append(
                f"{lib}: de {antes_m[lib]} a {ahora_m[lib]} fuentes GL-free "
                "atrapadas en una libreria con GL. Cada una es una prueba "
                "que ahora necesita ventana. Sacalas o parte la libreria")
        elif ahora_m[lib] < antes_m[lib]:
            mejoras.append(
                f"{lib}: de {antes_m[lib]} a {ahora_m[lib]} fuentes GL-free "
                "atrapadas")

    antes_d = base.get("fuentes_directas")
    ahora_d = actual.get("fuentes_directas")
    if isinstance(antes_d, int) and isinstance(ahora_d, int):
        if ahora_d > antes_d:
            problemas.append(
                f"fuentes de src/ compiladas sueltas dentro de un objetivo: "
                f"de {antes_d} a {ahora_d}. Enlaza la libreria que las "
                "contiene; si no puedes, es que esa libreria hay que partirla")
        elif ahora_d < antes_d:
            mejoras.append(f"fuentes compiladas sueltas: baja de {antes_d} "
                           f"a {ahora_d}")
    return problemas, mejoras


def escribe_base(raiz, foto, fecha):
    ruta = os.path.join(raiz, BASE_REL)
    contenido = dict(foto)
    contenido["_nota"] = (
        "Linea base del core: la deuda medida el dia que se congelo. "
        "La comprueba tools/linea_base_core.py y la mide "
        "tools/analizar_duplicados.py. Esta lista puede BAJAR (y entonces se "
        "actualiza con --actualizar, diciendo en el commit que se retiro o se "
        "cablea); no puede subir. Si tu paquete anade un fichero aqui, es que "
        "has escrito codigo que no llega a nadie: cablealo o no lo escribas.")
    contenido["fecha"] = fecha
    contenido["totales"] = {k: (v if isinstance(v, int) else len(v))
                            for k, v in foto.items()}
    contenido["techo"] = TECHO
    with open(ruta, "w", encoding="utf-8") as f:
        json.dump(contenido, f, ensure_ascii=False, indent=1, sort_keys=True)
    return ruta


def main(argv=None):
    ap = argparse.ArgumentParser(description="La guardia del core.")
    ap.add_argument("--raiz", default=RAIZ_POR_DEFECTO)
    ap.add_argument("--indice", action="store_true",
                    help="mira lo que se va a commitear en vez del disco "
                         "(lo que usa el gancho de pre-commit)")
    ap.add_argument("--rapido", action="store_true",
                    help="comprobaciones instantaneas de configuracion y estructura")
    ap.add_argument("--actualizar", action="store_true",
                    help="reescribe la linea base con la foto de hoy")
    ap.add_argument("--informe", help="informe json ya medido (para pruebas)")
    ap.add_argument("--estricto", action="store_true",
                    help="tambien falla si la deuda BAJA y no se ha "
                         "actualizado la linea base")
    a = ap.parse_args(argv)
    raiz = os.path.abspath(a.raiz)

    problemas = comprueba_configuracion(raiz, a.indice)
    for p in problemas:
        print("  CONFIGURACION: " + p)
    fuentes = comprueba_fuentes(raiz, a.indice)
    if a.indice:
        # Lo que TU commiteas tiene que configurar en cualquier clon.
        for p in fuentes:
            print("  FUENTE SIN VERSIONAR: " + p)
        problemas += fuentes
    else:
        # En el disco, casi siempre es trabajo en vuelo de otra sesion.
        for p in fuentes:
            print("  AVISO: " + p)
    duplicados = comprueba_simbolos(raiz)
    for p in duplicados:
        print("  SIMBOLO DUPLICADO: " + p)
    problemas += duplicados
    estructuras_mixtas = comprueba_estructura_mixta(raiz)
    for p in estructuras_mixtas:
        print("  ESTRUCTURA MIXTA: " + p)
    problemas += estructuras_mixtas
    if a.rapido:
        if problemas:
            print(f"linea_base_core: FALLO — {len(problemas)} problema(s) "
                  "instantaneo(s)")
            return 1
        print("linea_base_core: OK — configuracion y estructura instantanea "
              "correctas")
        return 0

    try:
        foto = mide(raiz, a.informe)
    except RuntimeError as error:
        print(f"linea_base_core: no se ha podido medir: {error}")
        return 2

    if a.actualizar:
        import time
        ruta = escribe_base(raiz, foto, time.strftime("%Y-%m-%d"))
        print("linea base escrita en " + os.path.relpath(ruta, raiz))
        for clave, valores in sorted(foto.items()):
            print(f"  {clave}: "
                  f"{valores if isinstance(valores, int) else len(valores)}")
        return 0

    ruta = os.path.join(raiz, BASE_REL)
    if not os.path.isfile(ruta):
        print(f"linea_base_core: no hay linea base en {BASE_REL}. "
              "Crea una con --actualizar y commiteala.")
        return 2
    with open(ruta, encoding="utf-8") as f:
        base = json.load(f)

    viejos = frescura_del_alcance(raiz)
    for v in viejos:
        print("  AVISO: " + v)
    nuevos, mejoras = compara(foto, base)
    for p in nuevos:
        print("  DEUDA NUEVA: " + p)
    for m in mejoras:
        print("  baja: " + m)

    fallos = problemas + nuevos + ((mejoras + viejos) if a.estricto else [])
    if fallos:
        print(f"linea_base_core: FALLO — {len(problemas)} instantaneos, "
              f"{len(nuevos)} de deuda nueva"
              + (f", {len(mejoras)} sin actualizar la linea base"
                 if a.estricto and mejoras else ""))
        return 1
    if mejoras:
        print(f"linea_base_core: OK — y ademas la deuda BAJA en "
              f"{len(mejoras)}. Actualiza la linea base: "
              "python3 tools/linea_base_core.py --actualizar")
        return 0
    print("linea_base_core: OK — configura, y la deuda medida no ha subido "
          f"desde la linea base del {base.get('fecha', '?')} "
          + " · ".join(f"{k}:{v if isinstance(v, int) else len(v)}"
                       for k, v in sorted(foto.items())))
    return 0


if __name__ == "__main__":
    sys.exit(main())
