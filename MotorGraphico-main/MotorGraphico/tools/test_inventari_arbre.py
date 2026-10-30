#!/usr/bin/env python3
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from inventari_arbre import classifica  # noqa: E402


def main() -> int:
    r = classifica(" M src/Game/AuraMoral.cpp\n?? .claude_tmp/\n")
    assert r == {"codi": ["src/Game/AuraMoral.cpp"], "dades": [], "docs": [], "config_local": [], "brossa": [".claude_tmp/"]}, r
    entrada = "\n".join([
        " M a/CMakeLists.txt", " M x/assets/levels/n.json", " M notas.md",
        " M .obsidian/workspace.json", "?? .papelera_sandbox/x", " M include/A.h",
        ' M "carpeta con espacio/informe_ni\\303\\261o.md"', " M otro.txt",
    ])
    r = classifica(entrada)
    assert r["codi"] == ["a/CMakeLists.txt", "include/A.h"], r
    assert r["dades"] == ["x/assets/levels/n.json"], r
    assert r["config_local"] == [".obsidian/workspace.json"], r
    assert r["brossa"] == [".papelera_sandbox/x"], r
    assert r["docs"] == ["notas.md", "carpeta con espacio/informe_niño.md", "otro.txt"], r
    vacio = classifica("")
    assert vacio == {c: [] for c in ("codi", "dades", "docs", "config_local", "brossa")}, vacio
    print("OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
