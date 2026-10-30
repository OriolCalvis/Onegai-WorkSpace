#!/usr/bin/env python3
"""Fábrica de software de Software/: validadores automáticos, registro total y entrega con pruebas.

Una sola herramienta para todos los agentes (Claude, Codex, Qwen, Antigravity…). Solo usa la
biblioteca estándar y funciona igual en Mac, Windows y Linux.

Órdenes (todas escriben en out/fabrica/<ENCARGO>/ y dejan rastro en fabrica.log):

  inicio   <ENCARGO>      Línea base: hash, rama, entorno y todos los checks ANTES de tocar nada.
  check    [--fase F]     Ejecuta los checks (rapida | completa). Separa fallos nuevos de heredados.
  vigilar  [--cada S]     Fase de checks continuada: re-ejecuta la fase rápida cada vez que cambia
                          un fichero de la zona. --max N para acabar (los agentes siempre con --max).
  depurar  <CHECK>        Repite un check con su orden de depuración, extrae el primer error con
                          contexto y cruza con lo que has cambiado desde la línea base.
  entregar [--sin-clon]   Fase completa desde un clon limpio del commit actual, EVIDENCIA.md,
                          zip con todos los registros y huellas sha256. Sale con 1 si hay fallos nuevos.

Disco: toda fase que compila (checks con "necesita_disco") y todo clon exigen "disco_minimo_gb"
libres (fabrica.json; FABRICA_DISCO_MIN_GB lo cambia). Si no los hay, la orden no empieza y sale
con 3: BLOQUEADA por disco, no ROJO.
  estado                  Qué encargo está activo y cómo fue la última ejecución.

El encargo activo se guarda en out/fabrica/ACTUAL; --encargo lo cambia en cualquier orden.
Configuración: fabrica.json (al lado de este fichero) o --config.
"""

from __future__ import annotations

import argparse
import datetime as dt
import fnmatch
import hashlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import time
import zipfile
from pathlib import Path

AQUI = Path(__file__).resolve().parent
VERSION = "1.1"

# Git en iCloud: sin esto, git lee ficheros no descargados y peta con «Bus error».
GIT = "git -c core.trustctime=false -c core.checkStat=minimal"

# Código de salida cuando falta disco: no es un fallo del encargo, es un bloqueo.
SALIDA_SIN_DISCO = 3


# ----------------------------------------------------------------------------- utilidades

def ahora() -> str:
    return dt.datetime.now().astimezone().isoformat(timespec="seconds")


def sello() -> str:
    return dt.datetime.now().strftime("%Y%m%d-%H%M%S")


def sh(cmd: str, cwd: Path, timeout: int | None = None, env: dict | None = None) -> tuple[int, str, float]:
    """Ejecuta una orden de shell y devuelve (código, salida combinada, segundos)."""
    t0 = time.monotonic()
    e = dict(os.environ)
    if env:
        e.update(env)
    try:
        p = subprocess.run(cmd, shell=True, cwd=str(cwd), stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           timeout=timeout, env=e)
        salida = p.stdout.decode("utf-8", errors="replace")
        codigo = p.returncode
    except subprocess.TimeoutExpired as ex:
        salida = (ex.stdout or b"").decode("utf-8", errors="replace") + f"\n[fabrica] TIEMPO AGOTADO tras {timeout} s\n"
        codigo = 124
    return codigo, salida, time.monotonic() - t0


def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for bloque in iter(lambda: f.read(1 << 16), b""):
            h.update(bloque)
    return h.hexdigest()


def git(raiz: Path, args: str) -> str:
    c, s, _ = sh(f"{GIT} {args}", raiz, timeout=60)
    return s.strip() if c == 0 else ""


def plataforma() -> str:
    return {"darwin": "darwin", "win32": "win32"}.get(sys.platform, "linux")


# ----------------------------------------------------------------------------- configuración

