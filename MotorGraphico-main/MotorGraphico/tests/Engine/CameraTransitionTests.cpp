// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/Camera.h"
#include "Core/Math/Vector2.h"

#include "TestHarness.h"

#include <cmath>

MOTOR_TEST_CASE(camera_constructor_sets_initial_values) {
    Camera camera(800, 600);
    MOTOR_CHECK(camera.position().x == 0.0f);
    MOTOR_CHECK(camera.position().y == 0.0f);
    MOTOR_CHECK(camera.zoom() == 1.0f);
    MOTOR_CHECK(camera.viewportWidth() == 800);
    MOTOR_CHECK(camera.viewportHeight() == 600);
}

MOTOR_TEST_CASE(camera_set_zoom_changes_zoom) {
    Camera camera(800, 600);
    camera.setZoom(2.0f);
    MOTOR_CHECK(camera.zoom() == 2.0f);
}

MOTOR_TEST_CASE(camera_transition_to_instant_when_duration_zero) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{100.0f, 200.0f}, 2.0f, 0.0f);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.0f);
    MOTOR_CHECK(std::abs(camera.position().x - 100.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 200.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.zoom() - 2.0f) < 0.001f);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_transition_to_linear_interpolation) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{100.0f, 100.0f}, 1.0f, 1.0f, Camera::Easing::Linear);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.5f);
    MOTOR_CHECK(std::abs(camera.position().x - 50.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 50.0f) < 0.001f);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.5f);
    MOTOR_CHECK(std::abs(camera.position().x - 100.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 100.0f) < 0.001f);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_transition_to_linear_zoom) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{0.0f, 0.0f}, 2.0f, 1.0f, Camera::Easing::Linear);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.5f);
    MOTOR_CHECK(std::abs(camera.zoom() - 1.5f) < 0.001f);
    camera.update(0.5f);
    MOTOR_CHECK(std::abs(camera.zoom() - 2.0f) < 0.001f);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_transition_updates_is_transitioning_flag) {
    Camera camera(800, 600);
    MOTOR_CHECK(!camera.isTransitioning());
    camera.transitionTo(Vector2{100.0f, 100.0f}, 1.0f, 0.5f);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.5f);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_easing_enum_values_exist) {
    // Solo verificamos que los enum valores compilan
    MOTOR_CHECK(static_cast<int>(Camera::Easing::Linear) >= 0);
    MOTOR_CHECK(static_cast<int>(Camera::Easing::EaseInOutQuad) >= 0);
    MOTOR_CHECK(static_cast<int>(Camera::Easing::EaseOutCubic) >= 0);
}


