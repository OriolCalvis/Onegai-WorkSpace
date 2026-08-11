#!/usr/bin/env python3
"""
Validador: comprueba que los JSON generados cumplen el contrato de ObjectCatalog.
- Campo "objects" es array
- Cada entrada tiene "id" no vacio y "category" valida (prop/enemy/pickup/npc)
- Si category=pickup: pickup.effect es none/heal/restoreMana
- Sin ids duplicados dentro de un mismo archivo
- (Opcional) Prueba de carga del archivo completo sin ids colisionando
"""
import json
import sys
from pathlib import Path

DIR = Path("/Users/admin/Documents/Documentos - Oriol Os (2)/Software/MotorGraphico-main/MotorGraphico/assets/objects")
CATEGORIAS_OK = {"prop", "enemy", "pickup", "npc"}
EFECTOS_OK = {"none", "heal", "restoreMana"}

ARCHIVOS = [
    "libreria_enemigos.json",
    "libreria_armas.json",
    "libreria_consumibles.json",
    "libreria_clases.json",
    "libreria_razas.json",
    "libreria_transfondos.json",
    "libreria_habilidades.json",
    "libreria_hechizos.json",
    "libreria_misc.json",
    "libreria_completa.json",
]

def validar_archivo(ruta: Path) -> tuple[int, list[str]]:
    errores = []
    with open(ruta, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, dict):
        errores.append("Raiz no es objeto")
        return 0, errores
    objs = data.get("objects")
    if not isinstance(objs, list):
        errores.append('Falta campo "objects" o no es array')
        return 0, errores
    ids_vistos = set()
    for i, o in enumerate(objs):
        if not isinstance(o, dict):
            errores.append(f"objects[{i}] no es objeto")
            continue
        obj_id = o.get("id")
        if not obj_id or not isinstance(obj_id, str):
            errores.append(f"objects[{i}] falta id o no es string")
            continue
        if obj_id in ids_vistos:
            errores.append(f'objects[{i}] id DUPLICADO: "{obj_id}"')
        ids_vistos.add(obj_id)
        cat = o.get("category")
        if cat not in CATEGORIAS_OK:
            errores.append(f'objects[{i}] ("{obj_id}") categoria desconocida: "{cat}"')
        if cat == "pickup":
            pk = o.get("pickup", {})
            ef = pk.get("effect", "none")
            if ef not in EFECTOS_OK:
                errores.append(f'objects[{i}] ("{obj_id}") pickup.effect desconocido: "{ef}"')
    return len(objs), errores

def main():
    print("=== Validacion de JSON generados (contrato ObjectCatalog) ===")
    print()
    todo_ok = True
    ids_global = {}
    for nombre in ARCHIVOS:
        ruta = DIR / nombre
        if not ruta.exists():
            print(f"[SKIP] {nombre}: no existe")
            continue
        n, errs = validar_archivo(ruta)
        status = "OK" if not errs else "FAIL"
        print(f"[{status}] {nombre}: {n} objetos, {len(errs)} errores")
        for e in errs:
            print(f"       - {e}")
        if errs:
            todo_ok = False
        if nombre != "libreria_completa.json":
            with open(ruta, "r", encoding="utf-8") as f:
                for o in json.load(f)["objects"]:
                    oid = o.get("id", "")
                    if oid:
                        ids_global.setdefault(oid, []).append(nombre)
    print()
    print("[INFO] Revisando colisiones de ID entre archivos individuales...")
    colisiones = {k: v for k, v in ids_global.items() if len(v) > 1}
    if colisiones:
        print(f"  [AVISO] {len(colisiones)} ids aparecen en multiples archivos:")
        for k, v in list(colisiones.items())[:10]:
            print(f"    - {k}: {v}")
        if len(colisiones) > 10:
            print(f"    ... y {len(colisiones)-10} mas")
        print("  (Nota: ObjectCatalog permite sobreescribir con loadFromString encadenados)")
    else:
        print("  OK: sin colisiones de id entre archivos individuales")
    print()
    if todo_ok:
        print("=== TODOS LOS ARCHIVOS PASAN LA VALIDACION ESTRUCTURAL ===")
        return 0
    else:
        print("=== HAY ERRORES ESTRUCTURALES ===")
        return 1

if __name__ == "__main__":
    sys.exit(main())