class Fabrica:
    def __init__(self, cfg_path: Path, raiz: Path | None):
        self.cfg_path = cfg_path
        self.cfg = json.loads(cfg_path.read_text(encoding="utf-8"))
        self.raiz = raiz or self._busca_raiz()
        self.dir_registros = self.raiz / self.cfg.get("registros", "out/fabrica")
        self.dir_registros.mkdir(parents=True, exist_ok=True)
        self.vars = {"preset": self.cfg.get("preset", {}).get(plataforma(), ""),
                     "python": sys.executable if " " not in sys.executable else f'"{sys.executable}"'}
        self.vars.update(self.cfg.get("variables", {}))

    def _busca_raiz(self) -> Path:
        r = git(Path.cwd(), "rev-parse --show-toplevel")
        return Path(r) if r else Path.cwd()

    # --- encargo activo
    def encargo(self, nombre: str | None) -> str:
        actual = self.dir_registros / "ACTUAL"
        if nombre:
            actual.write_text(nombre, encoding="utf-8")
            return nombre
        if actual.exists():
            return actual.read_text(encoding="utf-8").strip()
        sys.exit("[fabrica] No hay encargo activo. Empieza con: fabrica.py inicio <ENCARGO>")

    def dir_encargo(self, encargo: str) -> Path:
        d = self.dir_registros / encargo
        d.mkdir(parents=True, exist_ok=True)
        return d

    def log(self, encargo: str, evento: str, **datos):
        """Registro total: una línea JSON por cada cosa que hace la fábrica."""
        linea = {"t": ahora(), "encargo": encargo, "evento": evento, **datos}
        with (self.dir_encargo(encargo) / "fabrica.log").open("a", encoding="utf-8") as f:
            f.write(json.dumps(linea, ensure_ascii=False) + "\n")

    def checks(self, fase: str) -> list[dict]:
        orden = {"rapida": 0, "completa": 1}
        nivel = orden[fase]
        out = []
        for c in self.cfg["checks"]:
            if orden.get(c.get("fase", "rapida"), 0) > nivel:
                continue
            solo = c.get("solo_en")
            if solo and plataforma() not in solo:
                continue
            out.append(c)
        return out

    def orden(self, c: dict, clave: str = "cmd") -> str:
        v = c.get(clave, "")
        if isinstance(v, dict):
            v = v.get(plataforma(), v.get("default", ""))
        return v.format(**self.vars)


# ----------------------------------------------------------------------------- disco

def libres_gb(ruta: Path) -> float:
    r = ruta
    while not r.exists() and r != r.parent:
        r = r.parent
    return shutil.disk_usage(str(r)).free / 1e9


def disco_minimo(fab: "Fabrica") -> float:
    v = os.environ.get("FABRICA_DISCO_MIN_GB")
    return float(v) if v else float(fab.cfg.get("disco_minimo_gb", 0))


def exige_disco(fab: "Fabrica", encargo: str, para: str, rutas: list[Path]) -> None:
    """Corta la orden antes de empezar si alguna ruta se queda por debajo del mínimo."""
    minimo = disco_minimo(fab)
    if minimo <= 0:
        return
    for ruta in rutas:
        gb = libres_gb(ruta)
        if gb < minimo:
            fab.log(encargo, "bloqueada_disco", para=para, ruta=str(ruta), libres_gb=round(gb, 2), minimo_gb=minimo)
            print(f"[fabrica] BLOQUEADA por disco: {gb:.2f} GB libres en {ruta}, hacen falta {minimo:g} GB "
                  f"para {para}. No se ha ejecutado nada.")
            sys.exit(SALIDA_SIN_DISCO)


def fase_compila(fab: "Fabrica", fase: str) -> bool:
    return any(c.get("necesita_disco") for c in fab.checks(fase))


# ----------------------------------------------------------------------------- un check

def analiza(c: dict, codigo: int, salida: str) -> dict:
    """Saca de la salida los fallos con nombre (items) y los contadores de errores y avisos."""
    r: dict = {"codigo": codigo}
    for clave in ("errores", "avisos"):
        rx = c.get(f"{clave}_regex")
        if rx:
            m = re.findall(rx, salida, flags=re.MULTILINE)
            # con grupo numérico: el ÚLTIMO número (el resumen final); sin grupo: cuántas líneas casan
            r[clave] = int(m[-1]) if m and all(str(x).isdigit() for x in m) else len(m)
    rx = c.get("items_regex")
    items = sorted(set(m.strip() for m in re.findall(rx, salida, flags=re.MULTILINE))) if rx else []
    if codigo != 0 and not items:
        items = [f"{c['id']}: código {codigo}"]
    r["items"] = items
    return r


