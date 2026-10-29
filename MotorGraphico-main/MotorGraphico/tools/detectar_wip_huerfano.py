#!/usr/bin/env python3
"""detectar_wip_huerfano — paquetes del corte B marcados hechos en
planificacion/seguimiento/ cuyo codigo sigue SIN commitear.

Por que existe: el workspace lo atacan varias familias de agentes y no
todas siguen el protocolo D0001 (TABLERO + sesion + commit con
git-safe). El 12/09 se encontraron DOCE paquetes con seguimiento escrito
y codigo sin versionar (Claude Cowork, TRAE Cowork, opencode). Este
herramiento convierte esa situacion en un informe reproducible en vez de
un descubrimiento arqueologico por sesion.

Que cruza:
  1. seguimiento/*.json  -> id, autor, fecha, estado declarado
  2. MAPA_DEL_CODIGO.md  -> paquete -> ficheros que teoria toca
  3. git status          -> ficheros modificados/nuevos sin commitear
  4. ../TABLERO.md       -> filas "En curso" (sesiones VIVAS: no rescatar)

Salida: por paquete, cuantos de sus ficheros estan sin versionar y si su
sesion sigue viva. Es un informe de COORDINACION, no de calidad: no
ejecuta tests ni decide si el trabajo es bueno.

Uso:  python3 tools/detectar_wip_huerfano.py [--json]
No modifica nada. Paths relativos al propio script (BITACORA 2.15).
"""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

MOTOR = Path(__file__).resolve().parent.parent
SEGUIMIENTO = MOTOR / "planificacion" / "seguimiento"
MAPA = MOTOR / "planificacion" / "MAPA_DEL_CODIGO.md"
TABLERO = MOTOR.parent.parent / "TABLERO.md"


def cargar_seguimientos():
    lotes = {}
    for f in sorted(SEGUIMIENTO.glob("*.json")):
        try:
            d = json.loads(f.read_text(encoding="utf-8"))
        except (OSError, ValueError) as e:
            print(f"avis: {f.name}: {e}", file=sys.stderr)
            continue
        pid = d.get("id") or f.stem
        lotes[pid] = {
            "autor": d.get("author", "?"),
            "fecha": d.get("date", "?"),
            "estado": (d.get("status") or "?").strip(),
        }
    return lotes


def cargar_mapa():
    """paquete -> [ficheros] desde la seccion 2 del MAPA_DEL_CODIGO."""
    ficheros = {}
    if not MAPA.is_file():
        return ficheros
    paquete = None
    for line in MAPA.read_text(encoding="utf-8").splitlines():
        m = re.match(r"^\*\*([CDEFG]-\d+)", line)
        if m:
            paquete = m.group(1)
            ficheros.setdefault(paquete, [])
            continue
        if paquete and line.startswith("- `"):
            ruta = line.strip("- `").split("`", 1)[0].rstrip("/")
            ficheros[paquete].append(ruta)
    return ficheros


def git_status_por_fichero():
    out = subprocess.run(["git", "-C", str(MOTOR), "status", "--porcelain"],
                         capture_output=True, text=True, check=True).stdout
    estado = {}
    for line in out.splitlines():
        if not line.strip():
            continue
        codigo, ruta = line[:2].strip(), line[3:].strip().strip('"')
        # renames "viejo -> nuevo": cuenta ambos
        if " -> " in ruta:
            ruta = ruta.split(" -> ")[1]
        estado[ruta] = codigo or "??"
    return estado


def paquete_versionado(pid):
    """¿Algún commit nombra el id del paquete? La convención del repo es
    mencionarlo en el asunto ("feat(rpg): D-02 — ..."), así que un grep
    sobre el log discrimina: separa el paquete ya versionado cuyo
    fichero reforma OTRA sesión del paquete que nunca commiteó."""
    out = subprocess.run(
        ["git", "-C", str(MOTOR), "log", "--oneline", "-1",
         "--grep", re.escape(pid), "-E"],
        capture_output=True, text=True, check=True).stdout.strip()
    return bool(out)


