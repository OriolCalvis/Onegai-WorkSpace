#include "Render/BitmapFont.h"

#include "Core/Resources/Texture.h"
#include "Render/SpriteBatch.h"

#include <algorithm>
#include <cstdint>
#include <vector>

#include <glad/glad.h>

namespace {

// Layout del atlas: 16x5 celdas = 80. Las 64 primeras son el ASCII
// imprimible de 32 (espacio) a 95 ('_'): digitos, mayusculas y la
// puntuacion base. Las 16 de la ultima fila son las letras acentuadas
// del castellano/catalan (F0 de la auditoria UI, bitacora: sin ellas los
// acentos, la ñ y «·» se dibujaban como ESPACIO EN BLANCO y media UI
// mentia en silencio).
constexpr int kCellW = 5;
constexpr int kCellH = 7;
constexpr int kCols = 16;
constexpr int kRows = 5;
constexpr int kFirstChar = 32;
constexpr int kLastChar = kFirstChar + 64 - 1;  // 95
constexpr int kExtendedBase = 64;               // celda de la 1ª acentuada

struct GlyphDef {
    char ch;
    unsigned char rows[kCellH];  // bit (kCellW-1) = columna izquierda, bit 0 = columna derecha
};

// Patrones 5x7 clasicos de fuente de puntos (ver el comentario de
// BitmapFont en el header). Solo se listan los caracteres con forma
// (el resto de las 64 celdas del atlas quedan en blanco/transparente:
// generateAtlas() no encuentra su GlyphDef y no dibuja nada en esa
// celda -- mismo resultado visual que un espacio).
constexpr GlyphDef kGlyphs[] = {
    {'0', {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}},
    {'1', {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
    {'2', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111}},
    {'3', {0b11111, 0b00010, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110}},
    {'4', {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}},
    {'5', {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110}},
    {'6', {0b00011, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}},
    {'7', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000}},
    {'8', {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}},
    {'9', {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100}},
    {'A', {0b01110, 0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001}},
    {'B', {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}},
    {'C', {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110}},
    {'D', {0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100}},
    {'E', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}},
    {'F', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}},
    {'G', {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111}},
    {'H', {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
    {'I', {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
    {'J', {0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b10001, 0b01110}},
    {'K', {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001}},
    {'L', {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}},
    {'M', {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}},
    {'N', {0b10001, 0b11001, 0b10101, 0b10101, 0b10011, 0b10001, 0b10001}},
    {'O', {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
    {'P', {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}},
    {'Q', {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}},
    {'R', {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}},
    {'S', {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}},
    {'T', {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
    {'U', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
    {'V', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100}},
    {'W', {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010}},
    {'X', {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}},
    {'Y', {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}},
    {'Z', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}},
    {'.', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100}},
    {',', {0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100, 0b01000}},
    {'!', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100}},
    {'?', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100}},
    {':', {0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b00000}},
    {'\'', {0b01000, 0b01000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
    {'"', {0b01010, 0b01010, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
    {'-', {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000}},
    {'(', {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010}},
    {')', {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000}},
    // Anadidos tras verificar el HUD con un render real: sin estos, los
    // caracteres se dibujaban como ESPACIO EN BLANCO (ver glyphUV) y el
    // texto mentia en silencio -- "PV 30/30" salia "PV 30 30", y el
    // cursor "> " de HudCommandMenu era invisible, dejando la seleccion
    // fiada solo al color (justo lo que su comentario en
    // HudTextWidgets.cpp dice que NO debe pasar). Todos caen dentro del
    // rango 32..95 que ya cubre el atlas, asi que no hace falta ampliarlo.
    {'/', {0b00001, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b10000}},
    {'\\', {0b10000, 0b10000, 0b01000, 0b00100, 0b00010, 0b00001, 0b00001}},
    {'>', {0b10000, 0b01000, 0b00100, 0b00010, 0b00100, 0b01000, 0b10000}},
    {'<', {0b00001, 0b00010, 0b00100, 0b01000, 0b00100, 0b00010, 0b00001}},
    {'+', {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000}},
    {'=', {0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000}},
    {'*', {0b00000, 0b00100, 0b10101, 0b01110, 0b10101, 0b00100, 0b00000}},
    {'%', {0b11001, 0b11010, 0b00010, 0b00100, 0b01000, 0b01011, 0b10011}},
    {'#', {0b01010, 0b01010, 0b11111, 0b01010, 0b11111, 0b01010, 0b01010}},
    {'[', {0b01110, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01110}},
    {']', {0b01110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01110}},
    {'_', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111}},
    {';', {0b00000, 0b00100, 0b00000, 0b00000, 0b00100, 0b00100, 0b01000}},
    {'&', {0b01100, 0b10010, 0b10100, 0b01000, 0b10101, 0b10010, 0b01101}},
};

// Fila extendida (celdas 64..79): vocales acentuadas (agudas Y graves:
// castellano y catalan), Ñ/Ç/Ü, «·» del l·l catalan y ¿/¡. Las letras
// pierden una fila de cuerpo para dejar sitio al signo diacritico en la
// fila superior: patron clasico de las fuentes 5x7 con acentos.
struct GlyphDefX {
    uint32_t cp;  // codepoint Unicode (Latin-1)
    unsigned char rows[kCellH];
};

constexpr GlyphDefX kGlyphsExtra[] = {
    {0x00C0, {0b01000, 0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001}},  // À
    {0x00C1, {0b00010, 0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001}},  // Á
    {0x00C8, {0b01000, 0b11111, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}},  // È
    {0x00C9, {0b00010, 0b11111, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}},  // É
    {0x00CC, {0b01000, 0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},  // Ì
    {0x00CD, {0b00010, 0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},  // Í
    {0x00D2, {0b01000, 0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},  // Ò
    {0x00D3, {0b00010, 0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},  // Ó
    {0x00D9, {0b01000, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},  // Ù
    {0x00DA, {0b00010, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},  // Ú
    {0x00D1, {0b01010, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001}},  // Ñ
    {0x00C7, {0b01110, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110, 0b00100}},  // Ç
    {0x00DC, {0b01010, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},  // Ü
    {0x00BF, {0b00100, 0b00000, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110}},  // ¿
    {0x00A1, {0b00100, 0b00000, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},  // ¡
    {0x00B7, {0b00000, 0b00000, 0b01100, 0b01100, 0b00000, 0b00000, 0b00000}},  // ·
};
constexpr int kNumExtra = static_cast<int>(sizeof(kGlyphsExtra) / sizeof(kGlyphsExtra[0]));

// Normaliza a la forma que existe en el atlas: la fuente sigue siendo de
// CAJA ALTA (estilo retro del proyecto), asi que minusculas -> mayusculas,
// incluidas las acentuadas (á -> Á).
uint32_t normalizeCodepoint(uint32_t cp) {
    if (cp >= 'a' && cp <= 'z') {
        return cp - 'a' + 'A';
    }
    // Latin-1: las minusculas acentuadas E0..FE (salvo ÷ F7) se
    // corresponden con sus mayusculas en C0..DE restando 0x20.
    if (cp >= 0x00E0 && cp <= 0x00FE && cp != 0x00F7) {
        return cp - 0x20;
    }
    return cp;
}

// Decodifica UN codepoint UTF-8 de "text" en la posicion "i" y avanza
// "i". Secuencias malformadas: se consume un byte y se devuelve tal cual
// (caera en la celda del espacio como cualquier caracter sin glifo: el
// criterio permisivo de siempre, nunca revienta).
uint32_t nextCodepoint(const std::string& text, std::size_t& i) {
    const auto b0 = static_cast<unsigned char>(text[i]);
    if (b0 < 0x80) {
        ++i;
        return b0;
    }
    if ((b0 & 0xE0) == 0xC0 && i + 1 < text.size()) {
        const uint32_t cp = (static_cast<uint32_t>(b0 & 0x1Fu) << 6) |
                            (static_cast<unsigned char>(text[i + 1]) & 0x3Fu);
        i += 2;
        return cp;
    }
    if ((b0 & 0xF0) == 0xE0 && i + 2 < text.size()) {
        const uint32_t cp = (static_cast<uint32_t>(b0 & 0x0Fu) << 12) |
                            ((static_cast<unsigned char>(text[i + 1]) & 0x3Fu) << 6) |
                            (static_cast<unsigned char>(text[i + 2]) & 0x3Fu);
        i += 3;
        return cp;
    }
    ++i;
    return b0;
}

// Indice de celda [0, kCols*kRows) para un codepoint: ASCII 32..95 en
// las 64 primeras, acentuadas en la fila extendida, el resto cae en el
// espacio (indice 0) sin salirse nunca de la tabla.
int cellIndexForCodepoint(uint32_t cp) {
    cp = normalizeCodepoint(cp);
    if (cp >= static_cast<uint32_t>(kFirstChar) && cp <= static_cast<uint32_t>(kLastChar)) {
        return static_cast<int>(cp) - kFirstChar;
    }
    for (int k = 0; k < kNumExtra; ++k) {
        if (kGlyphsExtra[k].cp == cp) {
            return kExtendedBase + k;
        }
    }
    return 0;
}

// Filas del glifo de la celda "idx" (nullptr = celda en blanco).
const unsigned char* glyphRowsForCell(int idx) {
    if (idx < kExtendedBase) {
        const char c = static_cast<char>(kFirstChar + idx);
        for (const GlyphDef& g : kGlyphs) {
            if (g.ch == c) {
                return g.rows;
            }
        }
        return nullptr;
    }
    const int k = idx - kExtendedBase;
    if (k >= kNumExtra) {
        return nullptr;
    }
    return kGlyphsExtra[k].rows;
}

UVRect uvForCell(int idx) {
    const int col = idx % kCols;
    const int row = idx / kCols;
    return UVRect{
        static_cast<float>(col) / static_cast<float>(kCols),
        static_cast<float>(row) / static_cast<float>(kRows),
        static_cast<float>(col + 1) / static_cast<float>(kCols),
        static_cast<float>(row + 1) / static_cast<float>(kRows),
    };
}

}  // namespace

UVRect BitmapFont::glyphUVCodepoint(uint32_t codepoint) {
    return uvForCell(cellIndexForCodepoint(codepoint));
}

UVRect BitmapFont::glyphUV(char c) {
    return glyphUVCodepoint(static_cast<unsigned char>(c));
}

Vector2 BitmapFont::measureText(const std::string& text, int glyphWidth, int glyphHeight) {
    int maxWidth = 0;
    int currentWidth = 0;
    int lines = 1;
    std::size_t i = 0;
    while (i < text.size()) {
        if (text[i] == '\n') {
            maxWidth = std::max(maxWidth, currentWidth);
            currentWidth = 0;
            ++lines;
            ++i;
            continue;
        }
        nextCodepoint(text, i);  // un codepoint = un glifo, sea 1 o 3 bytes
        currentWidth += glyphWidth;
    }
    maxWidth = std::max(maxWidth, currentWidth);
    return Vector2{static_cast<float>(maxWidth), static_cast<float>(lines * glyphHeight)};
}

Texture* BitmapFont::generateAtlas() {
    constexpr int kAtlasW = kCols * kCellW;
    constexpr int kAtlasH = kRows * kCellH;

    // RGBA8 inicializado a transparente (alpha 0): las celdas sin glifo
    // definido se quedan asi, que es exactamente lo que se quiere para
    // el espacio y para cualquier caracter sin forma listada.
    std::vector<unsigned char> pixels(static_cast<std::size_t>(kAtlasW) * kAtlasH * 4, 0);

    for (int idx = 0; idx < kCols * kRows; ++idx) {
        const unsigned char* rows = glyphRowsForCell(idx);
        if (rows == nullptr) {
            continue;
        }
        const int cellCol = idx % kCols;
        const int cellRow = idx / kCols;
        for (int py = 0; py < kCellH; ++py) {
            unsigned char rowBits = rows[py];
            for (int px = 0; px < kCellW; ++px) {
                bool set = ((rowBits >> (kCellW - 1 - px)) & 1) != 0;
                if (!set) {
                    continue;
                }
                const int atlasX = cellCol * kCellW + px;
                const int atlasY = cellRow * kCellH + py;
                std::size_t i =
                    (static_cast<std::size_t>(atlasY) * kAtlasW + atlasX) * 4;
                pixels[i + 0] = 255;
                pixels[i + 1] = 255;
                pixels[i + 2] = 255;
                pixels[i + 3] = 255;
            }
        }
    }

    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kAtlasW, kAtlasH, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                pixels.data());
    // GL_NEAREST: texto de pixel-art nitido, sin difuminar los bordes de
    // los glifos al escalar (mismo motivo que ColorLUT/FogOfWar).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return new Texture(glID, kAtlasW, kAtlasH);
}

BitmapFont::BitmapFont(int scale)
    : m_atlas(generateAtlas())
    , m_scale(std::max(scale, 1))
    , m_glyphWidth(kCellW * m_scale)
    , m_glyphHeight(kCellH * m_scale) {}

BitmapFont::~BitmapFont() { delete m_atlas; }

void BitmapFont::drawText(SpriteBatch& batch, const std::string& text, const Vector2& topLeft,
                          const Vector4& color) const {
    Vector2 cursor = topLeft;
    Vector2 size{static_cast<float>(m_glyphWidth), static_cast<float>(m_glyphHeight)};
    std::size_t i = 0;
    while (i < text.size()) {
        if (text[i] == '\n') {
            cursor.x = topLeft.x;
            // lineHeight() y no glyphHeight(): sin interlineado las
            // lineas quedan pegadas (ver su comentario en el .h).
            cursor.y += static_cast<float>(lineHeight());
            ++i;
            continue;
        }
        const uint32_t cp = nextCodepoint(text, i);
        batch.submit(cursor, size, glyphUVCodepoint(cp), m_atlas, color);
        cursor.x += static_cast<float>(m_glyphWidth);
    }
}
