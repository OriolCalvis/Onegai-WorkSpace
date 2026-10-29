#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Prueba de `analizar_duplicados.py` sobre un árbol SEMBRADO.

POR QUÉ ASÍ. Un detector de duplicados que se prueba contra el árbol real no
es falsable: si mañana alguien borra el duplicado, el test «falla» aunque la
herramienta funcione; y si la herramienta se rompe pero el árbol tiene ruido
de sobra, el test pasa. Así que se planta un árbol pequeño con un caso de
cada tipo, se exige que el analizador lo vea, y se exige además que NO vea
los negativos (dos ficheros de nombre parecido y función distinta tienen que
salir como DISIMILAR, no como clon).

Se ejecuta sola (`python3 tools/test_analizar_duplicados.py`) o a través de
`analizar_duplicados.py --comprobar`, que importa `ejecutar_casos`.
"""
from __future__ import annotations

import importlib.util
import json
import os
import shutil
import sys
import tempfile
import time
from pathlib import Path

AQUI = Path(__file__).resolve()


def _cargar_analizador():
    spec = importlib.util.spec_from_file_location(
        "analizar_duplicados", AQUI.parent / "analizar_duplicados.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


WIDGET = """// Widget de prueba
#include "Alpha/Widget.h"
namespace alpha {
int Widget::calcular(int base, int factor) {
    int acumulado = base;
    for (int i = 0; i < factor; ++i) {
        acumulado = acumulado * factor + base;
        if (acumulado > 1000) { acumulado = acumulado - 1000; }
    }
    return acumulado;
}
int Widget::validar(int valor) {
    if (valor < 0) { return -1; }
    if (valor > 100) { return 1; }
    return 0;
}
}  // namespace alpha
"""

# mismo algoritmo, identificadores distintos y una rama más: clon parcial
WIDGET_COPIA = """// Copia con nombres propios
#include "Beta/Widget.h"
namespace beta {
int Widget::calcular(int inicio, int paso) {
    int total = inicio;
    for (int i = 0; i < paso; ++i) {
        total = total * paso + inicio;
        if (total > 1000) { total = total - 1000; }
        if (total == 0) { total = inicio; }
    }
    return total;
}
int Widget::validar(int dato) {
    if (dato < 0) { return -1; }
    if (dato > 100) { return 1; }
    return 0;
}
}  // namespace beta
"""

GRANDE = """// Cabecera grande con muchas declaraciones
#pragma once
#include <string>
#include <vector>
namespace grande {
struct Configuracion { int ancho; int alto; bool completo; };
class Motor {
public:
    explicit Motor(Configuracion cfg);
    int arrancar();
    int parar();
    int estado() const;
    void notificar(const std::string& mensaje);
    std::vector<int> pendientes() const;
private:
    Configuracion m_cfg;
    int m_estado;
};
class Auxiliar {
public:
    int procesar(int valor);
    int descartar(int valor);
};
}  // namespace grande
"""

CORTO = """// Solo la parte de Auxiliar, copiada de la grande
#pragma once
#include <string>
#include <vector>
namespace grande {
class Auxiliar {
public:
    int procesar(int valor);
    int descartar(int valor);
};
}  // namespace grande
"""

ORACULO = """#pragma once
inline int rangoHistorico(int inicio, int pasos) {
    int acumulado = inicio;
    for (int i = 0; i < pasos; ++i) {
        acumulado = acumulado * pasos + inicio;
        if (acumulado > 1000) { acumulado -= 1000; }
    }
    return acumulado;
}
"""

ORACULO_TEST = """#include "Alpha/Oracle.h"

static int legacyRangoHistorico(int inicio, int pasos) {
    int acumulado = inicio;
    for (int i = 0; i < pasos; ++i) {
        acumulado = acumulado * pasos + inicio;
        if (acumulado > 1000) { acumulado -= 1000; }
    }
    return acumulado;
}

