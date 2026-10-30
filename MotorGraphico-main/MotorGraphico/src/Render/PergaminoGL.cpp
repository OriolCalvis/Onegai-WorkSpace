// PERGAMINO-01 — ver include/Render/PergaminoGL.h.
#include "Render/PergaminoGL.h"

#include <iostream>
#include <utility>
#include <vector>

#include "Core/Math/IsoMath.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureManager.h"
#include "HUD/Pergamino/Pincel.h"  // COMBATE-01: los golpes
#include "HUD/Pergamino/Tema.h"
#include "Render/SpriteBatch.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Render {

namespace {

// Los materiales que llevan este numero de fotogramas sin pedirse se
// sueltan (el aviso de interaccion cambia de ancho con cada nombre).
constexpr int kFotogramasSinUso = 300;

std::unique_ptr<Texture> subeImagen(const Pergamino::Imagen& img, GLint filtro) {
    if (img.vacia()) return nullptr;
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtro);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtro);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Fila 0 de la imagen = borde de arriba: queda en v = 0, que es justo el
    // espacio de UV de los lienzos (no hace falta voltear, ver dibuja()).
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.ancho, img.alto, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 img.rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return std::make_unique<Texture>(id, img.ancho, img.alto);
}

// Una hoja de render: textura + framebuffer propios. No se usa
// Render::Framebuffer porque la SpriteBatch pide un Texture* y aquel es
// dueno de su textura: envolverla en un Texture la borraria dos veces.
struct HojaRender {
    GLuint fbo = 0;
    std::unique_ptr<Texture> textura;
    int ancho = 0, alto = 0;

    bool crea(int w, int h) {
        libera();
        GLuint id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        textura = std::make_unique<Texture>(id, w, h);
        glGenFramebuffers(1, &fbo);
        GLint previo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
        const bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previo));
        ancho = w;
        alto = h;
        if (!ok) libera();
        return ok;
    }
    void libera() {
        if (fbo != 0) glDeleteFramebuffers(1, &fbo);
        fbo = 0;
        textura.reset();
        ancho = alto = 0;
    }
    ~HojaRender() { libera(); }
};

struct Material {
    std::unique_ptr<Texture> textura;
    int ultimoUso = 0;
};

}  // namespace

struct PergaminoGL::Recursos {
    Texture* letras = nullptr;   // de TextureManager
    Texture* iconos = nullptr;   // de TextureManager
    Texture* blanca = nullptr;   // de Application
    std::map<Pergamino::PedidoMaterial, Material> materiales;
    std::unique_ptr<Texture> minimapa;
    int versionMinimapa = -1;
    std::unique_ptr<Texture> retratos;  // UI-BG3-01
    int versionRetratos = -1;
    HojaRender hojas[2];
    int viva = 0;
    int vueltasVistas = 0;
    int fotograma = 0;
};

