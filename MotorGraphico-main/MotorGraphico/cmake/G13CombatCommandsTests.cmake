# G-13 — Comandos de combate y objetivos desde datos.
# Anticolisión multi-agent: NO linka motor_core (Application.cpp shared 14
# paquets está INCOMPLETO: equipItem/unequipItem/kPanelColor/kBorderColor
# sin declarar por otro agente).
#
# WARNING: el archivo src/RPG/SkillEffect.cpp actual usa dos símbolos que
# no existen: (1) ConditionEngine sin include directo (2)
# RPG::Catalogs::ConditionCatalog::get() que RpgCatalogs.h NO declara.
# Ambos fueron añadidos por otro agente y no compilean. En lugar de tocar
# motor_rpg shared, proporcionamos stubs aquí (solo para el test) via
# -include StubsSkillEffectG13.h preprocessor.
if (BUILD_TESTING AND TARGET motor_rpg AND TARGET motor_level)
    add_executable(test_g13_combat_commands
        tests/Game/G13CombatCommandsTests.cpp
        tests/Game/StubsSkillEffectG13.cpp
        # AV13-11 — BattleState, Skill y la logica del menu (H1-01) se toman de
        # motor_level, donde viven: ya no se compilan sueltas aqui.
    )
    target_include_directories(test_g13_combat_commands PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        "${CMAKE_CURRENT_SOURCE_DIR}/examples"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2"
        "${CMAKE_BINARY_DIR}/_deps/glfw-src/include"
        "${CMAKE_BINARY_DIR}/_deps/glm-src")
    # AV13-12: el tablero del combate (BattleState lo usa si lo tiene) vive
    # en motor_level; se enlaza la libreria, no sus fuentes sueltas.
    target_link_libraries(test_g13_combat_commands PRIVATE
        motor_level motor_json motor_rpg motor_math)
    if (APPLE)
        target_link_options(test_g13_combat_commands PRIVATE
            -Wl,-undefined,dynamic_lookup)
    endif ()
    if (TARGET copiar_assets)
        add_dependencies(test_g13_combat_commands copiar_assets)
    endif ()
    add_test(NAME test_g13_combat_commands COMMAND test_g13_combat_commands)
    set_tests_properties(test_g13_combat_commands PROPERTIES
        LABELS "unit;gl-free"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    set_property(GLOBAL APPEND PROPERTY MOTOR_GL_FREE_TARGETS test_g13_combat_commands)
    if (TARGET suite_gl_free)
        add_dependencies(suite_gl_free test_g13_combat_commands)
    endif ()
endif ()