def ejecuta_fase(fab: Fabrica, encargo: str, fase: str, raiz: Path, etiqueta: str) -> dict:
    if fase_compila(fab, fase):
        exige_disco(fab, encargo, f"la fase {fase} (compila)", [raiz])
    d = fab.dir_encargo(encargo) / "ejecuciones" / f"{sello()}_{etiqueta}"
    d.mkdir(parents=True, exist_ok=True)
    resultado = {"t": ahora(), "fase": fase, "etiqueta": etiqueta, "raiz": str(raiz),
                 "commit": git(raiz, "rev-parse HEAD"), "checks": {}}
    for c in fab.checks(fase):
        cmd = fab.orden(c)
        cwd = raiz / c.get("cwd", ".").format(**fab.vars)
        if not cwd.exists():
            r = {"codigo": 127, "items": [f"{c['id']}: no existe {c.get('cwd')}"], "segundos": 0.0, "cmd": cmd}
            resultado["checks"][c["id"]] = r
            fab.log(encargo, "check", id=c["id"], **r)
            continue
        print(f"[fabrica] {c['id']:<22} … ", end="", flush=True)
        codigo, salida, seg = sh(cmd, cwd, timeout=c.get("timeout", 1800))
        (d / f"{c['id']}.log").write_text(f"$ {cmd}\n(cwd {cwd})\n\n{salida}\n[código {codigo} · {seg:.1f} s]\n",
                                          encoding="utf-8")
        r = analiza(c, codigo, salida)
        r.update(segundos=round(seg, 1), cmd=cmd, log=str((d / f"{c['id']}.log").relative_to(fab.dir_registros)))
        resultado["checks"][c["id"]] = r
        print("ok" if codigo == 0 else f"FALLA ({len(r['items'])})", f"{seg:.1f} s")
        fab.log(encargo, "check", id=c["id"], codigo=codigo, segundos=r["segundos"], items=r["items"],
                errores=r.get("errores"), avisos=r.get("avisos"))
        if codigo != 0 and c.get("corta", False):
            print(f"[fabrica] {c['id']} corta la fase: los siguientes no tienen sentido sin él.")
            break
    (d / "resumen.json").write_text(json.dumps(resultado, ensure_ascii=False, indent=2), encoding="utf-8")
    resultado["dir"] = str(d)
    return resultado


def compara(base: dict | None, actual: dict) -> dict:
    """Fallo nuevo = item que no estaba en la línea base, o contador de errores/avisos que sube."""
    nuevos, heredados, arreglados = [], [], []
    bchecks = (base or {}).get("checks", {})
    for cid, r in actual["checks"].items():
        b = bchecks.get(cid, {"items": [], "codigo": 0})
        bi, ai = set(b.get("items", [])), set(r.get("items", []))
        nuevos += [f"{cid} · {x}" for x in sorted(ai - bi)]
        heredados += [f"{cid} · {x}" for x in sorted(ai & bi)]
        arreglados += [f"{cid} · {x}" for x in sorted(bi - ai)]
        for clave in ("errores", "avisos"):
            if r.get(clave) is not None and b.get(clave) is not None and r[clave] > b[clave]:
                nuevos.append(f"{cid} · {clave}: {b[clave]} → {r[clave]}")
    return {"nuevos": nuevos, "heredados": heredados, "arreglados": arreglados}


def linea_base(fab: Fabrica, encargo: str) -> dict | None:
    p = fab.dir_encargo(encargo) / "linea_base.json"
    return json.loads(p.read_text(encoding="utf-8")) if p.exists() else None


def imprime_veredicto(cmp: dict) -> None:
    for x in cmp["nuevos"]:
        print(f"  ✗ NUEVO     {x}")
    for x in cmp["arreglados"]:
        print(f"  ✓ ARREGLADO {x}")
    if cmp["heredados"]:
        print(f"  · {len(cmp['heredados'])} heredados (ya fallaban en la línea base)")
    print("[fabrica] VERDE: ningún fallo nuevo." if not cmp["nuevos"] else
          f"[fabrica] ROJO: {len(cmp['nuevos'])} fallo(s) nuevo(s). Siguiente paso: fabrica.py depurar <check>")


