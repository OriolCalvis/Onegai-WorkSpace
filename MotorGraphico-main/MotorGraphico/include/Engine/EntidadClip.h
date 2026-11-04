#pragma once

// U3-05.E07b -- AnimatedEntity que puede llevar su idle como clip (Game::ClipActor).
// El clip se avanza en update(), el mismo punto por el que el juego ya avanza todos los actores
// (ApplicationBucle / ApplicationCinematica), asi que no hay registro aparte: el ClipActor vive y
// muere con la entidad y no puede haber punteros colgantes al cambiar de nivel. Sin clip se
// comporta exactamente como AnimatedEntity. No hay eventos aun: los puertos no hacen nada.
// Control: mientras el clip manda, AnimatedEntity::controlExterno() es true. Si alguien llama a
// play() (tambien via AnimatedEntity*: animaciones direccionales de la cinematica o del bucle),
// la entidad recupera el control, el clip se descarta en el siguiente update() y se usa
// AnimatedEntity::update. La guarda de update() NO cambia (D-T9).
// U3-06.E06b: para volver al clip, ponIdle()/idleCuerpoBase() o, con una maquina de animacion
// puesta (ponMaquina), aplicaEntradas(): en el flanco de perdida de control la maquina llama a
// ponClip() (reenganche). Sin maquina el comportamiento es el de siempre: tras una cue «idle» de
// cinematica el cuerpo base se queda en la pose que puso play().
// Fase: el clip cuenta en ms (double) y AnimatedEntity en float; en multiplos exactos de 0,22 s
// el cambio de fotograma puede llegar un paso de update() antes o despues que con la animacion
// anterior. El ritmo (220 ms) y la secuencia 1..12 son los mismos.
// REENTRADA PROHIBIDA: ponClip/aplicaEntradas no se llaman desde un puerto ni un DestinoFotograma
// de esta misma entidad (los puertos de EntidadClip no hacen nada).

#include <memory>

#include "Engine/MaquinaEntidad.h"  // completo: el constructor heredado (ApplicationNivel.cpp) destruye m_maquina
#include "Game/CatalogoAnimacion.h"
#include "Game/ClipActor.h"
#include "Game/SeleccionAnimacion.h"
#include "Render/AnimatedEntity.h"

class EntidadClip : public AnimatedEntity {
public:
    using AnimatedEntity::AnimatedEntity;
    EntidadClip(const EntidadClip&) = delete;
    EntidadClip& operator=(const EntidadClip&) = delete;
    ~EntidadClip() override;

    void update(float deltaTime) override;

    // Cuerpo base sin hoja de personaje: sprite de personaje + idle desde assets/animaciones.
    // Si el asset no carga, cae a addAnimation("idle", 1..12)+play (aviso por std::cerr).
    // Marca la entidad como cuerpo base (esCuerpoBase()).
    void idleCuerpoBase(TextureAtlas* atlas, int drawW, int drawH, const Vector2& anchor);

    // `clip` nullptr = fallback (animacion propia 1..12). Entrega ya el primer fotograma.
    void ponIdle(const Game::ClipAnimacion* clip);

    // Reconstruye el ClipActor con `clip` y recupera el control; entrega ya el primer fotograma
    // (avanza(0.0) -> ponerFotograma, deja controlExterno() cierto antes del siguiente update).
    void ponClip(const Game::ClipAnimacion& clip);

    // Pone (o quita, con `maquina` nula) la maquina de animacion. No propietaria: ambos punteros son
    // del catalogo de proceso (Game::catalogoAnimacionDelJuego), inmutable y de vida de proceso.
    void ponMaquina(const Game::CatalogoAnimacion* catalogo, const Game::MaquinaAnimacion* maquina);

    // Un fotograma de la maquina. `enRuta`: otro propietario manda (ruta de combate, D-T7).
    // Devuelve false SIN tocar nada si no hay maquina (el llamador aplica la politica de siempre).
    bool aplicaEntradas(const Game::EntradasAnimacion& entradas, bool enRuta);

    bool esCuerpoBase() const;
    bool tieneClip() const { return m_actor != nullptr; }

private:
    std::unique_ptr<Game::ClipActor> m_actor;
    std::unique_ptr<Game::DestinoFuncion> m_destino;
    std::unique_ptr<Engine::MaquinaEntidad> m_maquina;
};
