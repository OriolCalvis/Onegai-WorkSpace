// E-DISPARADORES-1 (D0026) — Lo que el motor pone al disparador de las cinematicas (la logica: Game/Disparador.h,
// Game/CinematicaDisparador.cpp): el ambito del nivel en curso, lo que tapa la vista y como se lleva al jugador.
// Cabecera interna que solo incluye ApplicationCinematica.cpp (en el, para no crecer ni dejar una fuente sin GL
// atrapada en motor_core: linea_base_core). Las funciones son miembros estaticos de ApplicationCinematica.
#pragma once

#include "Engine/Application.h"
#include "Engine/ApplicationCinematica.h"
#include "Level/LevelLoader.h"
#include "World/World.h"

inline const Game::Cinematica* ApplicationCinematica::laQueEmpieza(Application& app, const Game::TieneFlag& tiene) {
    EstadoCinematicas& estado = *app.m_cine;
    if (estado.ambitoDeNivel != app.m_currentLevelPath) {  // otro nivel: su ambito, del JSON y del tamano del mapa
        estado.ambitoDeNivel = app.m_currentLevelPath;
        auto nivel = LevelLoader::loadFromFile(app.m_currentLevelPath);
        estado.ambito = ambitoDelNivel(nivel.isOk() ? nivel.value().ambito : AmbitoNivel::Deducido, app.m_currentLevelPath,
                                       app.m_map.getWidth(), app.m_map.getHeight());
    }
    Game::VistaDelNivel vista;
    vista.ambito = estado.ambito;
    vista.tapa = [&app](int x, int y) {  // un tile con colision (un edificio, la roca) o un objeto que tapa la vista
        if (x < 0 || y < 0 || x >= app.m_map.getWidth() || y >= app.m_map.getHeight()) return true;
        for (int capa = 0; capa < app.m_map.getLayerCount(); ++capa)
            if (app.m_map.getTile(capa, x, y).hasCollision()) return true;
        if (!app.m_world) return false;
        const Engine::EntityHandle h = app.m_world->findObjectAt(x, y);
        const Engine::WorldEntity* e = h.valid() ? app.m_world->get(h) : nullptr;
        const ObjectDefinition* def = e != nullptr ? app.m_objectCatalog.find(e->object.objectId) : nullptr;
        return def != nullptr && def->tapaVista;  // los PNJ y los objetos pequenos no
    };
    return estado.catalogo.laQueEmpieza(app.m_currentLevelPath, app.m_session->playerPosition(), tiene, &vista);
}

inline void ApplicationCinematica::lanzaCombate(Application& app, const std::string& enemigo, const std::string& gana,
                                                 const std::string& pierde) {
    RPG::BattleRequest peticion;  // lo mismo que un beat con startBattle; huir cuenta como perder (flagHuida vacia)
    peticion.monsterId = enemigo;
    peticion.flagVictory = gana;
    peticion.flagDefeat = pierde;
    app.m_session->startNarrativeBattle(peticion);
}

inline Game::Cinematica ApplicationCinematica::efectiva(Application& app, const Game::Cinematica& c) {
    Game::LlevaDelJugador lleva;
    lleva.jugador = app.m_session->playerPosition();
    lleva.ruta = [&app](const GridCoord& destino) { return app.m_session->rutaAndando(destino); };
    return app.m_cine->catalogo.efectiva(c, &lleva);
}
