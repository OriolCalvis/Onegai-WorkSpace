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

    // 'A' (65): indice 65-32=33 -> col=33%16=1, row=33/16=2. Atlas 16x5:
    // u0=1/16, v0=2/5.
    UVRect a = BitmapFont::glyphUV('A');
    require(a.u0 == 1.0f / 16.0f);
    require(a.v0 == 2.0f / 5.0f);
    require(a.u1 == 2.0f / 16.0f);
    require(a.v1 == 3.0f / 5.0f);

    // Minuscula: mismo resultado que la mayuscula (ver el comentario de
    // la clase, "min/mayuscula tratada igual").
    UVRect aLower = BitmapFont::glyphUV('a');
    require(aLower.u0 == a.u0 && aLower.v0 == a.v0);

    // '0' (48): indice 48-32=16 -> col=0, row=1.
    UVRect zero = BitmapFont::glyphUV('0');
    require(zero.u0 == 0.0f);
    require(zero.v0 == 1.0f / 5.0f);

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

void testAcentuadas() {
    // F0 de la auditoria UI: las acentuadas existen (antes se dibujaban
    // como espacio en blanco). Viven en la fila extendida del atlas
    // (fila 4 de 5).
    UVRect aAguda = BitmapFont::glyphUVCodepoint(0x00C1);   // Á
    require(aAguda.v0 == 4.0f / 5.0f);
    UVRect space = BitmapFont::glyphUV(' ');
    require(aAguda.u0 != space.u0 || aAguda.v0 != space.v0);

    // Minuscula acentuada -> la MISMA celda que su mayuscula.
    UVRect aMin = BitmapFont::glyphUVCodepoint(0x00E1);     // á
    require(aMin.u0 == aAguda.u0 && aMin.v0 == aAguda.v0);

    // Ñ y Ç tienen celda propia; ¿/¡ y «·» tambien.
    UVRect enye = BitmapFont::glyphUVCodepoint(0x00D1);     // Ñ
    UVRect cedilla = BitmapFont::glyphUVCodepoint(0x00C7);  // Ç
    UVRect puntoMedio = BitmapFont::glyphUVCodepoint(0x00B7);  // ·
    require(enye.u0 != cedilla.u0);
    require(puntoMedio.v0 == 4.0f / 5.0f);

    // UTF-8 en measureText: "ÁÉ" son 4 bytes pero 2 glifos; "AÑA" son
    // 5 bytes y 3 glifos. Sin la decodificacion UTF-8 esto media el
    // doble/ancho de mas (y en drawText salian glifos basura).
    Vector2 dosAcentos = BitmapFont::measureText("\xC3\x81\xC3\x89", 10, 14);
    require(dosAcentos.x == 20.0f);
    Vector2 conEnye = BitmapFont::measureText("A\xC3\x91" "A", 10, 14);
    require(conEnye.x == 30.0f);

    std::cout << "[BITMAPFONT] acentuadas (fila extendida, UTF-8) correcto.\n";
}

void testWrapText() {
    // 70px a 10px por glifo = 7 glifos por linea: "AAA BBB" cabe justo
    // (3 + espacio + 3), "CCC" ya no.
    std::vector<std::string> w = BitmapFont::wrapText("AAA BBB CCC", 70, 10);
    require(w.size() == 2);
    require(w[0] == "AAA BBB");
    require(w[1] == "CCC");

    // Palabra que no cabe en ninguna linea: corte con guion (3 + '-').
    std::vector<std::string> h = BitmapFont::wrapText("ABCDEFGH", 40, 10);
    require(h.size() == 3);
    require(h[0] == "ABC-");
    require(h[1] == "DEF-");
    require(h[2] == "GH");

    // Los '\n' existentes se respetan.
    std::vector<std::string> n = BitmapFont::wrapText("AA\nBB", 50, 10);
    require(n.size() == 2 && n[0] == "AA" && n[1] == "BB");

    // Sin caja valida no se envuelve.
    std::vector<std::string> z = BitmapFont::wrapText("AAA BBB", 0, 10);
    require(z.size() == 1 && z[0] == "AAA BBB");

    // Acentuadas cuentan como UN glifo al envolver.
    std::vector<std::string> acc =
        BitmapFont::wrapText("\xC3\x81\xC3\x89 \xC3\x8D\xC3\x93", 20, 10);
    require(acc.size() == 2);
    require(acc[0] == "\xC3\x81\xC3\x89");
    require(acc[1] == "\xC3\x8D\xC3\x93");

    std::cout << "[BITMAPFONT] wrapText() correcto.\n";
}

void testClipText() {
    require(BitmapFont::clipText("HOLA", 50, 10) == "HOLA");
    require(BitmapFont::clipText("HOLA MUNDO", 50, 10) == "HO...");
    require(BitmapFont::clipText("ABCDEFGH", 30, 10) == "ABC");
    require(BitmapFont::clipText("LINEA\nOTRA", 100, 10) == "LINEA");

    std::cout << "[BITMAPFONT] clipText() correcto.\n";
}

}  // namespace

int main() {
    testGlyphUvKnownCells();
    testGlyphUvOutOfRangeFallsBackToSpace();
    testMeasureText();
    testAcentuadas();
    testWrapText();
    testClipText();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
