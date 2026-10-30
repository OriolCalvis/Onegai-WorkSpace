#pragma once

// U3-05.E07b -- AnimatedEntity que puede llevar su idle como clip (Game::ClipActor).
// El clip se avanza en update(), el mismo punto por el que el juego ya avanza todos los actores
// (ApplicationBucle / ApplicationCinematica), asi que no hay registro aparte: el ClipActor vive y
// muere con la entidad y no puede haber punteros colgantes al cambiar de nivel. Sin clip se
// comporta exactamente como AnimatedEntity. No hay eventos aun: los puertos no hacen nada.
// Control: mientras el clip manda, AnimatedEntity::controlExterno() es true. Si alguien llama a
// play() (tambien via AnimatedEntity*: animaciones direccionales de la cinematica o del bucle),
// la entidad recupera el control, el clip se descarta en el siguiente update() y se usa
// AnimatedEntity::update. Para volver al clip hay que llamar de nuevo a ponIdle()/idleCuerpoBase().
// Comportamiento conocido e intencional por ahora: ningun llamador del juego vuelve a llamarlo. Tras una
// cue «idle» de cinematica (ApplicationCinematica.cpp:150-171) o la animacion de un PNJ sin hoja
// (ApplicationBucle.cpp:301-318) el cuerpo base se queda en la pose que puso play(), igual que antes
// de E07b (no es regresion). Reenganchar el clip seria una E de integracion futura.
// Fase: el clip cuenta en ms (double) y AnimatedEntity en float; en multiplos exactos de 0,22 s
// el cambio de fotograma puede llegar un paso de update() antes o despues que con la animacion
// anterior. El ritmo (220 ms) y la secuencia 1..12 son los mismos.

#include <memory>

#include "Game/ClipActor.h"
#include "Render/AnimatedEntity.h"

class EntidadClip : public AnimatedEntity {
public:
    using AnimatedEntity::AnimatedEntity;
    EntidadClip(const EntidadClip&) = delete;
    EntidadClip& operator=(const EntidadClip&) = delete;

    void update(float deltaTime) override;

    // Cuerpo base sin hoja de personaje: sprite de personaje + idle desde assets/animaciones.
    // Si el asset no carga, cae a addAnimation("idle", 1..12)+play (aviso por std::cerr).
    void idleCuerpoBase(TextureAtlas* atlas, int drawW, int drawH, const Vector2& anchor);

    // `clip` nullptr = fallback (animacion propia 1..12). Entrega ya el primer fotograma.
    void ponIdle(const Game::ClipAnimacion* clip);

    bool tieneClip() const { return m_actor != nullptr; }

private:
    std::unique_ptr<Game::ClipActor> m_actor;
    std::unique_ptr<Game::DestinoFuncion> m_destino;
};
