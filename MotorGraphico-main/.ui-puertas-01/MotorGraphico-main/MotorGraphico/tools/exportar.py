#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Exporta un proyecto a una carpeta que se juega SIN el repositorio.

    python3 tools/exportar.py boundington --destino ~/entregas/boundington
    python3 tools/exportar.py boundington --plataforma windows --binario X.exe
    python3 tools/exportar.py boundington --solo-assets      # sin runtime

POR QUE EXISTE (G-06 / H7-01)
-----------------------------
Un juego que solo se puede jugar desde el directorio de build no se ha
acabado. Hoy hay dos medias soluciones y ninguna entrega:

  * `cmake --build` deja el binario en build/ y los assets al lado, pero
    mezclados: los 98 niveles de cuatro autores, las 70 texturas y los 24
    catalogos completos. "Pasame Boundington" significa pasar 30 MB y que
    el otro adivine por donde se entra.
  * `tools/build_proyecto.py` ya separa la historia (lee el manifiesto,
    sigue las puertas, comprueba los objectId) pero deja la carpeta DENTRO
    del repo, copia las texturas enteras y no pone ni runtime ni forma de
    arrancar. Sirve para trabajar; no sirve para entregar.

Esto es la segunda mitad: coge lo que build_proyecto ya sabe decidir, se
queda SOLO con lo que el proyecto usa de verdad, mete el runtime y una
forma de arrancarlo, y lo deja fuera del repositorio.

QUE COMPARTE Y QUE NO CON build_proyecto.py
-------------------------------------------
Comparte el contrato: el manifiesto se lee con `project_manifest.
load_manifest`, que es la unica fuente del formato Project v1. No se
reimplementa aqui.

No comparte el recolector, y es a proposito: `build_proyecto.cierre_de_
niveles` sigue solo `targetLevel` dentro de los niveles. Para entregar no
basta. Aqui el cierre es generico -- se sigue CUALQUIER ruta `assets/...`
que aparezca dentro de un fichero copiado, mas las imagenes que los TMX
referencian con rutas relativas -- porque lo que rompe una entrega es
siempre la referencia que nadie declaro. Cuando esto lleve un tiempo en
verde, `build_proyecto.py` deberia pasar a llamar a `recoge()` en vez de
mantener su propia version (queda apuntado en H8, no duplicar).

EL SUELO DEL MOTOR NO SE ESCRIBE A MANO
---------------------------------------
`Application.cpp` carga un punado de assets por ruta fija: los shaders,
la textura del personaje, los catalogos de habilidades y condiciones. Si
esa lista se copiara aqui, caducaria el dia que alguien anada una linea
-- que es exactamente lo que le pasa a los documentos viejos de este
repo. Asi que se LEE del codigo: se buscan los literales "assets/..." en
`src/`, `include/` y `examples/juego.cpp`, y se queda con los que existen
en disco (los de las pruebas, "assets/maps/x.tmx", se caen solos). Si el
motor deja de cargar algo, deja de exportarse el mismo dia.

