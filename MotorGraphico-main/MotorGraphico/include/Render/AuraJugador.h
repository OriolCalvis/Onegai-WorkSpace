// PERGAMINO-01 — El aura de la moral a los pies del jugador.
//
// Oriol (29/09): la moral sale del HUD y se ve en el aspecto del avatar. La
// regla (que tramo, que color, donde va cada particula) es Game/AuraMoral;
// esto la dibuja en el pase de sombras del IsometricRenderer, bajo todos los
// sprites, como una sombra blob con color: halo dorado que se suma a la luz
// para el heroe y el justo, humo rojo oscuro que tapa para el canalla y el
// villano. Las motas de luz del heroe suben por delante del sprite.
// MORAL-01 (Oriol, 08/10): siete tramos; el heroe y el villano llevan alas
// (celestiales o demoniacas) detras del sprite, y el villano, motas de sombra.
// MORAL-02 (Oriol, 08/10): quince grados a cada lado; las alas son sprites de
// energia del atlas assets/textures/alas_moral.png (ocho niveles, de 1 a 8
// hojas por ala), que se cargan la primera vez que hacen falta: detras del
// sprite de frente y de perfil, delante de espaldas; baten despacio y la
// energia corre (cuatro frames). Salvador y Desgraciado, rayos de luz o de
// sombra detras del cuerpo. Lo de detras del cuerpo lo dibuja una segunda
// pieza de la cola (Detras), justo antes que el jugador: asi lo que queda
// delante del jugador tambien tapa las alas, y lo que queda detras, no.
//
// Reutiliza la textura radial de la sombra blob (BlobShadow.h): el color lo
// pone el tinte. Sigue al jugador por su posicion visible (casilla + paso
// interpolado), no por la logica: el aura no se adelanta al sprite.
//
// ESTETICA-01: tambien el brillo del equipo puesto (Game/Personajes/
// BrilloEquipo): el halo, que late y suma luz encima del sprite con el mismo
// frame que este, y los destellos y particulas de cada pieza, pixel a pixel.
#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "Core/Math/Vector2.h"
#include "Game/AuraMoral.h"
#include "Game/Personajes/BrilloEquipo.h"
#include "Render/IRenderable.h"

class Entity;
class HojaPersonaje;
class IsometricRenderer;
class Texture;

class AuraJugador final : public IRenderable {
public:
    AuraJugador(const Entity* jugador, int anchoBaldosa, int altoBaldosa);

    // Recalcula el aspecto con el valor de la moral (-100..100).
    void ponMoral(int valor) { m_aspecto = Game::aspectoAura(valor); }
    const Game::AspectoAura& aspecto() const { return m_aspecto; }

    // ESTETICA-01: la hoja del jugador (con su brillo) y una textura blanca de
    // 1x1 para las particulas. nullptr = sin brillo. No propietario.
    void ponBrillo(const HojaPersonaje* hoja, Texture* blanco) {
        m_hoja = hoja;
        m_blanco = blanco;
    }

    void render(SpriteBatch& batch) override;
    void renderShadow(SpriteBatch& batch, Texture* texturaSombra) override;
    // Justo despues del jugador: las motas quedan por delante de su sprite.
    int getSortKey() const override;

    // MORAL-02: entra (y sale) de la cola con su pieza de detras.
    void encolar(IsometricRenderer& renderer);
    void desencolar(IsometricRenderer& renderer);

private:
    // Lo que va detras del jugador (los rayos y las alas de frente y de
    // perfil): se dibuja justo antes que el.
    class Detras final : public IRenderable {
    public:
        explicit Detras(AuraJugador& aura) : m_aura(aura) {}
        void render(SpriteBatch& batch) override;
        int getSortKey() const override;

    private:
        AuraJugador& m_aura;
    };

    const Entity* m_jugador;  // no propietario: el Player de Application
    int m_anchoBaldosa;
    int m_altoBaldosa;
    Game::AspectoAura m_aspecto;
    Texture* m_textura = nullptr;  // la de la sombra blob (la recibe renderShadow)
    std::chrono::steady_clock::time_point m_inicio = std::chrono::steady_clock::now();
    const HojaPersonaje* m_hoja = nullptr;  // ESTETICA-01
    Texture* m_blanco = nullptr;
    std::vector<Game::Destello> m_destellos;  // se reusa cada fotograma
    std::unique_ptr<Texture> m_texturaAlas;   // MORAL-02: el atlas de las alas (se carga al usarlo)
    bool m_alasCargadas = false;              // ya se intento (si falta el PNG, no se reintenta)
    std::vector<Game::PuntoAla> m_puntosAla[Game::kFilasAla];  // donde nacen las chispas de cada fila del atlas
    Detras m_detras{*this};

    void renderMotas(SpriteBatch& batch, float segundos);
    void renderBrillo(SpriteBatch& batch, float segundos);
    // MORAL-02: delante = true, las de espaldas (encima del sprite); false, las de frente y de perfil.
    void renderAlas(SpriteBatch& batch, float segundos, bool delante);
    void renderRayos(SpriteBatch& batch, float segundos);  // MORAL-02
    void renderChispas(SpriteBatch& batch, float segundos, const Game::AlaEnSprite* alas, int n, float sx, float sy,
                       const Vector2& esquina);  // MORAL-02
    void renderDetras(SpriteBatch& batch);
    float segundos() const;
    Game::Reparto reparto() const;  // PL-JUEGO-01: el de la hoja del jugador
    Texture* texturaAlas();
    // El dibujo del sprite (Entity::dibujoDePersonaje) o, sin hoja de personaje, uno de 64 px sobre la baldosa.
    void dibujo(Vector2& esquina, Vector2& tam, int& region) const;
};
