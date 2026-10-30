#!/usr/bin/env python3
"""Recalibra ECONOMIA.md con la cueva de Gorran dentro (AV19-04, 16/10): los saldos
que da simular_economia.py con 58 contenedores y los márgenes a ±15 %."""
import json, re, sys

p = sys.argv[1]
t = open(p, encoding="utf-8").read()

NUEVO = {
    "historia":     ([112, 369, 378], [[90, 130], [310, 430], [320, 440]]),
    "explora_todo": ([113, 194, 160], [[90, 130], [160, 230], [130, 190]]),
    "optimiza":     ([273, 578, 659], [[230, 320], [490, 670], [560, 760]]),
}

# §6: la tabla de saldos
for perfil, (saldos, m) in NUEVO.items():
    fila = re.compile(r"^\| " + perfil + r" \| \d+ \| \d+ \| \d+ \| [^|]+\|$", re.M)
    nueva = f"| {perfil} | {saldos[0]} | {saldos[1]} | {saldos[2]} | " + "; ".join(f"{a}–{b}" for a, b in m) + " |"
    assert fila.search(t), perfil
    t = fila.sub(nueva, t)

old = "**Calibrado el 16/10 (AV18-04b, §9):** estos saldos son los que da"
new = ("**Calibrado el 16/10 (AV18-04b, §9; recalibrado la noche del 16/10 con la cueva de Gorran, AV19-04):** "
       "estos saldos son los que da")
assert old in t
t = t.replace(old, new)

# §8: el bloque JSON
m = re.search(r"```json economia\n(\{.*?\n\})\n```", t, re.S)
bloque = m.group(1)
d = json.loads(bloque)
for perfil, (saldos, margenes) in NUEVO.items():
    d["perfiles"][perfil]["margenes"] = margenes
nuevo_bloque = json.dumps(d, indent=2, ensure_ascii=False)
t = t[:m.start(1)] + nuevo_bloque + t[m.end(1):]

# §9: la fila del reparto y la nota de la cueva
old = "| 1–3 | cofres/entorno/escondites | 820 (ciudad, interiores, cripta) | 820 colocados; **afueras y cueva (420) esperan a los niveles de Qwen** | 0 cuando entren |"
new = "| 1–3 | cofres/entorno/escondites | 1.240 (las cinco regiones) | 1.240 colocados: 58 contenedores (afueras y cueva ya dentro, 16/10) | 0 |"
assert old in t
t = t.replace(old, new)

old = "Pendiente que no cierra esto:"
new = ("**Recalibración con la cueva (Fable, noche del 16/10, AV19-04):** con `cueva_gorran` en el manifiesto, "
       "`repartir_botin.py` coloca 11 contenedores en la cueva (día 2: 120/20/100; día 3: 60/0/60, al 100 %) y los "
       "saldos suben donde se esperaba: explora_todo 113/194/160 (antes 97/127/56) y optimiza 273/578/659 (antes "
       "246/473/509); historia apenas se mueve (112/369/378). Márgenes a ±15 % de esos saldos; `simular_economia.py` "
       "sale a 0. Es la segunda y última recalibración prevista (la primera dejó la cueva fuera a sabiendas).\n\n"
       "Pendiente que no cierra esto:")
assert old in t
t = t.replace(old, new, 1)
open(p, "w", encoding="utf-8").write(t)
print("ECONOMIA.md recalibrada")
