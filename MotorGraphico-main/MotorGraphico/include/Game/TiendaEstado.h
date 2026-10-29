// TIENDA-02 — La tienda como modelo, sin GL: dos pestañas (Comprar y
// Vender), el foco de cada una y el resultado de la última compra o venta.
//
// Qué vende el tendero, cuánto cuesta y cuánto paga siguen siendo de
// GameSession (shopOffers, buy, sell, sellPriceOf): aquí solo se ordena lo
// que se ve y se recuerda por dónde iba el jugador. Lo pintan la hoja de
// pergamino (HUD/Pergamino/TiendaHoja) y, sin tema, la lista de siempre;
// las dos leen las mismas filas, así que no pueden contar cosas distintas.
//
// Sustituye al par m_shopSellOffset/m_sellIndices de Application, que
// mezclaba compra y venta en una sola lista (MODELO_VISUAL.md E29: «la fila
// de venta aparece y mueve el foco»).
#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "Game/InventoryMenuState.h"

class GameSession;

namespace Game {

enum class PestanaTienda { Comprar = 0, Vender = 1 };

// Una fila de la pestaña: una oferta del tendero o un objeto de la mochila
// (agrupado: diez pociones son una fila con «tienes 10»).
struct FilaTienda {
    std::string objectId;
    std::string nombre;
    std::string descripcion;  // la del inventario (describeObjeto)
    ItemCategory categoria = ItemCategory::Other;
    int precio = 0;           // Comprar: lo que cuesta. Vender: lo que paga el tendero.
    int tienes = 0;           // cuántos lleva el héroe en la mochila
    int curaPv = 0;           // una poción de vida: cuánto cura (0 = no cura)
    std::size_t indice = 0;   // Comprar: la oferta. Vender: el primero en la mochila.
};

class TiendaEstado {
public:
    // Cada fotograma, con el modo de la partida: una tienda recién abierta
    // empieza en Comprar, con el foco arriba y sin resultado; cerrada, se
    // olvida (la próxima vez vuelve a empezar).
    void sigue(bool abierta);
    // ← →: de Comprar a Vender y vuelta (sin dar la vuelta por el borde).
    void cambiaPestana(int delta);
    void ponPestana(PestanaTienda pestana);
    // W S: el foco de la pestaña activa, entre 0 y filas - 1. El ratón
    // señala una fila con ponFoco.
    void mueve(int delta, std::size_t filas);
    void ponFoco(std::size_t fila, std::size_t filas);
    // ENTER: compra o vende la fila con foco y deja la línea de estado
    // («Comprado: Poción · –30 de oro · ahora tienes 3»). Sin oro no compra
    // y lo dice («Te faltan 20 de oro.»).
    void confirma(GameSession& sesion);

    PestanaTienda pestana() const { return m_pestana; }
    // El foco de la pestaña activa, acotado a las filas que hay ahora (al
    // vender lo último que quedaba de algo, su fila se va).
    std::size_t foco(std::size_t filas) const;
    const std::string& estado() const { return m_estado; }
    bool estadoError() const { return m_error; }

    // Lo que se ve en una pestaña: Comprar, las ofertas del tendero en su
    // orden; Vender, la mochila agrupada por objeto en el orden en que se
    // cogió, solo lo que este tendero paga.
    static std::vector<FilaTienda> filas(const GameSession& sesion, PestanaTienda pestana);

private:
    PestanaTienda m_pestana = PestanaTienda::Comprar;
    std::array<std::size_t, 2> m_foco{};
    std::string m_estado;
    bool m_error = false;
    bool m_abierta = false;
};

}  // namespace Game