# ----------------------------------------------------------------------------- órdenes

def cmd_inicio(fab: Fabrica, a) -> int:
    enc = fab.encargo(a.encargo_pos or a.encargo)
    d = fab.dir_encargo(enc)
    fase = "rapida" if a.rapida else "completa"
    if fase_compila(fab, fase):  # antes de escribir nada: una línea base a medias engaña más que ninguna
        exige_disco(fab, enc, "la línea base completa (compila); con --rapida no compila", [fab.raiz])
    entorno = {
        "t": ahora(), "fabrica": VERSION, "plataforma": platform.platform(), "python": sys.version.split()[0],
        "raiz": str(fab.raiz), "rama": git(fab.raiz, "rev-parse --abbrev-ref HEAD"),
        "commit": git(fab.raiz, "rev-parse HEAD"), "sucio": git(fab.raiz, "status --porcelain").splitlines(),
        "herramientas": {},
    }
    for h, v in fab.cfg.get("versiones", {}).items():
        c, s, _ = sh(v, fab.raiz, timeout=30)
        entorno["herramientas"][h] = s.strip().splitlines()[0] if c == 0 and s.strip() else "no disponible"
    (d / "entorno.json").write_text(json.dumps(entorno, ensure_ascii=False, indent=2), encoding="utf-8")
    fab.log(enc, "inicio", rama=entorno["rama"], commit=entorno["commit"], sucio=len(entorno["sucio"]))
    print(f"[fabrica] Encargo {enc} · rama {entorno['rama']} · {entorno['commit'][:10]}")
    if entorno["sucio"]:
        print(f"[fabrica] Aviso: {len(entorno['sucio'])} ficheros sin commitear entran en la línea base.")
    r = ejecuta_fase(fab, enc, fase, fab.raiz, f"base_{fase}")
    (d / "linea_base.json").write_text(json.dumps(r, ensure_ascii=False, indent=2), encoding="utf-8")
    fallos = sum(len(x["items"]) for x in r["checks"].values())
    print(f"[fabrica] Línea base guardada: {fallos} fallo(s) heredado(s). Lo que ya falla no te bloqueará; "
          "lo nuevo, sí.")
    return 0


def cmd_check(fab: Fabrica, a) -> int:
    enc = fab.encargo(a.encargo)
    r = ejecuta_fase(fab, enc, a.fase, fab.raiz, a.fase)
    cmp = compara(linea_base(fab, enc), r)
    fab.log(enc, "veredicto", fase=a.fase, nuevos=cmp["nuevos"], arreglados=cmp["arreglados"],
            heredados=len(cmp["heredados"]))
    (fab.dir_encargo(enc) / "ultimo.json").write_text(json.dumps({**r, "comparacion": cmp}, ensure_ascii=False,
                                                                 indent=2), encoding="utf-8")
    imprime_veredicto(cmp)
    return 1 if cmp["nuevos"] else 0


def huella_zona(fab: Fabrica) -> dict:
    """mtime y tamaño de cada fichero vigilado (zona del encargo o, si no hay, las globs de la config)."""
    globs = fab.cfg.get("vigilar", ["**/*"])
    ignora = fab.cfg.get("ignorar", []) + [str(fab.dir_registros.relative_to(fab.raiz)) + "/*", ".git/*"]
    h = {}
    for root, dirs, files in os.walk(fab.raiz):
        rel_root = os.path.relpath(root, fab.raiz).replace("\\", "/")
        dirs[:] = [x for x in dirs if not any(fnmatch.fnmatch(f"{rel_root}/{x}/".lstrip("./"), g) or
                                              fnmatch.fnmatch(x, g.rstrip("/*")) for g in ignora)]
        for f in files:
            rel = f"{rel_root}/{f}".lstrip("./") if rel_root != "." else f
            if any(fnmatch.fnmatch(rel, g) for g in ignora):
                continue
            if any(fnmatch.fnmatch(rel, g) for g in globs):
                st = os.stat(os.path.join(root, f))
                h[rel] = (st.st_mtime_ns, st.st_size)
    return h


