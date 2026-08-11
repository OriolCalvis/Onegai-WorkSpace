#include "Render/BitmapFont.h"

#include "Core/Resources/Texture.h"
#include "Render/SpriteBatch.h"

#include <algorithm>
#include <vector>

#include <glad/glad.h>

namespace {

// Layout del atlas: 16x4 celdas = 64, una por caracter ASCII imprimible
// de 32 (espacio) a 95 ('_'). Cubre digitos, mayusculas y la puntuacion
// que usan los mensajes de BattleState::log() (ver Skill.h/BattleState.h):
// parentesis, punto, coma, dos puntos, comillas, apostrofe, guion.
constexpr int kCellW = 5;
constexpr int kCellH = 7;
constexpr int kCols = 16;
constexpr int kRows = 4;
constexpr int kFirstChar = 32;
constexpr int kLastChar = kFirstChar + kCols * kRows - 1;  // 95

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
    {'6', {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}},
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

char normalizeChar(char c) {
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

// nullptr si "c" no tiene glifo definido: generateAtlas() lo trata como
// celda en blanco, glyphUV() como la celda del espacio.
const unsigned char* findGlyphRows(char c) {
    char normalized = normalizeChar(c);
    for (const GlyphDef& g : kGlyphs) {
        if (g.ch == normalized) {
            return g.rows;
        }
    }
    return nullptr;
}

// Indice de celda [0, kCols*kRows) para "c": fuera del rango soportado
// (o sin glifo listado) cae en el espacio (indice 0), nunca fuera de la
// tabla.
int cellIndexForChar(char c) {
    char normalized = normalizeChar(c);
    if (normalized < kFirstChar || normalized > kLastChar) {
        return 0;
    }
    return normalized - kFirstChar;
}

}  // namespace

UVRect BitmapFont::glyphUV(char c) {
    int idx = cellIndexForChar(c);
    int col = idx % kCols;
    int row = idx / kCols;
    return UVRect{
        static_cast<float>(col) / static_cast<float>(kCols),
        static_cast<float>(row) / static_cast<float>(kRows),
        static_cast<float>(col + 1) / static_cast<float>(kCols),
        static_cast<float>(row + 1) / static_cast<float>(kRows),
    };
}

Vector2 BitmapFont::measureText(const std::string& text, int glyphWidth, int glyphHeight) {
    int maxWidth = 0;
    int currentWidth = 0;
    int lines = 1;
    for (char c : text) {
        if (c == '\n') {
            maxWidth = std::max(maxWidth, currentWidth);
            currentWidth = 0;
            ++lines;
            continue;
        }
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
        char c = static_cast<char>(kFirstChar + idx);
        const unsigned char* rows = findGlyphRows(c);
        if (rows == nullptr) {
            continue;
        }
        int cellCol = idx % kCols;
        int cellRow = idx / kCols;
        for (int py = 0; py < kCellH; ++py) {
            unsigned char rowBits = rows[py];
            for (int px = 0; px < kCellW; ++px) {
                bool set = ((rowBits >> (kCellW - 1 - px)) & 1) != 0;
                if (!set) {
                    continue;
                }
                int atlasX = cellCol * kCellW + px;
                int atlasY = cellRow * kCellH + py;
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
    for (char c : text) {
        if (c == '\n') {
            cursor.x = topLeft.x;
            // lineHeight() y no glyphHeight(): sin interlineado las
            // lineas quedan pegadas (ver su comentario en el .h).
            cursor.y += static_cast<float>(lineHeight());
            continue;
        }
        batch.submit(cursor, size, glyphUV(c), m_atlas, color);
        cursor.x += static_cast<float>(m_glyphWidth);
    }
}
