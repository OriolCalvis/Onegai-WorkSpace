// UI-BG3-01 — El cofre de El Vacio: el kit de la clase, a la mochila.
//
// Oriol (05/10): «en la pantalla del sueño, que es la primera escena, haya
// un cofre con el equipo basico para la clase seleccionada» y el kit «va a la
// mochila». Maquetas C04a/C04b: el personaje empieza sin equipo; al lado hay
// un cofre ([E] Abrir cofre); al abrirlo, un papiro dice lo que trae, lo
// guarda en la mochila y da el objetivo de ponerselo.
//
// El cofre es un objeto del nivel («cofre», assets/objects/vacio_objetos.json)
// y su estado son dos flags de la partida, que se guardan con ella:
//   kit_basico_recogido  se abrio (el cofre ya no esta);
//   kit_basico_puesto    no queda en la mochila ninguna pieza del kit sin
//                        poner (puesta, o tirada: nunca se queda atascado).
// El prologo espera a «puesto» para seguir (ver boundington_prologo.json).
// Si un nivel no tiene cofre, las dos flags se dan solas: sin cofre no se
// puede pedir que se abra.
//
// GL-free. Lo usa Application (que tiene el mundo, la mochila y las flags):
// GameSession.h esta en su techo de lineas.
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "World/EntityHandle.h"

namespace Engine {
class World;
}
namespace RPG {
class NarrativeState;
}

namespace Game {

class CofreKit {
public:
    // OBJETOS-01: su propio id. Era «cofre», el mismo que el mueble de los
    // interiores (mobles_kit.json): en Boundington la ficha de El Vacio pisaba
    // la del mueble, y revisa() borraba los cofres de la posada o la opera.
    static constexpr const char* kObjeto = "cofre_del_vacio";
    static constexpr const char* kRecogido = "kit_basico_recogido";
    static constexpr const char* kPuesto = "kit_basico_puesto";

    // El papiro del cofre abierto (C04b): que hay y con que se queda el foco.
    struct Pieza {
        std::string nombre;  // «Armadura de Cuero»
        std::string ranura;  // «Torso»
        std::string bono;    // «+1 DES» (vacio si no sube nada)
    };
    struct Papiro {
        std::string clase;  // «Bardo de las Cuatro Estaciones»
        std::vector<Pieza> piezas;
        int foco = 0;  // 0 = acción, 1 = Cerrar
        std::string contenedor;  // vacío = kit; identidad estable del botín
        std::string accion = "Ponérmelo";
    };

    // El cofre al alcance de la E: la celda del jugador y las cuatro de al
    // lado, en el orden de GameSession::interact(). Invalido si no hay.
    static Engine::EntityHandle alAlcance(const Engine::World& mundo, const GridCoord& jugador);

    // Abre el cofre: los objetos del kit (`objetos`, ya en ids de la mochila)
    // a la mochila, la flag «recogido», el cofre fuera del mundo y el papiro
    // abierto con lo que trae (`piezas`, ids de equipment.json).
    void abre(Engine::World& mundo, Engine::EntityHandle cofre, const std::vector<std::string>& piezas,
              const std::vector<std::string>& objetos, const RPG::Catalogs::EquipmentCatalog& equipo,
              const std::string& clase, std::vector<std::string>& mochila, RPG::NarrativeState& flags);

    // Cada fotograma: el cofre ya abierto no vuelve al cargar el nivel;
    // «puesto» cuando no queda en la mochila ninguna pieza del kit; y un
    // nivel sin cofre da las dos flags. `objetos`: el kit en ids de mochila.
    void revisa(Engine::World& mundo, const std::vector<std::string>& mochila, const std::vector<std::string>& objetos,
                RPG::NarrativeState& flags) const;

    const Papiro* papiro() const { return m_abierto ? &m_papiro : nullptr; }
    void enfoca(int foco) { m_papiro.foco = foco < 0 ? 0 : (foco > 1 ? 1 : foco); }
    void cierra() { m_abierto = false; }
    void presenta(Papiro papiro) { m_papiro = std::move(papiro); m_abierto = true; }

private:
    Papiro m_papiro;
    bool m_abierto = false;
};

}  // namespace Game
