// PIXELLAB-01 — Los materiales del tema leidos de imagenes (9-slice), con el
// procedimental de PERGAMINO-01 de reserva.
//
// Pergamino::fabrica() dibuja cada material (hoja, papiro, cuero, pestana,
// lomo, adorno y las piezas fijas) con algoritmos del motor. El plan
// PixelLab (Claude outputs/pixellab/PLA_PIXELLAB.md, lote 1) los sustituye
// por imagenes pintadas, y esto es la pieza que las mete sin tocar a quien
// las pide: los paneles siguen pidiendo un PedidoMaterial a SU medida y el
// banco lo sirve
//   - de una PIEL (un PNG con sus cortes de 9-slice) si hay una que encaje
//     con el material y sus parametros, repitiendo bordes y cuerpo sin
//     estirar pixeles (esquinas intactas);
//   - o del procedimental de siempre si no la hay (o su PNG no carga).
//
// assets/textures/pergamino/materiales.json dice que pieles hay:
//   {"pieles": [{"material": "hoja", "imagen": "hoja.png", "cortes": [t, r, b, l]},
//               {"material": "hoja", "filete": 6, "imagenes": ["pagina_a.png", "pagina_b.png"],
//                "cortes": [24, 24, 24, 24]},
//               {"material": "pestana", "activa": true, "imagen": "pestana_activa.png", ...},
//               {"material": "sello", "imagen": "sello.png"}]}
// Los parametros que una piel nombra (rasgado, filete, manchas, oscuro para
// la hoja; costura, remaches, esquinas para el cuero; activa para la
// pestana) tienen que coincidir con los del pedido; la piel que nombra mas
// parametros gana. Con varias «imagenes», la semilla del pedido elige una.
// Una hoja pedida mas oscura (oscuro) que su piel se oscurece encima. Las
// piezas fijas (sello, chincheta, cinta, tachuela, flecha) van sin cortes y
// su PNG mide lo que medidaPieza().
//
// Sin OpenGL (motor_pergamino): lo usa Render/PergaminoGL en lugar de llamar
// a fabrica() a pelo, y se prueba sin ventana.
#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "HUD/Pergamino/Materiales.h"

class JsonValue;

namespace Pergamino {

struct Piel {
    Material tipo = Material::Hoja;
    std::map<std::string, int> parametros;  // los que la piel fija
    std::vector<Imagen> imagenes;           // una o varias (la semilla elige)
    int cortes[4] = {0, 0, 0, 0};           // arriba, derecha, abajo, izquierda
};

class BancoMateriales {
public:
    // Lee <carpeta>/materiales.json y sus PNG. Sin fichero no es un error:
    // el banco queda vacio y todo sale del procedimental. Una piel rota se
    // apunta en avisos() y se salta.
    Result<bool> carga(const std::string& carpeta);
    bool cargado() const { return m_cargado; }
    const std::vector<std::string>& avisos() const { return m_avisos; }
    std::size_t pieles() const { return m_pieles.size(); }

    // La piel que serviria a un pedido (nullptr = procedimental).
    const Piel* piel(const PedidoMaterial& pedido) const;
    // La imagen del pedido: de su piel (9-slice a la medida pedida) o de
    // Pergamino::fabrica(). Misma regla que fabrica(): un pedido sin sentido
    // devuelve una imagen vacia.
    Imagen fabrica(const PedidoMaterial& pedido) const;

    // 9-slice puro: esquinas copiadas, bordes y cuerpo repetidos (no
    // estirados). Si la medida no cabe en los cortes, estos se encogen.
    static Imagen recorta(const Imagen& piel, const int cortes[4], int ancho, int alto);
    // El nombre de cada parametro de un material, en el orden a, b, c, d.
    static std::vector<std::string> nombresParametros(Material tipo);

private:
    Result<bool> cargaPiel(const std::string& carpeta, const JsonValue& datos);

    std::vector<Piel> m_pieles;
    std::vector<std::string> m_avisos;
    bool m_cargado = false;
};

}  // namespace Pergamino
