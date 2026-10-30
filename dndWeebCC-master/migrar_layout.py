#!/usr/bin/env python3
"""Migración batch al layout decorator Blocks/layout-gm.

Transforma una plantilla gm-app clásica (cabecera + body + scripts) en una
página que hereda del layout base vía layout:decorate.

Categorías soportadas (auto-detectadas):
  - simple:        sólo title + content
  - con extra-css: title + extra-css + content  (carta-fisica, wizard)
  - con extra-js:  title + content + extra-js   (selector-csv)

NO migra (deben quedar fuera o tratarse aparte):
  - mapa/index.html
  - personatges/detall.html (cache-bust + no-print)
  - aventuras/imprimir.html (cache-bust)
  - configuracio.html / exportaciones (legacy Bootstrap)
  - error.html (sin panels.js)
"""
from __future__ import annotations
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent / "src/main/resources/templates"

# Plantillas a migrar y su configuración
# (path_relativo, titulo_texto_o_None_si_hay_que_extraerlo,
#  css_extra_lista, js_extra_lista, subnav_grupo_o_None)
CONFIGS = []

# ────────────────────────────────────────────────────────────────────
# Categoría 1: simples (sin extra-css ni extra-js)
# ────────────────────────────────────────────────────────────────────
SIMPLES = [
    # cartas/index y diagnostico (sin subnav)
    ("cartas/index.html", None, [], [], None),
    ("diagnostico.html", None, [], [], None),
    # aventuras (constructor y detall son simples; llista tiene INLINE-JS → manual)
    ("aventuras/constructor.html", None, [], [], None),
    ("aventuras/detall.html", None, [], [], None),
    # cartas formularios SIN selector-csv
    ("cartas/condiciones/formulario.html", None, [], [], "condiciones"),
    ("cartas/consumibles/formulario.html", None, [], [], "consumibles"),
    ("cartas/rasgos/formulario.html", None, [], [], "rasgos"),
    ("cartas/transfondos/formulario.html", None, [], [], "transfondos"),
    # detalle de clases/dotes/pasivas/hechizos SIN carta-fisica (verificar antes)
    ("cartas/condiciones/detalle.html", None, [], [], "condiciones"),
    ("cartas/consumibles/detalle.html", None, [], [], "consumibles"),
    ("cartas/deidades/detalle.html", None, [], [], "deidades"),
    ("cartas/hechizos/detalle.html", None, [], [], "hechizos"),
    ("cartas/pasivas/detalle.html", None, [], [], "pasivas"),
    # eventos (lista y detalle son simples, sin inline-js)
    ("eventos/lista.html", None, [], [], None),
    ("eventos/detalle.html", None, [], [], None),
    # NOTA: personatges/llista, historias/lista, npcs/lista, tesoros/lista
    # tienen INLINE-JS simple (filtro sin panel preview) → su cierre SÍ encaja
    # en OLD_TAIL_RE. Se migran aquí tal cual (el inline-js va a extra-js).
    ("personatges/llista.html", None, [], [], None),
    ("historias/lista.html", None, [], [], None),
    ("npcs/lista.html", None, [], [], None),
    ("tesoros/lista.html", None, [], [], None),
    # NOTA: aventuras/llista sí es simple (sin inline-js)
    ("aventuras/llista.html", None, [], [], None),
]

# ────────────────────────────────────────────────────────────────────
# Categoría 2: con CSS extra (carta-fisica.css)
# ────────────────────────────────────────────────────────────────────
CON_CARTA_FISICA = [
    ("cartas/armas/detalle.html", "armas"),
    ("cartas/dotes/detalle.html", "dotes"),
    ("cartas/enemigos/detalle.html", "enemigos"),
    ("cartas/habilidades/detalle.html", "habilidades"),
    ("cartas/invocaciones/detalle.html", "invocaciones"),
    ("cartas/razas/detalle.html", "razas"),
    ("cartas/transfondos/detalle.html", "transfondos"),
    ("historias/detalle.html", None),
    ("tesoros/detalle.html", None),
]
for path, subnav in CON_CARTA_FISICA:
    CONFIGS.append((path, None, ["/css/carta-fisica.css"], [], subnav))

