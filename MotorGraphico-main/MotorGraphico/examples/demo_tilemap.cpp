// Fase 2 del Gantt: "mapas y tiles". Verifica TileMap::loadFromFile()
// sobre un TMX real (formato Tiled) y TextureAtlas, sin necesitar GL ni
// ventana: Texture es un simple struct RAII cuyo constructor no llama a
// ningun GL (solo Texture::bind() lo hace, y no se usa aqui), asi que
// basta con un Texture "falso" (glID=0: el destructor no intenta
// glDeleteTextures) para probar la aritmetica de TextureAtlas.
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/Texture.h"
#include "Render/Camera.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {

bool nearlyEqual(float a, float b, float epsilon = 0.0001f) { return std::fabs(a - b) <= epsilon; }

// Camera solo expone move()+update() (paneo suavizado): para posicionarla
// exactamente en un punto en un test, hay que dejar que el lerp converja.
void placeCameraAt(Camera& camera, const Vector2& target) {
    camera.move(target - camera.position());
    for (int i = 0; i < 300 && !nearlyEqual(camera.position().x, target.x, 0.01f); ++i) {
        camera.update(1.0f / 60.0f);
    }
}

}  // namespace

int main() {
    // Caso 1: carga de un TMX valido (assets/maps/test_map.tmx: 4x3,
    // tileset embebido de 64x32, GID 2 marcado con collision=true).
    TileMap map;
    auto result = map.loadFromFile("assets/maps/test_map.tmx");
    require(result.isOk());
    std::cout << "[TMX] test_map.tmx cargado: " << map.getWidth() << "x" << map.getHeight() << ", "
              << map.getLayerCount() << " capa(s)\n";
    require(map.getWidth() == 4);
    require(map.getHeight() == 3);
    require(map.getLayerCount() == 1);

    // Caso 2: contenido de las celdas (CSV de la capa "suelo"):
    //   1,1,1,1,
    //   1,2,2,1,
    //   1,1,1,1
    // GID 1 = sin colision, GID 2 = con colision (tile id=1 del tileset,
    // firstgid=1 -> GID 1+1=2).
    const TileMap& constMap = map;  // getTile() const: sin side effects para los assert de abajo
    const Tile& corner = constMap.getTile(0, 0, 0);
    const Tile& wall1 = constMap.getTile(0, 1, 1);
    const Tile& wall2 = constMap.getTile(0, 2, 1);
    const Tile& otherCorner = constMap.getTile(0, 3, 2);

    require(corner.getTilesetID() == 1 && !corner.hasCollision());
    require(wall1.getTilesetID() == 2 && wall1.hasCollision());
    require(wall2.getTilesetID() == 2 && wall2.hasCollision());
    require(otherCorner.getTilesetID() == 1 && !otherCorner.hasCollision());
    std::cout << "[TMX] colision en (1,1) y (2,1): " << wall1.hasCollision() << ", "
              << wall2.hasCollision() << " (esperado: 1, 1)\n";

    // Caso 3: gridToScreen/screenToGrid usan el tilewidth/tileheight del
    // propio TMX (64x32), no un valor por defecto ajeno al mapa.
    GridCoord cell{2, 1};
    Vector2 screen = map.gridToScreen(cell);
    GridCoord back = map.screenToGrid(screen);
    require(back.x == cell.x && back.y == cell.y);
    std::cout << "[TMX] gridToScreen(2,1) = (" << screen.x << ", " << screen.y
              << "), round-trip OK\n";

    // Caso 4: getTile() fuera de rango lanza (bug del llamador, no un
    // Result: ver comentario en TileMap.h).
    bool threw = false;
    try {
        map.getTile(0, 99, 99);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    require(threw);
    std::cout << "[TMX] getTile() fuera de rango lanza std::out_of_range: " << threw << "\n";

    // Caso 5: TMX invalido (menos celdas de las declaradas) -> Result
    // controlado, no crash ni mapa a medias.
    TileMap invalidMap;
    auto invalidResult = invalidMap.loadFromFile("assets/maps/test_map_invalid.tmx");
    require(!invalidResult.isOk());
    std::cout << "[TMX] error esperado: " << invalidResult.errorMessage() << "\n";

    // Caso 6: fichero inexistente -> tambien Result::Error, no excepcion
    // visible para el llamador.
    TileMap missingMap;
    auto missingResult = missingMap.loadFromFile("assets/maps/no_existe.tmx");
    require(!missingResult.isOk());
    std::cout << "[TMX] error esperado (fichero inexistente): " << missingResult.errorMessage()
              << "\n";

    // Caso 7: TextureAtlas sobre un atlas "falso" 16x8 con dos regiones
    // de 8x8 una al lado de la otra (columnas 0 y 1, fila 0).
    Texture fakeAtlasTexture(/*glID=*/0, /*width=*/16, /*height=*/8);
    TextureAtlas atlas(&fakeAtlasTexture, /*tileWidth=*/8, /*tileHeight=*/8);
    atlas.defineRegion(1, 0, 0);
    atlas.defineRegion(2, 1, 0);

    UVRect uv1 = atlas.getUV(1);
    require(nearlyEqual(uv1.u0, 0.0f) && nearlyEqual(uv1.u1, 0.5f));
    require(nearlyEqual(uv1.v0, 0.0f) && nearlyEqual(uv1.v1, 1.0f));

    UVRect uv2 = atlas.getUV(2);
    require(nearlyEqual(uv2.u0, 0.5f) && nearlyEqual(uv2.u1, 1.0f));

    UVRect uvUndefined = atlas.getUV(999);
    require(nearlyEqual(uvUndefined.u0, 0.0f) && nearlyEqual(uvUndefined.u1, 1.0f));

    std::cout << "[ATLAS] region(1) = (" << uv1.u0 << ", " << uv1.v0 << ")-(" << uv1.u1 << ", "
              << uv1.v1 << "); region(2).u0 = " << uv2.u0 << " (esperado: 0.5)\n";

    // Caso 8: TileMap::visibleRange() (culling, Fase 2 "Culling y
    // batching estatico"). Con un viewport enorme centrado en el mapa,
    // el rango visible debe cubrir el mapa entero (4x3).
    Camera wideCamera(4000, 4000);
    GridBounds fullRange = map.visibleRange(wideCamera);
    require(!fullRange.isEmpty());
    require(fullRange.minX == 0 && fullRange.maxX == map.getWidth() - 1);
    require(fullRange.minY == 0 && fullRange.maxY == map.getHeight() - 1);
    std::cout << "[CULL] viewport grande: rango visible = [" << fullRange.minX << ".."
              << fullRange.maxX << "]x[" << fullRange.minY << ".." << fullRange.maxY
              << "] (esperado: [0..3]x[0..2], el mapa entero)\n";

    // Con un viewport pequeno bien lejos del mapa, no hay interseccion:
    // rango vacio (nada que dibujar esa capa).
    Camera farCamera(800, 600);
    placeCameraAt(farCamera, Vector2{100000.0f, 100000.0f});
    GridBounds emptyRange = map.visibleRange(farCamera);
    require(emptyRange.isEmpty());
    std::cout << "[CULL] camara lejos del mapa: rango vacio = " << emptyRange.isEmpty()
              << " (esperado: 1)\n";

    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
