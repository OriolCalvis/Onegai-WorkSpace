// PERGAMINO-01 — Las piezas pequenas del tema, pintadas en un Lienzo.
//
// Texto en las dos voces, iconos del atlas del juego, teclas, ayudas,
// insignias, barras, la cinta del foco, filetes y rotulos de bloque. Es el
// equivalente de componentes.css + tema_pergamino.css de las maquetas: las
// medidas salen de alli (tecla de 28 px con contorno de 2, fila de 44, etc.)
// para que el juego se vea como la muestra que Oriol aprobo.
//
// Sin estado propio: recibe la fuente y el manifiesto de iconos (no los
// posee) y cada llamada anade trazos al lienzo que se le pasa.
#pragma once

#include <string>
#include <vector>

#include "HUD/Iconos.h"
#include "HUD/Pergamino/FuenteAtlas.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino {

class Pincel {
public:
    Pincel(const FuenteAtlas& letras, const Hud::Iconos& iconos) : m_letras(letras), m_iconos(iconos) {}

    const FuenteAtlas& letras() const { return m_letras; }
    const Hud::Iconos& iconos() const { return m_iconos; }

    // Texto Lora en una caja de linea de alto `altoCaja` que empieza en
    // yCaja (la linea base se centra como en CSS). Devuelve el ancho.
    float texto(Lienzo& l, Letra letra, const std::string& s, float x, float yCaja, float altoCaja,
                const Vector4& color) const;
    // Voz pixel: yCaja es el borde de arriba de su caja de 9 filas (las 2
    // primeras son para las tildes). Devuelve el ancho.
    float pixel(Lienzo& l, const std::string& s, float x, float yCaja, int escala,
                const Vector4& color) const;
    float anchoPixel(const std::string& s, int escala) const { return static_cast<float>(m_letras.ancho(Letra::Pixel, s, escala)); }
    // OPCIONES-01 — El ancho de un texto Lora (para medir una lista antes de pintarla).
    float anchoTexto(Letra letra, const std::string& s) const { return static_cast<float>(m_letras.ancho(letra, s)); }
    // Parrafo envuelto a anchoMax; devuelve el alto usado. maxLineas > 0
    // recorta con puntos suspensivos la ultima linea visible.
    float parrafo(Lienzo& l, Letra letra, const std::string& s, float x, float y, float anchoMax,
                  float altoLinea, const Vector4& color, int maxLineas = 0) const;

    // Icono del atlas del juego (16 px) a escala 1 o 2.
    void icono(Lienzo& l, const std::string& nombre, float x, float y, int escala,
               const Vector4& tinte = Color::kBlanco) const;
    void iconoFrame(Lienzo& l, int frame, float x, float y, int escala,
                    const Vector4& tinte = Color::kBlanco) const;

    // Tecla: caja de papel claro de 28 px con contorno de tinta de 2 px y el
    // relieve de abajo. Devuelve su ancho (sin el contorno).
    float tecla(Lienzo& l, const std::string& etiqueta, float x, float y) const;
    float anchoTecla(const std::string& etiqueta) const;
    // Ayuda: una o varias teclas y el verbo en Lora 18.
    float ayuda(Lienzo& l, const std::vector<std::string>& teclas, const std::string& verbo, float x,
                float y, const Vector4& colorVerbo) const;
    float anchoAyuda(const std::vector<std::string>& teclas, const std::string& verbo) const;

    // Insignia: etiqueta pixel en una caja de 26 px con borde de 2.
    float insignia(Lienzo& l, const std::string& etiqueta, float x, float y, const Vector4& color,
                   const Vector4& fondo = Vector4{0, 0, 0, 0}) const;
    float anchoInsignia(const std::string& etiqueta) const { return anchoPixel(etiqueta, 2) + 16.0f; }

    // Barra de recurso: pista de papel claro con contorno de tinta; la
    // barra no tapa nunca su etiqueta (va debajo, MODELO_VISUAL.md §5).
    void barra(Lienzo& l, const Rect& r, float fraccion, const Vector4& relleno) const;
    // Fila con foco: lavado carmin, subrayado y la cinta de marcapaginas.
    void filaFoco(Lienzo& l, const Rect& fila) const;
    void contorno(Lienzo& l, const Rect& r, float grosor, const Vector4& color) const;
    // Linea de trazos de 2 px (4 de tinta, 4 de hueco), como el divisor de
    // las maquetas. Vertical u horizontal.
    void punteadoV(Lienzo& l, float x, float y0, float y1, const Vector4& color) const;
    void punteadoH(Lienzo& l, float x0, float x1, float y, const Vector4& color) const;
    // «IDENTIDAD ------»: rotulo pixel de tinta-3 y el punteado hasta `ancho`.
    void tituloBloque(Lienzo& l, const std::string& etiqueta, float x, float y, float ancho) const;
    // Titulo de hoja con un filete a cada lado, centrado en [x, x+ancho].
    void tituloConFiletes(Lienzo& l, const std::string& etiqueta, float x, float y, float ancho,
                          int anchoFilete) const;

private:
    const FuenteAtlas& m_letras;
    const Hud::Iconos& m_iconos;
};

}  // namespace Pergamino
