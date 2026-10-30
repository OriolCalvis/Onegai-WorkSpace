#!/usr/bin/env python3
"""Migra les 13 plantillas cartas/*/lista.html (tenen panel-preview flotant
+ inline-js de filtre) al layout decorator.

Patró tret del pilot cartas/clases/lista.html (migrat a mà).

També afegeix cartas/rasgos/detalle.html (SIMPLE que faltava al script principal).
"""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent / "src/main/resources/templates"

# Subnav plural per cada tipus de carta
SUBNAV = {
    "armas": "armas", "condiciones": "condiciones", "consumibles": "consumibles",
    "deidades": "deidades", "dotes": "dotes", "enemigos": "enemigos",
    "habilidades": "habilidades", "hechizos": "hechizos", "invocaciones": "invocaciones",
    "pasivas": "pasivas", "rasgos": "rasgos", "razas": "razas", "transfondos": "transfondos",
}
PLURAL = {  # per al missatge "X de Y razas"
    "armas": "armas", "condiciones": "condiciones", "consumibles": "consumibles",
    "deidades": "deidades", "dotes": "dotes", "enemigos": "enemigos",
    "habilidades": "habilidades", "hechizos": "hechizos", "invocaciones": "invocaciones",
    "pasivas": "pasivas", "rasgos": "rasgos", "razas": "razas", "transfondos": "trasfondos",
}

# ────────────────────────────────────────────────────────────────────
# Head clàssic (mateix patró que migrar_layout.py però captura el títol)
# ────────────────────────────────────────────────────────────────────
OLD_HEAD_RE = re.compile(
    r'<!DOCTYPE html>\s*\n'
    r'<html lang="es" xmlns:th="http://www\.thymeleaf\.org">\s*\n'
    r'<head>\s*\n'
    r'\s*<meta charset="UTF-8">\s*\n'
    r'\s*<meta name="viewport" content="width=device-width, initial-scale=1">\s*\n'
    r'\s*<title(?P<title_attrs>[^>]*)>(?P<title>[^<]*)</title>\s*\n'
    r'\s*<link th:href="@\{/css/tokens\.css\}" rel="stylesheet">\s*\n'
    r'\s*<link th:href="@\{/css/base\.css\}" rel="stylesheet">\s*\n'
    r'\s*<link th:href="@\{/css/components\.css\}" rel="stylesheet">\s*\n'
    r'\s*<link th:href="@\{/css/utilities\.css\}" rel="stylesheet">\s*\n'
    r'</head>\s*\n'
    r'<body>\s*\n'
    r'<div class="gm-app">\s*\n'
    r'\s*<div th:replace="~\{Blocks/header-v2 :: header\}"></div>\s*\n'
    r'\s*<div class="gm-body">\s*\n'
    r'\s*<div th:replace="~\{Blocks/sidebar-v2 :: sidebar\}"></div>\s*\n'
    r'\s*<main class="gm-main">\s*\n',
    re.MULTILINE,
)

# Tail: en cartas/*/lista.html el patró és:
#   </main>
#   </div>  ← tanca gm-body
#   <div ...footer-v2...></div>
#   </div>  ← tanca gm-app
#   <div id="gm-panel-backdrop"...></div>
#   <div id="gm-panel-preview"...>...</div>   ← panel preview (fora del gm-app)
#   <script theme.js>
#   <script panels.js>
#   <script th:inline="javascript"> ... </script>
#   </body>
#   </html>
OLD_TAIL_RE = re.compile(
    r'\s*</main>\s*\n'
    r'\s*</div>\s*\n'                               # tanca gm-body
    r'\s*<div th:replace="~\{Blocks/footer-v2 :: footer\}"></div>\s*\n'
    r'</div>\s*\n'                                   # tanca gm-app
    r'(?P<extra_panels>\s*[\s\S]*?)'                # qualsevol cosa (panels, comentaris...)
    r'\s*<script th:src="@\{/js/theme\.js\}"></script>\s*\n'
    r'\s*<script th:src="@\{/js/panels\.js\}"></script>\s*\n'
    r'(?P<inline_js>\s*<script th:inline="javascript">[\s\S]*?</script>\s*\n)'
    r'</body>\s*\n'
    r'</html>\s*$',
    re.MULTILINE,
)

