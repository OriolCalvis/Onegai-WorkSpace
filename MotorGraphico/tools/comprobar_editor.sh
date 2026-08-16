#!/usr/bin/env bash
# Comprueba tipos y sintaxis de los ejemplos que dependen de OpenGL, sin
# GPU y sin haber configurado CMake. Ver tools/stub_gl/glad/glad.h.
#
#   ./tools/comprobar_editor.sh                  # level_editor
#   ./tools/comprobar_editor.sh examples/juego.cpp
set -u
cd "$(dirname "$0")/.."
OBJETIVO="${1:-examples/level_editor.cpp}"

GLFW=$(find . -path "*/glfw-src/include" -type d 2>/dev/null | head -1)
GLM=$(find . -path "*/glm-src" -type d 2>/dev/null | head -1)
if [ -z "$GLFW" ]; then
  echo "No encuentro los headers de GLFW. Configura CMake una vez:"
  echo "  cmake -B build && ./tools/comprobar_editor.sh"
  exit 2
fi

g++ -std=c++17 -fsyntax-only \
    -Iinclude -Itools/stub_gl -Iexamples \
    -I"$GLFW" -I"$GLM" -Ithird_party/tinyxml2 -Ithird_party/stb \
    "$OBJETIVO"
CODIGO=$?
if [ $CODIGO -eq 0 ]; then
  echo "$OBJETIVO: tipos y sintaxis OK (sin GL de verdad)"
fi
exit $CODIGO
