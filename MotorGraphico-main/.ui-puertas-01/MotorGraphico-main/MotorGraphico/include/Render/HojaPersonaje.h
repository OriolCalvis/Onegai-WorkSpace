// APARIENCIA-01 — La hoja de un personaje montado, en la GPU.
//
// Game::MontajePersonajes monta la hoja de 20 frames en la CPU (sin ventana,
// igual que el probador de tools/). Esto la sube a una textura con su atlas
// de 20 regiones y sirve para dos cosas:
//
//   - Vestir a un actor: ponerA() le da la hoja como sprite de personaje y
//     los ciclos de andar bien repartidos (arriba 1-3, derecha 4-6, abajo
//     7-9, izquierda 10-12; cada ciclo empieza en el frame quieto, que es el
//     que Player ensena al pararse). animar() hace lo mismo con cualquier
//     hoja de ese reparto, personaje.png incluida: arregla que el jugador se
//     quedara de espaldas al pararse (ARTE-01). ESTETICA-01: la hoja montada
//     lleva ademas el reposo (13-20), y ponerA() le da al actor los ciclos
//     "idle_<rumbo>" (animarReposo), que Player pone al quedarse parado.
//
//   - Como elemento del HUD, la vista previa de la creacion: un escenario con
//     el personaje grande andando y girando, y debajo sus cuatro lados.
//
// vestir() solo vuelve a montar y subir si la apariencia o lo puesto
// cambiaron: se puede llamar cada fotograma. La textura se crea una vez y se
// reescribe en su sitio, asi que el atlas que tiene un actor sigue valido.
#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "Core/Errors/Result.h"
#include "Core/Math/Vector2.h"
#include "Game/Personajes/BrilloEquipo.h"
#include "Game/Personajes/MontajePersonajes.h"
#include "Render/HudElement.h"

class AnimatedEntity;
class Texture;
class TextureAtlas;

class HojaPersonaje : public IHudElement {
public:
    // blanco: textura 1x1 para pintar el escenario de la vista previa
    // (nullptr = solo el personaje). No propietario.
    explicit HojaPersonaje(Texture* blanco = nullptr);
    ~HojaPersonaje() override;

    // Monta con `montaje` y sube la hoja si algo cambio desde la ultima vez.
    // Ok(true) si la ha rehecho, Ok(false) si ya estaba; Error si no se pudo
    // montar (se queda la anterior y no se reintenta hasta que cambie algo).
    // ESTETICA-01: `brillos` (ranura -> brillo de lo puesto) monta tambien el
    // brillo del equipo: el halo en otra textura con el mismo reparto y las
    // piezas que echan destellos y particulas (lo dibuja AuraJugador).
    Result<bool> vestir(Game::MontajePersonajes& montaje, const Game::Apariencia& apariencia,
                        const std::map<std::string, std::string>& equipo = {},
                        const std::map<std::string, Game::BrilloDePieza>& brillos = {});
    bool lista() const { return m_atlas != nullptr; }
    const Game::Apariencia& apariencia() const { return m_apariencia; }
    TextureAtlas* atlas() const { return m_atlas.get(); }
    const Game::HojaDeBrillo& brillo() const { return m_brillo; }
    TextureAtlas* atlasBrillo() const { return m_brillo.vacia() ? nullptr : m_atlasBrillo.get(); }

    // Le da la hoja a un actor (sprite de personaje, ciclos de andar y de
    // reposo) y lo deja en reposo mirando hacia abajo. Sin hoja lista no hace
    // nada.
    void ponerA(AnimatedEntity& actor, int anchoDibujo, int altoDibujo,
                const Vector2& anclaje) const;
    // Los ciclos de andar de cualquier hoja con este reparto (las horneadas
    // de 12 frames incluidas). Oriol (24/10): el reparto sale de las columnas
    // de la hoja que lleva el actor (40 = en diagonal, la de PixelLab).
    static void animar(AnimatedEntity& actor);
    static void animar(AnimatedEntity& actor, Game::Reparto reparto);
    // ESTETICA-01: los ciclos de reposo ("idle_up", "idle_right", "idle_down",
    // "idle_left"; "idle" es el de abajo) de una hoja montada de 20 frames.
    // No para las horneadas de 12: no tienen esos frames (si para las de 40).
    static void animarReposo(AnimatedEntity& actor);
    static void animarReposo(AnimatedEntity& actor, Game::Reparto reparto);
    // El reparto de la hoja de personaje de un actor (clasico si no tiene).
    static Game::Reparto repartoDe(const AnimatedEntity& actor);
    // Si la hoja del actor trae reposo propio (20 o 40 frames): las de 12 solo tienen el quieto.
    static bool tieneReposo(const AnimatedEntity& actor);
    Game::Reparto reparto() const { return m_reparto; }

    // --- Vista previa en el HUD ---
    void setMarco(const HudTransform& marco) { m_marco = marco; }
    // El paseo vuelve a empezar: abajo, derecha, arriba, izquierda.
    void reiniciarPaseo() { m_inicio = std::chrono::steady_clock::now(); }
    void render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const override;

private:
    // Sube una hoja de 1280x64 (o 2560x64, en diagonal) a su textura (la crea la primera vez).
    static void subir(const imagen::ImagenRGBA& hoja, std::unique_ptr<Texture>& textura,
                      std::unique_ptr<TextureAtlas>& atlas);

    Texture* m_blanco;
    Game::Reparto m_reparto = Game::Reparto::Clasico;
    std::unique_ptr<Texture> m_textura;
    std::unique_ptr<TextureAtlas> m_atlas;
    Game::HojaDeBrillo m_brillo;
    std::unique_ptr<Texture> m_texturaBrillo;
    std::unique_ptr<TextureAtlas> m_atlasBrillo;
    Game::Apariencia m_apariencia;
    std::string m_clave;
    HudTransform m_marco;
    std::chrono::steady_clock::time_point m_inicio;
};
