#!/usr/bin/env python3
"""Comprueba que los controles ANUNCIADOS existan de verdad en el codigo.

    python3 tools/verificar_controles.py

POR QUE EXISTE. La cabecera de level_editor.cpp llego a documentar un
"F7 = cambiar temporalmente a modo jugador" y un "F8/F9 = nivel anterior/
siguiente" durante un tiempo en el que ninguna de las dos cosas estaba
implementada: controles escritos antes de hacerlos y nunca borrados. Un
control fantasma es peor que uno sin documentar, porque el usuario pulsa,
no pasa nada, y se queda pensando que el editor esta roto.

Ademas hay cuatro AIs y Oriol tocando el mismo fichero, asi que la barra
de estado y la cabecera se desincronizan solas.

QUE COMPRUEBA, en las dos direcciones:

  1. Toda tecla que aparece en la cabecera o en la barra de estado esta
     enlazada de verdad en el bucle de teclado.
  2. Toda tecla enlazada aparece anunciada en algun sitio. Una funcion que
     nadie sabe que existe es trabajo tirado.

NO comprueba que la accion haga lo correcto. Solo que exista.
"""
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(AQUI)
FUENTE = os.path.join(RAIZ, "editor", "src", "level_editor.cpp")
ENUM = os.path.join(RAIZ, "include", "Editor", "EditorState.h")

# Como se escribe cada tecla en el texto que ve el usuario. GLFW la llama
# de una forma y la cabecera de otra ("," frente a GLFW_KEY_COMMA).
NOMBRES = {
    "COMMA": [","], "PERIOD": ["."], "ESCAPE": ["ESC"], "ENTER": ["ENTER"],
    "SPACE": ["SPACE", "ESPACIO"], "BACKSPACE": ["BACKSPACE"], "HOME": ["HOME"], "UP": ["W/S", "W", "ARRIBA"],
    "DOWN": ["W/S", "S", "ABAJO"], "MINUS": ["-", "+/-"], "EQUAL": ["+", "+/-"],
    "KP_ADD": ["+", "+/-"], "KP_SUBTRACT": ["-", "+/-"],
    "LEFT_CONTROL": ["CTRL"], "RIGHT_CONTROL": ["CTRL"],
    "LEFT_SHIFT": ["SHIFT"], "RIGHT_SHIFT": ["SHIFT"],
    "LEFT_SUPER": ["CTRL", "CMD"], "RIGHT_SUPER": ["CTRL", "CMD"],
    "LEFT_BRACKET": ["[", "[/]"], "RIGHT_BRACKET": ["]", "[/]"],
    # El pan de camara se anuncia como bloque ("WASD"), no tecla a tecla.
    "W": ["W", "WASD", "W/S"], "S": ["S", "WASD", "W/S"], "D": ["D", "WASD"],
}

# Teclas que se enlazan por barrido de rango, no una a una (el campo de
# texto del id acepta A-Z y 0-9 enteros). Anunciar 26 letras no ayuda.
RANGOS = {"A", "Z", "0", "9"}


# Palabras que delatan una cadena de ayuda al usuario. Se filtra por
# ellas en vez de mirar TODAS las cadenas del fichero porque una letra
# suelta ("G") hace match con casi cualquier texto y el comprobador daria
# por anunciado lo que no lo esta.
PISTAS = ("HERRAM", "PALETA", "GUARDAR", "mover", "abrir", "nuevo",
          "compilar", "borra", "crea", "cancela", "ESCENARIO", "PROYECTOS")


def texto_anunciado(src):
    """La cabecera de comentarios + las cadenas de ayuda: todo lo que el
    usuario llega a leer de verdad."""
    cabecera = []
    for linea in src.split("\n"):
        if not linea.startswith("//"):
            break
        cabecera.append(linea)
    # Literales de C de verdad: la version anterior usaba [^"\\]{12,}, que
    # cruza saltos de linea, asi que emparejaba la comilla de cierre de un
    # literal con la de apertura del siguiente y devolvia el CODIGO que hay
    # en medio. El comprobador leia el fuente creyendo leer la ayuda.
    literales = re.findall(r'"((?:[^"\\\n]|\\.)*)"', src)
    barras = [c for c in literales if len(c) >= 12 and any(p in c for p in PISTAS)]
    return "\n".join(cabecera) + "\n" + "\n".join(barras)


def aparece(etiqueta, texto):
    """Una etiqueta de una sola letra tiene que aparecer SUELTA. Sin esto,
    'G' se da por anunciada por cualquier cadena que lleve una G dentro y
    el comprobador no comprueba nada."""
    e = etiqueta.upper()
    if e.isalnum():
        return re.search(r"(?<![A-Z0-9])" + re.escape(e) + r"(?![A-Z0-9])", texto) is not None
    return e in texto


def sin_comentarios(src):
    """Quita comentarios sin confundir // dentro de un literal con codigo."""
    return re.sub(r'"(?:[^"\\]|\\.)*"|//[^\n]*|/\*.*?\*/',
                  lambda m: " " if m.group().startswith(("//", "/*")) else m.group(),
                  src, flags=re.S)