# Tail simple (per a rasgos/detalle sense panels ni inline-js)
OLD_TAIL_SIMPLE_RE = re.compile(
    r'\s*</main>\s*\n'
    r'\s*</div>\s*\n'
    r'\s*<div th:replace="~\{Blocks/footer-v2 :: footer\}"></div>\s*\n'
    r'</div>\s*\n'
    r'\s*<script th:src="@\{/js/theme\.js\}"></script>\s*\n'
    r'\s*<script th:src="@\{/js/panels\.js\}"></script>\s*\n'
    r'</body>\s*\n'
    r'</html>\s*$',
    re.MULTILINE,
)


def migrar_llista(path: Path, subnav: str) -> str:
    src = path.read_text(encoding="utf-8")
    if "layout:decorate" in src:
        return f"  ⊘ {path.name}: ja migrada"

    m = OLD_HEAD_RE.search(src)
    if not m:
        return f"  ⚠ {path.name}: head no trobat"
    title = m.group("title")
    title_attrs = m.group("title_attrs") or ""

    new_head = (
        '<!DOCTYPE html>\n'
        '<html lang="es" xmlns:th="http://www.thymeleaf.org"\n'
        '      xmlns:layout="http://www.ultraq.net.nz/thymeleaf/layout"\n'
        '      layout:decorate="~{Blocks/layout-gm :: layout}">\n'
        '<head>\n'
        f'    <title{title_attrs} layout:fragment="title">{title}</title>\n'
        '</head>\n'
        '<body>\n'
        '<main layout:fragment="content">\n'
    )
    src = src[: m.start()] + new_head + src[m.end():]

    m = OLD_TAIL_RE.search(src)
    if not m:
        return f"  ⚠ {path.name}: tail (amb panels+inline) no trobat"
    extra_panels = m.group("extra_panels") or ""
    inline_js = m.group("inline_js") or ""

    # Els panels queden dins del content (són position:fixed, funciona igual),
    # i l'inline-js va a extra-js.
    extra_js = f"\n<th:block layout:fragment=\"extra-js\">\n{inline_js.strip()}\n</th:block>"
    new_tail = f"{extra_panels.strip()}\n</main>{extra_js}\n</body>\n</html>\n"
    src = src[: m.start()] + new_tail

    path.write_text(src, encoding="utf-8")
    return f"  ✓ {path.name}"


def migrar_simple(path: Path, subnav: str | None = None) -> str:
    src = path.read_text(encoding="utf-8")
    if "layout:decorate" in src:
        return f"  ⊘ {path.name}: ja migrada"

    m = OLD_HEAD_RE.search(src)
    if not m:
        return f"  ⚠ {path.name}: head no trobat"
    title = m.group("title")
    title_attrs = m.group("title_attrs") or ""
    new_head = (
        '<!DOCTYPE html>\n'
        '<html lang="es" xmlns:th="http://www.thymeleaf.org"\n'
        '      xmlns:layout="http://www.ultraq.net.nz/thymeleaf/layout"\n'
        '      layout:decorate="~{Blocks/layout-gm :: layout}">\n'
        '<head>\n'
        f'    <title{title_attrs} layout:fragment="title">{title}</title>\n'
        '</head>\n'
        '<body>\n'
        '<main layout:fragment="content">\n'
    )
    src = src[: m.start()] + new_head + src[m.end():]

    m = OLD_TAIL_SIMPLE_RE.search(src)
    if not m:
        return f"  ⚠ {path.name}: tail simple no trobat"
    src = src[: m.start()] + '</main>\n</body>\n</html>\n'

    path.write_text(src, encoding="utf-8")
    return f"  ✓ {path.name}"


def main():
    print("── 13 cartas/*/lista.html (amb panel-preview + inline-js) ──")
    ok = fail = 0
    for tipus, subnav in SUBNAV.items():
        path = ROOT / f"cartas/{tipus}/lista.html"
        if not path.exists():
            print(f"  ⊘ {tipus}: no existe"); continue
        r = migrar_llista(path, subnav)
        print(r)
        ok += r.startswith("  ✓"); fail += r.startswith("  ⚠")

    print("\n── cartas/rasgos/detalle.html (simple que faltava) ──")
    r = migrar_simple(ROOT / "cartas/rasgos/detalle.html")
    print(r); ok += r.startswith("  ✓"); fail += r.startswith("  ⚠")

    print(f"\n═══ Total: {ok} OK · {fail} fallos ═══")


if __name__ == "__main__":
    main()