NO PUBLICA UN PAQUETE PARCIAL
-----------------------------
Se construye en `<destino>.parcial/` y solo se renombra a `<destino>`
cuando la carpeta esta completa. Si falta el runtime de la plataforma
pedida, o queda una referencia sin resolver, la carpeta se queda con su
sufijo `.parcial`, el diagnostico dentro y el programa sale con codigo
distinto de cero. Exportar a medias y decir que fue bien es peor que no
exportar.
"""
import argparse
import json
import os
import re
import shutil
import stat
import sys
import time

AQUI = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, AQUI)
from project_manifest import load_manifest  # noqa: E402  (contrato Project v1)

RAIZ_POR_DEFECTO = os.path.dirname(AQUI)

# Ficheros que se leen buscando referencias. Lo demas (png, ttf, binarios)
# son hojas del arbol: se copian y no se miran por dentro.
TEXTO = (".json", ".tmx", ".tsx", ".vert", ".frag", ".txt", ".md")

# Sin espacios a proposito: ningun asset del repo los lleva, y con
# espacios la expresion se tragaba la frase entera que venia detras de la
# ruta dentro de un texto en prosa ("races.json. raceId es la referencia
# estable al catalogo RPG"), y luego se declaraba rota una referencia que
# nadie habia escrito.
REF = re.compile(r'assets/[A-Za-z0-9_./\-]+')
TMX_SOURCE = re.compile(r'source="([^"]+)"')

# Extensiones que se consideran una referencia a un fichero. Lo que no
# esta aqui ("assets/catalogs" a secas, "assets/textures/foo.json's") es
# una mencion, no una carga.
EXTENSIONES = (".json", ".tmx", ".tsx", ".png", ".jpg", ".vert", ".frag",
               ".txt", ".md", ".ttf", ".wav", ".ogg", ".mp3")

# Firmas de ejecutable, para no entregar un binario de otra plataforma.
FIRMAS = {
    b"\x7fELF": "linux",
    b"MZ": "windows",
    b"\xcf\xfa\xed\xfe": "macos",
    b"\xca\xfe\xba\xbe": "macos",
}
BINARIO = {"linux": "juego", "macos": "juego", "windows": "juego.exe"}


def limpia(cita):
    """La ruta que hay dentro de una cita, sin la puntuacion de la frase.

    Dentro de los JSON de contenido hay prosa: «...usa assets/catalogs/
    races.json.» El punto final no es parte del nombre del fichero, y si
    no se quita pasan las dos cosas malas a la vez: el fichero real no se
    arrastra a la entrega, y ademas se declara rota una referencia que en
    realidad estaba bien escrita.
    """
    return cita.rstrip(" .,;:)]}\"'")


def carga(ruta):
    with open(ruta, encoding="utf-8") as f:
        return json.load(f)


def lee_texto(ruta):
    with open(ruta, encoding="utf-8", errors="replace") as f:
        return f.read()


def plataforma_del_binario(ruta):
    """linux / windows / macos, o None si no se reconoce."""
    try:
        with open(ruta, "rb") as f:
            cabecera = f.read(4)
    except OSError:
        return None
    for firma, nombre in FIRMAS.items():
        if cabecera.startswith(firma):
            return nombre
    return None


def rutas_literales_del_motor(raiz):
    """Los assets que el motor carga por ruta fija, leidos del codigo.

    Devuelve rutas relativas a `raiz` que existen en disco. Un prefijo de
    shader ("assets/shaders/sprite") se expande a sus dos ficheros.
    """
    encontradas = set()
    fuentes = [os.path.join(raiz, "src"), os.path.join(raiz, "include"),
               os.path.join(raiz, "examples", "juego.cpp")]
    for fuente in fuentes:
        ficheros = []
        if os.path.isfile(fuente):
            ficheros = [fuente]
        elif os.path.isdir(fuente):
            for base, _, nombres in os.walk(fuente):
                ficheros += [os.path.join(base, n) for n in nombres
                             if n.endswith((".cpp", ".h", ".hpp"))]
        for f in ficheros:
            for cruda in REF.findall(lee_texto(f)):
                encontradas.update(_expande(raiz, limpia(cruda)))
    return encontradas


def _expande(raiz, rel):
    """Una ruta del codigo -> las rutas reales que hay que copiar."""
    absoluta = os.path.join(raiz, rel)
    if os.path.isfile(absoluta):
        return {rel}
    if os.path.isdir(absoluta):
        # Una carpeta entera ("assets/catalogs") no se arrastra por si
        # aparezca en un mensaje de error: si el motor la necesita
        # completa, se dice con --incluir y queda escrito en export.json.
        return set()
    # Prefijo de shader: el motor pide "assets/shaders/sprite" y carga
    # sprite.vert + sprite.frag.
    salida = {rel + ext for ext in (".vert", ".frag")
              if os.path.isfile(absoluta + ext)}
    return salida


def referencias(raiz, rel):
    """Rutas `assets/...` que existen, citadas dentro de un fichero."""
    absoluta = os.path.join(raiz, rel)
    if not rel.endswith(TEXTO) or not os.path.isfile(absoluta):
        return set()
    texto = lee_texto(absoluta)
    salida = set()
    for cruda in REF.findall(texto):
        cruda = limpia(cruda)
        if os.path.isfile(os.path.join(raiz, cruda)):
            salida.add(cruda)
    if rel.endswith((".tmx", ".tsx")):
        # Los TMX citan su imagen en relativo desde su propia carpeta
        # ("../textures/terreno_mundo.png"). Sin esto, el mapa se copia y
        # el tileset se queda en el repo: el jugador ve el nivel vacio.
        carpeta = os.path.dirname(rel)
        for fuente in TMX_SOURCE.findall(texto):
            destino = os.path.normpath(os.path.join(carpeta, fuente))
            if os.path.isfile(os.path.join(raiz, destino)):
                salida.add(destino.replace(os.sep, "/"))
    return salida


def recoge(raiz, pid, incluir=()):
    """Todo lo que hay que copiar para que ese proyecto se juegue solo.

    Devuelve un informe: ficheros (cierre transitivo), lo declarado que
    falta, lo que se arrastro sin declarar y los objectId sin ficha.
    """
    ruta_manifiesto = os.path.join("assets", "proyectos", pid + ".json")
    if not os.path.isfile(os.path.join(raiz, ruta_manifiesto)):
        raise FileNotFoundError(f"no existe el proyecto '{pid}' "
                                f"({ruta_manifiesto})")
    m = load_manifest(os.path.join(raiz, ruta_manifiesto))
    dirs = {k: m["settings"][s] for k, s in
            (("levels", "levelsDirectory"), ("maps", "mapsDirectory"),
             ("adventures", "adventuresDirectory"),
             ("catalogs", "catalogsDirectory"))}

    declarado, faltan = {ruta_manifiesto}, []
    for clave, sub in dirs.items():
        for nombre in m[clave]:
            rel = f"assets/{sub}/{nombre}"
            (declarado.add(rel) if os.path.isfile(os.path.join(raiz, rel))
             else faltan.append(rel))
    if m["entry"]:
        if os.path.isfile(os.path.join(raiz, m["entry"])):
            declarado.add(m["entry"])
        else:
            faltan.append(m["entry"])

    motor = rutas_literales_del_motor(raiz)
    extra = set()
    for rel in incluir:
        rel = rel.strip("/")
        absoluta = os.path.join(raiz, rel)
        if os.path.isfile(absoluta):
            extra.add(rel)
        elif os.path.isdir(absoluta):
            for base, _, nombres in os.walk(absoluta):
                for n in nombres:
                    extra.add(os.path.relpath(os.path.join(base, n),
                                              raiz).replace(os.sep, "/"))
        else:
            faltan.append(rel)

    # Cierre transitivo: nadie declara la puerta que lleva al nivel de al
    # lado, ni el tileset del mapa. Se sigue hasta que no aparece nada.
    ficheros, arrastrados = set(), []
    pendientes = sorted(declarado | motor | extra)
    origen = {}
    while pendientes:
        rel = pendientes.pop()
        if rel in ficheros:
            continue
        ficheros.add(rel)
        for hijo in sorted(referencias(raiz, rel)):
            if hijo not in ficheros:
                pendientes.append(hijo)
                origen.setdefault(hijo, rel)
    for rel in sorted(ficheros - declarado - motor - extra):
        arrastrados.append({"arrastra": rel, "desde": origen.get(rel, "?")})

    return {"id": pid, "manifiesto": m, "dirs": dirs, "ficheros": ficheros,
            "declarado": declarado, "motor": motor, "extra": extra,
            "faltan": faltan, "arrastrados": arrastrados,
            "sin_ficha": sin_ficha(raiz, ficheros, dirs)}


def sin_ficha(raiz, ficheros, dirs):
    """objectId usados por los niveles que no tienen ficha DENTRO.

    Un catalogo que se queda fuera no da error al cargar: da un objeto
    invisible, que es peor (build_proyecto.py ya avisaba de esto).
    """
    catalogo = set()
    for rel in ficheros:
        if not rel.startswith(f"assets/{dirs['catalogs']}/"):
            continue
        try:
            d = carga(os.path.join(raiz, rel))
        except (ValueError, OSError):
            continue
        for o in (d.get("objects") or d.get("entries") or []):
            if isinstance(o, dict) and o.get("id"):
                catalogo.add(o["id"])
    huerfanos = set()
    for rel in sorted(ficheros):
        if not rel.startswith(f"assets/{dirs['levels']}/"):
            continue
        try:
            d = carga(os.path.join(raiz, rel))
        except (ValueError, OSError):
            continue
        for o in d.get("objects", []):
            oid = o.get("objectId")
            if oid and oid not in catalogo:
                huerfanos.add(oid)
    return sorted(huerfanos)


LANZADOR_SH = """#!/bin/sh
# Lanzador del juego exportado.
#
# Existe porque el motor busca sus assets en rutas relativas
# ("assets/levels/...", "assets/shaders/sprite"): el juego tiene que
# arrancar con ESTA carpeta como directorio de trabajo, no con la del
# terminal de quien lo lanza. Un doble clic desde el escritorio entra
# por aqui y funciona.
#
#   ./jugar.sh        juega
#   ./jugar.sh 60     corre 60 fotogramas y sale (humo, para CI)
cd "$(dirname "$0")" || exit 1
exec ./juego "$@"
"""

LANZADOR_BAT = """@echo off
rem Lanzador del juego exportado. El motor busca los assets en rutas
rem relativas, asi que hay que arrancar desde esta carpeta.
cd /d "%~dp0"
juego.exe %*
"""

LEEME = """{nombre}
{subrayado}