def cmd_vigilar(fab: Fabrica, a) -> int:
    enc = fab.encargo(a.encargo)
    fab.log(enc, "vigilar_inicio", cada=a.cada, max=a.max)
    print(f"[fabrica] Vigilando cada {a.cada} s (fase {a.fase}). Ctrl+C para parar.")
    previa = huella_zona(fab)
    ejec, ultimo_rojo = 0, 0
    try:
        primera = True
        while a.max == 0 or ejec < a.max:
            actual = huella_zona(fab)
            cambiados = sorted(k for k in set(actual) | set(previa) if actual.get(k) != previa.get(k))
            if cambiados or primera:
                primera = False
                if cambiados:
                    print(f"[fabrica] Cambian {len(cambiados)}: {', '.join(cambiados[:5])}{' …' if len(cambiados) > 5 else ''}")
                fab.log(enc, "vigilar_cambio", ficheros=cambiados[:50])
                r = ejecuta_fase(fab, enc, a.fase, fab.raiz, f"vigilar_{a.fase}")
                cmp = compara(linea_base(fab, enc), r)
                fab.log(enc, "veredicto", fase=a.fase, nuevos=cmp["nuevos"], arreglados=cmp["arreglados"],
                        heredados=len(cmp["heredados"]), origen="vigilar")
                imprime_veredicto(cmp)
                ultimo_rojo = 1 if cmp["nuevos"] else 0
                ejec += 1
                previa = huella_zona(fab)
            if a.max and ejec >= a.max:
                break
            time.sleep(a.cada)
    except KeyboardInterrupt:
        pass
    fab.log(enc, "vigilar_fin", ejecuciones=ejec)
    return ultimo_rojo


def primer_error(salida: str, patrones: list[str], contexto: int = 6, preferir: list[str] | None = None) -> str:
    """Primera línea de error; si hay fallos nuevos, la primera que nombra uno de ellos."""
    lineas = salida.splitlines()
    candidatas = [i for i, l in enumerate(lineas) if any(re.search(p, l) for p in patrones)]
    if preferir:
        propias = [i for i in candidatas if any(x in lineas[i] for x in preferir)]
        candidatas = propias or candidatas
    for i in candidatas[:1]:
        l = lineas[i]
        if True:
            a, b = max(0, i - contexto), min(len(lineas), i + contexto + 1)
            return "\n".join(f"{'>>' if j == i else '  '} {lineas[j]}" for j in range(a, b))
    return "\n".join(lineas[-25:])


def cmd_depurar(fab: Fabrica, a) -> int:
    enc = fab.encargo(a.encargo)
    c = next((x for x in fab.cfg["checks"] if x["id"] == a.check), None)
    if not c:
        sys.exit(f"[fabrica] No existe el check {a.check}. Hay: {', '.join(x['id'] for x in fab.cfg['checks'])}")
    cmd = fab.orden(c, "depurar") or fab.orden(c)
    codigo, salida, seg = sh(cmd, fab.raiz / c.get("cwd", ".").format(**fab.vars), timeout=c.get("timeout", 1800))
    d = fab.dir_encargo(enc) / "depuracion"
    d.mkdir(exist_ok=True)
    base = linea_base(fab, enc) or {}
    commit_base = base.get("commit", "")
    cambios = git(fab.raiz, f"diff --stat {commit_base}") if commit_base else git(fab.raiz, "diff --stat")
    nuevos_fich = git(fab.raiz, "ls-files --others --exclude-standard")
    r = analiza(c, codigo, salida)
    items_base = set(base.get("checks", {}).get(a.check, {}).get("items", []))
    nuevos = [x for x in r["items"] if x not in items_base]
    patrones = fab.cfg.get("patrones_error", [r"error", r"FAILED", r"Traceback", r"Assertion", r"fail"])
    informe = [
        f"# Depuración de `{a.check}` — {enc}", "",
        f"- Cuándo: {ahora()} · código {codigo} · {seg:.1f} s",
        f"- Orden: `{cmd}`",
        f"- Línea base: `{commit_base[:10] or 'sin línea base'}` → ahora `{git(fab.raiz, 'rev-parse HEAD')[:10]}`", "",
        "## Fallos con nombre", "",
        *([f"- {'**NUEVO**' if x in nuevos else 'heredado'} · {x}" for x in r["items"]] or ["- ninguno"]), "",
        "## Primer error" + (" de los nuevos" if nuevos else "") + ", con contexto", "", "```",
        primer_error(salida, patrones, preferir=nuevos), "```", "",
        "## Lo que has cambiado desde la línea base", "", "```", cambios or "(nada commiteado ni modificado)",
        *(["", "Sin seguimiento:", nuevos_fich] if nuevos_fich else []), "```", "",
        "## Cómo seguir", "",
        "1. Lee el primer error: suele ser la causa; los demás, consecuencia.",
        "2. Cruza el fichero del error con la lista de cambios. Si no lo has tocado, el fallo puede ser heredado:",
        "   compáralo con `linea_base.json` antes de arreglar nada ajeno.",
        "3. Reproduce con la orden de arriba, arregla, y `fabrica.py check` hasta VERDE.",
        "4. Si no es tuyo ni de tu zona, no lo arregles: apúntalo en el encargo (AGENTS.md §2.4).",
    ]
    p = d / f"{sello()}_{a.check}.md"
    p.write_text("\n".join(informe) + "\n", encoding="utf-8")
    (d / f"{p.stem}.log").write_text(salida, encoding="utf-8")
    fab.log(enc, "depurar", id=a.check, codigo=codigo, informe=str(p.relative_to(fab.dir_registros)))
    print("\n".join(informe[:-6]))
    print(f"\n[fabrica] Informe completo: {p}")
    return 0 if codigo == 0 else 1


