#!/usr/bin/env python3
"""Separa el catalogo de dndWeebCC en SISTEMA (reglamento reutilizable) y
EJEMPLO (el micro-setting de Llerba/la Tomba, que Oriol marco como material
de muestra a sustituir por historias canonicas).

Por que existe: sin esta separacion se invierte esfuerzo en contenido que va
a desaparecer -- exportarlo al motor, arreglar sus avisos, enlazarlo al canon.

Metodo:
  1. Semilla = toda carta que declara una de las facciones inventadas.
  2. Cierre transitivo = toda carta que referencia por id a una carta semilla.
Salida: reports/inventario_sistema_vs_ejemplo.json
"""
import json, glob, os, collections

AQUI = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.join(AQUI, "..", "..", "dndWeebCC-master", "data")
OUT = os.path.join(AQUI, "..", "reports", "inventario_sistema_vs_ejemplo.json")


def facciones_de(d, acc):
    if isinstance(d, dict):
        for k in ("faction", "factionName"):
            if isinstance(d.get(k), str):
                acc.add(d[k])
        for v in d.values():
            facciones_de(v, acc)
    elif isinstance(d, list):
        for v in d:
            facciones_de(v, acc)


def main():
    docs, cat, facs = {}, {}, set()
    for f in glob.glob(os.path.join(DATA, "**", "*.json"), recursive=True):
        try:
            d = json.load(open(f, encoding="utf-8"))
        except Exception:
            continue
        p = os.path.relpath(f, DATA).split(os.sep)
        c = p[1] if p[0] == "cartas" and len(p) > 2 else p[0]
        i = str(d.get("id", os.path.splitext(os.path.basename(f))[0])) if isinstance(d, dict) \
            else os.path.basename(f)
        docs[i] = f
        cat[i] = c

    semilla, texto = set(), {}
    for i, f in docs.items():
        d = json.load(open(f, encoding="utf-8"))
        texto[i] = json.dumps(d, ensure_ascii=False)
        acc = set()
        facciones_de(d, acc)
        if acc:
            semilla.add(i)
            facs |= acc

    ejemplo = set(semilla)
    for _ in range(8):                      # cierre transitivo
        nuevos = {i for i, t in texto.items()
                  if i not in ejemplo and any(len(m) > 6 and f'"{m}"' in t for m in ejemplo)}
        if not nuevos:
            break
        ejemplo |= nuevos

    sistema = set(docs) - ejemplo
    por_cat = collections.Counter(cat.values())
    ej_cat = collections.Counter(cat[i] for i in ejemplo)
    out = {
        "_nota": ("EJEMPLO = micro-setting de Llerba/la Tomba, material de muestra "
                  "a sustituir por historias canonicas. SISTEMA = reglamento y "
                  "catalogo reutilizable. No exportar ni enlazar al canon lo marcado "
                  "como ejemplo."),
        "facciones_de_ejemplo": sorted(facs),
        "resumen": {"total": len(docs), "sistema": len(sistema), "ejemplo": len(ejemplo)},
        "por_categoria": {c: {"total": por_cat[c], "ejemplo": ej_cat[c],
                              "sistema": por_cat[c] - ej_cat[c]}
                          for c in sorted(por_cat)},
        "ids_ejemplo": sorted(ejemplo),
    }
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    json.dump(out, open(OUT, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    print(f"{'categoria':16} {'total':>6} {'ejemplo':>8} {'sistema':>8}")
    print("-" * 42)
    for c in sorted(por_cat, key=lambda k: -por_cat[k]):
        print(f"{c:16} {por_cat[c]:6} {ej_cat[c]:8} {por_cat[c]-ej_cat[c]:8}")
    print("-" * 42)
    print(f"{'TOTAL':16} {len(docs):6} {len(ejemplo):8} {len(sistema):8}")


if __name__ == "__main__":
    main()