namespace {

// Las dos capas del HUD: la baja antes de los widgets de siempre y la alta
// al final, para que el velo de los menus lo tape todo.
class Capa final : public IHudElement {
public:
    Capa(PergaminoGL& dueno, bool alta) : m_dueno(dueno), m_alta(alta) {}
    void render(SpriteBatch& batch, int ancho, int alto) const override {
        const Pergamino::InterfazPergamino& i = m_dueno.interfaz();
        m_dueno.dibuja(batch, m_alta ? i.capaAlta() : i.capaBaja());
        if (m_alta || !m_dueno.listo()) return;
        // COMBATE-01: los golpes, encima de los retales y debajo de los menus.
        Pergamino::GolpesEnPantalla& golpes = m_dueno.golpes();
        golpes.pinta(Pergamino::Pincel(i.letras(), i.iconos()), ancho, alto);
        if (!golpes.lienzo().vacio()) m_dueno.dibuja(batch, golpes.lienzo());
    }
    void update(float segundos) override {
        if (!m_alta) m_dueno.interfaz().avanza(segundos);
        if (!m_alta) m_dueno.golpes().avanza(segundos);  // COMBATE-01
    }

private:
    PergaminoGL& m_dueno;
    bool m_alta;
};

// El rombo del suelo: sin sprite propio, solo sombra (va bajo todos).
class Marca final : public IRenderable {
public:
    explicit Marca(const PergaminoGL& dueno) : m_dueno(dueno) {}
    void render(SpriteBatch&) override {}
    void renderShadow(SpriteBatch& batch, Texture*) override { m_dueno.dibujaMarca(batch); }
    int getSortKey() const override { return 0; }

private:
    const PergaminoGL& m_dueno;
};

// Un anillo con forma de rombo alrededor del centro c: entre el rombo de
// semiejes (a, b)*fuera y el de (a, b)*dentro, en cuatro quads.
void anilloRombo(SpriteBatch& batch, Texture* blanca, Vector2 c, float a, float b, float fuera, float dentro,
                 const Vector4& color) {
    const Vector2 dir[4] = {{0.0f, -1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f}};
    const Vector2 uv[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    for (int i = 0; i < 4; ++i) {
        const Vector2 p = dir[i], q = dir[(i + 1) % 4];
        const Vector2 esquinas[4] = {
            Vector2{c.x + p.x * a * fuera, c.y + p.y * b * fuera},
            Vector2{c.x + q.x * a * fuera, c.y + q.y * b * fuera},
            Vector2{c.x + q.x * a * dentro, c.y + q.y * b * dentro},
            Vector2{c.x + p.x * a * dentro, c.y + p.y * b * dentro},
        };
        batch.submitQuad(esquinas, uv, blanca, color);
    }
}

}  // namespace

PergaminoGL::PergaminoGL()
    : m_recursos(std::make_unique<Recursos>()),
      m_capaBaja(std::make_unique<Capa>(*this, false)),
      m_capaAlta(std::make_unique<Capa>(*this, true)),
      m_marca(std::make_unique<Marca>(*this)),
      m_efectos(std::make_unique<EfectosDeCartas>()) {}

PergaminoGL::~PergaminoGL() = default;

Result<bool> PergaminoGL::carga(TextureManager& texturas, Texture* blanca, Texture* iconos,
                                const std::string& carpeta) {
    // PART-02d: que particulas echa cada carta (sin el fichero, solo las que tienen elemento).
    m_efectos->ponBlanco(blanca);
    if (auto mapa = Game::ParticulasDeCartas::cargar(carpeta + "/textures/particulas_cartas.json"); mapa.isOk())
        m_efectos->ponMapa(std::move(mapa.value()));
    auto letras = texturas.load("letras_pergamino", carpeta + "/textures/letras_pergamino.png");
    if (!letras.isOk()) return Result<bool>::Error("PergaminoGL: " + letras.errorMessage());
    auto interfaz = m_interfaz.carga(carpeta);
    if (!interfaz.isOk()) return interfaz;
    auto pieles = m_banco.carga(carpeta + "/textures/pergamino");
    if (!pieles.isOk()) std::cerr << pieles.errorMessage() << " (materiales procedimentales)\n";
    for (const std::string& aviso : m_banco.avisos())
        std::cerr << "BancoMateriales: " << aviso << " (esa piel sale procedimental)\n";
    m_recursos->letras = letras.value();
    m_recursos->iconos = iconos;
    m_recursos->blanca = blanca;
    return Result<bool>::Ok(true);
}

bool PergaminoGL::listo() const {
    return m_recursos->letras != nullptr && m_recursos->blanca != nullptr && m_interfaz.cargada();
}

void PergaminoGL::prepara(SpriteBatch& batch, Shader& shader, int anchoVista, int altoVista) {
    (void)anchoVista;
    (void)altoVista;
    if (!listo()) return;
    Recursos& r = *m_recursos;
    ++r.fotograma;
    // Materiales pedidos por las tres capas: se fabrican los que falten.
    for (const Pergamino::Lienzo* l :
         {&m_interfaz.capaBaja(), &m_interfaz.capaAlta(), &m_interfaz.hojaViva()}) {
        for (const Pergamino::PedidoMaterial& pedido : l->materiales()) {
            Material& m = r.materiales[pedido];
            if (!m.textura) m.textura = subeImagen(m_banco.fabrica(pedido), GL_NEAREST);
            m.ultimoUso = r.fotograma;
        }
    }
    for (auto it = r.materiales.begin(); it != r.materiales.end();) {
        if (r.fotograma - it->second.ultimoUso > kFotogramasSinUso) it = r.materiales.erase(it);
        else ++it;
    }
    // El terreno del minimapa, cuando cambia el nivel.
    const Pergamino::MinimapaTinta& mini = m_interfaz.minimapa();
    if (mini.version() != r.versionMinimapa) {
        r.minimapa = subeImagen(mini.terreno(), GL_NEAREST);
        r.versionMinimapa = mini.version();
    }
    // UI-BG3-01 — Los retratos de las rejillas, cuando entra uno nuevo.
    const Pergamino::AtlasRetratos& atlas = m_interfaz.retratos();
    if (atlas.version() != r.versionRetratos) {
        r.retratos = subeImagen(atlas.imagen(), GL_NEAREST);
        r.versionRetratos = atlas.version();
    }
    if (!m_interfaz.hojaVisible()) return;
    // La hoja viva del cuaderno, pintada en su textura. Cuando empieza una
    // vuelta, la viva de antes se queda como foto (la hoja que se va).
    const Pergamino::Rect rect = m_interfaz.rectHoja();
    const int w = static_cast<int>(rect.w), h = static_cast<int>(rect.h);
    for (HojaRender& hoja : r.hojas)
        if (hoja.ancho != w || hoja.alto != h) hoja.crea(w, h);
    if (m_interfaz.vueltasDeHoja() != r.vueltasVistas) {
        r.viva = 1 - r.viva;
        r.vueltasVistas = m_interfaz.vueltasDeHoja();
    }
    HojaRender& viva = r.hojas[r.viva];
    if (viva.fbo == 0) return;
    GLint fboPrevio = 0;
    GLint vista[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fboPrevio);
    glGetIntegerv(GL_VIEWPORT, vista);
    glBindFramebuffer(GL_FRAMEBUFFER, viva.fbo);
    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    // El alfa se acumula como cobertura: una letra suavizada sobre el papel
    // no debe dejar el papel medio transparente.
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    shader.use();
    shader.setUniformMat4("uViewProjection",
                          glm::ortho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, -1.0f, 1.0f));
    shader.setUniformInt("uTexture", 0);
    batch.begin();
    dibuja(batch, m_interfaz.hojaViva());
    batch.end();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fboPrevio));
    glViewport(vista[0], vista[1], vista[2], vista[3]);
}