# ────────────────────────────────────────────────────────────────────
# Categoría 3: con JS extra (selector-csv.js) — formularios
# ────────────────────────────────────────────────────────────────────
CON_SELECTOR_CSV = [
    ("cartas/armas/formulario.html", "armas"),
    ("cartas/deidades/formulario.html", "deidades"),
    ("cartas/dotes/formulario.html", "dotes"),
    ("cartas/habilidades/formulario.html", "habilidades"),
    ("cartas/hechizos/formulario.html", "hechizos"),
    ("cartas/invocaciones/formulario.html", "invocaciones"),
    ("cartas/pasivas/formulario.html", "pasivas"),
    ("cartas/razas/formulario.html", "razas"),
    ("eventos/formulario.html", None),
]
for path, subnav in CON_SELECTOR_CSV:
    CONFIGS.append((path, None, [], ["/js/selector-csv.js"], subnav))

# wizard.css para aventuras/formulari
CONFIGS.append(("aventuras/formulari.html", None, ["/css/wizard.css"], [], None))

# historias/npcs formularios tienen carta-fisica.css
CONFIGS.append(("historias/formulario.html", None, ["/css/carta-fisica.css"], [], None))
CONFIGS.append(("npcs/formulario.html", None, ["/css/carta-fisica.css"], [], None))
CONFIGS.append(("npcs/detalle.html", None, ["/css/carta-fisica.css"], [], None))

# Sumar los simples
for c in SIMPLES:
    CONFIGS.append(c)


# ────────────────────────────────────────────────────────────────────
# Lógica de transformación
# ────────────────────────────────────────────────────────────────────

# Cabecera clásica (acepta: title literal o th:text, carta-fisica antes o después de los 4 CSS)
OLD_HEAD_RE = re.compile(
    r'<!DOCTYPE html>\s*\n'
    r'<html lang="es" xmlns:th="http://www\.thymeleaf\.org">\s*\n'
    r'<head>\s*\n'
    r'\s*<meta charset="UTF-8">\s*\n'
    r'\s*<meta name="viewport" content="width=device-width, initial-scale=1">\s*\n'
    r'\s*<title(?P<title_attrs>[^>]*)>(?P<title>[^<]*)</title>\s*\n'
    r'(?:\s*<link th:href="@\{/css/carta-fisica\.css\}" rel="stylesheet">\s*\n)?'  # opcional, antes
    r'\s*<link th:href="@\{/css/tokens\.css\}" rel="stylesheet">\s*\n'
    r'\s*<link th:href="@\{/css/base\.css\}" rel="stylesheet">\s*\n'
    r'\s*<link th:href="@\{/css/components\.css\}" rel="stylesheet">\s*\n'
    r'\s*<link th:href="@\{/css/utilities\.css\}" rel="stylesheet">\s*\n'
    r'(?:\s*<link th:href="@\{/css/carta-fisica\.css\}" rel="stylesheet">\s*\n)?'  # opcional, después
    r'(?:\s*<link th:href="@\{/css/wizard\.css\}" rel="stylesheet">\s*\n)?'  # opcional, wizard
    r'</head>\s*\n'
    r'<body>\s*\n'
    r'<div class="gm-app">\s*\n'
    r'\s*<div th:replace="~\{Blocks/header-v2 :: header\}"></div>\s*\n'
    r'\s*<div class="gm-body">\s*\n'
    r'\s*<div th:replace="~\{Blocks/sidebar-v2 :: sidebar\}"></div>\s*\n'
    r'\s*<main class="gm-main">\s*\n',
    re.MULTILINE,
)

