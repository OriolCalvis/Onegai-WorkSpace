#!/usr/bin/env python3
"""Vuelca el atlas al proyecto documental en dos formatos generados:
  1. mapa_egaroth_geometrico.svg  — fronteras reales, ciudades y zonas
  2. naciones/<Nacion>.md         — ficha por nacion para el vault Obsidian

Ambos se REGENERAN; no se editan a mano. El lore sigue viviendo en el
corpus: estas fichas solo situan lo que el corpus ya cuenta.
"""
import json, os, collections, statistics

AQUI = os.path.dirname(os.path.abspath(__file__))
GEO = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "dndWeebCC-master", "data", "mapa", "geografia.json")
OV = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                 "..", "atlas", "overlays", "correcciones.json"), encoding="utf-8"))
CAPITALES_CANON = {k: v for k, v in OV["capitales_canon"].items() if not k.startswith("_")}
EPOCA = OV["epoca"]
ALIAS = {"Ayashi": "Ayashii", "Giladdokx": "Gliaddokx",
         "Tabaxi Continental": "Tabaxi", "Tabaxi Occidental": "Tabaxi"}

G = json.load(open(GEO, encoding="utf-8"))
W = json.load(open(os.path.join(AQUI, "..", "atlas", "world_grid.json"), encoding="utf-8"))


def svg():
    partes = ['<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1600 1260" '
              'font-family="Georgia,serif">',
              '<rect width="1600" height="1260" fill="#a8c4d4"/>',
              f'<!-- GENERADO desde geografia.json. Nombres de capital: canon Onegai.'
              f' Epoca: {EPOCA["etiqueta"]}. -->',
              f'<text x="800" y="46" font-size="34" text-anchor="middle" fill="#10202c">'
              f'EGAROTH — {EPOCA["etiqueta"]}</text>',
              f'<text x="800" y="72" font-size="16" text-anchor="middle" fill="#2c4454" '
              f'font-style="italic">{EPOCA["era"]}</text>']
    for n in G["naciones"]:
        partes.append(f'<polygon points="{n["points"]}" fill="{n.get("color","#888")}" '
                      f'stroke="#33404a" stroke-width="2" opacity="0.85"/>')
    for z in G["zonas"]:
        if z.get("tipo") == "santuario":
            partes.append(f'<circle cx="{z["x"]}" cy="{z["y"]}" r="7" fill="#fff4b0" stroke="#8a6d1f" stroke-width="2"/>')
    for c in G["ciudades"]:
        cap = c.get("tamano") == "capital"
        r = 6 if cap else 3
        partes.append(f'<circle cx="{c["x"]}" cy="{c["y"]}" r="{r}" '
                      f'fill="{"#f2e3b0" if cap else "#e8e8e8"}" stroke="#2b2b2b" stroke-width="1.5"/>')
        if cap:
            nom = CAPITALES_CANON.get(ALIAS.get(c["nacion"], c["nacion"]), c["nombre"])
            partes.append(f'<text x="{c["x"]+9}" y="{c["y"]+4}" font-size="15" fill="#1a1a1a">{nom}</text>')
    for n in G["naciones"]:
        pts = [tuple(map(float, p.split(","))) for p in n["points"].split()]
        if not pts: continue
        cx, cy = statistics.mean(p[0] for p in pts), statistics.mean(p[1] for p in pts)
        partes.append(f'<text x="{cx:.0f}" y="{cy:.0f}" font-size="21" font-weight="bold" '
                      f'fill="#101820" text-anchor="middle" opacity="0.75">{n["nombre"]}</text>')
    partes.append('<text x="24" y="1244" font-size="13" fill="#24404f">'
                  'Instantanea del ano en que empieza la Primera Cruzada · '
                  'generado desde el atlas, no editar a mano</text>')
    partes.append('</svg>')
    return "\n".join(partes)


