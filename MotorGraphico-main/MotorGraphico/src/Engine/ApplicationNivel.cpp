// EST-20 (relevo B) — CARGAR UN NIVEL.
//
// loadLevel es el otro flujo largo del fichero: leer el nivel, construir
// el mundo, repartir sprites a jugador, enemigos, PNJ y objetos, y
// rehacer la sesion. Se ejecuta al arrancar, al cruzar una puerta y al
// cargar una partida, y en los tres casos tiene que dejar el mundo
// entero coherente o no dejar nada.
//
// Las dos tablas de sprites que vivian aqui (raceNpcFrame y
// boundingtonStoryFrame) son datos desde SPRITES-01: assets/textures/
// sprites.json, leido por Game::RegistroSprites y servido por
// Render::BancoAtlas.

#include "Engine/Application.h"

#include <cstdint>
#include <set>
#include "Engine/ApplicationInternal.h"
#include "Engine/ApplicationCinematica.h"

#include "Core/Math/IsoMath.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/Texture.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/ApplicationLoop.h"
#include "Engine/Window.h"
#include "Render/AnimatedEntity.h"
#include "Render/BancoAtlas.h"
#include "Render/BloquesNivel.h"
#include "Render/VidaNivel.h"
#include "Render/MonumentosNivel.h"
#include "Render/HojaPersonaje.h"
#include "Render/BlobShadow.h"
#include "Render/Camera.h"
#include "Render/DynamicLightMap.h"
#include "Render/FogOfWar.h"
#include "Engine/InputState.h"
#include "Game/CatalogoCartas.h"  // H2-06
#include "Game/EventosDeLaSesion.h"  // C8-03c
#include "Game/DiarioMisiones.h"  // GUARDAR-FICHA: capituloDeLaCampana
#include "Game/SaveGame.h"
#include "Level/LevelLoader.h"
#include "Render/PergaminoGL.h"
#include "Render/Player.h"
#include "Render/StaticEntity.h"
#include "Game/RutinasPnj.h"  // PNJ-02
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