# Cierre clásico (acepta: scripts extra sin th: entre panels.js y </body>)
OLD_TAIL_RE = re.compile(
    r'\s*</main>\s*\n'
    r'\s*</div>\s*\n'
    r'\s*<div th:replace="~\{Blocks/footer-v2 :: footer\}"></div>\s*\n'
    r'</div>\s*\n'
    r'(?:\s*<script th:src="@\{/js/selector-csv\.js\}"></script>\s*\n)?'
    r'\s*<script th:src="@\{/js/theme\.js\}"></script>\s*\n'
    r'\s*<script th:src="@\{/js/panels\.js\}"></script>\s*\n'
    r'(?P<extra_scripts>(?:\s*<script[^>]*>[\s\S]*?</script>\s*\n)*)'  # scripts extra (con o sin th:)
    r'(?:\s*<script th:src="@\{/js/selector-csv\.js\}"></script>\s*\n)?'
    r'</body>\s*\n'
    r'</html>\s*$',
    re.MULTILINE,
)


def migrar(path: Path, css_extra: list[str], js_extra: list[str]) -> str:
    src = path.read_text(encoding="utf-8")

    # 1. Cabecera
    m = OLD_HEAD_RE.search(src)
    if not m:
        return f"  ⚠ {path}: cabecera clásica no encontrada"
    title = m.group("title")
    title_attrs = m.group("title_attrs") or ""  # vacío si title literal; ' th:text="..."' si dinámico

    extra_css_block = "\n".join(
        f'    <link th:href="@{{{c}}}" rel="stylesheet">' for c in css_extra
    )
    if extra_css_block:
        extra_css_block = f'\n    <th:block layout:fragment="extra-css">\n{extra_css_block}\n    </th:block>'

    # Si el title tenía th:text, lo conservamos en el fragment para que siga siendo dinámico
    # (layout:fragment="title" acepta atributos th:* en el tag title).
    title_open = f'<title{title_attrs} layout:fragment="title">'
    new_head = (
        '<!DOCTYPE html>\n'
        '<html lang="es" xmlns:th="http://www.thymeleaf.org"\n'
        '      xmlns:layout="http://www.ultraq.net.nz/thymeleaf/layout"\n'
        '      layout:decorate="~{Blocks/layout-gm :: layout}">\n'
        '<head>\n'
        f'    {title_open}{title}</title>{extra_css_block}\n'
        '</head>\n'
        '<body>\n'
        '<main layout:fragment="content">\n'
    )
    src = src[: m.start()] + new_head + src[m.end():]

    # 2. Cierre
    m = OLD_TAIL_RE.search(src)
    if not m:
        return f"  ⚠ {path}: cierre clásico no encontrado"

    # Recoger scripts extra (los que estaban entre panels.js y </body>, con o sin th:)
    extra_scripts_raw = m.group("extra_scripts") or ""
    extra_js_lines = []
    if js_extra:
        for j in js_extra:
            extra_js_lines.append(f'    <script th:src="@{{{j}}}"></script>')
    # Añadir los scripts extra originales (preservándolos tal cual)
    if extra_scripts_raw.strip():
        # Re-indentar limpio: una línea por <script>...</script>
        for sm in re.finditer(r'<script[^>]*>[\s\S]*?</script>', extra_scripts_raw):
            extra_js_lines.append("    " + sm.group(0))

    extra_js_block = ""
    if extra_js_lines:
        joined = "\n".join(extra_js_lines)
        extra_js_block = f'\n<th:block layout:fragment="extra-js">\n{joined}\n</th:block>'

    new_tail = f'</main>{extra_js_block}\n</body>\n</html>\n'
    src = src[: m.start()] + new_tail

    path.write_text(src, encoding="utf-8")
    return f"  ✓ {path}"


def main():
    print(f"Migrando {len(CONFIGS)} plantillas…\n")
    ok = 0
    fail = 0
    for cfg in CONFIGS:
        rel, _title, css_extra, js_extra, _subnav = cfg
        path = ROOT / rel
        if not path.exists():
            print(f"  ✗ {rel}: archivo no existe")
            fail += 1
            continue
        result = migrar(path, css_extra, js_extra)
        print(result)
        if result.startswith("  ✓"):
            ok += 1
        else:
            fail += 1
    print(f"\nTotal: {ok} OK, {fail} fallos")


if __name__ == "__main__":
    main()
