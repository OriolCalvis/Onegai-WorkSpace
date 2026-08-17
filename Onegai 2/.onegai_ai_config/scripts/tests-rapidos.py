#!/usr/bin/env python3
"""Test ABREVIADO y RÁPIDO de 2 agentes. Duración ~ 2-4 min max."""
import json, urllib.request, sys, time
from concurrent.futures import ThreadPoolExecutor

OLLAMA = "http://localhost:11434/api/generate"

def ask(model, prompt, nctx=8192, npredict=256, temp=0.4, timeout=240):
    data = json.dumps({
        "model": model, "prompt": prompt, "stream": False,
        "options": {"temperature": temp, "num_predict": npredict, "num_ctx": nctx}
    }).encode()
    req = urllib.request.Request(OLLAMA, data=data, headers={"Content-Type":"application/json"})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        d = json.loads(r.read().decode())
        resp = d.get("response","")
        if "</think>" in resp: resp = resp.split("</think>",1)[-1].strip()
        return resp, d

total_ok = 0; total_fail = 0
def check(name, condition, detail_ok, detail_fail):
    global total_ok, total_fail
    if condition:
        total_ok += 1
        print(f"  ✔ TEST {name:15s} OK  → {detail_ok}")
    else:
        total_fail += 1
        print(f"  ✖ TEST {name:15s} FAIL→ {detail_fail}")

print("="*72)
print("  ONEGAI AI — TESTS ABREVIADOS DE ESTABILIDAD")
print("="*72)
t0 = time.time()

# --------- TEST A1: Agente 1 identidad + prompt template ---------
print("\n🖋  TEST A - AGENTE 1 (STORY-WRITER):")
out, meta = ask("onegai/story-writer",
    "Contesta SOLO 2 frases CORTAS en ESPAÑOL y ACABA EXACTAMENTE con el texto '💡 Siguiente paso sugerido:'. ¿Cuál es tu rol?",
    npredict=180, temp=0.4)
check("A1-ID-writer",
      "especialista" in out.lower() or "creativ" in out.lower() or "escritor" in out.lower() or "💡" in out,
      f"Respuesta: {out[:140]}",
      f"Salida: {out[:180]}")

# --------- TEST A2: Agente 1 outline + tests DnD ---------
out, meta = ask("onegai/story-writer",
    "Crea un outline de NIVEL 3 (escenas) para una escena en Pico Dragón: Venides y Skilla llegan a medianoche y escuchan un ruido. Incluye AL MENOS 2 tests de habilidad con CD (DnD 5e), p.e. 'CD 14 Percepción'. Sé breve, solo outline.",
    npredict=500, temp=0.75, nctx=16384)
check("A2-outline+CD",
      "CD" in out and (("Escena" in out or "Scene" in out)),
      f"CD detectado + estructura escena. Tokens: {meta.get('eval_count','?')} t={meta.get('total_duration',0)/1e9:.1f}s",
      f"Falta estructura/CD. Output: {out[:180]}")

# --------- TEST B1: Agente 2 crítica estructura 5 puntos ---------
print("\n🔎 TEST B - AGENTE 2 (AUTHOR-CRITIC):")
out, meta = ask("onegai/author-critic",
    "TEXTO: 'Venides entró en la taberna. Tenía sed. Bebió agua. Luego se fue. Todo fue normal. Fin.' Haz una crítica siguiendo tu estructura (RESUMEN, POSITIVOS, PROBLEMAS, INCONSISTENCIAS, CHECKLIST). Breve.",
    npredict=600, temp=0.3, nctx=16384)
hit = sum(1 for kw in ("RESUMEN","POSITIVOS","PROBLEMAS","INCONSISTEN","CHECKLIST","ASPECTOS","✅","⚠️","🔍","📋","🎯") if kw in out)
check("B1-estruct-crític",
      hit >= 3,
      f"Se detectan {hit} de los 5 marcadores de estructura de crítica.",
      f"Solo {hit} marcadores. Output head: {out[:200]}")

