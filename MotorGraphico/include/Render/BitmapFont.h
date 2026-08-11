#pragma once

#include <string>

#include "Core/Math/UVRect.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

class Texture;
class SpriteBatch;

// Fuente bitmap monoespaciada 5x7 (motor_grafico_gantt_rpg.puml, Fase 9:
// "Sistema de fuentes bitmap", el prerrequisito de texto que faltaba para
// el menu de comandos y el cuadro de dialogo del HUD -- ver
// HudTextWidgets.h).
//
// SOLO MAYUSCULAS + digitos + puntuacion basica (ver kGlyphs en el .cpp):
// una fuente 5x7 con mayusculas Y minusculas dobla la tabla de glifos sin
// anadir legibilidad util a un HUD retro de pixel art -- el propio
// Dragon Quest clasico va en mayusculas. drawText() convierte a
// mayusculas automaticamente (ver toupper en el .cpp), asi que el
// llamador no necesita acordarse.
//
// El atlas es una textura PROCEDURAL (generateAtlas(), mismo patron que
// makeProceduralLightmap()/makeWhiteTexture() en los demos de Fase 4/9):
// no hay ninguna imagen de fuente vendorizada en third_party/ (a
// diferencia de GLAD/stb/tinyxml2 -- ver README, "GLAD/stb_image/
// tinyxml2 vendorizados"; este entorno de trabajo no tiene red para
// descargar un PNG de fuente). Los patrones de bits de cada glifo son de
// este archivo: la forma clasica de fuente de puntos 5x7 que decenas de
// datasheets de LCD/microcontrolador usan desde los años 80 (de dominio
// publico por su propia naturaleza geometrica: formas de letras, no hay
// "codigo" que vendorizar).
class BitmapFont {
public:
    // scale: cada glifo logico es 5x7; a escala 1 son 5x7 PIXELS reales,
    // minusculo e ilegible en una pantalla moderna (los demos de Fase 4
    // son 1280x720) -- 3 (glifo 15x21) es el valor por defecto razonable.
    explicit BitmapFont(int scale = 3);
    ~BitmapFont();

    BitmapFont(const BitmapFont&) = delete;
    BitmapFont& operator=(const BitmapFont&) = delete;

    // --- Puras, sin necesitar una instancia (ni por tanto un atlas GL ya
    // generado): testeables sin contexto OpenGL -- ver
    // examples/demo_bitmap_font.cpp. Las versiones de instancia de mas
    // abajo son conveniencia sobre estas dos. ---

    // UV de la celda del atlas para "c" (min/mayuscula tratada igual,
    // ver el comentario de la clase). Caracteres sin glifo definido (o
    // fuera del rango soportado) devuelven la celda del espacio en
    // blanco, nunca lanzan.
    static UVRect glyphUV(char c);

    // Tamano en pixels de "text" a un tamano de glifo glyphWidth x
    // glyphHeight dado, contando '\n' como salto de linea (ancho = el de
    // la linea mas larga, alto = numero de lineas * glyphHeight, sin
    // espaciado extra entre lineas).
    static Vector2 measureText(const std::string& text, int glyphWidth, int glyphHeight);

    // Version de instancia: usa lineHeight() (CON interlineado) para el
    // alto, a diferencia de la estatica de arriba -- que se queda con el
    // contrato "sin espaciado extra entre lineas" del que dependen sus
    // tests (demo_bitmap_font.cpp). Siempre coincide con lo que dibuja
    // drawText() de esta misma instancia, que es lo que necesita quien
    // mide un panel contra su contenido.
    Vector2 measureText(const std::string& text) const {
        return measureText(text, m_glyphWidth, lineHeight());
    }

    // Encola un quad por caracter en "batch" (empezando en "topLeft",
    // avanzando a la derecha; '\n' salta de linea, volviendo a
    // topLeft.x). Caracteres sin glifo (ver glyphUV()) se dibujan como
    // espacio en blanco: nunca lanza ni crashea con datos de contenido
    // inesperados (mismo criterio permisivo que JsonValue/TextureAtlas::
    // getUV()).
    void drawText(SpriteBatch& batch, const std::string& text, const Vector2& topLeft,
                 const Vector4& color = Vector4{1.0f, 1.0f, 1.0f, 1.0f}) const;

    int glyphWidth() const { return m_glyphWidth; }
    int glyphHeight() const { return m_glyphHeight; }

    // Avance vertical entre lineas de drawText(): el alto del glifo MAS
    // un interlineado proporcional a la escala. Los glifos 5x7 ocupan
    // las 7 filas de su celda, asi que avanzar solo glyphHeight() deja
    // las lineas literalmente pegadas -- ilegible en cuanto hay dos
    // renglones (se vio en el cuadro de dialogo del combate). Quien mida
    // un bloque de texto debe usar ESTO, no glyphHeight().
    int lineHeight() const { return m_glyphHeight + kLineSpacing * m_scale; }

    Texture* atlas() const { return m_atlas; }

private:
    // Interlineado en pixeles LOGICOS (se multiplica por m_scale, ver
    // lineHeight()): 2 sobre un glifo de 7 filas separa los renglones
    // sin abrir un hueco que rompa el bloque de texto. Constante y no
    // parametro: mismo criterio que kTextPadding en HudTextWidgets.cpp
    // -- no hay hoy ningun caso de uso que necesite ajustarlo.
    static constexpr int kLineSpacing = 2;

    static Texture* generateAtlas();

    // Propietario (a diferencia de HudBar/HudPanel, que reciben un
    // Texture* ajeno): el atlas es especifico de esta fuente, nadie mas
    // lo comparte, asi que BitmapFont lo crea y lo libera el mismo (ver
    // ~BitmapFont). Puntero, no Texture por valor: un metodo const (ej.
    // drawText()) necesita pasarlo a SpriteBatch::submit(), que pide
    // Texture* no-const -- con un puntero miembro, "this const" solo
    // hace el PUNTERO constante, no lo que apunta (Texture* const), asi
    // que se puede pasar tal cual sin const_cast; con un Texture por
    // valor no habria forma de obtener un Texture* no-const desde un
    // metodo const sin uno.
    Texture* m_atlas;
    int m_scale;
    int m_glyphWidth;
    int m_glyphHeight;
};
