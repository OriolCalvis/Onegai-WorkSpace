#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Prueba de `exportar.py` sobre un repositorio SEMBRADO.

POR QUE ASI. Probar el exportador contra `assets/` de verdad no es
falsable: el dia que alguien declare un nivel mas en el manifiesto de
Boundington cambian los numeros y el test se pone rojo sin que la
herramienta se haya roto; y al reves, con 98 niveles y 70 texturas de
ruido es facil que un recolector roto siga «acertando». Asi que se
siembra un repositorio minusculo con un caso de cada cosa que puede
salir mal en una entrega, y se exige las dos mitades: que arrastre lo
que hace falta y que NO se lleve lo que no se usa.

Los seis casos son los seis que rompen una entrega de verdad:

  1. la puerta que lleva a un nivel que nadie declaro;
  2. el tileset que el TMX cita en relativo;
  3. la ruta escrita en prosa, con el punto de la frase pegado;
  4. lo que el motor carga por ruta fija y ningun manifiesto declara;
  5. un binario de otra plataforma (entregar eso es mentir);
  6. un objectId sin ficha dentro: no da error, da un objeto invisible.

Se ejecuta sola:  python3 tools/test_exportar.py
"""
from __future__ import annotations

import importlib.util
import json
import os
import shutil
import sys
import tempfile
from pathlib import Path

AQUI = Path(__file__).resolve()


def _cargar():
    spec = importlib.util.spec_from_file_location(
        "exportar", AQUI.parent / "exportar.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


APLICACION = '''// Aplicacion de mentira: solo tiene que llevar dentro las
// rutas fijas que el exportador debe leer del codigo.
void Application::init() {
    m_shaderManager.load("sprite", "assets/shaders/sprite");
    m_skills.loadFromFile("assets/catalogs/habilidades.json");
    m_proyectos.load("assets/proyectos/index.json");
    // Esta no existe en disco y no debe exportarse: "assets/maps/x.tmx"
}
'''

TMX = '''<?xml version="1.0" encoding="UTF-8"?>
<map version="1.10" orientation="isometric" width="4" height="4"
     tilewidth="64" tileheight="32">
 <tileset firstgid="1" name="suelo" tilewidth="64" tileheight="32">
  <image source="../textures/{imagen}" width="64" height="32"/>
 </tileset>
 <layer id="1" name="suelo" width="4" height="4">
  <data encoding="csv">1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1</data>
 </layer>
</map>
'''


def escribe(ruta: Path, contenido):
    ruta.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(contenido, (dict, list)):
        ruta.write_text(json.dumps(contenido, ensure_ascii=False, indent=1),
                        encoding="utf-8")
    elif isinstance(contenido, bytes):
        ruta.write_bytes(contenido)
    else:
        ruta.write_text(contenido, encoding="utf-8")


def siembra(raiz: Path, objeto_sin_ficha=False):
    """Un repositorio de bolsillo con un caso de cada trampa."""
    escribe(raiz / "src" / "Engine" / "Application.cpp", APLICACION)
    escribe(raiz / "assets" / "shaders" / "sprite.vert", "// vert\n")
    escribe(raiz / "assets" / "shaders" / "sprite.frag", "// frag\n")
    escribe(raiz / "assets" / "catalogs" / "habilidades.json",
            {"entries": [{"id": "tajo", "name": "Tajo"}]})
    escribe(raiz / "assets" / "catalogs" / "razas.json",
            {"entries": [{"id": "humano", "name": "Humano"}]})

    # El patio esta declarado; la cripta NO, y solo se llega por la puerta.
    objetos_patio = [
        {"objectId": "farol", "position": {"x": 1, "y": 1}},
        {"objectId": "puerta_cripta", "position": {"x": 3, "y": 3},
         "targetLevel": "assets/levels/cripta.json",
         "targetPosition": {"x": 1, "y": 1}},
    ]
    if objeto_sin_ficha:
        objetos_patio.append({"objectId": "estatua_que_nadie_definio",
                              "position": {"x": 2, "y": 2}})
    escribe(raiz / "assets" / "levels" / "patio.json",
            {"name": "Patio", "map": "assets/maps/patio.tmx",
             "playerStart": {"x": 0, "y": 0}, "objects": objetos_patio})
    escribe(raiz / "assets" / "levels" / "cripta.json",
            {"name": "Cripta", "map": "assets/maps/cripta.tmx",
             "playerStart": {"x": 1, "y": 1},
             "objects": [{"objectId": "cofre", "position": {"x": 2, "y": 2}}]})
    # Nadie lleva aqui: no debe salir en la entrega.
    escribe(raiz / "assets" / "levels" / "isla_lejana.json",
            {"name": "Isla", "map": "assets/maps/isla.tmx",
             "playerStart": {"x": 0, "y": 0}, "objects": []})

    escribe(raiz / "assets" / "maps" / "patio.tmx", TMX.format(imagen="suelo.png"))
    escribe(raiz / "assets" / "maps" / "cripta.tmx", TMX.format(imagen="suelo.png"))
    escribe(raiz / "assets" / "maps" / "isla.tmx", TMX.format(imagen="nunca.png"))
    escribe(raiz / "assets" / "textures" / "suelo.png", b"\x89PNG\r\n\x1a\nsuelo")
    escribe(raiz / "assets" / "textures" / "nunca.png", b"\x89PNG\r\n\x1a\nnunca")

    # La prosa con el punto pegado al nombre: el fichero existe y hay que
    # arrastrarlo; el punto no es parte del nombre.
    escribe(raiz / "assets" / "objects" / "demo_objetos.json",
            {"_fuente": "La raza sale de assets/catalogs/razas.json. raceId es "
                        "la referencia estable al catalogo RPG.",
             "objects": [
                 {"id": "farol", "name": "Farol", "spriteId": 3},
                 {"id": "puerta_cripta", "name": "Puerta", "spriteId": 4},
                 {"id": "cofre", "name": "Cofre", "spriteId": 5}]})
    escribe(raiz / "assets" / "adventures" / "prologo.json",
            {"id": "prologo", "beats": [{"id": "b1", "text": "Empieza."}]})
    escribe(raiz / "assets" / "proyectos" / "index.json",
            {"_nota": "Proyectos vivos del repositorio.",
             "proyectos": ["demo", "otro_que_no_va_en_la_entrega"]})
    escribe(raiz / "assets" / "proyectos" / "demo.json", {
        "schemaVersion": 1, "id": "demo", "name": "Demo de bolsillo",
        "description": "Proyecto sembrado para la prueba del exportador.",
        "epoch": "0 b.f.", "author": "prueba", "prefix": "",
        "settings": {"schemaVersion": 1, "mapsDirectory": "maps",
                     "levelsDirectory": "levels",
                     "adventuresDirectory": "adventures",
                     "catalogsDirectory": "objects", "initialMap": ""},
        "entry": "assets/adventures/prologo.json",
        "levels": ["patio.json"], "maps": ["patio.tmx"],
        "adventures": ["prologo.json"], "catalogs": ["demo_objetos.json"]})
    return raiz


def binario_falso(ruta: Path, plataforma="linux"):
    firma = {"linux": b"\x7fELF\x02\x01\x01", "windows": b"MZ\x90\x00",
             "macos": b"\xcf\xfa\xed\xfe"}[plataforma]
    escribe(ruta, firma + b"\x00" * 64)
    os.chmod(ruta, 0o755)
    return str(ruta)


def ejecutar_casos():
    ex = _cargar()
    casos, fallos = 0, []
    tmp = Path(tempfile.mkdtemp(prefix="test_exportar_"))

    def comprueba(nombre, condicion, detalle=""):
        nonlocal casos
        casos += 1
        if not condicion:
            fallos.append(f"{nombre}: {detalle}" if detalle else nombre)

    try:
        raiz = siembra(tmp / "repo")
        binario = binario_falso(tmp / "repo" / "build" / "juego", "linux")
        salida = tmp / "entrega"
        informe = ex.exporta(str(raiz), "demo", str(salida), "linux",
                             binario, silencioso=True)
        # Se mira la carpeta que el informe diga, publicada o .parcial:
        # si una mutacion rompe el recolector, el caso tiene que salir en
        # rojo con su nombre, no reventar la prueba entera con un rastro.
        carpeta = Path(informe["carpeta"])
        dentro = {str(p.relative_to(carpeta)).replace(os.sep, "/")
                  for p in carpeta.rglob("*") if p.is_file()}

        # 1. La entrega se publica y se sabe arrancar.
        comprueba("publica", informe["jugable"] and salida.is_dir(),
                  f"motivos={informe['motivos']}")
        comprueba("no queda parcial", not (tmp / "entrega.parcial").exists())
        comprueba("lleva lanzador y binario",
                  {"jugar.sh", "juego", "export.json", "LEEME.txt"} <= dentro,
                  f"dentro={sorted(dentro)[:6]}")
        comprueba("el lanzador es ejecutable",
                  os.access(carpeta / "jugar.sh", os.X_OK))
        comprueba("el lanzador entra por su propia carpeta",
                  'cd "$(dirname "$0")"' in (carpeta / "jugar.sh").read_text())

        # 2. Lo que nadie declara y hace falta igual.
        comprueba("arrastra el nivel de la puerta",
                  "assets/levels/cripta.json" in dentro)
        comprueba("arrastra el mapa de ese nivel",
                  "assets/maps/cripta.tmx" in dentro)
        comprueba("arrastra el tileset del TMX",
                  "assets/textures/suelo.png" in dentro)
        comprueba("arrastra la ruta citada en prosa con punto final",
                  "assets/catalogs/razas.json" in dentro,
                  "el punto de la frase se ha comido el nombre del fichero")
        comprueba("arrastra el suelo del motor leido del codigo",
                  {"assets/shaders/sprite.vert", "assets/shaders/sprite.frag",
                   "assets/catalogs/habilidades.json"} <= dentro)
        comprueba("el arrastre queda explicado",
                  any(a["arrastra"] == "assets/levels/cripta.json"
                      and a["desde"] == "assets/levels/patio.json"
                      for a in informe["arrastrados"]))

        indice = json.loads(
            (carpeta / "assets" / "proyectos" / "index.json").read_text(
                encoding="utf-8"))
        comprueba("el indice de la entrega lista solo lo que va dentro",
                  indice["proyectos"] == ["demo"], str(indice["proyectos"]))
        comprueba("y el indice del repositorio no se toca",
                  json.loads((raiz / "assets" / "proyectos" / "index.json")
                             .read_text(encoding="utf-8"))["proyectos"]
                  == ["demo", "otro_que_no_va_en_la_entrega"])
        comprueba("el ajuste queda escrito en export.json",
                  any("index.json" in a for a in informe["ajustes"]),
                  str(informe["ajustes"]))

        # 3. Solo lo que usa: lo demas se queda en el repositorio.
        comprueba("no se lleva el nivel al que nadie llega",
                  "assets/levels/isla_lejana.json" not in dentro)
        comprueba("no se lleva el mapa de ese nivel",
                  "assets/maps/isla.tmx" not in dentro)
        comprueba("no se lleva la textura que nadie usa",
                  "assets/textures/nunca.png" not in dentro)
        comprueba("no se inventa la ruta de prueba del codigo",
                  "assets/maps/x.tmx" not in dentro)

        # 4. La entrega no mira hacia el repositorio.
        comprueba("ninguna cita sale de la carpeta",
                  ex.comprueba(str(carpeta)) == [],
                  str(ex.comprueba(str(carpeta))))
        comprueba("ningun texto lleva la ruta del repositorio",
                  ex.comprueba(str(carpeta), str(raiz)) == [])

        # 5. Y la comprobacion prueba algo: si falta una pieza, la ve.
        tileset = carpeta / "assets" / "textures" / "suelo.png"
        if tileset.exists():          # si el caso 2 ya fallo, no esta
            os.remove(tileset)
        problemas = ex.comprueba(str(carpeta))
        comprueba("si borro el tileset, la comprobacion lo dice",
                  any("suelo.png" in p for p in problemas), str(problemas))

        # 6. Plataforma: un binario de macOS no es una entrega de Windows.
        raiz2 = siembra(tmp / "repo2")
        mac = binario_falso(tmp / "repo2" / "build" / "juego", "macos")
        salida2 = tmp / "entrega_win"
        inf2 = ex.exporta(str(raiz2), "demo", str(salida2), "windows", mac,
                          silencioso=True)
        comprueba("no publica una entrega de otra plataforma",
                  not inf2["jugable"] and not salida2.exists())
        comprueba("deja el intento en .parcial",
                  Path(str(salida2) + ".parcial").is_dir())
        comprueba("y explica por que",
                  any("windows" in m and "macos" in m for m in inf2["motivos"]),
                  str(inf2["motivos"]))
        comprueba("no dice que hay ejecutable cuando no lo hay",
                  inf2["ejecutable"] is None)

        # 7. Un objectId sin ficha no se entrega: seria un objeto invisible.
        raiz3 = siembra(tmp / "repo3", objeto_sin_ficha=True)
        bin3 = binario_falso(tmp / "repo3" / "build" / "juego", "linux")
        inf3 = ex.exporta(str(raiz3), "demo", str(tmp / "entrega3"), "linux",
                          bin3, silencioso=True)
        comprueba("no publica con objetos sin ficha", not inf3["jugable"])
        comprueba("y dice cual",
                  inf3["objectIds_sin_ficha"] == ["estatua_que_nadie_definio"],
                  str(inf3["objectIds_sin_ficha"]))

        # 8. --solo-assets no se disfraza de entrega jugable.
        inf4 = ex.exporta(str(raiz3), "demo", str(tmp / "entrega4"), "linux",
                          solo_assets=True, silencioso=True)
        comprueba("solo-assets no se marca jugable", not inf4["jugable"])

        # 9. No se tira una carpeta que no es mia.
        ajena = tmp / "carpeta_ajena"
        ajena.mkdir()
        (ajena / "tesis.txt").write_text("trabajo de otro", encoding="utf-8")
        try:
            ex.exporta(str(raiz), "demo", str(ajena), "linux", binario,
                       silencioso=True)
            comprueba("no pisa una carpeta ajena", False, "la ha borrado")
        except SystemExit:
            comprueba("no pisa una carpeta ajena",
                      (ajena / "tesis.txt").exists())

        # 10. El proyecto que no existe se dice, no se revienta.
        try:
            ex.exporta(str(raiz), "no_existe", str(tmp / "nada"), "linux",
                       binario, silencioso=True)
            comprueba("proyecto inexistente avisa", False, "no ha avisado")
        except FileNotFoundError:
            comprueba("proyecto inexistente avisa", True)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    return casos, fallos


def main():
    casos, fallos = ejecutar_casos()
    if fallos:
        print("test_exportar: FALLO (%d casos, %d fallos)" % (casos, len(fallos)))
        for f in fallos:
            print("  - " + f)
        return 1
    print("test_exportar: OK - %d casos, todos verdes" % casos)
    return 0


if __name__ == "__main__":
    sys.exit(main())