# --------- TEST B2: Agente 2 contradicciones canon ---------
out, meta = ask("onegai/author-critic",
    "Texto con inconsistencias: 'Boundinghton se fundó en 930 d.C. — pero un párrafo más tarde dice que en 930 era Surysal (pueblo pescador). Skilla tiene 19 años; pero luego dice que nació hace 22 inviernos.' Señala las INCONSISTENCIAS EN TABLA (o al menos una lista con nivel de gravedad CRÍTICO/ALTO/MEDIO).",
    npredict=400, temp=0.2)
hit2 = sum(1 for kw in ("930","Surysal","Boundinghton","19","22","19 año","22 invierno","edad") if kw in out)
check("B2-canon-contrad",
      hit2 >= 3,
      f"Detectadas {hit2} frases clave de las 3 contradicciones. Tokens: {meta.get('eval_count','?')}",
      f"Faltan frases clave. Output: {out[:200]}")

# --------- TEST C: Paralelismo simultáneo ---------
print("\n⚡ TEST C - PARALELISMO SIMULTÁNEO (2 modelos a la vez):")
def writer_job():
    return ask("onegai/story-writer",
       "Escribe SOLO una frase exacta: 'AGENTE 1 STORY-WRITER OK'. Nada más.", npredict=64, temp=0.0)
def critic_job():
    return ask("onegai/author-critic",
       "Escribe SOLO una frase exacta: 'AGENTE 2 AUTHOR-CRITIC OK'. Nada más.", npredict=64, temp=0.0)

t_c0 = time.time()
with ThreadPoolExecutor(max_workers=2) as ex:
    fw = ex.submit(writer_job)
    fc = ex.submit(critic_job)
    wr, _ = fw.result()
    cr, _ = fc.result()
dt_c = time.time()-t_c0

check("C1-paralelo-writer",
      "AGENTE 1" in wr and "STORY" in wr and "CRITIC" not in wr,
      f"Writer contesta solo: {wr.strip()}",
      f"Salida: {wr.strip()}")
check("C2-paralelo-critic",
      "AGENTE 2" in cr and "CRITIC" in cr and "WRITER" not in cr,
      f"Critic contesta solo: {cr.strip()}",
      f"Salida: {cr.strip()}")
check("C3-paralelo-veloc",
      dt_c < 60,
      f"2 peticiones en paralelo completadas en {dt_c:.1f}s (< 1 min, buen rendimiento)",
      f"Lento ({dt_c:.1f}s) — revisar GPU offload o num_parallel")

# --------- TEST D: Alternancia (1→2→1→2→1) ---------
print("\n🔄 TEST D - ALTERNANCIA RÁPIDA 1↔2 (5 peticiones):")
alternated_ok = True
last_model = None
for i, (model, expected) in enumerate([
    ("onegai/story-writer",  ("Agente 1","escritor","writer","creativ","💡")),
    ("onegai/author-critic", ("Agente 2","crítico","critic","RESUMEN","PROBLEMAS")),
    ("onegai/story-writer",  ("Agente 1","escritor","writer")),
    ("onegai/author-critic", ("Agente 2","crítico","CHECKLIST","🎯")),
    ("onegai/story-writer",  ("Agente 1","escritor","💡")),
], start=1):
    out,_ = ask(model, "¿Cuál es tu nombre y propósito? Responde en UNA SOLA FRASE corta.",
               npredict=120, temp=0.3)
    hit = any(k.lower() in out.lower() for k in expected)
    status = "OK" if hit else "FAIL"
    print(f"   D{i} ({model:24s}) → {status}. Frase: {out[:110].replace(chr(10),' ')}")
    if not hit: alternated_ok = False
check("D-alternancia", alternated_ok, "5/5 alternancias sin confusión de personalidad", "alguna confusión de rol")

# --------- RESUMEN ---------
print("\n"+"="*72)
print(f"  TESTS PASADOS : {total_ok:2d} / {total_ok+total_fail}  ({100*total_ok/(total_ok+total_fail):.0f}%)")
print(f"  TESTS FALLIDOS: {total_fail:2d}")
print(f"  TIEMPO TOTAL  : {time.time()-t0:.1f}s")
level = "✅ CONFIGURACIÓN ESTABLE" if total_fail==0 else ("⚠️  FALLOS MENORES" if total_fail <=2 else "❌ FALLOS CRÍTICOS")
print(f"  RESULTADO     : {level}")
print("="*72)
sys.exit(0 if total_fail==0 else 2 if total_fail>=3 else 1)
