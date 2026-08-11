// Fase 9 (motor_grafico_gantt_rpg.puml): fuente bitmap. Prueba SOLO las
// dos funciones estaticas y puras de BitmapFont (glyphUV()/measureText(),
// ver el comentario de la clase en BitmapFont.h: "testeables sin
// contexto OpenGL") -- deliberadamente NUNCA se construye un BitmapFont
// aqui (eso llamaria a generateAtlas(), que hace glGenTextures/
// glTexImage2D de verdad: necesita un contexto GL real inicializado con
// gladLoadGLLoader(), que este demo no tiene). Se enlaza contra
// BitmapFont.cpp + Texture.cpp + glad.c (mismo patron que demo_tilemap.cpp,
// ver CMakeLists.txt) solo porque el enlazador necesita resolver esos
// simbolos -- nunca se EJECUTAN en este binario.
#include "Render/BitmapFont.h"

#include "Check.h"

#include <iostream>

namespace {

void testGlyphUvKnownCells() {
    // ' ' (32) es el primer caracter del atlas: celda (0,0).
    UVRect space = BitmapFont::glyphUV(' ');
    require(space.u0 == 0.0f);
    require(space.v0 == 0.0f);

    // 'A' (65): indice 65-32=33 -> col=33%16=1, row=33/16=2. Atlas 16x4:
    // u0=1/16, v0=2/4.
    UVRect a = BitmapFont::glyphUV('A');
    require(a.u0 == 1.0f / 16.0f);
    require(a.v0 == 2.0f / 4.0f);
    require(a.u1 == 2.0f / 16.0f);
    require(a.v1 == 3.0f / 4.0f);

    // Minuscula: mismo resultado que la mayuscula (ver el comentario de
    // la clase, "min/mayuscula tratada igual").
    UVRect aLower = BitmapFont::glyphUV('a');
    require(aLower.u0 == a.u0 && aLower.v0 == a.v0);

    // '0' (48): indice 48-32=16 -> col=0, row=1.
    UVRect zero = BitmapFont::glyphUV('0');
    require(zero.u0 == 0.0f);
    require(zero.v0 == 1.0f / 4.0f);

    std::cout << "[BITMAPFONT] glyphUV() en celdas conocidas (espacio, 'A'/'a', '0') correcto.\n";
}

void testGlyphUvOutOfRangeFallsBackToSpace() {
    // Fuera del rango soportado (32-95): cae en la celda del espacio
    // (indice 0), nunca lanza ni se sale de la tabla.
    UVRect tab = BitmapFont::glyphUV('\t');
    UVRect space = BitmapFont::glyphUV(' ');
    require(tab.u0 == space.u0 && tab.v0 == space.v0);

    UVRect tilde = BitmapFont::glyphUV('~');  // 126, fuera de [32,95]
    require(tilde.u0 == space.u0 && tilde.v0 == space.v0);

    std::cout << "[BITMAPFONT] glyphUV() fuera de rango cae en la celda del espacio, sin "
                 "crashear.\n";
}

void testMeasureText() {
    // Una linea de 4 caracteres a glifo 10x14: ancho 40, alto 14.
    Vector2 oneLine = BitmapFont::measureText("TEST", 10, 14);
    require(oneLine.x == 40.0f);
    require(oneLine.y == 14.0f);

    // Dos lineas ("HP:30" de 5, "MP:10" de 5): ancho = el de la linea
    // mas larga (5*10=50), alto = 2*14=28.
    Vector2 twoLines = BitmapFont::measureText("HP:30\nMP:10", 10, 14);
    require(twoLines.x == 50.0f);
    require(twoLines.y == 28.0f);

    // Lineas de longitud distinta: el ancho es el de la MAS LARGA, no la
    // primera ni la ultima.
    Vector2 uneven = BitmapFont::measureText("A\nBBBBB\nCC", 10, 14);
    require(uneven.x == 50.0f);  // "BBBBB" = 5 caracteres
    require(uneven.y == 42.0f);  // 3 lineas * 14

    // Cadena vacia: 1 "linea" (vacia) -> alto = glyphHeight, ancho = 0.
    Vector2 empty = BitmapFont::measureText("", 10, 14);
    require(empty.x == 0.0f);
    require(empty.y == 14.0f);

    std::cout << "[BITMAPFONT] measureText() (una linea, multilinea, cadena vacia) correcto.\n";
}

}  // namespace

int main() {
    testGlyphUvKnownCells();
    testGlyphUvOutOfRangeFallsBackToSpace();
    testMeasureText();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