Result<bool> Application::loadLevel(const std::string& levelPath, bool useEntry,
                                   GridCoord entry, const std::function<void()>& beforeCommit,
                                   const SaveGameData* savedGame) {
    auto levelResult = LevelLoader::loadFromFile(levelPath);
    if (!levelResult.isOk()) {
        return Result<bool>::Error("Nivel: " + levelResult.errorMessage());
    }
    LevelDefinition level = levelResult.value();

    TileMap preparedMap;
    auto mapResult = preparedMap.loadFromFile(level.mapPath);
    if (!mapResult.isOk()) {
        return Result<bool>::Error("Mapa " + level.mapPath + ": " + mapResult.errorMessage());
    }
    // AV19-02: la altura del terreno del nivel (plano si no trae "alturas").
    preparedMap.alturas().construye(level.alturas, preparedMap.getWidth(), preparedMap.getHeight());

    // --- Tileset: el que declare el propio TMX (ver TileMap::
    // tilesetImagePath). Antes se cargaba "assets/textures/
    // test_checker.png" fijo con seis defineRegion escritos a mano, asi
    // que TODOS los mapas estaban atados a ese puñado de tiles y un
    // nivel con arte propio era imposible sin recompilar. ---
    const std::string tilesetPath = preparedMap.tilesetImagePath().empty()
                                        ? std::string("assets/textures/test_checker.png")
                                        : preparedMap.tilesetImagePath();
    auto textureResult = m_textureManager.load("tileset:" + tilesetPath, tilesetPath);
    if (!textureResult.isOk()) {
        return Result<bool>::Error("Tileset " + tilesetPath + ": " + textureResult.errorMessage());
    }
    trace("loadLevel: tileset cargado");

    const int cellW = preparedMap.getTilesetTileWidth() > 0 ? preparedMap.getTilesetTileWidth() : 8;
    const int cellH = preparedMap.getTilesetTileHeight() > 0 ? preparedMap.getTilesetTileHeight() : 8;
    auto preparedAtlas = std::make_unique<TextureAtlas>(textureResult.value(), cellW, cellH);

    // Regiones declaradas en BLOQUE con la convencion de Tiled: el GID
    // firstGid+i es la celda i del atlas en orden de lectura. Antes eran
    // seis llamadas a mano que habia que ampliar cada vez que el
    // contenido usaba un sprite nuevo -- justo el origen del defecto 02
    // (objetos con spriteId 3-6 dibujados con UVs vacias). Ahora
    // cualquier tileset queda cubierto entero sin tocar codigo.
    //
    // PIXELLAB-L3 (01/10): TextureManager invierte los PNG al cargarlos,
    // asi que la fila 0 del atlas es la ULTIMA del fichero; la region del
    // GID se define con la fila contada desde abajo (rows - 1 - row), como
    // hacen HudIconos, BancoAtlas y HojaPersonaje. Hasta ahora se definia
    // con la fila de lectura y, desde que terreno_mundo.png tiene 10 filas
    // (REMAPEO), cada GID dibujaba la celda de la fila espejo: la pradera
    // (GID 1) salia como la chabola (GID 73). Los mapas se generaron por
    // NOMBRE (tools/remapear_suelos.py), asi que esto es lo que querian decir.
    const int columns = preparedMap.getTilesetColumns() > 0
                            ? preparedMap.getTilesetColumns()
                            : std::max(1, textureResult.value()->getWidth() / cellW);
    const int rows = std::max(1, textureResult.value()->getHeight() / cellH);
    const int firstGid = preparedMap.getTilesetFirstGid();
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            preparedAtlas->defineRegion(firstGid + row * columns + col, col, rows - 1 - row);
        }
    }


    if (savedGame) {
        // Validar compatibilidad contra una sesión aislada; restoreSaveGame
        // no puede tocar al Player ni al inventario de la partida viva.
        GameSession validation(&preparedMap, &m_objectCatalog, &m_skillCatalog,
                               level, nullptr, nullptr, nullptr);
        std::string error;
        if (!validation.restoreSaveGame(*savedGame, &error)) {
            return Result<bool>::Error("Guardado incompatible: " + error);
        }
    }

    // Nada del mundo vivo se sustituye hasta validar nivel, TMX y textura.
    // Las direcciones de m_map/m_objectCatalog se mantienen para sus consumidores.
    if (beforeCommit) beforeCommit();
    m_map = std::move(preparedMap);
    m_atlas = std::move(preparedAtlas);
    const auto mapId = ids::MapId::parse(std::filesystem::path(level.mapPath).stem().string());
    m_currentMapId = mapId.isOk() ? mapId.value() : ids::MapId{};

    // El atlas es NUEVO, y Player/marcadores guardan un puntero al
    // anterior: hay que re-apuntarlos o dibujarian contra memoria
    // liberada. Los marcadores se reconstruyen abajo; el Player
    // sobrevive al cambio de nivel (lleva la vida y las animaciones del
    // jugador), asi que se le cambia el atlas en caliente.
    m_player->setAtlas(m_atlas.get());
    m_player->setAlturas(&m_map.alturas());  // AV19-02
    if (m_renderer != nullptr) {
        m_renderer->setAtlas(m_atlas.get());
        // AV16-10: las transiciones hablan de los gids de terreno_mundo; con otro tileset, sin ellas.
        const bool esTerreno = tilesetPath.size() >= 17 && tilesetPath.compare(tilesetPath.size() - 17, 17, "terreno_mundo.png") == 0;
        m_renderer->setTransiciones(esTerreno ? m_transiciones.get() : nullptr, esTerreno ? m_atlasTransiciones.get() : nullptr);
    }

    // C-23 — Post-processat per nivell. Si el LevelDefinition ho demana,
    // es crea l'objecte i es connecta al renderer; si no, es desactiva
    // (zero crides GL, cost zero).
    m_fog.reset();
    m_lightMap.reset();
    m_fogRevealRadius = 1;
    if (m_renderer != nullptr) {
        m_renderer->setFogEnabled(false);
        m_renderer->setPostFXEnabled(false);
    }
    if (level.fogEnabled && m_fogShader != nullptr && m_renderer != nullptr) {
        m_fog = std::make_unique<FogOfWar>(
            m_map.getWidth(), m_map.getHeight(),
            m_width, m_height,
            m_map.getTileWidth(), m_map.getTileHeight());
        m_renderer->setFog(m_fogShader, m_fog.get(), m_width, m_height);
        m_renderer->setFogEnabled(true);
        m_fogRevealRadius = level.fogRevealRadius;
        trace("loadLevel: fog of war activat");
    }
    if (level.lightingEnabled && m_lightmapShader != nullptr && m_renderer != nullptr) {
        m_lightMap = std::make_unique<DynamicLightMap>(m_width, m_height);
        m_renderer->setPostFX(m_lightmapShader, m_lightMap->texture(), m_width, m_height);
        m_renderer->setPostFXEnabled(true);
        m_renderer->setAmbientColor(Vector4{level.ambientR, level.ambientG, level.ambientB, 1.0f});
        trace("loadLevel: dynamic lighting activat");
    }

    // Marcadores del nivel anterior fuera de la escena ANTES de destruir
    // la sesion que los respaldaba.
    for (WorldMarker& marker : m_worldMarkers) {
        if (m_renderer != nullptr) {
            m_renderer->removeFromQueue(marker.entity.get());
        }
    }
    m_worldMarkers.clear();

    // C-03 — El estado del JUGADOR (oro, moral, negocios y el contador
    // de rentas) pertenece al jugador, no al mapa: se rescata entero de
    // la sesion saliente y se devuelve a la entrante. Antes aqui solo se
    // rescataba el oro, asi que entrar a una posada reiniciaba la
    // reputacion, borraba los negocios comprados y perdia el contador de
    // rentas. Un solo tipo (PlayerCarryOver) en vez de una lista de
    // campos suelta.
    const PlayerCarryOver carried =
        m_session != nullptr ? m_session->captureCarryOver() : PlayerCarryOver{};
    // TUTORIAL-01 — Y por donde se sale de este nivel: al volver, la puerta en la que se aparezca lleva aqui.
    const std::string nivelDeAntes = m_session != nullptr && savedGame == nullptr ? m_currentLevelPath : "";
    const GridCoord puertaDeAntes = m_session != nullptr ? m_session->playerPosition() : GridCoord{};

    const GridCoord start = useEntry ? entry : level.playerStart;
    m_player->setGridPosition(start);
    level.playerStart = start;

    m_currentLevelPath = levelPath;  // F1: timers expiran por salir de la zona
    // PERGAMINO-01: el retal del minimapa. MAPA-01 (Oriol, 14/10: «si ponemos
    // Boundington al principio siempre, no sabemos en qué zona estamos»): la
    // ciudad se sobreentiende; va la zona («Barrios Altos»).
    std::string nombreZona = level.name;
    if (const std::size_t guion = nombreZona.find(" - "); guion != std::string::npos) nombreZona = nombreZona.substr(guion + 3);
    // C-17 — El World es el propietario de las entidades y lo inyectamos
    // a la sesion nueva (la que se crea abajo): el render sigue a
    // objetos/enemigos por handle estable, no por listas de GameSession.
    // La sesion de validacion de arriba es aislada y se hace con su
    // propio World (sin inyectar).
    m_world = std::make_unique<Engine::World>();
    m_session = std::make_unique<GameSession>(&m_map, &m_objectCatalog, &m_skillCatalog,
                                              std::move(level), m_player.get(), &m_playerSkills,
                                              &m_inventory, m_world.get(), &m_monsterCatalog);
    m_session->applyCarryOver(carried);
    if (!nivelDeAntes.empty()) m_session->ponVuelta(nivelDeAntes, puertaDeAntes);
    m_session->setConditionCatalog(&m_conditions);
    // H2-06 — Las cartas de clase en combate (se cargan la primera vez).
    if (!m_cartas) {
        auto cartas = Game::CatalogoCartas::cargar("assets/catalogs");
        if (cartas.isOk()) m_cartas = std::move(cartas.value());
        else std::fprintf(stderr, "Cartas de combate: %s\n", cartas.errorMessage().c_str());
    }
    m_session->setCatalogoCartas(m_cartas.get());
    // H0-04 — El catalogo de equipo viaja a la sesion con las REGLAS de
    // inventario, y con el resto de colaboradores: m_monsterCatalog entra
    // por el constructor, las condiciones por setConditionCatalog, pero las
    // reglas no tenian ni una sola llamada en todo el motor. Sin ellas
    // equipmentFor() devuelve nullptr SIEMPRE, asi que equipar y vender
    // responden "no hay catalogo de equipo cargado", las piezas quedan
    // inertes y ningun precio de la tienda se resuelve (H2-08: el precio
    // del equipo sale del catalogo real). Solo se fija `equipment`, que es
    // lo que faltaba; playerClass y tiers siguen a nullptr como estaban, y
    // cablearlos es otro item.
    // GUARDAR-FICHA — El heroe del guardado (su ficha, lo puesto y su aspecto), antes que su progresion,
    // y la aventura del capitulo en el que iba (sin esto, la del prologo: ni un beat tras «Continuar»).
    if (savedGame != nullptr && m_nuevaPartida) {
        if (RPG::CharacterSheet* ficha = m_nuevaPartida->restaurarHeroe(*savedGame)) {
            m_player->set_character_sheet(ficha);
            m_nuevaPartida->vestir(*m_player);
        }
    }
    if (savedGame != nullptr && narrativeWiring(m_narrativaLista, m_narrativeCampaignTransitions).chainCampaignTimeline) {
        const RPG::AdventureScript* capitulos[] = {&m_prologo, &m_dia1, &m_diaGorran, &m_dia2, &m_ocaso};
        m_narrative.setAdventure(capitulos[Game::capituloDeLaCampana(m_narrativeState)]);
    }
    InventoryRules inventoryRules;
    inventoryRules.equipment = &m_equipmentCatalog;
    // UI-BG3-01 — Con ficha, tambien su clase y los tiers: equipar el kit del
    // cofre recalcula la vida y las defensas con ellos.
    if (m_nuevaPartida && m_player->character_sheet() != nullptr)
        inventoryRules = m_nuevaPartida->reglasDeInventario(m_player->character_sheet());
    m_session->setInventoryRules(inventoryRules);
    if (savedGame) {
        std::string error;
        m_session->restoreSaveGame(*savedGame, &error);  // validado antes del commit
        m_player->setGridPosition(m_session->playerPosition());
    }

    // C-02 — El minimapa (ahora del tema pergamino) no guarda la sesion: la
    // recibe cada fotograma en refreshHud, asi que no puede leer una ya
    // destruida. Solo hay que decirle que la zona es otra. El cuaderno se
    // cierra: su pantalla apuntaba a la sesion que se acaba de sustituir.
    if (m_pergamino) m_pergamino->interfaz().ponZona(nombreZona, levelPath);
    m_showInventory = false;

    // Re-enchufa la narrativa en el session recien construido. loadLevel
    // destruye el GameSession en cada transicion; la narrativa (motor,
    // estado y catalogo Nd6) vive en Application y sobrevive, asi que
    // hay que volver a pasarsela al session nuevo. Sin esto, al cambiar
    // de nivel los beats dejarian de disparar.
    //
    // C-07 — La condicion era "m_narrativaLista && m_narrativeCampaignTransitions".
    // Con --precuela (o con cualquier aventura autonoma pasada por
    // --aventura) la bandera de campana queda a false, no se llamaba
    // nunca a setNarrative() y la aventura se cargaba para no hacer
    // absolutamente nada: ni un beat. La bandera decide si se hacen las
    // TRANSICIONES DE LA CRONOLOGIA de Venides (prologo -> Dia 1 -> ...,
    // en update()), no si la narrativa existe.
    //
    // La decision vive en Application::narrativeWiring (Application.h),
    // que es pura y por tanto probable sin ventana: no la reescribas a
    // mano aqui o el test de regresion deja de cubrir este if.
    if (narrativeWiring(m_narrativaLista, m_narrativeCampaignTransitions).attachToSession) {
        m_session->setNarrative(&m_narrative, &m_narrativeState);
        m_session->setNd6SkillCatalog(&m_nd6Skills);
        if (!m_eventosIntentados) {  // C8-03c: assets/eventos/<proyecto>.json, una vez
            m_eventosIntentados = true;
            std::string errorEventos;
            if (!m_projectIndex.projects().empty())
                m_eventos = Game::cargaEventosDelProyecto(m_projectIndex.projects().front().id, &errorEventos);
            if (!errorEventos.empty()) m_session->logNote("eventos: " + errorEventos);
        }
        m_session->setEventosDelMundo(m_eventos.get());
        if (savedGame) m_session->avaluaEventosDelMundo();  // reconciliacion al cargar
        // m_session ya tiene su propio Xoroshiro128p por defecto; solo
        // inyectariamos otro si quisieramos reproducibilidad (tests).
        // Dispara el beat "enter" del nivel que acaba de cargar.
        m_narrativeState.ponLugar(levelPath);  // DIALOGOS-01: requires.lugar
        if (!savedGame) m_session->enterLevelNarrative(levelPath);
    }

    trace("loadLevel: GameSession OK");
    // --- Marcadores del mundo (defecto 03): un Entity por objeto y por
    // enemigo del World, encolados en el renderer.
    //
    // Desde el sprite de personaje: NPCs y Enemigos ya NO son cuadrados
    // tintados -- son AnimatedEntity con el bebé (assets/textures/
    // personaje.png), sin tint (la distincion va por posicion y HUD, ver
    // init). Los Pickup/Prop siguen siendo StaticEntity contra el tileset
    // con tint por categoria, porque un "bebé-pocion" no tiene sentido: el
    // override de personaje es solo para actores. ---
    // BLOQUES-01 — los edificios con volumen, antes que los marcadores: las
    // puertas del nivel (objetos puerta_*) deciden que cara lleva puerta, y
    // una puerta que ya esta en la pared se dibuja abajo como umbral.
    auto esPuerta = [](const std::string& id) {
        return id.rfind("puerta_", 0) == 0 || id.rfind("on_puerta_", 0) == 0;
    };
    // ALMA-01 — las celdas que pinta un monumento (casas enteras, locales con
    // patio): la puerta que da a una de ellas ya esta dibujada en el sprite.
    std::set<std::pair<int, int>> cubiertas;
    if (m_bloques != nullptr && m_bancoAtlas != nullptr && m_renderer != nullptr && m_world != nullptr) {
        // Del World, no de level.objects: el LevelDefinition ya se movio a
        // la sesion y el World es la unica fuente de verdad (C-17).
        std::vector<GridCoord> puertas;
        m_world->eachEntity([&](Engine::EntityHandle, Engine::WorldEntity& entity) {
            if (entity.kind == Engine::EntityKind::Object && esPuerta(entity.objectId()))
                puertas.push_back(entity.position());
        });
        // BLOQUES-03 — primero los monumentos con sprite propio; sus celdas no levantan bloque.
        if (m_monumentos != nullptr) cubiertas = m_monumentos->monta(m_map, m_map.getTilesetName(), *m_bancoAtlas, *m_renderer);
        m_bloques->monta(m_map, m_map.getTilesetName(), puertas, *m_bancoAtlas, *m_renderer, &cubiertas);
        // BLOQUES-02 / MASC-01 — la capa de vida: la bandada sobre azoteas y
        // tejados del kit, y las mascotas (objetos mascota_*) por las calles,
        // por donde no hay colision ni objeto que cierre el paso. La semilla
        // sale del nombre del nivel.
        if (m_vida != nullptr) {
            std::uint32_t semilla = 2166136261u;
            for (const char c : levelPath) semilla = (semilla ^ static_cast<unsigned char>(c)) * 16777619u;
            Render::VidaNivel::Entrada vida;
            vida.bloques = &m_bloques->colocados();
            if (m_monumentos != nullptr) vida.monumentos = &m_monumentos->montados();
            m_world->eachEntity([&](Engine::EntityHandle, Engine::WorldEntity& entity) {
                const std::string id = entity.objectId();
                if (entity.kind == Engine::EntityKind::Object && id.rfind("mascota_", 0) == 0) {
                    vida.mascotes.push_back({id.substr(8), entity.position()});
                    return;
                }
                const ObjectDefinition* def = m_objectCatalog.find(id);
                if (entity.kind == Engine::EntityKind::Enemy ||
                    (def != nullptr && (def->blocksMovement || def->category == ObjectCategory::Npc)))
                    for (const GridCoord& c : Game::RutinasPnj::zona(def, entity.position(), *m_world)) vida.ocupades.insert({c.x, c.y});  // PNJ-02: y por donde pasea
            });
            m_vida->monta(vida, m_map, semilla, *m_bancoAtlas, *m_renderer, kTileW, kTileH);
        }
    }
    // PNJ-01 — cuantas veces ha salido ya cada PNJ en este nivel: la colocacion
    // n lleva su variante n (otra cara), asi la anciana o la guardia no son clones.
    std::map<std::string, int> colocados;
    auto makeMarker = [&](Engine::EntityHandle handle, const std::string& id,
                          const GridCoord& position, float scale = 1.0f) {
        if (id.rfind("mascota_", 0) == 0) return;  // MASC-01: las dibuja (y las mueve) VidaNivel
        const ObjectDefinition* def = m_objectCatalog.find(id);
        if (def == nullptr) {
            return;  // id sin definicion: nada que dibujar (permisivo)
        }
        WorldMarker marker;
        marker.handle = handle;
        marker.id = id;
        marker.position = position;

        const bool esActor =
            def->category == ObjectCategory::Npc || def->category == ObjectCategory::Enemy;
        // SPRITES-01 — cada objeto pregunta al registro por su hoja; sin ella,
        // los actores llevan el cuerpo base y los props el tile tenido.
        // BLOQUES-01 — la puerta que ya esta en la pared se dibuja como umbral.
        auto juntoAMonumento = [&](const GridCoord& c) {
            static const int dx[4] = {0, -1, 1, 0}, dy[4] = {-1, 0, 0, 1};
            for (int k = 0; k < 4; ++k)
                if (cubiertas.count({c.x + dx[k], c.y + dy[k]}) != 0) return true;
            return false;
        };
        const bool umbral = !esActor && m_bloques != nullptr && esPuerta(id) &&
                            !m_bloques->plan().umbral().empty() &&
                            (m_bloques->puertaEnPared(position) || juntoAMonumento(position));
        const Game::SpriteActor* hojaActor = esActor && m_bancoAtlas  // MODELOS-PL-01: un PNJ sin fila lleva el respaldo
            ? Game::actorConRespaldo(m_bancoAtlas->registro(), id, def->raceId, def->category == ObjectCategory::Npc) : nullptr;
        const Game::SpriteFijo* hojaFija =
            !esActor && m_bancoAtlas ? m_bancoAtlas->registro().fijo(umbral ? m_bloques->plan().umbral() : id) : nullptr;
        TextureAtlas* atlasActor =
            hojaActor ? m_bancoAtlas->fila(hojaActor->hoja, hojaActor->filaPara(colocados[id]++)) : nullptr;
        TextureAtlas* atlasFijo = hojaFija ? m_bancoAtlas->hoja(hojaFija->hoja) : nullptr;
        if (esActor) {
            // AnimatedEntity con hoja de personaje: descansa en su frame quieto mirando abajo y lleva
            // los ciclos de andar. Sin fila (solo un enemigo que no es persona: lobo, slime...), el
            // cuerpo base: la "idle" reproduce el ciclo entero de caminar, lento, para verse vivo.
            auto anim = std::make_unique<AnimatedEntity>(position, m_atlas.get(), kTileW, kTileH,
                                                          /*frameTime=*/0.22f);
            if (atlasActor != nullptr) {
                anim->setCharacterSprite(atlasActor, kCharacterDrawW, kCharacterDrawH, characterAnchor());
                HojaPersonaje::animar(*anim);
                anim->play("idle");
                anim->showRestFrame();
            } else {
                anim->setCharacterSprite(m_characterAtlas.get(), kCharacterDrawW, kCharacterDrawH,
                                         characterAnchor());
                anim->addAnimation("idle", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
                anim->play("idle");
            }
            marker.entity = std::move(anim);
        } else if (atlasFijo != nullptr) {
            // Dibujo propio (props_p0, pickups_p0...): la celda de 64x64
            // apoyada en la baldosa como un actor, sin tinte. BLOQUES-04:
            // una hoja de celdas mayores (barcos, grua: 128x128) se dibuja
            // a su tamano, centrada y con la base en la base del rombo.
            const Game::HojaSprites* hoja = m_bancoAtlas->registro().hoja(hojaFija->hoja);
            const int w = hoja != nullptr ? hoja->celdaAncho : kCharacterDrawW;
            const int h = hoja != nullptr ? hoja->celdaAlto : kCharacterDrawH;
            auto fijo = std::make_unique<StaticEntity>(position, hojaFija->frame, m_atlas.get(), kTileW, kTileH);
            fijo->setCharacterSprite(atlasFijo, w, h,
                                     Vector2{static_cast<float>(kTileW - w) * 0.5f, static_cast<float>(kTileH - h)});
            // INT-03: los dibujos del kit llevan su sombra pintada debajo (o no la quieren).
            fijo->setSombraBlob(hoja == nullptr || hoja->sombraBlob);
            marker.entity = std::move(fijo);
        } else {
            marker.entity = std::make_unique<StaticEntity>(position, def->spriteId, m_atlas.get(),
                                                           kTileW, kTileH);
            switch (def->category) {
                case ObjectCategory::Pickup:
                    marker.entity->setTint(Vector4{1.0f, 0.95f, 0.45f, 1.0f});
                    break;
                case ObjectCategory::Prop:
                    // Las puertas (props con destino) en dorado, para que se
                    // distingan del decorado: son lo unico del mapa con lo
                    // que se puede interactuar.
                    marker.entity->setTint(Vector4{0.55f, 0.9f, 0.55f, 1.0f});
                    break;
                case ObjectCategory::Npc:
                case ObjectCategory::Enemy:
                    // Actores: no llegan aqui (rama de arriba). Sin tint por
                    // defecto no haria falta, pero dejamos el caso cubierto.
                    break;
            }
        }
        // AV16-04: alfombras, felpudos y rombos de salida al plano; tejados encima.
        marker.entity->setCapa(CapasDeDibujo::capaDeObjeto(id));
        marker.entity->setAlturas(&m_map.alturas());  // AV19-02: sube con el terreno
        marker.entity->setVisualScale(scale);
        m_renderer->addToQueue(marker.entity.get());
        m_worldMarkers.push_back(std::move(marker));
    };
    // C-17 — Los marcadores nacen del World (la unica fuente de verdad) y
    // se cuelgan del HANDLE estable: a partir de aqui se siguen por
    // identidad, no por posicion ni por indice (ver refreshWorldMarkers).
    m_world->eachEntity([&](Engine::EntityHandle handle, Engine::WorldEntity& entity) {
        if (entity.kind == Engine::EntityKind::Object) {
            makeMarker(handle, entity.objectId(), entity.position(), entity.object.scale);
        } else if (entity.kind == Engine::EntityKind::Enemy) {
            makeMarker(handle, entity.objectId(), entity.position());
        }
    });
    m_session->rutinasPnj().montaNivel(*m_world, m_objectCatalog, levelPath);  // PNJ-02: los PNJ andan (RutinasPnj.h)
    if (m_vida != nullptr) m_session->rutinasPnj().ponMascotas(&m_vida->mascotes().logica());  // y no pisan a las mascotas
    // La camara sigue al jugador desde el primer frame del nivel nuevo
    // (m_lastCameraCell fuera de rango fuerza el recentrado en update).
    m_lastCameraCell = GridCoord{-9999, -9999};
    trace("loadLevel: marcadores OK");
    return Result<bool>::Ok(true);
}

