// C-19 — Una sola funcion de culling.
//
// Habia dos, copia literal la una de la otra: el visibleRangeFor()
// anonimo de IsometricRenderer.cpp y TileMap::visibleRange(). Esta prueba
// es la red de seguridad de la consolidacion: lleva DENTRO una version
// historica congelada de comportamiento y comprueba que la funcion unica
// (VisibleGridRange) da exactamente el mismo rango sobre diez camaras y
// varios tamanos de mapa. Si la consolidacion hubiera cambiado el
// comportamiento -- una franja de mas o de menos al borde de la pantalla,
// que a ojo no se ve -- esto lo dice.
//
// Unit y sin GL: Camera y RuntimeTileMap son GL-free; el mapa se compila
// desde un MapDocument, no desde un TMX en disco.

#include "TestHarness.h"

#include "Map/MapDocument.h"
#include "Render/Camera.h"
#include "Render/Culling.h"
#include "Runtime/RuntimeTileMapBuilder.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

template <class Id>
Id id(const char* value) {
    const Result<Id> result = Id::parse(value);
    if (!result.isOk()) {
        throw std::runtime_error(result.errorMessage());
    }
    return result.value();
}

// La implementacion HISTORICA de TileMap::visibleRange antes de C-19. Es un
// oraculo de equivalencia, no una copia byte a byte: firma y construccion del
// resultado se adaptaron al arnes. No se cambia para acompanar produccion.
GridBounds legacyVisibleRange(const RuntimeTileMap& map, const Camera& camera) {
    if (map.getWidth() <= 0 || map.getHeight() <= 0) {
        return GridBounds{};
    }

    const std::array<Vector2, 4> corners = {
        camera.screenToWorld(Vector2{0.0f, 0.0f}),
        camera.screenToWorld(Vector2{static_cast<float>(camera.viewportWidth()), 0.0f}),
        camera.screenToWorld(Vector2{0.0f, static_cast<float>(camera.viewportHeight())}),
        camera.screenToWorld(Vector2{static_cast<float>(camera.viewportWidth()),
                                     static_cast<float>(camera.viewportHeight())}),
    };

    int minX = map.screenToGrid(corners[0]).x;
    int maxX = minX;
    int minY = map.screenToGrid(corners[0]).y;
    int maxY = minY;
    for (int i = 1; i < 4; ++i) {
        const GridCoord grid = map.screenToGrid(corners[i]);
        minX = std::min(minX, grid.x);
        maxX = std::max(maxX, grid.x);
        minY = std::min(minY, grid.y);
        maxY = std::max(maxY, grid.y);
    }

    GridBounds bounds;
    bounds.minX = std::max(0, minX - 1);
    bounds.maxX = std::min(map.getWidth() - 1, maxX + 1);
    bounds.minY = std::max(0, minY - 1);
    bounds.maxY = std::min(map.getHeight() - 1, maxY + 1);
    return bounds;
}

bool same(const GridBounds& a, const GridBounds& b) {
    return a.minX == b.minX && a.maxX == b.maxX && a.minY == b.minY && a.maxY == b.maxY;
}

RuntimeTileMap makeMap(int width, int height) {
    MapDocument document(id<ids::MapId>("culling"), "Culling", width, height, 64, 32);
    std::vector<int> gids(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 1);
    document.addLayer(MapLayer::tiles({id<ids::LayerId>("suelo"), "Suelo", true, false, 1.0f, 10},
                                      TileLayerDocument(width, height, std::move(gids))));
    RuntimeTileMapBuilder builder;
    Result<RuntimeTileMap> result = builder.build(document);
    if (!result.isOk()) {
        throw std::runtime_error(result.errorMessage());
    }
    return std::move(result.value());
}

struct CameraSetup {
    int viewportWidth;
    int viewportHeight;
    Vector2 position;
    float zoom;
};

// Diez camaras: viewports distintos, la camara dentro y muy fuera del
// mapa, y zooms de muy alejado a muy cerca.
const std::vector<CameraSetup>& setups() {
    static const std::vector<CameraSetup> data = {
        {1280, 720, {0.0f, 0.0f}, 1.0f},
        {1280, 720, {500.0f, 300.0f}, 1.0f},
        {1280, 720, {-800.0f, -600.0f}, 1.0f},
        {800, 600, {0.0f, 0.0f}, 0.25f},
        {800, 600, {120.0f, 90.0f}, 4.0f},
        {1920, 1080, {2000.0f, 2000.0f}, 1.0f},
        {1920, 1080, {-2000.0f, 1500.0f}, 0.5f},
        {640, 480, {64.0f, 32.0f}, 2.0f},
        {640, 480, {0.0f, 0.0f}, 8.0f},
        {320, 240, {-50.0f, 400.0f}, 0.75f},
    };
    return data;
}

}  // namespace

// El nucleo del paquete: la funcion unica coincide con la historica sobre
// diez camaras y cuatro tamanos de mapa (40 combinaciones).
MOTOR_TEST_CASE(la_funcion_unica_da_el_mismo_rango_que_la_historica) {
    const int sizes[][2] = {{8, 8}, {32, 16}, {64, 64}, {3, 40}};

    int compared = 0;
    for (const auto& size : sizes) {
        const RuntimeTileMap map = makeMap(size[0], size[1]);
        for (const CameraSetup& setup : setups()) {
            Camera camera(setup.viewportWidth, setup.viewportHeight);
            camera.setZoom(setup.zoom);
            camera.move(setup.position);
            camera.update(1000.0f);  // el lerp llega al destino de una vez

            MOTOR_CHECK(same(VisibleGridRange(map, camera), legacyVisibleRange(map, camera)));
            ++compared;
        }
    }
    MOTOR_CHECK(compared == 40);
}

// Un mapa vacio devuelve el rango vacio y no un rango negativo: es el
// caso limite que las dos copias ya trataban y que no se puede perder.
MOTOR_TEST_CASE(un_mapa_sin_dimensiones_da_rango_vacio) {
    const RuntimeTileMap vacio;
    Camera camera(1280, 720);

    const GridBounds bounds = VisibleGridRange(vacio, camera);
    MOTOR_CHECK(same(bounds, GridBounds{}));
    MOTOR_CHECK(same(bounds, legacyVisibleRange(vacio, camera)));
}

// El rango nunca se sale del mapa, por lejos que este la camara: es la
// invariante de la que depende getTile(layer, x, y) dentro del bucle de
// dibujado.
MOTOR_TEST_CASE(el_rango_siempre_cae_dentro_del_mapa) {
    const RuntimeTileMap map = makeMap(16, 12);

    for (const CameraSetup& setup : setups()) {
        Camera camera(setup.viewportWidth, setup.viewportHeight);
        camera.setZoom(setup.zoom);
        camera.move(setup.position);
        camera.update(1000.0f);

        const GridBounds bounds = VisibleGridRange(map, camera);
        for (int y = bounds.minY; y <= bounds.maxY; ++y) {
            for (int x = bounds.minX; x <= bounds.maxX; ++x) {
                MOTOR_CHECK(x >= 0 && x < map.getWidth());
                MOTOR_CHECK(y >= 0 && y < map.getHeight());
            }
        }
    }
}

int main() { return motor_test::runAllTests(); }
