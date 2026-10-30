#!/usr/bin/env python3
"""Pruebas de tools/buscar_sobrante.py, sobre un repositorio sembrado.

No mira el arbol de verdad a proposito: un detector probado contra el
arbol que analiza no es falsable (mismo criterio que test_exportar.py).
"""
import importlib.util
import pathlib
import sys
import tempfile


def _cargar():
    ruta = pathlib.Path(__file__).with_name("buscar_sobrante.py")
    spec = importlib.util.spec_from_file_location("buscar_sobrante", ruta)
    modulo = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modulo)
    return modulo


def escribe(ruta: pathlib.Path, texto: str):
    ruta.parent.mkdir(parents=True, exist_ok=True)
    ruta.write_text(texto, encoding="utf-8")


CUERPO = """void volcar(const std::string& ruta, int ancho) {
    std::vector<unsigned char> px(ancho);
    std::ofstream out(ruta);
    out << "P6";
    out.write((char*)px.data(), ancho);
}
"""


def ejecutar():
    bs = _cargar()
    casos, fallos = 0, []

    def comprueba(nombre, condicion, detalle=""):
        nonlocal casos
        casos += 1
        if not condicion:
            fallos.append(nombre + ((" — " + detalle) if detalle else ""))

    with tempfile.TemporaryDirectory() as tmp:
        raiz = pathlib.Path(tmp)

        # --- 1. una cabecera a la que se refiere OTRO no es un muerto.
        escribe(raiz / "include" / "Viva.h", "#pragma once\nvoid viva();\n")
        escribe(raiz / "src" / "Viva.cpp", '#include "Viva.h"\nvoid viva() {}\n')
        escribe(raiz / "src" / "Usuario.cpp", '#include "Viva.h"\nint main(){viva();}\n')

        # --- 2. una cabecera que solo se citan ella, su .cpp y su prueba.
        escribe(raiz / "include" / "Sola.h", "#pragma once\nvoid sola();\n")
        escribe(raiz / "src" / "Sola.cpp", '#include "Sola.h"\nvoid sola() {}\n')
        escribe(raiz / "tests" / "SolaTests.cpp", '#include "Sola.h"\nint main(){}\n')

        m = {f["cabecera"] for f in bs.muertos(str(raiz))}
        comprueba("una cabecera que alguien usa no es un muerto",
                  "include/Viva.h" not in m, str(m))
        comprueba("una cabecera con solo su .cpp y su prueba si lo es",
                  "include/Sola.h" in m, str(m))
        fila = next(f for f in bs.muertos(str(raiz)) if f["cabecera"] == "include/Sola.h")
        comprueba("y se lleva con ella su .cpp y su prueba",
                  set(fila["se_va_con_ella"]) == {"src/Sola.cpp", "tests/SolaTests.cpp"},
                  str(fila))

        # --- 3. el mismo cuerpo en dos ficheros.
        escribe(raiz / "src" / "UnoA.cpp", CUERPO)
        escribe(raiz / "src" / "UnoB.cpp", CUERPO)
        # el mismo cuerpo, pero solo con comentarios distintos: sigue siendo el mismo
        escribe(raiz / "src" / "UnoC.cpp", "// otro comentario\n" + CUERPO.replace(
            'out << "P6";', 'out << "P6";  // cabecera del formato'))
        # y uno que de verdad es distinto
        escribe(raiz / "src" / "Otro.cpp", CUERPO.replace('"P6"', '"P3"'))

        grupos = bs.repetido(str(raiz))
        volcar = [g for g in grupos if g["nombre"] == "volcar"]
        comprueba("encuentra el cuerpo repetido", len(volcar) >= 1, str(grupos))
        if volcar:
            mayor = max(volcar, key=lambda g: g["copias"])
            ficheros = {s["fichero"] for s in mayor["sitios"]}
            comprueba("agrupa las tres copias iguales", len(ficheros) == 3, str(ficheros))
            comprueba("los comentarios no cuentan",
                      "src/UnoC.cpp" in ficheros, str(ficheros))
            comprueba("un cuerpo distinto no entra en el grupo",
                      "src/Otro.cpp" not in ficheros, str(ficheros))
            comprueba("cuenta lo que sobra, no lo que hay",
                      mayor["lineas_de_sobra"] == mayor["lineas"] * 2,
                      str(mayor["lineas_de_sobra"]))

        # --- 4. las pruebas no cuentan como duplicacion.
        escribe(raiz / "tests" / "CopiaTests.cpp", CUERPO)
        ficheros = {s["fichero"] for g in bs.repetido(str(raiz)) for s in g["sitios"]}
        comprueba("lo que hay en tests/ no se cuenta como repetido",
                  not any(f.startswith("tests") for f in ficheros), str(ficheros))

        # --- 5. un cuerpo de dos lineas es coincidencia, no duplicacion.
        corto = "int dos() {\n    return 2;\n}\n"
        escribe(raiz / "src" / "CortoA.cpp", corto)
        escribe(raiz / "src" / "CortoB.cpp", corto)
        comprueba("un cuerpo demasiado corto no se reporta",
                  not any(g["nombre"] == "dos" for g in bs.repetido(str(raiz))))

    if fallos:
        print("test_buscar_sobrante: FALLO - %d de %d" % (len(fallos), casos))
        for f in fallos:
            print("   " + f)
        return 1
    print("test_buscar_sobrante: OK - %d casos, todos verdes" % casos)
    return 0


if __name__ == "__main__":
    sys.exit(ejecutar())