// AV13-29 — Los que aparecen a mitad de partida, con dibujo (ApplicationCinematica.h): cada
// acompanante del combate ("acompanante_<id>", el PNJ <id>) y los enemigos de las emboscadas y
// oleadas (spawnEnemyFromCatalog), que hasta ahora no tenian. Retirarlos ya lo hace refreshWorldMarkers.
void ApplicationCinematica::acompanantesALaVista(Application& app) {
    if (!app.m_world || !app.m_renderer) return;
    static const std::string kPrefijo = "acompanante_";
    app.m_world->eachEntity([&](Engine::EntityHandle h, const Engine::WorldEntity& e) {
        const bool acompanante = e.kind == Engine::EntityKind::Object && e.object.objectId.rfind(kPrefijo, 0) == 0;
        if (!acompanante && e.kind != Engine::EntityKind::Enemy) return;
        for (const auto& m : app.m_worldMarkers)
            if (m.handle == h) return;  // ya lo tiene
        const std::string id = acompanante ? e.objectId().substr(kPrefijo.size()) : e.objectId();
        const ObjectDefinition* def = app.m_objectCatalog.find(id);
        const Game::SpriteActor* hoja = app.m_bancoAtlas ? Game::actorConRespaldo(  // MODELOS-PL-01
            app.m_bancoAtlas->registro(), id, def != nullptr ? def->raceId : "", acompanante) : nullptr;
        TextureAtlas* atlas = hoja ? app.m_bancoAtlas->fila(hoja->hoja, hoja->filaPara(0)) : nullptr;
        auto anim = std::make_unique<AnimatedEntity>(e.position(), app.m_atlas.get(), kTileW, kTileH, 0.22f);
        if (atlas != nullptr) {
            anim->setCharacterSprite(atlas, kCharacterDrawW, kCharacterDrawH, characterAnchor());
            HojaPersonaje::animar(*anim);
            anim->play("idle");
            anim->showRestFrame();
        } else {  // sin hoja (enemigo que no es persona): el cuerpo base
            anim->setCharacterSprite(app.m_characterAtlas.get(), kCharacterDrawW, kCharacterDrawH, characterAnchor());
            anim->addAnimation("idle", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
            anim->play("idle");
        }
        app.m_renderer->addToQueue(anim.get());
        app.m_worldMarkers.push_back(Application::WorldMarker{e.objectId(), e.position(), h, std::move(anim)});
    });
}