{descripcion}

Epoca: {epoca}
Autor: {autor}

COMO SE JUEGA

  {orden}

No hace falta el repositorio, ni compilar, ni herramientas: esta carpeta
lleva dentro el juego y solo los assets que usa. Se puede copiar, comprimir
y mandar entera.

QUE HAY AQUI

  {binario:<14} el juego
  assets/        {n_assets} ficheros: sus niveles, mapas, aventuras,
                 catalogos, shaders y texturas
  export.json    que hay dentro, de donde salio y por donde se entra

Generado por tools/exportar.py (paquete G-06) el {fecha}.
"""


def ajusta_indice_de_proyectos(destino, pid):
    """En una entrega solo hay un proyecto, y el indice tiene que decirlo.

    `assets/proyectos/index.json` es la lista de proyectos vivos del
    REPOSITORIO: seis. El motor la lee para montar la pantalla de titulo
    (G-08). Copiada tal cual a una entrega, el jugador veria cinco
    escenarios cuyo manifiesto no esta dentro, y cinco formas de que el
    juego no encuentre lo que promete. Se reescribe con el unico proyecto
    que va dentro; el indice del repositorio no se toca.
    """
    rel = "assets/proyectos/index.json"
    ruta = os.path.join(destino, rel)
    if not os.path.isfile(ruta):
        return []
    try:
        d = carga(ruta)
    except (ValueError, OSError):
        return []
    if d.get("proyectos") == [pid]:
        return []
    with open(ruta, "w", encoding="utf-8") as f:
        json.dump({"_nota": "Indice de una ENTREGA: aqui solo vive el "
                            "proyecto exportado. El del repositorio lista "
                            "todos y no se ha tocado.",
                   "proyectos": [pid]}, f, ensure_ascii=False, indent=1)
    return [f"{rel}: reducido a ['{pid}'] "
            f"(en el repositorio son {len(d.get('proyectos', []))})"]


def escribe_lanzador(destino, plataforma):
    if plataforma == "windows":
        ruta = os.path.join(destino, "jugar.bat")
        with open(ruta, "w", encoding="utf-8", newline="\r\n") as f:
            f.write(LANZADOR_BAT)
        return "jugar.bat"
    ruta = os.path.join(destino, "jugar.sh")
    with open(ruta, "w", encoding="utf-8") as f:
        f.write(LANZADOR_SH)
    os.chmod(ruta, os.stat(ruta).st_mode | stat.S_IXUSR | stat.S_IXGRP
             | stat.S_IXOTH)
    return "jugar.sh"


def comprueba(destino, raiz=None):
    """Lo que hace que la carpeta valga: que no le falte nada de fuera.

    Tres preguntas, las tres respondidas mirando la carpeta entregada y
    no el repositorio:
      1. toda ruta `assets/...` citada dentro resuelve DENTRO;
      2. ningun fichero de texto lleva escrita una ruta del repositorio;
      3. el sitio por donde se entra existe.
    """
    problemas = []
    dentro = set()
    for base, _, nombres in os.walk(destino):
        for n in nombres:
            dentro.add(os.path.relpath(os.path.join(base, n),
                                       destino).replace(os.sep, "/"))
    raiz_abs = os.path.abspath(raiz) if raiz else None
    for rel in sorted(dentro):
        if not rel.endswith(TEXTO):
            continue
        texto = lee_texto(os.path.join(destino, rel))
        for cruda in set(REF.findall(texto)):
            cruda = limpia(cruda)
            # Solo se exige lo que parece un fichero: "assets/catalogs" a
            # secas, en un mensaje, no es una referencia rota.
            if not cruda.endswith(EXTENSIONES) or cruda in dentro:
                continue
            if raiz and not os.path.isfile(os.path.join(raiz, cruda)):
                # Cita muerta: no esta dentro, pero es que no existe en
                # ninguna parte. Es deuda de contenido anterior a esta
                # exportacion, no algo que la entrega se haya dejado.
                continue
            problemas.append(f"{rel}: cita {cruda}, que no esta dentro")
        if raiz_abs and raiz_abs in texto:
            problemas.append(f"{rel}: lleva escrita la ruta del repositorio")
        if rel.endswith((".tmx", ".tsx")):
            carpeta = os.path.dirname(rel)
            for fuente in set(TMX_SOURCE.findall(texto)):
                objetivo = os.path.normpath(
                    os.path.join(carpeta, fuente)).replace(os.sep, "/")
                if objetivo not in dentro:
                    problemas.append(f"{rel}: su tileset {fuente} no esta dentro")
    return sorted(set(problemas))


def exporta(raiz, pid, destino, plataforma="auto", binario=None,
            incluir=(), solo_assets=False, forzar=False, silencioso=False):
    """Deja en `destino` una carpeta jugable. Devuelve el informe."""
    def di(*a):
        if not silencioso:
            print(*a)

    plan = recoge(raiz, pid, incluir)
    m = plan["manifiesto"]

    origen_binario, plataforma_real = None, None
    if not solo_assets:
        candidatos = [binario] if binario else [
            os.path.join(raiz, "build", "juego"),
            os.path.join(raiz, "out", "build", "macos-release", "juego"),
            os.path.join(raiz, "out", "build", "macos-debug", "juego"),
            os.path.join(raiz, "juego"),
            os.path.join(raiz, "build", "juego.exe")]
        for c in candidatos:
            if c and os.path.isfile(c):
                origen_binario = c
                plataforma_real = plataforma_del_binario(c)
                break

    if plataforma == "auto":
        plataforma = plataforma_real or ("windows" if os.name == "nt"
                                         else sys.platform.replace(
                                             "darwin", "macos").replace(
                                             "linux2", "linux"))

    parcial = destino + ".parcial"
    for ruta in (parcial, destino):
        if os.path.isdir(ruta):
            if ruta == destino and not forzar and not os.path.isfile(
                    os.path.join(ruta, "export.json")):
                raise SystemExit(
                    f"{destino} ya existe y no es una exportacion mia. "
                    "Borrala tu o pasa --forzar; no voy a tirar una carpeta "
                    "de la que no se que hay dentro.")
            shutil.rmtree(ruta)
    os.makedirs(parcial)

    for rel in sorted(plan["ficheros"]):
        salida = os.path.join(parcial, rel)
        os.makedirs(os.path.dirname(salida), exist_ok=True)
        shutil.copy2(os.path.join(raiz, rel), salida)

    nombre_binario = BINARIO.get(plataforma, "juego")
    motivos = []
    if solo_assets:
        motivos.append("exportado --solo-assets: la carpeta no lleva runtime")
    elif not origen_binario:
        motivos.append("no hay ningun binario del juego: compila primero "
                       "(cmake --build) o pasa --binario")
    elif plataforma_real != plataforma:
        tiene = plataforma_real or "formato desconocido"
        motivos.append(
            f"el binario encontrado es de {tiene} y se pidio {plataforma}: "
            "este equipo no puede producir esa entrega. Compilalo en esa "
            "plataforma y pasalo con --binario")
    else:
        salida = os.path.join(parcial, nombre_binario)
        shutil.copy2(origen_binario, salida)
        os.chmod(salida, os.stat(salida).st_mode | stat.S_IXUSR
                 | stat.S_IXGRP | stat.S_IXOTH)

    ajustes = ajusta_indice_de_proyectos(parcial, pid)
    lanzador = escribe_lanzador(parcial, plataforma)
    if plan["faltan"]:
        motivos.append(f"{len(plan['faltan'])} ficheros declarados no existen")
    if plan["sin_ficha"]:
        motivos.append(f"{len(plan['sin_ficha'])} objectId sin ficha dentro")
    if not m["entry"]:
        motivos.append("el manifiesto no dice por donde se entra (entry)")
    rotas = comprueba(parcial, raiz)
    if rotas:
        motivos.append(f"{len(rotas)} referencias no resuelven dentro")

    jugable = not motivos
    n_assets = sum(1 for r in plan["ficheros"] if r.startswith("assets/"))
    informe = {
        "_nota": "Exportacion de un proyecto. Generada por tools/exportar.py "
                 "(G-06). Esta carpeta no necesita el repositorio.",
        "schemaVersion": 1,
        "id": pid,
        "nombre": m["name"],
        "descripcion": m["description"],
        "epoca": m["epoch"],
        "autor": m["author"],
        "entrada": m["entry"],
        "plataforma": plataforma,
        "ejecutable": nombre_binario if origen_binario and not motivos else None,
        "lanzador": lanzador,
        "generado": time.strftime("%Y-%m-%d"),
        "contenido": {
            "ficheros": len(plan["ficheros"]),
            "declarados": len(plan["declarado"]),
            "del_motor": len(plan["motor"]),
            "incluidos_a_mano": len(plan["extra"]),
            "arrastrados": len(plan["arrastrados"]),
        },
        "ajustes": ajustes,
        "arrastrados": plan["arrastrados"][:50],
        "ficheros_declarados_que_faltan": plan["faltan"],
        "objectIds_sin_ficha": plan["sin_ficha"],
        "referencias_rotas": rotas[:50],
        "jugable": jugable,
        "motivos": motivos,
    }
    with open(os.path.join(parcial, "export.json"), "w", encoding="utf-8") as f:
        json.dump(informe, f, ensure_ascii=False, indent=1)

    orden = (f".\\{lanzador}" if plataforma == "windows" else f"./{lanzador}")
    with open(os.path.join(parcial, "LEEME.txt"), "w", encoding="utf-8") as f:
        f.write(LEEME.format(
            nombre=m["name"], subrayado="=" * len(m["name"]),
            descripcion=m["description"], epoca=m["epoch"], autor=m["author"],
            orden=orden, binario=nombre_binario, n_assets=n_assets,
            fecha=informe["generado"]))

    if jugable:
        os.rename(parcial, destino)
        informe["carpeta"] = destino
    else:
        informe["carpeta"] = parcial

    di(f"  {pid:13} {len(plan['ficheros']):4} ficheros -> "
       f"{informe['carpeta']}{os.sep}")
    di(f"                declarados {len(plan['declarado']):3} · "
       f"del motor {len(plan['motor']):2} · "
       f"arrastrados {len(plan['arrastrados']):3}")
    for a in plan["arrastrados"][:3]:
        di(f"                arrastrado: {a['arrastra']}  (desde {a['desde']})")
    for a in ajustes:
        di(f"                ajustado: {a}")
    if jugable:
        di(f"                JUGABLE: {orden}")
    else:
        di("                NO SE PUBLICA, queda en .parcial/:")
        for motivo in motivos:
            di(f"                  - {motivo}")
    return informe


def main(argv=None):
    ap = argparse.ArgumentParser(
        description="Exporta un proyecto a una carpeta jugable sin el repo.")
    ap.add_argument("proyecto", help="id del proyecto (assets/proyectos/<id>.json)")
    ap.add_argument("--destino", help="carpeta de salida "
                                      "(por defecto: entregas/<id>)")
    ap.add_argument("--plataforma", default="auto",
                    choices=("auto", "linux", "windows", "macos"))
    ap.add_argument("--binario", help="ruta del ejecutable ya compilado "
                                      "para esa plataforma")
    ap.add_argument("--incluir", action="append", default=[],
                    help="fichero o carpeta extra (se puede repetir)")
    ap.add_argument("--solo-assets", action="store_true",
                    help="sin runtime; la carpeta NO se marca jugable")
    ap.add_argument("--forzar", action="store_true",
                    help="sobrescribe el destino aunque no sea mio")
    ap.add_argument("--raiz", default=RAIZ_POR_DEFECTO)
    a = ap.parse_args(argv)

    destino = a.destino or os.path.join(a.raiz, "entregas", a.proyecto)
    try:
        informe = exporta(a.raiz, a.proyecto, os.path.abspath(destino),
                          a.plataforma, a.binario, a.incluir,
                          a.solo_assets, a.forzar)
    except (FileNotFoundError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    return 0 if informe["jugable"] else 1


if __name__ == "__main__":
    sys.exit(main())
