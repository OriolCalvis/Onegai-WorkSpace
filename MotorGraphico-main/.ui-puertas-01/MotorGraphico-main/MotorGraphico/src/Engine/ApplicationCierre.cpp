// EST-20 (relevo A) — ABRIR UN PROYECTO Y CERRAR LA APLICACION.
//
// Los dos extremos de la vida del proceso. shutdown es el unico sitio del
// motor donde el orden importa de verdad: los recursos GL tienen que
// morir ANTES que el contexto que los creo (C-05), y un cierre que se
// equivoca de orden no falla aqui: falla en el driver, sin traza.
// Aislarlo es lo que permite leerlo entero de una sentada.

#include "Engine/Application.h"

#include "Core/Math/IsoMath.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/ApplicationLoop.h"
#include "Engine/Window.h"
#include "Render/AnimatedEntity.h"
#include "Render/BlobShadow.h"
#include "Render/Camera.h"
#include "Render/DynamicLightMap.h"
#include "Render/FogOfWar.h"
#include "Engine/InputState.h"
#include "Game/SaveGame.h"
#include "Level/LevelLoader.h"
#include "Render/AuraJugador.h"
#include "Render/BancoAtlas.h"
#include "Render/BloquesNivel.h"
#include "Render/VidaNivel.h"
#include "Render/MonumentosNivel.h"
#include "Render/PergaminoGL.h"
#include "Render/Player.h"
#include "Render/StaticEntity.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

// glad ANTES que GLFW (mismo orden y motivo que src/Engine/Window.cpp).
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include "Engine/ApplicationInternal.h"

using namespace Engine::Interno;

Result<bool> Application::openProject(const Editor::Project& project) {
    // Preparar catálogos y guion sin tocar los que sigue usando la campaña.
    ObjectCatalog catalog = m_objectCatalog;
    for (const auto& file : project.catalogs) {
        const auto path = "assets/" + project.settings.catalogsDirectory + "/" + file;
        const auto loaded = catalog.loadFromFile(path);
        if (!loaded.isOk()) return Result<bool>::Error("Catálogo " + path + ": " + loaded.errorMessage());
    }
    std::string levelPath;
    RPG::AdventureScript script;
    const bool hasAdventure = project.entryKind == Editor::Project::EntryKind::Adventure;
    if (project.entryKind == Editor::Project::EntryKind::Level) {
        levelPath = project.entryPath;
    } else if (hasAdventure) {
        const auto loaded = RPG::AdventureScript::loadFromFile(project.entryPath);
        if (!loaded.isOk()) return Result<bool>::Error(loaded.errorMessage());
        if (project.levels.empty()) return Result<bool>::Error("El proyecto no declara niveles");
        script = loaded.value();
        levelPath = "assets/" + project.settings.levelsDirectory + "/" + project.levels[0];
    } else {
        return Result<bool>::Error("El proyecto no tiene entrada jugable");
    }
    return loadLevel(levelPath, false, GridCoord{}, [&] {
        m_objectCatalog = std::move(catalog);
        m_narrativaLista = hasAdventure;
        m_narrativeCampaignTransitions = false;
        m_narrativeState = RPG::NarrativeState{};
        if (hasAdventure) {
            m_prologo = std::move(script);
            m_narrative.setAdventure(&m_prologo);
        } else {
            m_narrative.setAdventure(nullptr);
        }
    });
}

void Application::shutdown() {
    trace("shutdown: inicio");
    // Orden inverso al de creacion: lo que depende de la ventana/contexto
    // GL (texturas, fuente, renderer) muere ANTES que el Window, o sus
    // destructores llamarian a glDeleteTextures sin contexto valido.
    // PRUEBAS-01 — y en SU contexto: si hay otra Application viva (las
    // pruebas de cargar partida), el actual puede ser el de la otra.
    if (m_window) m_window->makeCurrent();
    m_running = false;
    m_scenes.shutdown();
    // C-02 — La interfaz de pergamino no guarda la sesion entre
    // fotogramas (la recibe en cada EstadoJuego): soltarla aqui es seguro.
    m_session.reset();
    m_hud = HudManager{};  // suelta los punteros a widgets antes de destruirlos
    m_title.reset();
    m_businessMenu.reset();
    m_businessText.reset();
    m_businessPanel.reset();
    m_shopMenu.reset();
    m_shopTitle.reset();
    m_shopPanel.reset();
    m_dialogue.reset();
    m_dialoguePanel.reset();
    m_hintText.reset();
    m_fpsText.reset();
    m_font.reset();
    // PERGAMINO-01 — texturas y hojas de render del tema: con el contexto
    // GL vivo y despues de m_hud, que tenia sus dos capas registradas. Su
    // rombo del suelo sale antes de la cola del renderer.
    if (m_renderer && m_pergamino)
        for (IRenderable* r : m_pergamino->delMundo()) m_renderer->removeFromQueue(r);
    m_pergamino.reset();
    m_hudBatch.reset();  // recurso GL: antes que la Window
    m_shadowTexture.reset();
    m_whiteTexture.reset();
    // C-23 — Fog/LightMap posseeixen textures GL: s'han de destruir
    // abans del renderer (que les referenciava via setFog/setPostFX).
    m_fog.reset();
    m_lightMap.reset();
    // PERGAMINO-01 — el aura sale de la cola antes que el renderer y muere
    // antes que el Player al que sigue.
    if (m_renderer && m_aura) m_aura->desencolar(*m_renderer);
    m_aura.reset();
    // BLOQUES-01 — sus entidades salen de la cola antes que el renderer.
    if (m_renderer && m_vida) m_vida->desmonta(*m_renderer);
    m_vida.reset();
    if (m_renderer && m_monumentos) m_monumentos->desmonta(*m_renderer);
    m_monumentos.reset();
    if (m_renderer && m_bloques) m_bloques->desmonta(*m_renderer);
    m_bloques.reset();
    m_renderer.reset();
    m_worldMarkers.clear();  // tras el renderer (que solo los referenciaba)
    m_player.reset();
    // APARIENCIA-01 — Sus hojas de personaje son texturas GL (y el Player,
    // ya destruido, llevaba su ficha y su atlas): antes que la Window.
    m_nuevaPartida.reset();
    m_camera.reset();
    m_atlas.reset();
    m_characterAtlas.reset();  // mismo criterio que m_atlas (sin GL propio)
    m_bancoAtlas.reset();      // SPRITES-01: sus atlas tampoco poseen textura

    // C-05 — Los dos ResourceManager se vacian AQUI, con el contexto GL
    // todavia vivo. Antes no se tocaban en shutdown(): sobrevivian hasta
    // el destructor de Application, que corre DESPUES de que
    // ~Window haya llamado a glfwTerminate() -- y entonces
    // ~Texture/~Shader emitian glDeleteTextures/glDeleteProgram sin
    // ningun contexto donde ir. Van despues de los atlas, que solo
    // referencian esas texturas y no las poseen. Idempotente: shutdown()
    // se llama a mano y otra vez desde el destructor.
    m_shaderManager.clear();
    m_textureManager.clear();

    m_window.reset();  // ~Window -> glfwTerminate() si era la ultima: nada de GL despues de esto
}
