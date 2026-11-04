// C2-04a — El estado vivo de la escena, sin GL: posturas (herido, muerto), fuegos
// encendidos y bocadillos de emoción. Lo escriben las órdenes de cinemática
// (tumba/levanta, arde/apaga, emocion) y lo lee quien dibuja.
//
// PROVISIONAL: `reconstruye` rehace el fuego y las posturas persistentes a partir
// de las cinemáticas ya vistas (flag «cine_<id>_vista»). Según el criterio de
// Oriol, cuando existan los eventos del mundo (C8) el fuego y las posturas
// persistentes tendrían que venir del estado del mundo y no de haber visto la
// escena.
//
// Las emociones son efímeras: no se reconstruyen (salir del nivel las pierde).
#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Game/Cinematica.h"

namespace Game {

class EscenaViva {
public:
    struct Emocion {
        std::string actor, icono;
        float restante = 0.0f, duracion = 0.0f;
        // Opacidad (0..1): entra en kEntrada s, sale en kSalida s; si dura menos que las dos, se reparte.
        float alfa() const;
    };
    static constexpr float kEntrada = 0.15f, kSalida = 0.25f;

    // `postura`: herido | muerto | de_pie (de_pie borra la entrada).
    void postura(const std::string& actor, const std::string& postura);
    void fuego(const GridCoord& celda, bool encendido);
    void emocion(const std::string& actor, const std::string& icono, float segundos);
    // Resta tiempo a las emociones y retira las caducadas.
    void avanza(float dt);
    // Rehace fuegos y posturas de las cinemáticas de `nivel` ya vistas, en el
    // orden del catálogo (el último paso gana). No toca las emociones.
    void reconstruye(const CatalogoCinematicas& cat, const std::string& nivel, const TieneFlag& flag);

    // Alias: alegre -> encantado, enfadado -> enfurecido; el resto, tal cual.
    static std::string iconoDeEmocion(const std::string& nombre);

    const std::map<std::string, std::string>& posturas() const { return m_posturas; }
    const std::set<std::pair<int, int>>& fuegos() const { return m_fuegos; }
    const std::vector<Emocion>& emociones() const { return m_emociones; }
    // La postura del actor: «de_pie» si no tiene ninguna.
    std::string posturaDe(const std::string& actor) const;
    bool hayFuego(const GridCoord& celda) const { return m_fuegos.count({celda.x, celda.y}) > 0; }

private:
    std::map<std::string, std::string> m_posturas;
    std::set<std::pair<int, int>> m_fuegos;
    std::vector<Emocion> m_emociones;
};

}  // namespace Game
