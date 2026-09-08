#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Engine/Application.h"
#include "TestHarness.h"

#include <set>
#include <stdexcept>

namespace {

// Observa el driver real, no sustituye la Application ni su orden de cierre.
// Vive MAS que Application para observar tambien sus destructores de miembros.
struct DeletionProbe {
    inline static DeletionProbe* active = nullptr;
    PFNGLDELETETEXTURESPROC deleteTextures = nullptr;
    PFNGLDELETEPROGRAMPROC deleteProgram = nullptr;
    GLFWwindow* context = nullptr;
    std::set<GLuint> textures;
    std::set<GLuint> programs;
    int calls = 0;
    int invalidContext = 0;
    int duplicateNames = 0;

    void install() {
        context = glfwGetCurrentContext();
        MOTOR_CHECK(context != nullptr);
        deleteTextures = glad_glDeleteTextures;
        deleteProgram = glad_glDeleteProgram;
        MOTOR_CHECK(deleteTextures != nullptr && deleteProgram != nullptr);
        active = this;
        glad_glDeleteTextures = &observeTextures;
        glad_glDeleteProgram = &observeProgram;
    }

    bool validContext() {
        ++calls;
        if (glfwGetCurrentContext() == context) return true;
        ++invalidContext;
        // No invocar el driver sin contexto: el fallo del test debe ser
        // determinista incluso donde esa llamada seria un crash.
        return false;
    }

    static void APIENTRY observeTextures(GLsizei count, const GLuint* names) {
        auto& probe = *active;
        const bool valid = probe.validContext();
        for (GLsizei i = 0; i < count; ++i) {
            if (names[i] != 0 && !probe.textures.insert(names[i]).second)
                ++probe.duplicateNames;
        }
        if (valid) probe.deleteTextures(count, names);
    }

    static void APIENTRY observeProgram(GLuint name) {
        auto& probe = *active;
        const bool valid = probe.validContext();
        if (name != 0 && !probe.programs.insert(name).second)
            ++probe.duplicateNames;
        if (valid) probe.deleteProgram(name);
    }

    ~DeletionProbe() {
        if (active == this) {
            glad_glDeleteTextures = deleteTextures;
            glad_glDeleteProgram = deleteProgram;
            active = nullptr;
        }
    }
};

void checkProbe(const DeletionProbe& probe, bool expectTextures) {
    MOTOR_CHECK(probe.invalidContext == 0);
    MOTOR_CHECK(probe.duplicateNames == 0);
    // No aprobar si la prueba no llego a crear recursos reales.
    MOTOR_CHECK(!probe.programs.empty());
    if (expectTextures) MOTOR_CHECK(!probe.textures.empty());
    MOTOR_CHECK(glfwGetCurrentContext() == nullptr);
}

MOTOR_TEST_CASE(explicit_shutdown_releases_resources_before_window_and_is_idempotent) {
    DeletionProbe probe;
    int callsAfterShutdown = 0;
    int callsAfterSecondShutdown = 0;
    {
        Application app;
        const auto init = app.init(640, 480, "C-05: cierre explicito");
        if (!init.isOk()) throw std::runtime_error(init.errorMessage());
        probe.install();
        app.shutdown();
        callsAfterShutdown = probe.calls;
        app.shutdown();
        callsAfterSecondShutdown = probe.calls;
    }
    checkProbe(probe, true);
    MOTOR_CHECK(probe.calls == callsAfterShutdown);
    MOTOR_CHECK(probe.calls == callsAfterSecondShutdown);
}

MOTOR_TEST_CASE(destructor_alone_releases_resources_with_their_context_alive) {
    DeletionProbe probe;
    {
        Application app;
        const auto init = app.init(640, 480, "C-05: destructor");
        if (!init.isOk()) throw std::runtime_error(init.errorMessage());
        probe.install();
    }
    checkProbe(probe, true);
}

MOTOR_TEST_CASE(failed_initialization_releases_already_loaded_shader_before_window) {
    DeletionProbe probe;
    {
        Application app;
        const auto init = app.init(640, 480, "C-05: inicio fallido",
                                   "assets/levels/test_level.json",
                                   "assets/objects/__c05_missing_catalog__.json");
        MOTOR_CHECK(!init.isOk());
        MOTOR_CHECK(init.errorMessage().find("Catalogo de objetos:") == 0);
        // El fallo ocurre despues de cargar el shader. El destructor debe
        // limpiar tambien este arranque parcial, sin llamar a run().
        probe.install();
    }
    checkProbe(probe, false);
    MOTOR_CHECK(probe.programs.size() == 1);
    MOTOR_CHECK(probe.textures.empty());
}

MOTOR_TEST_CASE(failed_adventure_releases_manager_textures_before_window) {
    DeletionProbe probe;
    GLint lastTexture = 0;
    {
        Application app;
        const auto init = app.init(640, 480, "C-05: aventura ausente",
                                   "assets/levels/test_level.json",
                                   "assets/objects/test_objects.json",
                                   "assets/adventures/__c05_missing_adventure__.json");
        MOTOR_CHECK(!init.isOk());
        MOTOR_CHECK(init.errorMessage().find("Aventura inicial:") == 0);
        probe.install();
        // Aun no hay texturas procedurales ni renderer: esta textura
        // pertenece al manager, no al HUD que ya se cerraba correctamente.
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
        MOTOR_CHECK(lastTexture != 0);
        MOTOR_CHECK(glIsTexture(static_cast<GLuint>(lastTexture)) == GL_TRUE);
    }
    checkProbe(probe, true);
    MOTOR_CHECK(probe.textures.count(static_cast<GLuint>(lastTexture)) == 1);
    MOTOR_CHECK(probe.programs.size() == 1);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
