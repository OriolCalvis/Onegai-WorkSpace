// PERGAMINO-01 — Las seis hojas del cuaderno, rellenas desde la partida.
//
// Traduce los modelos del juego (InventoryScreen, InventoryMenuState,
// FichaDatos, DiarioDatos, CodiceDatos) a PaginaCuaderno. Solo lee: quien
// decide que se puede usar o equipar y por que no sigue siendo
// InventoryMenuState, y su motivo es el que se pinta. La interfaz no
// inventa (MODELO_VISUAL.md §2.6): lo que no existe en los datos no sale,
// y Acciones muestra un estado vacio honesto mientras no haya habilidades
// de exploracion.
#pragma once

#include <map>
#include <string>

#include "Core/Errors/Result.h"
#include "HUD/Pergamino/Cuaderno.h"
#include "HUD/Pergamino/PaginaCuaderno.h"

class GameSession;

namespace Game {
enum class ItemCategory;
class InventoryScreen;
struct DiarioDatos;
struct CodiceDatos;
struct FichaDatos;
}  // namespace Game

namespace Pergamino {

// Nombres de las siete elecciones de historia (vinculo, miedo...), leidos
// de assets/catalogs/backgrounds.json: la ficha guarda ids y la hoja debe
// pintar el nombre del canon («La balanza de la familia»), no el id.
class NombresHistoria {
public:
    Result<bool> carga(const std::string& rutaBackgrounds);
    std::string nombre(const std::string& id) const;

private:
    std::map<std::string, std::string> m_nombres;
};

struct FuentesCuaderno {
    GameSession* sesion = nullptr;
    Game::InventoryScreen* pantalla = nullptr;
    const Game::DiarioDatos* diario = nullptr;
    const Game::CodiceDatos* codice = nullptr;
    const NombresHistoria* nombres = nullptr;
    int cicatrices = 0;  // AV13-33: las de este personaje (Game::Cicatrices::de); 0 = ninguna
};

// La hoja en la que esta el modelo (InventoryScreen::focus()). Tambien le
// dice al modelo cuantas filas tienen Misiones y Codice, que no son suyas.
PaginaCuaderno paginaActual(const FuentesCuaderno& f);
CabeceraCuaderno cabeceraCuaderno(const FuentesCuaderno& f);

// «Neutral (0)», «Héroe (82)»: el tramo con tilde y el valor.
std::string textoMoral(int valor);

// AV13-33 — «Cicatrices: 2» en el estado de la ficha; con ninguna, no hay fila.
void anotaCicatrices(std::vector<DatoPagina>& estado, int cuantas);

// TIENDA-02 — El icono y la insignia de cada categoria de objeto (la hoja
// del inventario y la de la tienda dicen lo mismo de la misma pocion).
const char* iconoCategoria(Game::ItemCategory c);
const char* insigniaCategoria(Game::ItemCategory c);

}  // namespace Pergamino
