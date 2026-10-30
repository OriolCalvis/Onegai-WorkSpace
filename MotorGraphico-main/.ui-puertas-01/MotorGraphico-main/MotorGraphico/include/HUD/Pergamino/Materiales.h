// PERGAMINO-01 — Los materiales del tema, fabricados en el motor.
//
// Hoja (pergamino de canto rasgado), papiro (con rodillos de madera y
// remates de laton), cuero (tapas y lomo del cuaderno), pestana de indice,
// adornos de tinta, sello de lacre, chincheta y cinta de marcapaginas. Son
// los mismos algoritmos que Claude outputs/estudio_uiux_2026-09-28/maquetas/
// _generador/pergamino/texturas.py: paleta corta de papel, ruido de valor,
// tramado Bayer y pixel logico x2, como los sprites.
//
// Se fabrican aqui y no se leen de un PNG por la misma razon que la fuente y
// la sombra blob (BitmapFont::generateAtlas, FillBlobShadowPixels): cada
// panel pide su material a SU tamano exacto, sin estirar pixeles ni repetir
// mosaicos, y el mismo pedido da siempre la misma imagen. Sin OpenGL: la
// textura la sube Render/PergaminoGL.
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Pergamino {

// RGBA8, fila 0 = borde SUPERIOR (orden de lectura, como un PNG).
struct Imagen {
    int ancho = 0;
    int alto = 0;
    std::vector<std::uint8_t> rgba;

    void crea(int w, int h);  // todo transparente
    bool vacia() const { return ancho <= 0 || alto <= 0; }
    std::uint8_t* pixel(int x, int y) {
        return &rgba[(static_cast<std::size_t>(y) * static_cast<std::size_t>(ancho) +
                      static_cast<std::size_t>(x)) * 4];
    }
    const std::uint8_t* pixel(int x, int y) const {
        return &rgba[(static_cast<std::size_t>(y) * static_cast<std::size_t>(ancho) +
                      static_cast<std::size_t>(x)) * 4];
    }
};

enum class Material : std::uint8_t {
    Hoja,       // a=rasgado b=filete c=manchas d=oscuro*100
    Papiro,
    Cuero,      // a=costura b=remaches c=esquinas de laton
    Pestana,    // a=activa
    Adorno,     // filete de tinta con rombo central (alto 10)
    Sello,      // lacre 24x24
    Chincheta,  // laton 12x12
    Cinta,      // marcapaginas carmin 14x28
    Lomo,       // bisagra del cuaderno
    Tachuela,   // remache del lomo 12x12
    Flecha,     // punta de tinta bajo un aviso 12x8
};

// La clave de la cache de texturas: dos pedidos iguales son la misma imagen,
// bit a bit. Los parametros a..d significan lo que dice cada Material.
struct PedidoMaterial {
    Material tipo = Material::Hoja;
    int ancho = 0;
    int alto = 0;
    int semilla = 1;
    int a = 0, b = 0, c = 0, d = 0;

    bool operator==(const PedidoMaterial& o) const;
    bool operator<(const PedidoMaterial& o) const;
};

// Pedidos con nombre (medidas en pixeles de pantalla; pares, por el pixel
// logico x2).
PedidoMaterial hoja(int w, int h, int semilla, int rasgado = 2, int filete = 0, int manchas = 3,
                    int oscuro100 = 0);
PedidoMaterial papiro(int w, int h, int semilla = 3);
PedidoMaterial cuero(int w, int h, int semilla, bool costura, bool remaches, bool esquinas);
PedidoMaterial pestana(int w, int h, bool activa, int semilla);
PedidoMaterial adorno(int w);
PedidoMaterial pieza(Material tipo);          // Sello, Chincheta, Cinta, Tachuela, Flecha
PedidoMaterial lomo(int w, int h);

// Tamano de las piezas de medida fija (Sello, Chincheta...).
void medidaPieza(Material tipo, int& w, int& h);

// Fabrica la imagen. Pura y determinista; un pedido sin sentido (medidas
// <= 0) devuelve una imagen vacia en vez de lanzar.
Imagen fabrica(const PedidoMaterial& pedido);

}  // namespace Pergamino
