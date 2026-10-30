// EST-26 (relevo A) — Implementacion. Ver la cabecera.

#include "EditorVistaDeMapa.h"
#include "Editor/MapFraming.h"
#include "Render/CapturaGL.h"

#include "Project/ProjectCatalog.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace Editor {

void MapView::center() { framePending = true; }

void MapView::render(int width, int height) {
    // El docking puede dar un tamano provisional en el primer frame.
    // Recalcular durante ese ajuste; cerrar la peticion al repetir tamano.
    const bool stableSize = target && target->width() == width && target->height() == height;
    if (!stableSize)
        target = std::make_unique<Framebuffer>(width, height);
    camera.setViewportSize(width, height);
    if (framePending) {
        const auto framing = fitMap(runtime.getWidth(), runtime.getHeight(),
            runtime.getTileWidth(), runtime.getTileHeight(), width, height);
        if (!framing.isOk()) throw std::runtime_error(framing.errorMessage());
        camera.transitionTo(framing.value().center, framing.value().zoom, 0);
        camera.update(0);
        framePending = !stableSize;
    }
    target->bind(); glViewport(0, 0, width, height);
    glClearColor(0.075f, 0.086f, 0.105f, 1); glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const auto rendered = renderer->renderFrame();
    target->unbind();
    if (!rendered.isOk()) throw std::runtime_error(rendered.errorMessage());
}

void capture(const std::string& path, int width, int height) {
    // EST-55 — el volcado vive en Render/CapturaGL.h; aqui solo queda la
    // frontera de excepciones del editor, que es la que main.cpp espera.
    const auto volcado = Render::VuelcaFramebufferPPM(path, width, height);
    if (!volcado.isOk()) throw std::runtime_error(volcado.errorMessage());
}

}  // namespace Editor