def fichas():
    vecinos = collections.defaultdict(list)
    for f in W["fronteras"]:
        vecinos[f["a"]].append((f["b"], f["celdas_contacto"]))
        vecinos[f["b"]].append((f["a"], f["celdas_contacto"]))
    masa_de = {}
    for m in W["masas_de_tierra"]:
        for n in m["naciones"]:
            masa_de.setdefault(n, []).append(m)
    ciudades = collections.defaultdict(list)
    for c in G["ciudades"]:
        ciudades[ALIAS.get(c["nacion"], c["nacion"])].append(c)
    zonas = collections.defaultdict(list)
    for z in G["zonas"]:
        zonas[ALIAS.get(z.get("nacion", ""), z.get("nacion", ""))].append(z)
    out = {}
    for nac, datos in sorted(W["naciones"].items()):
        cap = CAPITALES_CANON.get(nac)
        cs = sorted(ciudades.get(nac, []), key=lambda c: c["nombre"])
        sin_nombre = [c for c in cs if c["nombre"].split()[0] in ("Capital","Ciudad","Pueblo","Aldea")]
        frente = datos.get("frente_cruzada")
        L = [f"# {nac}", "",
             f"> [!info] Ficha generada automáticamente desde el atlas · **{EPOCA['etiqueta']}**",
             f"> {EPOCA['era']}.",
             "> El mapa es una instantánea de ese año: no vale para eras anteriores ni posteriores.",
             "> No editar a mano, se regenera. El lore va en el corpus; esto solo lo sitúa.", "",
             "## Situación", "",
             f"- **Época del mapa:** {EPOCA['etiqueta']}",
             (f"- **Frente de las Cruzadas:** {frente}" if frente
              else "- **Frente de las Cruzadas:** no participa"),
             f"- **Capital:** {('[['+cap+']]') if cap else '⬜ sin capital en el canon'}",
             f"- **Superficie:** {datos['celdas']} celdas de la rejilla canónica ({datos['celdas']*100//242}% de la tierra emergida)",
             f"- **Masa de tierra:** {', '.join(m['id'] for m in masa_de.get(nac,[]))}", ""]
        vs = sorted(vecinos.get(nac, []), key=lambda v: -v[1])
        L += ["## Fronteras terrestres", ""]
        L += [f"- [[{v}]] — {n} celda{'s' if n>1 else ''} de contacto" for v, n in vs] if vs else \
             ["- Ninguna: es una nación insular."]
        L += ["", f"## Ciudades ({len(cs)})", ""]
        for c in cs:
            marca = " ⬜ *sin nombre canónico*" if c in sin_nombre else ""
            gob = f" · gobierna {c['gobernante']}" if c.get("gobernante") else ""
            pob = f" · {c['poblacion']}" if c.get("poblacion") else ""
            L.append(f"- **{c['nombre']}** ({c.get('tamano','?')}){pob}{gob}{marca}")
        zs = zonas.get(nac, [])
        if zs:
            L += ["", f"## Zonas ({len(zs)})", ""]
            L += [f"- **{z['nombre']}** — {z.get('tipo','?')}" for z in zs]
        if sin_nombre:
            L += ["", "## Trabajo pendiente", "",
                  f"- {len(sin_nombre)} asentamientos tienen sitio en el mapa y ningún nombre en el canon."]
        out[nac] = "\n".join(L) + "\n"
    return out


if __name__ == "__main__":
    os.makedirs(os.path.join(AQUI, "..", "atlas", "salida/naciones"), exist_ok=True)
    open(os.path.join(AQUI, "..", "atlas", "salida/mapa_egaroth_geometrico.svg"), "w", encoding="utf-8").write(svg())
    fs = fichas()
    for nac, txt in fs.items():
        open(os.path.join(AQUI, os.path.join("..", "atlas", "salida", "naciones", nac + ".md")), "w", encoding="utf-8").write(txt)
    print(f"SVG generado · {len(fs)} fichas de nación")
