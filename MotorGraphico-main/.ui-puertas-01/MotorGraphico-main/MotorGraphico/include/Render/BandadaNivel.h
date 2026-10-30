// BLOQUES-02 — Los pajaros de la ciudad en la cola del renderer.
//
// Game::Bandada decide donde esta cada pajaro (sin GL); esto les pone una
// entidad con la hoja «pixellab_pajaros» del BancoAtlas (celdas de 32x32:
// por especie, posado / alas arriba / alas abajo mirando a la derecha y las
// tres espejadas) y la mueve cada frame. Un pajaro en vuelo se ordena por
// encima de todo (va a 128 px, mas alto que cualquier bloque); uno posado,
// justo despues del bloque de su azotea. La sombra se queda en el suelo.
//
// Implementado en src/Render/VidaNivel.cpp (la capa de vida en una sola
// unidad). Colaborador de Render/VidaNivel (MASC-01; antes, de Application): monta()
// cada loadLevel con las azoteas de BloquesNivel y los tejados del kit,
// avanza() en update() y desmonta() al salir.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/Bandada.h"
#include "Game/Bloques.h"
#include "Render/Entity.h"

class IsometricRenderer;
class TextureAtlas;

namespace Render {

class BancoAtlas;

class PajaroEntity : public Entity {
public:
    PajaroEntity(TextureAtlas* atlas, int tileW, int tileH);
    void update(float /*dt*/) override {}
    int getSortKey() const override;
    void renderShadow(SpriteBatch& batch, Texture* shadowTexture) override;
    // Coloca la entidad donde dice el pajaro (celda + desplazamiento continuo
    // + altura) y elige su frame (base 1-based de la especie en la hoja).
    // «orden»: la celda que ordena el dibujo de un pajaro posado en un tejado
    // del kit (MASC-01); {-1, -1} = la suya.
    void sigue(const Game::Pajaro& p, int frameBase, const GridCoord& orden = GridCoord{-1, -1});

private:
    float m_altura = 0.0f;
    bool m_volando = false;
};

class BandadaNivel {
public:
    BandadaNivel();
    ~BandadaNivel();
    BandadaNivel(const BandadaNivel&) = delete;
    BandadaNivel& operator=(const BandadaNivel&) = delete;

    // Monta una bandada sobre las azoteas del plan de bloques y los tejados
    // de los monumentos del kit (MASC-01). Sin azoteas, o sin hoja
    // «pixellab_pajaros», no hay pajaros.
    void monta(const std::vector<Game::BloqueColocado>& plan, const std::vector<Game::Azotea>& tejados,
               std::uint32_t semilla, BancoAtlas& banco, IsometricRenderer& renderer, int tileW, int tileH);
    void desmonta(IsometricRenderer& renderer);
    void avanza(float dt, const GridCoord& jugador);
    std::size_t cuantos() const { return m_entidades.size(); }

private:
    GridCoord ordenDe(const Game::Pajaro& p) const;

    Game::Bandada m_bandada;
    std::vector<std::unique_ptr<PajaroEntity>> m_entidades;
    std::vector<int> m_frameBase;  // por especie, 1-based en la hoja
    IsometricRenderer* m_renderer = nullptr;
    bool m_avisado = false;
};

}  // namespace Render
