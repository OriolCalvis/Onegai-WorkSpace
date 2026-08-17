#include "Core/Math/IsoMath.h"
#include "Render/Camera.h"

#include "Check.h"

#include <cmath>
#include <iostream>

namespace {

bool nearlyEqual(float a, float b, float epsilon = 0.001f) { return std::fabs(a - b) <= epsilon; }

}  // namespace

int main() {
    const float kTileWidth = 64.0f;
    const float kTileHeight = 32.0f;

    // Caso 1: IsoMath::gridToScreen / screenToGrid son inversas exactas en
    // una cuadricula de prueba (grid isometrico de prueba, Fase 1 del
    // Gantt). tileWidth/tileHeight son potencias de dos: la ida y vuelta
    // no acumula error de redondeo en float.
    int roundTripFailures = 0;
    for (int gx = -20; gx <= 20; ++gx) {
        for (int gy = -20; gy <= 20; ++gy) {
            GridCoord original{gx, gy};
            Vector2 screen = IsoMath::gridToScreen(original, kTileWidth, kTileHeight);
            GridCoord back = IsoMath::screenToGrid(screen, kTileWidth, kTileHeight);
            if (back.x != original.x || back.y != original.y) {
                ++roundTripFailures;
            }
        }
    }
    require(roundTripFailures == 0);
    std::cout << "[ISO] round-trip grid<->screen en 41x41 celdas: " << roundTripFailures
              << " fallos (esperado: 0)\n";

    // Sanity check del signo/orientacion: (1,0) cae a la derecha y abajo
    // del origen; (0,1) a la izquierda y abajo (proyeccion 2:1 estandar).
    Vector2 east = IsoMath::gridToScreen(GridCoord{1, 0}, kTileWidth, kTileHeight);
    Vector2 southwest = IsoMath::gridToScreen(GridCoord{0, 1}, kTileWidth, kTileHeight);
    require(east.x > 0.0f && east.y > 0.0f);
    require(southwest.x < 0.0f && southwest.y > 0.0f);
    std::cout << "[ISO] orientacion (1,0)=(" << east.x << "," << east.y << ") (0,1)=("
              << southwest.x << "," << southwest.y << ")\n";

    // Caso 2: Camera::update() suaviza el paneo hacia move() con lerp
    // exponencial, sin overshoot, y converge en un numero finito de pasos.
    Camera camera(800, 600);
    camera.move(Vector2{100.0f, 50.0f});
    require(nearlyEqual(camera.position().x, 0.0f) && nearlyEqual(camera.position().y, 0.0f));

    for (int i = 0; i < 200 && !nearlyEqual(camera.position().x, 100.0f); ++i) {
        camera.update(1.0f / 60.0f);
    }
    require(nearlyEqual(camera.position().x, 100.0f, 0.5f));
    require(nearlyEqual(camera.position().y, 50.0f, 0.5f));
    std::cout << "[CAMERA] tras converger, position() = (" << camera.position().x << ", "
              << camera.position().y << ") (esperado: ~100, ~50)\n";

    // Caso 3: screenToWorld() es la inversa exacta de "que pixel de
    // pantalla ocupa este punto de mundo" para el zoom/posicion actuales.
    camera.setZoom(2.0f);
    Vector2 worldPoint{140.0f, 70.0f};  // position() + (20, 20)
    Vector2 screenPoint{
        800.0f / 2.0f + (worldPoint.x - camera.position().x) * camera.zoom(),
        600.0f / 2.0f + (worldPoint.y - camera.position().y) * camera.zoom(),
    };
    Vector2 recovered = camera.screenToWorld(screenPoint);
    require(nearlyEqual(recovered.x, worldPoint.x));
    require(nearlyEqual(recovered.y, worldPoint.y));
    std::cout << "[CAMERA] screenToWorld(screenToWorld^-1(w)) = (" << recovered.x << ", "
              << recovered.y << ") (esperado: 140, 70)\n";

    // Caso 4: Camera::worldToGrid delega en IsoMath con el tile por
    // defecto (64x32): un punto exactamente sobre la celda (3,-2) debe
    // resolver a esa celda.
    GridCoord target{3, -2};
    Vector2 targetWorld = IsoMath::gridToScreen(target, kTileWidth, kTileHeight);
    GridCoord resolved = camera.worldToGrid(targetWorld);
    require(resolved.x == target.x && resolved.y == target.y);
    std::cout << "[CAMERA] worldToGrid(gridToScreen(3,-2)) = (" << resolved.x << ", " << resolved.y
              << ") (esperado: 3, -2)\n";

    // Caso 5: transitionTo() anima posicion/zoom con una curva de easing
    // en "duration" segundos, no con el lerp exponencial continuo. Con
    // Easing::Linear, a mitad de duration el progreso debe ser
    // exactamente 0.5; al completarse, llega EXACTO al destino (no solo
    // "cerca", a diferencia del lerp exponencial de move()) y
    // isTransitioning() pasa a false.
    Camera cam2(800, 600);
    cam2.transitionTo(Vector2{200.0f, 0.0f}, 2.0f, 1.0f, Camera::Easing::Linear);
    require(cam2.isTransitioning());
    cam2.update(0.5f);  // mitad de la duracion (1.0s)
    require(nearlyEqual(cam2.position().x, 100.0f, 0.01f));
    require(nearlyEqual(cam2.zoom(), 1.5f, 0.01f));
    require(cam2.isTransitioning());
    cam2.update(0.5f);  // completa la duracion
    require(!cam2.isTransitioning());
    require(nearlyEqual(cam2.position().x, 200.0f) && nearlyEqual(cam2.position().y, 0.0f));
    require(nearlyEqual(cam2.zoom(), 2.0f));
    std::cout << "[CAMERA] transitionTo(Linear) llega exacto al destino y termina en duration\n";

    // Caso 6: tras terminar la transicion, move()/update() retoman el
    // paneo continuo normal sin salto (m_targetPosition quedo fijado en
    // el destino de la transicion, ver su comentario).
    cam2.move(Vector2{10.0f, 0.0f});
    for (int i = 0; i < 200 && !nearlyEqual(cam2.position().x, 210.0f); ++i) {
        cam2.update(1.0f / 60.0f);
    }
    require(nearlyEqual(cam2.position().x, 210.0f, 0.5f));
    std::cout << "[CAMERA] paneo continuo tras la transicion converge a (210, 0)\n";

    // Caso 7: EaseInOutQuad no es lineal: arranca mas lento (a t=0.25 el
    // progreso es menor que 0.25) y es simetrica en su punto medio (a
    // t=0.5 el progreso es exactamente 0.5, igual que Linear ahi).
    Camera cam3(800, 600);
    cam3.transitionTo(Vector2{100.0f, 0.0f}, 1.0f, 1.0f, Camera::Easing::EaseInOutQuad);
    cam3.update(0.25f);
    require(cam3.position().x < 25.0f);
    Camera cam4(800, 600);
    cam4.transitionTo(Vector2{100.0f, 0.0f}, 1.0f, 1.0f, Camera::Easing::EaseInOutQuad);
    cam4.update(0.5f);
    require(nearlyEqual(cam4.position().x, 50.0f, 0.01f));
    std::cout << "[CAMERA] EaseInOutQuad: mas lento que lineal al inicio, simetrico en el punto "
                 "medio\n";

    // Caso 8: duration <= 0 es un corte instantaneo: la siguiente
    // update() (con cualquier deltaTime) aplica el destino completo de
    // una vez.
    Camera cam5(800, 600);
    cam5.transitionTo(Vector2{50.0f, 50.0f}, 3.0f, 0.0f);
    cam5.update(1.0f / 60.0f);
    require(!cam5.isTransitioning());
    require(nearlyEqual(cam5.position().x, 50.0f) && nearlyEqual(cam5.position().y, 50.0f));
    require(nearlyEqual(cam5.zoom(), 3.0f));
    std::cout << "[CAMERA] transitionTo(duration<=0) aplica el destino de inmediato\n";

    // Caso 9: re-llamar a transitionTo() mientras una esta en curso
    // reinicia desde la posicion ACTUAL (ya interpolada), no desde el
    // destino de la transicion cancelada: sin salto brusco.
    Camera cam6(800, 600);
    cam6.transitionTo(Vector2{100.0f, 0.0f}, 1.0f, 1.0f, Camera::Easing::Linear);
    cam6.update(0.5f);  // a mitad de camino: position().x ~= 50
    float midway = cam6.position().x;
    require(nearlyEqual(midway, 50.0f, 0.01f));
    cam6.transitionTo(Vector2{0.0f, 0.0f}, 1.0f, 1.0f, Camera::Easing::Linear);
    require(nearlyEqual(cam6.position().x, midway));  // sin salto: sigue en ~50
    std::cout << "[CAMERA] re-transitionTo() a mitad de otra reinicia sin salto (desde " << midway
              << ")\n";

    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
