// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-23  (fase C, 4 j) - El post-processat arriba al joc
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "TestHarness.h"
#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Render/Camera.h"
#include "Render/ColorLUT.h"
#include "Render/DynamicLightMap.h"
#include "Render/FogOfWar.h"
#include "Render/Framebuffer.h"
#include "Render/IRenderable.h"
#include "Render/IsometricRenderer.h"
#include "Render/SpriteBatch.h"
#include "Runtime/RuntimeTileMap.h"

#include <array>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace {
constexpr int size = 64;
using Pixel = std::array<unsigned char, 4>;

struct Context {
    GLFWwindow* window = nullptr;
    Context() {
        if (!glfwInit()) throw std::runtime_error("No se pudo inicializar GLFW");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = glfwCreateWindow(128, 128, "C-23 postprocesado", nullptr, nullptr);
        if (!window) { glfwTerminate(); throw std::runtime_error("No se pudo crear contexto GL"); }
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            glfwDestroyWindow(window); glfwTerminate();
            throw std::runtime_error("No se pudo cargar GL");
        }
        glDisable(GL_DITHER);
    }
    ~Context() { glfwDestroyWindow(window); glfwTerminate(); }
};

std::unique_ptr<Texture> solid(unsigned char r, unsigned char g, unsigned char b) {
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    const Pixel pixel{r, g, b, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return std::make_unique<Texture>(id, 1, 1);
}

struct Pattern : IRenderable {
    Texture* texture;
    bool fail = false;
    explicit Pattern(Texture* value) : texture(value) {}
    int getSortKey() const override { return 0; }
    void render(SpriteBatch& batch) override {
        if (fail) throw std::runtime_error("fallo de render inducido");
        // Asimétrico en Y: cualquier inversión hace fallar el caso de píxeles.
        batch.submit({0, 0}, {64, 32}, {0, 0, 1, 1}, texture, {1, 1.0f / 3, 0, 1});
        batch.submit({0, 32}, {64, 32}, {0, 0, 1, 1}, texture, {0, 1.0f / 3, 1, 1});
    }
};

struct Scene {
    Context context; // vive más que cualquier recurso GL
    ShaderManager shaders;
    std::unique_ptr<Texture> white = solid(255, 255, 255);
    std::unique_ptr<Texture> light = solid(153, 77, 230);
    TextureAtlas atlas{white.get(), 1, 1};
    RuntimeTileMap map;
    Camera camera{size, size};
    FogOfWar fog{1, 1, size, size, size, size};
    ColorLUT lut{4};
    Pattern pattern{white.get()};
    Framebuffer output{96, 96};
    Framebuffer readTarget{8, 8};
    std::unique_ptr<IsometricRenderer> renderer;
    Shader* shader(const std::string& name) {
        auto result = shaders.load(name, "assets/shaders/" + name);
        if (!result.isOk()) throw std::runtime_error(result.errorMessage());
        return result.value();
    }
    Scene() {
        camera.transitionTo({32, 32}, 1, 0);
        camera.update(0);
        fog.reveal({0, 0}, 0);
        fog.updateTexture(camera);
        renderer = std::make_unique<IsometricRenderer>(&camera, &map, &atlas, shader("sprite"));
        renderer->addToQueue(&pattern);
        renderer->setPostFX(shader("lightmap"), light.get(), size, size);
        renderer->setFog(shader("fog"), &fog, size, size);
        renderer->setLUT(shader("lut"), &lut, size, size);
        renderer->setAmbientColor({0, 0, 0, 1});
        renderer->setFogColor({0.1f, 0.2f, 0.3f, 1});
        output.bind();
        glViewport(0, 0, 96, 96);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(8, 12, size, size);
    }
    void draw() {
        const auto result = renderer->renderFrame();
        if (!result.isOk()) throw std::runtime_error(result.errorMessage());
        MOTOR_CHECK(glGetError() == GL_NO_ERROR);
    }
    Pixel pixel(int x, int y) {
        Pixel value{};
        glReadPixels(x + 8, y + 12, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, value.data());
        return value;
    }
};

void near(Pixel actual, Pixel expected, int tolerance = 2) {
    for (int channel = 0; channel < 4; ++channel) {
        if (std::abs(int(actual[channel]) - int(expected[channel])) > tolerance) {
            throw std::runtime_error("Canal " + std::to_string(channel) + ": obtenido " +
                std::to_string(actual[channel]) + ", esperado " + std::to_string(expected[channel]));
        }
    }
}

// Cuenta llamadas reales a FBO, sin sustituir el driver ni el render.
struct FboProbe {
    inline static FboProbe* active = nullptr;
    PFNGLGENFRAMEBUFFERSPROC gen = glad_glGenFramebuffers;
    PFNGLDELETEFRAMEBUFFERSPROC del = glad_glDeleteFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC bind = glad_glBindFramebuffer;
    PFNGLGETINTEGERVPROC get = glad_glGetIntegerv;
    int generated = 0, deleted = 0, binds = 0, queries = 0;
    FboProbe() {
        active = this;
        glad_glGenFramebuffers = [](GLsizei n, GLuint* names) { active->generated += n; active->gen(n, names); };
        glad_glDeleteFramebuffers = [](GLsizei n, const GLuint* names) { active->deleted += n; active->del(n, names); };
        glad_glBindFramebuffer = [](GLenum t, GLuint f) { ++active->binds; active->bind(t, f); };
        glad_glGetIntegerv = [](GLenum t, GLint* v) {
            if (t == GL_VIEWPORT || t == GL_DRAW_FRAMEBUFFER_BINDING || t == GL_READ_FRAMEBUFFER_BINDING)
                ++active->queries;
            active->get(t, v);
        };
    }
    ~FboProbe() {
        glad_glGenFramebuffers = gen; glad_glDeleteFramebuffers = del;
        glad_glBindFramebuffer = bind; glad_glGetIntegerv = get; active = nullptr;
    }
};

MOTOR_TEST_CASE(each_effect_and_every_combination_preserve_scene_orientation) {
    Scene s;
    s.renderer->setPostFX(s.shader("lightmap"), s.white.get(), size, size);
    s.draw();
    const auto top = s.pixel(16, 48), bottom = s.pixel(16, 16);
    MOTOR_CHECK(top[0] > 240 && top[2] < 10);
    MOTOR_CHECK(bottom[2] > 240 && bottom[0] < 10);
    for (int effects = 1; effects < 8; ++effects) {
        s.renderer->setPostFXEnabled(effects & 1);
        s.renderer->setFogEnabled(effects & 2);
        s.renderer->setLUTEnabled(effects & 4);
        s.draw();
        near(s.pixel(16, 48), top);
        near(s.pixel(16, 16), bottom);
    }
}

MOTOR_TEST_CASE(lighting_then_fog_then_lut_all_contribute_to_the_final_pixel) {
    Scene s;
    s.draw();
    const Pixel baseline = s.pixel(16, 48);
    s.fog.beginFrame(); // celda explorada: intensidad 100/255
    s.fog.updateTexture(s.camera);
    const std::array<double, 3> light{153.0 / 255, 77.0 / 255, 230.0 / 255};
    const std::array<double, 3> fogColor{.1, .2, .3};
    for (int effects = 1; effects < 8; ++effects) {
        s.renderer->setPostFXEnabled(effects & 1);
        s.renderer->setFogEnabled(effects & 2);
        s.renderer->setLUTEnabled(effects & 4);
        Pixel expected{0, 0, 0, 255};
        for (int c = 0; c < 3; ++c) {
            double v = baseline[c] / 255.0;
            if (effects & 1) v = std::round(v * light[c] * 255) / 255;
            if (effects & 2) v = std::round((fogColor[c] * (155.0 / 255) + v * (100.0 / 255)) * 255) / 255;
            if (effects & 4) v = std::round(v * 3) / 3;
            expected[c] = static_cast<unsigned char>(std::round(v * 255));
        }
        s.draw();
        near(s.pixel(16, 48), expected);
    }
}

MOTOR_TEST_CASE(dynamic_light_stays_at_its_world_position) {
    Scene s;
    DynamicLightMap light(size, size);
    light.addLight({{16, 8}, 8, 1, {1, 1, 1, 1}});
    light.updateTexture(s.camera);
    s.renderer->setPostFX(s.shader("lightmap"), light.texture(), size, size);
    s.renderer->setPostFXEnabled(true);
    s.draw();
    MOTOR_CHECK(s.pixel(16, 55)[0] > 180);
    MOTOR_CHECK(s.pixel(16, 8)[2] < 10);
    light.lights()[0].worldPos = {16, 56};
    light.updateTexture(s.camera);
    s.draw();
    MOTOR_CHECK(s.pixel(16, 7)[2] > 180);
    MOTOR_CHECK(s.pixel(16, 55)[0] < 10);
}

MOTOR_TEST_CASE(disabled_effects_do_not_allocate_bind_or_query_framebuffers) {
    Scene s;
    FboProbe probe;
    s.renderer->setFog(s.shader("fog"), &s.fog, size, size);
    s.renderer->setPostFX(s.shader("lightmap"), s.light.get(), size, size);
    s.renderer->setLUT(s.shader("lut"), &s.lut, size, size);
    s.draw();
    MOTOR_CHECK(probe.generated == 0 && probe.binds == 0 && probe.queries == 0);
    s.renderer->setFogEnabled(true);
    s.draw();
    MOTOR_CHECK(probe.generated == 1);
    s.renderer->setFogEnabled(false);
    const int binds = probe.binds, queries = probe.queries;
    s.draw();
    MOTOR_CHECK(probe.generated == 1 && probe.binds == binds && probe.queries == queries);
}

MOTOR_TEST_CASE(framebuffers_are_reused_resized_and_released_on_level_reset) {
    Scene s;
    FboProbe probe;
    s.renderer->setPostFXEnabled(true);
    s.renderer->setFogEnabled(true);
    s.draw();
    MOTOR_CHECK(probe.generated == 2);
    s.renderer->setFog(s.shader("fog"), &s.fog, size, size);
    s.draw();
    MOTOR_CHECK(probe.generated == 2 && probe.deleted == 0);
    s.renderer->setFog(s.shader("fog"), &s.fog, 80, 80);
    s.draw();
    MOTOR_CHECK(probe.generated == 4 && probe.deleted == 2);
    s.renderer->clearPostProcessing();
    MOTOR_CHECK(probe.deleted == 4);
    MOTOR_CHECK(!s.renderer->fogEnabled() && !s.renderer->postFXEnabled() && !s.renderer->lutEnabled());
    s.renderer->clearPostProcessing();
    s.draw();
    MOTOR_CHECK(probe.generated == 4 && probe.deleted == 4);
    near(s.pixel(16, 48), {255, 85, 0, 255});
}

void checkTargets(GLint draw, GLint read, const std::array<GLint, 4>& viewport) {
    GLint actual = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &actual); MOTOR_CHECK(actual == draw);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &actual); MOTOR_CHECK(actual == read);
    std::array<GLint, 4> result{};
    glGetIntegerv(GL_VIEWPORT, result.data()); MOTOR_CHECK(result == viewport);
}

MOTOR_TEST_CASE(external_framebuffers_and_viewport_survive_success_and_render_failure) {
    Scene s;
    GLint draw = 0, read = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
    s.readTarget.bind();
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(draw));
    const std::array<GLint, 4> viewport{8, 12, size, size};
    s.renderer->setFogEnabled(true);
    s.renderer->setPostFXEnabled(true);
    s.draw();
    checkTargets(draw, read, viewport);
    s.pattern.fail = true;
    MOTOR_CHECK(!s.renderer->renderFrame().isOk());
    checkTargets(draw, read, viewport);
    MOTOR_CHECK(glGetError() == GL_NO_ERROR);
}

MOTOR_TEST_CASE(invalid_effect_viewport_returns_error_without_losing_the_destination) {
    Scene s;
    GLint draw = 0, read = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
    s.renderer->setFog(s.shader("fog"), &s.fog, 0, size);
    s.renderer->setFogEnabled(true);
    MOTOR_CHECK(!s.renderer->renderFrame().isOk());
    checkTargets(draw, read, {8, 12, size, size});
    s.renderer->setFogEnabled(false);
    s.draw();
}
}  // namespace

int main() { return motor_test::runAllTests(); }