def sesiones_vivas():
    """Ids de paquete mencionados en filas 'En curso' del TABLERO."""
    vivas = set()
    if not TABLERO.is_file():
        return vivas
    en_curso = False
    for line in TABLERO.read_text(encoding="utf-8").splitlines():
        if line.startswith("## "):
            en_curso = line.startswith("## En curso")
            continue
        if en_curso and line.startswith("|"):
            for pid in re.findall(r"\b([CDEFG]-\d+)\b", line):
                vivas.add(pid)
    return vivas


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", action="store_true", help="salida JSON")
    args = ap.parse_args()

    seguimientos = cargar_seguimientos()
    mapa = cargar_mapa()
    status = git_status_por_fichero()
    vivas = sesiones_vivas()

    informe = []
    for pid, info in sorted(seguimientos.items()):
        rutas = mapa.get(pid, [])
        tocados = []
        for ruta in rutas:
            base = f"MotorGraphico/{ruta}" if not ruta.startswith("..") else ruta
            if base in status:
                tocados.append((ruta, status[base]))
                continue
            # assets/ y directorios: cualquier fichero debajo cuenta
            prefijo = f"MotorGraphico/{ruta}/"
            tocados += [(f"{ruta}/", c) for r, c in status.items()
                        if r.startswith(prefijo)]
        if not tocados:
            continue  # todo lo que declara el mapa esta versionado
        # Nivel 1 (seguro): ficheros NUEVOS del paquete sin commitear.
        # Nivel 2 (posible): solo ficheros compartidos reformados — el
        # cambio puede ser de este paquete o de otra sesión que toca el
        # mismo fichero de cruce; se verifica con el símbolo distintivo
        # a mano. El grep por id es solo ANOTACIÓN: los commits citan
        # ids de otros paquetes en su prosa (p. ej. el de D-04 nombra
        # E-01) y otros versionan trabajo sin nombrar el id (05edf18 =
        # C-13/C-15 sin decirlo).
        Nuevos_sin_versionar = sorted({r for r, c in tocados if c == "??"})
        informe.append({
            "paquete": pid,
            "autor": info["autor"],
            "fecha": info["fecha"],
            "estado": info["estado"],
            "sesion_viva": pid in vivas,
            "nivel": 1 if Nuevos_sin_versionar else 2,
            "commit_que_lo_nombra": paquete_versionado(pid),
            "ficheros_sin_versionar": Nuevos_sin_versionar,
            "ficheros_compartidos_reformados": sorted({r for r, c in tocados if c != "??"}),
        })

    if args.json:
        print(json.dumps(informe, ensure_ascii=False, indent=1))
        return 0

    if not informe:
        print("OK: ningun paquete con seguimiento tiene ficheros con cambios sin versionar.")
        return 0
    nivel1 = [l for l in informe if l["nivel"] == 1]
    nivel2 = [l for l in informe if l["nivel"] == 2]
    print(f"NIVEL 1 — {len(nivel1)} paquete(s) con ficheros NUEVOS sin versionar (seguro):\n")
    for lote in nivel1:
        vivo = "SESION VIVA — NO RESCATAR" if lote["sesion_viva"] else "huerfano"
        print(f"  {lote['paquete']}  [{vivo}]  autor={lote['autor']}  fecha={lote['fecha']}")
        print(f"    estado: {lote['estado'][:100]}")
        for f in lote["ficheros_sin_versionar"]:
            print(f"      + {f}")
        if lote["ficheros_compartidos_reformados"]:
            print(f"    ademas reforman compartidos: "
                  f"{', '.join(lote['ficheros_compartidos_reformados'])}")
        print()
    print(f"NIVEL 2 — {len(nivel2)} paquete(s) que solo reforman ficheros compartidos "
          f"(posible huerfano: verificar simbolo distintivo):\n")
    for lote in nivel2:
        vivo = "sesion viva" if lote["sesion_viva"] else "sin fila viva"
        nombra = "commit que lo nombra: si" if lote["commit_que_lo_nombra"] else "commit que lo nombra: NO"
        print(f"  {lote['paquete']}  [{vivo}; {nombra}]  autor={lote['autor']}  fecha={lote['fecha']}")
        print(f"    reformados: {', '.join(lote['ficheros_compartidos_reformados'])}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