def clon_limpio(fab: Fabrica, destino: Path) -> Path:
    commit = git(fab.raiz, "rev-parse HEAD")
    if not commit:
        sys.exit("[fabrica] La raíz no es un repo git: no se puede entregar desde clon limpio (usa --sin-clon).")
    # --shared: el clon toma los objetos del repo original en vez de copiarlos (.git pesa ~2 GB);
    # --no-checkout + checkout del commit: un solo árbol, el del commit, no el de la rama por defecto.
    c, s, _ = sh(f'{GIT} clone --quiet --shared --no-checkout "{fab.raiz}" "{destino}"', fab.raiz.parent,
                 timeout=900)
    if c != 0:
        sys.exit(f"[fabrica] No se pudo clonar:\n{s}")
    c, s, _ = sh(f"{GIT} checkout --quiet --detach {commit}", destino, timeout=900)
    if c != 0:
        sys.exit(f"[fabrica] No se pudo sacar {commit[:10]} en el clon:\n{s}")
    for extra in fab.cfg.get("clon_copiar", []):  # lo que no está en git pero hace falta (assets grandes…)
        o = fab.raiz / extra
        if o.is_dir():
            shutil.copytree(o, destino / extra, dirs_exist_ok=True)
        elif o.exists():
            (destino / extra).parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(o, destino / extra)
    return destino