// --- Regresion numerica U3-07.E08 ---
// Valores registrados con el codigo ANTERIOR de Camera.cpp (applyEasing +
// mezcla float propia), destino (110,80) zoom 3 desde (0,0) zoom 1,
// duracion 1. Tolerancia 1e-5; el fotograma de fin es exacto.
namespace {

constexpr float kTolRegresion = 1e-5f;
const Camera::Easing kCurvas[3] = {Camera::Easing::Linear, Camera::Easing::EaseInOutQuad,
                                   Camera::Easing::EaseOutCubic};

struct MuestraT {
    float dt;
    float x;
    float y;
    float zoom;
    bool activa;
};

struct MuestraSecuencia {
    int update;
    float x;
    float zoom;
    bool activa;
};

const MuestraT kMuestrasT[3][5] = {
    {
        {0.0f, 0.0f, 0.0f, 1.0f, true},
        {0.25f, 27.5f, 20.0f, 1.5f, true},
        {0.5f, 55.0f, 40.0f, 2.0f, true},
        {0.75f, 82.5f, 60.0f, 2.5f, true},
        {1.0f, 110.0f, 80.0f, 3.0f, false},
    },
    {
        {0.0f, 0.0f, 0.0f, 1.0f, true},
        {0.25f, 13.75f, 10.0f, 1.25f, true},
        {0.5f, 55.0f, 40.0f, 2.0f, true},
        {0.75f, 96.25f, 70.0f, 2.75f, true},
        {1.0f, 110.0f, 80.0f, 3.0f, false},
    },
    {
        {0.0f, 0.0f, 0.0f, 1.0f, true},
        {0.25f, 63.59375f, 46.25f, 2.15625f, true},
        {0.5f, 96.25f, 70.0f, 2.75f, true},
        {0.75f, 108.28125f, 78.75f, 2.96875f, true},
        {1.0f, 110.0f, 80.0f, 3.0f, false},
    },
};

const MuestraSecuencia kSecuencia[3][7] = {
    {
        {1, 1.83333337f, 1.0333333f, true},
        {15, 27.5f, 1.5f, true},
        {30, 55.0000191f, 2.00000048f, true},
        {45, 82.4999924f, 2.5f, true},
        {59, 108.166634f, 2.96666622f, true},
        {60, 109.999969f, 2.99999952f, true},
        {61, 110.0f, 3.0f, false},
    },
    {
        {1, 0.0611111186f, 1.00111115f, true},
        {15, 13.75f, 1.25f, true},
        {30, 55.0000381f, 2.00000072f, true},
        {45, 96.2499924f, 2.75f, true},
        {59, 109.938889f, 2.99888897f, true},
        {60, 110.0f, 3.0f, true},
        {61, 110.0f, 3.0f, false},
    },
    {
        {1, 5.4088397f, 1.09834254f, true},
        {15, 63.59375f, 2.15625f, true},
        {30, 96.2500153f, 2.75000024f, true},
        {45, 108.28125f, 2.96875f, true},
        {59, 109.999489f, 2.9999907f, true},
        {60, 110.0f, 3.0f, true},
        {61, 110.0f, 3.0f, false},
    },
};

// Fotograma (1-based) en que 60 x (1.0f/60) termina la transicion: el reloj
// float de Camera acumula 0.99999970 tras 60 updates y cierra en el 61.
constexpr int kUpdateFin = 61;

bool casi(float a, float b) { return std::abs(a - b) < kTolRegresion; }

}  // namespace

MOTOR_TEST_CASE(camera_regresion_muestras_de_t_por_curva) {
    for (int e = 0; e < 3; ++e) {
        for (const MuestraT& m : kMuestrasT[e]) {
            Camera camera(800, 600);
            camera.transitionTo(Vector2{110.0f, 80.0f}, 3.0f, 1.0f, kCurvas[e]);
            camera.update(m.dt);
            MOTOR_CHECK(casi(camera.position().x, m.x));
            MOTOR_CHECK(casi(camera.position().y, m.y));
            MOTOR_CHECK(casi(camera.zoom(), m.zoom));
            MOTOR_CHECK(camera.isTransitioning() == m.activa);
        }
    }
}

MOTOR_TEST_CASE(camera_regresion_secuencia_60_updates_y_fotograma_de_fin) {
    for (int e = 0; e < 3; ++e) {
        Camera camera(800, 600);
        camera.transitionTo(Vector2{110.0f, 80.0f}, 3.0f, 1.0f, kCurvas[e]);
        int fin = -1;
        int siguiente = 0;
        for (int i = 1; i <= 62; ++i) {
            camera.update(1.0f / 60.0f);
            if (siguiente < 7 && kSecuencia[e][siguiente].update == i) {
                const MuestraSecuencia& m = kSecuencia[e][siguiente++];
                MOTOR_CHECK(casi(camera.position().x, m.x));
                MOTOR_CHECK(casi(camera.zoom(), m.zoom));
                MOTOR_CHECK(camera.isTransitioning() == m.activa);
            }
            if (fin < 0 && !camera.isTransitioning()) fin = i;
        }
        MOTOR_CHECK(siguiente == 7);
        MOTOR_CHECK(fin == kUpdateFin);
    }
}

MOTOR_TEST_CASE(camera_duracion_negativa_es_corte_instantaneo) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{100.0f, 200.0f}, 2.0f, -3.0f, Camera::Easing::EaseOutCubic);
    camera.update(0.016f);
    MOTOR_CHECK(camera.position().x == 100.0f);
    MOTOR_CHECK(camera.position().y == 200.0f);
    MOTOR_CHECK(camera.zoom() == 2.0f);
    MOTOR_CHECK(!camera.isTransitioning());
}

int main() { return motor_test::runAllTests(); }
