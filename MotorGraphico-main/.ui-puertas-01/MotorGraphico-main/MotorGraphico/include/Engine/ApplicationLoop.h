#pragma once

#include <cstddef>
#include <functional>

// RUN-01 — Núcleo del bucle de aplicación, deliberadamente independiente de
// GLFW, OpenGL y de la lógica del juego. Recibe las operaciones que necesita
// por inyección, lo cual deja el orden del ciclo verificable sin ventana.
namespace Engine {

struct ApplicationLoopSettings {
    // La simulación avanza a pasos constantes: combate, IA, temporizadores y
    // animación reciben el mismo dt aunque el render se ejecute a otra tasa.
    float fixedStepSeconds = 1.0f / 60.0f;

    // Evita que una pausa de depurador o una ventana minimizada inyecte un
    // salto arbitrario de tiempo en la simulación.
    float maxFrameSeconds = 0.10f;

    // Límite por frame contra la "spiral of death". El tiempo que no cabe se
    // descarta explícitamente y queda informado en ApplicationLoopFrame.
    std::size_t maxFixedUpdatesPerFrame = 6;
};

struct ApplicationLoopFrame {
    bool rendered = false;
    bool inputProcessed = false;
    std::size_t fixedUpdates = 0;
    float acceptedFrameSeconds = 0.0f;
    float droppedSimulationSeconds = 0.0f;
    // Fracción [0, 1) pendiente del siguiente paso fijo. El renderizador
    // podrá usarla para interpolar la vista cuando los componentes visuales
    // conserven estado previo y actual.
    float interpolationAlpha = 0.0f;
};

struct ApplicationLoopServices {
    // Debe reflejar tanto la salida solicitada por la plataforma como la del
    // juego. Se consulta antes y después de los puntos que pueden pedir parar.
    std::function<bool()> shouldKeepRunning;
    std::function<void()> pollEvents;
    std::function<void()> processInput;
    std::function<void(float)> fixedUpdate;
    std::function<void(float)> render;
};

class ApplicationLoop {
public:
    explicit ApplicationLoop(ApplicationLoopSettings settings = {});

    const ApplicationLoopSettings& settings() const { return m_settings; }
    void reset();

    // Ejecuta un frame con el delta real recibido por la plataforma. Las
    // funciones de ApplicationLoopServices son obligatorias; si falta alguna,
    // el frame no se ejecuta para no ocultar una integración incompleta.
    ApplicationLoopFrame tick(float realFrameSeconds, const ApplicationLoopServices& services);

private:
    ApplicationLoopSettings m_settings;
    double m_accumulatorSeconds = 0.0;
};

}  // namespace Engine