def cmd_entregar(fab: Fabrica, a) -> int:
    enc = fab.encargo(a.encargo)
    d = fab.dir_encargo(enc)
    sucio = git(fab.raiz, "status --porcelain").splitlines()
    sucio = [x for x in sucio if fab.cfg.get("registros", "out/fabrica") not in x]
    fab.log(enc, "entregar_inicio", sin_clon=a.sin_clon, sucio=len(sucio))
    if sucio and not a.sin_clon:
        print(f"[fabrica] Aviso: {len(sucio)} fichero(s) sin commitear NO entran en el clon limpio:")
        for x in sucio[:10]:
            print(f"    {x}")
    tmp = None
    if a.sin_clon:
        raiz = fab.raiz
        if fase_compila(fab, "completa"):
            exige_disco(fab, enc, "la entrega (compila)", [fab.raiz])
    else:
        exige_disco(fab, enc, "el clon limpio de la entrega", [Path(tempfile.gettempdir())])
    try:
        if not a.sin_clon:
            tmp = Path(tempfile.mkdtemp(prefix=f"fabrica_{enc}_"))
            raiz = clon_limpio(fab, tmp / "clon")
            print(f"[fabrica] Clon limpio en {raiz}")
        r = ejecuta_fase(fab, enc, "completa", raiz, "entrega")
    finally:
        if tmp and not a.conservar_clon:
            shutil.rmtree(tmp, ignore_errors=True)
    cmp = compara(linea_base(fab, enc), r)
    fab.log(enc, "veredicto", fase="completa", origen="entrega", nuevos=cmp["nuevos"],
            arreglados=cmp["arreglados"], heredados=len(cmp["heredados"]))
    base = linea_base(fab, enc) or {}
    entorno = json.loads((d / "entorno.json").read_text(encoding="utf-8")) if (d / "entorno.json").exists() else {}
    commit_base = base.get("commit", "")
    diff = git(fab.raiz, f"diff --stat {commit_base} HEAD") if commit_base else ""
    log_commits = git(fab.raiz, f"log --oneline {commit_base}..HEAD") if commit_base else ""
    veredicto = "VERDE" if not cmp["nuevos"] else "ROJO"

    filas = []
    bchecks = base.get("checks", {})
    for cid, x in r["checks"].items():
        b = bchecks.get(cid, {})
        estado = "ok" if x["codigo"] == 0 else ("heredado" if set(x["items"]) <= set(b.get("items", [])) else "**NUEVO**")
        cuenta = " · ".join(f"{k} {b.get(k, '–')}→{x[k]}" for k in ("errores", "avisos") if x.get(k) is not None)
        filas.append(f"| `{cid}` | {estado} | {x['codigo']} | {x['segundos']} s | {cuenta or '–'} |")

    ev = [
        f"## Evidencia de {enc} (fábrica {VERSION})", "",
        f"**{veredicto}** · {ahora()} · {'clon limpio' if not a.sin_clon else 'árbol de trabajo (sin clon)'} "
        f"del commit `{r['commit'][:10]}` en la rama `{git(fab.raiz, 'rev-parse --abbrev-ref HEAD')}`.",
        f"Línea base: `{commit_base[:10] or '—'}` ({base.get('t', 'sin línea base')}). "
        f"Entorno: {entorno.get('plataforma', platform.platform())}.", "",
        "| Check | Estado | Código | Tiempo | Contadores (base→ahora) |", "|---|---|---|---|---|", *filas, "",
    ]
    if cmp["nuevos"]:
        ev += ["Fallos nuevos (bloquean):", "", *[f"- {x}" for x in cmp["nuevos"]], ""]
    if cmp["arreglados"]:
        ev += ["Arreglados respecto a la línea base:", "", *[f"- {x}" for x in cmp["arreglados"]], ""]
    if cmp["heredados"]:
        ev += [f"Heredados (ya fallaban antes, no son de este encargo): {len(cmp['heredados'])}. "
               "Lista en `linea_base.json`.", ""]
    if sucio and not a.sin_clon:
        ev += [f"Sin commitear y fuera de la prueba: {len(sucio)} fichero(s).", ""]
    ev += ["Commits del encargo:", "", "```", log_commits or "(ninguno desde la línea base)", "```", "",
           "Cambios:", "", "```", diff or "(sin cambios)", "```", "",
           "Prueba jugable (la escribe el agente, la juega Oriol):", "",
           "1. `./build/juego --nivel …`", "2. Qué hacer:", "3. Qué se ve:", "",
           f"Registros: `entrega_{enc}.zip` (logs de cada check, `fabrica.log`, línea base, huellas sha256).", ""]
    (d / "EVIDENCIA.md").write_text("\n".join(ev), encoding="utf-8")

    # huellas y zip
    ficheros = sorted(p for p in d.rglob("*") if p.is_file() and not p.name.startswith("entrega_")
                      and p.name != "HUELLAS.sha256")
    (d / "HUELLAS.sha256").write_text("".join(f"{sha256(p)}  {p.relative_to(d).as_posix()}\n" for p in ficheros),
                                      encoding="utf-8")
    zp = d / f"entrega_{enc}.zip"
    with zipfile.ZipFile(zp, "w", zipfile.ZIP_DEFLATED) as z:
        for p in ficheros + [d / "HUELLAS.sha256"]:
            z.write(p, f"{enc}/{p.relative_to(d).as_posix()}")
    fab.log(enc, "entregar_fin", veredicto=veredicto, zip=str(zp.relative_to(fab.dir_registros)),
            sha256=sha256(zp))

    if a.encargo_md:
        pega_evidencia(Path(a.encargo_md), "\n".join(ev))
        print(f"[fabrica] Evidencia pegada en {a.encargo_md}")
    imprime_veredicto(cmp)
    print(f"[fabrica] {d / 'EVIDENCIA.md'}\n[fabrica] {zp}")
    if veredicto == "VERDE":
        print(f"[fabrica] Para RESULTADOS.md: | {enc} | producido | {dt.date.today().isoformat()} | "
              f"evidencia en out/fabrica/{enc}/EVIDENCIA.md |")
    return 0 if veredicto == "VERDE" else 1


