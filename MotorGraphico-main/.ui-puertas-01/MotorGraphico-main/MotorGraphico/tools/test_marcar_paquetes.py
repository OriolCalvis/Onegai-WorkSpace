#!/usr/bin/env python3
"""Pruebas de tools/marcar_paquetes.py, sobre ficheros sembrados.

No mira el arbol de verdad a proposito: una herramienta que escribe en el
arbol no se prueba escribiendo en el arbol (mismo criterio que
test_buscar_sobrante.py y test_exportar.py). Cada caso siembra lo que
necesita en un temporal.

Lo que fija, por orden de lo que costo aprenderlo:
  - un bloque copiado a mano con el cierre mal escrito («>>>») se quita,
    y queda UNO (el caso de include/Editor/ProjectDatabaseState.h);
  - un cierre roto nunca se lleva codigo por delante;
  - un fichero con trabajo sin commitear no se toca, aunque lleve horas
    quieto;
  - el bloque no deja un fichero de src/, include/ o editor/ por encima
    de su techo;
  - `git status` se pregunta sin tomar .git/index.lock.
"""
import importlib.util
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import time


def _cargar():
    ruta = pathlib.Path(__file__).with_name("marcar_paquetes.py")
    spec = importlib.util.spec_from_file_location("marcar_paquetes", ruta)
    modulo = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modulo)
    return modulo


def paquete(pid, titulo="Algo", fase="E", jornadas=3):
    return {"id": pid, "titulo": titulo, "fase": fase, "jornadas": jornadas,
            "criterio": "", "prueba": ""}


def escribe(ruta, texto, antiguedad_min=120):
    """Escribe y envejece el fichero: la guarda de los minutos no debe
    confundir estos casos con un agente trabajando."""
    ruta.parent.mkdir(parents=True, exist_ok=True)
    with open(ruta, "w", encoding="utf-8", newline="") as fh:
        fh.write(texto)
    viejo = time.time() - antiguedad_min * 60
    os.utime(ruta, (viejo, viejo))


def lee(ruta):
    with open(ruta, encoding="utf-8", newline="") as fh:
        return fh.read()


CODIGO = "#pragma once\n\nnamespace X {\nint f();\n}  // namespace X\n"