int main() {
    const int entradas[][2] = {{1, 1}, {2, 3}, {8, 5}, {11, 7}};
    int fallos = 0;
    for (const auto& entrada : entradas) {
        const int inicio = entrada[0];
        const int pasos = entrada[1];
        if (legacyRangoHistorico(inicio, pasos) != rangoHistorico(inicio, pasos)) {
            ++fallos;
        }
    }
    return fallos;
}
"""

ORACULO_SIN_REGISTRO = """#pragma once
inline int checksumSinRegistro(int valor, int vueltas) {
    int checksum = valor;
    for (int i = 0; i < vueltas; ++i) {
        checksum = (checksum << 1) ^ (valor + i);
        if ((checksum & 1) != 0) { checksum += vueltas; }
    }
    return checksum;
}
"""

ORACULO_SIN_REGISTRO_TEST = """#include "Alpha/OracleWithoutRecord.h"

static int legacyChecksumSinRegistro(int valor, int vueltas) {
    int checksum = valor;
    for (int i = 0; i < vueltas; ++i) {
        checksum = (checksum << 1) ^ (valor + i);
        if ((checksum & 1) != 0) { checksum += vueltas; }
    }
    return checksum;
}

int main() {
    const int entradas[][2] = {{3, 2}, {5, 4}, {9, 6}, {17, 3}};
    int fallos = 0;
    for (const auto& entrada : entradas) {
        if (legacyChecksumSinRegistro(entrada[0], entrada[1])
                != checksumSinRegistro(entrada[0], entrada[1])) {
            ++fallos;
        }
    }
    return fallos;
}
"""

SHIM = """#pragma once
// compatibilidad de rutas: la definicion vive en Alpha/Widget.h
#include "Alpha/Widget.h"
"""

VALIDA_A = '''#!/usr/bin/env python3
"""Validador A: comprueba el campo tier de los catalogos."""
import json
from pathlib import Path
RAIZ = Path(__file__).resolve().parent.parent


def cargar(ruta):
    return json.loads(Path(ruta).read_text(encoding="utf-8"))


def comprobar_tier(entrada):
    if "tier" not in entrada:
        return "falta tier"
    if "rarity" not in entrada:
        return "falta rarity"
    if "id" not in entrada:
        return "falta id"
    return None


def main():
    fallos = 0
    for ruta in sorted((RAIZ / "assets/catalogs").glob("*.json")):
        datos = cargar(ruta)
        for entrada in datos.get("entries", []):
            problema = comprobar_tier(entrada)
            if problema:
                fallos += 1
                print(ruta.name, entrada.get("id"), problema)
    return 1 if fallos else 0


if __name__ == "__main__":
    raise SystemExit(main())
'''

VALIDA_B = '''#!/usr/bin/env python3
"""Validador B: misma comprobacion, escrita del reves."""
import json, sys
from pathlib import Path
BASE = Path(__file__).resolve().parents[1]
OBLIGATORIOS = ("id", "tier", "rarity")


def leer(p):
    with open(p, encoding="utf-8") as fh:
        return json.load(fh)


def revisar(elemento):
    for campo in OBLIGATORIOS:
        if campo not in elemento:
            yield campo


def recorre():
    total = 0
    for archivo in (BASE / "assets" / "catalogs").glob("*.json"):
        for fila in leer(archivo).get("entries", list()):
            for campo in revisar(fila):
                total += 1
                print("%s -> %s sin %s" % (archivo, fila.get("id"), campo))
    return total


if __name__ == "__main__":
    sys.exit(1 if recorre() else 0)
'''

NORMALIZAR_SUELOS = '''#!/usr/bin/env python3
"""Normaliza los suelos de los TMX: reasigna gids."""
import re
from pathlib import Path
RAIZ = Path(__file__).resolve().parent.parent


def gids(texto):
    return [int(g) for g in re.findall(r"gid=.(\\d+).", texto)]


def reasignar(lista, tabla):
    return [tabla.get(g, g) for g in lista]


def main():
    for tmx in sorted((RAIZ / "assets" / "maps").rglob("*.tmx")):
        datos = tmx.read_text(encoding="utf-8")
        reasignar(gids(datos), {})
        tmx.write_text(datos, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
'''

# nombre casi igual, funcion completamente distinta: el caso DISIMILAR
NORMALIZAR_SUELOS_CI = '''#!/usr/bin/env python3
"""Comprueba que el workflow de CI ejecute la etiqueta correcta de ctest."""
import sys
from pathlib import Path
RAIZ = Path(__file__).resolve().parent.parent
ETIQUETAS = ("gl-free", "static")


def leer_workflow():
    ruta = RAIZ / ".github" / "workflows" / "ci.yml"
    return ruta.read_text(encoding="utf-8")


def faltan(texto):
    return [e for e in ETIQUETAS if e not in texto]


def main():
    ausentes = faltan(leer_workflow())
    for etiqueta in ausentes:
        print("ci.yml no ejecuta la etiqueta", etiqueta)
    return 1 if ausentes else 0


if __name__ == "__main__":
    sys.exit(main())
'''


def escribir(raiz: Path, rel: str, texto: str, mtime=None):
    p = raiz / rel
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(texto, encoding="utf-8")
    if mtime is not None:
        os.utime(p, (mtime, mtime))
    return p


def sembrar(raiz: Path):
    ahora = time.time()
    viejo = ahora - 86400 * 30

    escribir(raiz, "include/Alpha/Widget.h",
             "#pragma once\nnamespace alpha { class Widget { public: int calcular(int, int);"
             " int validar(int); }; }\n")
    escribir(raiz, "include/Beta/Widget.h",
             "#pragma once\nnamespace beta { class Widget { public: int calcular(int, int);"
             " int validar(int); }; }\n")
    escribir(raiz, "include/Alpha/Grande.h", GRANDE)
    escribir(raiz, "include/Alpha/Corto.h", CORTO)
    escribir(raiz, "include/Alpha/Oracle.h", ORACULO)
    escribir(raiz, "include/Alpha/OracleWithoutRecord.h", ORACULO_SIN_REGISTRO)
    escribir(raiz, "include/Alpha/Shim.h", SHIM)
    # mismo nombre plenamente cualificado en dos cabeceras: conflicto real
    escribir(raiz, "include/Alpha/Doble.h",
             "#pragma once\nnamespace comun { struct Doble { int valor; }; }\n")
    escribir(raiz, "include/Beta/Doble.h",
             "#pragma once\nnamespace comun { struct Doble { int otro; }; }\n")
    escribir(raiz, "src/Alpha/Widget.cpp", WIDGET)
    escribir(raiz, "src/Alpha/Widget 2.cpp", WIDGET)      # copia macOS, identica
    escribir(raiz, "src/Beta/WidgetCopia.cpp", WIDGET_COPIA)
    escribir(raiz, "src/Alpha/Ruido.cpp",
             '#include "Alpha/Grande.h"\nint otro_asunto(int a, int b) { return a - b; }\n')
    escribir(raiz, "examples/juego.cpp",
             '#include "Alpha/Widget.h"\nint main() { alpha::Widget w; return w.validar(1); }\n')
    escribir(raiz, "tests/Alpha/WidgetTests.cpp",
             '#include "Alpha/Widget.h"\nint main() { alpha::Widget w; return w.calcular(2, 3) == 0; }\n')
    escribir(raiz, "tests/Alpha/OracleTests.cpp", ORACULO_TEST)
    escribir(raiz, "tests/Alpha/OracleWithoutRecordTests.cpp", ORACULO_SIN_REGISTRO_TEST)
    # la logica del paquete vive en el test y en ningun sitio mas
    escribir(raiz, "tests/Alpha/LogicaPropiaTests.cpp",
             '#include "Alpha/Widget.h"\n'
             'int logicaSoloEnTest(int valor) { return valor * 2; }\n'
             'int main() { return logicaSoloEnTest(2) == 4; }\n')

    escribir(raiz, "tools/valida_a.py", VALIDA_A)
    escribir(raiz, "tools/valida_b.py", VALIDA_B)
    escribir(raiz, "tools/normalizar_suelos.py", NORMALIZAR_SUELOS)
    escribir(raiz, "tools/normalizar_suelos_ci.py", NORMALIZAR_SUELOS_CI)
    escribir(raiz, ".github/workflows/ci.yml",
             "jobs:\n  test:\n    run: ctest -L gl-free\n")

    ids = [{"id": "cosa_%d" % i, "tier": i % 5} for i in range(8)]
    escribir(raiz, "assets/catalogs/cosas.json",
             json.dumps({"entries": ids}, ensure_ascii=False))
    escribir(raiz, "assets/catalogs/cosas_gemelo.json",
             json.dumps({"entries": [dict(x) for x in ids]}, ensure_ascii=False))
    escribir(raiz, "assets/catalogs/otras.json",
             json.dumps({"entries": [{"id": "otra_%d" % i} for i in range(8)]},
                        ensure_ascii=False))
    (raiz / "assets/maps").mkdir(parents=True, exist_ok=True)

    # alcance real simulado: la salida de medir_alcance.py
    escribir(raiz, "planificacion/alcance_real.json", json.dumps({
        "cabeceras": {
            "include/Alpha/Widget.h": {
                "implementacion": ["src/Alpha/Widget.cpp"], "lineas": 5, "pruebas": 1,
                "juego": {"desde": "examples/juego.cpp", "por": ["Widget"]}},
            "include/Alpha/Muerto.h": {
                "implementacion": ["src/Alpha/Muerto.cpp"], "lineas": 40, "pruebas": 3},
            "include/Alpha/Grande.h": {
                "implementacion": [], "lineas": 30, "pruebas": 0,
                "motor_editor": {"desde": "editor/src/x.cpp", "por": ["Motor"]}},
        },
        "ejecutables": {},
        "paquetes": {
            "H-99": {"ficheros": 3, "juego": [], "motor_editor": [],
                     "level_editor": [], "nadie": ["src/Alpha/LogicaEnTest.cpp"]},
            "H-98": {"ficheros": 1, "juego": ["src/Alpha/Widget.cpp"],
                     "motor_editor": [], "level_editor": [], "nadie": []},
        },
        "contenido": {},
    }, ensure_ascii=False))
    escribir(raiz, "planificacion/seguimiento/H-99.json", json.dumps({
        "paquete": "H-99", "estado": "closed",
        "archivos_creados": ["src/Alpha/LogicaEnTest.cpp - la logica vive en el test",
                             "include/Alpha/Muerto.h - nucleo sin consumidor"]},
        ensure_ascii=False))
    escribir(raiz, "planificacion/seguimiento/H-97.json", json.dumps({
        "paquete": "H-97", "estado": "closed",
        "criterios_cierre": {"Logica entregada": "logicaSoloEnTest devuelve el doble"},
        "tests": {"test_logica": {"count": 1, "pass": 1, "fail": 0}},
        "excepciones_duplicados": [
            {
                "tipo": "ENVOLTORIO",
                "archivos": ["include/Alpha/Oracle.h", "tests/Alpha/OracleTests.cpp"],
                "clasificacion": "ORACULO_HISTORICO_DE_PRUEBA",
                "motivo": "La función legacy conserva el comportamiento previo como oráculo.",
                "oraculo": "legacyRangoHistorico",
                "prueba": "test_oracle",
                "regla_de_cambio": "No editar el oráculo para acompañar producción.",
            },
            {
                "tipo": "ENVOLTORIO",
                "archivos": ["include/Alpha/Corto.h", "include/Alpha/Grande.h"],
                "clasificacion": "ORACULO_HISTORICO_DE_PRUEBA",
                "motivo": "Registro inválido: no hay una prueba en el par.",
                "oraculo": "no_debe_aceptarse",
                "prueba": "test_inexistente",
                "regla_de_cambio": "No debe silenciar el envoltorio real.",
            },
            {
                "tipo": "ENVOLTORIO",
                "archivos": [{"ruta": "include/Alpha/Roto.h"},
                              "tests/Alpha/RotoTests.cpp"],
                "clasificacion": "ORACULO_HISTORICO_DE_PRUEBA",
                "motivo": "Registro mal formado que debe ignorarse sin abortar.",
                "oraculo": "no_debe_aceptarse",
                "prueba": "test_inexistente",
                "regla_de_cambio": "No debe romper el analizador.",
            },
            {
                "tipo": "ENVOLTORIO",
                "archivos": ["include/Alpha/Conflicto.h", "tests/Alpha/ConflictoTests.cpp"],
                "clasificacion": "ORACULO_HISTORICO_DE_PRUEBA",
                "motivo": "Primera declaración de un par que no existe.",
                "oraculo": "conflictoUno",
                "prueba": "test_conflicto",
                "regla_de_cambio": "Un duplicado debe invalidar el par.",
            },
            {
                "tipo": "ENVOLTORIO",
                "archivos": ["include/Alpha/Conflicto.h", "tests/Alpha/ConflictoTests.cpp"],
                "clasificacion": "ORACULO_HISTORICO_DE_PRUEBA",
                "motivo": "Segunda declaración incompatible del mismo par.",
                "oraculo": "conflictoDos",
                "prueba": "test_conflicto",
                "regla_de_cambio": "Un duplicado debe invalidar el par.",
            },
        ]},
        ensure_ascii=False))
    escribir(raiz, "planificacion/seguimiento/H-98.json", json.dumps({
        "paquete": "H-98", "estado": "abierto",
        "archivos_creados": ["src/Alpha/Widget.cpp"]}, ensure_ascii=False))

    # un doc desactualizado y otro que se declara historico (no debe salir)
    escribir(raiz, "planificacion/DOC_ANTIGUO.md",
             "# Doc viejo\n\nUsa `Widget.cpp`, `Grande.h`, `WidgetTests.cpp` y "
             "`valida_a.py`.\n", mtime=viejo)
    escribir(raiz, "planificacion/DOC_HISTORICO.md",
             "# Doc congelado\n\n> CONGELADA como historico: no se actualiza.\n\n"
             "Cita `Widget.cpp`, `Grande.h`, `WidgetTests.cpp` y `valida_a.py`.\n",
             mtime=viejo)
    for rel in ("src/Alpha/Widget.cpp", "include/Alpha/Grande.h",
                "tests/Alpha/WidgetTests.cpp", "tools/valida_a.py"):
        escribir(raiz, rel, (raiz / rel).read_text(encoding="utf-8"), mtime=ahora)


def ejecutar_casos(analizar=None, enfocar=None):
    """Devuelve (casos_verificados, fallos). fallos vacio = todo bien."""
    modulo = _cargar_analizador()
    if analizar is None:
        analizar = modulo.analizar
    if enfocar is None:
        enfocar = modulo.enfocar
    tmp = Path(tempfile.mkdtemp(prefix="dup-"))
    fallos = []
    casos = 0
    try:
        sembrar(tmp)
        inf = analizar(tmp, con_git=False)
        por_tipo = {}
        for h in inf["hallazgos"]:
            por_tipo.setdefault(h["tipo"], []).append(h)

        def tiene(tipo, *rutas, exacto=False):
            objetivo = set(rutas)
            return any((objetivo == set(h["archivos"]) if exacto
                        else objetivo <= set(h["archivos"]))
                       for h in por_tipo.get(tipo, []))

        # positivos: un caso por tipo de hallazgo
        positivos = [
            ("DUPLICADO", ("src/Alpha/Widget.cpp", "src/Alpha/Widget 2.cpp"),
             "duplicado exacto + copia macOS ' 2'"),
            ("CLON_PARCIAL", ("src/Alpha/Widget.cpp", "src/Beta/WidgetCopia.cpp"),
             "clon parcial con identificadores distintos"),
            ("ENVOLTORIO", ("include/Alpha/Corto.h", "include/Alpha/Grande.h"),
             "envoltorio por contencion"),
            ("ENVOLTORIO", ("include/Alpha/OracleWithoutRecord.h",
                             "tests/Alpha/OracleWithoutRecordTests.cpp"),
             "oráculo sin registro que sigue siendo envoltorio"),
            ("ORACULO_HISTORICO_DE_PRUEBA",
             ("include/Alpha/Oracle.h", "tests/Alpha/OracleTests.cpp"),
             "oráculo histórico registrado de forma exacta"),
            ("API_DUPLICADA", ("include/Alpha/Doble.h", "include/Beta/Doble.h"),
             "comun::Doble definido en dos cabeceras"),
            ("NOMBRE_AMBIGUO", ("include/Alpha/Widget.h", "include/Beta/Widget.h"),
             "Widget con el mismo nombre en dos modulos"),
            ("LOGICA_EN_EL_TEST", ("tests/Alpha/LogicaPropiaTests.cpp",),
             "paquete cerrado con la logica dentro del test"),
            ("CABECERA_ENVOLTORIO", ("include/Alpha/Shim.h",),
             "cabecera que solo reexporta"),
            ("SOLAPAMIENTO_FUNCIONAL", ("tools/valida_a.py", "tools/valida_b.py"),
             "dos validadores que comprueban lo mismo"),
            ("CATALOGO_GEMELO", ("assets/catalogs/cosas.json",
                                 "assets/catalogs/cosas_gemelo.json"),
             "catalogos gemelos"),
            ("MUERTO", ("include/Alpha/Muerto.h",), "codigo muerto"),
            ("REGISTRO_CERRADO_SIN_ALCANCE", ("src/Alpha/LogicaEnTest.cpp",),
             "paquete cerrado con la logica en el test"),
            ("DOC_DESACTUALIZADO", ("planificacion/DOC_ANTIGUO.md",),
             "doc que cita codigo cambiado despues"),
            ("DISIMILAR", ("tools/normalizar_suelos.py", "tools/normalizar_suelos_ci.py"),
             "disimilares de nombre parecido"),
        ]
        for tipo, rutas, nombre in positivos:
            casos += 1
            if not tiene(tipo, *rutas):
                fallos.append("no ve %s (%s): %s" % (nombre, tipo, " + ".join(rutas)))

        # La reclasificación debe ser de un par exacto y transportar todo el
        # contrato de intención; no basta con que haya rutas como subconjunto.
        casos += 1
        rutas_oraculo = {"include/Alpha/Oracle.h", "tests/Alpha/OracleTests.cpp"}
        oraculos = [h for h in por_tipo.get("ORACULO_HISTORICO_DE_PRUEBA", [])
                    if set(h["archivos"]) == rutas_oraculo]
        if len(oraculos) != 1:
            fallos.append("el oráculo registrado no aparece como par exacto")
        else:
            oraculo = oraculos[0]
            evidencia = oraculo.get("evidencia", {})
            esperado = {
                "oraculo_historico": "legacyRangoHistorico",
                "registro_de_intencion": "planificacion/seguimiento/H-97.json",
                "prueba_que_lo_vigila": "test_oracle",
            }
            if oraculo.get("severidad") != "info":
                fallos.append("el oráculo registrado no tiene severidad informativa")
            for campo, valor in esperado.items():
                if evidencia.get(campo) != valor:
                    fallos.append("el oráculo no conserva %s" % campo)
            if not oraculo.get("justificacion") or not oraculo.get("regla_de_cambio"):
                fallos.append("el oráculo no conserva motivo y regla de cambio")

        casos += 1
        if enfocar(inf, "include/Alpha/Oracle.h").get("veredicto") != "ORACULO_HISTORICO_INTENCIONAL":
            fallos.append("enfocar no distingue el oráculo histórico intencional")

        casos += 1
        excepciones = modulo.cargar_excepciones_duplicados(tmp)
        clave_oraculo = ("ENVOLTORIO", tuple(sorted(rutas_oraculo)))
        clave_conflicto = ("ENVOLTORIO", ("include/Alpha/Conflicto.h",
                                            "tests/Alpha/ConflictoTests.cpp"))
        if clave_oraculo not in excepciones:
            fallos.append("el registro válido del oráculo no se carga")
        if clave_conflicto in excepciones:
            fallos.append("dos registros del mismo par se resolvieron por orden")
        if inf.get("excepciones_duplicados_sin_aplicar"):
            fallos.append("queda una excepción válida sin aplicar en el árbol sembrado")

        # negativos: lo que NO debe decir
        negativos = [
            ("DOC_DESACTUALIZADO", ("planificacion/DOC_HISTORICO.md",),
             "marca un doc que se declara congelado"),
            ("CLON_PARCIAL", ("tools/normalizar_suelos.py", "tools/normalizar_suelos_ci.py"),
             "llama clon a dos scripts que no lo son"),
            ("DUPLICADO", ("tools/normalizar_suelos.py", "tools/normalizar_suelos_ci.py"),
             "llama duplicado a dos scripts distintos"),
            ("SOLAPAMIENTO_FUNCIONAL", ("tools/normalizar_suelos.py",
                                        "tools/normalizar_suelos_ci.py"),
             "llama solapados a dos scripts de funcion distinta"),
            ("CLON_PARCIAL", ("src/Alpha/Ruido.cpp", "src/Alpha/Widget.cpp"),
             "llama clon a dos .cpp sin relacion"),
            ("ENVOLTORIO", ("include/Alpha/Oracle.h", "tests/Alpha/OracleTests.cpp"),
             "trata como deuda el oráculo histórico registrado"),
            ("CATALOGO_GEMELO", ("assets/catalogs/cosas.json", "assets/catalogs/otras.json"),
             "llama gemelos a catalogos con ids distintos"),
            ("API_DUPLICADA", ("include/Alpha/Widget.h", "include/Beta/Widget.h"),
             "llama API duplicada a dos tipos de ambitos distintos"),
            ("LOGICA_EN_EL_TEST", ("tests/Alpha/WidgetTests.cpp",),
             "acusa a un test que solo usa el arnes"),
            ("MUERTO", ("include/Alpha/Widget.h",),
             "da por muerto Widget.h, que si llega a juego"),
            ("MUERTO", ("include/Alpha/Grande.h",),
             "da por muerto Grande.h, que llega a motor_editor"),
            ("REGISTRO_CERRADO_SIN_ALCANCE", ("src/Alpha/Widget.cpp",),
             "acusa a H-98, que esta abierto y si llega"),
        ]
        for tipo, rutas, nombre in negativos:
            casos += 1
            if tiene(tipo, *rutas):
                fallos.append("falso positivo: %s (%s)" % (nombre, tipo))

        # afectacion: el muerto sale sin ejecutables y con su paquete
        casos += 1
        muerto = [h for h in por_tipo.get("MUERTO", [])
                  if "include/Alpha/Muerto.h" in h["archivos"]]
        if not muerto:
            fallos.append("sin hallazgo MUERTO donde comprobar la afectacion")
        else:
            a = muerto[0].get("afectacion", {}).get("include/Alpha/Muerto.h", {})
            if a.get("llega_a"):
                fallos.append("afectacion dice que Muerto.h llega a un ejecutable")
            if "H-99" not in a.get("paquetes_del_gantt", []):
                fallos.append("afectacion de Muerto.h no cita el paquete H-99")

        # todo hallazgo sale con su regla escrita
        casos += 1
        sin_regla = [h["id"] for h in inf["hallazgos"] if not h.get("regla")]
        if sin_regla:
            fallos.append("%d hallazgos sin regla de veredicto" % len(sin_regla))
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    return casos, fallos


def main():
    casos, fallos = ejecutar_casos()
    if fallos:
        print("test_analizar_duplicados: FALLO (%d casos, %d fallos)"
              % (casos, len(fallos)))
        for f in fallos:
            print("  - " + f)
        return 1
    print("test_analizar_duplicados: OK - %d casos plantados, todos vistos" % casos)
    return 0


if __name__ == "__main__":
    sys.exit(main())
