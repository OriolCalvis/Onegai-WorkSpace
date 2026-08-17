#!/usr/bin/env python3
"""Hoja 4 de 'Ciudades y sus elementos.xlsx' -> world_grid.json
La Hoja 4 es una rejilla 26x26 donde cada celda lleva el nombre del reino
('~' = mar). Es, de facto, el mapamundi canonico de Egaroth."""
import openpyxl, json, collections, os, sys
AQUI = os.path.dirname(os.path.abspath(__file__))
XLSX = os.path.join(AQUI, "..", "..", "Onegai 2", "Onegai", "THE ONEGAI PROJECT",
                    "THE ONEGAI PROJECT", "Ciudades y sus elementos.xlsx")
OVERLAY = os.path.join(AQUI, "..", "atlas", "overlays", "correcciones.json")
N = 26
wb = openpyxl.load_workbook(XLSX)
ws = wb["Hoja 4"]
grid = [[("" if v is None else str(v).strip()) for v in row]
        for row in ws.iter_rows(min_row=1, max_row=N, max_col=N, values_only=True)]
def is_land(c): return bool(c) and c != "~"

# --- correcciones manuales (celdas anadidas a mano, epoca, capitales) ---
ov = json.load(open(OVERLAY, encoding="utf-8")) if os.path.exists(OVERLAY) else {}
for c in ov.get("celdas", []):
    grid[c["fila"]][c["columna"]] = c["nacion"]

# --- masas de tierra conexas (4-vecindad) ---
seen, masas = set(), []
for y in range(N):
    for x in range(N):
        if is_land(grid[y][x]) and (y, x) not in seen:
            pila, celdas = [(y, x)], []
            seen.add((y, x))
            while pila:
                cy, cx = pila.pop(); celdas.append((cy, cx))
                for dy, dx in ((1,0),(-1,0),(0,1),(0,-1)):
                    ny, nx = cy+dy, cx+dx
                    if 0 <= ny < N and 0 <= nx < N and (ny,nx) not in seen and is_land(grid[ny][nx]):
                        seen.add((ny,nx)); pila.append((ny,nx))
            masas.append(sorted(celdas))
masas.sort(key=len, reverse=True)

# --- adyacencias entre naciones (fronteras terrestres) ---
front = collections.Counter()
for y in range(N):
    for x in range(N):
        a = grid[y][x]
        if not is_land(a): continue
        for dy, dx in ((1,0),(0,1)):
            ny, nx = y+dy, x+dx
            if ny < N and nx < N:
                b = grid[ny][nx]
                if is_land(b) and b != a:
                    front[tuple(sorted((a,b)))] += 1

naciones = sorted({c for row in grid for c in row if is_land(c)})
frentes = ov.get("frentes_cruzada", {})
out = {
  "fuente": "THE ONEGAI PROJECT/Ciudades y sus elementos.xlsx · Hoja 4",
  "epoca": ov.get("epoca", {}),
  "correcciones_aplicadas": ov.get("celdas", []),
  "dimensiones": {"ancho": N, "alto": N},
  "leyenda": {"mar": "~"},
  "grid": grid,
  "naciones": {n: {"celdas": sum(r.count(n) for r in grid),
                   "frente_cruzada": ("occidental" if n in frentes.get("occidental", [])
                                      else "oriental" if n in frentes.get("oriental", []) else None),
                   "capital_canon": ov.get("capitales_canon", {}).get(n)}
               for n in naciones},
  "masas_de_tierra": [
      {"id": f"landmass_{i+1}",
       "celdas": len(m),
       "bbox": {"x": min(x for _,x in m), "y": min(y for y,_ in m),
                "w": max(x for _,x in m)-min(x for _,x in m)+1,
                "h": max(y for y,_ in m)-min(y for y,_ in m)+1},
       "naciones": sorted({grid[y][x] for y,x in m})}
      for i, m in enumerate(masas)],
  "fronteras": [{"a": a, "b": b, "celdas_contacto": n} for (a,b), n in front.most_common()],
}
json.dump(out, open("world_grid.json","w",encoding="utf-8"), ensure_ascii=False, indent=1)
print(f"epoca: {out['epoca'].get('etiqueta','(sin fechar)')}")
print(f"naciones: {len(naciones)} · tierra: {sum(1 for r in grid for c in r if is_land(c))} celdas"
      f" · masas: {len(masas)} · fronteras: {len(front)}")