def ejecutar():
    mp = _cargar()
    casos, fallos = 0, []

    def comprueba(nombre, condicion, detalle=""):
        nonlocal casos
        casos += 1
        if not condicion:
            fallos.append(nombre + ((" — " + detalle) if detalle else ""))

    bloque = mp.banner("include/X.h", [paquete("E-16")])

    # --- 1. sin bloque: se pone arriba y el codigo queda intacto.
    nuevo, estado = mp.reescribe(CODIGO, bloque)
    comprueba("sin bloque: estado nuevo", estado == "nuevo", estado)
    comprueba("sin bloque: el bloque va arriba", nuevo.startswith(mp.INICIO))
    comprueba("sin bloque: el codigo queda intacto detras",
              nuevo.endswith("\n" + CODIGO), repr(nuevo[-60:]))

    # --- 2. con el bloque al dia: nada que hacer.
    comprueba("al dia: estado igual", mp.reescribe(nuevo, bloque) == (nuevo, "igual"))

    # --- 3. el caso real: el generado y una copia a mano con el cierre mal escrito.
    copia_a_mano = (mp.INICIO + "\n"
                    "// Este fichero pertenece a 1 paquete del corte B:\n"
                    "//   E-16  (fase E, 8 j) - Edicio RPG minima del projecte\n"
                    "//\n"
                    "// Si tocas este fichero estas trabajando dentro de ese paquete: su\n"
                    "// <<< fin del bloque generado >>>\n")
    repetido = bloque + "\n" + copia_a_mano + "\n" + CODIGO
    limpio, estado = mp.reescribe(repetido, bloque)
    comprueba("repetido: estado repetido", estado == "repetido", estado)
    comprueba("repetido: queda un solo bloque", limpio.count(mp.INICIO) == 1,
              str(limpio.count(mp.INICIO)))
    comprueba("repetido: sin rastro del cierre mal escrito",
              "fin del bloque generado >>>" not in limpio)
    comprueba("repetido: el codigo queda intacto", limpio.endswith("\n" + CODIGO))

    # --- 4. idempotente: la segunda pasada no cambia nada.
    comprueba("idempotente", mp.reescribe(limpio, bloque) == (limpio, "igual"))

    # --- 5. una apertura sin cierre no se toca.
    abierto = mp.INICIO + "\n// sin cierre\n" + CODIGO
    comprueba("apertura sin cierre: roto y sin tocar",
              mp.reescribe(abierto, bloque) == (abierto, "roto"))

    # --- 6. un cierre roto no se come codigo, aunque haya un cierre bueno mas abajo.
    trampa = (mp.INICIO + "\n// bloque sin cerrar\n" + CODIGO +
              "// " + "<<< fin del bloque generado <<<\n" + "int g();\n")
    salida, estado = mp.reescribe(trampa, bloque)
    comprueba("cierre roto: no se come codigo", estado == "roto" and salida == trampa, estado)

    # --- 7. un bloque que no esta arriba sube arriba, y solo queda uno.
    enmedio = "#pragma once\n" + bloque + "\nint h();\n"
    subido, estado = mp.reescribe(enmedio, bloque)
    comprueba("bloque en medio: se mueve arriba",
              estado == "cambia" and subido.startswith(mp.INICIO) and subido.count(mp.INICIO) == 1,
              estado)
    comprueba("bloque en medio: el resto queda", "#pragma once\n" in subido and "int h();" in subido)

    # --- 8. CRLF: el bloque nuevo usa el mismo fin de linea que el fichero.
    crlf = CODIGO.replace("\n", "\r\n")
    con_crlf, estado = mp.reescribe(crlf, bloque)
    solo_lf = con_crlf.replace("\r\n", "")
    comprueba("CRLF: no mezcla finales de linea", "\n" not in solo_lf and estado == "nuevo")
    comprueba("CRLF: idempotente", mp.reescribe(con_crlf, bloque) == (con_crlf, "igual"))

    # --- 9. limite(): que carpetas mira la ley del repositorio.
    comprueba("limite: include/ usa el techo", mp.limite("include/A.h", 500, {}) == 500)
    comprueba("limite: editor/src/ usa el techo", mp.limite("editor/src/A.cpp", 500, {}) == 500)
    comprueba("limite: un cuello usa el suyo",
              mp.limite("include/B.h", 500, {"include/B.h": 519}) == 519)
    comprueba("limite: tests/ no lo mira la ley", mp.limite("tests/A.cpp", 500, {}) is None)
    comprueba("limite: 'includes/' no es 'include/'", mp.limite("includes/A.h", 500, {}) is None)

    # --- 10. marcar_en sobre un arbol sembrado.
    with tempfile.TemporaryDirectory() as tmp:
        raiz = pathlib.Path(tmp)
        uno = [paquete("E-16")]
        dos = [paquete("E-16"), paquete("E-15")]
        cerca = "".join("// linea %d\n" % i for i in range(495))  # 495 lineas
        escribe(raiz / "include" / "Normal.h", CODIGO)
        escribe(raiz / "include" / "Sucio.h", CODIGO)
        escribe(raiz / "include" / "Grande.h", cerca)
        escribe(raiz / "tests" / "GrandeTests.cpp", cerca)
        escribe(raiz / "include" / "Repetido.h", repetido)
        escribe(raiz / "include" / "Roto.h", abierto)
        cuello_texto = mp.banner("include/Cuello.h", uno) + "\n" + "".join(
            "// c %d\n" % i for i in range(519 - mp.lineas(mp.banner("include/Cuello.h", uno)) - 1))
        escribe(raiz / "include" / "Cuello.h", cuello_texto)
        escribe(raiz / "include" / "Reciente.h", CODIGO, antiguedad_min=0)
        mapa = {
            "include/Normal.h": uno,
            "include/Sucio.h": uno,
            "include/Grande.h": uno,
            "tests/GrandeTests.cpp": uno,
            "include/Repetido.h": uno,
            "include/Roto.h": uno,
            "include/Cuello.h": dos,
            "include/Reciente.h": uno,
            "include/NoExiste.h": uno,
            "planificacion/NoEsCodigo.md": uno,
        }
        antes = {p: lee(raiz / p) for p in mapa if (raiz / p).is_file()}
        comprueba("siembra: el cuello mide 519", mp.lineas(antes["include/Cuello.h"]) == 519,
                  str(mp.lineas(antes["include/Cuello.h"])))

        r = mp.marcar_en(str(raiz), mapa, dry_run=True, minutos_quietos=30,
                         sucias={"include/Sucio.h"}, techo=500,
                         cuellos={"include/Cuello.h": 519})
        comprueba("dry-run: no escribe nada",
                  all(lee(raiz / p) == t for p, t in antes.items()))
        comprueba("marcar_en: los que tocan",
                  sorted(r["escritos"]) == ["include/Normal.h", "include/Repetido.h",
                                            "tests/GrandeTests.cpp"], str(r["escritos"]))
        comprueba("marcar_en: el repetido se dice", r["repetidos"] == ["include/Repetido.h"])
        comprueba("marcar_en: el sucio se salta", r["sucios"] == ["include/Sucio.h"])
        comprueba("marcar_en: el techo salta en include/ y no en tests/",
                  [t[0] for t in r["techo"]] == ["include/Grande.h", "include/Cuello.h"],
                  str(r["techo"]))
        comprueba("marcar_en: el roto se dice", r["rotos"] == ["include/Roto.h"])
        comprueba("marcar_en: el reciente se salta",
                  [x[0] for x in r["recientes"]] == ["include/Reciente.h"])
        comprueba("marcar_en: el ausente se dice", r["ausentes"] == ["include/NoExiste.h"])

        r = mp.marcar_en(str(raiz), mapa, dry_run=False, minutos_quietos=30,
                         sucias={"include/Sucio.h"}, techo=500,
                         cuellos={"include/Cuello.h": 519})
        comprueba("escritura: el sucio sigue como estaba",
                  lee(raiz / "include/Sucio.h") == antes["include/Sucio.h"])
        comprueba("escritura: el roto sigue como estaba",
                  lee(raiz / "include/Roto.h") == antes["include/Roto.h"])
        comprueba("escritura: el grande de include/ sigue como estaba",
                  lee(raiz / "include/Grande.h") == antes["include/Grande.h"])
        comprueba("escritura: el repetido queda con un bloque",
                  lee(raiz / "include/Repetido.h").count(mp.INICIO) == 1)
        for p in ("include/Normal.h", "include/Repetido.h", "tests/GrandeTests.cpp"):
            os.utime(raiz / p, (time.time() - 7200, time.time() - 7200))
        r = mp.marcar_en(str(raiz), mapa, dry_run=False, minutos_quietos=30,
                         sucias={"include/Sucio.h"}, techo=500,
                         cuellos={"include/Cuello.h": 519})
        comprueba("segunda pasada: nada que escribir", r["escritos"] == [], str(r["escritos"]))

    # --- 11. sucios(): git status sin candado, con la carpeta del motor por debajo de la raiz.
    git = shutil.which("git")
    if git:
        with tempfile.TemporaryDirectory() as tmp:
            repo = pathlib.Path(tmp)
            motor = repo / "Motor"

            def g(*args):
                return subprocess.run([git, "-C", str(repo), "-c", "user.name=prueba",
                                       "-c", "user.email=prueba@local", *args],
                                      capture_output=True, text=True, check=True)
            g("init", "-q")
            for nombre in ("limpio.h", "tocado.h", "viejo.h"):
                escribe(motor / "include" / nombre, CODIGO)
            escribe(repo / "Fuera.txt", "x\n")
            g("add", "-A")
            g("commit", "-q", "-m", "siembra")
            escribe(motor / "include" / "tocado.h", CODIGO + "int z();\n")
            escribe(motor / "include" / "nuevo con espacio.h", CODIGO)
            g("mv", "Motor/include/viejo.h", "Motor/include/renombrado.h")
            escribe(repo / "Fuera.txt", "y\n")
            vistos = mp.sucios(str(motor))
            comprueba("sucios: modificado, sin versionar y renombrado, relativos al motor",
                      vistos == {"include/tocado.h", "include/nuevo con espacio.h",
                                 "include/renombrado.h"}, str(vistos))
            comprueba("sucios: no deja el candado del indice",
                      not (repo / ".git" / "index.lock").exists())
        with tempfile.TemporaryDirectory() as tmp:
            comprueba("sucios: fuera de un repo, None", mp.sucios(tmp) is None)

    for f in fallos:
        print("FALLA: " + f)
    print("%d/%d" % (casos - len(fallos), casos))
    return 1 if fallos else 0


if __name__ == "__main__":
    sys.exit(ejecutar())
