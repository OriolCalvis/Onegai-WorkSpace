#!/usr/bin/env python3
# C-24 — Ho prova del paquet (PLA_DEFINITIU §C-24):
#   «Cobertura mesurada: cap fitxer de src/Game/ ni la part GL-free de
#    src/Engine/ per sota del 70% de línies.»
#
# Construeix un build de cobertura de clang (source-based, -fprofile-instr-generate)
# fora de l'arbre, executa els tests GL-free que condueixen src/Game i la part
# GL-free de src/Engine, i amb `llvm-cov export` calcula el % de línies per fitxer.
# Surt 0 si TOT està >= 70%, 1 si algun fitxer queda per sota (llista els sospitosos).
#
# GL-free de src/Engine (el que no és Render/GL): ApplicationLoop, InputSystem,
# SceneManager — únicament els que tenen test propi GL-free.
#
# Ús:
#   python3 tools/medir_cobertura_c24.py                    # build complet + mesura
#   python3 tools/medir_cobertura_c24.py --build-dir DIR     # reusa un build existent
#   python3 tools/medir_cobertura_c24.py --build-dir DIR --no-build
#
# Requereix: clang (AppleClang), llvm-profdata, llvm-cov.

import argparse
import glob
import json
import os
import shutil
import subprocess
import sys
import tempfile

UMBRAL = 70.0


def find_tool(name):
    # Preferix el mateix clang del sistema (AppleClang → CommandLineTools).
    cand = "/Library/Developer/CommandLineTools/usr/bin/" + name
    if os.path.exists(cand):
        return cand
    return shutil.which(name) or "xcrun " + name


MERGE = find_tool("llvm-profdata")
COV = find_tool("llvm-cov")

ENGINE_GL_FREE = frozenset({
    "Engine/ApplicationLoop.cpp",
    "Engine/Input/InputSystem.cpp",
    "Engine/Scene/SceneManager.cpp",
})

TARGETS = [
    "test_game_session",
    "test_battle_state",
    "test_save_game",
    "test_skill",
    "test_application_loop",
    "test_input_system",
    "test_scene_manager",
    "test_enemy_catalog_bridge",
    "test_game_mode",
    "test_carry_over",
    "test_hud_minimap",
    "test_business_mode",
]

FLAGS = "-fprofile-instr-generate -fcoverage-mapping"


def run(cmd, cwd=None, allow_fail=False):
    print("+", " ".join(str(c) for c in cmd))
    r = subprocess.run(cmd, cwd=cwd, text=True,
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if r.returncode != 0 and not allow_fail:
        print(r.stdout[-4000:])
        sys.exit("fallo del paso: %s" % " ".join(str(c) for c in cmd[:3]))
    return r.stdout


def configure(src, build_dir):
    run(["cmake", "-S", src, "-B", build_dir,
         "-DCMAKE_CXX_FLAGS=" + FLAGS,
         "-DCMAKE_EXE_LINKER_FLAGS=" + FLAGS])


def build(build_dir):
    for t in TARGETS:
        run(["cmake", "--build", build_dir, "--target", t, "-j8"])
    # copiar_assets és un target ALL; aquí l'assegurem explícitament per si no
    # es va construir amb la regla completa.
    run(["cmake", "--build", build_dir, "--target", "copiar_assets"])


def run_tests(build_dir, profile_root):
    failed = []
    for t in TARGETS:
        exe = os.path.join(build_dir, t)
        if not os.path.exists(exe):
            continue
        env = dict(os.environ)
        env["LLVM_PROFILE_FILE"] = os.path.join(profile_root, "prof-%m.profraw")
        r = subprocess.run([exe], cwd=build_dir, env=env, text=True,
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        tail = r.stdout[-300:]
        if r.returncode != 0:
            failed.append((t, tail))
    if failed:
        for t, out in failed:
            print("[FAIL] %s:\n%s" % (t, out))
        sys.exit("tests amb fallo — la cobertura no és fiable")


def merge_and_export(build_dir, profile_root, out_profdata, out_json):
    profraws = glob.glob(os.path.join(profile_root, "prof-*.profraw"))
    if not profraws:
        sys.exit("no hi ha profraws (executa primer sense --no-build)")
    profdata = os.path.join(out_profdata)
    run([MERGE, "merge", "-sparse"] + profraws + ["-o", profdata])

    objects = []
    for t in TARGETS:
        exe = os.path.join(build_dir, t)
        if os.path.exists(exe):
            objects += ["-object", exe]

    outf = open(out_json, "w")
    r = subprocess.run([COV, "export", "-instr-profile=" + profdata] + objects +
                       ["-format=text"], stdout=outf,
                       stderr=subprocess.PIPE, text=True)
    outf.close()
    if r.returncode != 0:
        sys.exit("llvm-cov export ha fallat:\n" + r.stderr[-800:])


def parse(out_json):
    with open(out_json) as f:
        data = json.load(f)
    files = data["data"][0]["files"]
    result = {}
    for fn in files:
        name = fn.get("filename", "")
        rel = name.split("src/")[-1]
        if rel.endswith(".cpp") and (rel.startswith("Game/") or rel in ENGINE_GL_FREE):
            pct = fn.get("summary", {}).get("lines", {}).get("percent", 0.0)
            result[rel] = pct if pct is not None else 0.0
    return result


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build-dir", default=None)
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    src = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))  # MotorGraphico/
    if args.build_dir:
        build_dir = os.path.abspath(args.build_dir)
    else:
        build_dir = tempfile.mkdtemp(prefix="motor_cov_")

    if not args.no_build:
        if args.build_dir:
            if not os.path.exists(build_dir):
                os.makedirs(build_dir)
            tests = os.path.join(build_dir, "CMakeCache.txt")
            if not os.path.exists(tests):
                configure(src, build_dir)
        else:
            configure(src, build_dir)
        build(build_dir)

    run_tests(build_dir, build_dir)

    out_profdata = os.path.join(build_dir, "cov.profdata")
    out_json = os.path.join(build_dir, "cov.json")
    merge_and_export(build_dir, build_dir, out_profdata, out_json)

    cov = parse(out_json)
    if not cov:
        sys.exit("no s'han trobat fitxers src/Game/ ni Engine GL-free en la cobertura")

    rows = sorted(cov.items(), key=lambda kv: kv[1])
    print("\n== Cobertura de línies per fitxer (llindar %.0f%%) ==" % UMBRAL)
    below = 0
    for name, pct in rows:
        flag = "OK " if pct >= UMBRAL else "FALLA"
        if pct < UMBRAL:
            below += 1
        print("  [%s] %-42s %5.1f%%" % (flag, name, pct))

    if below:
        print("\n%d fitxer(s) per sota del %.0f%% — Ho prova de C-24 en VERMELL." % (below, UMBRAL))
        sys.exit(1)
    print("\nCobertura C-24 OK: 0 fitxers per sota del %.0f%%." % UMBRAL)
    print("Build de cobertura reutilitzable: --build-dir %s" % build_dir)


if __name__ == "__main__":
    main()