#pragma once

#include <string>

#include "Core/Errors/Result.h"
#include "Level/LevelDefinition.h"

// Parsea un JSON de nivel (ver assets/levels/test_level.json para el
// formato) a un LevelDefinition, usando JsonValue (motor_json, Fase 6 de
// motor_grafico_gantt_rpg.puml). Result<T>, no excepcion (igual que
// ResourceManager<T>::load()): un nivel mal formado es un error
// recuperable de contenido, no un fallo de inicializacion del motor.
//
// GL-free (no incluye nada de OpenGL/GLAD): puede compilarse y probarse
// sin ventana ni contexto GL, igual que TileMap/TextureAtlas en
// motor_map (ver examples/demo_level_loader.cpp).
class LevelLoader {
public:
    static Result<LevelDefinition> loadFromFile(const std::string& path);

    // Expuesto aparte de loadFromFile() para poder testear el parseo del
    // esquema sin tocar disco (ver examples/demo_level_loader.cpp), mismo
    // motivo por el que JsonValue::parse() toma un std::string en vez de
    // una ruta.
    static Result<LevelDefinition> loadFromString(const std::string& jsonText);
};
