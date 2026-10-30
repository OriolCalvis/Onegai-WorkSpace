#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""C-22 — Comprobador de declaraciones sin definición en include/RPG/.

Motivo: siete métodos de TierRules vivieron declarados y nunca definidos
(el enlazador no se queja porque nadie los llamaba). Este script es la
red estática que impide que vuelva a pasar: cada método declarado en las
cabeceras de include/RPG/ debe tener cuerpo (inline en la cabecera o
`Clase::metodo` en algún .cpp).

Es un analizador HEURÍSTICO hecho a la medida de estas cabeceras
(sintaxis uniforme, sin macros), no un parser de C++: escanea caracteres
(sin regex sobre textos largos, para evitar backtracking), ignora
declaraciones con cuerpo inline, `= default`, `= delete`, virtuales
puras (`= 0`), plantillas e inicializadores de miembro, y toma una
definición por buena si aparece `Clase::nombre(` en algún src/**/*.cpp
(métodos) o `nombre(` a inicio de línea con tipo de retorno (funciones
libres).

Uso: python3 tools/comprobar_declaraciones_rpg.py [raiz_del_motor]
Sin argumento usa el padre del directorio del script (paths relativos a
__file__, bitácora 2.15). Sale 0 si todo está definido, 1 si falta algo.
"""
import re
import sys
from pathlib import Path

RAIZ = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent

# C-15 — Las carpetas que se comprueban.
#
# Nacio mirando solo include/RPG/ porque el caso que lo provoco fue
# TierRules. El 11/09 aparecio el mismo fallo FUERA de ese arbol:
# ProjectIndex::addLevelToProject vivio declarada sin cuerpo, y este
# comprobador paso en verde porque no miraba include/Project/. Una red
# que solo cubre el sitio donde ya te caiste una vez no es una red.
#
# Se anaden las carpetas cuyas cabeceras siguen la misma sintaxis
# uniforme para la que el analizador esta hecho. Render/, Core/ y
# Engine/ quedan fuera A PROPOSITO de momento: llevan plantillas,
# macros de GL y cabeceras de terceros que este analizador heuristico
# no sabe leer, y un comprobador que da falsos positivos se acaba
# ignorando, que es peor que no tenerlo. Ampliar la lista es barato
# cuando alguien verifique que esas carpetas salen limpias.
CARPETAS = ["RPG", "Project", "Map", "Editor", "Level", "World"]

INCLUDES = [RAIZ / "include" / nombre for nombre in CARPETAS]
SRC = RAIZ / "src"

# TRINQUETE. Al ampliar las carpetas aparecieron dos declaraciones sin
# cuerpo que ya estaban ahi. NO se arreglan desde aqui: decidir que hace
# ObjectInspector::confirmar() o que JSON emite ObjectCatalog::exportJson()
# es diseno del paquete que las posee, no un arreglo mecanico.
#
# Tampoco se esconden. Se listan aqui con su dueno: se avisan en cada
# ejecucion y NO tumban la bateria, pero cualquier declaracion huerfana
# NUEVA si la tumba. Asi la deuda conocida no crece y no se bloquea a
# cinco agentes por dos casos que vienen de antes.
#
# El dia que se resuelvan, se borra su linea de aqui. Si alguien borra
# la linea sin resolverlas, el comprobador vuelve a ponerse en rojo solo.
# El dia que se resuelvan, se borra su linea de aqui. Si alguien borra
# la linea sin resolverlas, el comprobador vuelve a ponerse en rojo solo.
CONOCIDAS = {
}

COMENTARIO_LINEA = re.compile(r"//[^\n]*")
COMENTARIO_BLOQUE = re.compile(r"/\*.*?\*/", re.DOTALL)
APERTURA_CLASE = re.compile(r"\b(?:class|struct)\s+(\w+)")
PLANTILLA = re.compile(r"\btemplate\s*<")
PALABRAS_CLAVE = {"if", "for", "while", "switch", "return", "sizeof", "case", "new", "delete"}


def limpiar(texto):
    return COMENTARIO_LINEA.sub("", COMENTARIO_BLOQUE.sub("", texto))


def nombre_declarado(declaracion):
    """Nombre del método si la sentencia es una declaración de función."""
    d = declaracion.strip()
    if d.startswith(("friend", "using", "typedef", "enum", "extern", "namespace", "#")):
        return None
    abierto = d.find("(")
    if abierto < 0:
        return None
    if "=" in d[:abierto]:
        return None  # dato miembro / constante con inicializador
    # cierre del paréntesis de argumentos
    prof, cerrado = 0, -1
    for i, ch in enumerate(d[abierto:], abierto):
        if ch == "(":
            prof += 1
        elif ch == ")":
            prof -= 1
            if prof == 0:
                cerrado = i
                break
    if cerrado < 0 or "{" in d:
        return None
    cola = d[cerrado + 1:].strip()
    for palabra in ("const", "noexcept", "override", "final"):
        if cola.startswith(palabra):
            cola = cola[len(palabra):].strip()
    if cola.startswith("="):
        return None  # = 0 (virtual pura), = default, = delete
    prefijo = d[:abierto].rstrip()
    i = len(prefijo)
    while i > 0 and (prefijo[i - 1].isalnum() or prefijo[i - 1] in "_~"):
        i -= 1
    nombre = prefijo[i:]
    if not nombre or nombre.lstrip("~") in PALABRAS_CLAVE:
        return None
    return nombre


def declaraciones_de(cabecera):
    """Pares (clase o None, nombre) declarados sin cuerpo en la cabecera."""
    texto = limpiar(cabecera.read_text(encoding="utf-8"))
    if PLANTILLA.search(texto):
        # Las plantillas se definen inline en la cabecera por necesidad;
        # este comprobador no las audita.
        return []
    resultado = []
    clase_actual = None
    profundidad = 0
    buffer = ""
    for c in texto:
        if c == "{":
            if clase_actual is None and profundidad == 1:
                m = APERTURA_CLASE.search(buffer)
                if m and "enum" not in buffer:
                    clase_actual = m.group(1)
            profundidad += 1
            buffer = ""
        elif c == "}":
            profundidad -= 1
            if clase_actual is not None and profundidad == 1:
                clase_actual = None
            buffer = ""
        elif c == ";":
            declaracion = buffer
            buffer = ""
            nivel_ok = ((clase_actual is not None and profundidad == 2)
                        or (clase_actual is None and profundidad == 1))
            if not nivel_ok:
                continue
            nombre = nombre_declarado(declaracion)
            if nombre:
                resultado.append((clase_actual, nombre))
        else:
            buffer += c
    return resultado


def definida(clase, nombre, fuentes):
    if clase is not None:
        patron = re.compile(rf"{re.escape(clase)}::{re.escape(nombre)}\s*\(")
        return any(patron.search(texto) for texto in fuentes)
    # Los tipos de retorno pueden tener argumentos de plantilla separados
    # por comas y repartirse entre lineas (p. ej. std::array<T, 14>).
    patron = re.compile(rf"^[\w:<>&*,\s]+{re.escape(nombre)}\s*\(", re.MULTILINE)
    return any(patron.search(texto) for texto in fuentes)


def main():
    cabeceras = []
    for carpeta in INCLUDES:
        if carpeta.is_dir():
            cabeceras.extend(sorted(carpeta.rglob("*.h")))
    # editor/src tambien lleva cuerpos (la UI del editor de niveles). Sin
    # el, cualquier metodo definido alli se reportaria como huerfano.
    fuentes = [p.read_text(encoding="utf-8", errors="replace")
               for p in sorted(SRC.rglob("*.cpp")) + sorted((RAIZ / "editor" / "src").rglob("*.cpp"))]
    faltan = []
    total = 0
    for cabecera in cabeceras:
        for clase, nombre in declaraciones_de(cabecera):
            total += 1
            if not definida(clase, nombre, fuentes):
                faltan.append(f"{cabecera.relative_to(RAIZ)}: {clase + '::' if clase else ''}{nombre}")
    conocidas = [f for f in faltan if f in CONOCIDAS]
    nuevas = [f for f in faltan if f not in CONOCIDAS]
    for linea in conocidas:
        print(f"C-22 AVISO (deuda conocida): {linea}\n           {CONOCIDAS[linea]}")
    resueltas = [f for f in CONOCIDAS if f not in faltan]
    for linea in resueltas:
        print(f"C-22: '{linea}' ya tiene cuerpo — quita su linea de CONOCIDAS "
              f"en {Path(__file__).name}.")
    if nuevas:
        print(f"C-22: {len(nuevas)} declaración(es) NUEVA(s) sin definición "
              f"(carpetas: {', '.join(CARPETAS)}):")
        for linea in nuevas:
            print(f"  - {linea}")
        print("Una declaración sin cuerpo no da error hasta que alguien la llama, "
              "y entonces el fallo sale en el enlazado de otro paquete.")
        return 1
    print(f"C-22: OK — {total} declaraciones comprobadas en {len(cabeceras)} cabeceras "
          f"de include/{{{','.join(CARPETAS)}}}/"
          f"{f'; {len(conocidas)} de deuda conocida' if conocidas else ''}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
