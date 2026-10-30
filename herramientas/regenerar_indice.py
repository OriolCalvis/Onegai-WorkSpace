#!/usr/bin/env python3
r"""Regenera sesiones/INDICE.md desde los fragmentos de sesión.

- La Parte 0 de BITACORA_DEL_PROYECTO.md quedó congelada (decisión D0001):
  este índice es su sucesor vivo.
- Reglas de la casa: paths relativos a __file__ (nunca /Users/admin,
  BITACORA 2.15) y ensure_ascii=False al escribir (trampa del \uXXXX —
  que además mordió a este docstring: tiene que ser raw).
- Uso: python3 herramientas/regenerar_indice.py
"""
from datetime import datetime
from pathlib import Path
import re, sys

RAIZ = Path(__file__).resolve().parent.parent
DIR, SALIDA = RAIZ / "sesiones", RAIZ / "sesiones" / "INDICE.md"
PATRON = re.compile(r"^(?P<fecha>\d{4}-\d{2}-\d{2})-(?P<autor>[a-z0-9]+)-(?P<slug>.+)\.md$")

def campo(texto, nombre):
    m = re.search(rf"^\s*-\s*\*\*{nombre}:?\*\*\s*:?\s*(.+)$", texto, re.MULTILINE)
    return m.group(1).strip() if m else "—"

def main():
    if not DIR.is_dir():
        print(f"No existe {DIR}; crea sesiones/ primero.", file=sys.stderr); return 1
    filas = []
    for f in sorted(DIR.glob("*.md")):
        if f.name == "INDICE.md": continue
        m = PATRON.match(f.name)
        if not m: continue
        t = f.read_text(encoding="utf-8")
        filas.append((m["fecha"], m["autor"], m["slug"].replace("-", " "), f.name,
                      campo(t, "Commits"), campo(t, "Deja esperando a Oriol")))
    filas.sort(key=lambda r: r[0], reverse=True)
    lineas = ["# Índice de sesiones — regenerado, no editar a mano", "",
              f"_Regenerado: {datetime.now().isoformat(timespec='minutes')} · {len(filas)} sesiones_", "",
              "| Fecha | Autor | Sesión | Commits | Esperando a Oriol |", "|---|---|---|---|---|"]
    for fecha, autor, titulo, fichero, commits, oriol in filas:
        lineas.append(f"| {fecha} | {autor} | [{titulo}]({fichero}) | {commits} | {oriol} |")
    SALIDA.write_text("\n".join(lineas) + "\n", encoding="utf-8")
    print(f"INDICE.md regenerado con {len(filas)} sesiones.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