void PergaminoGL::dibujaMarca(SpriteBatch& batch) const {
    if (listo() && m_suelo.visible) dibujaSuelo(batch);
    if (listo()) {
        // AV16-06 — La marca de suelo de cada salida: una flecha de pergamino hacia fuera (abajo en la
        // pantalla) con un filete de tinta y dos peldanos delante, sobre el suelo y bajo los sprites.
        const Vector2 uv[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
        const Vector4 papel = Pergamino::Color::kPapelLuz;
        const Vector4 tinta{Pergamino::Color::kTinta.x, Pergamino::Color::kTinta.y, Pergamino::Color::kTinta.z, 0.85f};
        const auto poligono = [&](const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3,
                                  const Vector4& color) {
            const Vector2 esquinas[4] = {p0, p1, p2, p3};
            batch.submitQuad(esquinas, uv, m_recursos->blanca, color);
        };
        for (const Pergamino::MarcaSalida& sal : m_interfaz.salidas()) {
            const float a = sal.anchoBaldosa * 0.5f, b = sal.altoBaldosa * 0.5f;
            const Vector2 esq = IsoMath::gridToScreen(GridCoord{sal.celdaX, sal.celdaY}, sal.anchoBaldosa, sal.altoBaldosa);
            const Vector2 c{esq.x + a, esq.y + sal.altoBaldosa * sal.pies};
            // Un triangulo es un quad con dos vertices iguales. `e` ensancha el filete de tinta.
            const auto flecha = [&](float e, const Vector4& color) {
                const float asta = a * 0.14f + e, cabeza = a * 0.40f + e;
                poligono(Vector2{c.x - asta, c.y - b * 0.80f - e}, Vector2{c.x + asta, c.y - b * 0.80f - e},
                         Vector2{c.x + asta, c.y - b * 0.05f}, Vector2{c.x - asta, c.y - b * 0.05f}, color);
                poligono(Vector2{c.x - cabeza, c.y - b * 0.05f}, Vector2{c.x + cabeza, c.y - b * 0.05f},
                         Vector2{c.x, c.y + b * 0.75f + e}, Vector2{c.x, c.y + b * 0.75f + e}, color);
            };
            flecha(1.5f, tinta);
            flecha(0.0f, papel);
            // Los peldanos: dos barras de papel con filete, delante de la punta.
            for (int k = 0; k < 2; ++k) {
                const float y = c.y + b * (0.95f + 0.40f * static_cast<float>(k));
                const float w = a * (0.55f - 0.15f * static_cast<float>(k));
                poligono(Vector2{c.x - w - 1.5f, y - 1.5f}, Vector2{c.x + w + 1.5f, y - 1.5f},
                         Vector2{c.x + w + 1.5f, y + 3.5f}, Vector2{c.x - w - 1.5f, y + 3.5f}, tinta);
                poligono(Vector2{c.x - w, y}, Vector2{c.x + w, y}, Vector2{c.x + w, y + 2.0f}, Vector2{c.x - w, y + 2.0f}, papel);
            }
        }
    }
    const Pergamino::MarcaSuelo& m = m_interfaz.marcaSuelo();
    if (!listo() || !m.visible) return;
    // P03a: trazo carmin de 2 px (en vertical; 4 en horizontal, como una
    // linea isometrica de pixel art) con un filete de tinta a cada lado.
    const float a = m.anchoBaldosa * 0.5f, b = m.altoBaldosa * 0.5f;
    const Vector2 esquina = IsoMath::gridToScreen(GridCoord{m.celdaX, m.celdaY}, m.anchoBaldosa, m.altoBaldosa);
    const Vector2 centro{esquina.x + a, esquina.y + m.altoBaldosa * m.pies};
    const float px = 1.0f / b;  // un pixel vertical, en fraccion del semieje
    anilloRombo(batch, m_recursos->blanca, centro, a, b, 1.0f + px, 1.0f - 3.0f * px,
                Pergamino::Color::kRomboContorno);
    anilloRombo(batch, m_recursos->blanca, centro, a, b, 1.0f, 1.0f - 2.0f * px, Pergamino::Color::kRombo);
}

// AV13-10 — El borde del tablero: cada arista, un trazo de tinta de 2 px con un
// halo suave de 4 debajo (como el filete de los retales), sobre el suelo y
// bajo los sprites. Los vertices, con el mismo centro de rombo que la marca.
void PergaminoGL::dibujaSuelo(SpriteBatch& batch) const {
    const Pergamino::SueloCombate& s = m_suelo;
    const float a = s.anchoBaldosa * 0.5f, b = s.altoBaldosa * 0.5f;
    const Vector2 ajuste{a, s.altoBaldosa * s.pies - b};
    const Vector2 uv[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const Vector4 halo{Pergamino::Color::kTinta.x, Pergamino::Color::kTinta.y, Pergamino::Color::kTinta.z, 0.22f};
    const Vector4 trazo{Pergamino::Color::kTinta.x, Pergamino::Color::kTinta.y, Pergamino::Color::kTinta.z, 0.62f};
    // AV13-12: lo que alcanzas y la ruta, rombos llenos y sin borde (la tinta del papel).
    const auto rombo = [&](GridCoord c, const Vector4& color) {
        const Vector2 o = IsoMath::gridToScreen(c, s.anchoBaldosa, s.altoBaldosa) + ajuste;  // vertice de arriba
        const Vector2 esquinas[4] = {o, Vector2{o.x + a, o.y + b}, Vector2{o.x, o.y + 2.0f * b}, Vector2{o.x - a, o.y + b}};
        batch.submitQuad(esquinas, uv, m_recursos->blanca, color);
    };
    // Lo que alcanzas: papel claro con su filete de tinta (sobre la piedra oscura se lee; la tinta sola no).
    const Vector4 alcance{Pergamino::Color::kPapelLuz.x, Pergamino::Color::kPapelLuz.y, Pergamino::Color::kPapelLuz.z, 0.20f};
    const Vector4 filete{Pergamino::Color::kLinea.x, Pergamino::Color::kLinea.y, Pergamino::Color::kLinea.z, 0.40f};
    for (const GridCoord& c : s.alcance) {
        rombo(c, alcance);
        const Vector2 o = IsoMath::gridToScreen(c, s.anchoBaldosa, s.altoBaldosa) + ajuste;
        anilloRombo(batch, m_recursos->blanca, Vector2{o.x, o.y + b}, a, b, 0.96f, 0.90f, filete);
    }
    // AV13-13: hasta donde llega la carta senalada: un velo carmin y un filete por dentro del de papel.
    const Vector4 velo{Pergamino::Color::kCarmin.x, Pergamino::Color::kCarmin.y, Pergamino::Color::kCarmin.z, 0.08f};
    const Vector4 fileteTiro{Pergamino::Color::kCarmin.x, Pergamino::Color::kCarmin.y, Pergamino::Color::kCarmin.z, 0.50f};
    for (const GridCoord& c : s.tiro) {
        rombo(c, velo);
        const Vector2 o = IsoMath::gridToScreen(c, s.anchoBaldosa, s.altoBaldosa) + ajuste;
        anilloRombo(batch, m_recursos->blanca, Vector2{o.x, o.y + b}, a, b, 0.82f, 0.76f, fileteTiro);
    }
    for (const GridCoord& c : s.area)  // AV13-14: lo que pilla el area apuntada
        rombo(c, Vector4{Pergamino::Color::kCarmin.x, Pergamino::Color::kCarmin.y, Pergamino::Color::kCarmin.z, 0.32f});
    for (std::size_t i = 0; i < s.ruta.size(); ++i) {
        const float fuerza = i + 1 == s.ruta.size() ? 0.45f : 0.22f;
        rombo(s.ruta[i], Vector4{Pergamino::Color::kCarmin.x, Pergamino::Color::kCarmin.y, Pergamino::Color::kCarmin.z, fuerza});
    }
    for (const Pergamino::AristaSuelo& r : s.aristas()) {
        const Vector2 p = IsoMath::gridToScreen(r.desde, s.anchoBaldosa, s.altoBaldosa) + ajuste;
        const Vector2 q = IsoMath::gridToScreen(r.hasta, s.anchoBaldosa, s.altoBaldosa) + ajuste;
        for (const auto& [grosor, color] : {std::pair{4.0f, halo}, std::pair{2.0f, trazo}}) {
            const float m = grosor * 0.5f;
            const Vector2 esquinas[4] = {Vector2{p.x, p.y - m}, Vector2{q.x, q.y - m}, Vector2{q.x, q.y + m},
                                         Vector2{p.x, p.y + m}};
            batch.submitQuad(esquinas, uv, m_recursos->blanca, color);
        }
    }
}

void PergaminoGL::dibuja(SpriteBatch& batch, const Pergamino::Lienzo& lienzo) const {
    if (!listo()) return;
    const Recursos& r = *m_recursos;
    const auto& materiales = lienzo.materiales();
    for (const Pergamino::Trazo& t : lienzo.trazos()) {
        Texture* textura = nullptr;
        bool voltea = false;  // PNG cargados con stb (volteados) y texturas de render
        switch (t.fuente) {
            case Pergamino::Fuente::Blanco: textura = r.blanca; break;
            case Pergamino::Fuente::Letras: textura = r.letras; voltea = true; break;
            case Pergamino::Fuente::Iconos: textura = r.iconos; voltea = true; break;
            case Pergamino::Fuente::Minimapa: textura = r.minimapa.get(); break;
            case Pergamino::Fuente::Retratos: textura = r.retratos.get(); break;
            case Pergamino::Fuente::HojaViva: textura = r.hojas[r.viva].textura.get(); voltea = true; break;
            case Pergamino::Fuente::HojaFoto: textura = r.hojas[1 - r.viva].textura.get(); voltea = true; break;
            case Pergamino::Fuente::Material:
                if (t.material >= 0 && static_cast<std::size_t>(t.material) < materiales.size()) {
                    const auto it = r.materiales.find(materiales[static_cast<std::size_t>(t.material)]);
                    if (it != r.materiales.end()) textura = it->second.textura.get();
                }
                break;
        }
        if (textura == nullptr) continue;
        Vector2 uv[4];
        for (int i = 0; i < 4; ++i) uv[i] = Vector2{t.uv[i].x, voltea ? 1.0f - t.uv[i].y : t.uv[i].y};
        batch.submitQuad(t.p, uv, textura, t.color);
    }
}

}  // namespace Render
