#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Prueba de `linea_base_core.py` sobre arboles SEMBRADOS.

POR QUE ASI. La guardia se prueba contra casos plantados y no contra el
repositorio real por la misma razon que el analizador: contra el arbol de
verdad no es falsable. El dia que alguien parta `Application.cpp` la
guardia tiene que seguir siendo correcta, y el test no puede ponerse rojo
por eso.

Los casos cubren los fallos que la guardia existe para coger:

  1. un `include(cmake/X.cmake)` sin su fichero — HEAD no configura;
  2. el mismo con el fichero presente pero SIN VERSIONAR — configura en
     la maquina de quien lo escribio y en ningun clon (lo que paso de
     verdad el 19/09, y nadie se entero en seis commits);
  3. un fichero del core que engorda por encima del techo — el trinquete.
  4. datos locales y un controlador en el mismo `.cpp` — estructura mixta.

Se ejecuta sola:  python3 tools/test_linea_base_core.py
"""
from __future__ import annotations

import importlib.util
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

AQUI = Path(__file__).resolve()


def _cargar():
    spec = importlib.util.spec_from_file_location(
        "linea_base_core", AQUI.parent / "linea_base_core.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def escribe(ruta: Path, texto: str):
    ruta.parent.mkdir(parents=True, exist_ok=True)
    ruta.write_text(texto, encoding="utf-8")


def siembra_repo(raiz: Path, incluye=("cmake/Uno.cmake",), crea=("cmake/Uno.cmake",),
                 versiona=("CMakeLists.txt", "cmake/Uno.cmake"), con_git=True):
    lineas = ["project(demo)"] + [f"include({c})" for c in incluye]
    escribe(raiz / "CMakeLists.txt", "\n".join(lineas) + "\n")
    for c in crea:
        escribe(raiz / c, "# fragmento\n")
    if con_git:
        subprocess.run(["git", "init", "-q"], cwd=raiz, check=True,
                       capture_output=True)
        for v in versiona:
            if (raiz / v).exists():
                subprocess.run(["git", "add", v], cwd=raiz, check=True,
                               capture_output=True)
    return raiz


def fichero_de(raiz: Path, rel: str, lineas: int):
    escribe(raiz / rel, "\n".join(f"// linea {i}" for i in range(lineas)) + "\n")


def ejecutar_casos():
    lb = _cargar()
    casos, fallos = 0, []
    tmp = Path(tempfile.mkdtemp(prefix="test_linea_base_"))

    def comprueba(nombre, condicion, detalle=""):
        nonlocal casos
        casos += 1
        if not condicion:
            fallos.append(f"{nombre}: {detalle}" if detalle else nombre)

    try:
        # --- 1. include sin fichero: el repositorio no configura.
        r1 = siembra_repo(tmp / "sin_fichero", crea=(), versiona=("CMakeLists.txt",))
        p1 = lb.comprueba_configuracion(str(r1))
        comprueba("include sin fichero falla", len(p1) == 1, str(p1))
        comprueba("y dice que no existe",
                  any("NO EXISTE" in p for p in p1), str(p1))

        # --- 2. el fichero esta, pero sin versionar. ESTE es el de verdad.
        r2 = siembra_repo(tmp / "sin_versionar", versiona=("CMakeLists.txt",))
        p2 = lb.comprueba_configuracion(str(r2))
        comprueba("include con fichero sin versionar falla", len(p2) == 1, str(p2))
        comprueba("y dice que no esta versionado",
                  any("NO ESTA VERSIONADO" in p for p in p2), str(p2))

        # --- 3. todo en su sitio: pasa.
        r3 = siembra_repo(tmp / "correcto")
        comprueba("include correcto pasa", lb.comprueba_configuracion(str(r3)) == [])

        # --- 4. sin git no se inventa nada: solo mira que el fichero este.
        r4 = siembra_repo(tmp / "sin_git", con_git=False)
        comprueba("sin git no inventa problemas",
                  lb.comprueba_configuracion(str(r4)) == [],
                  str(lb.comprueba_configuracion(str(r4))))

        # --- 3 bis. el gancho mira el INDICE, no el disco: el trabajo en
        # vuelo de otra sesion no puede bloquear un commit ajeno.
        r3b = siembra_repo(tmp / "indice")
        (r3b / "cmake" / "EnVuelo.cmake").write_text("# de otra sesion\n", encoding="utf-8")
        (r3b / "CMakeLists.txt").write_text(
            "project(demo)\ninclude(cmake/Uno.cmake)\ninclude(cmake/EnVuelo.cmake)\n",
            encoding="utf-8")
        comprueba("mirando el disco, el include en vuelo se ve",
                  len(lb.comprueba_configuracion(str(r3b))) == 1,
                  str(lb.comprueba_configuracion(str(r3b))))
        comprueba("mirando el indice, no bloquea a quien no lo commitea",
                  lb.comprueba_configuracion(str(r3b), desde_indice=True) == [],
                  str(lb.comprueba_configuracion(str(r3b), desde_indice=True)))

        # --- 3 ter. FUENTES citadas por CMake y no versionadas (EST-33).
        # El fallo real: siete baterias de Render que CMake compilaba desde
        # C-24 y que vivian solo en el disco de quien las escribio. En un
        # clon limpio, CMake no las encontraba.
        rf = siembra_repo(tmp / "fuentes")
        escribe(rf / "tests" / "Render" / "CameraTests.cpp", "int main(){}\n")
        escribe(rf / "src" / "Existe.cpp", "void a(){}\n")
        subprocess.run(["git", "add", "src/Existe.cpp"], cwd=rf, check=True,
                       capture_output=True)
        (rf / "CMakeLists.txt").write_text(
            "project(demo)\n"
            "include(cmake/Uno.cmake)\n"
            "add_executable(t tests/Render/CameraTests.cpp src/Existe.cpp)\n"
            "target_sources(t PRIVATE ${GLAD_DIR}/src/glad.c)\n"
            "# comentario que nombra src/Fantasma.cpp y no lo compila\n",
            encoding="utf-8")
        pf = lb.comprueba_fuentes(str(rf))
        comprueba("una fuente sin versionar se canta",
                  len(pf) == 1 and "CameraTests.cpp" in pf[0], str(pf))
        comprueba("una fuente versionada no molesta",
                  not any("Existe.cpp" in x for x in pf), str(pf))
        comprueba("una ruta hecha con una variable no se comprueba",
                  not any("glad.c" in x for x in pf), str(pf))
        comprueba("un comentario que nombra un fichero no lo cita",
                  not any("Fantasma" in x for x in pf), str(pf))

        subprocess.run(["git", "add", "tests/Render/CameraTests.cpp"], cwd=rf,
                       check=True, capture_output=True)
        comprueba("versionarla la arregla", lb.comprueba_fuentes(str(rf)) == [],
                  str(lb.comprueba_fuentes(str(rf))))

        # El modo indice mira lo que se COMMITEA: una fuente que otra sesion
        # acaba de dejar en el disco no bloquea un commit ajeno.
        rf2 = siembra_repo(tmp / "fuentes_indice")
        subprocess.run(["git", "commit", "-q", "-m", "base", "--no-verify"],
                       cwd=rf2, check=True, capture_output=True,
                       env=dict(os.environ, GIT_AUTHOR_NAME="t", GIT_AUTHOR_EMAIL="t@t",
                                GIT_COMMITTER_NAME="t", GIT_COMMITTER_EMAIL="t@t"))
        escribe(rf2 / "tests" / "EnVuelo.cpp", "int main(){}\n")
        (rf2 / "CMakeLists.txt").write_text(
            "project(demo)\ninclude(cmake/Uno.cmake)\n"
            "add_executable(t tests/EnVuelo.cpp)\n", encoding="utf-8")
        comprueba("en el disco, la fuente en vuelo se ve",
                  len(lb.comprueba_fuentes(str(rf2))) == 1,
                  str(lb.comprueba_fuentes(str(rf2))))
        comprueba("en el indice, no bloquea a quien no la commitea",
                  lb.comprueba_fuentes(str(rf2), desde_indice=True) == [],
                  str(lb.comprueba_fuentes(str(rf2), desde_indice=True)))

        # Un fragmento HUERFANO —versionado, pero sin include() en
        # CMakeLists.txt— no participa en la configuracion y no puede romper
        # ningun clon, asi que en modo indice no debe bloquear. Caso real:
        # cmake/FichaVistaTests.cmake esta en HEAD desde el 17/09 sin sus
        # cuatro fuentes y sin su include(), y con la version anterior de
        # _cmakes_del_indice bloqueaba todos los commits de todos los agentes.
        rh = siembra_repo(tmp / "fragmento_huerfano")
        escribe(rh / "cmake" / "Huerfano.cmake",
                "add_executable(h tests/Huerfano.cpp)\n")
        subprocess.run(["git", "add", "cmake/Huerfano.cmake"], cwd=rh,
                       check=True, capture_output=True)
        comprueba("un fragmento huerfano con fuentes sin versionar no bloquea",
                  lb.comprueba_fuentes(str(rh), desde_indice=True) == [],
                  str(lb.comprueba_fuentes(str(rh), desde_indice=True)))
        # Y en cuanto alguien lo incluye, la ley vuelve a aplicar entera: no
        # es un agujero, es que antes no configuraba nada.
        escribe(rh / "CMakeLists.txt",
                "project(demo)\ninclude(cmake/Uno.cmake)\n"
                "include(cmake/Huerfano.cmake)\n")
        subprocess.run(["git", "add", "CMakeLists.txt"], cwd=rh,
                       check=True, capture_output=True)
        pfh = lb.comprueba_fuentes(str(rh), desde_indice=True)
        comprueba("al incluirlo, sus fuentes sin versionar si bloquean",
                  len(pfh) == 1 and "Huerfano.cpp" in pfh[0], str(pfh))

        # --- 4 bis. el mismo cuerpo en dos fuentes no enlaza.
        rd = tmp / "duplicado"
        escribe(rd / "src" / "Uno.cpp", "void Clase::hazlo() {\n}\n")
        comprueba("una sola definicion no molesta",
                  lb.comprueba_simbolos(str(rd)) == [],
                  str(lb.comprueba_simbolos(str(rd))))
        escribe(rd / "src" / "Dos.cpp", "void Clase::hazlo() {\n}\n")
        pro = lb.comprueba_simbolos(str(rd))
        comprueba("la misma definicion en dos fuentes falla", len(pro) == 1, str(pro))
        comprueba("y nombra el simbolo y los dos ficheros",
                  "Clase::hazlo" in pro[0] and "Uno.cpp" in pro[0] and "Dos.cpp" in pro[0],
                  str(pro))

        # --- 4 bis-2. Funcion LIBRE dentro de un namespace con nombre.
        # Es la forma que se le escapo a la regla: el 28/09 ValidateAdventure
        # estaba definida en AdventureValidation.cpp y en
        # AdventureBeatValidation.cpp, la suite entera no enlazaba, y la guardia
        # decia OK porque DEFINICION solo casa `Clase::metodo`.
        rn = tmp / "duplicado_libre"
        cuerpo_libre = ("namespace RPG {\n\n"
                        "ValidationReport Validar(const Script& s) {\n"
                        "  return {};\n}\n\n}  // namespace RPG\n")
        escribe(rn / "src" / "Uno.cpp", cuerpo_libre)
        comprueba("una funcion libre sola no molesta",
                  lb.comprueba_simbolos(str(rn)) == [],
                  str(lb.comprueba_simbolos(str(rn))))
        escribe(rn / "src" / "Dos.cpp", cuerpo_libre)
        pro = lb.comprueba_simbolos(str(rn))
        comprueba("la misma funcion libre en dos fuentes falla", len(pro) == 1, str(pro))
        comprueba("y la nombra con su namespace", "RPG::Validar" in pro[0], str(pro))

        # La forma exacta del fallo real: namespace con nombre y, dentro, un
        # namespace ANONIMO con los ayudantes. Lo de fuera duplica; lo de
        # dentro no, porque tiene vinculacion interna.
        rmix = tmp / "anonimo_dentro"
        escribe(rmix / "src" / "Uno.cpp",
                "namespace RPG {\nnamespace {\n\nvoid Ayuda() {\n}\n\n"
                "}  // namespace\n\nValidationReport Validar(const Script& s) {\n"
                "  return {};\n}\n\n}  // namespace RPG\n")
        escribe(rmix / "src" / "Dos.cpp",
                "namespace RPG {\nnamespace {\n\nvoid Ayuda() {\n}\n\n"
                "}  // namespace\n\nValidationReport Validar(const Script& s) {\n"
                "  return {};\n}\n\n}  // namespace RPG\n")
        pro = lb.comprueba_simbolos(str(rmix))
        comprueba("el ayudante anonimo no cuenta como duplicado",
                  all("Ayuda" not in x for x in pro), str(pro))
        comprueba("pero lo que esta fuera del anonimo si",
                  any("RPG::Validar" in x for x in pro), str(pro))

        # Dos copias del MISMO ayudante en namespace anonimo son legales: cada
        # fuente tiene la suya. Fallar aqui seria un falso positivo, y un falso
        # positivo bloquea los commits de todo el mundo.
        ra = tmp / "anonimo"
        texto_anon = ("namespace RPG {\nnamespace {\n\nvoid Ayuda() {\n}\n\n"
                      "}  // namespace\n}  // namespace RPG\n")
        escribe(ra / "src" / "Uno.cpp", texto_anon)
        escribe(ra / "src" / "Dos.cpp", texto_anon)
        comprueba("dos copias en namespace anonimo no son duplicado",
                  lb.comprueba_simbolos(str(ra)) == [],
                  str(lb.comprueba_simbolos(str(ra))))

        # Una declaracion termina en ';' y no define nada.
        rdec = tmp / "declaraciones"
        escribe(rdec / "src" / "Uno.cpp", "namespace RPG {\n\nvoid Ayuda();\n\n}\n")
        escribe(rdec / "src" / "Dos.cpp", "namespace RPG {\n\nvoid Ayuda();\n\n}\n")
        comprueba("dos declaraciones del mismo simbolo no molestan",
                  lb.comprueba_simbolos(str(rdec)) == [],
                  str(lb.comprueba_simbolos(str(rdec))))

        # Llaves dentro de comentarios y de strings. Si se cuentan, la
        # profundidad se descoloca: fue el fallo del primer intento, que trataba
        # cualquier '}' en columna 0 como cierre de namespace.
        rcom = tmp / "comentarios"
        escribe(rcom / "src" / "Uno.cpp",
                "namespace Editor {\n\n// } } } llaves en un comentario\n"
                "/* y } aqui tambien */\n"
                "bool existe(const std::string& ruta) {\n"
                "  return ruta.find(\"}\") != std::string::npos;\n}\n\n}\n")
        escribe(rcom / "src" / "Dos.cpp",
                "namespace Editor {\n\nbool otraCosa(int x) {\n  return x > 0;\n}\n\n}\n")
        comprueba("las llaves de un comentario o un string no cuentan",
                  lb.comprueba_simbolos(str(rcom)) == [],
                  str(lb.comprueba_simbolos(str(rcom))))

        # Lo que esta DENTRO de un cuerpo no es un simbolo libre del modulo.
        rcue = tmp / "cuerpos"
        cuerpo = ("namespace RPG {\n\nvoid Externo() {\n"
                  "auto interno = []() {\n};\n}\n\n}\n")
        escribe(rcue / "src" / "Uno.cpp", cuerpo)
        escribe(rcue / "src" / "Dos.cpp", cuerpo)
        pro = lb.comprueba_simbolos(str(rcue))
        comprueba("lo de dentro de un cuerpo no se cuenta como libre",
                  all("interno" not in x for x in pro), str(pro))

        # --- 4 ter. La forma canonica no mezcla datos y controlador.
        rm = tmp / "estructura_mixta"
        escribe(rm / "src" / "SoloDatos.cpp", "struct DialogData {\n  int page;\n};\n")
        escribe(rm / "src" / "SoloControlador.cpp",
                "void DialogController::advance() {\n}\n")
        escribe(rm / "src" / "Comentarios.cpp",
                "// struct DialogData { };\n"
                "// void DialogController::advance() {}\n")
        comprueba("datos o controlador aislados no se mezclan",
                  lb.comprueba_estructura_mixta(str(rm)) == [],
                  str(lb.comprueba_estructura_mixta(str(rm))))
        escribe(rm / "src" / "Mezclado.cpp",
                "struct DialogData {\n  int page;\n};\n"
                "Result<bool> DialogController::advance() {\n  return {};\n}\n")
        mixtas = lb.comprueba_estructura_mixta(str(rm))
        comprueba("datos y controlador en el mismo cpp fallan", len(mixtas) == 1,
                  str(mixtas))
        comprueba("el diagnostico nombra fichero y corte", "Mezclado.cpp" in mixtas[0]
                  and "Extrae XData/XState" in mixtas[0], str(mixtas))
        escribe(rm / "src" / "ControladorLocal.cpp",
                "struct MenuState {\n  int option;\n};\n"
                "class MenuController {\n public:\n  void advance() {}\n};\n")
        comprueba("un controlador definido localmente tambien falla",
                  any("ControladorLocal.cpp" in problema
                      for problema in lb.comprueba_estructura_mixta(str(rm))))
        rmc = siembra_repo(tmp / "estructura_mixta_cli")
        escribe(rmc / "src" / "Mezclado.cpp",
                "struct DialogData {\n  int page;\n};\n"
                "void DialogController::advance() {\n}\n")
        comprueba("la guardia rapida rechaza la estructura mixta",
                  lb.main(["--raiz", str(rmc), "--rapido"]) == 1)

        # --- 5. el techo: quien lo pasa y quien no.
        r5 = tmp / "core"
        fichero_de(r5, "src/Gordo.cpp", lb.TECHO + 50)
        fichero_de(r5, "src/Fino.cpp", 40)
        fichero_de(r5, "include/Justo.h", lb.TECHO)
        cuellos = lb.mide_cuellos(str(r5))
        comprueba("el techo coge al gordo", "src/Gordo.cpp" in cuellos)
        comprueba("y deja pasar al fino", "src/Fino.cpp" not in cuellos)
        comprueba("el techo es estricto, no lo cruza quien lo iguala",
                  "include/Justo.h" not in cuellos, str(sorted(cuellos)))

        # --- 6. el trinquete.
        base = {"fecha": "2026-09-20", "muertos": ["src/Muerto.cpp"],
                "logica_en_pruebas": [], "cierres_sin_alcance": [],
                "cuellos": {"src/Gordo.cpp": 1000},
                "mezcladas": [], "fuentes_directas": 10}
        peor = dict(base, cuellos={"src/Gordo.cpp": 1001})
        pro, _ = lb.compara(peor, base)
        comprueba("crecer por encima del techo falla", len(pro) == 1, str(pro))
        comprueba("y dice cuanto ha crecido",
                  any("1000 a 1001" in p for p in pro), str(pro))

        nuevo = dict(base, cuellos={"src/Gordo.cpp": 1000, "src/Otro.cpp": 900})
        pro, _ = lb.compara(nuevo, base)
        comprueba("un fichero nuevo por encima del techo falla",
                  len(pro) == 1 and "src/Otro.cpp" in pro[0], str(pro))

        mejor = dict(base, cuellos={"src/Gordo.cpp": 700})
        pro, mej = lb.compara(mejor, base)
        comprueba("adelgazar no falla", pro == [], str(pro))
        comprueba("adelgazar se cuenta como mejora",
                  any("baja de 1000 a 700" in m for m in mej), str(mej))

        partido = dict(base, cuellos={})
        pro, mej = lb.compara(partido, base)
        comprueba("partirlo del todo no falla", pro == [], str(pro))
        comprueba("y se anuncia",
                  any("por debajo del techo" in m for m in mej), str(mej))

        # --- 7. la deuda de siempre: muertos y logica en la prueba.
        con_muerto = dict(base, muertos=["src/Muerto.cpp", "src/NuevoMuerto.cpp"])
        pro, _ = lb.compara(con_muerto, base)
        comprueba("un muerto nuevo falla",
                  len(pro) == 1 and "NuevoMuerto" in pro[0], str(pro))

        con_logica = dict(base, logica_en_pruebas=["tests/X.cpp"])
        pro, _ = lb.compara(con_logica, base)
        comprueba("logica dentro de una prueba falla",
                  len(pro) == 1 and "X.cpp" in pro[0], str(pro))

        sin_muerto = dict(base, muertos=[])
        pro, mej = lb.compara(sin_muerto, base)
        comprueba("retirar un muerto no falla", pro == [], str(pro))
        comprueba("retirar un muerto es mejora", len(mej) == 1, str(mej))

        # --- 8. la frontera GL y las fuentes compiladas sueltas.
        r8 = tmp / "libs"
        escribe(r8 / "src" / "Gl.cpp", '#include <glad/glad.h>\nvoid a(){}\n')
        escribe(r8 / "src" / "Puro.cpp", "void b(){}\n")
        escribe(r8 / "src" / "Puro2.cpp", "void c(){}\n")
        escribe(r8 / "CMakeLists.txt",
                "add_library(mezclada src/Gl.cpp src/Puro.cpp)\n"
                "add_library(pura src/Puro2.cpp)\n"
                # parentesis dentro del bloque: si se lee con una regla
                # perezosa, esta fuente no se cuenta.
                "add_executable(t tests/x.cpp src/Puro.cpp\n"
                "    $<TARGET_FILE:pura>)\n")
        libs = lb.mide_librerias(str(r8))
        comprueba("detecta la libreria que cruza la frontera GL",
                  len(libs["mezcladas"]) == 1
                  and libs["mezcladas"][0].startswith("mezclada"),
                  str(libs["mezcladas"]))
        comprueba("no marca la libreria pura",
                  not any(m.startswith("pura") for m in libs["mezcladas"]))
        comprueba("cuenta la fuente suelta aunque el bloque lleve parentesis",
                  libs["fuentes_directas"] == 1, str(libs))

        base2 = dict(base, mezcladas=[], fuentes_directas=10)
        pro, _ = lb.compara(dict(base2, mezcladas=["nueva (1 con GL, 1 sin)"]),
                            base2)
        comprueba("una libreria mezclada nueva falla",
                  len(pro) == 1 and "nueva" in pro[0], str(pro))
        # La trampa que se colo durante meses: crecer en fuentes CON GL
        # dentro de una libreria que YA estaba mezclada no es deuda nueva,
        # y sobre todo no es una mejora. Comparar el texto entero decia
        # las dos cosas a la vez.
        base3 = dict(base, mezcladas=["motor_core (13 con GL, 12 sin)"],
                     fuentes_directas=10)
        pro, mej = lb.compara(
            dict(base3, mezcladas=["motor_core (16 con GL, 12 sin)"]), base3)
        comprueba("mas fuentes CON GL en una libreria ya mezclada no es deuda",
                  pro == [], str(pro))
        comprueba("y tampoco es mejora", mej == [], str(mej))

        pro, _ = lb.compara(
            dict(base3, mezcladas=["motor_core (13 con GL, 14 sin)"]), base3)
        comprueba("mas rehenes GL-free si falla",
                  len(pro) == 1 and "de 12 a 14" in pro[0], str(pro))

        _, mej = lb.compara(
            dict(base3, mezcladas=["motor_core (13 con GL, 9 sin)"]), base3)
        comprueba("menos rehenes GL-free es mejora",
                  any("de 12 a 9" in m for m in mej), str(mej))

        _, mej = lb.compara(dict(base3, mezcladas=[]), base3)
        comprueba("una libreria que deja de mezclar es mejora",
                  any("ya no cruza la frontera GL" in m for m in mej), str(mej))

        pro, _ = lb.compara(dict(base2, fuentes_directas=11), base2)
        comprueba("mas fuentes sueltas falla",
                  len(pro) == 1 and "de 10 a 11" in pro[0], str(pro))
        pro, mej = lb.compara(dict(base2, fuentes_directas=9), base2)
        comprueba("menos fuentes sueltas es mejora",
                  pro == [] and any("baja de 10 a 9" in m for m in mej),
                  f"{pro} {mej}")

        # --- 9. la foto de alcance vieja se avisa: los muertos salen de ahi.
        r9 = tmp / "frescura"
        fichero_de(r9, "src/Nuevo.cpp", 10)
        comprueba("sin foto de alcance, lo dice",
                  any("falta planificacion/alcance_real.json" in x
                      for x in lb.frescura_del_alcance(str(r9))))
        escribe(r9 / "planificacion" / "alcance_real.json", "{}")
        os.utime(r9 / "planificacion" / "alcance_real.json", (1_000_000, 1_000_000))
        comprueba("una foto mas vieja que el codigo se avisa",
                  any("mas viejo" in x for x in lb.frescura_del_alcance(str(r9))),
                  str(lb.frescura_del_alcance(str(r9))))
        import time as _t
        os.utime(r9 / "planificacion" / "alcance_real.json", (_t.time() + 60,) * 2)
        comprueba("una foto reciente no se avisa",
                  lb.frescura_del_alcance(str(r9)) == [],
                  str(lb.frescura_del_alcance(str(r9))))

        # --- 10. la linea base no se compara consigo misma por accidente.
        pro, mej = lb.compara(base, base)
        comprueba("igual a la linea base: ni problemas ni mejoras",
                  pro == [] and mej == [], f"{pro} {mej}")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    return casos, fallos


def main():
    casos, fallos = ejecutar_casos()
    if fallos:
        print("test_linea_base_core: FALLO (%d casos, %d fallos)"
              % (casos, len(fallos)))
        for f in fallos:
            print("  - " + f)
        return 1
    print("test_linea_base_core: OK - %d casos, todos verdes" % casos)
    return 0


if __name__ == "__main__":
    sys.exit(main())
