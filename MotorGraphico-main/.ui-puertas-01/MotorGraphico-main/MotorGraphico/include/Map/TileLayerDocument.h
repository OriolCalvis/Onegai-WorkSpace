#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"

// Datos de autoría de una capa de tiles. La cuadrícula se conserva plana en
// orden fila-major para evitar exponer la triple indexación del TileMap de
// runtime. Un GID 0 representa una celda vacía, igual que en TMX.
class TileLayerDocument {
public:
    TileLayerDocument() = default;
    TileLayerDocument(int width, int height, std::vector<int> gids);

    int width() const { return m_width; }
    int height() const { return m_height; }
    const std::vector<int>& gids() const { return m_gids; }

    // --- C-12: mutacion ---
    //
    // El documento era de escritura unica: se construia entero y ya no se
    // podia tocar. Mientras siguiera asi, el editor no podia migrarse a
    // el aunque quisiera -- pintar una celda obligaba a reconstruir la
    // capa entera.
    //
    // Todas las mutaciones son todo-o-nada y devuelven Result: una
    // coordenada fuera de la cuadricula o un GID negativo se rechazan sin
    // tocar nada, en vez de dejar la capa a medias. Fuera de rango NO es
    // una excepcion: pintar es una accion de usuario y el editor tiene
    // que poder decir "ahi no" sin que reviente la partida.
    int gidAt(int x, int y) const;
    Result<bool> setGid(int x, int y, int gid);
    // Rellena un rectangulo inclusivo. Un area vacia (ver
    // GridBounds::isEmpty) no es un error: no pinta nada.
    Result<bool> fill(int minX, int minY, int maxX, int maxY, int gid);

    Result<bool> validate() const;

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<int> m_gids;
};
