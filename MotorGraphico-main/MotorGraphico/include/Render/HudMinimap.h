#pragma once

#include "Core/Math/Vector4.h"
#include "Render/HudElement.h"

class GameSession;
class SpriteBatch;
class Texture;
class TileMap;

// Minimapa del HUD: dibuja el TileMap en miniatura (un quad de color por
// celda, via whiteTexture -- mismo mecanismo que HudBar/HudPanel, ver
// HudWidgets.h) mas los marcadores de la partida leidos de GameSession:
// jugador, enemigos vivos y objetos del mundo. Vista cenital (x,y de
// grid directos), NO isometrica: un minimapa cuadrado se lee mejor y
// evita reimplementar la proyeccion de IsoMath para 8x8 pixeles.
//
// No posee nada (mismo criterio de no-propiedad que el resto de widgets,
// ver HudWidgets.h): map/session/whiteTexture deben vivir mas que el
// widget -- en Application todos son miembros con vida de partida
// completa. Lee el estado en cada render(): no hay que "refrescar" el
// minimapa, los enemigos derrotados y pickups recogidos desaparecen
// solos porque GameSession ya no los devuelve.
//
// El tamano de celda se deriva de transform.size / dimensiones del mapa
// (el menor de los dos ejes, celdas cuadradas) y la rejilla se centra
// dentro del rectangulo del transform: un mapa no cuadrado no deforma
// las celdas ni desborda el marco.
class HudMinimap : public IHudElement {
public:
    HudMinimap(const HudTransform& transform, Texture* whiteTexture, const TileMap* map,
               const GameSession* session);

    // Lado maximo, EN CELDAS, de lo que se dibuja. Si el mapa cabe en
    // ese limite se ve entero; si lo supera, se muestra una VENTANA de
    // este tamano centrada en el jugador y pegada a los bordes del mapa
    // cuando esta cerca de ellos (como el minimapa de cualquier RPG con
    // mundo grande).
    //
    // Existe por dos motivos, ambos medidos: un mapa de 64x64 dibujado
    // entero son 4096 quads POR FRAME solo de minimapa, y en un widget
    // de 180px cada celda quedaria a 2.8px -- el jugador seria un punto
    // indistinguible del terreno. Con la ventana, el coste es constante
    // (maxCells^2) sea cual sea el tamano del mapa.
    void setMaxCells(int maxCells);
    int maxCells() const { return m_maxCells; }

    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    HudTransform m_transform;
    Texture* m_whiteTexture;
    const TileMap* m_map;
    const GameSession* m_session;

    // 24x24 celdas: en un widget de 180px son ~7px por celda, donde el
    // marcador del jugador todavia se distingue de un vistazo.
    int m_maxCells = 24;

    Vector4 m_backgroundColor{0.03f, 0.03f, 0.05f, 0.8f};
    Vector4 m_walkableColor{0.25f, 0.28f, 0.35f, 0.9f};
    Vector4 m_blockedColor{0.10f, 0.11f, 0.15f, 0.9f};
    Vector4 m_objectColor{0.95f, 0.85f, 0.25f, 1.0f};
    Vector4 m_enemyColor{0.9f, 0.25f, 0.2f, 1.0f};
    Vector4 m_playerColor{0.3f, 0.95f, 0.4f, 1.0f};
};
