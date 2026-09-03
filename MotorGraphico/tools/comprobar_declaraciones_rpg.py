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
INCLUDE = RAIZ / "include" / "RPG"
SRC = RAIZ / "src"

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
    patron = re.compile(rf"^[\w:<>&* \t]+{re.escape(nombre)}\s*\(", re.MULTILINE)
    return any(patron.search(texto) for texto in fuentes)


def main():
    cabeceras = sorted(INCLUDE.rglob("*.h"))
    fuentes = [p.read_text(encoding="utf-8", errors="replace") for p in sorted(SRC.rglob("*.cpp"))]
    faltan = []
    total = 0
    for cabecera in cabeceras:
        for clase, nombre in declaraciones_de(cabecera):
            total += 1
            if not definida(clase, nombre, fuentes):
                faltan.append(f"{cabecera.relative_to(RAIZ)}: {clase + '::' if clase else ''}{nombre}")
    if faltan:
        print(f"C-22: {len(faltan)} declaración(es) de include/RPG/ sin definición:")
        for linea in faltan:
            print(f"  - {linea}")
        return 1
    print(f"C-22: OK — {total} declaraciones comprobadas en {len(cabeceras)} cabeceras de include/RPG/.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