def comprobar(src, enum_src):
    codigo = sin_comentarios(src)
    # Los nombres en comentarios y mensajes no son enlaces de teclado.
    sin_literales = re.sub(r'"(?:[^"\\]|\\.)*"', '""', codigo)
    enlazadas = set(re.findall(r"GLFW_KEY_([A-Z0-9_]+)", sin_literales))
    anunciado = texto_anunciado(src).upper()

    fantasmas, silenciosas = [], []

    for tecla in sorted(enlazadas):
        if tecla in RANGOS or tecla.startswith("LAST"):
            continue
        etiquetas = NOMBRES.get(tecla, [tecla])
        if not any(aparece(e, anunciado) for e in etiquetas):
            silenciosas.append(tecla)

    # Teclas de funcion y letras sueltas nombradas en el texto: que existan.
    for f_tecla in sorted(set(re.findall(r"\bF([1-9]|1[0-2])\b", anunciado))):
        if "F" + f_tecla not in enlazadas:
            fantasmas.append("F" + f_tecla)

    # Tambien comprueba teclas con nombre (TAB era un fantasma que no
    # detectaba la comprobacion limitada a F1-F12).
    for nombre, tecla in {"TAB": "TAB", "ENTER": "ENTER", "ESC": "ESCAPE",
                          "BACKSPACE": "BACKSPACE", "HOME": "HOME",
                          "SPACE": "SPACE", "ESPACIO": "SPACE"}.items():
        if aparece(nombre, anunciado) and tecla not in enlazadas:
            fantasmas.append(tecla)
    # Las declaraciones de controles de la cabecera tienen etiqueta = accion.
    # No tomamos letras sueltas de la prosa, que no necesariamente son teclas.
    for etiqueta in re.findall(r"^//\s+([A-Z0-9+/ .,-]+?)\s+=", src, re.M | re.I):
        for tecla in re.findall(r"(?<![A-Z0-9])[A-Z](?![A-Z0-9])", etiqueta.upper()):
            if tecla not in enlazadas:
                fantasmas.append(tecla)
    fantasmas = sorted(set(fantasmas))

    # Herramientas: compara IDENTIDADES y numeros, no solo cantidades.
    # Siete teclas que seleccionan todas PaintTile no son siete herramientas.
    enum = re.search(r"enum\s+class\s+EditorTool\s*\{([^}]+)\}",
                     sin_comentarios(enum_src), re.S)
    nombres = [e.strip() for e in enum.group(1).split(",") if e.strip()] if enum else []
    enum_valido = bool(nombres) and all(re.fullmatch(r"[A-Za-z_]\w*", n) for n in nombres)
    esperadas = [(str(i), n) for i, n in enumerate(nombres, 1)]
    menu = re.findall(r'"(\d+) "\s*\+\s*std::string\(toolName\(EditorTool::(\w+)\)\)', codigo)
    teclas = re.findall(r"GLFW_KEY_(\d)\s*\)\s*\)\s*(?:\{\s*)?"
                        r"editor\.setTool\(EditorTool::(\w+)\)", sin_literales)
    rangos = re.findall(r"1-(\d+) HERRAM", texto_anunciado(src))
    n_enum, n_menu, n_teclas = len(nombres), len(menu), len(teclas)
    n_barra = int(rangos[0]) if rangos else 0
    herramientas = []
    if not enum_valido:
        herramientas.append("no se pudo leer un enum EditorTool valido")
    if menu != esperadas:
        herramientas.append(f"menu {menu}, esperado {esperadas}")
    if sorted(teclas) != sorted(esperadas):
        herramientas.append(f"teclas {teclas}, esperado {esperadas}")
    if not rangos or any(int(n) != n_enum for n in rangos):
        herramientas.append(f"barra {rangos}, esperado 1-{n_enum}")
    cabecera = re.search(r"^//\s+1\.\.(\d+)\s+=\s+herramienta", src, re.M)
    if not cabecera or int(cabecera.group(1)) != n_enum:
        herramientas.append(f"cabecera: esperado 1..{n_enum}")

    print("Controles de editor/src/level_editor.cpp\n")
    print(f"  teclas enlazadas en el codigo: {len(enlazadas)}")
    print(f"  herramientas: {n_enum} en el enum, {n_teclas} con tecla, "
          f"{n_menu} en el menu, 1-{n_barra} en la barra")

    if herramientas:
        print("\n  LAS HERRAMIENTAS NO CUADRAN: " + ", ".join(herramientas))

    if fantasmas:
        print("\n  ANUNCIADAS Y NO ENLAZADAS (el usuario pulsa y no pasa nada):")
        for t in fantasmas:
            print(f"      {t}")
    if silenciosas:
        print("\n  ENLAZADAS Y NO ANUNCIADAS (funciones que nadie sabe que existen):")
        for t in silenciosas:
            print(f"      {t}")

    if not fantasmas and not silenciosas and not herramientas:
        print("\n  todo lo anunciado existe y todo lo que existe esta anunciado")
        return 0
    print(f"\n  {len(fantasmas)} fantasma(s), {len(silenciosas)} sin anunciar"
          + (", herramientas descuadradas" if herramientas else ""))
    return 1


def main():
    try:
        with open(FUENTE, encoding="utf-8") as f:
            src = f.read()
        with open(ENUM, encoding="utf-8") as f:
            enum_src = f.read()
    except OSError as exc:
        print(f"No se pueden verificar los controles: {exc}", file=sys.stderr)
        return 1
    return comprobar(src, enum_src)


if __name__ == "__main__":
    sys.exit(main())
