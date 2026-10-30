// PERGAMINO-01 — El lienzo: una pantalla de pergamino descrita como una
// lista de trazos, sin OpenGL.
//
// Todo lo que el tema dibuja (retales del HUD, hoja de dialogo, cuaderno,
// papiro) se describe aqui como cuadrilateros texturizados en pixeles de
// pantalla. Un unico ejecutor GL, Render/PergaminoGL, los envia a la
// SpriteBatch. Asi la maquetacion entera se prueba sin ventana: una prueba
// puede preguntar donde cae la cinta del foco o cuanto mide una hoja, que es
// lo que la vista en pantalla no deja comprobar (MODELO_VISUAL.md §10.7).
#pragma once

#include <cstdint>
#include <vector>

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "HUD/Pergamino/Materiales.h"

namespace Pergamino {

// De donde saca un trazo su textura. El ejecutor GL sabe conseguir cada una.
enum class Fuente : std::uint8_t {
    Blanco,    // textura blanca 1x1: rectangulos de color
    Letras,    // assets/textures/letras_pergamino.png
    Iconos,    // assets/textures/iconos.png
    Material,  // un PedidoMaterial fabricado bajo demanda (indice en materiales())
    Minimapa,  // el terreno del minimapa en tinta
    HojaViva,  // la hoja del cuaderno que se esta mirando (textura de render)
    HojaFoto,  // la hoja que se va, fotografiada al empezar la vuelta
    Retratos,  // UI-BG3-01: el atlas de retratos de personajes (AtlasRetratos)
};

struct Rect {
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    float derecha() const { return x + w; }
    float abajo() const { return y + h; }
};

// Un cuadrilatero. Esquinas en pixeles de pantalla (arriba-izquierda,
// arriba-derecha, abajo-derecha, abajo-izquierda) y UV normalizadas en el
// espacio de la IMAGEN: v = 0 es su fila de arriba, como en un PNG. Cuatro
// esquinas libres y no un rectangulo porque la hoja que cae es un trapecio.
struct Trazo {
    Fuente fuente = Fuente::Blanco;
    int material = -1;  // indice en Lienzo::materiales() si fuente == Material
    Vector2 p[4];
    Vector2 uv[4];
    Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

// UI-BG3-01 — Un trozo de pantalla que responde al raton (Oriol, 05/10: las
// pantallas se manejan con raton o con teclado). Quien pinta decide que es
// (`tipo`, un enum de su pantalla) y cual (`indice`); el lienzo solo sabe
// cual queda debajo del cursor. Como lo que se ve, se prueba sin ventana.
struct Zona {
    Rect rect;
    int tipo = 0;
    int indice = 0;
    bool contiene(float x, float y) const {
        return x >= rect.x && y >= rect.y && x < rect.derecha() && y < rect.abajo();
    }
};

class Lienzo {
public:
    void limpia();

    // Rectangulo alineado con la pantalla. `uv` es la region de la imagen,
    // normalizada (x, y, ancho, alto en 0..1).
    void rect(Fuente fuente, const Rect& destino, const Rect& uv, const Vector4& color,
              int material = -1);
    void color(const Rect& destino, const Vector4& color);
    void trazo(const Trazo& t) { m_trazos.push_back(t); }

    // Indice del material en este lienzo (los pedidos repetidos comparten
    // indice: el ejecutor fabrica cada uno una sola vez).
    int pide(const PedidoMaterial& pedido);
    // El material entero, a su tamano, con la esquina en `pos`.
    void material(const PedidoMaterial& pedido, const Vector2& pos,
                  const Vector4& tinte = Vector4{1.0f, 1.0f, 1.0f, 1.0f});
    // Lo mismo con su sombra de papel (4 px abajo, tinta al 45%), que es lo
    // que separa un retal del suelo arenoso.
    void materialConSombra(const PedidoMaterial& pedido, const Vector2& pos,
                           const Vector4& sombra);

    // Multiplica la opacidad de los trazos anadidos desde `desde` (para
    // fundir un grupo entero, como el aviso de objetivo que se va).
    void atenua(std::size_t desde, float factor);
    std::size_t cuantos() const { return m_trazos.size(); }

    // UI-BG3-01 — Zonas para el raton. La que se anade despues queda
    // encima (un emergente tapa la rejilla que tiene debajo).
    void zona(const Rect& r, int tipo, int indice = 0) { m_zonas.push_back(Zona{r, tipo, indice}); }
    const Zona* zonaEn(float x, float y) const;
    const std::vector<Zona>& zonas() const { return m_zonas; }
    std::size_t cuantasZonas() const { return m_zonas.size(); }
    // Mueve lo anadido desde (trazo, zona): una pantalla maquetada a
    // 1280x720 se centra asi en una vista mas grande.
    void desplaza(std::size_t desdeTrazo, std::size_t desdeZona, float dx, float dy);

    const std::vector<Trazo>& trazos() const { return m_trazos; }
    const std::vector<PedidoMaterial>& materiales() const { return m_materiales; }
    bool vacio() const { return m_trazos.empty(); }

private:
    std::vector<Trazo> m_trazos;
    std::vector<PedidoMaterial> m_materiales;
    std::vector<Zona> m_zonas;
};

}  // namespace Pergamino