def pega_evidencia(encargo_md: Path, texto: str) -> None:
    """Pone la evidencia al final del encargo, entre marcas, y la sustituye si ya estaba."""
    ini, fin = "<!-- fabrica:evidencia -->", "<!-- /fabrica:evidencia -->"
    t = encargo_md.read_text(encoding="utf-8")
    bloque = f"{ini}\n{texto}\n{fin}"
    if ini in t and fin in t:
        t = t[:t.index(ini)] + bloque + t[t.index(fin) + len(fin):]
    else:
        t = t.rstrip() + "\n\n" + bloque + "\n"
    encargo_md.write_text(t, encoding="utf-8")


def cmd_estado(fab: Fabrica, a) -> int:
    actual = fab.dir_registros / "ACTUAL"
    if not actual.exists():
        print("[fabrica] Sin encargo activo.")
        return 0
    enc = actual.read_text(encoding="utf-8").strip()
    d = fab.dir_encargo(enc)
    base = linea_base(fab, enc)
    print(f"[fabrica] Encargo activo: {enc}")
    print(f"  línea base: {base['t'] + ' · ' + base['commit'][:10] if base else 'NO (ejecuta inicio)'}")
    u = d / "ultimo.json"
    if u.exists():
        j = json.loads(u.read_text(encoding="utf-8"))
        print(f"  último check: {j['t']} · fase {j['fase']} · nuevos {len(j['comparacion']['nuevos'])}")
    print(f"  evidencia: {'sí' if (d / 'EVIDENCIA.md').exists() else 'no'}")
    log = d / "fabrica.log"
    if log.exists():
        lineas = log.read_text(encoding="utf-8").splitlines()
        print(f"  registro: {len(lineas)} eventos; últimos:")
        for l in lineas[-5:]:
            j = json.loads(l)
            print(f"    {j['t']} {j['evento']} {j.get('id', '')}")
    return 0


# ----------------------------------------------------------------------------- main

def main(argv=None) -> int:
    for flujo in (sys.stdout, sys.stderr):  # la consola de Windows (cp1252) no sabe escribir ✗ ni ✓
        try:
            flujo.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser(prog="fabrica.py", description=__doc__.split("\n\n")[0])
    ap.add_argument("--config", default=str(AQUI / "fabrica.json"))
    ap.add_argument("--raiz", help="raíz del repo (por defecto, la de git)")
    ap.add_argument("--encargo", help="encargo (por defecto, el activo)")
    sub = ap.add_subparsers(dest="orden", required=True)

    p = sub.add_parser("inicio"); p.add_argument("encargo_pos", nargs="?")
    p.add_argument("--rapida", action="store_true", help="línea base solo con la fase rápida")
    p = sub.add_parser("check"); p.add_argument("--fase", choices=["rapida", "completa"], default="rapida")
    p = sub.add_parser("vigilar"); p.add_argument("--cada", type=int, default=5)
    p.add_argument("--max", type=int, default=0, help="ejecuciones máximas (0 = sin fin)")
    p.add_argument("--fase", choices=["rapida", "completa"], default="rapida")
    p = sub.add_parser("depurar"); p.add_argument("check")
    p = sub.add_parser("entregar"); p.add_argument("--sin-clon", action="store_true")
    p.add_argument("--conservar-clon", action="store_true")
    p.add_argument("--encargo-md", help="fichero del encargo donde pegar la evidencia")
    sub.add_parser("estado")

    a = ap.parse_args(argv)
    fab = Fabrica(Path(a.config), Path(a.raiz).resolve() if a.raiz else None)
    return {"inicio": cmd_inicio, "check": cmd_check, "vigilar": cmd_vigilar, "depurar": cmd_depurar,
            "entregar": cmd_entregar, "estado": cmd_estado}[a.orden](fab, a)


if __name__ == "__main__":
    sys.exit(main())
